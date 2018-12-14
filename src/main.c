#include "ast.h"
#include "common.h"
#include "vm.h"

#include "parser.tab.h"

void parse_test()
{
	fb_init();
	Stmt * statement;
	do {
		statement = parse();
	} while (statement);
}

int main()
{
	parse_test();
	vm_test();
	return 0;
}
