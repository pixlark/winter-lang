#pragma once

#include "vm.h"

Value value_none();
Value value_new_integer(int i);
Value value_new_float(float f);
Value value_new_bool(bool b);
Value value_new_string(const char * s);
Value value_new_function(const char * name, const char ** parameters, BC_Chunk * bytecode);

Value value_print(Value value);

Value value_negate(Value a, Assoc_Source assoc);
Value value_add(Value a, Value b, Assoc_Source assoc);
Value value_multiply(Value a, Value b, Assoc_Source assoc);
Value value_divide(Value a, Value b, Assoc_Source assoc);
Value value_not(Value a, Assoc_Source assoc);
Value value_and(Value a, Value b, Assoc_Source assoc);
Value value_or(Value a, Value b, Assoc_Source assoc);
Value value_equal(Value a, Value b, Assoc_Source assoc);
Value value_greater_than(Value a, Value b, Assoc_Source assoc);
Value value_less_than(Value a, Value b, Assoc_Source assoc);
Value value_cast(Value a, Value_Type type, Assoc_Source assoc);
