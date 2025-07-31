#ifndef cpandi_vm_h
#define cpandi_vm_h

#include "chunk.h"
#include "value.h"
#define STACK_MAX 256

/* Defining a data structure to keep a track of the state of the VM */
typedef struct {
    Chunk* chunk;
    //To keep a track of the location of the bytecode
    //instruction pointer -> IP
    //This points to the chunk array
    uint8_t* ip;
    //Creating a VM stack for interpreting instructions
    Value stack[STACK_MAX];
    Value* stackTop;
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
InterpretResult interpret(const char* source);

/*Stack operation to push a value on the stack*/
void push(Value value);

/*Stack operation to remove and return the topmost value from the stack*/
Value pop();

static InterpretResult run();

static void resetStack();

#endif