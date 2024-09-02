all:
	gcc -o webSlave server.c -ljson-c
	gcc -o client client.c
clean:
	rm webSlave client
