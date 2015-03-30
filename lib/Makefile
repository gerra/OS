all: helpers bufio

helpers: helpers.c
	gcc -fPIC -c -w helpers.c
	gcc -shared -o libhelpers.so helpers.o

bufio: bufio.c
	gcc -fPIC -c -w bufio.c
	gcc -shared -o libbufio.so bufio.o

clean:
	rm *.o *.so
