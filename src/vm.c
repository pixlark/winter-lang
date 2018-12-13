#include <string.h>

#include "common.h"
#include "vm.h"

// : Value

// Holds values in memory

typedef enum {
	VALUE_INTEGER,
} Value_Type;

typedef struct {
	Value_Type type;
	union {
		int _integer;
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

Variable_Map variable_map_new()
{
	Variable_Map map = (Variable_Map) {
		.size = 0,
		.names = NULL,
		.values = NULL,
	};
	return map;
}

void variable_map_add(Variable_Map * map, const char * name, Value * value)
{
	map->size++;
	sb_push(map->names, name);
	sb_push(map->values, value);
}

Value * variable_map_index(Variable_Map * map, const char * name)
{
	for (int i = 0; i < map->size; i++) {
		// TODO(pixlark): String interning to make this way faster
		if (strcmp(name, map->names[i]) == 0) {
			return map->values[i];
		}
	}
	return NULL;
}

bool variable_map_test()
{
	Variable_Map map = variable_map_new();
	
	Value val;
	val.type = VALUE_INTEGER;
	val._integer = 10;
	
	variable_map_add(&map, "test", &val);

	Value * val_p = variable_map_index(&map, "test");
	assert(val_p->type == VALUE_INTEGER);
	assert(val_p->_integer == 10);
}

// :\ Variable_Map

// : Call_Frame

// When a function is called, a new call frame is pushed onto the call
// stack, containing a Variable_Map for every variable that gets used
// in that function.

typedef struct {
	Variable_Map var_map;
} Call_Frame;

Call_Frame * call_frame_alloc()
{
	Call_Frame * frame = malloc(sizeof(Call_Frame));
	frame->var_map = variable_map_new();
	return frame;
}

void call_frame_test()
{
	Call_Frame * frame = call_frame_alloc();
	assert(frame->var_map.size == 0);
}

// :\ Call_Frame

// : BC_Chunk

// A chunk of bytecode representing a single "instruction"

enum Instruction {
	INSTR_NOP,
	INSTR_HALT,
};

typedef struct {
	enum Instruction instr;
} BC_Chunk;

// :\ BC_Chunk

// : Winter_Machine

// The central virtual machine that runs Winter bytecode

typedef struct {
	BC_Chunk * bytecode;
	size_t ip;
	Call_Frame ** call_stack;
	bool running;
} Winter_Machine;

Winter_Machine * winter_machine_alloc()
{
	Winter_Machine * wm = malloc(sizeof(Winter_Machine));
	wm->bytecode = NULL;
	wm->ip = 0;
	wm->call_stack = NULL;
	wm->running = false;
	return wm;
}

void winter_machine_step(Winter_Machine * wm)
{
	assert(wm->bytecode);
	BC_Chunk chunk = wm->bytecode[wm->ip++];
	switch (chunk.instr) {
	case INSTR_NOP:
		break;
	case INSTR_HALT:
		wm->running = false;
		break;
	}
}

void winter_machine_test()
{
	Winter_Machine * wm = winter_machine_alloc();
	BC_Chunk * bytecode = NULL;
	sb_push(bytecode, ((BC_Chunk) { INSTR_NOP }));
	sb_push(bytecode, ((BC_Chunk) { INSTR_NOP }));
	sb_push(bytecode, ((BC_Chunk) { INSTR_HALT }));
	wm->bytecode = bytecode;
	wm->running = true;
	while (wm->running) {
		winter_machine_step(wm);
	}
}

// :\ Winter_Machine

void vm_test()
{
	variable_map_test();
	call_frame_test();
	winter_machine_test();
}
