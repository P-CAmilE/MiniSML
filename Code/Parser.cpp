/*---------------------------------------------------
	this file implements the Parser
---------------------------------------------------*/

#include "Parser.h"

int Parser::getNextToken(Container *con) { return con->CurTok = lexer->gettok(con); }

int Parser::GetTokPrecedence(Container* con) {
	if (con->CurTok != tok_andalso && con->CurTok != tok_orelse && con->CurTok != tok_div &&
		con->CurTok != '>' && con->CurTok != '<>' && con->CurTok != '<=' && con->CurTok != '>=' &&
		con->CurTok != '=' && con->CurTok != '+' && con->CurTok != '-' && con->CurTok != '^' &&
		con->CurTok != '*' && con->CurTok != '<' && con->CurTok != '/')
		return -1;

	return con->BinopPrecedence[con->CurTok];
}

/*-------------------------------------------
				expression
-------------------------------------------*/

// parse int and real value
unique_ptr<ExprAST> Parser::ParseNumberExpr(Container* con) {
	if (con->CurTok == tok_int) {
		auto Result = llvm::make_unique<NumberExprAST>(con->NumVal, INT);
		getNextToken(con); // eat tok_int
		return std::move(Result);
	}
	else if (con->CurTok == tok_real) {
		auto Result = llvm::make_unique<NumberExprAST>(con->NumVal, REAL);
		getNextToken(con); // eat tok_real
		return std::move(Result);
	}
	return nullptr;
}

/// VariableExpr ::= Variable
///	CallExpr	 ::= Variable '(' ')'
///				 ::= Variable '(' Expression (',' Expression)* ')'
unique_ptr<ExprAST> Parser::ParseVariableExpr(Container* con) {
	string VariName = con->IdentifierStr;
 
	if (getNextToken(con) != '(') // eat identifier.
		return llvm::make_unique<VariableExprAST>(VariName);

	vector<unique_ptr<ExprAST>> Args;
	if (getNextToken(con) == ')') // eat (
		return llvm::make_unique<CallExprAST>(VariName, std::move(Args));

	auto PrExpr = ParseExpression(con);
	if (!PrExpr)
		// if error	
		return nullptr;

	Args.push_back(move(PrExpr));
	while (con->CurTok == ',') {
		getNextToken(con); // eat ,
		auto tmp = ParseExpression(con);
		if (!tmp)
			// if error or lack of expression
			return nullptr;
		Args.push_back(move(tmp));
	}
	if (con->CurTok != ')')
		return ParserExprError(con, "coupled parenthses without ')'");
	getNextToken(con); // eat )
	return llvm::make_unique<CallExprAST>(VariName, std::move(Args));
}

/// ParenExpr ::=  '(' Expression ')'
unique_ptr<ExprAST> Parser::ParseParenExpr(Container* con) {
	getNextToken(con); // eat (.
	auto expr = ParseExpression(con);
	if (!expr)
		return nullptr;

	if (con->CurTok != ')')
		return ParserExprError(con, "coupled parenthses without ')'");
	getNextToken(con); // eat ).
	return move(expr);
}

/// LetExpr ::= tok_let Value (',' value)* tok_in Expression tok_end
unique_ptr<ExprAST> Parser::ParseLetExpr(Container* con) {
	if (getNextToken(con) != tok_val) // eat let
		return ParserExprError(con, "expected var keyword in LET expression");
	
	auto prDec = ParseValue(con);
	if (!prDec)
		return nullptr;

	vector<unique_ptr<ValueDecAST>> letDec;
	letDec.push_back(move(prDec));
	while (con->CurTok == ',') {
		if(getNextToken(con) != tok_val)
			return ParserExprError(con, "expected var keyword in LET expression");
		auto tmp = ParseValue(con);
		letDec.push_back(move(tmp));
	}
	
	if (con->CurTok != tok_in)
		return ParserExprError(con, "lack of token(in) in LET expression");

	getNextToken(con);  // eat in
	auto expr = ParseExpression(con);
	if (!expr)
		// if error or lack of expression 
		return nullptr;

	if (con->CurTok != tok_end)
		return ParserExprError(con, "lack of token(end) in LET expression");
	
	getNextToken(con); // eat end
	return llvm::make_unique<LetExprAST>(std::move(letDec),std::move(expr));
}

