#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include <stdlib.h>

//Struct for storing tokens
typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

Chunk* compilingChunk;

/*The function returns the current array of compiled chunk*/
static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    //if the panic mode has been set, just exit the method (This ensures bytecode is read)
    // and normal processing happens.
    if (parser.panicMode) return;
    //set panic mode to true
    parser.panicMode = true;
    //Print in the stderr stream that there has been an error at line number....
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        //NOTHING DA
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

/*The function will ensure that it fetches a good token to process in the compiler*/
static void advance() {
    //The current token is stored in the previous token
    parser.previous = parser.current;

    for (;;) {
        //the current token is then scanned using the scanner
        parser.current = scanToken();
        //if the token is not an error token -> break
        if (parser.current.type != TOKEN_ERROR) break;

        //if it is an error -> display the error to the user
        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    //if the current token is of the desired type then advance to the next token
    if (parser.current.type == type) {
        advance();
        return;
    }
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static void endCompiler() {
    emitReturn();
}

/*When compiling, the tokenized source code and the chunk where the 
processed bytecode is to be stored is passed as args to the function*/
bool compile(const char* source, Chunk* chunk) {
    
    initScanner(source);
    compilingChunk = chunk;

    //Initialization for the bool variables in the parser !
    parser.hadError = false;
    parser.panicMode = false;

    //Reads the error free tokens !!
    advance();
    
    //
    expression();
    
    //Consumes the last token ! -> If there is no proper end... throws an error
    consume(TOKEN_EOF, "Expect end of expression");
    
    //End the compile function with writing return in the compiled chunk !
    endCompiler();
    return !parser.hadError;
    
}