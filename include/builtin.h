#pragma once

#include <stdlib.h>

#include "value.h"
#include "error.h"

// We have to do this because otherwise C gets confused about what
// size Builtin as an enum is, and it won't let use use Builtin inside
// of Value... sigh
typedef size_t Builtin;
//

extern const char * builtin_names[];
extern int builtin_arg_counts[];
extern Value (*builtin_functions[])(Value*, size_t, Assoc_Source);

enum {
	BUILTIN_PRINT,
	BUILTIN_ASSERT,
	NUM_BUILTINS,
};
