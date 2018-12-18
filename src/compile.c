#include "compile.h"

#include "common.h"

#define PNOP()      (sb_push(compiler->bytecode, bc_chunk_new_no_args(INSTR_NOP)))
#define P(x, as)    (sb_push(compiler->bytecode, (x)), sb_last(compiler->bytecode).assoc = (as))
#define L()         (sb_count(compiler->bytecode) - 1)
#define A(i, x, as) (compiler->bytecode[i] = (x), compiler->bytecode[i].assoc = (as))

void compile_operator(Compiler * compiler, Operator operator, Assoc_Source as)
{
	switch (operator) {
	case OP_NEGATE:
		P(bc_chunk_new_no_args(INSTR_NEGATE), as);
		break;
	case OP_ADD:
		P(bc_chunk_new_no_args(INSTR_ADD), as);
		break;
	case OP_MULTIPLY:
		P(bc_chunk_new_no_args(INSTR_MULT), as);
		break;
	case OP_DIVIDE:
		P(bc_chunk_new_no_args(INSTR_DIV), as);
		break;
	case OP_NOT:
		P(bc_chunk_new_no_args(INSTR_NOT), as);
		break;
	case OP_EQ:
		P(bc_chunk_new_no_args(INSTR_EQ), as);
		break;
	case OP_GT:
		P(bc_chunk_new_no_args(INSTR_GT), as);
		break;
	case OP_LT:
		P(bc_chunk_new_no_args(INSTR_LT), as);
		break;
	default:
		fatal_internal("A non-compileable operator reached the compilation phase.");
	}
}

void compile_expression(Compiler * compiler, Expr * expr)
{
	switch (expr->type) {
	case EXPR_ATOM:
		P(bc_chunk_new_push(expr->atom.value), expr->assoc);
		break;
	case EXPR_VAR:
		P(bc_chunk_new_get(expr->var.name), expr->assoc);
		break;
	case EXPR_FUNCALL:
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			compile_expression(compiler, expr->funcall.args[i]);
		}
		compile_expression(compiler, expr->funcall.func);
		P(bc_chunk_new_call(sb_count(expr->funcall.args)), expr->assoc);
		break;
	case EXPR_UNARY: {
		compile_expression(compiler, expr->unary.operand);
		compile_operator(compiler, expr->unary.operator, expr->assoc);
	} break;
	case EXPR_BINARY: {
		compile_expression(compiler, expr->binary.left);
		compile_expression(compiler, expr->binary.right);
		compile_operator(compiler, expr->binary.operator, expr->assoc);
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
		P(bc_chunk_new_no_args(INSTR_POP), stmt->assoc);
		break;
	case STMT_ASSIGN:
		compile_expression(compiler, stmt->assign.expr);
		P(bc_chunk_new_bind(stmt->assign.name), stmt->assoc);
		break;
	case STMT_PRINT:
		compile_expression(compiler, stmt->print.expr);
		P(bc_chunk_new_no_args(INSTR_PRINT), stmt->assoc);
		break;
	case STMT_RETURN:
		compile_expression(compiler, stmt->_return.expr);
		P(bc_chunk_new_no_args(INSTR_RETURN), stmt->assoc);
		break;
	case STMT_IF: {
		size_t * end_jumps = NULL;
		for (int i = 0; i < sb_count(stmt->_if.conditions); i++) {
			compile_expression(compiler, stmt->_if.conditions[i]);
			
			PNOP(); // Failure jump placeholder
			size_t failure_jump = L();
			
			compile_body(compiler, stmt->_if.bodies[i]);
			
			PNOP(); // End jump placeholder
			sb_push(end_jumps, L());
			
			PNOP(); // Failure landing
			A(failure_jump, bc_chunk_new_condjump(L() - failure_jump, false), stmt->assoc);
		}
		if (stmt->_if.else_body) {
			compile_body(compiler, stmt->_if.else_body);
		}
		PNOP(); // Landing spot for end jumps
		// Fill out end jumps
		for (int i = 0; i < sb_count(end_jumps); i++) {
			size_t loc = end_jumps[i];
			A(loc, bc_chunk_new_jump(L() - loc), stmt->assoc);
		}
		sb_free(end_jumps);
	} break;
	case STMT_LOOP: {
		PNOP(); // SET_LOOP placeholder
		size_t loc = L();
		compile_body(compiler, stmt->loop.body);
		P(bc_chunk_new_no_args(INSTR_LOOP_END), stmt->assoc);
		PNOP(); // Landing spot for loop end
		A(loc, bc_chunk_new_set_loop(L()), stmt->assoc);
	} break;
	case STMT_BREAK:
		P(bc_chunk_new_no_args(INSTR_BREAK), stmt->assoc);
		break;
	case STMT_CONTINUE:
		P(bc_chunk_new_no_args(INSTR_CONTINUE), stmt->assoc);
		break;
	case STMT_FUNC_DECL: {
		Compiler decl_compiler;
		decl_compiler.bytecode = NULL;
		compile_body(&decl_compiler, stmt->func_decl.body);
		Value function = value_new_function(stmt->func_decl.name,
											sb_copy(stmt->func_decl.parameters),
											decl_compiler.bytecode);
		P(bc_chunk_new_push(function), stmt->assoc);
		P(bc_chunk_new_bind(stmt->func_decl.name), stmt->assoc);
	} break;
	default:
		fatal_internal("A non-compileable statement reached the compilation phase");
	}
}
