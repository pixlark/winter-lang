#pragma once

#include "common.h"
#include "gc.h"
#include "builtin.h"
#include "value.h"

// : Variable_Map

// Maps variable names to pointers to values in memory.

typedef struct {
	size_t size;
	const char ** names;
	Value ** values;
} Variable_Map;

Variable_Map variable_map_new();
Value * variable_map_index(Variable_Map * map, const char * name);
Value * variable_map_update(Variable_Map * map, const char * name, Value value);

// :\ Variable_Map

// : Instruction

// Each instruction in the enum either has arguments, represented by a
// struct in the BC_Chunk union, or has no arguments, in which case
// there is no associated struct.

typedef struct BC_Chunk BC_Chunk;

typedef struct {
	Value value;
} Instr_Push;

typedef struct {
	const char * name;
} Instr_Get;

typedef struct {
	size_t arg_count;
} Instr_Call;

typedef struct {
	int jump_offset;
} Instr_Jump;

typedef struct {
	int jump_offset;
	bool cond;
} Instr_Condjump;

typedef struct {
	int end_offset;
} Instr_Set_Loop;

typedef struct {
	size_t parameter_count;
	BC_Chunk * bytecode;	
} Instr_Create_Function;

typedef struct {
	const char * literal;
} Instr_Create_String;

enum Instruction {
	// No args
	INSTR_NOP,
	INSTR_RETURN,
	INSTR_POP,
	INSTR_LOOP_END,
	INSTR_BREAK,
	INSTR_CONTINUE,
	INSTR_CLOSURE,
	INSTR_APPEND,
	INSTR_CAST,
	INSTR_BIND,
	INSTR_LIST_ASSIGN,
	INSTR_ADD_PAIR,
	// Operations
	INSTR_NEGATE,
	INSTR_ADD,
	INSTR_MULT,
	INSTR_DIV,
	INSTR_NOT,
	INSTR_EQ,
	INSTR_GT,
	INSTR_LT,
	INSTR_AND,
	INSTR_OR,
	INSTR_INDEX,
	// Args
	INSTR_PUSH,
	INSTR_GET,
	INSTR_CALL,
	INSTR_JUMP,
	INSTR_CONDJUMP,
	INSTR_SET_LOOP,
	// Creation of dynamically allocated values
	INSTR_CREATE_FUNCTION,
	INSTR_CREATE_LIST,
	INSTR_CREATE_STRING,
	INSTR_CREATE_DICTIONARY,
};

// :\ Instruction

// : BC_Chunk

// A chunk of bytecode representing a single instruction

struct BC_Chunk {
	enum Instruction instr;
	union {
		Instr_Push instr_push;
		Instr_Get  instr_get;
		Instr_Call instr_call;
		Instr_Jump instr_jump;
		Instr_Condjump instr_condjump;
		Instr_Set_Loop instr_set_loop;
		Instr_Create_Function instr_create_function;
		Instr_Create_String instr_create_string;
	};
	Assoc_Source assoc;
};

BC_Chunk bc_chunk_new_no_args(enum Instruction instr);
BC_Chunk bc_chunk_new_push(Value value);
BC_Chunk bc_chunk_new_get(const char * name);
BC_Chunk bc_chunk_new_call(size_t arg_count);
BC_Chunk bc_chunk_new_jump(int offset);
BC_Chunk bc_chunk_new_condjump(int offset, bool cond);
BC_Chunk bc_chunk_new_set_loop(size_t end_offset);
BC_Chunk bc_chunk_new_create_function(size_t parameter_count, BC_Chunk * bytecode);
BC_Chunk bc_chunk_new_create_string(const char * literal);

void bc_chunk_print(BC_Chunk chunk);

// :\ BC_Chunk

// : Call_Frame

// When a function is called, a new call frame is pushed onto the call
// stack, containing a Variable_Map for every variable that gets used
// in that function.

typedef struct {
	size_t start;
	size_t end;
} Loop;

typedef struct {
	Variable_Map var_map;
	BC_Chunk * bytecode;
	size_t ip;
	Loop * loop_stack;
} Call_Frame;

Call_Frame * call_frame_alloc(BC_Chunk * bytecode);

// :\ Call_Frame

// : Winter_Machine

// The central virtual machine that runs Winter bytecode

typedef struct {
	Call_Frame ** call_stack;
	Value * eval_stack;
	
	bool running;

	size_t cycles_since_collection;
} Winter_Machine;

Winter_Machine * winter_machine_alloc();
void winter_machine_step(Winter_Machine * wm);
void winter_machine_prime(Winter_Machine * wm, BC_Chunk * bytecode);
void winter_machine_garbage_collect(Winter_Machine * wm); // Defined in gc.c... should it be?

// :\ Winter_Machine

// : Function

typedef struct Function {
	//const char ** parameters; // sb
	Value parameter_list;
	Variable_Map closure;
	BC_Chunk * bytecode;
} Function;

// :\ Function
