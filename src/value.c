#include "common.h"
#include "value.h"
#include "gc.h"
#include "vm.h"
#include "builtin.h"

#include <ctype.h>

// : Value

const char * value_type_names[] = {
	"none",
	"type",
	"integer",
	"float",
	"bool",
	"string",
	"function",
	"builtin",
	"list",
	"dictionary",
};

// :\ Value

// : Value creation

Value value_none()
{
	return (Value) { VALUE_NONE };
}

Value value_new_type(Value_Type t)
{
	return (Value) { VALUE_TYPE, ._type = t };
}

Value value_new_integer(int i)
{
	return (Value) { VALUE_INTEGER, ._integer = i };
}

Value value_new_float(float f)
{
	return (Value) { VALUE_FLOAT, ._float = f };
}

Value value_new_bool(bool b)
{
	return (Value) { VALUE_BOOL, ._bool = b };
}

Value value_new_string(const char * s)
{
	Winter_String string;
	string.size = strlen(s);
	string.contents = global_alloc(string.size + 1);
	strcpy(string.contents, s);
	return (Value) { VALUE_STRING, ._string = string };
}

Value value_new_function(BC_Chunk * bytecode)
{
	Function * func = global_alloc(sizeof(Function));
	func->bytecode = bytecode;
	func->closure = variable_map_new();
	return (Value) {
		VALUE_FUNCTION, ._function = func
	};
}

Value value_new_builtin(Builtin b)
{
	return (Value) { VALUE_BUILTIN, ._builtin = b };
}

Value value_new_list()
{
	Winter_List * list = global_alloc(sizeof(Winter_List));
	list->size     = 0;
	list->capacity = 4;
	list->contents = global_alloc(sizeof(Value) * 4);
	return (Value) { VALUE_LIST, ._list = list };
}

Value * value_as_gc_pointer(Value value)
{
	Value * value_ptr = global_alloc(sizeof(Value));
	memcpy(value_ptr, &value, sizeof(Value));
	return value_ptr;
}

Value value_new_dictionary()
{
	Winter_Dictionary * dict = global_alloc(sizeof(Winter_Dictionary));
	dict->size   = 0;
	dict->keys   = value_as_gc_pointer(value_new_list());
	dict->values = value_as_gc_pointer(value_new_list());
	return (Value) { VALUE_DICTIONARY, ._dictionary = dict };
}

// :\ Value creation

// : Value operations

Value value_print(Value value)
{
	switch (value.type) {
	case VALUE_NONE:
		printf("none\n");
		break;
	case VALUE_INTEGER:
		printf("%d\n", value._integer);
		break;
	case VALUE_FLOAT:
		printf("%f\n", value._float);
		break;
	case VALUE_BOOL:
		printf("%s\n", value._bool ? "true" : "false");
		break;
	case VALUE_STRING:
		printf("%.*s\n", value._string.size, value._string.contents);
		break;
	case VALUE_FUNCTION:
		#if DEBUG_PRINTS
		printf("<function at %p; refcount %d>\n",
			   value._function,
			   gc_get_refcount(value._function));
		#else
		printf("<function at %p>\n", value._function);
		#endif
		break;
	case VALUE_BUILTIN:
		printf("<builtin function %s>\n",
			   builtin_names[value._builtin]);
		break;
	case VALUE_LIST: {
		if (value._list->size == 0) {
			printf("[]\n");
			break;
		}
		printf("[\n");
		for (int i = 0; i < value._list->size; i++) {
			printf("  ");
			value_print(value._list->contents[i]);
		}
		printf("]\n");
	} break;
	default:
		fatal_internal("Tried to print a value with no implemented print routine");
	}
}

#define fatal_given_type() fatal_assoc(assoc, "Not valid on given type");
#define check_same_type()												\
	do {																\
		if (a.type != b.type) {											\
			fatal_assoc(assoc, "Operation must be used on two of the same type"); \
		}																\
	} while (false)

Value value_negate(Value a, Assoc_Source assoc)
{
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(-a._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(-a._float);
		break;
	default:
		fatal_given_type();
	}
}

