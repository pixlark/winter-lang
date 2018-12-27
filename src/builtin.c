#include "builtin.h"

#include "common.h"

const char * builtin_names[] = {
	"print",
	"assert",
	"typeof",
	"list_append",
};

// -1 means varargs
int builtin_arg_counts[] = {
	-1,
	1,
	1,
	2,
};

Value builtin_print(Value * args, size_t arg_count, Assoc_Source assoc)
{
	for (int i = 0; i < arg_count; i++) {
		// to_print should get collected automatically
		Value to_print = value_cast(args[i], VALUE_STRING, assoc);
		internal_assert(to_print.type == VALUE_STRING);
		if (i != arg_count - 1) {
			printf("%s ", to_print._string.contents);
		} else {
			printf("%s", to_print._string.contents);
		}
	}
	printf("\n");
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

Value builtin_typeof(Value * args, size_t arg_count, Assoc_Source assoc)
{
	return value_new_type(args[0].type);
}

Value builtin_list_append(Value * args, size_t arg_count, Assoc_Source assoc)
{
	Value list = args[0];
	internal_assert(list.type == VALUE_LIST);
	Value to_append = args[1];
	value_append_list(list, to_append);
	return value_none();
}

Value (*builtin_functions[])(Value*, size_t, Assoc_Source) = {
	builtin_print,
	builtin_assert,
	builtin_typeof,
	builtin_list_append,
};
