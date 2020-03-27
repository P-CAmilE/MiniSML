/*---------------------------------------------------
	the head file of Lexer
---------------------------------------------------*/

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "Token.h"
#include "Container.h"

using namespace std;

class Lexer {

	//lex number when meet 'E'
	int readE(Container* con, string* NumStr);
	//lex number when meet '.'
	int readPoint(Container* con, string* NumStr);
	// lex number(int and real)
	int readNum(Container* con, bool isnegative);
	//lex symbal
	bool issymble(int LastChar);
	// lex string or char
	int readStr(Container* con, bool isStr);
	// lex escape sequence of character and string
	int readEscapeSequence(Container* con, bool isStr);

	// error handle
	int LexerError(Container* con, const char* info);
public:

	/// gettok - Return the next token from standard input.
	int gettok(Container* con);
	
};