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

void * gc_alloc(GC * gc, size_t sizen);
#define global_alloc(size) gc_alloc(&global_gc, (size))

void gc_unmark_all(GC * gc);
#define global_unmark_all() gc_unmark_all(&global_gc)

void gc_mark_external(void * external_ptr);

void gc_collect(GC * gc);
#define global_collect() gc_collect(&global_gc)

// :\ GC

