#include "common.h"
#include "value.h"

// : Value

// Holds values in memory

Value value_new_integer(int i)
{
	return (Value) { VALUE_INTEGER, ._integer = i };
}

Value value_new_float(float f)
{
	return (Value) { VALUE_FLOAT, ._float = f };
}

bool value_equal(Value a, Value b)
{
	if (a.type == b.type) {
		switch (a.type) {
		case VALUE_INTEGER:
			return a._integer == b._integer;
		default:
			assert(false);
		}
	}
	return false;
}

Value value_print(Value value)
{
	switch (value.type) {
	case VALUE_INTEGER:
		printf("%d\n", value._integer);
		break;
	case VALUE_FLOAT:
		printf("%f\n", value._float);
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
		assert(false); // TODO(pixlark): Proper error handling
	}
}

Value value_add(Value a, Value b)
{
	assert(a.type == b.type);
	switch (a.type) {
	case VALUE_INTEGER:
		return value_new_integer(a._integer + b._integer);
		break;
	case VALUE_FLOAT:
		return value_new_float(a._float + b._float);
		break;
	default:
		assert(false);
	}
}

// :\ Value
