#pragma once

#include "ast.h"
#include "lexer.h"

Expr * parse_expression(Lexer * lexer);
