#include "builtin.h"

#include "common.h"

const char * builtin_names[] = {
	"print",
	"assert",
};

// -1 means varargs
int builtin_arg_counts[] = {
	-1,
	1,
};

// TODO(pixlark): Seperate w/ spaces rather than newlines
Value builtin_print(Value * args, size_t arg_count, Assoc_Source assoc)
{
	for (int i = 0; i < arg_count; i++) {
		value_print(args[i]);
	}
	if (arg_count == 0) printf("\n");
	return value_none();
}

Value builtin_assert(Value * args, size_t arg_count, Assoc_Source assoc)
{
	internal_assert(arg_count == 1);
	Value condition = args[0];
	if (condition.type != VALUE_BOOL) {
		fatal_assoc(assoc, "Assert requires a bool");
	}
	if (condition._bool) {
		return value_none();
	} else {
		fatal_user_assert_failed(assoc);
	}
}

Value (*builtin_functions[])(Value*, size_t, Assoc_Source) = {
	builtin_print,
	builtin_assert,
};
