/*---------------------------------------------------
	this file contains the common values and objects

	NOTE:
		1.default datatype: 
			tuple(include unit) record bool int real char string
		2.default Binary operators: 
			bool: andalso orelse = <> (not)
			int: + - * div mod < > = <> >= <=
			real: + - * / < > = <> >= <=
			char: = <> <= >=
			string: ^ = <> <= >=
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

using namespace llvm;
using namespace llvm::orc;
using namespace std;

class Container {

public:

	int LastChar = ' ';
	int CurTok = 0;
	bool BoolVal = 1;
	double NumVal = 0;
	string IdentifierStr = "";
	char CharVal = 0x00;
	string StrVal = "";

	const int getCurTok() { return this->CurTok; }
	const string& getIdentifierStr() { return this->IdentifierStr; }
	const char getCharVal() { return this->CharVal; }
	const string& getStrVal() { return this->StrVal; }
	const double getNumVal() { return this->NumVal; }
	const bool getBoolVal() { return this->BoolVal; }

	void resetLastChar() { this->LastChar = ' '; }

	// define all default binary operator in SML
	std::map<char, int> BinopPrecedence{
		{ tok_andalso,10 }, { tok_orelse,10 }, { '<',20 }, { '>',20 }, { '<>',20 }, { '<=',20 },
		{ '>=',20 }, { '=', 20 }, { '+',30 }, { '-',30 }, { '^',30 }, { '*',40 }, { tok_div,40 }, { '/',40 }
	};

	std::map<std::string, Value*> GlobalValues;

	LLVMContext& TheContext;
	IRBuilder<>& Builder;
	std::unique_ptr<Module> TheModule;
	std::map<std::string, AllocaInst*> NamedValues;
	std::unique_ptr<legacy::FunctionPassManager> TheFPM;
	std::unique_ptr<SMLJIT> TheJIT;

	Container(LLVMContext& TheContext, IRBuilder<>& Builder)
		: TheContext(TheContext), Builder(Builder) {
		TheJIT = std::make_unique<SMLJIT>();
	}

};