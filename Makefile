.PHONY: bin docs docs-src include src

make:
	cd src && \
	gcc -g \
		main.c parser.c lexer.c lowering.c vm.c \
		value.c compile.c stretchy_buffer.c error.c ast.c \
		-I../include \
		-o ../bin/winter

docs:
	pandoc docs-src/style-guide.md > docs/style-guide.html
