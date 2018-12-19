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

Value * variable_map_index(Variable_Map * map, const char * name)
{
	for (int i = 0; i < map->size; i++) {
		// We can do this because names are interned in the lexer
		if (name == map->names[i]) {
			return map->values[i];
		}
	}
	return NULL;
}

void variable_map_update(Variable_Map * map, const char * name, Value value)
{
	Value * index = variable_map_index(map, name);
	if (index) {
		*index = value;
	} else {
		map->size++;
		sb_push(map->names, name);
		Value * storage = malloc(sizeof(Value));
		*storage = value;
		sb_push(map->values, storage);
	}
}

// :\ Variable_Map

// : Call_Frame

Call_Frame * call_frame_alloc(BC_Chunk * bytecode)
{
	Call_Frame * frame = malloc(sizeof(Call_Frame));
	frame->var_map = variable_map_new();
	frame->bytecode = bytecode;
	frame->ip = 0;
	frame->loop_stack = NULL;
	return frame;
}

void call_frame_free(Call_Frame * frame)
{
	// What do we need to deallocate? Anything which can't be used by
	// the program anymore...

	/* Because functions are first-class, we have to treat them like
	   any other kind of reference object, which means that we can't
	   clean up their data deterministically like this. The eventual
	   garbage collector will have to deal with it instead.
	   -Paul T. Mon Dec 17 19:37:57 2018 */
	/*
	for (int i = 0; i < frame->var_map.size; i++) {
		Value * storage = frame->var_map.values[i];
		if (storage->type == VALUE_FUNCTION) {
			sb_free(storage->_function->bytecode);
			sb_free(storage->_function->parameters);
			free(storage->_function);
		}
		}*/
	
	// 2. Variables that have fallen out of scope should have their space deallocated
	for (int i = 0; i < frame->var_map.size; i++) {
		free(frame->var_map.values[i]);
	}

	// 3. The frame itself should be deallocated
	free(frame);
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

BC_Chunk bc_chunk_new_call(size_t arg_count)
{
	return (BC_Chunk) { INSTR_CALL, .instr_call = (Instr_Call) { arg_count } };
}

BC_Chunk bc_chunk_new_jump(int offset)
{
	return (BC_Chunk) { INSTR_JUMP, .instr_jump = (Instr_Jump) { offset } };
}

BC_Chunk bc_chunk_new_condjump(int offset, bool cond)
{
	return (BC_Chunk) { INSTR_CONDJUMP,
			.instr_condjump = (Instr_Condjump) { offset, cond } };
}

BC_Chunk bc_chunk_new_set_loop(size_t end_offset)
{
	return (BC_Chunk) { INSTR_SET_LOOP,
			.instr_set_loop = (Instr_Set_Loop) { end_offset } };
}

BC_Chunk bc_chunk_new_cast(Value_Type type)
{
	return (BC_Chunk) { INSTR_CAST, .instr_cast = (Instr_Cast) { type } };
}

void bc_chunk_print(BC_Chunk chunk)
{
	const char * instr_names[] = {	
		[INSTR_NOP] = "NOP",
		[INSTR_RETURN] = "RETURN",
		[INSTR_PRINT] = "PRINT",
		[INSTR_POP] = "POP",

		[INSTR_NEGATE] = "NEGATE",
		[INSTR_ADD] = "ADD",
		[INSTR_NOT] = "NOT",
		[INSTR_EQ] = "EQ",
		[INSTR_GT] = "GT",
		[INSTR_LT] = "LT",
		
		[INSTR_PUSH] = "PUSH",
		[INSTR_BIND] = "BIND",
		[INSTR_GET] = "GET",
		[INSTR_CALL] = "CALL",
		[INSTR_JUMP] = "JUMP",
		[INSTR_CONDJUMP] = "CONDJUMP",
	};
	
	if (chunk.instr < INSTR_PUSH) {
		printf("%s\n", instr_names[chunk.instr]);
		return;
	}
	printf("%s: ", instr_names[chunk.instr]);
	switch (chunk.instr) {
	case INSTR_PUSH:
		value_print(chunk.instr_push.value);
		break;
	case INSTR_BIND:
	case INSTR_GET:
		printf("%s\n", chunk.instr_bind.name);
		break;
	case INSTR_CALL:
		printf("%d\n", chunk.instr_call.arg_count);
		break;
	case INSTR_JUMP:
		printf("%d\n", chunk.instr_jump.jump_offset);
		break;
	case INSTR_CONDJUMP:
		printf("%d if %s\n", chunk.instr_condjump.jump_offset,
			   chunk.instr_condjump.cond ? "true" : "false");
		break;
	}
}

// :\ BC_Chunk

// : Winter_Machine

Winter_Machine * winter_machine_alloc()
{
	Winter_Machine * wm = malloc(sizeof(Winter_Machine));
	/*
	wm->global_var_map = variable_map_new();
	wm->bytecode = NULL;
	wm->ip = 0;*/
	wm->eval_stack = NULL;
	wm->call_stack = NULL;
	sb_push(wm->call_stack, call_frame_alloc(NULL));
	wm->running = false;
	return wm;
}

Value winter_machine_pop(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->eval_stack) > 0);
	return sb_pop(wm->eval_stack);
}

