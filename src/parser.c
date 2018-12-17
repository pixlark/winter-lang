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

void __expect(Lexer * lexer, Token_Type type)
{
	if (!match(type)) {
		fatal("Expected %s, got %s instead",
			  token_to_string((Token) { type }),
			  token_to_string(lexer->token));
	}
}
#define expect(x) __expect(lexer, (x))

void __weak_expect(Lexer * lexer, Token_Type type)
{
	if (!is(type)) {
		fatal("Expected %s, got %s instead",
			  token_to_string((Token) { type }),
			  token_to_string(lexer->token));
	}
}
#define weak_expect(x) __weak_expect(lexer, (x))

#define token() lexer->token

Expr * parse_atom(Lexer * lexer)
{
	// Check for subexpression
	if (match('(')) {
		Expr * subexpr = parse_expression(lexer);
		expect(')');
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
			expect(')');
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

Stmt * parse_assignment(Lexer * lexer)
{
	STMT(STMT_ASSIGN);
	weak_expect(TOKEN_NAME);
	stmt->assign.name = token().name;
	advance();
	expect('=');
	stmt->assign.expr = parse_expression(lexer);
	expect(';');
	return stmt;
}

const char ** parse_function_parameters(Lexer * lexer)
{
	const char ** parameters = NULL;
	weak_expect(TOKEN_NAME);
	sb_push(parameters, token().name);
	advance();
	while (match(',')) {
		weak_expect(TOKEN_NAME);
		sb_push(parameters, token().name);
		advance();
	}
	return parameters;
}

Stmt ** parse_scope(Lexer * lexer)
{
	expect('{');
	Stmt ** body = NULL;
	while (!match('}')) {
		sb_push(body, parse_statement(lexer));
	}
	return body;
}

Stmt * parse_function_declaration(Lexer * lexer)
{
	STMT(STMT_FUNC_DECL);
	weak_expect(TOKEN_NAME);
	stmt->func_decl.name = token().name;
	advance();
	expect('(');
	if (match(')')) {
		stmt->func_decl.parameters = NULL;
	} else {
		stmt->func_decl.parameters = parse_function_parameters(lexer);
		expect(')');
	}
	stmt->func_decl.body = parse_scope(lexer);
	return stmt;
}

Stmt * parse_if_statement(Lexer * lexer)
{
	STMT(STMT_IF);
	stmt->_if.conditions = NULL;
	stmt->_if.bodies = NULL;
	stmt->_if.else_body = NULL;

	sb_push(stmt->_if.conditions, parse_expression(lexer));
	sb_push(stmt->_if.bodies, parse_scope(lexer));
	
	while (match(TOKEN_ELSE)) {
		if (match(TOKEN_IF)) {
			// Else if
			sb_push(stmt->_if.conditions, parse_expression(lexer));
			sb_push(stmt->_if.bodies, parse_scope(lexer));
		} else {
			// Else
			stmt->_if.else_body = parse_scope(lexer);
			break;
		}
	}
	return stmt;
}

Stmt * parse_statement(Lexer * lexer)
{
	if (is(TOKEN_EOF)) return NULL;
	
	if (match(TOKEN_PRINT)) {
		// print
		STMT(STMT_PRINT);
		stmt->print.expr = parse_expression(lexer);
		expect(';');
		return stmt;
	} else if (match(TOKEN_RETURN)) {
		// return
		STMT(STMT_RETURN);
		stmt->_return.expr = parse_expression(lexer);
		expect(';');
		return stmt;
	} else if (match(TOKEN_IF)) {
		// if
		return parse_if_statement(lexer);
	} else if (match(TOKEN_FUNC)) {
		// function declaration
		return parse_function_declaration(lexer);
	} else if (lexer_lookahead(lexer, 1).type == '=') {
		// TODO(pixlark): Kluge.
		/* Once we have l-expressions as a detailed thing, with
		   indexing and all that, how will we know that this is an
		   assignment??
		   -Paul T. Sun Dec 16 23:12:45 2018 */
		// assignment
		return parse_assignment(lexer);
	} else {
		// expression
		STMT(STMT_EXPR);
		stmt->expr.expr = parse_expression(lexer);
		expect(';');
		return stmt;
	}
}
