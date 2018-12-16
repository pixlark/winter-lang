#include "parser.h"

#include "common.h"
#include "vm.h"

#define EXPR(t)									\
	Expr * expr = malloc(sizeof(Expr));			\
	expr->type = t;

#define STMT(t)									\
	Stmt * stmt = malloc(sizeof(Stmt));			\
	stmt->type = t;

bool __match(Lexer * lexer, Token_Type type)
{
	if (lexer->token.type == type) {
		lexer_advance(lexer);
		return true;
	}
	return false;
}
#define match(x) __match(lexer, (x))

bool __is(Lexer * lexer, Token_Type type)
{
	return lexer->token.type == type;
}
#define is(x) __is(lexer, (x))

#define advance() lexer_advance(lexer)

Expr * parse_atom(Lexer * lexer)
{
	// Check for subexpression
	if (match('(')) {
		Expr * subexpr = parse_expression(lexer);
		if (!match(')')) {
			fatal("Expected ')', got %s instead",
				  token_to_string(lexer->token));
		}
		return subexpr;
	}
	Token token = lexer->token;
	switch (token.type) {
	case TOKEN_NONE: {
		EXPR(EXPR_ATOM);
		expr->atom.value = value_none();
		advance();
		return expr;
	} break;
	case TOKEN_INTEGER_LITERAL: {
		EXPR(EXPR_ATOM);
		expr->atom.value = value_new_integer(token.integer_literal);
		advance();
		return expr;
	} break;
	case TOKEN_FLOAT_LITERAL: {
		EXPR(EXPR_ATOM);
		expr->atom.value = value_new_float(token.float_literal);
		advance();		
		return expr;
	} break;
	case TOKEN_TRUE:
	case TOKEN_FALSE: {
		EXPR(EXPR_ATOM);
		expr->atom.value = value_new_bool(token.type == TOKEN_TRUE);
		advance();		
		return expr;
	} break;
	case TOKEN_NAME: {
		EXPR(EXPR_VAR);
		expr->var.name = token.name;
		advance();		
		return expr;
	} break;				
	}
}

Expr ** parse_comma_expression(Lexer * lexer)
{
	Expr ** expressions = NULL;
	sb_push(expressions, parse_expression(lexer));
	while (match(',')) {
		sb_push(expressions, parse_expression(lexer));
	}
	return expressions;
}

Expr * parse_function_call(Lexer * lexer)
{
	Expr * left = parse_atom(lexer);
	if (match('(')) {
		Expr ** args = NULL;
		if (match(')')) {
			args = NULL;
		} else {
			args = parse_comma_expression(lexer);
			if (!match(')')) {
				fatal("Expected ')', got %s instead",
					  token_to_string(lexer->token));
			}
		}
		EXPR(EXPR_FUNCALL);
		expr->funcall.func = left;
		expr->funcall.args = args;
		return expr;
	}
	return left;
}

Expr * parse_prefix(Lexer * lexer)
{
	if (match('-')) {
		EXPR(EXPR_UNARY);
		expr->unary.operator = OP_NEGATE;
		expr->unary.operand = parse_prefix(lexer);
		return expr;
	} else {
		return parse_function_call(lexer);
	}
}

Expr * parse_add_ops(Lexer * lexer)
{
	Expr * left = parse_prefix(lexer);
	if (is('+') || is('-')) {
		EXPR(EXPR_BINARY);
		switch (lexer->token.type) {
		case '+':
			expr->binary.operator = OP_ADD;
			advance();
			break;
		case '-':
			expr->binary.operator = OP_SUBTRACT;
			advance();
			break;
		}
		expr->binary.left = left;
		expr->binary.right = parse_add_ops(lexer);
		return expr;
	} else {
		return left;
	}
}

Expr * parse_expression(Lexer * lexer)
{
	if (is(TOKEN_EOF)) return NULL;
	return parse_add_ops(lexer);
}
