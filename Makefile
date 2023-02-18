# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.
all: parser lexer lib
	gcc -g -O2 parser.o lexer.o lib.o -o shell -lfl -lm


mac: parser lexer lib
	gcc -g -O2 parser.o lexer.o lib.o -o shell -ll -lm
	make clean


lib: lib.c
	gcc -g -O2 -c lib.c

parser: parser.y
	bison -d parser.y
	mv parser.tab.c parser.c
	gcc -g -O2 -c parser.c

lexer: shell.l
	flex shell.l
	mv lex.yy.c lexer.c
	gcc -g -O2 -c lexer.c

clean:
	rm -f *~
	rm -f *.o
	rm -f parser.c
	rm -f parser.tab.h
	rm -f lexer.c
	# rm -f a.out
