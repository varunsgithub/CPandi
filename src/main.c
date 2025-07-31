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
