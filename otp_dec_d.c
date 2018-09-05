/*******************************************************************************
** Author: James Hippler (HipplerJ)
** Oregon State University
** CS 344-400 (Spring 2018)
** Operating Systems I
**
** Description: Program 4 - OTP (Block 4)
** Due: Sunday, June 10, 2018
**
** Filename: otp_dec_d.c
**
** OBJECTIVES:
** In this assignment, you will be creating five small programs that encrypt
** and decrypt information using a one-time pad-like system. I believe that
** you will find the topic quite fascinating: one of your challenges will be
** to pull yourself away from the stories of real-world espionage and tradecraft
** that have used the techniques you will be implementing.
**
** These programs serve as a capstone to what you have been learning in this
** course, and will combine the multi-processing code you have been learning
** with socket-based inter-process communication. Your programs will also
** accessible from the command line using standard UNIX features like
** input/output redirection, and job control. Finally, you will write a
** short compilation script.
**
** otp_dec_d: This program performs exactly like otp_enc_d, in syntax and usage.
** In this case, however, otp_dec_d will decrypt ciphertext it is given, using
** the passed-in ciphertext and key. Thus, it returns plaintext again to
** otp_dec.
*******************************************************************************/

/*******************************************************************************
** PREPROCESSOR DIRECTIVES
*******************************************************************************/

#include <stdio.h>   																														// Include the stdio.h library
#include <stdlib.h>																															// Include the stdlib.h library
#include <string.h>																															// Include the string.h library
#include <unistd.h>																															// Include the unistd.h library
#include <sys/types.h>																													// Include the sys/types.h library
#include <sys/socket.h>																													// Include the sys/socket.h library
#include <netinet/in.h>																													// Include the netinet/in.h library
#include <stdbool.h>																														// Include the stdbool.h library

/*******************************************************************************
** GLOBAL CONSTANT DECLARATIONS
*******************************************************************************/

#define OTP_ENC "otp_dec"																												// Declare global constant for the process name "otp_dec"
#define OTP_ENC_D "otp_dec_d"																										// Declare global constant for the process name "otp_dec_d"
#define TERM_SIG "!!"																														// Declare global constatn for the transmission termination signal
#define MSG_BUFFER 1000																													// Declare global constant for total message transmission size
#define MAX_MSG_SIZE 75000																											// Declare global constant for maximum size of total message
#define MAX_PROCESSES 5																													// Declare global constant for total number of available processes

/*******************************************************************************
** STRUCTURE DECLARATIONS
*******************************************************************************/

// Establishes a struct to store process information
struct process_info{
	pid_t processes[MAX_PROCESSES];																								// Holds the child process id's that are forked
  int active;																																		// Manages the number of active processes
};

/*******************************************************************************
** FUNCTION DECLARATIONS
*******************************************************************************/

void confirm_arguments(int, char**);																						// Declare function to confirm user input
void set_address_struct(struct sockaddr_in*, char**);														// Declare function to set the address structure
void set_socket(int*);																													// Declare function to establish the connection socket
void enable_socket(struct sockaddr_in, int);																		// Declare function to enable the socket
void accept_connection(struct sockaddr_in*, int*, int);													// Declare function to accept the connection from the client
void check_availability(int, struct process_info);															// Declare function to check if the server is busy or available
void introduction(int);																													// Declare function to introduce the client and the server process information
void check_processes(pid_t, int, struct process_info*);													// Declare function to check on background process information
void receive_file_text(int, char*);																							// Declare function to receive the plain and key files from the client
char* receive_message(int);																											// Declare function to receive messages from client
void send_message(int, char*);																									// Declare function to send messages from client
void decryption(int, char*, char*);																							// Declare function to decrypt the plain text file using the key
void send_decryption(int, char*);																								// Declare function to send the decrypted plaintext to the client
void add_process(pid_t, struct process_info*);																	// Declare function to add the new child process that was forked
void remove_process(pid_t, struct process_info*);																// Declare function to remove the terminated child process
void close_connection(int*);																										// Declare function to close connections with the client
void close_listen_socket(int*);																									// Declare function to close the listen socket for the server
void error(const char*);																												// Declare function to output error messages and terminate the program

