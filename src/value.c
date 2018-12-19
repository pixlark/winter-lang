#include "common.h"
#include "value.h"

#include <ctype.h>

// : Value

// Holds values in memory

Value value_none()
{
	return (Value) { VALUE_NONE };
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
	return (Value) { VALUE_STRING, ._string =
			(Winter_String) { s ? strlen(s) : 0, s } };
}

Value value_new_function(const char * name, const char ** parameters, BC_Chunk * bytecode)
{
	Function * func = malloc(sizeof(Function));
	func->name = name;
	func->parameters = parameters;
	func->bytecode = bytecode;
	return (Value) {
		VALUE_FUNCTION, ._function = func
	};
}

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
		printf("%.*s\n", value._string.len, value._string.contents);
		break;
	case VALUE_FUNCTION:
		printf("<function at %p>\n", value._function);
		break;
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

Value value_equal(Value a, Value b, Assoc_Source assoc)
{
	check_same_type();
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_bool(a._integer == b._integer);
	case VALUE_FLOAT:
		return value_new_bool(a._float == b._float);
	case VALUE_BOOL:
		return value_new_bool(a._bool == b._bool);
	default:
		fatal_given_type();
	}
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

Value value_cast_integer(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (type) {
	case VALUE_INTEGER:
		return a;
	case VALUE_FLOAT:
		return value_new_float((float) a._integer);
	case VALUE_BOOL:
		return value_new_bool(a._integer);
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
	default:
		fatal_assoc(assoc, "Can't cast bool to given type");
	}
}

// TODO(pixlark): Should we share string casting procedures w/ the
// lexer? It would kind of make sense...

bool string_can_be_int(Winter_String s)
{
	for (int i = 0; i < s.len; i++) {
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
	default:
		fatal_assoc(assoc, "Can't cast function to given type");
	}
}

Value value_cast(Value a, Value_Type type, Assoc_Source assoc)
{
	switch (a.type) {
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
	default:
		fatal_internal("Not all switch cases covered in value_cast");
	}
}

// :\ Value
