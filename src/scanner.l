%option noyywrap
%{
	#include <stdio.h>
	#include "parser.tab.h"
%}

%%

<<EOF>> { return END_OF_FILE; }

\/\/[^\n]* {}

" "|"\t"|"\r"|"\n" {}

"("|")"|"+"|"-"|","|"="|";"|"{"|"}" {
	return *yytext;
}

"func"   { return FUNC;   }
"print"  { return PRINT;  }
"none"   { return NONE;   }
"return" { return RETURN; }
"true"   { return TRUE;   }
"false"  { return FALSE;  }

[_A-Za-z][_A-Za-z0-9]* {
	yylval.name = strdup(yytext);
	return NAME;
}

[0-9]+ {
	yylval.integer_literal = atoi(yytext);
	return INTEGER_LITERAL;
}

[-+]?[0-9]*\.?[0-9]+ {
	yylval.float_literal = atof(yytext);
	return FLOAT_LITERAL;
}

. {
	fatal("Unrecognized character '%c'", *yytext);
}

%%

void fb_init()
{
	yyin = fopen("testfile", "r");
}
