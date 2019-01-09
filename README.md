# Chat Application Using Socket Programming

This is a pair of chat programs that communicate to each other in a client-server manner. The chat client `chatclient.c` is coded in C, and the chat server `chatserve.py` is coded in Python. Both programs uses the socket API.

Boilerplate code from Oregon State University, CS 344 - Winter 2018 and Computer Networking: A Top-Down Approach, 7th Edition by Jim Kurose and Keith Ross were used in coding this project.

## Instructions

### Notes
* The chat server `chatserve.py` must be started prior to any chat client `chatclient.exe`.
* The server can hold multiple, concurrent chat sessions with different clients. However, the chat server is only able to respond to one client at a time, in an iterative first-come-first-serve fashion.
* The chat client and server take turn sending messages.
* Either the chat client or server can end a chat session by sending `\quit` when it is their turn to send a message. The session is closed gracefully on the other end.

### Running chatserve.py
1. Run `python3 chatserve.py <port_number>` to start the chat server, e.g. `python3 chatserve.py 12345`. In the unlikely event that the port is in use, use a different port number in the range of `0 - 65535`.
2. Now any number of chat clients, using `chatclient.exe`, can connect to the chat server.
3. Exit the chat server by sending SIGINT `[Ctrl]+C`.

### Running chatclient.exe
1. Run `make` to compile `chatclient.exe` from `chatclient.c`.
2. Run `chatclient.exe <hostname> <port_number>` to connect to the chat server.
  * The `hostname` should be the name of the machine hosting the chat server `chatserve.py`. If you are running locally, it is most likely `localhost` or `127.0.0.1`.
  * The `port_number` needs to match the port number the chat server is running on.
  * In the example above, the chat client would connect to the chat server by running `chatclient.exe localhost 12345`.