all: simplesh

simplesh: simplesh.c ../lib/helpers.c ../lib/bufio.c
	gcc -o simplesh -w simplesh.c ../lib/helpers.c ../lib/bufio.c -I.

clean:
	rm *.o simplesh
