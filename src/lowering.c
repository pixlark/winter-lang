#include "common.h"
#include "lowering.h"

// : Lowering Utility

// Useful functions for lowering

Expr * unary_of(Operator operator, Expr * expr)
{
	Expr * unary = malloc(sizeof(Expr));
	unary->type = EXPR_UNARY;
	unary->unary.operator = operator;
	unary->unary.operand = expr;
	unary->assoc = expr->assoc;
}

Expr * dup_expr(Expr * expr)
{
	Expr * dup = malloc(sizeof(Expr));
	memcpy(dup, expr, sizeof(Expr));
	return dup;
}

void not_sleeve(Expr * expr)
{
	Expr * inner = dup_expr(expr);
	expr->type = EXPR_UNARY;
	expr->unary.operator = OP_NOT;
	expr->unary.operand = inner;
}

// :\ Lowering Utility

// : Operation Lowering

// Reducing redundant operations to base components

void lower_binary(Expr * expr)
{
	internal_assert(expr->type == EXPR_BINARY);
	switch (expr->binary.operator) {
	case OP_SUBTRACT:
		// (a - b) -> (a + -b)
		expr->binary.right = unary_of(OP_NEGATE, expr->binary.right);
		expr->binary.operator = OP_ADD;
		break;
	case OP_NE: {
		// (a != b) -> !(a == b)
		not_sleeve(expr);
		Expr * inner = expr->unary.operand;
		inner->binary.operator = OP_EQ;
	} break;
	case OP_LTE: {
		// (a <= b) -> !(a > b)
		not_sleeve(expr);
		Expr * inner = expr->unary.operand;
		inner->binary.operator = OP_GT;	
	} break;
	case OP_GTE: {
		// (a >= b) -> !(a < b)
		not_sleeve(expr);
		Expr * inner = expr->unary.operand;
		inner->binary.operator = OP_LT;
	} break;
	}
}

void lower_operations_expr(Expr * expr)
{
	switch (expr->type) {
	case EXPR_ATOM:
		break;
	case EXPR_VAR:
		break;
	case EXPR_FUNCALL:
		lower_operations_expr(expr->funcall.func);
		for (int i = 0; i < sb_count(expr->funcall.args); i++) {
			lower_operations_expr(expr->funcall.args[i]);
		}
		break;
	case EXPR_FIELD_ACCESS:
		lower_operations_expr(expr->field_access.expr);
		break;
	case EXPR_UNARY:
		lower_operations_expr(expr->unary.operand);
		break;
	case EXPR_BINARY:
		lower_operations_expr(expr->binary.left);
		lower_operations_expr(expr->binary.right);
		lower_binary(expr);
		break;
	case EXPR_CAST:
		lower_operations_expr(expr->cast.expr);
		break;
	case EXPR_LIST:
		for (int i = 0; i < sb_count(expr->list.elements); i++) {
			lower_operations_expr(expr->list.elements[i]);
		}
		break;
	case EXPR_DICT:
		internal_assert(sb_count(expr->dict.keys) == sb_count(expr->dict.values));
		for (int i = 0; i < sb_count(expr->dict.keys); i++) {
			lower_operations_expr(expr->dict.keys[i]);
			lower_operations_expr(expr->dict.values[i]);
		}
		break;
	case EXPR_STRING:
		break;
	default:
		fatal_internal("An unlowerable expression reached lower_operations_expr");
	}
}

// :\ Operation Lowering

// : Lowering

void lower_expression(Expr * expr)
{
	lower_operations_expr(expr);
}

void lower_body(Stmt ** body)
{
	for (int i = 0; i < sb_count(body); i++) {
		lower_statement(body[i]);
	}
}

Stmt * lower_while(Stmt * stmt)
{
	internal_assert(stmt->type == STMT_WHILE);
	// Create lowered statement
	Stmt * lowered = malloc(sizeof(Stmt));
	lowered->type = STMT_LOOP;
	// Create body of lowered statement
	Stmt ** new_body = NULL;
	Stmt * condition = malloc(sizeof(Stmt));
	condition->type = STMT_IF;
	// Put the condition, negated, in an if
	condition->_if.conditions = NULL;
	{
		Expr * condition_expr = stmt->_while.condition;
		not_sleeve(condition_expr);
		sb_push(condition->_if.conditions, condition_expr);
	}
	// Put a break in the if body
	condition->_if.bodies = NULL;
	{
		Stmt ** if_body = NULL;
		Stmt * break_stmt = malloc(sizeof(Stmt));
		break_stmt->type = STMT_BREAK;
		sb_push(if_body, break_stmt);
		sb_push(condition->_if.bodies, if_body);
	}
	// Add if to beginning of lowered statement body
	sb_push(new_body, condition);
	// Add rest of while body to lowered statement body
	Stmt ** while_body = (stmt->_while.body);
	Stmt ** add_from = sb_add(new_body, sb_count(while_body));
	for (int i = 0; i < sb_count(while_body); i++) {
		add_from[i] = while_body[i];
	}
	// Add new body to lowered statement + assign lowered to input statement
	lowered->loop.body = new_body;
	// Clean up memory
	sb_free(while_body);
	return lowered;
}

// TODO(pixlark): Rewrite all lowering functions to operate via return
// value, simply handling any intermediate freeing explicitly. A lot
// simpler this way I think.

Stmt * lower_statement(Stmt * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		lower_expression(stmt->expr.expr);
		return stmt;
	case STMT_ASSIGN:
		lower_expression(stmt->assign.expr);
		return stmt;
	case STMT_RETURN:
		lower_expression(stmt->_return.expr);
		return stmt;
	case STMT_IF:
		for (int i = 0; i < sb_count(stmt->_if.conditions); i++) {
			lower_expression(stmt->_if.conditions[i]);
		}
		for (int i = 0; i < sb_count(stmt->_if.bodies); i++) {
			lower_body(stmt->_if.bodies[i]);
		}
		if (stmt->_if.else_body) lower_body(stmt->_if.else_body);
		return stmt;
	case STMT_LOOP:
		lower_body(stmt->loop.body);
		return stmt;
	case STMT_WHILE:
		lower_expression(stmt->_while.condition);
		lower_body(stmt->_while.body);
		stmt = lower_while(stmt);
		return stmt;
	case STMT_BREAK:
	case STMT_CONTINUE:
		return stmt;
	case STMT_FUNC_DECL:
		lower_body(stmt->func_decl.body);
		return stmt;
	case STMT_RECORD_DECL:
		return stmt;
	default:
		fatal_internal("An unlowerable statement reached lower_statement");
	}
}

// :\ Lowering
