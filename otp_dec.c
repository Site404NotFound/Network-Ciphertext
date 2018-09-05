/*******************************************************************************
** Author: James Hippler (HipplerJ)
** Oregon State University
** CS 344-400 (Spring 2018)
** Operating Systems I
**
** Description: Program 4 - OTP (Block 4)
** Due: Sunday, June 10, 2018
**
** Filename: otp_dec.c
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
** otp_dec: Similarly, this program will connect to otp_dec_d and will ask it
** to decrypt ciphertext using a passed-in ciphertext and key, and otherwise
** performs exactly like otp_enc, and must be runnable in the same three ways.
** otp_dec should NOT be able to connect to otp_enc_d, even if it tries to
** connect on the correct port - you'll need to have the programs reject each
** other, as described in otp_enc.
*******************************************************************************/

/*******************************************************************************
** PREPROCESSOR DIRECTIVES
*******************************************************************************/

#include <stdio.h>                                                              // Include the stdio.h library
#include <stdlib.h>                                                             // Include the stdlib.h library
#include <unistd.h>                                                             // Include the unistd.h library
#include <string.h>                                                             // Include the string.h library
#include <sys/types.h>                                                          // Include the sys/types.h library
#include <sys/socket.h>                                                         // Include the sys/socket.h library
#include <netinet/in.h>                                                         // Include the netinet/in.h library
#include <netdb.h>                                                              // Include the netdb.h library
#include <ctype.h>                                                              // Include library necessary for isalpha and isspace functions

/*******************************************************************************
** GLOBAL CONSTANT DECLARATIONS
*******************************************************************************/

#define OTP_ENC "otp_dec"                                                       // Declare global constant for the process name "otp_dec"
#define OTP_ENC_D "otp_dec_d"                                                   // Declare global constant for the process name "otp_dec_d"
#define SERVER_HOSTNAME "localhost"                                             // Declare global constant for the server hostname "localhost"
#define MSG_BUFFER 1000                                                         // Declare global constant for total message transmission size
#define TERM_SIG "!!"                                                           // Declare global constatn for the transmission termination signal

/*******************************************************************************
** STRUCTURE DECLARATIONS
*******************************************************************************/

// structure to store file information such as size, name, and content
struct file_info{
  char* file_name;                                                              // Establish variable to store character array for file file name
	char* content;                                                                // Establish variable to store character array for file content
  int size;                                                                     // Establish variable to store the number of characters in the file
};

/*******************************************************************************
** FUNCTION DECLARATIONS
*******************************************************************************/

void check_arguments(int, char**);                                              // Declare function to check arguments
void init_files(struct file_info*, char**, int);                                // Declare function to initialize the file structures
void set_file_names(struct file_info*, char*);                                  // Declare function to set the file names
void open_file(char*, FILE**);                                                  // Declare function to open the file and store pointer
void set_file_size(int*, FILE*);                                                // Declare function to set the file size
void read_files(char**, int, FILE*);                                            // Declare function to read through the files
void close_file(FILE**);                                                        // Declare function to close file pointer
void remove_new_line(struct file_info*);                                        // Declare function to remove new line characters
void confirm_chars(struct file_info);                                           // Declare function to check for bad characters
void compare_size(int, int, char*);                                             // Declare function to compare the key and plain text file sizes
void set_server_address(struct sockaddr_in*, struct hostent*, char**);          // Declare function to build the server struct informations
void set_socket(int*);                                                          // Declare function to build a socket for communication
void connect_server(int, struct sockaddr_in);                                   // Declare function to connect to the server
void check_availability(int);                                                   // Declare function to check if server is available
void introduction(int, char**);                                                 // Declare function to introduce server and client process information
void send_file_text(int, struct file_info);                                     // Declare function to send both key and plain text files
void receive_encryption(int, int);                                              // Declare function to receive the ciphertext from server
void send_message(int, char*, int);                                             // Declare function to send messages to the server
char* receive_message(int);                                                     // Declare function to receive messages from the server
void close_connection(int*);                                                    // Declare function to close socket connection with the server
void error(const char*);                                                        // Declare function to print error messages and terminate program

