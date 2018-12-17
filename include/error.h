#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	size_t line;
	size_t start;
	size_t end;
} Assoc_Source;

Assoc_Source assoc_source_new(size_t line,
							  size_t start,
							  size_t end);

void fatal(const char * fmt, ...);
void fatal_assoc(Assoc_Source assoc, const char * fmt, ...);
void fatal_internal(const char * fmt, ...);
void _internal_assert(bool cond, const char * file, size_t line);

#define internal_assert(cond) _internal_assert((cond), __FILE__, __LINE__)