Value value_add(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(a._integer + b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(a._float + b._float);
		break;
	default:
		fatal_given_type();
	}
}

Value value_multiply(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(a._integer * b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(a._float * b._float);
		break;
	default:
		fatal_given_type();
	}	
}

Value value_divide(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_float((float) a._integer /
							   (float) b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(a._float / b._float);
		break;
	default:
		fatal_given_type();
	}	
}

Value value_not(Value a, Assoc_Source assoc)
{
	if (a.type != VALUE_BOOL) {
		fatal_assoc(assoc, "Operation valid only for bools");
	}
	return value_new_bool(!a._bool);
}

Value value_and(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	if (a.type != VALUE_BOOL) {
		fatal_assoc(assoc, "Operation valid only for bools");
	}
	return value_new_bool(a._bool && b._bool);
}

Value value_or(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	if (a.type != VALUE_BOOL) {
		fatal_assoc(assoc, "Operation valid only for bools");
	}
	return value_new_bool(a._bool || b._bool);
}

bool value_internal_equal(Value a, Value b)
{
	switch (a.type) {
	case VALUE_NONE:
		return true;
	case VALUE_TYPE:
		return a._type == b._type;
	case VALUE_INTEGER:
		return a._integer == b._integer;
	case VALUE_FLOAT:
		return a._float == b._float;
	case VALUE_BOOL:
		return a._bool == b._bool;
	case VALUE_STRING:
		if (a._string.size != b._string.size) return false;
		return strncmp(a._string.contents, b._string.contents, a._string.size);
	case VALUE_FUNCTION:
		return a._function == b._function;
	case VALUE_BUILTIN:
		return a._builtin == b._builtin;
	case VALUE_LIST:
		internal_assert(false); // TODO(pixlark): Do this
	case VALUE_DICTIONARY:
		internal_assert(false);
	default:
		fatal_internal("Switch cases for value_internal_equal not complete");
	}
}

Value value_equal(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	return value_new_bool(value_internal_equal(a, b));
}

Value value_greater_than(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_bool(a._integer > b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_bool(a._float > b._float);
		break;
	default:
		fatal_given_type();
	}
}

Value value_less_than(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_bool(a._integer < b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_bool(a._float < b._float);
		break;
	default:
		fatal_given_type();
	}
}

Value value_cast_none(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_STRING:
		return value_new_string("none");
		break;
	default:
		fatal_assoc(assoc, "Can't cast none to given type");
	}
}

Value value_cast_type(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_STRING: {
		char buffer[512];
		sprintf(buffer, "<type: %s>", value_type_names[a._type]);
		return value_new_string(buffer);
	} break;
	default:
		fatal_assoc(assoc, "Can't cast type to given type");
	}
}

Value value_cast_integer(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_INTEGER:
		return a;
	case VALUE_FLOAT:
		return value_new_float((float) a._integer);
	case VALUE_BOOL:
		return value_new_bool(a._integer);
	case VALUE_STRING: {
		char buffer[512];
		sprintf(buffer, "%d", a._integer);
		return value_new_string(buffer);
	} break;
	default:
		fatal_assoc(assoc, "Can't cast integer to given type");
	}
}

Value value_cast_float(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_INTEGER:
		return value_new_integer((int) a._float);
	case VALUE_FLOAT:
		return a;
	case VALUE_STRING: {
		char buffer[512];
		sprintf(buffer, "%f", a._float);
		return value_new_string(buffer);
	} break;
	default:
		fatal_assoc(assoc, "Can't cast float to given type");
	}
}

Value value_cast_bool(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_INTEGER:
		return value_new_integer(a._bool ? 1 : 0);
	case VALUE_BOOL:
		return a;
	case VALUE_STRING:
		return value_new_string(a._bool ? "true" : "false");
	default:
		fatal_assoc(assoc, "Can't cast bool to given type");
	}
}

// TODO(pixlark): Should we share string casting procedures w/ the
// lexer? It would kind of make sense...

bool string_can_be_int(Winter_String s)
{
	for (int i = 0; i < s.size; i++) {
		if (!isdigit(s.contents[i])) return false;
	}
	return true;
}

