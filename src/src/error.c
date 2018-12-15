#include "error.h"

#include "common.h"

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, "encountered error:\n");
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
