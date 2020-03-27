/*---------------------------------------------------
	the head file of Parser	
---------------------------------------------------*/

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
#include "SMLJIT.h"
#include "Container.h"
#include "Lexer.h"
#include "AST.h"

using namespace std;
using namespace llvm;

class Parser {

	unique_ptr<Lexer> lexer;

	int GetTokPrecedence(Container* con);

	/*-------------------------------------------
					expression
	-------------------------------------------*/

	// parse int and real value
	unique_ptr<ExprAST> ParseNumberExpr(Container* con);

	/*------------Variable and function call------------*/
	/// VariableExpr ::= Variable
	///	CallExpr	 ::= Variable '(' ')'
	///				 ::= Variable '(' Expression (',' Expression)* ')'
	unique_ptr<ExprAST> ParseVariableExpr(Container* con);

	/*------------Paren and Let Expression-------------*/
	/// ParenExpr ::=  '(' Expression ')'
	unique_ptr<ExprAST> ParseParenExpr(Container* con);

	/// LetExpr ::= tok_let Value (',' value)* tok_in Expression tok_end
	unique_ptr<ExprAST> ParseLetExpr(Container* con);

	/// PrimaryExpr
	///   ::= VariableExpr
	///   ::= NumberExpr
	///   ::= LetExpr
	///   ::= ParenExpr
	unique_ptr<ExprAST> ParsePrimaryExpr(Container* con);

	/// BinaryExpr ::= (Op PrimaryExpr)*	
	unique_ptr<ExprAST> ParseBinaryExpr(Container* con, int ExprPrec,
		std::unique_ptr<ExprAST> LHS);

	/// IfExpr ::= tok_if Expression tok_then Expression tok_else Expression
	unique_ptr<ExprAST> ParseIfExpr(Container* con);

	/*--------------Expression(top level)--------------*/
	/// Expression ::= IfExpr 
	///			   ::= PrimaryExpr (BinaryExpr)* 
	unique_ptr<ExprAST> ParseExpression(Container* con);

	/// toplevelexpr ::= expression
	unique_ptr<FunctionDecAST> ParseTopLevelExpr(Container* con);

	/*-------------------------------------------
					declaration
	-------------------------------------------*/

	/// Function ::= tok_fun id '(' id ':' type (',' id ':' type)*  ')' : Type '=' Expression
	///			 ::= tok_fun id '(' ')' : type '=' Expression
	unique_ptr<FunctionDecAST> ParseFunction(Container* con);

	/// Value ::= tok_val id ':' type '=' Expression
	unique_ptr<ValueDecAST> ParseValue(Container* con);

	/*-------------------------------------------
					  type
	-------------------------------------------*/

	/// Type ::= int | real 
	int ParseType(Container* con);

	/*-------------------------------------------
					  Error Handle
	-------------------------------------------*/

	int ParserTypeError(Container* con, const char* info);

	unique_ptr<ExprAST> ParserExprError(Container* con, const char* info);

	unique_ptr<FunctionDecAST> ParserFuncDecError(Container* con, const char* info);

	unique_ptr<ValueDecAST> ParserValDecError(Container* con, const char* info);

	/*-------------------------------------------
		  Top-Level parsing and JIT Driver
	-------------------------------------------*/

	void HandleFunDeclaration(Container* con);

	void HandleValDeclaration(Container* con);

	void HandleTopLevelExpression(Container* con);

public:
	Parser() { lexer = std::make_unique<Lexer>(); }

	int getNextToken(Container* con);

	// init module at begin and end of every loop
	void InitializeModuleAndPassManager(Container* con);

	void MainLoop(Container* con);
};
