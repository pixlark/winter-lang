#include "parser.h"

#include "common.h"
#include "vm.h"

// : Parse utility

#define EXPR(t)									\
	Expr * expr = malloc(sizeof(Expr));			\
	expr->type = t;								\
	expr->marked = false;

#define STMT(t)									\
	Stmt * stmt = malloc(sizeof(Stmt));			\
	stmt->type = t;								\
	stmt->marked = false

void mark_expr(Expr * expr, Assoc_Source assoc)
{
	expr->marked = true;
	expr->assoc = assoc;
}

void mark_stmt(Stmt * stmt, Assoc_Source assoc)
{
	stmt->marked = true;
	stmt->assoc = assoc;
}

bool __match(Lexer * lexer, Token_Type type)
{
	if (lexer->token.type == type) {
		lexer_advance(lexer);
		return true;
	}
	return false;
}

#define token() (lexer->token)

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
		fatal_assoc(token().assoc,
					"Expected %s, got %s instead",
					token_type_to_string(type),
					token_to_string(lexer->token));
	}
}
#define expect(x) __expect(lexer, (x))

void __weak_expect(Lexer * lexer, Token_Type type)
{
	if (!is(type)) {
		fatal_assoc(token().assoc,
					"Expected %s, got %s instead",
					token_type_to_string(type),
					token_to_string(lexer->token));
	}
}
#define weak_expect(x) __weak_expect(lexer, (x))

// :\ Parse utility

// : Parsing

// Have this return just an expr
Expr ** parse_list_literal(Lexer * lexer)
{
	Expr ** elements = NULL;
	expect('[');
	while (!is(']')) {
		sb_push(elements, parse_expression(lexer));
		if (!match(',')) break;
	}
	expect(']');
	return elements;
}

Expr * parse_dict_literal(Lexer * lexer)
{
	EXPR(EXPR_DICT);
	mark_expr(expr, token().assoc);
	expect('{');
	expr->dict.keys = NULL;
	expr->dict.values = NULL;
	while (!is('}')) {
		sb_push(expr->dict.keys, parse_expression(lexer));
		expect(TOKEN_RARROW);
		sb_push(expr->dict.values, parse_expression(lexer));
		if (!match(',')) break;
	}
	internal_assert(sb_count(expr->dict.keys) == sb_count(expr->dict.values));
	expect('}');
	return expr;
}

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
		mark_expr(expr, token.assoc);
		expr->atom.value = value_none();
		advance();
		return expr;
	} break;
	case TOKEN_INTEGER_LITERAL: {
		EXPR(EXPR_ATOM);
		mark_expr(expr, token.assoc);
		expr->atom.value = value_new_integer(token.integer_literal);
		advance();
		return expr;
	} break;
	case TOKEN_FLOAT_LITERAL: {
		EXPR(EXPR_ATOM);
		mark_expr(expr, token.assoc);
		expr->atom.value = value_new_float(token.float_literal);
		advance();		
		return expr;
	} break;
	case TOKEN_TRUE:
	case TOKEN_FALSE: {
		EXPR(EXPR_ATOM);
		mark_expr(expr, token.assoc);
		expr->atom.value = value_new_bool(token.type == TOKEN_TRUE);
		advance();		
		return expr;
	} break;
	case TOKEN_STRING_LITERAL: {
		EXPR(EXPR_STRING);
		mark_expr(expr, token.assoc);
		expr->string.literal = token.string_literal;
		advance();
		return expr;
	} break;
	case TOKEN_NAME: {
		EXPR(EXPR_VAR);
		mark_expr(expr, token.assoc);
		expr->var.name = token.name;
		advance();		
		return expr;
	} break;
	case TOKEN_BUILTIN: {
		EXPR(EXPR_ATOM);
		mark_expr(expr, token.assoc);
		expr->atom.value = value_new_builtin(token.builtin);
		advance();
		return expr;
	} break;
	case '[': {
		EXPR(EXPR_LIST);
		mark_expr(expr, token.assoc);
		expr->list.elements = parse_list_literal(lexer);
		return expr;
	} break;
	case '{': {
		return parse_dict_literal(lexer);
	} break;
	case TOKEN_INT:
	case TOKEN_FLOAT:
	case TOKEN_BOOL:
	case TOKEN_STRING:
	case TOKEN_LIST: {
		EXPR(EXPR_ATOM);
		mark_expr(expr, token.assoc);
		Value_Type type;
		switch (token().type) {
		case TOKEN_INT:
			type = VALUE_INTEGER;
			break;
		case TOKEN_FLOAT:
			type = VALUE_FLOAT;
			break;
		case TOKEN_BOOL:
			type = VALUE_BOOL;
			break;
		case TOKEN_STRING:
			type = VALUE_STRING;
			break;
		case TOKEN_LIST:
			type = VALUE_LIST;
			break;
		}
		advance();
		expr->atom.value = value_new_type(type);
		return expr;
	} break;
	default: {
		printf("");
		fatal_assoc(token().assoc,
					"Expected expression, got %s instead",
					token_to_string(lexer->token));
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
		mark_expr(expr, left->assoc);
		expr->funcall.func = left;
		expr->funcall.args = args;
		return expr;
	}
	return left;
}

