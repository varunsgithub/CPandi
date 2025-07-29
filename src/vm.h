#ifndef cpandi_vm_h
#define cpandi_vm_h

#include "chunk.h"

/* Defining a data structure to keep a track of the state of the VM */
typedef struct {
    Chunk* chunk;
    //To keep a track of the location of the bytecode
    //instruction pointer -> IP
    //This points to the chunk array
    uint8_t* ip;
} VM;

/*Return values for the result of the interpretation of the VM*/
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

/*Method to initialize the VM*/
void initVM();
/*Method to clean the VM*/
void freeVM();
/*Method to interpret the bytecode*/
InterpretResult interpret(Chunk* chunk);

static InterpretResult run();

#endif