/*******************************************************************************
** Author: James Hippler (HipplerJ)
** Oregon State University
** CS 344-400 (Spring 2018)
** Operating Systems I
**
** Description: Program 4 - OTP (Block 4)
** Due: Sunday, June 10, 2018
**
** Filename: keygen.c
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
** keygen: This program creates a key file of specified length. The characters
** in the file generated will be any of the 27 allowed characters, generated
** using the standard UNIX randomization methods. Do not create spaces every
** five characters, as has been historically done. Note that you specifically
** do not have to do any fancy random number generation: we’re not looking for
** cryptographically secure random number generation! rand() is just fine. The
** last character keygen outputs should be a newline. All error text must be
** output to stderr, if any.
**
** The syntax for keygen is as follows:
** keygen keylength
**
** Where keylength is the length of the key file in characters. keygen outputs
** to stdout. Here is an example run, which redirects stdout to a key file of
** 256 characters called “mykey” (note that mykey is 257 characters long because
** of the newline):
*******************************************************************************/

/*******************************************************************************
** PREPROCESSOR DIRECTIVES
*******************************************************************************/

#include <stdio.h>                                                              // Include the stdio.h library
#include <stdlib.h>                                                             // Include the stdlib.h library
#include <string.h>                                                             // Include the string.h library
#include <time.h>                                                               // Include the time.h library
#include <ctype.h>                                                              // Include the ctype.h library

/*******************************************************************************
** GLOBAL CONSTANT DECLARATIONS
*******************************************************************************/

#define MAX_CHARACTERS 27                                                       // Global constant for the maximum number of characters (27 total)

/*******************************************************************************
** FUNCTION DECLARATIONS
*******************************************************************************/

void random_time();                                                             // Function to generate random time based on system clock
int confirm_input(int, char const**);                                           // Function used to confirm the user input the correct arguments
void generate_key(int);                                                         // Function to generate and output the key to stdout

/*******************************************************************************
* Description: main function
* Main function establish a character array to store letters and calls the
* necessary functions in the appropriate order to successfull orchestrate the
* program.
*******************************************************************************/

int main(int argc, char const *argv[]) {
  random_time();                                                                // Call function to initialize the randomizer to be based of the system clock
  generate_key(confirm_input(argc, argv));                                      // Call function to generate keys.  First calls function to confirm input and return length
  return EXIT_SUCCESS;                                                          // Successfully exit the program.  No errors encountered in the process
}

/*******************************************************************************
* Description: random_time function
* Function is called to set up our randomizer to be based on the system
* clock time.  This makes randomization more accurate and variable with
* each run of the program.
*******************************************************************************/

void random_time() {
  srand(time(NULL));                                                            // Seed time to randomizer to ensure most possible randomization
}

/*******************************************************************************
* Description: confirm_input function
* Function determines if the input for the keygen.c program is correct.
* The program requires two arguments (name of the program & key length)
* and the second argument must be an integer value (digit).  Function
* receives argc and argv from the command line.  Returns the length of
* the key (argv[1]).
*******************************************************************************/

int confirm_input(int argc, char const** argv) {
  int length = 0,                                                               // Establish an integer variable to store the key length
      i = 0;                                                                    // Establush an integer variable for counter in for loop
  if(argc != 2) {                                                               // If there were not exactly two arguments provided
    fprintf(stderr, "Incorrect number of arguments received\n");                // Print an error message to stderr
    exit(EXIT_FAILURE);                                                         // Exit the program with a failure status
  }
  for (i = 0; i < strlen(argv[1]); i ++) {                                      // Loop through each character in second argument (argv[1])
    if (!isdigit(argv[1][i])) {                                                 // If the character is not a digit
      fprintf(stderr, "%s: Input is not a valid key length\n", argv[1]);        // Print an error message to stderr
      exit(EXIT_FAILURE);                                                       // Exit the program with a failure status
    }
  }
  length = atoi(argv[1]);                                                       // Assign argument 2 to key length as an integer
  return length;                                                                // Return key length to main
}

/*******************************************************************************
* Description: generate_key function
* Function takes a character array of all 26 capital letters and space
* character (27 characters total).  It uses a randomizer function to pick
* those characters randomly and print them to stdout.  It does this as many
* times as was specified by the user on execution plus once more to also print
* a breakline '\n' character at the end.
*******************************************************************************/

void generate_key(int length) {
  char letters[MAX_CHARACTERS] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',  // Establish Character array of 27 to store all letters and a space " " character
                                  'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R',
                                  'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '};
  int i = 0;                                                                    // Establish an integer variable for counter in for loop
  for(i = 0; i < length ; i ++) {                                               // Loop through each element in the key array
    fprintf(stdout, "%c", letters[rand() % MAX_CHARACTERS]);                    // Randomly print a letter (or space) from the letters array
  }
  fprintf(stdout, "%c", '\n');                                                  // Add the newline character to the last element in the key array
}
