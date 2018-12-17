#include "ast.h"
#include "common.h"
#include "compile.h"
#include "lowering.h"
#include "vm.h"

#include "lexer.h"
#include "parser.h"

#include "parser.tab.h"

const char * test_source =
	"func my_func(a, b, c) { x = a + b + c; print x; }";

int main()
{
	Lexer * lexer = lexer_alloc(test_source);
	
	while (true) {
		Stmt * stmt = parse_statement(lexer);
		printf("%p\n", stmt);
		if (!stmt) break;
	}
	
	return 0;
	
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

		// Free AST
		deep_free(statement);
		
		// Executing
		winter_machine_prime(wm, compiler.bytecode,
							 sb_count(compiler.bytecode));
		wm->running = true;
		while (wm->running) {
			winter_machine_step(wm);
		}

		sb_free(compiler.bytecode);
	}

	free(wm);
	
	return 0;
}
