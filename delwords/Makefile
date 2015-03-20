all: delwords

delwords: delwords.c ../lib/helpers.c
	gcc -o delwords -w delwords.c ../lib/helpers.c -I.

clean:
	rm *.o delwords