// Precedence level 0.5
Expr * parse_postfix(Lexer * lexer)
{
	Expr * left = parse_function_call(lexer);
	while (is('[')) {
		Assoc_Source as = token().assoc;
		expect('[');
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		expr->binary.operator = OP_INDEX;
		expr->binary.left = left;
		expr->binary.right = parse_expression(lexer);
		expect(']');
		left = expr;
	}
	return left;
}

// Precedence level 1
Expr * parse_prefix(Lexer * lexer)
{
	if (is('-')) {
		Assoc_Source as = token().assoc;
		expect('-');
		EXPR(EXPR_UNARY);
		mark_expr(expr, as);
		expr->unary.operator = OP_NEGATE;
		expr->unary.operand = parse_prefix(lexer);
		return expr;
	} else if (is('!')) {
		Assoc_Source as = token().assoc;
		expect('!');
		EXPR(EXPR_UNARY);
		mark_expr(expr, as);
		expr->unary.operator = OP_NOT;
		expr->unary.operand = parse_prefix(lexer);
	} else {
		return parse_postfix(lexer);
	}
}

// Precedence level 1.5
Expr * parse_cast(Lexer * lexer)
{
	Expr * left = parse_prefix(lexer);
	while (is(TOKEN_AS)) {
		Assoc_Source as = token().assoc;
		expect(TOKEN_AS);
		EXPR(EXPR_CAST);
		mark_expr(expr, as);
		expr->cast.expr = left;
		expr->cast.type = parse_prefix(lexer);
		left = expr;
	}
	return left;
}

// Precedence level 2
Expr * parse_mul_ops(Lexer * lexer)
{
	Expr * left = parse_cast(lexer);
	while (is('*') || is('/')) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		switch (token().type) {
		case '*':
			expr->binary.operator = OP_MULTIPLY;
			break;
		case '/':
			expr->binary.operator = OP_DIVIDE;
			break;
		}
		advance();
		expr->binary.left = left;
		expr->binary.right = parse_cast(lexer);
		left = expr;
	}
	return left;
}

// Precedence level 3
Expr * parse_add_ops(Lexer * lexer)
{
	Expr * left = parse_mul_ops(lexer);
	while (is('+') || is('-')) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		switch (token().type) {
		case '+':
			expr->binary.operator = OP_ADD;
			break;
		case '-':
			expr->binary.operator = OP_SUBTRACT;
			break;
		}
		advance();
		expr->binary.left = left;
		expr->binary.right = parse_mul_ops(lexer);
		left = expr;
	}
	return left;
}

bool is_comp_op(Token_Type t)
{
	return
		t == '>' || t == '<' ||
		t == TOKEN_GTE || t == TOKEN_LTE;
}

// Precedence level 4
Expr * parse_comp_ops(Lexer * lexer)
{
	Expr * left = parse_add_ops(lexer);
	while (is_comp_op(token().type)) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		switch (token().type) {
		case '>':
			expr->binary.operator = OP_GT;
			break;
		case '<':
			expr->binary.operator = OP_LT;
			break;
		case TOKEN_GTE:
			expr->binary.operator = OP_GTE;
			break;
		case TOKEN_LTE:
			expr->binary.operator = OP_LTE;
			break;
		}
		advance();
		expr->binary.left = left;
		expr->binary.right = parse_add_ops(lexer);
		left = expr;
	}
	return left;
}

// Precedence level 5
Expr * parse_eq_ops(Lexer * lexer)
{
	Expr * left = parse_comp_ops(lexer);
	while (is(TOKEN_EQ) || is(TOKEN_NE)) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		switch (token().type) {
		case TOKEN_EQ:
			expr->binary.operator = OP_EQ;
			break;
		case TOKEN_NE:
			expr->binary.operator = OP_NE;
			break;
		}
		advance();
		expr->binary.left = left;
		expr->binary.right = parse_comp_ops(lexer);
		left = expr;
	}
	return left;
}

// Precedence level 6
Expr * parse_and(Lexer * lexer)
{
	Expr * left = parse_eq_ops(lexer);
	while (is(TOKEN_AND)) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		advance();
		expr->binary.operator = OP_AND;
		expr->binary.left = left;
		expr->binary.right = parse_eq_ops(lexer);
		left = expr;
	}
	return left;
}

