CC = gcc
CFLAGS = -Wall -g -D DEBUG
LIBS = -lm

OBJS = lib.o tests.o

tests: lib.c tests.c lib.h
	$(CC) $(CFLAGS) $(LIBS) -o tests lib.c lib.h tests.c

clean:
	rm -f *~ *.o tests
