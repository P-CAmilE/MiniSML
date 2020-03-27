#include "AST.h"

static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

/*-------------------------------------------
			   Error Handle
-------------------------------------------*/
static Value* ValCodeGenError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "CodeGen Error: %s\n", info);
	return nullptr;
}

static Function* FunCodeGenError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "CodeGen Error: %s\n", info);
	return nullptr;
}

//=---------------------------------------------------------------------------=//

static Function* getFunction(Container* con, string Name) {
	// First, see if the function has already been added to the current module.
	if (auto* F = con->TheModule->getFunction(Name))
		return F;

	// If not, check whether we can codegen the declaration from some existing
	// prototype.
	auto FI = FunctionProtos.find(Name);
	if (FI != FunctionProtos.end())
		return FI->second->codegen(con);

	// If no existing prototype exists, return null.
	return nullptr;
}

// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
/// the function.  This is used for mutable variables etc.
static AllocaInst* CreateEntryBlockAlloca(Container* con, Function* TheFunction,
	const string& VarName, string type) {
	IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
		TheFunction->getEntryBlock().begin());
	if (type == "real")
		return TmpB.CreateAlloca(Type::getDoubleTy(con->TheContext), nullptr, VarName);
	else
		return TmpB.CreateAlloca(Type::getInt32Ty(con->TheContext), nullptr, VarName);
}

Value* NumberExprAST::codegen(Container* con) {
	// return a int constant
	if (type == INT)
		return ConstantInt::get(Type::getInt32Ty(con->TheContext), (int32_t)NumVal, true);
	else
		return ConstantFP::get(Type::getDoublePtrTy(con->TheContext), APFloat(NumVal));
}


Value* VariableExprAST::codegen(Container* con) {
	// Look this variable up in the function.
	Value* Vari = con->NamedValues[VariName];
	if (!Vari) {
		// if not local, seaerch Global
		Vari = con->GlobalValues[VariName];
		if (!Vari)
			return ValCodeGenError(con, "Unknown variable name");
		else
			return Vari;
	}

	// Load the value.
	return con->Builder.CreateLoad(Vari, VariName.c_str());
}

Value* CallExprAST::codegen(Container* con) {
	// Look up the name in the global module table.
	Function* CalleeF = getFunction(con, Callee);
	if (!CalleeF)
		return ValCodeGenError(con, "Unknown function referenced");

	// If argument mismatch error.
	if (CalleeF->arg_size() != Args.size())
		return ValCodeGenError(con, "Incorrect arguments passed");

	std::vector<Value*> ArgsV;
	if (Args.size() != 0) {
		for (unsigned i = 0, e = Args.size(); i != e; ++i) {
			ArgsV.push_back(Args[i]->codegen(con));
			if (!ArgsV.back())
				return nullptr;
		}
		return con->Builder.CreateCall(CalleeF, ArgsV, "calltmp");
	}
	else
		return con->Builder.CreateCall(CalleeF, None, "Calltmp");
}

Value* LetExprAST::codegen(Container* con) {
	vector<AllocaInst*> OldBindings;
	// get the function container
	Function* TheFunction = con->Builder.GetInsertBlock()->getParent();

	// Register all variables and emit their initializer.
	for (unsigned i = 0, e = LetDec.size(); i != e; ++i) {
		string VarName = LetDec[i]->getName();
		ExprAST* Init = LetDec[i]->getExpr();
		Value* InitVal;
		// extern the specified of variable	definition
		if (Init) {
			InitVal = Init->codegen(con);
			if (!InitVal)
				// error in low level function
				return nullptr;
		}
		else // If not specified, use 0.0 (set default value for variable)
			InitVal = ConstantFP::get(con->TheContext, APFloat(0.0));

		AllocaInst* Alloca = CreateEntryBlockAlloca(con, TheFunction, VarName, "real");
		con->Builder.CreateStore(InitVal, Alloca);

		// Remember the old variable binding so that we can restore the binding when
		// we unrecurse.
		OldBindings.push_back(con->NamedValues[VarName]);

		// Remember this binding.
		con->NamedValues[VarName] = Alloca;
	}

	// Codegen the body, now that all vars are in scope.
	Value* BodyVal = InExpr->codegen(con);
	if (!BodyVal)
		return nullptr;

	// Pop all our variables from scope.
	for (unsigned i = 0, e = LetDec.size(); i != e; ++i)
		con->NamedValues[LetDec[i]->getName()] = OldBindings[i];

	// Return the body computation.
	return BodyVal;
}

