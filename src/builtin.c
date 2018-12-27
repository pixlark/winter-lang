#include "builtin.h"

#include "common.h"

const char * builtin_names[] = {
	"print",
	"assert",
	"typeof",
	"list_append",
	"list_pop",
	"list_count",
};

// -1 means varargs
int builtin_arg_counts[] = {
	-1,
	1,
	1,
	2,
	1,
	1,
};

#define DEFINE_BUILTIN(name) Value name (Value * args, size_t arg_count, Assoc_Source assoc)

DEFINE_BUILTIN(builtin_print)
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

DEFINE_BUILTIN(builtin_assert)
{
	internal_assert(arg_count == 1);
	Value condition = args[0];
	if (condition.type != VALUE_BOOL) {
		fatal_assoc(assoc, "assert requires a bool");
	}
	if (condition._bool) {
		return value_none();
	} else {
		fatal_user_assert_failed(assoc);
	}
}

DEFINE_BUILTIN(builtin_typeof)
{
	return value_new_type(args[0].type);
}

DEFINE_BUILTIN(builtin_list_append)
{
	Value list = args[0];
	if (list.type != VALUE_LIST) {
		fatal_assoc(assoc, "list_append requires a list");
	}
	Value to_append = args[1];
	value_append_list(list, to_append);
	value_modify_refcount(to_append, 1);
	return value_none();
}

DEFINE_BUILTIN(builtin_list_pop)
{
	Value list = args[0];
	if (list.type != VALUE_LIST) {
		fatal_assoc(assoc, "list_pop requires a list");
	}
	Value popped = value_pop_list(list);
	return popped;
}

DEFINE_BUILTIN(builtin_list_count)
{
	Value list = args[0];
	internal_assert(list.type == VALUE_LIST);
	return value_new_integer(list._list->size);
}

Value (*builtin_functions[])(Value*, size_t, Assoc_Source) = {
	builtin_print,
	builtin_assert,
	builtin_typeof,
	builtin_list_append,
	builtin_list_pop,
	builtin_list_count,
};
