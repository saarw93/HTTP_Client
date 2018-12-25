client: client.o
	gcc client.o -o client -Wvla -g -Wall

client.o: client.c
	gcc -c client.c
