#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lexer.h"

Assoc_Source assoc_source_new(Lexer * lexer,
							  size_t line,
							  size_t len,
							  size_t position);

void fatal(const char * fmt, ...);
void fatal_assoc(Assoc_Source assoc, const char * fmt, ...);
void fatal_internal(const char * fmt, ...);
void _internal_assert(bool cond, const char * file, size_t line);

#define internal_assert(cond) _internal_assert((cond), __FILE__, __LINE__)
