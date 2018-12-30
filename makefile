client: client.o
	gcc client.o -o client -g -Wall

client.o: client.c
	gcc -c client.c
