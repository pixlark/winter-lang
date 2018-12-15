#include "stretchy_buffer.h"

void * stb__sbgrowf(void *arr, int increment, int itemsize)
{
	int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
	int min_needed = sb_count(arr) + increment;
	int m = dbl_cur > min_needed ? dbl_cur : min_needed;
	int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
	if (p) {
		if (!arr)
			p[1] = 0;
		p[0] = m;
		return p+2;
	} else {
		assert(0);
	}
}

void * stb__copy(void * arr, size_t elem_size, size_t elem_count)
{
	size_t size = elem_size * elem_count + sizeof(int) * 2;
	void * new_arr = malloc(size);
	memcpy(new_arr, stb__sbraw(arr), size);
	return (void*) ((int*) new_arr + 2);
}
