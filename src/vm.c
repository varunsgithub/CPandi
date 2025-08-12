#include "common.h"
#include "vm.h"
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"
#include "compiler.h"


/*Defining a global variable called VM !*/
VM vm;

static void resetStack() {
    //This shows that the stack is empty since the stackTop points to 0
    vm.stackTop = vm.stack;
}

/*Print a runtime error for unlimited args*/
static void runtimeError(const char* format, ...) {
    //The va_list is a way that stores the unlimited (...) args
    va_list args;
    va_start(args, format);
    //vfprintf helps print those unlimited args
    vfprintf(stderr, format, args);
    va_end(args);
    
    fputs("\n", stderr);
    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk-> lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
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

/*Helps peek the stack with the given distance*/
static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

/*Helps typecheck the not operator*/
static bool isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}


static InterpretResult run() {
    //Start with defining macros
    /*The read byte macro, dereferences and reads the current instruction pointer*/
    #define READ_BYTE() (*vm.ip++)
    /*The read constant macro will read the index number of the constant value, and fetch 
    it from the value constant pool*/
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    //MACRO for binary operations !!!
    #define BINARY_OP(valueType, op) \
        do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop()); \
        push(valueType(a op b)); \
        } while (false)

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
            case OP_NIL:      push(NIL_VAL);                   break;
            case OP_TRUE:     push(BOOL_VAL(true));            break;
            case OP_FALSE:    push(BOOL_VAL(false));           break;
            case OP_EQUAL: {
                    Value b = pop();
                    Value a = pop();
                    push(BOOL_VAL(valuesEqual(a, b)));
                    break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:      BINARY_OP(NUMBER_VAL, +);        break;
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -);        break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *);        break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /);        break;
            case OP_NOT:      push(BOOL_VAL(isFalsey(pop()))); break;
            //In case the value is a simple negate instruction, take the constant at the 
            //top of the stack and simply pop and push a negative version of it.
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
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


InterpretResult interpret(const char* source) {
    //Create a new chunk for the bytecode !!
    Chunk chunk;

    //initialize the chunk !
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        
        freeChunk(&chunk);
        
        return INTERPRET_COMPILE_ERROR;
    }

    //the virtual machine's bytecode chunk is linked to the compiled chunk !1
    vm.chunk = &chunk;
    //the instruction pointer now points to the array of bytecode !!!
    vm.ip = vm.chunk->code;


    InterpretResult result = run();

    //delete the chunk and return the result !
    freeChunk(&chunk);
    return result;
}