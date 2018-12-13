.PHONY: bin docs docs-src include src

make:
	cd src && \
	gcc -g \
		main.c vm.c stretchy_buffer.c \
		-I../include \
		-o ../bin/winter

docs:
	pandoc docs-src/style-guide.md > docs/style-guide.html
