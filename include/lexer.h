#pragma once

#include "error.h"
#include "builtin.h"

typedef struct Lexer Lexer;

typedef enum {
	// 0-255 reserved for ASCII
	TOKEN_EOF = 256,
	TOKEN_NONE,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_RETURN,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_FUNC,
	TOKEN_LOOP,
	TOKEN_WHILE,
	TOKEN_BREAK,
	TOKEN_CONTINUE,

	TOKEN_AS,
	TOKEN_INT,
	TOKEN_FLOAT,
	TOKEN_BOOL,
	TOKEN_STRING,
	TOKEN_LIST,
	
	TOKEN_OR,
	TOKEN_AND,
	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_GTE,
	TOKEN_LTE,
	
	TOKEN_NAME,
	TOKEN_BUILTIN,
	TOKEN_INTEGER_LITERAL,
	TOKEN_FLOAT_LITERAL,
	TOKEN_STRING_LITERAL,
} Token_Type;

extern const char * token_type_names[];

typedef struct {
	Token_Type type;
	union {
		const char * name;
		Builtin builtin;
		int integer_literal;
		float float_literal;
		const char * string_literal;
	};
	Assoc_Source assoc;
} Token;

char * token_type_to_string(Token_Type type);
char * token_to_string(Token token);

struct Lexer {
	const char * source;
	size_t source_len;
	size_t position;
	size_t line;
	size_t column;
	Token token;
	const char ** interned_strings;
};

Lexer * lexer_alloc(const char * source);
Token lexer_next_token(Lexer * lexer);
void lexer_advance(Lexer * lexer);
Token lexer_lookahead(Lexer * lexer, size_t lookahead);
const char * lexer_intern_range(Lexer * lexer, size_t start, size_t end);
const char * lexer_intern_string(Lexer * lexer, const char * str);
