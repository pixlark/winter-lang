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
	case EXPR_UNARY:
		lower_operations_expr(expr->unary.operand);
		break;
	case EXPR_BINARY:
		lower_operations_expr(expr->binary.left);
		lower_operations_expr(expr->binary.right);
		lower_binary(expr);
		break;
	default:
		fatal_internal("An unlowerable expression reached lower_operations_expr");
	}
}

// :\ Operation Lowering

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

void lower_statement(Stmt * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		lower_expression(stmt->expr.expr);
		break;
	case STMT_ASSIGN:
		lower_expression(stmt->assign.expr);
		break;
	case STMT_PRINT:
		lower_expression(stmt->print.expr);
		break;
	case STMT_RETURN:
		lower_expression(stmt->_return.expr);
		break;
	case STMT_IF:
		for (int i = 0; i < sb_count(stmt->_if.conditions); i++) {
			lower_expression(stmt->_if.conditions[i]);
		}
		for (int i = 0; i < sb_count(stmt->_if.bodies); i++) {
			lower_body(stmt->_if.bodies[i]);
		}
		if (stmt->_if.else_body) lower_body(stmt->_if.else_body);
		break;
	case STMT_LOOP:
		lower_body(stmt->loop.body);
		break;
	case STMT_BREAK:
	case STMT_CONTINUE:
		break;
	case STMT_FUNC_DECL:
		lower_body(stmt->func_decl.body);
		break;
	default:
		fatal_internal("An unlowerable statement reached lower_statement");
	}
}
