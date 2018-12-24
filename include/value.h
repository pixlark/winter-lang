#pragma once

#include "error.h"

typedef size_t Builtin;

// : Value
typedef struct Function Function;
typedef struct BC_Chunk BC_Chunk;

typedef enum {
	VALUE_NONE,
	VALUE_INTEGER,
	VALUE_FLOAT,
	VALUE_BOOL,
	VALUE_STRING,
	VALUE_FUNCTION,
	VALUE_BUILTIN,
	VALUE_LIST,
} Value_Type;

typedef struct Value Value;

typedef struct {
	size_t len;
	char * contents;
} Winter_String;

typedef struct {
	size_t size;
	size_t capacity;
	Value * contents;
} Winter_List;

struct Value {
	Value_Type type;
	union {
		int _integer;
		float _float;
		bool _bool;
		Winter_String _string;
		Function * _function;
		Builtin _builtin;
		Winter_List _list;
	};
};

// :\ Value

// : Value creation
Value value_none();
Value value_new_integer(int i);
Value value_new_float(float f);
Value value_new_bool(bool b);
Value value_new_string(const char * s);
Value value_new_function(const char * name, const char ** parameters, BC_Chunk * bytecode);
Value value_new_builtin(Builtin b);
Value value_new_list();
// :\ Value creation

// : Value operations
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
/* temporary */ Value value_append_list(Value value, Value to_append);
Value value_append(Value array, Value to_append, Assoc_Source assoc);
// :\ Value operations

// : Value GC
void value_modify_refcount(Value value, int change);
// :\ Value GC
