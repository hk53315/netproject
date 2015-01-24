all:
	gcc ./clientDir/client.c -o ./clientDir/client
	gcc server.c -o server
