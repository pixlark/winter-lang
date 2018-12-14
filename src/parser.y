%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include "stretchy_buffer.h"
	#include "ast.h"
	int yylex();
	void yyerror(Stmt *** statement, char const *);
	void init();
%}

%code requires {
	#include "ast.h"
	#include "value.h"
	Stmt ** parse();
	#define EXPR(t)							\
		Expr * expr = malloc(sizeof(Expr)); \
		expr->type = t;
	#define STMT(t)							\
		Stmt * stmt = malloc(sizeof(Stmt)); \
		stmt->type = t;
}

%define parse.error verbose
%parse-param { Stmt *** statements }

%union {
	const char * name;
	int integer_literal;
	Expr * expression;
	Stmt * statement;
};

%token PRINT;
%token <name> NAME;
%token <integer_literal> INTEGER_LITERAL;

%type <expression> expression;
%type <statement> statement;

%left '+' '-'
%left UMINUS

%start program

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
;

program:
statement {
	sb_push(*statements, $1);
}
| program statement {
	sb_push(*statements, $2);
}
;

%%

Stmt ** parse()
{
	init();
	Stmt ** statements = NULL;
	if (yyparse(&statements)) {
		fprintf(stderr, "Encountered error while parsing. Exiting.\n");
		exit(1);
	}
	return statements;
}

void yyerror(Stmt *** statements, char const * s)
{
	fprintf(stderr, "%s\n", s);
}