/// PrimaryExpr
///   ::= VariableExpr
///   ::= NumberExpr
///   ::= LetExpr
///   ::= ParenExpr
unique_ptr<ExprAST> Parser::ParsePrimaryExpr(Container* con) {
	switch (con->CurTok) {
	default: // unknown token (lack of primary expression)
		return ParserExprError(con, "unknown token when primary expression");
	case tok_identifier:
		return ParseVariableExpr(con);
	case tok_int: case tok_real:
		return ParseNumberExpr(con);
		case tok_let:
			return ParseLetExpr(con);
		case '(':
			return ParseParenExpr(con);
	}
}

///  BinaryExpr ::= (Op PrimaryExpr)*	
unique_ptr<ExprAST> Parser::ParseBinaryExpr(Container* con, int ExprPrec,
	std::unique_ptr<ExprAST> LHS) {
	// If this is a binop, find its precedence.
	while (true) {
		int CurPrec = GetTokPrecedence(con);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if (CurPrec < ExprPrec)
			return move(LHS);

		// Okay, we know this is a binop.
		int BinOp = con->CurTok;
		getNextToken(con); // eat binop

		// Parse the primary expression after the binary operator.
		auto RHS = ParsePrimaryExpr(con);
		if (!RHS)
			// if error or lack of primary expression
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = GetTokPrecedence(con);
		if (CurPrec < NextPrec) {
			RHS = ParseBinaryExpr(con, CurPrec + 1, std::move(RHS));
			if (!RHS)
				// if error
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS =
			llvm::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
	}
}

/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
unique_ptr<ExprAST> Parser::ParseIfExpr(Container* con) {
	getNextToken(con); // eat if
	auto exprIF = ParseExpression(con);
	if (!exprIF)
		// if error or lack of expression
		return nullptr;

	if (con->CurTok != tok_then)
		return ParserExprError(con, "lack of token(then) in IF expression");

	getNextToken(con); // eat then
	auto exprTHEN = ParseExpression(con);
	if (!exprTHEN)
		// if error or lack of expression
		return nullptr;

	if (con->CurTok != tok_else)
		return ParserExprError(con, "lack of token(else) in IF expression");

	getNextToken(con); // eat else
	auto exprElSE = ParseExpression(con);
	if (!exprElSE)
		// if error or lack of expression
		return nullptr;

	return llvm::make_unique<IfExprAST>(move(exprIF), move(exprTHEN), move(exprElSE));
}

/// Expression ::= IfExpr 
///			   ::= PrimaryExpr BinaryExpr
unique_ptr<ExprAST> Parser::ParseExpression(Container* con) {
	switch (con->CurTok) {
	default: // unknown token (lack of expression)
		return ParserExprError(con, "unknown token when expected expression");
	case tok_if:
		return ParseIfExpr(con);
	case tok_identifier: case tok_int: case tok_real:
	case tok_let: case '(': {
		auto LHS = ParsePrimaryExpr(con);
		if (!LHS)
			// if error or lack of lack of primary expression
			return nullptr;
		return ParseBinaryExpr(con, 0, std::move(LHS));
	}
	}
}

/// toplevelexpr ::= expression
unique_ptr<FunctionDecAST> Parser::ParseTopLevelExpr(Container* con) {
	if (auto E = ParseExpression(con)) {
		// Make an anonymous proto.
		auto Proto = llvm::make_unique<PrototypeAST>("__anon_expr",
			vector<string>(), vector<int>(1,REAL), REAL);
		return llvm::make_unique<FunctionDecAST>(std::move(Proto), std::move(E));
	}
	return nullptr;
}


/*-------------------------------------------
				declaration
-------------------------------------------*/

/// Function ::= tok_fun id '(' id ':' type (',' id ':' type)*  ')' : Type '=' Expression
///			 ::= tok_fun id '(' ')' : Type '=' Expression
unique_ptr<FunctionDecAST> Parser::ParseFunction(Container* con) {
	// eat fun
	if (getNextToken(con) != tok_identifier) // lack of identifier(function name)
		return ParserFuncDecError(con, "lack of function name in function declaration");

	string funcName = con->IdentifierStr;
	if (getNextToken(con) != '(') // eat identifier
		return ParserFuncDecError(con, "lack of function tok('(') in function declaration");

	vector<string> argsName;
	vector<int> argsType;
	
	if (getNextToken(con) != ')') { // eat (
		// if not no args, id ':' type
		if (con->CurTok != tok_identifier)
			return ParserFuncDecError(con, "lack of args name in function declaration");
		argsName.push_back(con->IdentifierStr);
		if (getNextToken(con) != ':') // eat identifier
			return ParserFuncDecError(con, "lack of token(:) to difine type function declaration");
		getNextToken(con); // eat :
		int prType = ParseType(con);
		if (prType < 0)
			return nullptr;
		argsType.push_back(prType);

		while (con->CurTok == ',') {
			if (getNextToken(con) != tok_identifier) // eat ,
				return ParserFuncDecError(con, "lack of args name in function declaration");
			argsName.push_back(con->IdentifierStr);
			if (getNextToken(con) != ':') // eat identifier
				return ParserFuncDecError(con, "lack of token(:) to difine type function declaration");
			getNextToken(con); // eat :
			int tmpType = ParseType(con);
			if (tmpType < 0)
				return nullptr;
			argsType.push_back(tmpType);
		}

		if (con->CurTok != ')')
			return ParserFuncDecError(con, "lack of function tok(')') in function declaration");
	}

	if (getNextToken(con) != ':') // eat )
		return ParserFuncDecError(con, "lack of token(:) to difine ret function declaration");
	getNextToken(con); // eat :
	int retType = ParseType(con);
	if (retType < 0)
		return nullptr;

	auto proto = llvm::make_unique<PrototypeAST>(funcName, move(argsName), move(argsType), retType);
	
	if (con->CurTok != '=')
		return ParserFuncDecError(con, "lack of token(=) in function declaration");

	getNextToken(con); // eat =
	auto FuncBody = ParseExpression(con);
	if (!FuncBody)
		// if error or lack of expression
		return nullptr;

	return llvm::make_unique<FunctionDecAST>(move(proto), move(FuncBody));
}

/// Value ::= tok_val id ':' type '=' Expression
unique_ptr<ValueDecAST> Parser::ParseValue(Container* con) { 
	if (getNextToken(con) != tok_identifier) // eat val
		// if error or lack of pattern
		return ParserValDecError(con,"lack of identifier in val declaration");

	string valName = con->IdentifierStr;

	if (getNextToken(con) != ':') // eat id
		return ParserValDecError(con, "lack of token(:) to difine type val declaration");

	getNextToken(con); // :
	int valType = ParseType(con);
	if (valType < 0)
		// error or lack of type
		return nullptr;

	if(con->CurTok != '=')
		return ParserValDecError(con, "lack of token(=) in function declaration");

	getNextToken(con); // eat = 
	auto valExpr = ParseExpression(con);
	if (!valExpr)
		// if error or lack of expression
		return nullptr;

	return llvm::make_unique<ValueDecAST>(valName, valType, move(valExpr));
}

/*-------------------------------------------
				  type
-------------------------------------------*/

/// Type ::= int | real 
int Parser::ParseType(Container* con) {
	if (con->CurTok != tok_identifier)
		return ParserTypeError(con, "lack of type after type define symol(:)");
	string type = con->IdentifierStr;
	
	if (type == "int")
		return INT;
	else if (type == "real")
		return REAL;
	else
		return ParserTypeError(con, "unknown type");
}


/*-------------------------------------------
			   Error Handle
-------------------------------------------*/

int Parser::ParserTypeError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "Parser Error: %s\n", info);
	return -1;
}

