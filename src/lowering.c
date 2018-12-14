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

// :\ Lowering Utility

// : Operation Lowering

// Reducing redundant operations to base components

void lower_operations_expr(Expr * expr)
{
	switch (expr->type) {
	case EXPR_ATOM:
		break;
	case EXPR_VAR:
		break;
	case EXPR_UNARY:
		lower_operations_expr(expr->unary.operand);
		break;
	case EXPR_BINARY: {
		if (expr->binary.operator == OP_SUBTRACT) {
			// (a - b) -> (a + -b)
			expr->binary.right = unary_of(OP_NEGATE, expr->binary.right);
			expr->binary.operator = OP_ADD;
		}
		lower_operations_expr(expr->binary.left);
		lower_operations_expr(expr->binary.right);
	} break;
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