// TODO(pixlark): Cast to float
Value value_cast_string(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_INTEGER: {
		if (!string_can_be_int(a._string)) {
			fatal_assoc(assoc, "String not in integer form");
		}
		return value_new_integer(atoi(a._string.contents));
	} break;
	case VALUE_STRING:
		return a;
	default:
		fatal_assoc(assoc, "Can't cast string to given type");
	}
}

Value value_cast_function(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_FUNCTION:
		return a;
	case VALUE_STRING: {
		char buffer[512];
		sprintf(buffer, "<function at %p>", a._function);
		return value_new_string(buffer);
	} break;
	default:
		fatal_assoc(assoc, "Can't cast function to given type");
	}
}

Value value_cast_builtin(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_STRING: {
		char buffer[512];
		sprintf(buffer, "<builtin function %s>", builtin_names[a._builtin]);
		return value_new_string(buffer);
	} break;
	case VALUE_BUILTIN:
		return a;
	default:
		fatal_assoc(assoc, "Can't cast builtin to given type");
	}	
}

Value value_cast_list(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_STRING: {
		// TODO(pixlark): This of indeterminate length, so using a
		// static buffer is dangerous. Make a string builder and use that!
		char buffer[512];
		strcpy(buffer, "[");
		for (int i = 0; i < a._list->size; i++) {
			// s will get collected automatically
			Value s = value_cast(a._list->contents[i], VALUE_STRING, assoc);
			strcat(buffer, s._string.contents);
			if (i != a._list->size - 1) strcat(buffer, ", ");
		}
		strcat(buffer, "]");
		return value_new_string(buffer);
	} break;
	case VALUE_LIST:
		return a;
	default:
		fatal_assoc(assoc, "Can't cast list to given type");
	}
}

Value value_cast_dictionary(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_STRING: {
		// TODO(pixlark): See warning in value_cast_list!!
		char buffer[512];
		strcpy(buffer, "{");
		for (int i = 0; i < a._dictionary->size; i++) {
			Value k = value_cast(a._dictionary->keys->_list->contents[i], VALUE_STRING, assoc);
			strcat(buffer, k._string.contents);
			strcat(buffer, " -> ");
			Value v = value_cast(a._dictionary->values->_list->contents[i], VALUE_STRING, assoc);
			strcat(buffer, v._string.contents);
			if (i != a._dictionary->size - 1) strcat(buffer, ", ");
		}
		strcat(buffer, "}");
		return value_new_string(buffer);
	} break;
	case VALUE_DICTIONARY:
		return a;
	default:
		fatal_assoc(assoc, "Can't cast dictionary to given type");
	}
}

Value value_cast(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (a.type) {
	case VALUE_NONE:
		return value_cast_none(a, type, assoc);
	case VALUE_TYPE:
		return value_cast_type(a, type, assoc);
	case VALUE_INTEGER:
		return value_cast_integer(a, type, assoc);
	case VALUE_FLOAT:
		return value_cast_float(a, type, assoc);
	case VALUE_BOOL:
		return value_cast_bool(a, type, assoc);
	case VALUE_STRING:
		return value_cast_string(a, type, assoc);
	case VALUE_FUNCTION:
		return value_cast_function(a, type, assoc);
	case VALUE_BUILTIN:
		return value_cast_builtin(a, type, assoc);
	case VALUE_LIST:
		return value_cast_list(a, type, assoc);
	case VALUE_DICTIONARY:
		return value_cast_dictionary(a, type, assoc);
	default:
		fatal_internal("Not all switch cases covered in value_cast");
	}
}

Value * value_index(Value collection, Value index, Assoc_Source assoc)
{
	switch (collection.type) {
	case VALUE_LIST:
		if (index.type != VALUE_INTEGER) {
			fatal_assoc(assoc, "Invalid index for list");
		}
		if (index._integer >= collection._list->size || index._integer < 0) {
			fatal_assoc(assoc, "List index out of bounds");
		}
		return collection._list->contents + index._integer;
	case VALUE_DICTIONARY: {
		Value * val = value_index_dictionary(collection, index);
		if (!val) {
			fatal_assoc(assoc, "Key not found in dictionary");
		}
		return val;
	} break;
	default:
		fatal_assoc(assoc, "Can't index type");
	}

}