/*******************************************************************************
* Description: main function
* This is the main function of the program.  The program starts here and the
* main function calls all other functions in the appropriate order.  The
* function receives an integer for the argument total, and a char array for the
* actual arguments received on the command line.  Returns 0 and exits if
* program works successfully.
*******************************************************************************/

int main(int argc, char *argv[]) {
  int socketFD = 0;                                                             // Establish an integer variable to store socket information
	struct sockaddr_in serverAddress = { 0 };                                     // Establish an instance of the sockaddr_in struct called serverAddress
	struct hostent* serverHostInfo = NULL;                                        // Establish an instance of the hostent stuct called serverHostInfo
	struct file_info plain_file = { 0 };                                          // Create new file_info struct instance for the plain text message file
	struct file_info key_file = { 0 };                                            // Create new file_info struct instance for the key file

	check_arguments(argc, argv);                                                  // Call function to confirm that the user has input the correct number of arguments
  init_files(&plain_file, argv, 1);                                             // Call function to initialize the plain text file information
  init_files(&key_file, argv, 2);                                               // Call function to initialize the plain text file information
  compare_size(plain_file.size, key_file.size, key_file.file_name);             // Call function to ensure that the plain_text file is at most as long as the keys file
	set_server_address(&serverAddress, serverHostInfo, argv);                     // Call function to set the server's address (uses localhost)
	set_socket(&socketFD);                                                        // Call function to establish the connection socket
	connect_server(socketFD, serverAddress);                                      // Call function to connect to the server on the previously established socket
  check_availability(socketFD);                                                 // Call function to determine if server is available
	introduction(socketFD, argv);                                                 // Call function to send and receive process information with the server
	send_file_text(socketFD, plain_file);                                         // Call function to transmit the content of the plain text file
	send_file_text(socketFD, key_file);                                           // Call function to transmit the content of the key text file
	receive_encryption(socketFD, plain_file.size);                                // Call function to receive the ciphertext from the server
	close_connection(&socketFD);                                                  // Call function to close the socket and the connection with the server
	return 0;																																			// Exit the program successfully after all operations are completed
}

/*******************************************************************************
* Description: check_arguments function
* Function confirms that the user inputs the correct number of arguments
* when executing the program on the command line.
*******************************************************************************/

void check_arguments(int argc, char** argv) {
	if (argc != 4) {																															// If the user did not provide the correct number of arguments on the command line
		fprintf(stderr,"USAGE: %s <message_file> <key_file> <port>\n", argv[0]);		// Output an error message to stderr with the correct usage
		exit(EXIT_FAILURE);																													// Exit the program with a failure status.
	}
}

/*******************************************************************************
* Description: init_plain_files function
* Function coordinates the construction of the plain and key text structures.
* Initially it sets the variable information to null, then calls the appropriate
* functions to establish the file name, size, and content.  Also calls functions
* to remove the trailing newline char and confirm that no bad characters are
* present.
*******************************************************************************/

void init_files(struct file_info* file, char** argv, int argument) {
  FILE* fp = NULL;																															// Establish a File pointer and initialize to NULL
  file->file_name = NULL;                                                       // Initialize file name to null
  file->content = NULL;                                                         // Initialize file content to null
  file->size = 0;                                                               // Initialize file size to zero (0)
  set_file_names(file, argv[argument]);                                         // Call function to initialize the file name
  open_file(file->file_name, &fp);                                              // Call function to open the user specified files
  set_file_size(&file->size, fp);                                               // Call function to initialize the file size (number of characters)
  read_files(&file->content, file->size, fp);                                   // Call function to initialize the file contents
  close_file(&fp);                                                              // Call function to close the previously opened file
  remove_new_line(file);                                                        // Call function to remove the break line (\n) from the end of the string
  confirm_chars(*file);                                                         // Call function to determine if file contains illegal characters
}

/*******************************************************************************
* Description: set_file_names function
* Function receives the filename from either argv[1] (plaintext file) or from
* argv[2] (key text file) and the corresponding structure.  It assigns the
* user input as the file_name variable.
*******************************************************************************/

