%option noyywrap
%{
	#include <stdio.h>
	#include "parser.tab.h"
%}

%%

\/\/[^\n]* {}

" "|"\t"|"\r"|"\n" {}

"("|")"|"+"|"-"|","|"="|";"|"{"|"}" {
	return *yytext;
}

"func"  { return FUNC; }
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

void fb_init()
{
	yyin = fopen("testfile", "r");
}
