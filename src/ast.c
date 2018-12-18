#include "ast.h"

void deep_free_expr(Expr * expr)
{
	switch (expr->type) {
	case EXPR_ATOM:
		// TODO(pixlark): Figure out what to do about
		// VALUE_FUNCTIONs... should we copy and free here or what?
		break;
	case EXPR_VAR:
		break;
	case EXPR_FUNCALL:
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			deep_free_expr(expr->funcall.args[i]);
		}
		sb_free(expr->funcall.args);
		break;
	case EXPR_UNARY:
		deep_free_expr(expr->unary.operand);
		break;
	case EXPR_BINARY:
		deep_free_expr(expr->binary.left);
		deep_free_expr(expr->binary.right);
		break;
	default:
		fatal_internal("Can't free AST Expression!");
		break;
	}
}

void deep_free_body(Stmt ** body)
{
	for (int i = 0; i < sb_count(body); i++) {
		deep_free(body[i]);
	}
	sb_free(body);
}

void deep_free(Stmt * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		deep_free_expr(stmt->expr.expr);
		break;
	case STMT_ASSIGN:
		deep_free_expr(stmt->assign.expr);
		break;
	case STMT_PRINT:
		deep_free_expr(stmt->print.expr);
		break;
	case STMT_RETURN:
		deep_free_expr(stmt->_return.expr);
		break;
	case STMT_IF:
		// Free conditions
		for (int i = 0; i < sb_count(stmt->_if.conditions); i++) {
			deep_free_expr(stmt->_if.conditions[i]);
		}
		sb_free(stmt->_if.conditions);
		// Free bodies
		for (int i = 0; i < sb_count(stmt->_if.bodies); i++) {
			deep_free_body(stmt->_if.bodies[i]);
		}
		sb_free(stmt->_if.bodies);
		// Free else
		deep_free_body(stmt->_if.else_body);
		break;
	case STMT_LOOP:
		deep_free_body(stmt->loop.body);
		break;
	case STMT_BREAK:
	case STMT_CONTINUE:
		break;
	case STMT_FUNC_DECL:
		sb_free(stmt->func_decl.parameters);
		deep_free_body(stmt->func_decl.body);
		break;
	default:
		fatal_internal("Can't free AST Statement!");
		break;
	}
}
