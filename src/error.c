#include "error.h"

#include "common.h"

Assoc_Source assoc_source_new(Lexer * lexer,
							  size_t line,
							  size_t position,
							  size_t len)
{
	return (Assoc_Source) {
		lexer, line, position, len
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
	printf("\n");

	va_end(args);
	exit(1);
}

void print_assoc(Assoc_Source assoc)
{
	const char * pos = assoc.lexer->source + assoc.position;
	// Peek back until line start
	const char * line_start = pos;
	while (true) {
		if (line_start == assoc.lexer->source) break;
		if (*line_start == '\n') {
			line_start++;
			break;
		}
		line_start--;
	}
	// Peek forwards until line end
	const char * line_end = pos;
	while (true) {
		if ((line_end - assoc.lexer->source) == assoc.lexer->source_len - 1) break;
		if (*line_end == '\n') break;
		line_end++;
	}
	// Print line
	fprintf(stderr, "    " DIM("%.*s") "\n", line_end - line_start, line_start);
	// Print underline
	fprintf(stderr, "    ");
	for (int i = 0; i < (pos - line_start); i++) {
		fprintf(stderr, " ");
	}
	for (int i = 0; i < assoc.len; i++) {
		fprintf(stderr, RED("^"));
	}
	fprintf(stderr, "\n"); 
}

void fatal_assoc(Assoc_Source assoc, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	fprintf(stderr, RED(BOLD("encountered error")) ":\n");
	fprintf(stderr, ":%d\n", assoc.line);
	print_assoc(assoc);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, ("\n"));

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
