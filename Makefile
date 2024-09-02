all:
	gcc -o webSlave server.c -ljson-c
	gcc -o webClient client.c
clean:
	rm webSlave webClient