/*******************************************************************************
* Description: main function
* This is the main function where the program begins (and hopefully ends).
* It receives the total number of arguments and an array with the actual
* arguments received. It calls the appropriate functions in the correct order
* to communicate with the client.
*******************************************************************************/

int main(int argc, char *argv[]) {
	pid_t 	child_id = 0;																													// Establish a pid_t function to hold child ids
	int 		listenSocketFD,																												// Establish an integer variable to store socket information
					establishedConnectionFD,																							// Establish an integer variable to store socket information
					child_exit = -5;																											// Establish an integer variable to store child exit status
	char 		plain[MAX_MSG_SIZE],																									// Establish character array to store the plain text file
	 				key[MAX_MSG_SIZE];																										// Establish character array to store the key text file
	struct 	sockaddr_in serverAddress,																						// Establish a structure instance for sockaddr_in called serverAddress
					clientAddress;																												// Establish a structure instance for sockaddr_in called clientAddress
	struct 	process_info process;																									// Establish a structure instance for process_info called process

	process.active = 0;																														// Initialize active processes to zero
	memset(process.processes, '\0', MAX_PROCESSES);																// Clear the process array and set elements to null
	memset(plain, '\0', MAX_MSG_SIZE);																						// Clear the plain text array and set elements to null
	memset(key, '\0', MAX_MSG_SIZE);																							// Clear the key text array and set elements to null

	confirm_arguments(argc, argv);																								// Call function to confirm user input
	set_address_struct(&serverAddress, argv);																			// Call function to build server address structure
	set_socket(&listenSocketFD);																									// Call function to set the listen socket
	enable_socket(serverAddress, listenSocketFD);																	// Call function to enable the listen socket
	while(true) {																																	// Establish loop to run forever (while true)
		accept_connection(&clientAddress, &establishedConnectionFD, 								// Call function to accept client connections
			listenSocketFD);
		check_processes(child_id, child_exit, &process);														// Check the background processes for termination
		child_id = fork();																													// Fork a child process and store the retuned process id
		if (child_id < 0){																													// If there was an error forking
			fprintf(stderr, "ERROR: Unable to create new process\n");									// Print and error message to stderr but continue running the program
		} else if (child_id == 0) {																									// If the process id is zero then we are operating in the child
			check_availability(establishedConnectionFD, process);											// Call function to determine if the server is available
			introduction(establishedConnectionFD);																		// Call function to introduce the server and the client process information
			receive_file_text(establishedConnectionFD, plain);												// Call function to receive the plain text file
			receive_file_text(establishedConnectionFD, key);													// Call function to receive the key text file
			decryption(establishedConnectionFD, plain, key);													// Call function to create ciphertext with plain and key information
			close_connection(&establishedConnectionFD);																// Close the socket where the server is communicating with clients
			exit(EXIT_SUCCESS);																												// Exit the child process with a success status
		} else {																																		// Otherwise, we're operating in the parent process
			add_process(child_id, &process);																					// Add the new child process to the structure tracking that information
		}
	}
	close_listen_socket(&listenSocketFD);																					// Close the socket where the server is listening for communication
	return 0;																																			// Exit program successfully after all process are complete
}

/*******************************************************************************
* Description: confirm_arguments function
* Function confirms that the user input the appropriate number of arguments
* when executing the program from the command line.
*******************************************************************************/

void confirm_arguments(int argc, char** argv) {
	if (argc != 2) {																															// Check for correct number of arguments (2 total)
		fprintf(stderr,"USAGE: %s port\n", argv[0]);																// Output error message to stderr with correct program usage
		exit(EXIT_FAILURE);																													// Exit the program with a failure status
	}
}

