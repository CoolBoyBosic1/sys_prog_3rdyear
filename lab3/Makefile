CXX=g++
CXXFLAGS=-std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter
LEX=flex
YACC=bison -d -v -Wcounterexamples

OBJS=ast.o eval.o ast_dot.o gen_c.o main.o

all: mini_cpp

mini_cpp: parser.tab.c lex.yy.c $(OBJS)
	$(CXX) $(CXXFLAGS) parser.tab.c lex.yy.c $(OBJS) -o $@ -lfl

parser.tab.c parser.tab.h: parser.y
	$(YACC) parser.y

lex.yy.c: lexer.l parser.tab.h
	$(LEX) lexer.l

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run run-run ast
run: mini_cpp
	./mini_cpp example.mc++

run-run: mini_cpp
	./mini_cpp example.mc++ --run

ast: run
	dot -Tpng ast.dot -o ast.png

clean:
	rm -f mini_cpp lex.yy.c parser.tab.c parser.tab.h *.o parser.output ast.dot ast.png

emit-c: mini_cpp
	./mini_cpp example.mc++ --emit-c

emit-c-run: emit-c
	gcc out.c -o out && ./out
