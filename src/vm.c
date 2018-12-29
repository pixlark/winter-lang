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

void variable_map_free(Variable_Map map)
{
	for (int i = 0; i < sb_count(map.names); i++) {
		free((void*) map.names[i]);
	}
	sb_free(map.names);
	sb_free(map.values);
}

Value * variable_map_index(Variable_Map * map, const char * name)
{
	for (int i = 0; i < map->size; i++) {
		// Can't do string intern comparison because if name is a
		// function parameter, it's dynamically allocated. Way to fix
		// this?
		if (strcmp(name, map->names[i]) == 0) {
			return map->values[i];
		}
	}
	return NULL;
}

Value * variable_map_update(Variable_Map * map, const char * name, Value value)
{
	Value * index = variable_map_index(map, name);
	if (index) {
		*index = value;
		return index;
	} else {
		map->size++;
		sb_push(map->names, strdup(name));
		Value * storage = malloc(sizeof(Value));
		*storage = value;
		sb_push(map->values, storage);
		return storage;
	}
}

// note: Does not copy values
Variable_Map variable_map_copy(Variable_Map map)
{
	Variable_Map nmap = variable_map_new();
	nmap.size = map.size;
	nmap.names = NULL;
	for (int i = 0; i < sb_count(map.names); i++) {
		sb_push(nmap.names, strdup(map.names[i]));
	}
	nmap.values = sb_copy(map.values);
	return nmap;
}

