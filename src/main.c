#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

/*Declaring the REPL function here to help run the code from the command line*/
static void repl() {
  //Creates a char array of 1024 characters !
  char line[1024];
  
  for (;;) {
    //print the catface asking for line of code on each line
    printf("(˃ᆺ˂): ");

    //if you do not read anything then break out of the for loop
    if (!fgets(line, sizeof(line), stdin)) {
      //print a newline
      printf("\n");
      break;
    }

    //else keep interpreting the line !
    interpret(line);
  }
}

/*The runfile method will read the file contents and interpret the results*/
static void runFile(const char* path) {
  //The source of the file, is stored as a char* array !
  char* source = readFile(path);
  //The result of the interpret result is stored in a result factor
  InterpretResult result = interpret(source);
  //free the source pointer !!
  free(source);

  //if the result results into a compile error/ runtime error -> Exit 
  if (result == INTERPRET_COMPILE_ERROR) exit(65);
  if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

/*The read file method opens the file path and reads the entire content of the file*/
static char* readFile(const char* path) {
  //The file pointer opens the path with read binary !
  FILE* file = fopen(path, "rb");

  //If you are not able to open the file then exit the program !
  if (file == NULL) {
    fprintf(stderr, "Could not open the file \"%s\".\n", path);
    exit(74);
  }

  
  //This sets the file pointer to the end of the file !
  fseek(file, 0l, SEEK_END);
  //The ftell function tells -> the size of the file !!
  size_t fileSize = ftell(file);
  //rewind the file to the beginning position
  rewind(file);

  //a char* buffer is created using the malloc with file size + 1
  char* buffer = (char*) malloc (fileSize + 1);
  //If the malloc fails -> then print the error and exit !
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read \%s\".\n", path);
    exit(74);
  }
  
  
  //The bytesRead -> Read the buffer with the size of the file size
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  //If you cant read (read fails) -> then exit :(
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }
  
  
  buffer[bytesRead] = '\0';
  //fclose the file !
  fclose(file);
  return buffer;
}



int main (int argc, const char* argv[]) {
    //Initialize a VM when the program runs
    initVM();

    //If there is no argument provided to the code then run the REPL
    if (argc == 1) {
      repl();
    } else if (argc == 2) {
      //If there is one argument then run the code from the file
      runFile(argv[1]);
    } else {
      //Else syntax error -> use exit code 64 (incorrect syntax) and exit
      fprintf(stderr, "Usage: clox [path]\n");
      exit(64);
    }
    
    //Free the VM when exiting
    freeVM();


    return 0;
}
