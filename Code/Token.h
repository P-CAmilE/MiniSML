#pragma once

// The lexer returns tokens[0 - 255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {

	//exit
	tok_exit = -1,
	//error
	tok_error = -2,

	// identify
	tok_identifier = -4,

	// datatype of constant
	tok_bool = -10,
	tok_int = -11,
	tok_real = -12,
	tok_char = -13,
	tok_string = -14,

	// keywords
	tok_val = -15,
	tok_fn = -16,
	tok_fun = -17,
	tok_functor = -55,
	tok_type = -18,
	tok_datatype = -19,
	tok_abstype = -20,
	tok_eqtype = -21,
	tok_withtype = -22,
	tok_handle = -23,
	tok_infix = -24,
	tok_nonfix = -25,
	tok_infixr = -26,
	tok_local = -27,
	tok_sharing = -28,
	tok_and = -29,
	tok_as = -30,
	tok_with = -31,
	tok_then = -32,
	tok_where = -33,
	tok_include = -34,
	tok_of = -35,
	tok_open = -36,
	tok_op = -37,
	tok_raise = -38,
	tok_rec = -39,
	tok_exception = -40,
	tok_andalso = -41,
	tok_orelse = -42,
	tok_do = -43,
	tok_while = -44,
	tok_if = -45,
	tok_else = -46,
	tok_case = -47,
	tok_let = -48,
	tok_in = -49,
	tok_end = -50,
	tok_sig = -51,
	tok_signature = -52,
	tok_struct = -53,
	tok_structure = -54,
	tok_div = -55,
	tok_mod = -57,

};

enum type {
	BOOL,
	INT,
	REAL,
	CHAR,
	STRING,
	UNIT,
	TUPLE,
};