CC = gcc
CFLAGS = -Wall
LIBS = -lm

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS+= -g -D DEBUG
else
	CFLAGS+= -O2
endif

OBJS = lib.o tests.o

tests: lib.c tests.c lib.h
	$(CC) $(CFLAGS) $(LIBS) -o tests lib.c lib.h tests.c

master:
	$(CC) $(CFLAGS) $(LIBS) -o master master.c lib.h tests.c

clean:
	rm -f *~ *.o tests master
