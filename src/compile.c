#include "compile.h"

#include "common.h"

#define P(x)    (sb_push(compiler->bytecode, (x)))
#define L()     (sb_count(compiler->bytecode) - 1)
#define A(i, x) (compiler->bytecode[i] = (x))

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
		fatal_internal("A non-compileable operator reached the compilation phase.");
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
	case EXPR_FUNCALL:
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			compile_expression(compiler, expr->funcall.args[i]);
		}
		P(bc_chunk_new_get(expr->funcall.name));
		P(bc_chunk_new_call(sb_count(expr->funcall.args)));
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
	case STMT_RETURN:
		compile_expression(compiler, stmt->_return.expr);
		P(bc_chunk_new_no_args(INSTR_RETURN));
		break;
	case STMT_IF: {
		compile_expression(compiler, stmt->_if.expr);
		P(bc_chunk_new_no_args(INSTR_NOP)); // Placeholder for later condjump
		size_t jump_spot = L();
		for (int i = 0; i < sb_count(stmt->_if.body); i++) {
			compile_statement(compiler, stmt->_if.body[i]);
		}
		P(bc_chunk_new_no_args(INSTR_NOP)); // Landing position for condjump
		A(jump_spot, bc_chunk_new_condjump(L() - jump_spot));
	} break;
	case STMT_FUNC_DECL: {
		Compiler decl_compiler;
		decl_compiler.bytecode = NULL;
		for (int i = 0; i < sb_count(stmt->func_decl.body); i++) {
			compile_statement(&decl_compiler, stmt->func_decl.body[i]);
		}
		Value function = value_new_function(sb_copy(stmt->func_decl.parameters),
											decl_compiler.bytecode);
		P(bc_chunk_new_push(function));
		P(bc_chunk_new_bind(stmt->func_decl.name));
	} break;
	}
}