// Precedence level 7
Expr * parse_or(Lexer * lexer)
{
	Expr * left = parse_and(lexer);
	while (is(TOKEN_OR)) {
		Assoc_Source as = token().assoc;
		EXPR(EXPR_BINARY);
		mark_expr(expr, as);
		advance();
		expr->binary.operator = OP_OR;
		expr->binary.left = left;
		expr->binary.right = parse_and(lexer);
		left = expr;
	}
	return left;
}

Expr * parse_expression(Lexer * lexer)
{
	return parse_or(lexer);
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
	Assoc_Source as = token().assoc;
	expect(TOKEN_FUNC);
	
	STMT(STMT_FUNC_DECL);
	mark_stmt(stmt, as);
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

Stmt * parse_record_declaration(Lexer * lexer)
{
	Assoc_Source as = token().assoc;
	expect(TOKEN_RECORD);

	STMT(STMT_RECORD_DECL);
	mark_stmt(stmt, as);
	weak_expect(TOKEN_NAME);
	stmt->record_decl.name = token().name;
	advance();
	
	expect('{');
	stmt->record_decl.fields = NULL;
	while (!is('}')) {
		weak_expect(TOKEN_NAME);
		sb_push(stmt->record_decl.fields, token().name);
		advance();
		if (!is(',')) break;
		expect(',');
	}
	expect('}');
	
	return stmt;
}

Stmt * parse_if_statement(Lexer * lexer)
{
	Assoc_Source as = token().assoc;
	expect(TOKEN_IF);
	
	STMT(STMT_IF);
	mark_stmt(stmt, as);
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

bool is_assign_target(Expr * expr)
{
	return true;
}	

Stmt * parse_statement(Lexer * lexer)
{
	if (is(TOKEN_EOF)) return NULL;
	
	if (is(TOKEN_RETURN)) {
		// return
		Assoc_Source as = token().assoc;
		expect(TOKEN_RETURN);
		STMT(STMT_RETURN);
		mark_stmt(stmt, as);
		stmt->_return.expr = parse_expression(lexer);
		expect(';');
		return stmt;
	} else if (is(TOKEN_IF)) {
		// if
		return parse_if_statement(lexer);
	} else if (is(TOKEN_FUNC)) {
		// function declaration
		return parse_function_declaration(lexer);
	} else if (is(TOKEN_RECORD)) {
		// record declaration
		return parse_record_declaration(lexer);
	} else if (is(TOKEN_LOOP)) {
		Assoc_Source as = token().assoc;
		expect(TOKEN_LOOP);
		STMT(STMT_LOOP);
		mark_stmt(stmt, as);
		stmt->loop.body = parse_scope(lexer);
		return stmt;
	} else if (is(TOKEN_WHILE)) {
		Assoc_Source as = token().assoc;
		expect(TOKEN_WHILE);
		STMT(STMT_WHILE);
		mark_stmt(stmt, as);
		stmt->_while.condition = parse_expression(lexer);
		stmt->_while.body = parse_scope(lexer);
		return stmt;
	} else if (is(TOKEN_BREAK)) {
		Assoc_Source as = token().assoc;
		expect(TOKEN_BREAK);
		STMT(STMT_BREAK);
		mark_stmt(stmt, as);
		expect(';');
		return stmt;
	} else if (is(TOKEN_CONTINUE)) {
		Assoc_Source as = token().assoc;
		expect(TOKEN_CONTINUE);
		STMT(STMT_CONTINUE);
		mark_stmt(stmt, as);
		expect(';');
		return stmt;
	}
	#if 0
	else if (lexer_lookahead(lexer, 1).type == '=') {
		// TODO(pixlark): Kluge.
		/* Once we have l-expressions as a detailed thing, with
		   indexing and all that, how will we know that this is an
		   assignment??
		   -Paul T. Sun Dec 16 23:12:45 2018 */
		// assignment
		return parse_assignment(lexer);
	}
	#endif
	else {
		// Expression or Assignment
		Expr * left = parse_expression(lexer);
		if (is('=')) {
			// Assignment
			if (!is_assign_target(left)) {
				fatal_assoc(left->assoc, "Expression not valid target for assignment");
			}
			STMT(STMT_ASSIGN);
			stmt->assign.target = left;
			Assoc_Source as = token().assoc;
			mark_stmt(stmt, as);
			advance();
			stmt->assign.expr = parse_expression(lexer);
			expect(';');
			return stmt;
		} else {
			// Expression
			STMT(STMT_EXPR);
			stmt->expr.expr = left;
			mark_stmt(stmt, stmt->expr.expr->assoc);
			expect(';');
			return stmt;
		}
	}
}

// :\ Parsing
