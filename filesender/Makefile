all: filesender

filesender: filesender.c ../lib/helpers.c ../lib/bufio.c
	gcc -o filesender -w filesender.c ../lib/helpers.c ../lib/bufio.c -I.

clean:
	rm *.o filesender
