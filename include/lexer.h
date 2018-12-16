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
} Lexer;

Lexer * lexer_alloc(const char * source);
Token lexer_next_token(Lexer * lexer);
