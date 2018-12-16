#include "common.h"
#include "lexer.h"

#include <ctype.h>

const char * token_type_names[] = {
	[TOKEN_EOF] = "EOF",
	[TOKEN_NONE] = "none",
	[TOKEN_TRUE] = "true",
	[TOKEN_FALSE] = "false",
	[TOKEN_PRINT] = "print",
	[TOKEN_RETURN] = "return",
	[TOKEN_IF] = "if",

	[TOKEN_NAME] = "<name>",
	[TOKEN_INTEGER_LITERAL] = "<int>",
	[TOKEN_FLOAT_LITERAL] = "<float>",
};

char * token_to_string(Token token)
{
	if (token.type < 256) {
		char * s = malloc(2);
		s[0] = token.type;
		s[1] = '\0';
		return s;
	}
	switch (token.type) {
	case TOKEN_EOF:
	case TOKEN_NONE:
	case TOKEN_TRUE:
	case TOKEN_FALSE:
	case TOKEN_PRINT:
	case TOKEN_RETURN:
	case TOKEN_IF:
		return strdup(token_type_names[token.type]);
	case TOKEN_NAME: {
		char buffer[512];
		sprintf(buffer, "name: '%s'", token.name);
		return strdup(buffer);
	} break;
	case TOKEN_INTEGER_LITERAL: {
		char buffer[512];
		sprintf(buffer, "int: %d", token.integer_literal);
		return strdup(buffer);
	} break;
	case TOKEN_FLOAT_LITERAL: {
		char buffer[512];
		sprintf(buffer, "float: %f", token.float_literal);
		return strdup(buffer);
	} break;
	}
}

Lexer * lexer_alloc(const char * source)
{
	Lexer * lexer = malloc(sizeof(Lexer));
	lexer->source = source;
	lexer->source_len = strlen(source);
	lexer->position = 0;
	lexer_advance(lexer);
	return lexer;
}

const char * strdup_from_range(const char * source,
							   size_t start, size_t end)
{
	char * str = malloc(end - start + 1);
	str[end - start] = '\0';
	for (int i = start; i < end; i++) {
		str[i - start] = source[i];
	}
	return str;
}

char lexer_peek(Lexer * lexer)
{
	return lexer->source[lexer->position];
}

void lexer_advance_char(Lexer * lexer)
{
	lexer->position++;
}

const char * keywords[] = {
	"none", "true", "false",
	"print", "return", "if",
};

size_t keyword_count = sizeof(keywords) / sizeof(const char *);

Token_Type keyword_tokens[] = {
	TOKEN_NONE, TOKEN_TRUE, TOKEN_FALSE,
	TOKEN_PRINT, TOKEN_RETURN, TOKEN_IF,
};

Token lexer_next_token(Lexer * lexer)
{
 reset:
	if (lexer->position >= lexer->source_len) {
		return (Token) { TOKEN_EOF };
	}
	char next_char = lexer_peek(lexer);

	// Whitespace
	if (isspace(next_char)) {
		lexer_advance_char(lexer);
		goto reset;
	}
	
	// Names
	if (next_char == '_' || isalpha(next_char)) {
		size_t start = lexer->position;
		while (true) {
			if (!(next_char == '_' || isalnum(next_char))) {
				break;
			}
			lexer_advance_char(lexer);
			next_char = lexer_peek(lexer);
		}
		size_t end = lexer->position;
		const char * name =
			strdup_from_range(lexer->source, start, end);
		// Check against keywords
		for (int i = 0; i < keyword_count; i++) {
			if (strcmp(name, keywords[i]) == 0) {
				Token token;
				token.type = keyword_tokens[i];
				free((char*) name);
				return token;
			}
		}
		// Otherwise, it's a name
		Token token;
		token.type = TOKEN_NAME;
		token.name = name;
		return token;
	}

	// TODO(pixlark): Leading +/- signs
	
	// Integer/Float literals
	if (isdigit(next_char) || next_char == '.') {
		size_t start = lexer->position;
		bool has_decimal_point = false;
		while (true) {
			if (next_char == '.') has_decimal_point = true;
			if (!(next_char == '.' || isdigit(next_char))) {
				break;
			}
			lexer_advance_char(lexer);
			next_char = lexer_peek(lexer);
		}
		size_t end = lexer->position;
		const char * to_convert =
			strdup_from_range(lexer->source, start, end);
		if (has_decimal_point) {
			// Float literal
			Token token;
			token.type = TOKEN_FLOAT_LITERAL;
			token.float_literal = strtod(to_convert, NULL);
			free((char*) to_convert);
			return token;
		} else {
			// Integer literal
			Token token;
			token.type = TOKEN_INTEGER_LITERAL;
			token.integer_literal = strtol(to_convert, NULL, 10);
			free((char*) to_convert);
			return token;
		}
	}
	
	switch (next_char) {
		// Reserved chars
	case '{':
	case '}':
	case '(':
	case ')':
	case ';':
	case '+':
	case '-':
	case '=':
		lexer_advance_char(lexer);
		return (Token) { next_char };
	default:
		fatal("Unexpected character '%c'", next_char);
	}
}

void lexer_advance(Lexer * lexer)
{
	lexer->token = lexer_next_token(lexer);
}
