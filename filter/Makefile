all: filter

filter: filter.c ../lib/helpers.c
	gcc -o filter -w filter.c ../lib/helpers.c -I.

clean:
	rm *.o filter
