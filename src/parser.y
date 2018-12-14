%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "stretchy_buffer.h"
	#include "ast.h"
	int yylex();
	void yyerror(Stmt ** statement, char const *);
%}

%code requires {
	#include "ast.h"
	#include "value.h"
	Stmt * parse();
	void fb_init();
	#define EXPR(t)							\
		Expr * expr = malloc(sizeof(Expr)); \
		expr->type = t;
	#define STMT(t)							\
		Stmt * stmt = malloc(sizeof(Stmt)); \
		stmt->type = t;
}

%define parse.error verbose
%parse-param { Stmt ** statement }

%union {
	const char * name;
	int integer_literal;
	Expr * expression;
	Stmt * statement;
};

%token END_OF_FILE;
%token PRINT;
%token FUNC;
%token <name> NAME;
%token <integer_literal> INTEGER_LITERAL;

%type <expression> expression;

%left '+' '-'
%left UMINUS

%start statement

%%

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

statement:
expression ';' {
	STMT(STMT_EXPR);
	stmt->expr.expr = $1;
	*statement = stmt;
	YYACCEPT;
	//$$ = stmt;
}
| NAME '=' expression ';' {
	STMT(STMT_ASSIGN);
	stmt->assign.name = $1;
	stmt->assign.expr = $3;
	*statement = stmt;
	YYACCEPT;
	//$$ = stmt;
}
| PRINT expression ';' {
	STMT(STMT_PRINT);
	stmt->print.expr = $2;
	*statement = stmt;
	YYACCEPT;
	//$$ = stmt;
}
| END_OF_FILE {
	*statement = NULL;
	YYACCEPT;
}
;

%%

Stmt * parse()
{
	Stmt * statement = NULL;
	if (yyparse(&statement)) {
		fprintf(stderr, "Encountered error while parsing. Exiting.\n");
		return NULL;
	}
	return statement;
}

void yyerror(Stmt ** statement, char const * s)
{
	fprintf(stderr, "%s\n", s);
}