unique_ptr<ExprAST> Parser::ParserExprError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "Parser Error: %s\n", info);
	return nullptr;
}

unique_ptr<FunctionDecAST> Parser::ParserFuncDecError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "Parser Error: %s\n", info);
	return nullptr;
}

unique_ptr<ValueDecAST> Parser::ParserValDecError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "Parser Error: %s\n", info);
	return nullptr;
}

/*-------------------------------------------
	  Top-Level parsing and JIT Driver
-------------------------------------------*/

void Parser::InitializeModuleAndPassManager(Container* con) {
	// Open a new module.
	con->TheModule = std::make_unique<Module>("Standard ML", con->TheContext);
	con->TheModule->setDataLayout(con->TheJIT->getTargetMachine().createDataLayout());

	// Create a new pass manager attached to it.
	con->TheFPM = std::make_unique<legacy::FunctionPassManager>(con->TheModule.get());

	// Promote allocas to registers.
	con->TheFPM->add(createPromoteMemoryToRegisterPass());
	// Do simple "peephole" optimizations and bit-twiddling optzns.
	con->TheFPM->add(createInstructionCombiningPass());
	// Reassociate expressions.
	con->TheFPM->add(createReassociatePass());
	// Eliminate Common SubExpressions.
	con->TheFPM->add(createGVNPass());
	// Simplify the control flow graph (deleting unreachable blocks, etc).
	con->TheFPM->add(createCFGSimplificationPass());

	con->TheFPM->doInitialization();
}