/*******************************************************************************
* Description: set_address_struct function
* Function is called to built the sockadd_in structure.  It receives a pointer
* to the sockaddr_in structure and a pointer to the user's command line
* arguments.  Most of this code was obtained from the server.c file
*******************************************************************************/

void set_address_struct(struct sockaddr_in* serverAddress, char** argv) {
	int portNumber = 0;																														// Establish an integer variable to store port number
	// Set up the address struct for this process (the server)
	memset((char *)serverAddress, '\0', sizeof(serverAddress)); 									// Clear out the address struct
	portNumber = atoi(argv[1]); 																									// Get the port number, convert to an integer from a string
	serverAddress->sin_family = AF_INET; 																					// Create a network-capable socket
	serverAddress->sin_port = htons(portNumber); 																	// Store the port number
	serverAddress->sin_addr.s_addr = INADDR_ANY; 																	// Any address is allowed for connection to this process
}

/*******************************************************************************
* Description: set_socket function
* Function creates a socket that will be used to list for incoming traffic on
* the network.  Most of this code was obtained from the server.c file
*******************************************************************************/

void set_socket(int* listenSocketFD) {
	// Set up the socket
	*listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); 														// Create the socket
	if (listenSocketFD < 0) {																											// If socket not successfully created
		error("ERROR opening socket");																							// Output error message to stderr and exit program
	}
}

/*******************************************************************************
* Description: enable_socket function
* Function is used to bind and enable the socket and port information.  It
* receive the sockaddr_in structure and the cosket information.  Most of this
* code was obtained from the server.c file.
*******************************************************************************/

void enable_socket(struct sockaddr_in serverAddress, int listenSocketFD) {
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress,
	sizeof(serverAddress)) < 0) { 																								// Connect socket to port
		error("ERROR on binding");																									// If binding failed, output error message to stderr and exit program
	}
	listen(listenSocketFD, 5);																									 	// Flip the socket on - it can now receive up to 5 connections
}

/*******************************************************************************
* Description: accept_connection function
* Function accepts the incoming communication from the client and moves the
* connection from the listening port to another communication port.  Most of
* this code was obtained from the server.c file
*******************************************************************************/

void accept_connection(struct sockaddr_in* clientAddress, int* establishedConnectionFD, int listenSocketFD) {
	socklen_t sizeOfClientInfo;																									  // Establish a socklen_t variable to store size of client information.
	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); 																		// Get the size of the address for the client that will connect
	*establishedConnectionFD = accept(listenSocketFD,															// Call accept function to establish communication socket with the client
		(struct sockaddr *)clientAddress, &sizeOfClientInfo);
	if (establishedConnectionFD < 0) {																						// If there was an error while creating the connection socket
		fprintf(stderr, "ERROR: on accept with client\n");													// Print an error to stderr on the server
	}
}

/*******************************************************************************
* Description: check_availability function
* Function check the total number of running processes and either approves
* or rejects the clients connection.  The server can only manage a total of
* five individual processes.
*******************************************************************************/

void check_availability(int ecFD, struct process_info proc) {
	receive_message(ecFD);																												// Call function to receive message from client
	if(proc.active < MAX_PROCESSES){																							// Check if active process are less than maximum (5)
		send_message(ecFD, "AVAILABLE");																						// Let the client know that the server is available
	} else {
		send_message(ecFD, "OCCUPIED");																							// Let the client know that the server is at capacity
		close_connection(&ecFD);																										// Call the function to close the connection with the client
		exit(EXIT_FAILURE);																													// Exit from the child process and return to the parent
	}
}

/*******************************************************************************
* Description: introduction function
* Function is called for the server and the client to send their process
* information and determine if they've connected to a legal process.  The
* function receives an integer variable containing the socket information.
* No return variables.
*******************************************************************************/

