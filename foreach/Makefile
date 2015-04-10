all: foreach

foreach: foreach.c ../lib/helpers.c ../lib/bufio.c
	gcc -o foreach -w foreach.c ../lib/helpers.c ../lib/bufio.c -I.

clean:
	rm *.o foreach
