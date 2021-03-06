/*---------------------------------------------------
	the entrcance of the program StandardML
	the main fucntion lays below
---------------------------------------------------*/

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
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
#include "Container.h"
#include "AST.h"
#include "Lexer.h"
#include "Parser.h"

int main() {
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();

	LLVMContext TheContext;
	IRBuilder<> Builder(TheContext);
	Parser parser;
	Container con(TheContext, Builder);

	fprintf(stderr, " - StandardML JIT Conpiler\n - ");
	parser.getNextToken(&con);
	parser.InitializeModuleAndPassManager(&con);
	parser.MainLoop(&con);

	return 0;
}