void set_file_names(struct file_info* file, char* file_name) {
	file->file_name = file_name;                                                  // Assign the user's second argument argv[1] as the struct file_name
}

/*******************************************************************************
* Description: open_file function
* Function receives a filename and a file pointer.  Function opens the file and
* and returns the filepointer information.
*******************************************************************************/

void open_file(char* file_name, FILE** fp) {
	*fp = fopen(file_name, "r");																				          // Open the file name that was passed from main (open as read)
  if (*fp == NULL) {																													  // If the file was not located
		fprintf(stderr, "File \"%s\"", file_name);														      // Output an error
		error(" not found");																												// Call function to include system error and exit the program
	}
}

/*******************************************************************************
* Description: read_files function
* Function records the file size (number of characters) from the file pointer
* it receives as an argument.  It seeks to the end of the file, records its
* position then rewinds the pointer back to the beginning od the file.
*******************************************************************************/

void set_file_size(int* file_size, FILE* fp) {
  fseek(fp, 0L, SEEK_END);                                                      // Move the pointer to the end of the file
  *file_size = ftell(fp);                                                       // Get the location of the pointer (total characters) and assign to struct file size
  rewind(fp);                                                                   // Move file pointer back to the beginning of the file (rewind)
}

/*******************************************************************************
* Description: read_files function
* Function uses getline() to grab the content from the file pointer that it is
* passed.  It uses the file size to create a dynamically allocated character
* array to store the file contents received from getline().  It assigns the
* character array to the appropriate structure and frees the dynamically
* allocated memory.
*******************************************************************************/

void read_files(char** file_content, int file_size, FILE* fp) {
  char* temp_str = NULL;                                                        // Establish a temporary character array
  size_t buffer_size = file_size;																								// Establish a size_t variable to be used with malloc later in the function
  temp_str = (char*)malloc((file_size + 1) * (sizeof(char)));                   // Allocate memory to the string that is equivalent to the file size
  if(temp_str == NULL){																													// If we are not able to allocate more memory via malloc()
     fprintf(stderr, "Unable to allocate memory for temporary buffer\n");				// Print an error message to stderr letting the user memory could not be allocated
     exit(EXIT_FAILURE);																												// Exit the program with an failure status
   }
  getline(&temp_str,&buffer_size, fp);																					// Grab the entire first line from the messages file
  *file_content = temp_str;                                                     // Assign the temporary string to the structure file name
  temp_str = NULL;																															// Reset the temp_str file back to NULL before freeing the memory
  free(temp_str);                                                               // Free the allocated memory (malloc) assigned to the temp character array
}

/*******************************************************************************
* Description: close_file function
* Function receives an open file pointer and closes the associated file.
*******************************************************************************/

void close_file(FILE** fp) {
  fclose(*fp);                                                                  // Close the file once finished.
  *fp = NULL;                                                                   // Reset the file pointer back to a null value
}

/*******************************************************************************
* Description: remove_new_line function
* Function receives a string as an argument and replaces trailing
* break line characters with a null terminator.
*******************************************************************************/

void remove_new_line(struct file_info* file){
  file->content[strcspn(file->content, "\n")] = '\0';                           // Replace the break line with a null terminator
	file->size --;                                                                // Decrement the file size (number of characters) by 1
}

/*******************************************************************************
* Description: confirm_chars function
* Function loops through all the characters in the character array and checks
* that it is either an uppercase letter (A - Z) or a space (" ").  It uses
* ASCII variables to confirm.
*******************************************************************************/

void confirm_chars(struct file_info f) {
  int i = 0;                                                                    // Establish an integer variable that will be used as a counter
  for(i = 0; i < f.size; i ++){                                                 // Loop through each letter in the character array
    if(((int)f.content[i] >= 65 && (int)f.content[i] <= 90) ||                  // If the character is A - Z (ASCII 65-90) or a space (32)
		((int)f.content[i] == 32)){
      continue;                                                                 // Continue through the loop to the next character
    } else {                                                                    // If a bad character is found within the string
      fprintf(stderr, "Bad character \'%c\' found in file %s\n", f.content[i],  // Output an error message to stderr with the character and the filename
			f.file_name);
      exit(EXIT_FAILURE);                                                       // Exit the program with a failure status
    }
  }
}

