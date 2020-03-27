/*---------------------------------------------------
	this file implements the Lexer
---------------------------------------------------*/

#include "Lexer.h"

int Lexer::LexerError(Container* con, const char* info) {
	// clean buffer of keyboard
	int c;
	while ((c = getchar()) != '\n' && c != EOF);
	con->resetLastChar();
	fprintf(stdout, "Lexer Error: %s\n", info);
	return tok_error;
}

//lex symbal
bool Lexer::issymble(int LastChar) {
	return LastChar == '!' || LastChar == '%' || LastChar == '&' ||
		LastChar == '$' || LastChar == '#' || LastChar == '+' ||
		LastChar == '-' || LastChar == '*' || LastChar == '/' ||
		LastChar == ':' || LastChar == '<' || LastChar == '=' ||
		LastChar == '>' || LastChar == '?' || LastChar == '@' ||
		LastChar == '\\' || LastChar == '~' || LastChar == '`' ||
		LastChar == '^' || LastChar == '|' || LastChar == '.';
}

//lex number when meet 'E'
int Lexer::readE(Container* con, string* NumStr) {
	*NumStr += "E";
	con->LastChar = getchar();
	if (isdigit(con->LastChar)) {
		while (isdigit(con->LastChar)) {
			*NumStr += con->LastChar;
			con->LastChar = getchar();
		}
		con->NumVal = strtod((*NumStr).c_str(), nullptr);
		return tok_real;
	}
	else if (con->LastChar == '~') {
		*NumStr += "-";
		con->LastChar = getchar();
		if (isdigit(con->LastChar)) {
			while (isdigit(con->LastChar)) {
				*NumStr += con->LastChar;
				con->LastChar = getchar();
			}
			con->NumVal = strtod((*NumStr).c_str(), nullptr);
			return tok_real;
		}
		else
			return LexerError(con, "illegal expression of negative '~'");// ������E��ֻ��~û������
	}
	else
		return LexerError(con,"illegal expression of scientific notation 'E'");// ������E��Ϊ��
}

//lex number when meet '.'
int Lexer::readPoint(Container* con, string* NumStr) {
	*NumStr += ".";
	con->LastChar = getchar();
	while (isdigit(con->LastChar)) {
		NumStr += con->LastChar;
		con->LastChar = getchar();
	}
	if (con->LastChar == '.')
		return LexerError(con,"illegal expression of decimal '.'");// ���������ֶ����С����
	else if (con->LastChar == 'E')
		return readE(con, NumStr);
	else {
		con->NumVal = strtod((*NumStr).c_str(), nullptr);
		return tok_real;
	}
}

// lex number(int and real)
int Lexer::readNum(Container* con, bool isNegative) {
	string NumStr = "";
	if (isNegative)
		NumStr += "-";
	while (isdigit(con->LastChar)) {
		NumStr += con->LastChar;
		con->LastChar = getchar();
	}
	if (con->LastChar == '.')
		return readPoint(con, &NumStr);
	else if (con->LastChar == 'E')
		return readE(con, &NumStr);
	else {
		con->NumVal = strtod(NumStr.c_str(), nullptr);
		return tok_int;
	}
}

// lex escape sequence of character and string
// NOTE: ת�����а���: '\n' '\t' '\v' '\b' '\r' '\f' '\a' 
//                    '\\' '\"' '\ �ո��Ʊ��� ���� ��ҳ \'
int Lexer::readEscapeSequence(Container* con, bool isStr) {
	con->LastChar = getchar();
	if (con->LastChar == '\"' || con->LastChar == '\\')
		if (isStr)
			con->StrVal += con->LastChar;
		else
			con->CharVal = con->LastChar;
	else if (con->LastChar == 'n')
		if (isStr)
			con->StrVal += '\n';
		else
			con->CharVal = '\n';
	else if (con->LastChar == 't')
		if (isStr)
			con->StrVal += '\t';
		else
			con->CharVal = '\t';
	else if (con->LastChar == 'v')
		if (isStr)
			con->StrVal += '\v';
		else
			con->CharVal = '\v';
	else if (con->LastChar == 'b')
		if (isStr)
			con->StrVal += '\b';
		else
			con->CharVal = '\b';
	else if (con->LastChar == 'r')
		if (isStr)
			con->StrVal += '\r';
		else
			con->CharVal = '\r';
	else if (con->LastChar == 'f')
		if (isStr)
			con->StrVal += '\f';
		else
			con->CharVal = '\f';
	else if (con->LastChar == 'a')
		if (isStr)
			con->StrVal += '\a';
		else
			con->CharVal = '\a';
	else if (con->LastChar == '\n' || con->LastChar == '\t' || con->LastChar == ' ' || con->LastChar == '\f') {
		con->LastChar = getchar();
		while (con->LastChar == '\n' || con->LastChar == '\t' || con->LastChar == ' ' || con->LastChar == '\f')
			con->LastChar = getchar();
		if (con->LastChar != '\\')
			return LexerError(con,"illegal expression of character");
		return 0;
	}
	else
		return LexerError(con,"illegal expression of character");
	return 1;
}

