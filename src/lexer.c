#include "lexer.h"

#include "common.h"

#include <ctype.h>

// : String interning

int lexer_intern_fetch(Lexer * lexer, size_t start, size_t end)
{
	for (int i = 0; i < sb_count(lexer->interned_strings); i++) {
		const char * str = lexer->interned_strings[i];
		if (strlen(str) != end - start) continue; // Assert same length
		
		bool matches = true;
		for (int j = start; j < end; j++) {
			if (lexer->source[j] != str[j - start]) {
				matches = false;
				break;
			}
		}
		if (matches) {
			return i;
		}
	}
	return -1;
}

const char * lexer_dup_range(Lexer * lexer, size_t start, size_t end)
{
	char * intern = malloc(end - start + 1);
	intern[end - start] = '\0';
	strncpy(intern, lexer->source + start, end - start);
	return intern;
}

const char * lexer_intern_range(Lexer * lexer, size_t start, size_t end)
{
	int index = lexer_intern_fetch(lexer, start, end);
	if (index != -1) {
		return lexer->interned_strings[index];
	}
	// Not already interned
	const char * intern = lexer_dup_range(lexer, start, end);
	sb_push(lexer->interned_strings, intern);
	return intern;
}

const char * lexer_intern_string(Lexer * lexer, const char * str)
{
	internal_assert(str);
	for (int i = 0; i < sb_count(lexer->interned_strings); i++) {
		if (strcmp(str, lexer->interned_strings[i]) == 0) {
			return lexer->interned_strings[i];
		}
	}
	// Not already interned
	sb_push(lexer->interned_strings, strdup(str));
}

// :\ String interning

// : Token printing

const char * token_type_names[] = {
	[TOKEN_EOF] = "EOF",
	[TOKEN_NONE] = "none",
	[TOKEN_TRUE] = "true",
	[TOKEN_FALSE] = "false",
	[TOKEN_RETURN] = "return",
	[TOKEN_IF] = "if",
	[TOKEN_ELSE] = "else",
	[TOKEN_FUNC] = "func",
	[TOKEN_LOOP] = "loop",
	[TOKEN_WHILE] = "while",
	[TOKEN_BREAK] = "break",
	[TOKEN_CONTINUE] = "continue",
	[TOKEN_RARROW] = "->",
	[TOKEN_RECORD] = "record",

	[TOKEN_AS] = "as",
	[TOKEN_INT] = "int",
	[TOKEN_FLOAT] = "float",
	[TOKEN_BOOL] = "bool",
	[TOKEN_STRING] = "string",
	[TOKEN_LIST] = "list",
	
	[TOKEN_OR] = "or",
	[TOKEN_AND] = "and",
	[TOKEN_EQ] = "==",
	[TOKEN_NE] = "!=",
	[TOKEN_GTE] = ">=",
	[TOKEN_LTE] = "<=",

	[TOKEN_NAME] = "<name>",
	[TOKEN_BUILTIN] = "<builtin>",
	[TOKEN_INTEGER_LITERAL] = "<int>",
	[TOKEN_FLOAT_LITERAL] = "<float>",
	[TOKEN_STRING_LITERAL] = "<string>",
};

char * token_type_to_string(Token_Type type)
{
	if (type < 256) {
		char * s = malloc(2);
		s[0] = type;
		s[1] = '\0';
		return s;
	} else {
		return strdup(token_type_names[type]);
	}
}

