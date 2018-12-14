#include "compile.h"

#include "common.h"

#define P(x) sb_push(compiler->bytecode, (x))

void compile_operator(Compiler * compiler, Operator operator)
{
	switch (operator) {
	case OP_NEGATE:
		P(bc_chunk_new_no_args(INSTR_NEGATE));
		break;
	case OP_ADD:
		P(bc_chunk_new_no_args(INSTR_ADD));
		break;
	default:
		assert(false); // TODO(pixlark): Really need some proper error handling here
	}
}

void compile_expression(Compiler * compiler, Expr * expr)
{
	switch (expr->type) {
	case EXPR_ATOM:
		P(bc_chunk_new_push(expr->atom.value));
		break;
	case EXPR_VAR:
		P(bc_chunk_new_get(expr->var.name));
		break;
	case EXPR_UNARY: {
		compile_expression(compiler, expr->unary.operand);
		compile_operator(compiler, expr->unary.operator);
	} break;
	case EXPR_BINARY: {
		compile_expression(compiler, expr->binary.left);
		compile_expression(compiler, expr->binary.right);
		compile_operator(compiler, expr->binary.operator);
	} break;
	}
}

void compile_statement(Compiler * compiler, Stmt * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		compile_expression(compiler, stmt->expr.expr);
		P(bc_chunk_new_no_args(INSTR_POP));
		break;
	case STMT_ASSIGN:
		compile_expression(compiler, stmt->assign.expr);
		P(bc_chunk_new_bind(stmt->assign.name));
		break;
	case STMT_PRINT:
		compile_expression(compiler, stmt->print.expr);
		P(bc_chunk_new_no_args(INSTR_PRINT));
		break;
	}
}
