#include "ast.h"
#include "common.h"
#include "lowering.h"
#include "vm.h"

#include "parser.tab.h"

int main()
{
	fb_init();
	
	bool running = true;
	while (true) {
		Stmt * statement = parse();
		if (!statement) break;
		lower_statement(statement);
		printf("\n");
	}
	
	return 0;
}
