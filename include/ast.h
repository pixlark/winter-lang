#pragma once
#include "common.h"
#include "value.h"

// : Operator

typedef enum {
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
	OP_NOT,
	OP_EQ,
	OP_NE,
	OP_GT,
	OP_LT,
	OP_GTE,
	OP_LTE,
	OP_AND,
	OP_OR,
} Operator;

// :\ Operator

// : Expr

typedef enum {
	EXPR_ATOM,
	EXPR_VAR,
	EXPR_FUNCALL,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_CAST,
	EXPR_LIST,
	EXPR_STRING,
} Expr_Type;

typedef struct Expr {
	Expr_Type type;
	union {
		struct {
			Value value;
		} atom;
		struct {
			const char * name;
		} var;
		struct {
			struct Expr * func;
			struct Expr ** args;
		} funcall;
		struct {
			Operator operator;
			struct Expr * operand;
		} unary;
		struct {
			Operator operator;
			struct Expr * left;
			struct Expr * right;
		} binary;
		struct {
			struct Expr * expr;
			Value_Type type;
		} cast;
		struct {
			struct Expr ** elements;
		} list;
		struct {
			const char * literal;
		} string;
	};
	bool marked;
	Assoc_Source assoc;
} Expr;

// :\ Expr

// : Stmt

typedef enum {
	STMT_EXPR,
	STMT_ASSIGN,
	STMT_PRINT,
	STMT_RETURN,
	STMT_IF,
	STMT_LOOP,
	STMT_BREAK,
	STMT_CONTINUE,
	STMT_FUNC_DECL,
} Stmt_Type;

typedef struct Stmt {
	Stmt_Type type;
	union {
		struct {
			Expr * expr;
		} expr;
		struct {
			const char * name;
			Expr * expr;
		} assign;
		struct {
			Expr * expr;
		} _return;
		struct {
			Expr ** conditions;
			struct Stmt *** bodies;
			struct Stmt ** else_body;
		} _if;
		struct {
			struct Stmt ** body;
		} loop;
		struct {
			const char * name;
			const char ** parameters;
			struct Stmt ** body;
		} func_decl;
	};
	bool marked;
	Assoc_Source assoc;
} Stmt;

void deep_free(Stmt * statement);

// :\ Stmt
