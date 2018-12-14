#pragma once

#include "common.h"
#include "value.h"

// : Operator

typedef enum {
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
} Operator;

// :\ Operator

// : Expr

typedef enum {
	EXPR_ATOM,
	EXPR_VAR,
	EXPR_UNARY,
	EXPR_BINARY,
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
			Operator operator;
			struct Expr * operand;
		} unary;
		struct {
			Operator operator;
			struct Expr * left;
			struct Expr * right;
		} binary;
	};
} Expr;

// :\ Expr

// : Stmt

typedef enum {
	STMT_EXPR,
	STMT_ASSIGN,
	STMT_PRINT,
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
		} print;
	};
} Stmt;

// :\ Stmt