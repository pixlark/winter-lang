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
