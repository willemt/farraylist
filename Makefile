GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
CC     = gcc
CCFLAGS = -g -O2 -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS)


all: tests

main.c:
	sh make-tests.sh > main.c

tests: main.c fixed_arraylist.o test_fixedarraylist.c CuTest.o main.c
	$(CC) $(CCFLAGS) -o $@ $^
	./tests
	gcov main.c test_fixedarraylist.c fixed_arraylist.c

fixed_arraylist.o: fixed_arraylist.c
	$(CC) $(CCFLAGS) -c -o $@ $^

CuTest.o: CuTest.c
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f main.c fixed_arraylist.o tests $(GCOV_OUTPUT)
