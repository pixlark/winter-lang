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
	case EXPR_UNARY:
		lower_operations_expr(expr->unary.operand);
		break;
	case EXPR_BINARY:
		lower_operations_expr(expr->binary.left);
		lower_operations_expr(expr->binary.right);
		lower_binary(expr);
		break;
	}
}

void lower_operations(Stmt * stmt)
{
	switch (stmt->type) {
	case STMT_EXPR:
		lower_operations_expr(stmt->expr.expr);
		break;
	case STMT_ASSIGN:
		lower_operations_expr(stmt->assign.expr);
		break;
	case STMT_PRINT:
		lower_operations_expr(stmt->print.expr);
		break;
	}
}

// :\ Operation Lowering

void lower_statement(Stmt * stmt)
{
	lower_operations(stmt);
}
