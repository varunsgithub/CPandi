#include "common.h"
#include "vm.h"
#include <stdarg.h>
#include <stdio.h>
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "compiler.h"
#include <string.h>


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
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

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

/*Function helps concatenate two strings*/
static void concatenate() {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = takeString(chars, length);
  push(OBJ_VAL(result));
}



static InterpretResult run() {
    //Start with defining macros
    /*The read byte macro, dereferences and reads the current instruction pointer*/
    #define READ_BYTE() (*vm.ip++)
    /*The read constant macro will read the index number of the constant value, and fetch 
    it from the value constant pool*/
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    /*This macros helps read the number of places the body of the conditional occupies*/
    #define READ_SHORT() \
        (vm.ip += 2, (uint16_t)((vm.ip[-2] << 8 | vm.ip[-1])))
    /*This macro helps read the string from the stack*/
    #define READ_STRING() AS_STRING(READ_CONSTANT())
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
            case OP_POP:      pop();                           break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                //this operation finds the location of the element on the stack and pushes it on the top again
                push(vm.stack[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                vm.stack[slot] = peek(0);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                //the name of the string is stored using the read string macro
                ObjString* name = READ_STRING();
                //if the key is new then the variable does not exist 
                if (tableSet(&vm.globals, name, peek(0))) {
                    //then delete the name from the global hash table (the one we ended up setting in table set)
                    tableDelete(&vm.globals, name);
                    //push a runtime error ->undefined variable
                    runtimeError("Undefined variable '%s'", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                    Value b = pop();
                    Value a = pop();
                    push(BOOL_VAL(valuesEqual(a, b)));
                    break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {

                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate(); } 
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                runtimeError(
                    "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                } break;
            }
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
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_JUMP:  {
                uint16_t offset = READ_SHORT();
                vm.ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) vm.ip += offset;
                break;
            }
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
    #undef READ_STRING
    #undef READ_SHORT
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