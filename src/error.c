#include "error.h"

#include "common.h"

#define RESET         "\e[0m"
#define SET_BOLD      "\e[1m"
#define SET_DIM       "\e[2m"
#define SET_UNDERLINE "\e[4m"
#define SET_BLINK     "\e[5m"
#define SET_INVERTED  "\e[7m"
#define SET_HIDDEN    "\e[8m"

#define SET_RED "\e[31m"

#define BOLD(x) SET_BOLD x RESET
#define RED(x)  SET_RED x RESET

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);
	exit(1);
}

void fatal_internal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, "internal compiler error:\n");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);
	exit(1);
}
