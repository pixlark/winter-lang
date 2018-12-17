#include "error.h"

#include "common.h"

Assoc_Source assoc_source_new(size_t line,
							  size_t start,
							  size_t end)
{
	return (Assoc_Source) {
		line, start, end,
	};
}

#define RESET         "\e[0m"
#define SET_BOLD      "\e[1m"
#define SET_DIM       "\e[2m"
#define SET_UNDERLINE "\e[4m"
#define SET_BLINK     "\e[5m"
#define SET_INVERTED  "\e[7m"
#define SET_HIDDEN    "\e[8m"

#define SET_RED "\e[31m"

#define BOLD(x) SET_BOLD x RESET
#define DIM(x) SET_DIM x RESET
#define INVERTED(x) SET_INVERTED x RESET
#define RED(x)  SET_RED x RESET

void fatal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	vfprintf(stderr, fmt, args);
	
	// Bit of a kluge, perhaps there's a better way to deal with
	// bison's newlines
	{
		size_t len = strlen(fmt);
		if (fmt[len-1] != '\n') {
			fprintf(stderr, "\n");
		}
	}

	va_end(args);
	exit(1);
}

void fatal_assoc(Assoc_Source assoc, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	fprintf(stderr, DIM("Line %d\n"), assoc.line);
	vfprintf(stderr, fmt, args);
	
	// Bit of a kluge, perhaps there's a better way to deal with
	// bison's newlines
	{
		size_t len = strlen(fmt);
		if (fmt[len-1] != '\n') {
			fprintf(stderr, "\n");
		}
	}

	va_end(args);
	exit(1);	
}

void fatal_internal(const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, INVERTED(BOLD(RED("internal compiler error"))) ":\n");
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);
	exit(1);
}

void _internal_assert(bool cond, const char * file, size_t line)
{
	if (!cond) {
		fprintf(stderr, INVERTED(BOLD(RED("internal compiler error"))) ":\n");
		fprintf(stderr, "assertion failed at %s:%d\n", file, line);
		exit(1);
	}
}
