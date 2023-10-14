#
# Students' Makefile for the Malloc Lab
#
CC = gcc
CFLAGS = -Wall

OBJS = mdriver.o mm.o memlib.o fsecs.o fcyc.o clock.o ftimer.o

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS+= -g
else
	CFLAGS+= -O2
endif

mdriver: $(OBJS)
	$(CC) $(CFLAGS) -o mdriver $(OBJS)

mdriver.o: mdriver.c fsecs.h fcyc.h clock.h memlib.h config.h mm.h
memlib.o: memlib.c memlib.h
mm.o: mm.c mm.h memlib.h
fsecs.o: fsecs.c fsecs.h config.h
fcyc.o: fcyc.c fcyc.h
ftimer.o: ftimer.c ftimer.h config.h
clock.o: clock.c clock.h

clean:
	rm -f *~ *.o mdriver


