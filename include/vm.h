#pragma once

#include "common.h"

// : Variable_Map

// Maps variable names to pointers to values in memory.

typedef struct {
	size_t size;
	const char ** names;
	Value ** values;
} Variable_Map;

// :\ Variable_Map

// : Call_Frame

// When a function is called, a new call frame is pushed onto the call
// stack, containing a Variable_Map for every variable that gets used
// in that function.

typedef struct {
	Variable_Map var_map;
} Call_Frame;

Call_Frame * call_frame_alloc();

// :\ Call_Frame

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

// : Winter_Machine

// The central virtual machine that runs Winter bytecode

// To set up a Winter_Machine:
//  1. Allocate a machine with winter_machine_alloc()
//  2. Push something onto the call_stack
//  3. Put some bytecode into wm->bytecode
//  4. Set wm->running to true
//  5. Run with winter_machine_step until wm->running turns false
typedef struct {
	BC_Chunk * bytecode;
	size_t ip;
	Value * eval_stack;
	Call_Frame ** call_stack;
	bool running;
} Winter_Machine;

Winter_Machine * winter_machine_alloc();
void winter_machine_step(Winter_Machine * wm);
void winter_machine_reset(Winter_Machine * wm);

// :\ Winter_Machine

void vm_test();
