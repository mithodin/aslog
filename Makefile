.PHONY: all clean

all:
	gcc -static -c aslog.c -o aslog.o -lpthreads -fPIC
	ar rcs libaslog.a aslog.o
	rm aslog.o
	gcc -shared -o libaslog.so aslog.c -fPIC

test: test.c aslog.c
	gcc -g -o test test.c aslog.c -lpthread

clean:
	rm -f libaslog.so libaslog.a test

install: all
	cp libaslog.so /usr/local/lib/
	cp libaslog.a /usr/local/lib/
	cp aslog.h /usr/local/include/
