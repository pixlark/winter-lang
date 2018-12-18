#include "ast.h"
#include "common.h"
#include "compile.h"
#include "lexer.h"
#include "lowering.h"
#include "parser.h"
#include "vm.h"

char * load_string_from_file(char * path)
{
	FILE * file = fopen(path, "r");
	if (file == NULL) return NULL;
	int file_len = 0;
	while (fgetc(file) != EOF) file_len++;
	char * str = (char*) malloc(file_len + 1);
	str[file_len] = '\0';
	fseek(file, 0, SEEK_SET);
	for (int i = 0; i < file_len; i++) str[i] = fgetc(file);
	fclose(file);
	return str;
}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		fatal("Provide one source file");
	}
	const char * source = load_string_from_file(argv[1]);
	if (!source) {
		fatal("'%s' does not exist", argv[1]);
	}
	
	Lexer * lexer = lexer_alloc(source);

	Winter_Machine * wm = winter_machine_alloc();
	
	while (true) {
		Stmt * statement = parse_statement(lexer);
		if (!statement) break;
		
		// Lowering
		lower_statement(statement);
		
		// Compilation
		Compiler compiler;
		compiler.bytecode = NULL;
		compile_statement(&compiler, statement);

		// Free AST
		deep_free(statement);

		#if 0
		// DEBUG Print Bytecode
		printf(":: Bytecode\n\n");
		for (int i = 0; i < sb_count(compiler.bytecode); i++) {
			bc_chunk_print(compiler.bytecode[i]);
		}
		printf("\n\\:\n\n");
		#endif
		
		// Executing
		winter_machine_prime(wm, compiler.bytecode);
		wm->running = true;
		while (wm->running) {
			winter_machine_step(wm);
		}

		sb_free(compiler.bytecode);
	}

	free(wm);
	
	return 0;
}
