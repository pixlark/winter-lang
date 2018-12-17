#pragma once

#include "ast.h"
#include "lexer.h"

Expr * parse_expression(Lexer * lexer);
Stmt * parse_statement(Lexer * lexer);
