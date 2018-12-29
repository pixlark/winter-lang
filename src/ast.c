#include "ast.h"

// : Expr

void deep_free_expr(Expr * expr);

void deep_free_exprs(Expr ** exprs)
{
	for (int i = 0; i < sb_count(exprs); i++) {
		deep_free_expr(exprs[i]);
	}
	sb_free(exprs);
}

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
		deep_free_exprs(expr->funcall.args);
		break;
	case EXPR_FIELD_ACCESS:
		deep_free_expr(expr->field_access.expr);
		break;
	case EXPR_UNARY:
		deep_free_expr(expr->unary.operand);
		break;
	case EXPR_BINARY:
		deep_free_expr(expr->binary.left);
		deep_free_expr(expr->binary.right);
		break;
	case EXPR_CAST:
		deep_free_expr(expr->cast.expr);
		break;
	case EXPR_LIST:
		deep_free_exprs(expr->list.elements);
		break;
	case EXPR_DICT:
		deep_free_exprs(expr->dict.keys);
		deep_free_exprs(expr->dict.values);
		break;
	case EXPR_STRING:
		break;
	default:
		fatal_internal("Can't free AST Expression!");
		break;
	}
}

// :\ Expr

// : Stmt

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
	case STMT_WHILE:
		deep_free_expr(stmt->_while.condition);
		deep_free_body(stmt->_while.body);
		break;
	case STMT_BREAK:
	case STMT_CONTINUE:
		break;
	case STMT_FUNC_DECL:
		// TODO(pixlark): Free parameters?
		sb_free(stmt->func_decl.parameters);
		deep_free_body(stmt->func_decl.body);
		break;
	case STMT_RECORD_DECL:
		// TODO(pixlark): Free strings?
		sb_free(stmt->record_decl.fields);
		break;
	default:
		fatal_internal("Can't free AST Statement!");
		break;
	}
}

// :\ Stmt
