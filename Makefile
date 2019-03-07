.PHONY: all clean

all:
	gcc -static -c aslog.c -o aslog.o -lpthreads -fPIC
	ar rcs libaslog.a aslog.o
	rm aslog.o
	gcc -shared -o libaslog.so aslog.c -fPIC

clean:
	rm -f libaslog.so libaslog.a

install: all
	cp libaslog.so /usr/local/lib/
	cp libaslog.a /usr/local/lib/
	cp aslog.h /usr/local/include/