void introduction(int establishedConnectionFD) {
	char process_name[MSG_BUFFER];																								// Establish a character array to store the process name
	memset(process_name, '\0', MSG_BUFFER);																				// Clear the character array and set all elements to null
	char approved[MSG_BUFFER];																										// Establish a character array to store approval status
	memset(approved, '\0', MSG_BUFFER);																						// Clear the character array and set all elements to null
	strncpy(process_name, receive_message(establishedConnectionFD), MSG_BUFFER);	// Copy the received message to the process_name buffer
	send_message(establishedConnectionFD, OTP_ENC_D);															// Send a confirmation to the client
	strncpy(approved, receive_message(establishedConnectionFD), MSG_BUFFER);
	if((strcmp(process_name, OTP_ENC) == 0) && (strcmp(approved, "APPROVED")			// If the process name is correct and the client has approved
	== 0)) {
		send_message(establishedConnectionFD, "APPROVED");													// Send approved message to the client
	} else {																																			// Otherwise
		// fprintf(stderr, "Connection Refused: Illegal Process %s\n", process_name);	// Print an error message to stderr
		send_message(establishedConnectionFD, "REJECTED");													// Send a rejected message to the client
		close_connection(&establishedConnectionFD);																	// Close the connection with the client
		exit(EXIT_FAILURE);																													// Exit the client process with an failure status and return to the parent
	}
}

/*******************************************************************************
* Description: receive_file_text function
* Function receives the socket information (int) and either the file character
* arrays of plain or key files.  It loads the content information onto a
* character array 1000 characters and appends that information to the end of the
* final text character array.
*******************************************************************************/

void receive_file_text(int establishedConnectionFD, char* file){
	char text[MAX_MSG_SIZE];																											// Establish a character array to store the fill Message
	memset(text, '\0', MAX_MSG_SIZE);																							// Clear the array and set all elements to null
	char tmp_buffer[MSG_BUFFER];																									// Establish a character array to store temporary messages segements
	do {																																					// While we have not received the termination signal
		memset(tmp_buffer, '\0', MSG_BUFFER);																				// Clear the array for the temporary message buffer and set all elements to null
		strncpy(tmp_buffer, receive_message(establishedConnectionFD), MSG_BUFFER);	// Copy the received message into the temporary message buffer
		if(strcmp(tmp_buffer, TERM_SIG) != 0) {																			// If the message was not the termination signal
			strcat(text, tmp_buffer);																									// Append the temp buffer to the end of the full text character array
			send_message(establishedConnectionFD, "Message was RECEIVED");						// Send confirmation to the client that message was received
		} else {																																		// Otherwise
			send_message(establishedConnectionFD, "TERM_SIG was RECEIVED");						// Send message to client that the termination signal was received
		}
	} while(strcmp(tmp_buffer, TERM_SIG) != 0);																		// Continue until we have received the termination signal
	strncpy(file, text, MAX_MSG_SIZE);																						// Copy the final string to the passed file character array
}

/*******************************************************************************
* Description: decryption function
* Function receives the key and plain text information to create an decrypted
* plain text string. The plaintext is sent to the client over the network.
*******************************************************************************/

