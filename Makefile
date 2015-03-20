cat: ./cat/cat.c ./lib/helpers.c
	gcc -o ./cat/cat -w ./cat/cat.c ./lib/helpers.c -I.
helpers: ./lib/helpers.c
	gcc -fPIC -c -w ./lib/helpers.c
	gcc -shared -o ./lib/libhelpers.so ./lib/helpers.o
revwords: ./revwords/revwords.c ./lib/helpers.c
	gcc -o ./revwords/revwords -w ./revwords/revwords.c ./lib/helpers.c -I.
clean:
	rm ./cat/cat ./lib/libhelpers.so ./revwords/revwords
	rm ./cat/*.o ./lib/*.o ./revwords/*.o