// :\ Value operations

// : List operations

void value_append_list(Value value, Value to_append)
{
	internal_assert(value.type == VALUE_LIST);
	Winter_List * list = value._list;
	if (list->size >= list->capacity) {
		list->capacity *= 2;
		list->contents = global_realloc(list->contents,
										list->capacity * sizeof(Value));
	}
	list->contents[list->size] = to_append;
	list->size++;
}

Value value_pop_list(Value value)
{
	internal_assert(value.type == VALUE_LIST);
	Winter_List * list = value._list;
	Value popped = list->contents[--list->size];
	value_modify_refcount(popped, -1);
	return popped;
}

void value_append(Value list, Value to_append, Assoc_Source assoc)
{
	switch (list.type) {
	case VALUE_LIST:
		value_append_list(list, to_append);
		break;
	default:
		fatal_assoc(assoc, "Can't append to non-list");
	}
}

// :\ List operations

// : Dictionary operations

static void check_dictionary_sizes(Winter_Dictionary * dict)
{
	size_t dict_size = dict->size;
	size_t keys_size = dict->keys->_list->size;
	size_t vals_size = dict->values->_list->size;
	if (dict_size != keys_size ||
		keys_size != vals_size ||
		vals_size != dict_size) {
		fatal_internal("Dictionary sizes fallen out of sync: d:%d vs k:%d vs v:%d",
					   dict_size, keys_size, vals_size);
	}
}

Value * value_index_dictionary(Value collection, Value key)
{
	internal_assert(collection.type == VALUE_DICTIONARY);
	Winter_Dictionary * dict = collection._dictionary;
	check_dictionary_sizes(dict);
	for (int i = 0; i < dict->size; i++) {
		if (value_internal_equal(key, dict->keys->_list->contents[i])) {
			return dict->values->_list->contents + i;
		}
	}
	return NULL;
}

void value_add_pair_dictionary(Value dict, Value key, Value value)
{
	internal_assert(dict.type == VALUE_DICTIONARY);
	check_dictionary_sizes(dict._dictionary);
	dict._dictionary->size++;
	value_append_list(*(dict._dictionary->keys), key);
	value_append_list(*(dict._dictionary->values), value);
}

void value_add_pair(Value dict, Value key, Value value, Assoc_Source assoc)
{
	switch (dict.type) {
	case VALUE_DICTIONARY:
		value_add_pair_dictionary(dict, key, value);
		break;
	default:
		fatal_assoc(assoc, "Can't add pair to non-dictionary");
	}
}

// :\ Dictionary operations

// : Value GC

void value_modify_refcount(Value value, int change)
{
	switch (value.type) {
	case VALUE_NONE:
		break;
	case VALUE_TYPE:
		break;
	case VALUE_INTEGER:
		break;
	case VALUE_FLOAT:
		break;
	case VALUE_BOOL:
		break;
	case VALUE_STRING:
		gc_modify_refcount(value._string.contents, change);
		break;
	case VALUE_FUNCTION:
		gc_modify_refcount(value._function, change);
		value_modify_refcount(value._function->parameter_list, change);
		break;
	case VALUE_BUILTIN:
		break;
	case VALUE_LIST:
		gc_modify_refcount(value._list, change);
		gc_modify_refcount(value._list->contents, change);
		for (int i = 0; i < value._list->size; i++) {
			value_modify_refcount(value._list->contents[i], change);
		}
		break;
	case VALUE_DICTIONARY:
		gc_modify_refcount(value._dictionary, change);
		gc_modify_refcount(value._dictionary->keys, change);
		value_modify_refcount(*value._dictionary->keys, change);
		gc_modify_refcount(value._dictionary->values, change);
		value_modify_refcount(*value._dictionary->values, change);
		break;
	default:
		fatal_internal("Switch statement in value_modify_refcount not complete");
	}
}

// :\ Value GC