Value* BinaryExprAST::codegen(Container* con) {
	Value* L = LHS->codegen(con);
	Value* R = RHS->codegen(con);
	if (!L || !R)
		return nullptr;

	switch (Op) {
	case '+':
		if ((L->getType() == Type::getDoubleTy(con->TheContext)) &&
			(R->getType() == Type::getDoubleTy(con->TheContext)))
			return con->Builder.CreateFAdd(
				con->Builder.CreateUIToFP(L, Type::getDoubleTy(con->TheContext)),
				con->Builder.CreateUIToFP(R, Type::getDoubleTy(con->TheContext)), "addtmp");
		else if ((L->getType() == Type::getInt32Ty(con->TheContext)) &&
			(R->getType() == Type::getInt32Ty(con->TheContext)))
			return con->Builder.CreateAdd(L, R, "addtmp");
		else
			return ValCodeGenError(con, "only [int + int] or [real + real] supported");
	case '-':
		if ((L->getType() == Type::getDoubleTy(con->TheContext)) &&
			(R->getType() == Type::getDoubleTy(con->TheContext)))
			return con->Builder.CreateFSub(
				con->Builder.CreateUIToFP(L, Type::getDoubleTy(con->TheContext)),
				con->Builder.CreateUIToFP(R, Type::getDoubleTy(con->TheContext)), "subtmp");
		else if ((L->getType() == Type::getInt32Ty(con->TheContext)) &&
			(R->getType() == Type::getInt32Ty(con->TheContext)))
			return con->Builder.CreateSub(L, R, "subtmp");
		else
			return ValCodeGenError(con, "only [int - int] or [real - real] supported");
	case '*':
		if ((L->getType() == Type::getDoubleTy(con->TheContext)) &&
			(R->getType() == Type::getDoubleTy(con->TheContext)))
			return con->Builder.CreateFMul(
				con->Builder.CreateUIToFP(L, Type::getDoubleTy(con->TheContext)),
				con->Builder.CreateUIToFP(R, Type::getDoubleTy(con->TheContext)), "multmp");
		else if ((L->getType() == Type::getInt32Ty(con->TheContext)) &&
			(R->getType() == Type::getInt32Ty(con->TheContext)))
			return con->Builder.CreateMul(L, R, "multmp");
		else
			return ValCodeGenError(con, "only [int * int] or [real * real] supported");
	case '<':
		if ((L->getType() == Type::getDoubleTy(con->TheContext)) &&
			(R->getType() == Type::getDoubleTy(con->TheContext))) {
		/*	return con->Builder.CreateFCmpULT(L, R, "cmptmp");*/
			L = con->Builder.CreateFCmpULT(L, R, "cmptmp");
			return con->Builder.CreateUIToFP(L, Type::getDoubleTy(con->TheContext), "booltmp");
		}
		else if ((L->getType() == Type::getInt32Ty(con->TheContext)) &&
			(R->getType() == Type::getInt32Ty(con->TheContext)))
			return con->Builder.CreateICmpULT(L, R, "cmptmp");
		else
			return ValCodeGenError(con, "only [int < int] or [real < real] supported");
	case '>':
		if ((L->getType() == Type::getDoubleTy(con->TheContext)) &&
			(R->getType() == Type::getDoubleTy(con->TheContext))) {
			//return con->Builder.CreateFCmpULT(R, L, "cmptmp");
			L = con->Builder.CreateFCmpULT(R, L, "cmptmp");
			return con->Builder.CreateUIToFP(L, Type::getDoubleTy(con->TheContext), "booltmp");
		}
		else if ((L->getType() == Type::getInt32Ty(con->TheContext)) &&
			(R->getType() == Type::getInt32Ty(con->TheContext)))
			return con->Builder.CreateICmpULT(R, L, "cmptmp");
		else
			return ValCodeGenError(con, "only [int > int] or [real > real] supported");
	default:
		return ValCodeGenError(con, "undefined op");
	}
}

