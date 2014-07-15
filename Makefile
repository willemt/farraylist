GCOV_OUTPUT = *.gcda *.gcno *.gcov 
GCOV_CCFLAGS = -fprofile-arcs -ftest-coverage
CC     = gcc
CCFLAGS = -I. -Itests -g -O2 -Wall -Werror -W -fno-omit-frame-pointer -fno-common -fsigned-char $(GCOV_CCFLAGS)


all: test

main.c:
	sh tests/make-tests.sh tests/test*.c > main.c

test: main.c fixed_arraylist.o tests/test_fixedarraylist.c tests/CuTest.c main.c
	$(CC) $(CCFLAGS) -o $@ $^
	./test
	gcov main.c tests/test_fixedarraylist.c fixed_arraylist.c

fixed_arraylist.o: fixed_arraylist.c
	$(CC) $(CCFLAGS) -c -o $@ $^

clean:
	rm -f main.c fixed_arraylist.o test $(GCOV_OUTPUT)
