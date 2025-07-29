#include "common.h"
#include "vm.h"
#include <stdio.h>
#include "debug.h"


/*Defining a global variable called VM !*/
VM vm;

static void resetStack() {
    //This shows that the stack is empty since the stackTop points to 0
    vm.stackTop = vm.stack;
}

void initVM() {
    resetStack();
}

void freeVM() {}

void push(Value value) {
    //Dereference the top and put a value
    *vm.stackTop = value;
    //incrmeent the pointer.
    vm.stackTop++;
}

Value pop() {
    //Decrement the pointer to go to the value below
    //since the pointer has gone below, the current value where it points
    //becomes inaccessible !!!
    vm.stackTop--;
    //return the value
    return *vm.stackTop;
}


static InterpretResult run() {
    //Start with defining macros
    /*The read byte macro, dereferences and reads the current instruction pointer*/
    #define READ_BYTE() (*vm.ip++)
    /*The read constant macro will read the index number of the constant value, and fetch 
    it from the value constant pool*/
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    //MACRO for binary operations !!!
    #define BINARY_OP(op) \
        do {\
            double b = pop(); \
            double a = pop(); \
            push(a op b); \
            } while(false)

    for (;;) {
        //If the flag DTE is defined then print each instruction 
        #ifdef DEBUG_TRACE_EXECUTION
            printf("          ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
            }
            printf("\n");
        //doing pointer math to convert the ip back to a relative offset from the beginning.
        // eg if you started at x8828 and beginning is x8820 then it starts with offset 8 :)
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif
        
        uint8_t instruction;
        //Read the instruction code from Read_Byte macro
        switch (instruction = READ_BYTE()) {
            //Check which OP code it is
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                //The constant that is read is pushed on the VM's stack.
                push(constant);
                break;
            }
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            //In case the value is a simple negate instruction, take the constant at the 
            //top of the stack and simply pop and push a negative version of it.
            case OP_NEGATE: push(-pop()); break;
            case OP_RETURN: {
                //When a return is read, the stack is popped !!
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
    
    //Undefine the macros -> since they are function specific.
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}


InterpretResult interpret(Chunk* chunk) {
    //The code is stored in the VM's chunk
    vm.chunk = chunk;
    //Next the location of this bytecode is stored in a variable called IP
    vm.ip = vm.chunk->code;
    //the bytecode nstruction is run.
    return run();
}