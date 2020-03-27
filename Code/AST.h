#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Token.h"
#include "Container.h"

using namespace std;
using namespace llvm;

namespace {

	class ValueDecAST;

	/*-------------------------------------------
		Abstract Syntax Tree (aka Parse Tree)
	-------------------------------------------*/

	/*----------------father class----------------*/
	
	/////TypeAST - type of single pattern or single expression
	/////		   - type of multiple pattern or paren expression			
	//class TypeAST {
	//protected:
	//	string TypeName;
	//public:
	//  vector<TypeAST *> Contents;
	//	TypeAST(const string& TypeName)
	//		: TypeName(TypeName) {}
	//	bool FitType(const TypeAST* Type);
	//	const string& getTypeName() { return TypeName; }
	//};

	///// PattAST - Base class for all pattern nodes
	//class PattAST {
	//	string PattName;
	//public:
	//	int type;
	//	bool isSingle;
	//	vector<unique_ptr<PattAST>> Contents;
	//	PattAST(const string& PattName, bool isSingle, int type)
	//		: PattName(PattName), isSingle(isSingle), type(type) {}
	//	virtual ~PattAST() = default;
	//	const string& getPattName() { return PattName; }
	//};

	///// DecAST - Base class for all declaration nodes
	//class DecAST {
	//public:
	//	virtual ~DecAST() = default;
	//};

	/// ExprAST - Base class for all expression nodes.
	class ExprAST {
	public:
		virtual ~ExprAST() = default;
		virtual Value* codegen(Container* con) = 0;
	};

	/*-------------------------------------------
					expression
	-------------------------------------------*/

	/*----------------Constant Expression----------------*/

	///// only Constant can define type in Parser
	//class BoolExprAST : public ExprAST {
	//	bool BoolVal;
	//public:
	//	BoolExprAST(bool BoolVal) : BoolVal(BoolVal) {}
	//	Value* codegen(Container* con) override;
	//};

	class NumberExprAST : public ExprAST {
		double NumVal;
		int type;
	public:
		NumberExprAST(double NumVal, int type) : NumVal(NumVal), type(type) {}
		Value* codegen(Container* con) override;
	};


	/*----------------variable, Paren, Call and Let----------------*/
	/// VariableExprAST - name of value 
	class VariableExprAST : public ExprAST {
		string VariName;
	public:
		VariableExprAST(const string& VariName) : VariName(VariName) {}
		Value* codegen(Container* con) override;
	};


	/// CallExprAST - Expression class for function calls.
	class CallExprAST : public ExprAST {
		string Callee;
		vector<unique_ptr<ExprAST>> Args;
	public:
		CallExprAST(const string& Callee, vector<unique_ptr<ExprAST>> Args)
			: Callee(Callee), Args(move(Args)) {}
		Value* codegen(Container* con) override;
	};

	/// LetExprAST - let dec in expr end
	class LetExprAST : public ExprAST {
		vector<unique_ptr<ValueDecAST>> LetDec;
		unique_ptr<ExprAST> InExpr;
	public:
		LetExprAST(vector<unique_ptr<ValueDecAST>> LetDec,
			unique_ptr<ExprAST> InExpr)
			: LetDec(move(LetDec)), InExpr(move(InExpr)) {}
		Value* codegen(Container* con) override;
	};

	/*----------------Binary and If Expression----------------*/
	/// BinaryExprAST - Expression class for a binary operator.
	///TODO : include default binary(infix) operators and user-defined binary operators
	class BinaryExprAST : public ExprAST {
		char Op;
		unique_ptr<ExprAST> LHS, RHS;
	public:
		BinaryExprAST(char Op, unique_ptr<ExprAST> LHS,
			unique_ptr<ExprAST> RHS)
			: Op(Op), LHS(move(LHS)), RHS(move(RHS)) {}
		Value* codegen(Container* con) override;
	};

	/// IfExprAST - if expr then expr else expr
	class IfExprAST : public ExprAST {
		unique_ptr<ExprAST> IfExpr, ThenExpr, ElseExpr;
	public:
		IfExprAST(unique_ptr<ExprAST> IfExpr, unique_ptr<ExprAST> ThenExpr,
			unique_ptr<ExprAST> ElseExpr)
			: IfExpr(move(IfExpr)), ThenExpr(move(ThenExpr)), ElseExpr(move(ElseExpr)) {}
		Value* codegen(Container* con) override;
	};

	/*-------------------------------------------
                   declaration
    -------------------------------------------*/

        /// ValueDecAST - name a expression value
        class ValueDecAST {
          string ValName;
          int ValType;
          unique_ptr<ExprAST> ValExpr;

        public:
          ValueDecAST(const string &ValName, int ValType,
                      unique_ptr<ExprAST> ValExpr)
              : ValName(ValName), ValType(ValType), ValExpr(move(ValExpr)) {}
          Value *codegen(Container *con);
          const string &getName() const { return ValName; }
          int getType() { return ValType; }
          ExprAST *getExpr() { return ValExpr.get(); }
        };

        /*----------------function----------------*/
        // PrototypeAST - This class represents the "prototype" for a function,
        /// which captures its name, and its argument names (thus implicitly the
        /// number of arguments the function takes).
        class PrototypeAST {
          string FuncName;
          vector<string> ArgsName;
          vector<int> ArgsType;
          int RetType;

        public:
          PrototypeAST(const std::string &FuncName, vector<string> ArgsName,
                       vector<int> ArgsType, int RetType)
              : FuncName(FuncName), ArgsName(move(ArgsName)),
                ArgsType(move(ArgsType)), RetType(RetType) {}
          Function *codegen(Container *con);
          const std::string &getName() const { return FuncName; }
        };

        /// FunctionDecAST - This class represents a function definition itself.
        class FunctionDecAST {
          unique_ptr<PrototypeAST> Proto;
          unique_ptr<ExprAST> FuncBody;

        public:
          FunctionDecAST(unique_ptr<PrototypeAST> Proto,
                         unique_ptr<ExprAST> FuncBody)
              : Proto(move(Proto)), FuncBody(move(FuncBody)) {}
          Function *codegen(Container *con, Type **TypeToReturn);
        };
	
}