void variable_map_print(Variable_Map map)
{
	#if DEBUG_PRINTS
	for (int i = 0; i < sb_count(map.values); i++) {
		value_print(*map.values[i]);
	}
	#endif
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
	// Variable maps are copied into closures etc, so it's fine to free them here
	variable_map_free(frame->var_map);
	// Loop stack can't leave function, so that should get freed
	sb_free(frame->loop_stack);
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

BC_Chunk bc_chunk_new_create_function(size_t parameter_count, BC_Chunk * bytecode)
{
	Instr_Create_Function instr = (Instr_Create_Function) { parameter_count, bytecode };
	return (BC_Chunk) { INSTR_CREATE_FUNCTION, .instr_create_function = instr };
}

BC_Chunk bc_chunk_new_create_string(const char * literal)
{
	Instr_Create_String instr = (Instr_Create_String) { literal };
	return (BC_Chunk) { INSTR_CREATE_STRING, .instr_create_string = instr };
}

BC_Chunk bc_chunk_new_create_type_canon(size_t field_count)
{
	Instr_Create_Type_Canon instr = (Instr_Create_Type_Canon) { field_count };
	return (BC_Chunk) { INSTR_CREATE_TYPE_CANON, .instr_create_type_canon = instr };
}

void bc_chunk_print(BC_Chunk chunk)
{
	const char * instr_names[] = {	
		[INSTR_NOP] = "NOP",
		[INSTR_RETURN] = "RETURN",
		[INSTR_POP] = "POP",
		[INSTR_LOOP_END] = "LOOP_END",
		[INSTR_BREAK] = "BREAK",
		[INSTR_CONTINUE] = "CONTINUE",
		[INSTR_CLOSURE] = "CLOSURE",
		[INSTR_APPEND] = "APPEND",
		[INSTR_CAST] = "CAST",

		[INSTR_NEGATE] = "NEGATE",
		[INSTR_ADD] = "ADD",
		[INSTR_MULT] = "MULT",
		[INSTR_DIV] = "DIV",
		[INSTR_NOT] = "NOT",
		[INSTR_EQ] = "EQ",
		[INSTR_GT] = "GT",
		[INSTR_LT] = "LT",
		[INSTR_AND] = "AND",
		[INSTR_OR] = "OR",
		[INSTR_INDEX] = "INDEX",
		
		[INSTR_PUSH] = "PUSH",
		[INSTR_BIND] = "BIND",
		[INSTR_GET] = "GET",
		[INSTR_CALL] = "CALL",
		[INSTR_JUMP] = "JUMP",
		[INSTR_CONDJUMP] = "CONDJUMP",
		[INSTR_SET_LOOP] = "SET_LOOP",

		[INSTR_CREATE_FUNCTION] = "CREATE_FUNCTION",
		[INSTR_CREATE_LIST] = "CREATE_LIST",
		[INSTR_CREATE_STRING] = "CREATE_STRING",
		[INSTR_CREATE_DICTIONARY] = "CREATE_DICTIONARY",
	};

	#if DEBUG_PRINTS
	printf("%s: ", instr_names[chunk.instr]);
	switch (chunk.instr) {
	case INSTR_PUSH:
		value_print(chunk.instr_push.value);
		break;
	case INSTR_GET:
		printf("%s\n", chunk.instr_get.name);
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
	default:
		printf("\r%s    \n", instr_names[chunk.instr]);
		break;
	}
	#endif
}

// :\ BC_Chunk

// : Value Refcount

// :\ Value Refcount

// : Winter_Machine

Winter_Machine * winter_machine_alloc()
{
	Winter_Machine * wm = malloc(sizeof(Winter_Machine));
	wm->eval_stack = NULL;
	wm->call_stack = NULL;
	sb_push(wm->call_stack, call_frame_alloc(NULL));
	wm->running = false;
	wm->cycles_since_collection = 0;
	return wm;
}

Value winter_machine_pop(Winter_Machine * wm)
{
	internal_assert(sb_count(wm->eval_stack) > 0);
	value_modify_refcount(sb_last(wm->eval_stack), -1);
	return sb_pop(wm->eval_stack);
}

void winter_machine_push(Winter_Machine * wm, Value value)
{
	sb_push(wm->eval_stack, value);
	value_modify_refcount(sb_last(wm->eval_stack), +1);
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

void winter_machine_print_eval_stack(Winter_Machine * wm)
{
	#if DEBUG_PRINTS
	for (int i = sb_count(wm->eval_stack) - 1; i >= 0; i--) {
		value_print(wm->eval_stack[i]);
	}
	#endif
}

void winter_machine_return(Winter_Machine * wm)
{
	// Decrease reference count for every variable in varmap
	Call_Frame * frame = winter_machine_frame(wm);
	for (int i = 0; i < frame->var_map.size; i++) {
		value_modify_refcount(*frame->var_map.values[i], -1);
	}
	// Pop call stack
	winter_machine_pop_call_stack(wm);
}

void winter_machine_step(Winter_Machine * wm)
{
	dbprintf("\n");
	
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
			winter_machine_return(wm);
		}
	}

	if (!wm->running) return;

	BC_Chunk chunk;
	{
		Call_Frame * this_frame = winter_machine_frame(wm);
		chunk = this_frame->bytecode[this_frame->ip++];
	}

	bc_chunk_print(chunk);
	dbprintf("...\n");
	//dbprintf("! ! ! Executing line %d\n", chunk.assoc.line);
	
	switch (chunk.instr) {
		// No args
	case INSTR_NOP:
		break;
	case INSTR_RETURN: {
		if (sb_count(wm->call_stack) == 0) {
			fatal_assoc(chunk.assoc, "Can't return from global scope");
		}
		winter_machine_return(wm);
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
		Loop loop = sb_pop(frame->loop_stack);
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
	case INSTR_CLOSURE: {
		Value value = pop();
		if (value.type != VALUE_FUNCTION) {
			fatal_internal("Tried to close on something that's not a function");
		}
		Function * function = value._function;
		function->closure = variable_map_copy(winter_machine_frame(wm)->var_map);
		push(value);
	} break;
	case INSTR_APPEND: {
		Value to_append = pop();
		Value list = pop();
		value_append(list, to_append, chunk.assoc);
		push(list);
	} break;
	case INSTR_CAST: {
		//Instr_Cast instr = chunk.instr_cast;
		Value type = pop();
		if (type.type != VALUE_TYPE) {
			fatal_assoc(chunk.assoc, "Can't cast to non-type");
		}
		Value to_cast = pop();
		push(value_cast(to_cast, type._type.type, chunk.assoc));
	} break;
	case INSTR_BIND: {
		Value name = pop();
		internal_assert(name.type == VALUE_STRING);
		
		Value value = pop();
		value_modify_refcount(value, 1);
		
		Variable_Map * varmap = &(winter_machine_frame(wm)->var_map);
		variable_map_update(varmap, name._string.contents, value);
	} break;
	case INSTR_INDEX_ASSIGN: {
		Value index = pop();
		Value collection = pop();
		Value value = pop();
		if (collection.type == VALUE_DICTIONARY) {
			// Dictionaries are unique in that a failed lookup will
			// result in adding a new item
			Value * element = value_index_dictionary(collection, index);
			if (element) {
				*element = value;
			} else {
				value_add_pair_dictionary(collection, index, value);
			}
		} else {
			Value * element = value_index(collection, index, chunk.assoc);
			*element = value;
		}
		push(collection);
	} break;
	case INSTR_ADD_PAIR: {
		Value value = pop();
		Value key = pop();
		Value dict = pop();
		value_add_pair(dict, key, value, chunk.assoc);
		push(dict);
	} break;

		// Operations
	case INSTR_NEGATE:
		push(value_negate(pop(), chunk.assoc));
		break;
	case INSTR_ADD: {
		Value b = pop();
		Value a = pop();
		push(value_add(a, b, chunk.assoc));
	} break;
	case INSTR_MULT: {
		Value b = pop();
		Value a = pop();
		push(value_multiply(a, b, chunk.assoc));
	} break;
	case INSTR_DIV: {
		Value b = pop();
		Value a = pop();
		push(value_divide(a, b, chunk.assoc));
	} break;
	case INSTR_NOT:
		push(value_not(pop(), chunk.assoc));
		break;
	case INSTR_EQ: {
		Value b = pop();
		Value a = pop();
		push(value_equal(a, b, chunk.assoc));
	} break;
	case INSTR_GT: {
		Value b = pop();
		Value a = pop();
		push(value_greater_than(a, b, chunk.assoc));
	} break;
	case INSTR_LT: {
		Value b = pop();
		Value a = pop();
		push(value_less_than(a, b, chunk.assoc));
	} break;
	case INSTR_AND: {
		Value b = pop();
		Value a = pop();
		push(value_and(a, b, chunk.assoc));		
	} break;
	case INSTR_OR: {
		Value b = pop();
		Value a = pop();
		push(value_or(a, b, chunk.assoc));
	} break;
	case INSTR_INDEX: {
		Value index = pop();
		Value collection = pop();
		Value element = *value_index(collection, index, chunk.assoc);
		push(element);
	} break;
		// Args
	case INSTR_PUSH: {
		Instr_Push instr = chunk.instr_push;
		push(instr.value);
	} break;
	case INSTR_GET: {
		Instr_Get instr = chunk.instr_get;
		Variable_Map * var_map = &(winter_machine_frame(wm)->var_map);
		Value * var_storage = variable_map_index(var_map, instr.name);
		if (!var_storage) {
			// Resort to looking in global scope
			Variable_Map * global_var_map = &(winter_machine_global_frame(wm)->var_map);
			var_storage = variable_map_index(global_var_map, instr.name);
			if (!var_storage) {
				fatal_assoc(chunk.assoc, "%s not bound", instr.name);
			}
		}
		push(*var_storage);
	} break;
		// TODO(pixlark): Have calls push args in reverse order to simplify logic here
	case INSTR_CALL: {
		Value func_val = pop();
		Instr_Call instr = chunk.instr_call;
		if (func_val.type == VALUE_FUNCTION) {
			Function func = *(func_val._function);
			internal_assert(func.parameter_list.type == VALUE_LIST);
			Winter_List * parameters = func.parameter_list._list;
			if (parameters->size != instr.arg_count) {
				fatal_assoc(chunk.assoc, "Expected %d arguments, got %d", parameters->size, instr.arg_count);
			}
			Call_Frame * frame = call_frame_alloc(func.bytecode);
			// Start off varmap with closure
			frame->var_map = variable_map_copy(func.closure);
			// Push arguments into varmap
			for (int i = parameters->size - 1; i >= 0; i--) {
				Value arg = pop();
				internal_assert(parameters->contents[i].type == VALUE_STRING);
				variable_map_update(&(frame->var_map), parameters->contents[i]._string.contents, arg);
			}
			// Bump refcount for all variables in new varmap
			for (int i = 0; i < frame->var_map.size; i++) {
				value_modify_refcount(*frame->var_map.values[i], 1);
			}
			sb_push(wm->call_stack, frame);	
		} else if (func_val.type == VALUE_BUILTIN) {
			Builtin builtin = func_val._builtin;
			if (builtin_arg_counts[builtin] != -1) {
				if (builtin_arg_counts[builtin] != instr.arg_count) {
					fatal_assoc(chunk.assoc, "Wrong number of arguments to builtin function %s",
								builtin_names[builtin]);
				}
			}
			Value * args = malloc(sizeof(Value) * instr.arg_count);
			for (int i = 0; i < instr.arg_count; i++) {
				args[instr.arg_count - i - 1] = pop();
			}
			Value ret = builtin_functions[builtin](args, instr.arg_count, chunk.assoc);
			free(args);
			push(ret);
		} else if (func_val.type == VALUE_TYPE) {
			if (func_val._type.type != VALUE_RECORD) {
				fatal_assoc(chunk.assoc, "Can't construct non-record");
			}
			Value record = value_new_record(func_val._type.canon);
			if (instr.arg_count > func_val._type.canon->fields._list->size) {
				fatal_assoc(chunk.assoc, "Too many arguments for record initialization");
			}
			for (int i = instr.arg_count - 1; i >= 0; i--) {
				Value value = pop();
				Value s = value_cast(value, VALUE_STRING, (Assoc_Source) {0});
				Value * spot = value_index_dictionary(record._record->field_dict,
													  func_val._type.canon->fields._list->contents[i]);
				*spot = value;
			}
			push(record);
		} else {
			fatal_assoc(chunk.assoc, "Type not callable");
		}
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
		// Creation of dynamically allocated values
	case INSTR_CREATE_FUNCTION: {
		Instr_Create_Function instr = chunk.instr_create_function;
		Value parameter_list = value_new_list();
		for (int i = 0; i < instr.parameter_count; i++) {
			Value parameter = pop();
			internal_assert(parameter.type == VALUE_STRING);
			value_append_list(parameter_list, parameter);
		}
		Value func = value_new_function(instr.bytecode);
		func._function->parameter_list = parameter_list;
		push(func);
	} break;
	case INSTR_CREATE_LIST: {
		Value list = value_new_list();
		push(list);
	} break;
	case INSTR_CREATE_STRING: {
		Instr_Create_String instr = chunk.instr_create_string;
		Value string = value_new_string(instr.literal);
		push(string);
	} break;
	case INSTR_CREATE_DICTIONARY: {
		Value dict = value_new_dictionary();
		push(dict);
	} break;
	case INSTR_CREATE_TYPE_CANON: {
		Instr_Create_Type_Canon instr = chunk.instr_create_type_canon;
		Value fields = value_new_list();
		for (int i = 0; i < instr.field_count; i++) {
			Value field_name = pop();
			internal_assert(field_name.type == VALUE_STRING);
			value_append_list(fields, field_name);
		}
		Winter_Canon * canon = global_alloc(sizeof(Winter_Canon));
		canon->fields = fields;
		Value type_value = value_new_type(VALUE_RECORD);
		type_value._type.canon = canon;
		push(type_value);
	} break;
	default:
		fatal_internal("Nonexistent instruction reached winter_machine_step()");
	}

	dbprintf("Frame %d\n", sb_count(wm->call_stack) - 1);
	dbprintf("-- Eval Stack --\n");
	winter_machine_print_eval_stack(wm);
	
	dbprintf("-- Var Map --\n");
	variable_map_print(sb_last(wm->call_stack)->var_map);
	
	// Garbage collection
	if (wm->cycles_since_collection >= 0) {
		dbprintf("-- Collecting --\n");
		global_collect();
		wm->cycles_since_collection = 0;
	} else {
		wm->cycles_since_collection += 1;
	}

	dbprintf("\n");
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
