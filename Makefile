.PHONY: bin docs docs-src include src

make:
	mkdir -p bin
	cd src && \
	gcc -g \
		main.c parser.c lexer.c lowering.c vm.c gc.c \
		value.c compile.c stretchy_buffer.c error.c ast.c \
		builtin.c \
		-I../include \
		-o ../bin/winter

docs:
	pandoc docs-src/style-guide.md > docs/style-guide.html
