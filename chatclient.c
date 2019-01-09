/* Derrick Li - CS 372 Section 400 - Jul. 29, 2018
 * Project 1 - Part 1 of 2: chatclient.c
 *
 * A client chat program. Pairs with chatserve.py.
 * USAGE: chatclient hostname port
 *
 * Citation: Used boilerplate code 'client.c' from 'CS 344 - Operating Systems' course at
 * 			 Oregon State University (Winter 2018, Professor Benjamin Brewster)
 *     		 (https://oregonstate.instructure.com/courses/1662153/files/69465547)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Constants */
#define MAX_USERNAME_SIZE 		11			// Maximum byte size of username buffer, (11 chars total: 10 chars + null-terminator)
#define MAX_INPUT_BUFFER_SIZE 	501			// Maximum byte size of input buffer, (501 chars total: 500 chars + null-terminator)
#define	MAX_SEND_BUFFER_SIZE	513			// Maximum byte size of send buffer, (513: username(10) + "> " + message(500) + null-terminator)

/* Global variables */
	int socketFD,							// Socket file descriptor created for connection
		portNumber,							// Port number to connect to server on
		charsWritten,						// Count for number of bytes successfully written to socket
		charsRead;							// Count for number of bytes successfully read from socket
	struct sockaddr_in serverAddress;		// Server address struct needed for socket connection
	struct hostent* serverHostInfo;			// Host info struct needed for socket connection
	char username[MAX_USERNAME_SIZE];		// Client's username/handle
	char buffer[MAX_INPUT_BUFFER_SIZE];		// General purpose input buffer
	char sendBuffer[MAX_SEND_BUFFER_SIZE];	// Send buffer
	char recvBuffer[MAX_SEND_BUFFER_SIZE];	// Receive buffer

// Error function used for reporting issues
void error(const char *msg) {
	perror(msg);
	exit(0);
}

// Setup socket connection to server
void setUpSocketConnectionToServer(int argc, char *argv[]) {

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); 	// Clear out the address struct
	portNumber = atoi(argv[2]); 									// Get the port number, convert string to int

	serverAddress.sin_family = AF_INET; 							// Use IPv4 address family
	serverAddress.sin_port = htons(portNumber);						// Store the port number, convert to network byte order

	serverHostInfo = gethostbyname(argv[1]); 						// Convert the host name into a special form of address
	if (serverHostInfo == NULL) {									// Checks if host exists
		fprintf(stderr, "CLIENT: ERROR, no such host\n");
		exit(0);
	}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 	// Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); 					// Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");		// Check for error

	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 		// Connect socket to address
		error("CLIENT: ERROR connecting");
}

// Get username for client program from user
void getClientUsername() {
	printf("\nHello. Welcome to chatclient.\nWhat would you like your username to be (%d characters max)? ", sizeof(username) - 1);
	memset(username, '\0', sizeof(username)); 			// Clear out the username array
	fgets(username, sizeof(username), stdin); 			// Get username from the user, fgets gets (username - 1) chars, leaving room for '\0'.

	username[strcspn(username, "\n")] = '\0'; 			// Remove the trailing '\n' that fgets gets, replace with '\0'
	if(strlen(username) == sizeof(username) - 1) {		// In the special case that user input equals or exceeds (username - 1), flush input.
		char c;											// (Citation of source: https://stackoverflow.com/questions/7898215/how-to-clear-input-buffer-in-c)
		while ( (c = getchar()) != '\n' && c != EOF );
	}
}

// Get message from user (max character MAX_INPUT_BUFFER_SIZE - 1) and send into socket. Check if client wishes to quit, return 1 if so; else return 0.
int sendMessage() {

	// Get message from user
	printf("%s> ", username); 					// Print user's handle
	memset(buffer, '\0', sizeof(buffer)); 		// Clear out the buffer array
	fgets(buffer, sizeof(buffer), stdin); 		// Get input from the user, fgets gets (buffer - 1) chars, leaving room for '\0'.
	buffer[strcspn(buffer, "\n")] = '\0'; 		// Remove the trailing '\n' that fgets gets, replace with '\0'
	if(strlen(buffer) == sizeof(buffer) - 1) {	// In the special case that user input equals or exceeds (buffer - 1), flush input.
		char c;
		while ( (c = getchar()) != '\n' && c != EOF );
	}

	// Fill out send buffer with username + "> " + input buffer
	memset(sendBuffer, '\0', sizeof(sendBuffer)); 	// Clear out send buffer
	strcpy(sendBuffer, username);					// Copy username into send buffer
	strcat(sendBuffer, "> ");						// Concatenate "> " to send buffer
	strcat(sendBuffer, buffer);						// Concatenate client's message to send buffer

	// Send message to server
	charsWritten = send(socketFD, sendBuffer, strlen(sendBuffer), 0); 	// Write to socket
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");		// Check for errors
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");	// Check all data has been written

	// Checks if user entered "\quit". If so, break loop. Socket connection will be closed in main().
	if(strcmp(buffer, "\\quit") == 0) {
		printf("   *** Connection closed. ***\n");
		return 1;
	}
	return 0;
}

// Recevie message from server. Check if server wishes to quit, return 1 if so; else print message and return 0.
int receiveMessage() {

	// Get message from server
	memset(recvBuffer, '\0', sizeof(recvBuffer)); 						// Clear out the receive buffer
	charsRead = recv(socketFD, recvBuffer, sizeof(recvBuffer) - 1, 0); 	// Read data from the socket, leaving room for '\0' at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");		// Check for read errors

	// Checks if server wishes to quit. If so, break loop. Socket connection will be closed in main().
	// Search for substring "> \quit" in received messaged, ensure the beginning of substring is within the index [0, MAX_USERNAME_SIZE - 1] of received message,
	//    and that the following char after the found substring is the null-terminator.
	char *quitMsg = "> \\quit";						// Quit string to search for
	char *ptr = strstr(recvBuffer, quitMsg);		// Return pointer to string if substring is matched; else NULL is returned.
	if(ptr != NULL &&  ptr >= &recvBuffer[0] && ptr < &recvBuffer[MAX_USERNAME_SIZE] && *(ptr + strlen(quitMsg)) == '\0') {
		printf("   *** Server has closed connection. ***\n");
		return 1;
	}

	// Print message from server
	printf("%s\n", recvBuffer);

	return 0;
}


// Chat loop until either host quits
void chat() {
	while(1) {
		if(sendMessage() == 1)		// Get input from client and send to server; sendMessage() function return 1 if client wishes to quit.
			break;
		if(receiveMessage() == 1) 	// Receive and print message from server; receiveMessage() function return 1 if server wishes to quit.
			break;
	}
}

// Main function
int main(int argc, char *argv[])
{
	// Check usage and args is correct
	if (argc < 3) {
		fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
		exit(0);
	}

	// Setup socket connection to server
	setUpSocketConnectionToServer(argc, argv);

	// Get username from client program from user
	getClientUsername();

	// Inform user that the connection has been established
	printf("\nConnection established with server. Message size limit is %d.\nYou may begin chatting. Type \"\\quit\" to quit.\n\n", sizeof(buffer) - 1);

	// Chat until either host quits
	chat();

	// Say goodbye and close socket
	printf("\nThank you for using chatclient. Goodbye.\n");
	close(socketFD);

	return 0;
}
