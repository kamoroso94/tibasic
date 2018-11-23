# TI-BASIC Interpreter Makefile

CC=gcc
PROG=tibasic
OBJS=build/main.o build/runtime.o build/y.tab.o build/ast.o build/lex.yy.o

default: all
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ -lm -ll

build/main.o: src/main.c src/runtime.h src/ast.h
	$(CC) -o $@ -c src/main.c

build/runtime.o: src/runtime.c src/ast.h src/nodes.h
	$(CC) -o $@ -c src/runtime.c

build/y.tab.o: src/y.tab.c src/ast.h src/nodes.h src/y.tab.h
	$(CC) -o $@ -c src/y.tab.c

src/y.tab.c: src/parser.y
	$(YACC) -dv -o $@ src/parser.y

build/ast.o: src/ast.c src/ast.h
	$(CC) -o $@ -c src/ast.c

build/lex.yy.o: src/lex.yy.c src/ast.h src/y.tab.h
	$(CC) -o $@ -c src/lex.yy.c

src/lex.yy.c: src/lexer.l
	$(LEX) -t src/lexer.l > $@

clean:
	$(RM) $(PROG) $(OBJS) src/y.tab.? src/lex.yy.c

.PHONY: default all clean
