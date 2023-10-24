CC = gcc217
FLAGS = 

# Dependency rules for non-file targets
all: testsymtablelist testsymtablehash
clobber: clean
	rm -f ~ \#\#
clean:
	rm -f testsymtablelist testsymtablehash *.o

# Dependency rules for file targets
testsymtablelist: testsymtable.o symtablelist.o
	$(CC) $(FLAGS) testsymtable.o symtablelist.o -o testsymtablelist
testsymtablehash: testsymtable.o symtablehash.o
	$(CC) $(FLAGS) testsymtable.o symtablehash.o -o testsymtablehash

testsymtable.o: testsymtable.c symtable.h
	$(CC) $(FLAGS) -c testsymtable.c

symtablelist.o: symtablelist.c symtable.h
	$(CC) $(FLAGS) -c symtablelist.c
symtablehash.o: symtablehash.c symtable.h
	$(CC) $(FLAGS) -c symtablehash.c