void decryption(int establishedConnectionFD, char* plain, char* key) {
	int 	i = 0,																																	// Establish an integer variable for loop counter
				number = 0,																															// Establish an integer variable to store the number
				plain_encryption = 0,																										// Establish an integer variable to store the ciphtext encryption value
				key_encryption = 0;																											// Establish an integer variable to store the key encryption value
	char 	ciphertext[strlen(plain) + 1];																					// Establish a character array to store the plain text
	memset(ciphertext, '\0', strlen(plain) + 1);																	// Initialize the ciphertext array and set all elements to null

	for(i = 0; i < strlen(plain); i++) {																					// Loop through each character in the key an ciphertext array
		plain_encryption = (int)plain[i];																						// the ascii integer value of the current ciphertext array element is stored
		key_encryption = (int)key[i];																								// the ascii integer value of the current key text array element is stored
		if(plain_encryption == 32) {																								// If ciphertext character is a space (ASCII: 32)
			plain_encryption = 64;																										// Reassign it to the lower bounds of the uppercase alphabet (ASCII: 64)
		}
		if(key_encryption == 32) {																									// If key text character is a space (ASCII: 32)
			key_encryption = 91;																											// Reassign it to the upper bounds of the uppercase alphabet (ASCII: 91)
		}
		key_encryption -= 65;																												// Move Number back to standard alphabet values including space (1 - 27)
		plain_encryption -= 65;																											// Move Number back to standard alphabet values including space (1 - 27)
		number = plain_encryption - key_encryption;																	// Add the key and ciphertext text values together and store
    if (number < 0) {																														// If the result is less than zero (negative number)
      number += 27;																															// Increase the value by 27 (number of alpha/space characters)
    }
    number += 65;																																// Move the number back into the ASCII range of uppercase letters
    if (number == 91 || number == 64)	{																					// If the number is a value assigned to space earlier
			number = 32;																															// Convert the number to the a space character (ASCII 32)
		}
		ciphertext[i] = (char)number;																								// Assign character value of the number to the current element in the plain text array
	}
	send_decryption(establishedConnectionFD, ciphertext);													// Call function to send the decrypted text to the client
}

/*******************************************************************************
* Description: send_encryption function
* Function sends the decrypted plaintext to the client.  It will
* continually loop and send 1000 character messages until it reaches the end of
* the message string.  Once finished, the function will send the termination
* characters of '!!'.
*******************************************************************************/

void send_decryption(int establishedConnectionFD, char* ciphertext) {
	int 	sent = 0,																																// Establish an integer variable to store the total number of sent characters
				i = 0;																																	// Establish an integer variable to store the counter for the 1000 character message
	char 	tmp_buffer[MSG_BUFFER];																									// Establish a character array to store the 1000 character message segments
	receive_message(establishedConnectionFD);																			// Receive message from client letting you know we can proceed sending file
	while(sent < strlen(ciphertext)) {																						// While we have not sent all characters from the message string
		i = 0;																																			// Reset counter variable to zero
		memset(tmp_buffer, '\0', sizeof(tmp_buffer));																// Reset the temporary message buffer and assign all element to null
		while((i < MSG_BUFFER - 1) && (sent < strlen(ciphertext))) {								// While we're less than the message buffer and have not reached the end of the message string
			tmp_buffer[i] = ciphertext[sent];																					// add the character to the temporary array from the original message
			sent ++;																																	// Increment the total number of sent characters
			i ++;																																			// Increment the counter toward the 1000 character limit
		}
		send_message(establishedConnectionFD, tmp_buffer);													// Send the temporary message
		receive_message(establishedConnectionFD);																		// Receive confirmation from the client
	}
	send_message(establishedConnectionFD, TERM_SIG);															// Send termination signal to the client
	receive_message(establishedConnectionFD);																			// Receive confirmation from the client
}

/*******************************************************************************
* Description: receive_message function
* Function is called to receive messages from the client.  It receives the
* integer variable with the socket information.  It has a buffer that stores
* the string produced from the recv() function.  The buffer value is returned
* to the calling function.
*******************************************************************************/

char* receive_message(int establishedConnectionFD){
	static char buffer[MSG_BUFFER];																								// Establish a character string to capture received messages (1000 character capacity)
	int charsRead = 0;																														// Establish an integer variable for storing the number of characters read
	// Get the message from the client and display it
	memset(buffer, '\0', MSG_BUFFER);																							// Clear the character array and set all elements to null
	charsRead = recv(establishedConnectionFD, buffer, MSG_BUFFER, 0); 						// Read the client's message from the socket
	if (charsRead < 0) {																													// If there was an error receiving the messages
		fprintf(stderr, "ERROR: Failed reading from socket\n");											// Print an error message to stderr
		close_connection(&establishedConnectionFD);																	// Close connection with the client
		exit(EXIT_FAILURE);																													// Exit the client process with an failure status and return to parent
	}
	return buffer;																																// Return the received message to the calling function
}

