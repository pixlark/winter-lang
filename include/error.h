#pragma once

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

void fatal(const char * fmt, ...);
void fatal_internal(const char * fmt, ...);
void _internal_assert(bool cond, const char * file, size_t line);

#define internal_assert(cond) _internal_assert((cond), __FILE__, __LINE__)
