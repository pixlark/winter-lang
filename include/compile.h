#pragma once

#include "ast.h"
#include "vm.h"

typedef struct {
	BC_Chunk * bytecode;
} Compiler;

void compile_statement(Compiler * compiler, Stmt * stmt);