/*******************************************************************************
* Description: compare_size function
* Function is called to compare the file sizes of both the plaintext and key
* files.  If plain text is longer than the key file, an error will be output
* to stderr and the program will be terminated with an exit value of 1.  The
* function receives two integers for the file sizes, and a character array for
* the filename of the key.
*******************************************************************************/

void compare_size(int plain, int key, char* key_name) {
	if(key < plain) {																															// Compare the file sizes (characters) of plain text and the key
		fprintf(stderr, "Error: Key file \'%s\' is too short\n", key_name);				  // If key is too short, output error message to stderr
		exit(EXIT_FAILURE);																													// Exit the program with a failure status of 1
	}
}

/*******************************************************************************
* Description: set_server_address function
* Function is called to build a structure with the server address (localhost)
* and port information (provided on the commandline).  A majority of this code
* was used directly from the client.c file provided with the assignment.
*******************************************************************************/

void set_server_address(struct sockaddr_in* serverAddress, struct hostent* serverHostInfo, char** argv) {
	int portNumber;                                                               // Establish an integer variable to store the portnumber
	// Set up the server address struct
	memset((char*)serverAddress, '\0', sizeof(*serverAddress)); 									// Clear out the address struct
	portNumber = atoi(argv[3]);																									 	// Get the port number, convert to an integer from a string
	serverAddress->sin_family = AF_INET; 																					// Create a network-capable socket
	serverAddress->sin_port = htons(portNumber); 																	// Store the port number
	serverHostInfo = gethostbyname(SERVER_HOSTNAME); 															// Convert the machine name into a special form of address
	if (serverHostInfo == NULL) {																									// If the server host information is not set correctly
		fprintf(stderr, "ERROR: No such host\n");																		// Output error message to stderr
		exit(EXIT_FAILURE);																													// Exit the program with a failure status
	}
	memcpy((char*)&serverAddress->sin_addr.s_addr, (char*)serverHostInfo->h_addr,	// Copy in the address
	serverHostInfo->h_length);
}

/*******************************************************************************
* Description: set_socket function
* Function calls the socket function to build a socket for communicating with
* the server.  It receives an integer pointer that it modifies and returns.
* Again, most of this code was obtained in client.c
*******************************************************************************/

void set_socket(int* socketFD) {
	// Set up the socket
	*socketFD = socket(AF_INET, SOCK_STREAM, 0); 																	// Create the socket
	if (socketFD < 0) {                                                           // If socket was not successfully created
		error("CLIENT: ERROR opening socket");                                      // Print error message to stderr and exit the program
	}
}

/*******************************************************************************
* Description: connect_server function
* Function is called to use the server structure and the socket information to
* connect and communicate with the server.  Most of this code was obtained from
* client.c
*******************************************************************************/

void connect_server(int socketFD, struct sockaddr_in serverAddress) {
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, 											// Connect socket to address
	sizeof(serverAddress)) < 0) {                                                 // If the connection function returns a value that is less than zero, an error occurred
		error("CLIENT: ERROR connecting");                                          // Call the error function and send the appropriate error message
	}
}

/*******************************************************************************
* Description: check_availability function
* Function communicates with the server to determine if there is an availability
* for establishing another connection (server can only have 5 open).  If there's
* an available slot, then the connection will occur.
*******************************************************************************/

void check_availability(int socketFD) {
  char available[MSG_BUFFER];                                                   // Establish a character array to store the server response
  memset(available, '\0', MSG_BUFFER);                                          // Clear the response arrays memory and set all elements to null
  send_message(socketFD, "Attempting Connection", 21);                          // Send message to server letting it know we're trying to connect
  strncpy(available, receive_message(socketFD), MSG_BUFFER);                    // Receive response from server
  if(strcmp(available, "OCCUPIED") == 0) {                                      // If the response is "OCCUPIED"
    fprintf(stderr, "REJECTED: Server at capacity\n");                          // Print and error message to stderr
    close_connection(&socketFD);																								// Close the communication socket that was opened
    exit(EXIT_FAILURE);																													// Exit the program with a failure status
  }
}

