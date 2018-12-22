#include "gc.h"

// : GC

#define ALIGNMENT 8

GC global_gc;

void global_init()
{
	global_gc = (GC) { NULL };
}

size_t gc_allocations(GC * gc)
{
	return sb_count(gc->allocations);
}

void * gc_alloc(GC * gc, size_t size)
{
	uint8_t * allocation = malloc(size + ALIGNMENT);
	sb_push(gc->allocations, allocation);
	return (void*) (allocation + ALIGNMENT); // Hide mark bit
}

void gc_unmark_all(GC * gc)
{
	for (int i = 0; i < sb_count(gc->allocations); i++) {
		((uint8_t*) gc->allocations[i])[0] = 0;
	}
}

void gc_mark_external(void * external_ptr)
{
	// Turn external into internal
	uint8_t * internal_ptr = (uint8_t*) external_ptr - ALIGNMENT;
	// Mark
	internal_ptr[0] = 1;
}

bool is_marked(void * ptr)
{
	uint8_t * u8_ptr = (uint8_t*) ptr;
	return u8_ptr[0];
}

bool is_marked_external(void * ptr)
{
	return is_marked((uint8_t*) ptr - ALIGNMENT);
}

void gc_collect(GC * gc)
{
	void ** new_allocations = NULL;
	// Free all unmarked allocations
	for (int i = 0; i < sb_count(gc->allocations); i++) {
		if (is_marked(gc->allocations[i])) {
			sb_push(new_allocations, gc->allocations[i]);
		} else {
			printf("Freeing %p (external: %p)\n", gc->allocations[i], (uint8_t*) gc->allocations[i] + ALIGNMENT);
			free(gc->allocations[i]);
		}
	}
	sb_free(gc->allocations);
	gc->allocations = new_allocations;
}

// :\ GC

// : Winter_Machine

#include "vm.h"
#include "value.h"

void mark_var_map(Variable_Map var_map);
void mark_value(Value value);

// Find all the times we push a value to the stack, and mark those values
void mark_bytecode(BC_Chunk * bytecode)
{
	for (int i = 0; i < sb_count(bytecode); i++) {
		BC_Chunk chunk = bytecode[i];
		if (chunk.instr == INSTR_PUSH) {
			Value value = chunk.instr_push.value;
			mark_value(value);
		}
	}
}

void mark_value(Value value)
{
	switch (value.type) {
	case VALUE_NONE:
		break;
	case VALUE_INTEGER:
		break;
	case VALUE_FLOAT:
		break;
	case VALUE_BOOL:
		break;
	case VALUE_STRING:
		// Not dynamically allocated for now
		break;
	case VALUE_FUNCTION: {
		if (!is_marked_external(value._function)) {
			gc_mark_external(value._function);
			mark_var_map(value._function->closure);
			mark_bytecode(value._function->bytecode);
		}
	} break;
	default:
		fatal_internal("Switch case for mark_value not complete");
		break;
	}
}

void mark_var_map(Variable_Map var_map)
{
	for (int i = 0; i < sb_count(var_map.values); i++) {
		//printf("Marked ");
		//value_print(*var_map.values[i]);
		mark_value(*var_map.values[i]);
	}
}

void winter_machine_garbage_collect(Winter_Machine * wm)
{
	//printf("--- Collecting ---\n");
	global_unmark_all();
	// Mark all variables on the eval stack
	for (int i = 0; i < sb_count(wm->eval_stack); i++) {
		Value value = wm->eval_stack[i];
		mark_value(value);
	}
	// Mark all bound variables
	for (int i = 0; i < sb_count(wm->call_stack); i++) {
		Call_Frame * frame = wm->call_stack[i];
		mark_var_map(frame->var_map);
	}
	global_collect();
}

// :\ Winter_Machine
