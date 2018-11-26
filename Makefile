# TI-BASIC Interpreter Makefile

CC=gcc
PROG=tibasic
OBJS=$(patsubst %, build/%, main.o runtime.o y.tab.o ast.o lex.yy.o)

default: all
all: $(PROG)

$(PROG): $(OBJS)
	$(CC) -o $@ $^ -lm -ll

build/main.o: src/main.c src/ast.h src/runtime.h src/lexer.h
	$(CC) -o $@ -c $<

build/runtime.o: src/runtime.c src/runtime.h src/ast.h src/nodes.h src/lexer.h src/ti_types.h
	$(CC) -o $@ -c $<

build/y.tab.o: src/y.tab.c src/runtime.h src/ast.h src/nodes.h src/y.tab.h
	$(CC) -o $@ -c $<

src/y.tab.c: src/parser.y
	$(YACC) -dv -o $@ $<

build/ast.o: src/ast.c src/ast.h src/nodes.h
	$(CC) -o $@ -c $<

build/lex.yy.o: src/lex.yy.c src/ast.h src/y.tab.h src/lexer.h src/ti_types.h
	$(CC) -o $@ -c $<

src/lex.yy.c: src/lexer.l
	$(LEX) -t $< > $@

clean:
	$(RM) $(PROG) $(OBJS) src/y.tab.? src/lex.yy.c src/y.output

.PHONY: default all clean
