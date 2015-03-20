all: cat revwords helpers

helpers: ./lib/helpers.c
	gcc -fPIC -c -w ./lib/helpers.c
	gcc -shared -o ./lib/libhelpers.so ./lib/helpers.o
cat: ./cat/cat.c ./lib/helpers.c
	gcc -o ./cat/cat -w ./cat/cat.c ./lib/helpers.c -I.
revwords: ./revwords/revwords.c ./lib/helpers.c helpers
	gcc -o ./revwords/revwords -w ./revwords/revwords.c ./lib/helpers.c -I.
clean:
	rm ./cat/cat ./lib/libhelpers.so ./revwords/revwords
	rm ./cat/*.o ./lib/*.o ./revwords/*.o
