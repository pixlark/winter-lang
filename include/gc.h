#pragma once

#include "common.h"

// : GC

typedef struct {
	void ** allocations;
} GC;

// TODO(pixlark): I can't think of why this shouldn't be global...
extern GC global_gc;

void global_init();

size_t gc_allocations(GC * gc);
#define global_allocations() gc_allocations(&global_gc)

void * gc_alloc(GC * gc, size_t size);
#define global_alloc(size) gc_alloc(&global_gc, (size))

void * gc_realloc(GC * gc, void * external, size_t new_size);
#define global_realloc(ptr, size) gc_realloc(&global_gc, (ptr), (size))

void gc_modify_refcount(void * ptr, int change);

int32_t gc_get_refcount(void * ptr);

void gc_collect(GC * gc);
#define global_collect() gc_collect(&global_gc)

// :\ GC