/*******************************************************************************
* Description: send_message function
* Function is called for the server and the client to send their process
* information and determine if they've connected to a legal process.  The
* function receives an integer variable containing the socket information.
* No return variables.
*******************************************************************************/

void introduction(int socketFD, char** argv) {
	char approved[MSG_BUFFER];                                                    // Establish a character array to store the approval message
	memset(approved, '\0', MSG_BUFFER);                                           // Clear the array and set all elements to null
	char process_name[MSG_BUFFER];																								// Establish a character array to store the process information
	memset(process_name, '\0', MSG_BUFFER);                                       // Clear the array and set all elements to null
	send_message(socketFD, OTP_ENC, strlen(OTP_ENC));															// Call the send_message() function to send the process name (otp_dec)
	strncpy(process_name, receive_message(socketFD), MSG_BUFFER);								  // Call the receive_message() function to get the process name
	if (strcmp(process_name, OTP_ENC_D) != 0){																		// If connection was not made with the appropriate process
		send_message(socketFD, "REJECTED", strlen("REJECTED"));                     // Send the message Rejected to the server
		receive_message(socketFD);                                                  // Call the receive_message function to get server message
    fprintf(stderr, "Illegal connection with process \"%s\" on Port %s\n",    	// Output an error message to stderr
     process_name, argv[3]);
		close_connection(&socketFD);																								// Close the communication socket that was opened
		exit(EXIT_FAILURE);																													// Exit the program with a failure status
	} else {                                                                      // Otherwise...
		send_message(socketFD, "APPROVED", strlen("APPROVED"));                     // Send a message to the server saying it was approved
		strncpy(approved, receive_message(socketFD), MSG_BUFFER);                   // Receive the server's response
		if(strcmp(approved, "REJECTED") == 0) {                                     // If the server sent a rejected message
			fprintf(stderr, "Illegal process rejected by server on Port %d\n",        // Print an error message to stderr
      socketFD);
			close_connection(&socketFD);                                              // Close the socket Connection
			exit(2);                                                                  // Exit the program with an error value of 2
		}
	}
}

/*******************************************************************************
* Description: send_file_text function
* Function receives the socket information (int) and either the file structure
* plain or key files. It loads the content information onto a character array
* 1000 characters at at time and sends that information to the server.
*******************************************************************************/

void send_file_text(int socketFD, struct file_info f) {
	int 	sent = 0,                                                               // Establish an integer variable to store how many total character were sent
				i = 0;                                                                  // Establish an integer variable that is used as a counter to 1000
	char 	tmp_buffer[MSG_BUFFER];                                                 // Establish a character array that will store the 1000 character message
	while(sent < f.size) {                                                        // While the total number of sent characters is less than the file content size
		i = 0;                                                                      // Reset the counter variable to zero
		memset(tmp_buffer, '\0', sizeof(tmp_buffer));                               // Clear the message buffer and set all elements to null
		while((i < MSG_BUFFER - 1) && (sent < f.size)) {                            // While the counter is less than 1000 and total sent characters is less than the file size
			tmp_buffer[i] = f.content[sent];                                          // Add the character from the file to the message array
			sent ++;                                                                  // Increment the total characters sent value by one
			i ++;                                                                     // Increment the message size counter by one
		}
		send_message(socketFD, tmp_buffer, MSG_BUFFER);                             // Send the message once it reaches 1000 bytes or has reached the end of the content array
		receive_message(socketFD);                                                  // Receive server's response.  This just confirms that the server has received the file
	}
	send_message(socketFD, TERM_SIG, strlen(TERM_SIG));                           // Once all content has been sent.  Send the termination signal to end process
	receive_message(socketFD);                                                    // Receive the server's response
}