void winter_machine_push(Winter_Machine * wm, Value value)
{
	sb_push(wm->eval_stack, value);
}

#define pop() winter_machine_pop(wm)
#define push(x) winter_machine_push(wm, x)

bool winter_machine_reached_end(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->call_stack) > 0);
	Call_Frame * frame = sb_last(wm->call_stack);
	return frame->ip >= sb_count(frame->bytecode);
}

void winter_machine_pop_call_stack(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->call_stack) > 0);
	Call_Frame * frame = sb_pop(wm->call_stack);
	call_frame_free(frame);
}

Call_Frame * winter_machine_global_frame(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->call_stack) > 0);
	return wm->call_stack[0];
}

Call_Frame * winter_machine_frame(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->call_stack) > 0);
	return sb_last(wm->call_stack);
}

void winter_machine_step(Winter_Machine * wm)
{
	if (winter_machine_reached_end(wm)) {
		internal_assert(sb_count(wm->call_stack) > 0);
		if (sb_count(wm->call_stack) == 1) {
			// Not in function
			// Reached end of provided statement
			wm->running = false;
		} else {
			// In function
			// Inferred return, return and push none to the eval stack
			push(value_none());
			winter_machine_pop_call_stack(wm);
		}
	}

	if (!wm->running) return;

	BC_Chunk chunk;
	{
		Call_Frame * this_frame = winter_machine_frame(wm);
		chunk = this_frame->bytecode[this_frame->ip++];
	}
	
	switch (chunk.instr) {
		// No args
	case INSTR_NOP:
		break;
	case INSTR_RETURN: {
		if (sb_count(wm->call_stack) == 0) {
			fatal_assoc(chunk.assoc, "Can't return from global scope");
		}
		winter_machine_pop_call_stack(wm);
	} break;
	case INSTR_PRINT: {
		value_print(pop());
	} break;
	case INSTR_POP:
		pop();
		break;
	case INSTR_LOOP_END: {
		Call_Frame * frame = winter_machine_frame(wm);
		if (sb_count(frame->loop_stack) == 0) {
			fatal_internal("LOOP_END instruction executed outside of loop");
		}
		Loop loop = sb_last(frame->loop_stack);
		frame->ip = loop.start;
	} break;
	case INSTR_BREAK: {
		Call_Frame * frame = winter_machine_frame(wm);
		if (sb_count(frame->loop_stack) == 0) {
			fatal_assoc(chunk.assoc, "Can't use break in non-loop");
		}
		Loop loop = sb_last(frame->loop_stack);
		frame->ip = loop.end;
	} break;
	case INSTR_CONTINUE: {
		Call_Frame * frame = winter_machine_frame(wm);
		if (sb_count(frame->loop_stack) == 0) {
			fatal_assoc(chunk.assoc, "Can't use continue in non-loop");
		}
		Loop loop = sb_last(frame->loop_stack);
		frame->ip = loop.start;
	} break;

		// Operations
	case INSTR_NEGATE:
		push(value_negate(pop()));
		break;
	case INSTR_ADD: {
		Value b = pop();
		Value a = pop();
		push(value_add(a, b));
	} break;
	case INSTR_MULT: {
		Value b = pop();
		Value a = pop();
		push(value_multiply(a, b));
	} break;
	case INSTR_DIV: {
		Value b = pop();
		Value a = pop();
		push(value_divide(a, b));
	} break;
	case INSTR_NOT:
		push(value_not(pop()));
		break;
	case INSTR_EQ: {
		Value b = pop();
		Value a = pop();
		push(value_equal(a, b));
	} break;
	case INSTR_GT: {
		Value b = pop();
		Value a = pop();
		push(value_greater_than(a, b));
	} break;
	case INSTR_LT: {
		Value b = pop();
		Value a = pop();
		push(value_less_than(a, b));
	} break;
	case INSTR_AND: {
		Value b = pop();
		Value a = pop();
		push(value_and(a, b));		
	} break;
	case INSTR_OR: {
		Value b = pop();
		Value a = pop();
		push(value_or(a, b));
	} break;
		// Args
	case INSTR_PUSH: {
		Instr_Push instr = chunk.instr_push;
		sb_push(wm->eval_stack, instr.value);
	} break;
	case INSTR_BIND: {
		Instr_Bind instr = chunk.instr_bind;
		Variable_Map * varmap = &(winter_machine_frame(wm)->var_map);
		Value value = pop();
		variable_map_update(varmap, instr.name, value);
	} break;
	case INSTR_GET: {
		Instr_Get instr = chunk.instr_get;
		Variable_Map * varmap = &(winter_machine_frame(wm)->var_map);
		Value * var_storage = variable_map_index(varmap, instr.name);
		if (!var_storage) {
			// Resort to looking in global scope
			Variable_Map * global_varmap = &(winter_machine_global_frame(wm)->var_map);
			var_storage = variable_map_index(global_varmap, instr.name);
			if (!var_storage) {
				fatal_assoc(chunk.assoc, "%s not bound", instr.name);
			}
		}
		push(*var_storage);
	} break;
	case INSTR_CALL: {
		Value func_val = pop();
		if (func_val.type != VALUE_FUNCTION) {
			fatal_assoc(chunk.assoc, "Type not callable");
		}
		Function func = *(func_val._function);
		Instr_Call instr = chunk.instr_call;
		if (sb_count(func.parameters) != instr.arg_count) {
			fatal_assoc(chunk.assoc, "Wrong number of arguments to function");
		}
		Call_Frame * frame = call_frame_alloc(func.bytecode);
		// Push arguments into varmap
		for (int i = sb_count(func.parameters) - 1; i >= 0; i--) {
			Value arg = pop();
			variable_map_update(&(frame->var_map), func.parameters[i], arg);
		}
		// If the function is non-anonymous, push function itself into varmap (recursion)
		if (func.name) {
			variable_map_update(&(frame->var_map), func.name, func_val);
		}
		sb_push(wm->call_stack, frame);
	} break;
	case INSTR_JUMP: {
		Instr_Jump instr = chunk.instr_jump;
		winter_machine_frame(wm)->ip += instr.jump_offset;
	} break;
	case INSTR_CONDJUMP: {
		Instr_Condjump instr = chunk.instr_condjump;
		Value condition = pop();
		if (condition.type != VALUE_BOOL) {
			fatal_assoc(chunk.assoc, "If condition must be bool");
		}
		if (condition._bool == instr.cond) {
			winter_machine_frame(wm)->ip += instr.jump_offset;
		}
	} break;
	case INSTR_SET_LOOP: {
		Instr_Set_Loop instr = chunk.instr_set_loop;
		Call_Frame * frame = winter_machine_frame(wm);
		Loop new_loop = (Loop) { frame->ip, frame->ip + instr.end_offset };
		sb_push(frame->loop_stack, new_loop);
	} break;
	case INSTR_CAST: {
		Instr_Cast instr = chunk.instr_cast;
		Value to_cast = pop();
		push(value_cast(to_cast, instr.type));
	} break;
	default:
		fatal_internal("Nonexistent instruction reached winter_machine_step()");
	}
}

void winter_machine_prime(Winter_Machine * wm, BC_Chunk * bytecode)
{
	internal_assert(sb_count(wm->call_stack) == 1);
	Call_Frame * base_frame = sb_last(wm->call_stack);
	base_frame->bytecode = bytecode;
	base_frame->ip = 0;
	wm->running = true;
}

// :\ Winter_Machine
