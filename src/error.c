#include "error.h"

#include "common.h"

Assoc_Source assoc_source_new(Lexer * lexer,
							  size_t line,
							  size_t position,
							  size_t len)
{
	return (Assoc_Source) {
		lexer, line, position, len, false
	};
}

Assoc_Source assoc_source_eof(Lexer * lexer)
{
	return (Assoc_Source) {
		lexer, 0, 0, 0, true
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

void print_line(const char * start, const char * end)
{
	if (!end) {
		end = start + strlen(start);
	}
	for (const char * p = start; p < end; p++) {
		char c = *p;
		if (c == '\t') {
			fprintf(stderr, "    ");
		} else {
			fprintf(stderr, DIM("%c"), c);
		}
	}
	fprintf(stderr, "\n");
}

void print_underline(size_t len, size_t position)
{
	for (int i = 0; i < position; i++) {
		fprintf(stderr, " ");
	}
	for (int i = 0; i < len; i++) {
		fprintf(stderr, RED("^"));
	}
}

void print_assoc(Assoc_Source assoc)
{
	const char * pos = assoc.lexer->source + assoc.position;
	// Peek back until line start
	size_t tabs = 0;
	const char * line_start = pos;
	while (true) {
		if (line_start == assoc.lexer->source) break;
		if (*line_start == '\t') {
			tabs++;
		} else if (*line_start == '\n') {
			line_start++;
			break;
		}
		line_start--;
	}
	if (assoc.eof) {
		// Print line
		fprintf(stderr, "    ");
		//DIM("%s") "\n", line_start);
		print_line(line_start, NULL);
		// Print underline
		fprintf(stderr, "    ");
		print_underline(1, (pos - line_start) + 1);
	} else {
		// Peek forwards until line end
		const char * line_end = pos;
		while (true) {
			if ((line_end - assoc.lexer->source) >= assoc.lexer->source_len) break;
			if (*line_end == '\n') break;
			line_end++;
		}
		// Print line
		fprintf(stderr, "    ");
		//fprintf(stderr, "    " DIM("%.*s") "\n", line_end - line_start, line_start);
		print_line(line_start, line_end);
		// Print underline
		fprintf(stderr, "    ");
		size_t underline_pos = (pos - line_start - tabs) + (tabs * 4);
		print_underline(assoc.len, underline_pos);
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
