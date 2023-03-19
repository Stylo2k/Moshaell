# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.

# Variable to hold whether its a bonus

.PHONY: libs

all: parser lexer libs
	mkdir -p build
	mv *.o build
	gcc -g -O2 -Wall -D_POSIX_C_SOURCE=199309L -pedantic -std=c99 -O2 build/*.o -o shell -ll -lm; \

mac: parser lexer libs
	mkdir -p build
	mv *.o build
	gcc -g -O2 -Wall -pedantic -std=c99 -O2 build/*.o -o shell -ll -lm
	make clean


parser: parser.y
	bison -d parser.y
	mv parser.tab.c parser.c
	gcc -g -O2 -D_POSIX_C_SOURCE=199309L -Wall -pedantic -std=c99 -O2 -c parser.c

libs:
	gcc -g -O2 -D_POSIX_C_SOURCE=199309L -Wall -pedantic -std=c99 -O2 -c libs/*.c


lexer: shell.l
	flex shell.l
	mv lex.yy.c lexer.c
	gcc -O2 -D_POSIX_C_SOURCE=199309L -pedantic -std=c99 -g -O2 -c lexer.c

clean:
	rm -f *~
	rm -f *.o
	rm -f build/*.o
	rm -f parser.c
	rm -f parser.tab.h
	rm -f lexer.c
	rm -f vgcore*