char * token_to_string(Token token)
{
	if (token.type < 256) {
		char * s = malloc(2);
		s[0] = token.type;
		s[1] = '\0';
		return s;
	}
	switch (token.type) {		
	case TOKEN_NAME: {
		char buffer[512];
		sprintf(buffer, "name: '%s'", token.name);
		return strdup(buffer);
	} break;
	case TOKEN_BUILTIN: {
		char buffer[512];
		sprintf(buffer, "builtin: %s", builtin_names[token.builtin]);
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
	default:
		return strdup(token_type_names[token.type]);
	}
}

// :\ Token printing

// : Lexer utility

Lexer * lexer_alloc(const char * source)
{
	Lexer * lexer = malloc(sizeof(Lexer));
	lexer->source = source;
	lexer->source_len = strlen(source);
	lexer->position = 0;
	lexer->line = 1;
	lexer->column = 0;
	lexer->interned_strings = NULL;
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
	lexer->column++;
}

#define next() (lexer_advance_char(lexer), lexer_peek(lexer))

void lexer_advance(Lexer * lexer)
{
	lexer->token = lexer_next_token(lexer);
}

Token lexer_lookahead(Lexer * lexer, size_t lookahead)
{
	// TODO(pixlark): There's probably a fancier way to do this that
	// doesn't require any re-lexing. Let's figure that out later...
	// TODO(pixlark): Also we really need to get string interning
	// working so that this doesn't have memory leaks...
	size_t current_position = lexer->position;
	Token current_token = lexer->token;
	for (int i = 0; i < lookahead; i++) {
		lexer_advance(lexer);
	}
	Token peeked_token = lexer->token;
	lexer->position = current_position;
	lexer->token = current_token;
	return peeked_token;
}

// :\ Lexer utility

// : Lexing

const char * keywords[] = {
	"none",  "true",     "false",
	/*print*/"return",   "if",
	"else",  "func",     "loop",
	"break", "continue", "or",
	"and",   "as",       "int",
	"float", "bool",     "string",
	"list",  "while",    "record",
};

size_t keyword_count = sizeof(keywords) / sizeof(const char *);

Token_Type keyword_tokens[] = {
	TOKEN_NONE,  TOKEN_TRUE,     TOKEN_FALSE,
	/* print */  TOKEN_RETURN,   TOKEN_IF,
	TOKEN_ELSE,  TOKEN_FUNC,     TOKEN_LOOP,
	TOKEN_BREAK, TOKEN_CONTINUE, TOKEN_OR,
	TOKEN_AND,   TOKEN_AS,       TOKEN_INT,
	TOKEN_FLOAT, TOKEN_BOOL,     TOKEN_STRING,
	TOKEN_LIST,  TOKEN_WHILE,    TOKEN_RECORD,
};

Token lexer_next_token(Lexer * lexer)
{
 reset:
	if (lexer->position >= lexer->source_len) {
		Assoc_Source assoc = assoc_source_new(lexer, lexer->line,
											  lexer->position - 1, 0);
		assoc.eof = true;
		return (Token) { TOKEN_EOF, .assoc = assoc };
	}
	char next_char = lexer_peek(lexer);

	// Comments
	if (next_char == '#') {
		do {
			lexer_advance_char(lexer);
			next_char = lexer_peek(lexer);
		} while (next_char != '\n');
	}
	
	// Whitespace
	if (isspace(next_char)) {
		if (next_char == '\n') {
			lexer->line++;
			lexer->column = 0;
		}
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
		const char * name = lexer_intern_range(lexer, start, end);
		// Check against keywords
		for (int i = 0; i < keyword_count; i++) {
			if (strcmp(name, keywords[i]) == 0) {
				Token token;
				token.type = keyword_tokens[i];
				token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
				return token;
			}
		}
		// Check against builtins
		for (int i = 0; i < NUM_BUILTINS; i++) {
			if (strcmp(name, builtin_names[i]) == 0) {
				Token token;
				token.type = TOKEN_BUILTIN;
				token.builtin = i;
				token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
				return token;
			}
		}
		// Otherwise, it's a name
		Token token;
		token.type = TOKEN_NAME;
		token.name = name;
		token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
		return token;
	}

	// TODO(pixlark): Leading +/- signs
	
	// Integer/Float literals
	if (isdigit(next_char) /*|| next_char == '.'*/) {
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
		const char * to_convert = lexer_dup_range(lexer, start, end);
		if (has_decimal_point) {
			// Float literal
			Token token;
			token.type = TOKEN_FLOAT_LITERAL;
			token.float_literal = strtod(to_convert, NULL);
			token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
			free((char*) to_convert);
			return token;
		} else {
			// Integer literal
			Token token;
			token.type = TOKEN_INTEGER_LITERAL;
			token.integer_literal = strtol(to_convert, NULL, 10);
			token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
			free((char*) to_convert);
			return token;
		}
	}

	// String literals
	if (next_char == '"') {
		size_t start = lexer->position;
		next_char = next();
		char * buffer = NULL;
		while (next_char != '"') {
			// Escaped chars
			if (next_char == '\\') {
				lexer_advance_char(lexer);
				sb_push(buffer, lexer_peek(lexer));
				lexer_advance_char(lexer);
				continue;
			}
			sb_push(buffer, next_char);
			next_char = next();
		}
		lexer_advance_char(lexer);
		size_t end = lexer->position;
		Token token;
		token.type = TOKEN_STRING_LITERAL;
		// TODO(pixlark): Should this be interned in the same place as names/keywords?
		sb_push(buffer, '\0');
		token.string_literal = lexer_intern_string(lexer, buffer);
		sb_free(buffer);
		token.assoc = assoc_source_new(lexer, lexer->line, start, end - start);
		return token;
	}
	
	#define TWOCHARTOK(c1, c2, tok)										\
		lexer_advance_char(lexer);										\
		if (lexer_peek(lexer) == c2) {									\
			lexer_advance_char(lexer);									\
			return (Token) { tok, .assoc =								\
					assoc_source_new(lexer, lexer->line,				\
									 lexer->position - 2, 2) };			\
		} else {														\
			return (Token) { c1, .assoc =								\
					assoc_source_new(lexer, lexer->line,				\
									 lexer->position - 1, 1) };			\
		}																\
		break;
	
	switch (next_char) {
		// Reserved chars
	case '{':
	case '}':
	case '(':
	case ')':
	case '[':
	case ']':
	case ';':
	case '+':
	case '*':
	case '/':
	case ',':
	case '.':
		lexer_advance_char(lexer);
		return (Token) { next_char, .assoc = assoc_source_new(lexer, lexer->line,
															  lexer->position - 1, 1) };
	case '=':
		TWOCHARTOK('=', '=', TOKEN_EQ);
	case '!':
		TWOCHARTOK('!', '=', TOKEN_NE);
	case '>':
		TWOCHARTOK('>', '=', TOKEN_GTE);
	case '<':
		TWOCHARTOK('<', '=', TOKEN_LTE);
	case '-':
		TWOCHARTOK('-', '>', TOKEN_RARROW);
	default:
		fatal("Unexpected character '%c'", next_char);
	}

	#undef TWOCHARTOK
}

// :\ Lexing
