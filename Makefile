.PHONY: bin docs docs-src include src

make:
	cd src && \
	bison --defines=../include/parser.tab.h parser.y && \
	flex scanner.l && \
	gcc -g \
		main.c parser.tab.c lex.yy.c lowering.c vm.c \
		value.c compile.c stretchy_buffer.c error.c \
		-I../include \
		-o ../bin/winter

docs:
	pandoc docs-src/style-guide.md > docs/style-guide.html
