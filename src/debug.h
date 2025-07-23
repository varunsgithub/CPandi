#ifndef cpandi_debug_h
#define cpandi_debug_h

#include "chunk.h"

/*This method helps with disassembling the given instructions */
void disassembleChunk(Chunk* chunk, const char* name);

int disassembleInstruction(Chunk* chunk, int offset);

static int simpleInstruction(const char* name, int offset);

#endif