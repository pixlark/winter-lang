.PHONY: bin docs docs-src include src

make:
	cd src && bison --defines=../include/parser.tab.h parser.y && flex scanner.l
	cd src && \
	gcc -g \
		main.c parser.tab.c lex.yy.c vm.c value.c stretchy_buffer.c \
		-I../include \
		-o ../bin/winter

docs:
	pandoc docs-src/style-guide.md > docs/style-guide.html
