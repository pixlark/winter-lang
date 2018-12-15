#include "ast.h"

void deep_free_expr(Expr * expr)
{
	// ...
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
	case STMT_FUNC_DECL:
		break;
	}
}
