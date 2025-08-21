#ifndef cpandi_vm_h
#define cpandi_vm_h

#include "chunk.h"
#include "table.h"
#include "value.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/*Data structure to keep track of the Call frame*/
typedef struct {
    ObjFunction* function;
    //the caller stores the return address so that the function can jump here once
    //the callee finishes its execution
    uint8_t* ip;
    //The first slot in the stack that we had reserved -> is used by the function
    Value* slots;
} CallFrame;

/* Defining a data structure to keep a track of the state of the VM */
typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    //Creating a VM stack for interpreting instructions
    Value stack[STACK_MAX];
    Value* stackTop;
    //This is a hash table for storing the global variable
    Table globals;
    //The objects is an object pointer which is the head of our linked list !
    Table strings;
    Obj* objects;
} VM;

/*Return values for the result of the interpretation of the VM*/
typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

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