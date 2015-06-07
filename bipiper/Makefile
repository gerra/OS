all: forking 

forking: forking.c ../lib/helpers.c ../lib/bufio.c
	gcc -o forking -w forking.c ../lib/helpers.c ../lib/bufio.c -I.

clean:
	rm *.o forking
