#pragma once

typedef enum {
	VALUE_INTEGER,
	VALUE_FLOAT,
} Value_Type;

typedef struct {
	Value_Type type;
	union {
		int _integer;
		float _float;
	};
} Value;

Value value_new_integer(int i);
Value value_new_float(float f);

Value value_print(Value value);
bool value_equal(Value a, Value b);

Value value_negate(Value a);
Value value_add(Value a, Value b);
