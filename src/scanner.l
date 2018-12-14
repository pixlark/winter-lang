%option noyywrap
%{
	#include <stdio.h>
	#include "parser.tab.h"
%}

%%

" "|"\t"|"\r"|"\n" {}

"("|")"|"+"|"-"|","|"="|";" {
	return *yytext;
}

"print" { return PRINT; }

[_A-Za-z][_A-Za-z0-9]* {
	yylval.name = strdup(yytext);
	return NAME;
}

[0-9]+ {
	yylval.integer_literal = atoi(yytext);
	return INTEGER_LITERAL;
}

. {
	fprintf(stderr, "Unrecognized character '%c'\n", *yytext);
	exit(1);
}

%%

void init()
{
	yyin = fopen("testfile", "r");
}