/*******************************************************************************
* Description: receive_encryption function
* Function receives the encrypted ciphertext from the server.  It will
* continually loop and receive 1000 character messages until the server sends
* the termination characters of '!!'.  The function will print the encryption
* key to stdout to either be redirected to a file or printed to the console
*******************************************************************************/

void receive_encryption(int socketFD, int length) {
	char text[length];                                                            // Establish a character array to store the entire received message
	memset(text, '\0', length);                                                   // Clear the array and set all elements to null values
	char tmp_buffer[MSG_BUFFER];                                                  // Establish a tmp character array to store each 1000 character message
	memset(tmp_buffer, '\0', MSG_BUFFER);                                         // Clear the array and set all elements to null values
	send_message(socketFD, "Ready to Receive Key", MSG_BUFFER);                   // Send message to server that we are ready to receive ciphertext
	do {                                                                          // Loop until the correct conditions are met
		memset(tmp_buffer, '\0', MSG_BUFFER);                                       // Clear the temporary buffer and set all element to null values
		strncpy(tmp_buffer, receive_message(socketFD), MSG_BUFFER);                 // Receive message from server and set to tmp character array
		if(strcmp(tmp_buffer, TERM_SIG) != 0) {                                     // If the message was not the termination signal
			strcat(text, tmp_buffer);                                                 // Add the content to the end of the full text character array
			send_message(socketFD, "Message was RECEIVED", 20);                       // Let server know the message was received
		} else {                                                                    // If the termination signal was received
			send_message(socketFD, "TERM_SIG was RECEIVED", 21);                      // Let the user know that the termination signal was received
		}
	} while(strcmp(tmp_buffer, TERM_SIG) != 0);                                   // Continue looping until the termination signal is received
	fprintf(stdout, "%s\n", text);                                                // Print the finialized ciphertext message to stdout
}

/*******************************************************************************
* Description: send_message function
* Function is called to send messages to the server.  It receives an integer
* variable containing the scoket infromation, a string containing the message
* and an integer variable with the message size.  No returned variables
*******************************************************************************/

void send_message(int socketFD, char* msg_content, int msg_size) {
	int charsWritten = 0;                                                         // Establish an integer variable to recored the characters sent
	// Send message to server
	charsWritten = send(socketFD, msg_content, msg_size, 0); 											// Write to the server
	if (charsWritten < 0) {                                                       // If an error occurred when transmitting the message
		error("ERROR: Failed writing to socket");                                   // Call the error function and send the appropriate error message
	}
	if (charsWritten < msg_size) {                                                // If the number of characters sent is less than the total message length
		fprintf(stderr, "WARNING: Not all data written to socket\n");               // Print a Warning message to stderr but do not exit the program
	}
}

/*******************************************************************************
* Description: receive_message function
* Function is called to receive messages from the server.  It receives the
* integer variable with the socket information.  It has a buffer that stores
* the string produced from the recv() function.  The buffer value is returned
* to the calling function.
*******************************************************************************/

char* receive_message(int socketFD) {
	int charsRead = 0;                                                            // Establish a integer variable to record the number of characters received
	static char buffer[MSG_BUFFER + 1];                                           // Establish a character array to store the string received from the server
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); 																				// Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer), 0); 												// Read data from the socket, leaving \0 at end
	if (charsRead < 0) {                                                          // If an error occurred reading network message
		error("ERROR: Failed reading from socket");                                 // Call the error function to print an error message and exit the program
	}
	return buffer;                                                                // Return the character string that was received from the server
}

/*******************************************************************************
* Description: close_connection function
* Function receives socket information and closes the connection.  Receives
* a pointer to an integer value storing the socket information.
*******************************************************************************/

void close_connection(int* socketFD){
	close(*socketFD); 																														// Close the socket
}

/*******************************************************************************
* Description: error function
* Function is called when an error has occurred that requires the system to
* exit and report a failure status.  Function receives a string that contains
* the error message that needs to be printed.
*******************************************************************************/

void error (const char *msg) {
  perror(msg);                                                                  // Error function used for reporting issues
  exit(EXIT_FAILURE);                                                           // Exit the program and report an error status
}
