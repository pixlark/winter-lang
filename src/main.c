#include "ast.h"
#include "common.h"
#include "vm.h"

#include "parser.tab.h"

void parse_test()
{
	Stmt ** statements = parse();
}

int main()
{
	parse_test();
	vm_test();
	return 0;
}
