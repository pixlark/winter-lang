#include "ast.h"
#include "common.h"
#include "compile.h"
#include "lowering.h"
#include "vm.h"

#include "parser.tab.h"

int main()
{
	fb_init();

	Winter_Machine * wm = winter_machine_alloc();

	Stmt ** statements = parse();
	
	bool running = true;
	for (int i = 0; i < sb_count(statements); i++) {
		Stmt * statement = statements[i];
		
		// Lowering
		lower_statement(statement);

		// Compilation
		Compiler compiler;
		compiler.bytecode = NULL;
		compile_statement(&compiler, statement);

		// Executing
		winter_machine_prime(wm, compiler.bytecode,
							 sb_count(compiler.bytecode));
		wm->running = true;
		while (wm->running) {
			winter_machine_step(wm);
		}

		sb_free(compiler.bytecode);
	}
	
	return 0;
}
