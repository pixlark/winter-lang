#include <string.h>

#include "common.h"
#include "value.h"
#include "vm.h"

// : Variable_Map

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

Call_Frame * call_frame_alloc()
{
	Call_Frame * frame = malloc(sizeof(Call_Frame));
	frame->var_map = variable_map_new();
	frame->bytecode = NULL;
	frame->ip = 0;
	return frame;
}

void call_frame_test()
{
	Call_Frame * frame = call_frame_alloc();
	assert(frame->var_map.size == 0);
}

// :\ Call_Frame

// : BC_Chunk

// Convenience functions

BC_Chunk bc_chunk_new_no_args(enum Instruction instr)
{
	return (BC_Chunk) { instr };
}

BC_Chunk bc_chunk_new_push(Value value)
{
	return (BC_Chunk) { INSTR_PUSH, .instr_push = (Instr_Push) { value } };
}

BC_Chunk bc_chunk_new_bind(const char * name)
{
	return (BC_Chunk) { INSTR_BIND, .instr_bind = (Instr_Bind) { name } };
}

BC_Chunk bc_chunk_new_get(const char * name)
{
	return (BC_Chunk) { INSTR_GET, .instr_get = (Instr_Get) { name } };
}

// :\ BC_Chunk

// : Winter_Machine

Winter_Machine * winter_machine_alloc()
{
	Winter_Machine * wm = malloc(sizeof(Winter_Machine));
	wm->bytecode = NULL;
	wm->ip = 0;
	wm->eval_stack = NULL;
	wm->call_stack = NULL;
	wm->running = false;
	return wm;
}

Value winter_machine_pop(Winter_Machine * wm)
{
	return sb_pop(wm->eval_stack);
}

void winter_machine_push(Winter_Machine * wm, Value value)
{
	sb_push(wm->eval_stack, value);
}

#define pop() winter_machine_pop(wm)
#define push(x) winter_machine_push(wm, x)

Variable_Map * winter_machine_varmap(Winter_Machine * wm)
{
	if (sb_count(wm->call_stack) == 0) {
		return &(wm->global_var_map);
	} else {
		return &(sb_last(wm->call_stack)->var_map);
	}
}

void winter_machine_step(Winter_Machine * wm)
{
	if (wm->ip >= wm->bytecode_len) wm->running = false;
	if (!wm->running) return;

	assert(wm->bytecode);
	BC_Chunk chunk = wm->bytecode[wm->ip++];
	
	switch (chunk.instr) {
	case INSTR_NOP:
		break;
	case INSTR_NEGATE: {
		push(value_negate(pop()));
	} break;
	case INSTR_ADD: {
		Value b = pop();
		Value a = pop();
		push(value_add(a, b));
	} break;
	case INSTR_PUSH: {
		Instr_Push instr = chunk.instr_push;
		sb_push(wm->eval_stack, instr.value);
	} break;
	case INSTR_RETURN: {
		sb_pop(wm->call_stack);
	} break;
	case INSTR_BIND: {
		Instr_Bind instr = chunk.instr_bind;
		Variable_Map * varmap = winter_machine_varmap(wm);
		Value * var_storage = malloc(sizeof(Value));
		*var_storage = pop();
		variable_map_add(varmap, instr.name, var_storage);
	} break;
	case INSTR_GET: {
		Instr_Get instr = chunk.instr_get;
		Variable_Map * varmap = winter_machine_varmap(wm);
		Value * var_storage = variable_map_index(varmap, instr.name);
		push(*var_storage);
	} break;
	case INSTR_PRINT: {
		value_print(pop());
	} break;
	case INSTR_POP:
		pop();
		break;
	default:
		fatal_internal("Nonexistent instruction reached winter_machine_step()");
	}
}

void winter_machine_prime(Winter_Machine * wm, BC_Chunk * bytecode, size_t len)
{
	wm->bytecode = bytecode;
	wm->bytecode_len = len;
	wm->ip = 0;
	wm->running = true;
}

void winter_machine_test()
{
	Winter_Machine * wm = winter_machine_alloc();
	sb_push(wm->call_stack, call_frame_alloc());

	#define pb(x) sb_push(bytecode, x)
	BC_Chunk * bytecode = NULL;
	pb(bc_chunk_new_no_args(INSTR_NOP));
	pb(bc_chunk_new_push(value_new_integer(15)));
	pb(bc_chunk_new_push(value_new_integer(4)));
	pb(bc_chunk_new_no_args(INSTR_ADD));
	pb(bc_chunk_new_no_args(INSTR_NEGATE));
	pb(bc_chunk_new_bind("my_var"));
	pb(bc_chunk_new_get("my_var"));
	pb(bc_chunk_new_no_args(INSTR_PRINT));
	pb(bc_chunk_new_no_args(INSTR_RETURN));
	#undef pb

	winter_machine_prime(wm, bytecode, sb_count(bytecode));
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
