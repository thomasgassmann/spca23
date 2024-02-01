CFLAGS = -std=c99 -D_DEFAULT_SOURCE -Wall -Wextra

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CFLAGS+= -g -D DEBUG
else
	CFLAGS+= -O2
endif

ddriver: main.c libmpfcdevice.a
	$(CC) $(CFLAGS) main.c libmpfcdevice.a -o ddriver -lpthread

run: ddriver
	./ddriver

clean:
	$(RM) ddriver
