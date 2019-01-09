 # Derrick Li - CS 372 Section 400 - Jul. 29, 2018
 # Project 1 - Part 2 of 2: chatserve.py
 #
 # A server chat program with basic multi-threading functionality. Pairs with executable compiled from chatclient.c.
 # USAGE: python3 chatserve_ec.py port
 #
 # Citation: Used boilerplate code 'TCPServe.py' from Computer Networking: A Top-Down Approach, 7th Edition by
 # 			 Jim Kurose and Keith Ross (CH 2 - Application Layer, Section 2.7 - Socket Programming, p168-169).

import sys
import socket
import signal
import threading

# Constants
MAX_USERNAME_SIZE 	= 10											# Max username byte size
MAX_MESSAGE_SIZE 	= 500											# Max message byte size
MAX_RECV_SIZE 		= MAX_USERNAME_SIZE + MAX_MESSAGE_SIZE + 2		# Max receive byte size is MAX_USERNAME_SIZE + "> " + MAX_MESSAGE_SIZE
SERVER_NAME 		= "Server"										# Server's username/handle, must be less than or equal to 10 chars

# Global variables
serverSocket 		= 0				# Server's listening socket
serverName 			= SERVER_NAME	# Server's username/handle

# Handles SIGINT - exits on receipt.
# Citation: https://stackoverflow.com/questions/1112343/how-do-i-capture-sigint-in-python
def signal_handler(sig, frame):
        sys.exit(0)

# Check usage and args is correct
def checkUsage():
	if len(sys.argv) < 2:
		print("USAGE: python3 " + sys.argv[0] + " port")
		sys.exit(0)

# Setup listening socket on port supplied in arguments
def setUpListeningSocket():
	global serverSocket
	serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)	# Create TCP socket
	serverPort = int(sys.argv[1])										# Get port number from arguments
	serverSocket.bind(('', serverPort))									# Bind socket to port number
	serverSocket.listen(1)												# Begin listening on port

# Receive message from client through network. Check if client wishes to quit, return 1 if so; else print message and return 0.
def receiveMessage(connectionSocket):
	receivedMsg = connectionSocket.recv(MAX_RECV_SIZE).decode()					# Limit maximum receiving message size to MAX_RECV_SIZE

	quitMsg = "> \\quit"						# Quit string to search for
	foundIndex = receivedMsg.find(quitMsg)		# Perform substring search on received message

	# Ensure index of beginning of substring is found within the first MAX_USERNAME_SIZE chars of received message, and also ensure nothing else follows it.
	if foundIndex >= 0 and foundIndex <= MAX_USERNAME_SIZE and foundIndex + len(quitMsg) == len(receivedMsg):
		print("   *** Client has ended the connection. ***")	# If so, print termination message to screen, break out of loop and close connection
		return 1

	print(receivedMsg)											# Else, print received message to screen
	return 0

# Get message input from server. Truncate message if length exceeds MAX_MESSAGE_SIZE. Prepend server's username/handle to message and send.
# Check if server wishes to quit, return 1 if so; else return 0.
def sendMessage(connectionSocket):
	global serverName
	sendingMsg = input(serverName + "> ")							# Get server's input message
	if len(sendingMsg) > MAX_MESSAGE_SIZE:							# If input message exceeds MAX_MESSAGE_SIZE, truncate message to size
		sendingMsg = sendingMsg[:MAX_MESSAGE_SIZE]

	sendingMsgWithHandle = serverName + "> " + sendingMsg			# Prepend server's username/handle and "> " to input message
	connectionSocket.send(sendingMsgWithHandle.encode())			# Send message into socket
	if sendingMsg == "\\quit":										# If server's input is "\quit", then print termination message to screen,
		print("   *** Connection closed. ***")						#	break out of loop and close connection
		return 1
	return 0

# Chat loop until either host quits
def chat(connectionSocket):
	while True:
		if receiveMessage(connectionSocket) == 1:		# Receive and print message from client; receiveMessage() function return 1 if client wishes to quit.
			break
		if sendMessage(connectionSocket) == 1:			# Get input from server and send to client; sendMessage() function return 1 if server wishes to quit.
			break

# Creates a new chat session using the connectionSocket created from accepting the incoming socket connection request.
def createNewChatSession(connectionSocket, addr):

	# Print status of connection to screen and chat until either host quits then close socket
	print("\nEstablished connection with client on Host: " + addr[0] + ", Port: " + str(addr[1]) + ".\nMessage size limit is " + str(MAX_MESSAGE_SIZE) + ". Type \"\\quit\" to quit.\n");
	chat(connectionSocket)			# Chat until either host quits
	connectionSocket.close()		# Closes current connection

# Main function
def main():
	global serverSocket
	signal.signal(signal.SIGINT, signal_handler)	# Setup SIGINT handler
	checkUsage()									# Check for proper usage of commandline arguments
	setUpListeningSocket()							# Set up server's listening socket

	# Loop for each incoming connection request. Server can handle only one connection, i.e. chat session, at a given instance.
	while True:
		print("\n --- " + SERVER_NAME + " is listening for connections on port " + sys.argv[1] + " --- ")

		connectionSocket, addr = serverSocket.accept()						# Block until incoming request received, then accept

		# Instantiate a Thread object with the start function as createNewChatSession(), and start new thread.
		# Source citation: https://pymotw.com/2/threading/
		t = threading.Thread(target=createNewChatSession, args=(connectionSocket, addr,))
		t.start()   # Start new thread

if __name__ == "__main__":
	main()