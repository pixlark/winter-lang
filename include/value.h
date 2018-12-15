#pragma once

#include "vm.h"

Value value_none();
Value value_new_integer(int i);
Value value_new_float(float f);
Value value_new_function(const char ** parameters, size_t parameter_count, BC_Chunk * bytecode);

Value value_print(Value value);
bool value_equal(Value a, Value b);

Value value_negate(Value a);
Value value_add(Value a, Value b);
