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

Call_Frame * call_frame_alloc(BC_Chunk * bytecode)
{
	Call_Frame * frame = malloc(sizeof(Call_Frame));
	frame->var_map = variable_map_new();
	frame->bytecode = bytecode;
	frame->ip = 0;
	return frame;
}

void call_frame_free(Call_Frame * frame)
{
	// What do we need to deallocate? Anything which can't be used by
	// the program anymore...

	// 1. Functions that have fallen out of scope should become deallocated
	for (int i = 0; i < frame->var_map.size; i++) {
		Value * storage = frame->var_map.values[i];
		if (storage->type == VALUE_FUNCTION) {
			sb_free(storage->_function->bytecode);
			sb_free(storage->_function->parameters);
			free(storage->_function);
		}
	}
	
	// 2. Variables that have fallen out of scope should have their space deallocated
	for (int i = 0; i < frame->var_map.size; i++) {
		free(frame->var_map.values[i]);
	}

	// 3. The frame itself should be deallocated
	free(frame);
}

void call_frame_test()
{
	Call_Frame * frame = call_frame_alloc(NULL);
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
	wm->global_var_map = variable_map_new();
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

BC_Chunk winter_machine_advance_bytecode(Winter_Machine * wm)
{
	if (sb_count(wm->call_stack) == 0) {
		return wm->bytecode[wm->ip++];
	} else {
		Call_Frame * frame = sb_last(wm->call_stack);
		return frame->bytecode[frame->ip++];
	}
}

bool winter_machine_reached_end(Winter_Machine * wm)
{
	if (sb_count(wm->call_stack) == 0) {
		return wm->ip >= wm->bytecode_len;
	} else {
		Call_Frame * frame = sb_last(wm->call_stack);
		return frame->ip >= sb_count(frame->bytecode);
	}
}

void winter_machine_pop_call_stack(Winter_Machine * wm)
{
	Call_Frame * frame = sb_pop(wm->call_stack);
	call_frame_free(frame);
}

void winter_machine_step(Winter_Machine * wm)
{
	if (winter_machine_reached_end(wm)) {
		if (sb_count(wm->call_stack) == 0) wm->running = false;
		else {
			// Inferred return, return and push None to the eval stack
			push(value_none());
			winter_machine_pop_call_stack(wm);
		}
	}

	if (!wm->running) return;

	assert(wm->bytecode);
	BC_Chunk chunk = winter_machine_advance_bytecode(wm);
	
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
		if (sb_count(wm->call_stack) == 0) {
			fatal("Can't return from global scope");
		}
		winter_machine_pop_call_stack(wm);
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
		if (!var_storage) {
			fatal("%s not bound", instr.name);
		}
		push(*var_storage);
	} break;
	case INSTR_PRINT: {
		value_print(pop());
	} break;
	case INSTR_POP:
		pop();
		break;
	case INSTR_CALL: {
		Value func_val = pop();
		if (func_val.type != VALUE_FUNCTION) {
			fatal("Type not callable");
		}
		Function func = *(func_val._function);
		Call_Frame * frame = call_frame_alloc(func.bytecode);
		for (int i = func.parameter_count - 1; i >= 0; i--) {
			Value * arg_storage = malloc(sizeof(Value));
			*arg_storage = pop();
			variable_map_add(&(frame->var_map), func.parameters[i], arg_storage);
		}
		sb_push(wm->call_stack, frame);
	} break;
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

	Value function;
	{
		const char * parameters[2] = {
			"a", "b",
		};
		BC_Chunk * bytecode = NULL;
		#define pb(x) sb_push(bytecode, x)
		pb(bc_chunk_new_push(value_new_integer(101)));
		pb(bc_chunk_new_no_args(INSTR_PRINT));
		pb(bc_chunk_new_no_args(INSTR_RETURN));
		#undef pb
		function = value_new_function(parameters, 2, bytecode);
	}
	
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
	pb(bc_chunk_new_push(function));
	pb(bc_chunk_new_no_args(INSTR_CALL));
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
