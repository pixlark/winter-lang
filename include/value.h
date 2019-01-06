#pragma once

#include "error.h"

typedef size_t Builtin;

// : Value
typedef struct Function Function;
typedef struct BC_Chunk BC_Chunk;

typedef enum {
	VALUE_NONE,
	VALUE_TYPE,
	VALUE_INTEGER,
	VALUE_FLOAT,
	VALUE_BOOL,
	VALUE_STRING,
	VALUE_FUNCTION,
	VALUE_BUILTIN,
	VALUE_LIST,
	VALUE_DICTIONARY,
	VALUE_RECORD,
} Value_Type;

extern const char * value_type_names[];

typedef struct Value Value;

typedef struct {
	size_t size;
	char * contents;
} Winter_String;

typedef struct {
	size_t size;
	size_t capacity;
	Value * contents;
} Winter_List;

// TODO(pixlark): This should be a real hash table, right now it's
// just two VALUE_LISTs strung together.
typedef struct {
	size_t size;
	Value * keys;   // VALUE_LIST
	Value * values; // VALUE_LIST
} Winter_Dictionary;

typedef struct Winter_Canon Winter_Canon;

typedef struct {
	Value_Type type;
	Winter_Canon * canon;
} Winter_Type;

typedef struct Winter_Record Winter_Record;

struct Value {
	Value_Type type;
	union {
		Winter_Type _type;
		int _integer;
		float _float;
		bool _bool;
		Winter_String _string;
		Function * _function;
		Builtin _builtin;
		Winter_List * _list;
		Winter_Dictionary * _dictionary;
		Winter_Record * _record;
	};
};

struct Winter_Record {
	Winter_Canon * canon;
	Value field_dict;
};

struct Winter_Canon {
	Value fields;
};

// :\ Value

// : Value creation
Value value_none();
Value value_new_type(Value_Type t);
Value value_new_integer(int i);
Value value_new_float(float f);
Value value_new_bool(bool b);
Value value_new_string(const char * s);
Value value_new_function(BC_Chunk * bytecode);
Value value_new_builtin(Builtin b);
Value value_new_list();
Value value_new_dictionary();
Value value_new_record(Winter_Canon * canon);
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
// :\ Value operations

// : List operations
void value_append(Value array, Value to_append, Assoc_Source assoc);
void value_append_list(Value value, Value to_append);
Value * value_mutable_index(Value collection, Value index, Assoc_Source assoc);
Value value_index(Value collection, Value index, Assoc_Source assoc);
Value value_pop_list(Value value);
// :\ List operations

// : Dictionary operations
Value * value_index_dictionary(Value collection, Value key);
void value_add_pair_dictionary(Value dict, Value key, Value value);
void value_add_pair(Value dict, Value key, Value value, Assoc_Source assoc);
// :\ Dictionary operations

// : Value GC
void value_modify_refcount(Value value, int change);
// :\ Value GC
