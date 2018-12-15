#include "common.h"
#include "value.h"

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

Value value_new_function(const char ** parameters, BC_Chunk * bytecode)
{
	Function * func = malloc(sizeof(Function));
	func->parameters = parameters;
	func->bytecode = bytecode;
	return (Value) {
		VALUE_FUNCTION, ._function = func
	};
}

bool value_equal(Value a, Value b)
{
	if (a.type == b.type) {
		switch (a.type) {
		case VALUE_INTEGER:
			return a._integer == b._integer;
		case VALUE_BOOL:
			return a._bool == b._bool;
		default:
			fatal_internal("Type is not comparable");
		}
	} else {
		fatal_internal("Can't compare differing types");
	}
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
	case VALUE_FUNCTION:
		printf("<function at %p>\n", value._function);
		break;
	}
}

Value value_negate(Value a)
{
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(-a._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(-a._float);
		break;
	default:
		fatal("Negate not valid on given type");
	}
}

Value value_add(Value a, Value b)
{
	if (a.type != b.type) {
		fatal("Add must be used on two of the same type");
	}
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(a._integer + b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(a._float + b._float);
		break;
	default:
		fatal("Add not valid on given types");
	}
}

// :\ Value
