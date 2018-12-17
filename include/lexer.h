#pragma once

typedef enum {
	// 0-255 reserved for ASCII
	TOKEN_EOF = 256,
	TOKEN_NONE,
	TOKEN_TRUE,
	TOKEN_FALSE,
	TOKEN_PRINT,
	TOKEN_RETURN,
	TOKEN_IF,
	TOKEN_ELSE,
	TOKEN_FUNC,

	TOKEN_EQ,
	TOKEN_NE,
	TOKEN_GTE,
	TOKEN_LTE,
	
	TOKEN_NAME,
	TOKEN_INTEGER_LITERAL,
	TOKEN_FLOAT_LITERAL,
} Token_Type;

extern const char * token_type_names[];

typedef struct {
	Token_Type type;
	union {
		const char * name;
		int integer_literal;
		float float_literal;
	};
} Token;

char * token_to_string(Token token);

typedef struct {
	const char * source;
	size_t source_len;
	size_t position;
	Token token;
	const char ** interned_strings;
} Lexer;

Lexer * lexer_alloc(const char * source);
Token lexer_next_token(Lexer * lexer);
void lexer_advance(Lexer * lexer);
Token lexer_lookahead(Lexer * lexer, size_t lookahead);
const char * lexer_intern_range(Lexer * lexer, size_t start, size_t end);