/*******************************************************************************
* Description: send_message function
* Function is called to send messages to the client.  It receives an integer
* variable containing the scoket infromation and a string containing the
* message.  No returned variables
*******************************************************************************/

void send_message(int establishedConnectionFD, char* message) {
	int charsRead = 0;																														// Establish an integer variable for storing the number of characters read
	charsRead = send(establishedConnectionFD, message, strlen(message), 0);				// Send the passed message to the client
	if (charsRead < 0) {																													// If there was an error sending information
		fprintf(stderr, "ERROR: Failed writing to socket\n");												// Print and error message to stderr
		close_connection(&establishedConnectionFD);																	// Close the connection with the client
		exit(EXIT_FAILURE);																													// Exit the child process with a failure status
	}
	if (charsRead < strlen(message)) {																						// If the number of characters sent was less than the received string length
		fprintf(stderr, "WARNING: Not all data written to sockets\n");							// Print a warning to stderr but continue operations
	}
}

/*******************************************************************************
* Description: check_processes function
* Funtion is called to check for processes that were forked off from the parent.
* When it finds a child pid that has terminated, it calls another function, to
* remove that process from the array and decrement the total number of running
* processes.
*******************************************************************************/

void check_processes(pid_t child_id, int child_exit, struct process_info* proc) {
	while ((child_id = waitpid(-1, &child_exit, WNOHANG)) > 0) {									// Call waipid function without hanging parent.  Check for terminated background processes
		remove_process(child_id, proc);																							// Call function to remove process from tracking structure
	}
}

/*******************************************************************************
* Description: add_process function
* Function receives the process_info structure and a child_id pid.  It searches
* the pid array for the matching child_id and adds that child id to the first
* available (null) element in the process array. Function also increments total
* number of processes once complete.
*******************************************************************************/

void add_process(pid_t child_id, struct process_info* proc) {
	int i = 0;																																		// Establishes integer variable for counter
	for(i = 0; i < MAX_PROCESSES; i ++) {																					// Loops through each element of the structure process array
		if(proc->processes[i] == '\0'){																							// If the current element in the array matches the received child id
			proc->processes[i] = child_id;																						// Assign the child id to that element in the array
			proc->active ++;																													// Increment the total number of processes that are running
			break;																																		// Break the loop once we've found what we're looking for
		}
	}
}

/*******************************************************************************
* Description: remove_process function
* Function receives the process_info structure and a child_id pid.  It searches
* the pid array for the matching child_id and removes that element (replaces
* with a null value).  Function also decrements total number of processes once
* complete.
*******************************************************************************/

void remove_process(pid_t child_id, struct process_info* proc) {
	int i = 0;																																		// Establishes integer variable for counter
	for(i = 0; i < MAX_PROCESSES; i ++) {																					// Loops through each element of the structure process array
		if(proc->processes[i] == child_id){																					// If the current element in the array matches the received child id
			proc->processes[i] = '\0';																								// Remove that child id and reassign the element to null
			proc->active --;																													// Decrement the total number of processes that are running
			break;																																		// Break the loop once we've found what we're looking for
		}
	}
}

/*******************************************************************************
* Description: close_connection function
* Function is called to closed the connection with the client.  It receives
* an integer pointer to the socket information.
*******************************************************************************/

void close_connection(int* establishedConnectionFD) {
	close(*establishedConnectionFD); 																							// Close the existing socket which is connected to the client
}

/*******************************************************************************
* Description: close_listen_socket function
* Function receives an integer pointer to the listening socket information and
* passes it to the close() function to terminate the socket.
*******************************************************************************/

void close_listen_socket(int* listenSocketFD) {
	close(*listenSocketFD); 																											// Close the listening socket
}

/*******************************************************************************
* Description: error function
* Error function is used to write messages to perror and terminate the program.
* It receives a character array containing the message to pass to the perror
* function.
*******************************************************************************/

void error(const char *msg) {
	perror(msg);  																																// Error function used for reporting issues
	exit(EXIT_FAILURE);																														// Exit Program with failure status
}
