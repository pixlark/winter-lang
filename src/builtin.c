#include "builtin.h"

#include "common.h"

const char * builtin_names[] = {
	"print",
};

// -1 means varargs
int builtin_arg_counts[] = {
	-1,
};

// TODO(pixlark): Seperate w/ spaces rather than newlines
Value builtin_print(Value * args, size_t arg_count)
{
	for (int i = 0; i < arg_count; i++) {
		value_print(args[i]);
	}
	if (arg_count == 0) printf("\n");
	return value_none();
}

Value (*builtin_functions[])(Value*, size_t) = {
	builtin_print,
};