void Parser::HandleFunDeclaration(Container* con) {
	if (auto FunAST = ParseFunction(con)) {
		Type* reType;
		if (auto* FnIR = FunAST->codegen(con, &reType)) {
			fprintf(stderr, "> Read function definition:");
			FnIR->print(errs());
			fprintf(stderr, "\n");
			con->TheJIT->addModule(std::move(con->TheModule));
			InitializeModuleAndPassManager(con);
		}
		else
			getNextToken(con);
	}
	else
		// Skip token for error recovery.
		getNextToken(con);
}

void Parser::HandleValDeclaration(Container* con) {
	if (auto ValAST = ParseValue(con)) {
		if (auto* ValIR = ValAST->codegen(con)) {
			fprintf(stderr, "> val %s = ", ValAST->getName().c_str());
			ValIR->print(errs());
			fprintf(stderr, "\n");
		}
		else
			getNextToken(con);
	}
	else
		getNextToken(con);
}

void Parser::HandleTopLevelExpression(Container* con) {
	Type* reType = nullptr;
	// Evaluate a top-level expression into an anonymous function.
	if (auto ExprAST = ParseTopLevelExpr(con)) {
		if (ExprAST->codegen(con, &reType)) {
			// JIT the module containing the anonymous expression, keeping a handle so
			// we can free it later.
			auto H = con->TheJIT->addModule(std::move(con->TheModule));
			InitializeModuleAndPassManager(con);

			// Search the JIT for the __anon_expr symbol.
			auto ExprSymbol = con->TheJIT->findSymbol("__anon_expr");
			assert(ExprSymbol && "Function not found");

			if (reType == Type::getDoubleTy(con->TheContext)) {
				// Get the symbol's address and cast it to the right type (takes no
				// arguments, returns a double) so we can call it as a native function.
				double (*FP)() =
					(double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());

				Value* FTry = ConstantFP::get(con->TheContext, APFloat(FP()));
				con->GlobalValues["it"] = FTry;

				fprintf(stderr, "> val it = %f : real\n", FP());
			}
			else {
				int (*IP)() = (int (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
				Value* ITry =
					ConstantInt::get(Type::getInt32Ty(con->TheContext), (int32_t)IP(), true);
				con->GlobalValues["it"] = ITry;
				fprintf(stderr, " > val it = %d : int\n", IP());
			}

			// Delete the anonymous expression module from the JIT.
			con->TheJIT->removeModule(H);
		}
		else
			getNextToken(con);
	}
	else
		// Skip token for error recovery.
		getNextToken(con);
}

void Parser::MainLoop(Container* con) {
	while (true) {
		switch (con->CurTok) {
		case tok_exit:
			return;
		case ';': // ignore top-level semicolons.
			getNextToken(con);
			break;
		case tok_fun:
			HandleFunDeclaration(con);
			fprintf(stderr, " - ");
			break;
		case tok_val:
			HandleValDeclaration(con);
			fprintf(stderr, " - ");
			break;
		default:
			HandleTopLevelExpression(con);
			fprintf(stderr, " - ");
			break;
		}
	}
}