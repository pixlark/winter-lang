#pragma once

#include "common.h"

// : Struct prototypes

typedef struct Function Function;

// :\ Struct prototypes

// : Value

typedef enum {
	VALUE_NONE,
	VALUE_INTEGER,
	VALUE_FLOAT,
	VALUE_FUNCTION,
} Value_Type;

typedef struct {
	Value_Type type;
	union {
		int _integer;
		float _float;
		Function * _function;
	};
} Value;

// :\ Value

// : Variable_Map

// Maps variable names to pointers to values in memory.

typedef struct {
	size_t size;
	const char ** names;
	Value ** values;
} Variable_Map;

// :\ Variable_Map

// : Instruction

// Each instruction in the enum either has arguments, represented by a
// struct in the BC_Chunk union, or has no arguments, in which case
// there is no associated struct.

typedef struct {
	Value value;
} Instr_Push;

typedef struct {
	const char * name;
} Instr_Bind;

typedef struct {
	const char * name;
} Instr_Get;

enum Instruction {
	// No args
	INSTR_NOP,
	INSTR_NEGATE,
	INSTR_ADD,
	INSTR_RETURN,
	INSTR_PRINT,
	INSTR_POP,
	INSTR_CALL,
	// Args
	INSTR_PUSH,
	INSTR_BIND,
	INSTR_GET,
};

// :\ Instruction

// : BC_Chunk

// A chunk of bytecode representing a single instruction

typedef struct {
	enum Instruction instr;
	union {
		Instr_Push instr_push;
		Instr_Bind instr_bind;
		Instr_Get  instr_get;
	};
} BC_Chunk;

BC_Chunk bc_chunk_new_no_args(enum Instruction instr);
BC_Chunk bc_chunk_new_push(Value value);
BC_Chunk bc_chunk_new_bind(const char * name);
BC_Chunk bc_chunk_new_get(const char * name);

// :\ BC_Chunk

// : Call_Frame

// When a function is called, a new call frame is pushed onto the call
// stack, containing a Variable_Map for every variable that gets used
// in that function.

typedef struct {
	Variable_Map var_map;
	BC_Chunk * bytecode;
	size_t ip;
} Call_Frame;

Call_Frame * call_frame_alloc(BC_Chunk * bytecode);

// :\ Call_Frame

// : Winter_Machine

// The central virtual machine that runs Winter bytecode

typedef struct {
	Variable_Map global_var_map;
	BC_Chunk * bytecode;
	size_t bytecode_len;
	size_t ip;

	Call_Frame ** call_stack;
	Value * eval_stack;
	
	bool running;
} Winter_Machine;

Winter_Machine * winter_machine_alloc();
void winter_machine_step(Winter_Machine * wm);
void winter_machine_prime(Winter_Machine * wm, BC_Chunk * bytecode, size_t len);

// :\ Winter_Machine

// : Function

typedef struct Function {
	const char ** parameters; // sb
	BC_Chunk * bytecode;
} Function;

// :\ Function

void vm_test();