// lex string or char
int Lexer::readStr(Container* con, bool isStr) {
	con->LastChar = getchar();
	if (isStr) {
		con->StrVal = "";
		while (con->LastChar != '\"') {
			if (con->LastChar == '\n' || con->LastChar == '\t')
				return LexerError(con,"unclosed string");
			else if (con->LastChar == '\\') {
				if (readEscapeSequence(con, 1) == tok_error)
					return tok_error;
				con->LastChar = getchar();
				continue;
			}
			con->StrVal += con->LastChar;
			con->LastChar = getchar();
		}
		// ���� " 
		con->LastChar = getchar();
		return tok_string;
	}
	else {
		bool isSetChar = false;
		if (con->LastChar == '\"')
			return LexerError(con,"character cannot be empty"); //�������֡�������Ϊ��
		while (con->LastChar == '\\') {
			int i = readEscapeSequence(con, 0);
			if (i == tok_error)
				return tok_error;
			else if (i == 1 && !isSetChar)
				isSetChar = true;
			else if (i == 1 && isSetChar)
				return LexerError(con,"length of character not 1");
			con->LastChar = getchar();
		}
		if (!isSetChar) {
			if (con->LastChar == '\n' || con->LastChar == '\t')
				return LexerError(con,"unclosed character");
			else if (con->LastChar == '\"')
				return LexerError(con,"character cannot be empty");
			con->CharVal = con->LastChar;
			con->LastChar = getchar();
		}
		else
			while (con->LastChar != '\\') {
				int i = readEscapeSequence(con, 0);
				if (i == tok_error)
					return tok_error;
				else if (i == 1)
					return LexerError(con,"length of character not 1");
				con->LastChar = getchar();
			}
		if (con->LastChar == '\"') {
			con->LastChar = getchar();
			return tok_char;
		}
		else
			return LexerError(con,"length of character not 1"); //�ַ����ȳ���1 or �ַ�δ�պ�
	}
}

