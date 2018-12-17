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
	case OP_NOT:
		P(bc_chunk_new_no_args(INSTR_NOT));
		break;
	case OP_EQ:
		P(bc_chunk_new_no_args(INSTR_EQ));
		break;
	case OP_GT:
		P(bc_chunk_new_no_args(INSTR_GT));
		break;
	case OP_LT:
		P(bc_chunk_new_no_args(INSTR_LT));
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
		compile_expression(compiler, expr->funcall.func);
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

void compile_body(Compiler * compiler, Stmt ** body)
{
	for (int i = 0; i < sb_count(body); i++) {
		compile_statement(compiler, body[i]);
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
		size_t * end_jumps = NULL;
		for (int i = 0; i < sb_count(stmt->_if.conditions); i++) {
			compile_expression(compiler, stmt->_if.conditions[i]);
			
			P(bc_chunk_new_no_args(INSTR_NOP)); // Failure jump placeholder
			size_t failure_jump = L();
			
			compile_body(compiler, stmt->_if.bodies[i]);
			
			P(bc_chunk_new_no_args(INSTR_NOP)); // End jump placeholder
			sb_push(end_jumps, L());
			
			P(bc_chunk_new_no_args(INSTR_NOP)); // Failure landing
			A(failure_jump, bc_chunk_new_condjump(L() - failure_jump, false));
		}
		if (stmt->_if.else_body) {
			compile_body(compiler, stmt->_if.else_body);
		}
		P(bc_chunk_new_no_args(INSTR_NOP)); // Landing spot for end jumps
		// Fill out end jumps
		for (int i = 0; i < sb_count(end_jumps); i++) {
			size_t loc = end_jumps[i];
			A(loc, bc_chunk_new_jump(L() - loc));
		}
		sb_free(end_jumps);
	} break;
	case STMT_FUNC_DECL: {
		Compiler decl_compiler;
		decl_compiler.bytecode = NULL;
		compile_body(&decl_compiler, stmt->func_decl.body);
		Value function = value_new_function(sb_copy(stmt->func_decl.parameters),
											decl_compiler.bytecode);
		P(bc_chunk_new_push(function));
		P(bc_chunk_new_bind(stmt->func_decl.name));
	} break;
	}
}
