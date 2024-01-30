CFLAGS = -std=c99 -D_DEFAULT_SOURCE -Wall -Wextra -O2

ddriver: main.c libmpfcdevice.a
	$(CC) $(CFLAGS) main.c libmpfcdevice.a -o ddriver -lpthread

run: ddriver
	./ddriver

clean:
	$(RM) ddriver