/// gettok - Return the next token from standard input.
int Lexer::gettok(Container* con) {

	// �����ո�tab��������
	while (isspace(con->LastChar))
		con->LastChar = getchar();

	//��ĸ���֡��ؼ���
	if (isalpha(con->LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9|_|']*
		con->IdentifierStr = con->LastChar;
		while (isalnum((con->LastChar = getchar())) || con->LastChar == '_' ||
			con->LastChar == '\'')
			con->IdentifierStr += con->LastChar;
		//�ų�������
		if (con->IdentifierStr == "val")
			return tok_val;
		if (con->IdentifierStr == "abstype")
			return tok_abstype;
		if (con->IdentifierStr == "and")
			return tok_and;
		if (con->IdentifierStr == "andalso")
			return tok_andalso;
		if (con->IdentifierStr == "orelse")
			return tok_orelse;
		if (con->IdentifierStr == "as")
			return tok_as;
		if (con->IdentifierStr == "case")
			return tok_case;
		if (con->IdentifierStr == "datatype")
			return tok_datatype;
		if (con->IdentifierStr == "do")
			return tok_do;
		if (con->IdentifierStr == "else")
			return tok_else;
		if (con->IdentifierStr == "end")
			return tok_end;
		if (con->IdentifierStr == "eqtype")
			return tok_eqtype;
		if (con->IdentifierStr == "exception")
			return tok_exception;
		if (con->IdentifierStr == "fn")
			return tok_fn;
		if (con->IdentifierStr == "fun")
			return tok_fun;
		if (con->IdentifierStr == "functor")
			return tok_functor;
		if (con->IdentifierStr == "handle")
			return tok_handle;
		if (con->IdentifierStr == "if")
			return tok_if;
		if (con->IdentifierStr == "in")
			return tok_in;
		if (con->IdentifierStr == "include")
			return tok_include;
		if (con->IdentifierStr == "infix")
			return tok_infix;
		if (con->IdentifierStr == "infixer")
			return tok_infixr;
		if (con->IdentifierStr == "let")
			return tok_let;
		if (con->IdentifierStr == "local")
			return tok_local;
		if (con->IdentifierStr == "nonfix")
			return tok_nonfix;
		if (con->IdentifierStr == "of")
			return tok_of;
		if (con->IdentifierStr == "op")
			return tok_op;
		if (con->IdentifierStr == "open")
			return tok_open;
		if (con->IdentifierStr == "raise")
			return tok_raise;
		if (con->IdentifierStr == "rec")
			return tok_rec;
		if (con->IdentifierStr == "sharing")
			return tok_sharing;
		if (con->IdentifierStr == "sig")
			return tok_sig;
		if (con->IdentifierStr == "signature")
			return tok_signature;
		if (con->IdentifierStr == "struct")
			return tok_struct;
		if (con->IdentifierStr == "structure")
			return tok_structure;
		if (con->IdentifierStr == "then")
			return tok_then;
		if (con->IdentifierStr == "type")
			return tok_type;
		if (con->IdentifierStr == "where")
			return tok_where;
		if (con->IdentifierStr == "while")
			return tok_while;
		if (con->IdentifierStr == "with")
			return tok_with;
		if (con->IdentifierStr == "withtype")
			return tok_withtype;
		if (con->IdentifierStr == "div")
			return tok_div;
		if (con->IdentifierStr == "mod")
			return tok_mod;

		//ʶ������true��false������tok_bool
		if (con->IdentifierStr == "false") {
			con->BoolVal = false;
			return tok_bool;
		}
		if (con->IdentifierStr == "true") {
			con->BoolVal = true;
			return tok_bool;
		}

		return tok_identifier;
	}

	//�������֡���������
	if (issymble(con->LastChar)) { // identifier: [!|%|&|$|#|+|-|*|/|:|<|=|>|?|@|\|~|`|^||]*
							   // - [:|||=|=>|->|#|:>|+|-|*|/|>|<|<=|>=|<>|^]
		con->IdentifierStr = con->LastChar;
		while (issymble((con->LastChar = getchar())))
			con->IdentifierStr += con->LastChar;

		if (con->IdentifierStr == ":")
			return ':';
		if (con->IdentifierStr == "|")
			return '|';
		if (con->IdentifierStr == "=")
			return '=';
		if (con->IdentifierStr == "=>")
			return '=>';
		if (con->IdentifierStr == "->")
			return '->';
		if (con->IdentifierStr == ":>")
			return ':>';
		if (con->IdentifierStr == "+")
			return '+';
		if (con->IdentifierStr == "-")
			return '-';
		if (con->IdentifierStr == "*")
			return '*';
		if (con->IdentifierStr == "/")
			return '/';
		if (con->IdentifierStr == ">")
			return '>';
		if (con->IdentifierStr == "<")
			return '<';
		if (con->IdentifierStr == "<=")
			return '<=';
		if (con->IdentifierStr == ">=")
			return '>=';
		if (con->IdentifierStr == "<>")
			return '<>';
		if (con->IdentifierStr == "^")
			return '^';

		//�����ַ����� # ��ʶ��
		if (con->IdentifierStr == "#") {
			if (con->LastChar == '"')
				return readStr(con, false);
			else
				return '#';
			//~���ź��棺��"------ʶ��Ϊ�ַ���������------ʶ��Ϊ������š�
		}

		//���ָ����� ~ ��ʶ��
		if (con->IdentifierStr == "~") {
			if (isdigit(con->LastChar))
				return readNum(con, true);
			else return tok_identifier;
			//~���ź��棺������------ʶ��Ϊ���ţ�����------ʶ��Ϊ��ʶ����
		}

		return tok_identifier;
	}

	//�ַ���
	if (con->LastChar == '"')
		readStr(con, true);

	//����
	if (isdigit(con->LastChar)) // Number: [0-9.]+
		return  readNum(con, false);

	//ע�� ���﷨��������
	//ע�⣡��ע����Ҫʶ�������ַ� �ʷ�getchar()û�취���� ֻ�ܰ�(��*�ֱ�ʶ�𲢷���ascii��
	//(**)��(aBC+b)
	if (con->LastChar == '(') {
		if ((con->LastChar = getchar()) != '*')
			return '(';
		else
			while (1) {
				con->LastChar = getchar();
				if (con->LastChar == '*')
					if ((con->LastChar = getchar()) == ')')//����ע��ֱ������
						return gettok(con);
					else
						continue;
				else if (con->LastChar == '\n')
					return LexerError(con,"unclosed annotation"); //������ע��δ�պ�
			}
	}

	//��������
	if (con->LastChar == EOF)
		return tok_exit;

	// �ֶη� ; 
	if (con->LastChar == ';' || con->LastChar == '('
		|| con->LastChar == ')' || con->LastChar == ',') {
		int thisChar = con->LastChar;
		con->LastChar = getchar();
		return thisChar;
	}

	// ���ֲ��Ϸ���asciiֵ
	return LexerError(con,"illegal symbol");
}
