%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "stretchy_buffer.h"
	#include "ast.h"
	int yylex();
	void yyerror(Stmt *** global_statements, char const * s);
%}

%code requires {
	#include "ast.h"
	#include "value.h"
	Stmt ** parse();
	void fb_init();
	#define EXPR(t)							\
		Expr * expr = malloc(sizeof(Expr)); \
		expr->type = t;
	#define STMT(t)							\
		Stmt * stmt = malloc(sizeof(Stmt)); \
		stmt->type = t;
}

%define parse.error verbose
%parse-param { Stmt *** global_statements }

%union {
	const char * name;
	const char ** names;
	int integer_literal;
	Expr * expression;
	Stmt * statement;
	Expr ** expressions;
	Stmt ** statements;
};

%token PRINT;
%token FUNC;
%token <name> NAME;
%token <integer_literal> INTEGER_LITERAL;

%type <expression> expression;
%type <statement> statement;
%type <statements> statements;
%type <statements> scope;
%type <names> arg_list;
%type <expressions> comma_expression;

%left '+' '-'
%left UMINUS

%start program

%%

comma_expression:
/* empty */ {
	$$ = NULL;
}
| expression {
	Expr ** list = NULL;
	sb_push(list, $1);
	$$ = list;
}
| comma_expression ',' expression {
	Expr ** list = $1;
	sb_push(list, $3);
	$$ = list;
}
;

expression:
INTEGER_LITERAL {
	EXPR(EXPR_ATOM);
	expr->atom.value = value_new_integer($1);
	$$ = expr;
}
| NAME {
	EXPR(EXPR_VAR);
	expr->var.name = $1;
	$$ = expr;
}
| NAME '(' comma_expression ')' {
	EXPR(EXPR_FUNCALL);
	expr->funcall.name = $1;
	expr->funcall.args = $3;
	$$ = expr;
}
| expression '+' expression {
	EXPR(EXPR_BINARY);
	expr->binary.operator = OP_ADD;
	expr->binary.left = $1;
	expr->binary.right = $3;
	$$ = expr;
}
| expression '-' expression {
	EXPR(EXPR_BINARY);
	expr->binary.operator = OP_SUBTRACT;
	expr->binary.left = $1;
	expr->binary.right = $3;
	$$ = expr;
}
| '-' expression %prec UMINUS {
	EXPR(EXPR_UNARY);
	expr->unary.operator = OP_NEGATE;
	expr->unary.operand = $2;
	$$ = expr;
}
| '(' expression ')' {
	$$ = $2;
}
;

statements:
statement {
	Stmt ** list = NULL;
	sb_push(list, $1);
	$$ = list;
}
| statements statement {
	Stmt ** list = $1;
	sb_push(list, $2);
	$$ = list;
}
;

scope:
'{' '}' {
	$$ = NULL;
}
| '{' statements '}' {
	$$ = $2;
}
;

arg_list:
/* empty */ {
	$$ = NULL;
}
| NAME {
	const char ** list = NULL;
	sb_push(list, $1);
	$$ = list;
}
| arg_list ',' NAME {
	const char ** list = $1;
	sb_push(list, $3);
	$$ = list;
}
;

statement:
expression ';' {
	STMT(STMT_EXPR);
	stmt->expr.expr = $1;
	$$ = stmt;
}
| NAME '=' expression ';' {
	STMT(STMT_ASSIGN);
	stmt->assign.name = $1;
	stmt->assign.expr = $3;
	$$ = stmt;
}
| PRINT expression ';' {
	STMT(STMT_PRINT);
	stmt->print.expr = $2;
	$$ = stmt;	
}
| FUNC NAME '(' arg_list ')' scope {
	STMT(STMT_FUNC_DECL);
	stmt->func_decl.name = $2;
	stmt->func_decl.parameters = $4;
	stmt->func_decl.body = $6;
	$$ = stmt;
}
;

program:
statement {
	sb_push(*global_statements, $1);
}
| program statement {
	sb_push(*global_statements, $2);
}
;

%%

Stmt ** parse()
{
	Stmt ** global_statements = NULL;
	if (yyparse(&global_statements)) {
		fprintf(stderr, "Encountered error while parsing. Exiting.\n");
		return NULL;
	}
	return global_statements;
}

void yyerror(Stmt *** global_statements, char const * s)
{
	fprintf(stderr, "%s\n", s);
}
