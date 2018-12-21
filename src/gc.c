#include "gc.h"

#define ALIGNMENT 8

GC global_gc;

void global_init()
{
	global_gc = (GC) { NULL };
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
