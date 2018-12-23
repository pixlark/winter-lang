#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

// : Assoc_Source
typedef struct Lexer Lexer;

typedef struct {
	struct Lexer * lexer;
	size_t line;
	size_t position;
	size_t len;
	bool eof;
} Assoc_Source;

Assoc_Source assoc_source_new(Lexer * lexer,
							  size_t line,
							  size_t position,
							  size_t len);

Assoc_Source assoc_source_eof(Lexer * lexer);
// :\ Assoc_Source

// : Fatal functions
void fatal(const char * fmt, ...);
void fatal_assoc(Assoc_Source assoc, const char * fmt, ...);
void fatal_user_assert_failed(Assoc_Source assoc);
void fatal_internal(const char * fmt, ...);
void _internal_assert(bool cond, const char * file, size_t line);

#define internal_assert(cond) _internal_assert((cond), __FILE__, __LINE__)
// :\ Fatal functions

// : Debug printing
void dbprintf(const char * fmt, ...);
// :\ Debug printing
