all:
	gcc -g -O0 -o webSlave server.c -ljson-c
	gcc -g -O0 -o webClient client.c -ljson-c
clean:
	rm webSlave webClient
