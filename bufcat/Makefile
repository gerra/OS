all: bufcat

bufcat: bufcat.c ../lib/bufio.c
	gcc -o bufcat -w --debug bufcat.c ../lib/bufio.c -I.

clean:
	rm *.o bufcat
