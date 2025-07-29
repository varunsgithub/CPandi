#include "common.h"
#include "vm.h"
#include <stdio.h>
#include "debug.h"


/*Defining a global variable called VM !*/
VM vm;

void initVM() {}

void freeVM() {}


static InterpretResult run() {
    //Start with defining macros
    /*The read byte macro, dereferences and reads the current instruction pointer*/
    #define READ_BYTE() (*vm.ip++)
    /*The read constant macro will read the index number of the constant value, and fetch 
    it from the value constant pool*/
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    for (;;) {
        //If the flag DTE is defined then print each instruction 
        #ifdef DEBUG_TRACE_EXECUTION
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
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_RETURN: {
                return INTERPRET_OK;
            }
        }
    }
    
    //Undefine the macros -> since they are function specific.
    #undef READ_BYTE
    #undef READ_CONSTANT
}


InterpretResult interpret(Chunk* chunk) {
    //The code is stored in the VM's chunk
    vm.chunk = chunk;
    //Next the location of this bytecode is stored in a variable called IP
    vm.ip = vm.chunk->code;
    //the bytecode nstruction is run.
    return run();
}