Value* IfExprAST::codegen(Container* con) {
	Value* ifVal = IfExpr->codegen(con);
	if (!ifVal)
		return nullptr;

	// Convert condition to a bool by comparing non-equal to 0.0.
	bool isDouble = false;
	if (ifVal->getType() == Type::getDoubleTy(con->TheContext)) {
		isDouble = true;
		ifVal = con->Builder.CreateFCmpONE(
			ifVal, ConstantFP::get(con->TheContext, APFloat(0.0)), "ifcond");
	}
	else
		ifVal = con->Builder.CreateICmpNE(ifVal,
			ConstantInt::get(Type::getInt32Ty(con->TheContext), (int32_t)0, true),"ifcond");
	
	Function* TheFunction = con->Builder.GetInsertBlock()->getParent();

	// Create blocks for the then and else cases.  Insert the 'then' block at the
	// end of the function.
	BasicBlock* ThenBB = BasicBlock::Create(con->TheContext, "then", TheFunction);
	BasicBlock* ElseBB = BasicBlock::Create(con->TheContext, "else");
	BasicBlock* MergeBB = BasicBlock::Create(con->TheContext, "ifcont");

	con->Builder.CreateCondBr(ifVal, ThenBB, ElseBB);

	// Emit then value.
	con->Builder.SetInsertPoint(ThenBB);

	Value* ThenV = ThenExpr->codegen(con);
	if (!ThenV)
		return nullptr;

	con->Builder.CreateBr(MergeBB);
	// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
	ThenBB = con->Builder.GetInsertBlock();

	// Emit else block.
	TheFunction->getBasicBlockList().push_back(ElseBB);
	con->Builder.SetInsertPoint(ElseBB);

	Value* ElseV = ElseExpr->codegen(con);
	if (!ElseV)
		return nullptr;

	con->Builder.CreateBr(MergeBB);
	// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
	ElseBB = con->Builder.GetInsertBlock();

	// Emit merge block.
	TheFunction->getBasicBlockList().push_back(MergeBB);
	con->Builder.SetInsertPoint(MergeBB);

	PHINode* PN = nullptr;

	if (isDouble)
		PN = con->Builder.CreatePHI(Type::getDoubleTy(con->TheContext), 2, "iftmp");
	else
		PN = con->Builder.CreatePHI(Type::getInt32Ty(con->TheContext), 2, "iftmp");

	PN->addIncoming(ThenV, ThenBB);
	PN->addIncoming(ElseV, ElseBB);

	return PN;
}

Value* ValueDecAST::codegen(Container* con) {
	auto Val = ValExpr->codegen(con);
	con->GlobalValues[ValName] = Val;
	return Val;
}

Function* PrototypeAST::codegen(Container* con) {
	FunctionType* FT = nullptr;
	vector<Type*> argsType;
	bool isNoArgs = 1; // store whether it has args

	if (ArgsType.size() != 0) {
		isNoArgs = 0;
		for (unsigned i = 0, e = ArgsType.size(); i != e; ++i) {
			if (ArgsType[i] == INT)
				argsType.push_back(Type::getInt32Ty(con->TheContext));
			else if (ArgsType[i] == REAL)
				argsType.push_back(Type::getDoubleTy(con->TheContext));
			else
				return FunCodeGenError(con, "unknown args type in fun declaration");
		}
	}

	if (RetType == INT)
		if(isNoArgs)
			FT = FunctionType::get(Type::getInt32Ty(con->TheContext), false);
		else
			FT = FunctionType::get(Type::getInt32Ty(con->TheContext), argsType, false);
	else if(RetType == REAL)
		if(isNoArgs)
			FT = FunctionType::get(Type::getDoubleTy(con->TheContext), false);
		else
			FT = FunctionType::get(Type::getDoubleTy(con->TheContext), argsType, false);
	else
		return FunCodeGenError(con, "unknown ret type in fun declaration");

	Function* F =
		Function::Create(FT, Function::ExternalLinkage, FuncName, con->TheModule.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for (auto& Arg : F->args())
		Arg.setName(ArgsName[Idx++]);

	return F;
}

Function* FunctionDecAST::codegen(Container* con, Type** TypeToReturn) {
	// Transfer ownership of the prototype to the FunctionProtos map, but keep a
  // reference to it for use below.
	auto& P = *Proto;
	FunctionProtos[Proto->getName()] = std::move(Proto);
	Function* TheFunction = getFunction(con, P.getName());
	if (!TheFunction)
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(con->TheContext, "entry", TheFunction);
	con->Builder.SetInsertPoint(BB);

	// Record the function arguments in the NamedValues map.
	con->NamedValues.clear();
	for (auto& Arg : TheFunction->args()) {
		AllocaInst* Alloca;
		if (TheFunction->getReturnType() == Type::getDoubleTy(con->TheContext))
			// Create an alloca for this variable.
			Alloca = CreateEntryBlockAlloca(con, TheFunction, Arg.getName(), "real");
		else if (TheFunction->getReturnType() == Type::getInt32Ty(con->TheContext))
			Alloca = CreateEntryBlockAlloca(con, TheFunction, Arg.getName(), "int");

		// Store the initial value into the alloca.
		con->Builder.CreateStore(&Arg, Alloca);

		// Add arguments to variable symbol table.
		con->NamedValues[Arg.getName()] = Alloca;
	}

	if (Value* RetVal = FuncBody->codegen(con)) {
		// Give the back type
		*TypeToReturn = RetVal->getType();

		// Finish off the function.
		con->Builder.CreateRet(RetVal);

		// Validate the generated code, checking for consistency.
		verifyFunction(*TheFunction);

		// Run the optimizer on the function.
		con->TheFPM->run(*TheFunction);

		return TheFunction;
	}

	// Error reading body, remove function.
	TheFunction->eraseFromParent();
	return nullptr;
}