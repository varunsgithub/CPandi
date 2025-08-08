#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    //Print the beginning with the name !
    printf("== %s ==\n", name);

    //start with an offset of 0, while the offset is less than 
    //array's length, disassemble the instructions in the
    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }

}

int disassembleInstruction(Chunk* chunk, int offset) {
    //Print the offset !
    printf("%04d ", offset);
      
    if (offset > 0 &&
    chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
  } else {
        printf("%4d ", chunk->lines[offset]);
  }

    //the instruction is then taken in from the array !
    // from the offset
    uint8_t instruction = chunk->code[offset];

    //Check the instruction
    switch(instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);
        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);
        //Disassemblers for binary operators !
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        
        //If it is OP_NEGATE then just return the Simple Instruction with OP_NEGATE!
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        
        //If it is OP_Return then return, Simple Instructions
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

/*Method to read constant instructions !! */
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    //Store the constant (which should be next to OP code hence +1)
    uint8_t constant = chunk->code[offset+1];
    //print the name and the constant
    printf("%-16s %4d ' ", name, constant);
    //get the value from the constant pool
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset+2;
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}