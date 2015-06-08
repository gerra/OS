all: forking polling 

forking: forking.c ../lib/helpers.c ../lib/bufio.c
	gcc -o forking -w forking.c ../lib/helpers.c ../lib/bufio.c -I.

polling: polling.c ../lib/helpers.c ../lib/bufio.c
	gcc -o polling -w polling.c ../lib/helpers.c ../lib/bufio.c -I.

clean:
	rm *.o forking polling
