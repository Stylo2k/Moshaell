# We construct a separate target for the flex output file lex.yy.c, so that
# it can be compiled separately. Then in "all" you will combine all other
# code you might have into a single final executable.
all: parser lexer libs
	mv *.o build
	gcc -g -O2 -Wall -pedantic -std=c99 -Wno-unused-result -O2 build/*.o -o shell -lfl -lm
	make clean

mac: parser lexer lib
	gcc -g -O2 -Wall -pedantic -std=c99 -Wno-unused-result -O2 build/*.o -o shell -ll -lm
	make clean

parser: parser.y
	bison -d parser.y
	mv parser.tab.c parser.c
	gcc -g -O2 -Wall -pedantic -std=c99 -Wno-unused-result -O2 -c parser.c

# libs rule. Which compiles all the c files in the libs folder
libs: libs/*.c
	gcc -g -O2 -Wall -pedantic -std=c99 -Wno-unused-result -O2 -c libs/*.c


lexer: shell.l
	flex shell.l
	mv lex.yy.c lexer.c
	gcc -O2 -Wall -pedantic -std=c99 -Wno-unused-result -g -O2 -c lexer.c

clean:
	rm -f *~
	rm -f *.o
	rm -f build/*.o
	rm -f parser.c
	rm -f parser.tab.h
	rm -f lexer.c
	rm -f vgcore*
	# rm -f a.out
