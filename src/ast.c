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
	case STMT_FUNC_DECL:
		sb_free(stmt->func_decl.parameters);
		for (int i = 0; i < sb_count(stmt->func_decl.body); i++) {
			deep_free(stmt->func_decl.body[i]);
		}
		sb_free(stmt->func_decl.body);
		break;
	default:
		fatal_internal("Can't free AST Statement!");
		break;
	}
}
