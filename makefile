default: chatclient.c
	gcc -o chatclient chatclient.c

clean:
	rm -f chatclient chatclient.exe