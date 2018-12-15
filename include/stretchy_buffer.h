/* This file is adapted from https://gihub.com/nothings/stb under the public domain
 */

// IMPORTANT NOTE: This operates on a high-water-mark system. The
// array will grow with items added, but WILL NOT shrink with items
// removed.

#pragma once

#define sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define sb_last(a)         ((a)[stb__sbn(a)-1])
#define sb_pop(a)          ((a)[--stb__sbn(a)])
#define sb_copy(a)         ((a) ? stb__copy((void*)(a), sizeof(sb_last(a)), sb_count(a)) : 0)

// Get pointer to before-pointer information
#define stb__sbraw(a) ((int *) (a) - 2)
// Get capacity of list
#define stb__sbm(a)   stb__sbraw(a)[0]
// Get amount in list
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void * stb__sbgrowf(void *arr, int increment, int itemsize);
void * stb__copy(void * arr, size_t elem_size, size_t elem_count);
