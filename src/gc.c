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
	int32_t * allocation = malloc(size + ALIGNMENT);
	*allocation = 0; // Zero refcount by default
	sb_push(gc->allocations, (void*) allocation);
	return (void*) ((char*) allocation + ALIGNMENT); // Hide reference count
}

// On external-facing pointer
void gc_modify_refcount(void * ptr, int change)
{
	int32_t * refcount = (int32_t*) ((char*) ptr - ALIGNMENT);
	*refcount += change;
}

int32_t gc_get_refcount(void * ptr)
{
	return *((int32_t*) ((char*) ptr - ALIGNMENT));
}

// On internal-facing pointer
int32_t gc_get_refcount_internal(void * ptr)
{
	return *((int32_t*) ptr);
}

void gc_collect(GC * gc)
{
	void ** new_allocations = NULL;
	// Free all refcount zero or less
	for (int i = 0; i < sb_count(gc->allocations); i++) {
		int32_t refcount = gc_get_refcount_internal(gc->allocations[i]);
		dbprintf("%p refcount: %d\n", gc->allocations[i], refcount);
		if (refcount > 0) {
			sb_push(new_allocations, gc->allocations[i]);
		} else {
			dbprintf("Freeing %p (external: %p)\n", gc->allocations[i], (uint8_t*) gc->allocations[i] + ALIGNMENT);
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

#if 0
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
#endif

// :\ Winter_Machine
