#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include <stdlib.h>

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

//Struct for storing tokens
typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

/*The operands are consumed as per the order of precedence
If a higher precedence (eg assignment) is passed to the precedence function then all the other operators will be chewed
through until the assignment operator is arrived at !*/
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct {
    //get the function for the prefix operator !
    ParseFn prefix;
    //get the function for the infix operator !
    ParseFn infix;
    //get the function for the precedence !
    Precedence precedence;
} ParseRule;

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

static uint8_t makeConstant(Value value) {
    //The add constant method will access the bytecode's constant pool and add the value to it
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk da ");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
    #ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
    #endif
}

//These are the function declarations (Feed forwards) so that they can be used by 
// the remaining function calls !
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary() {
    // The token type is saved 
    TokenType operatorType = parser.previous.type;
    // the rule is obtained from the operator type !
    ParseRule* rule = getRule(operatorType);
    // the precedence is set
    parsePrecedence((Precedence) (rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        
        //basis the operator ->the chunk of bytecode !
        case TOKEN_PLUS:    emitByte(OP_ADD); break;
        //the minus is then -> subtracted
        case TOKEN_MINUS:   emitByte(OP_SUBTRACT); break;
        //star -> the multiply the bytecode
        case TOKEN_STAR:    emitByte(OP_MULTIPLY); break;
        // divide
        case TOKEN_SLASH:   emitByte(OP_DIVIDE); break;
        
        default: return;
    }
}

static void literal() {
    switch(parser.previous.type) {
        case TOKEN_FALSE:   emitByte(OP_FALSE); break;
        case TOKEN_NIL:     emitByte(OP_NIL);   break;
        case TOKEN_TRUE:    emitByte(OP_TRUE);  break;
        default: return;
    }
}

static void expression() {
    //the expression function shall parse the lowest precedence which shall 
    //consume all other expressions !
    parsePrecedence(PREC_ASSIGNMENT);
}

static void grouping() {
    //The expression is ... ?
    expression();
    //The right parenthesis is consumed !
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


static void number() {
    //strtod converts a string to a double int 
    double value = strtod(parser.previous.start, NULL);
    //emit the value but first use the number casting macro to cast the value to a number !
    emitConstant(NUMBER_VAL(value));
}

/*This function helps form a bytecode for strings*/
static void string() {
    //(The +1 and -2 trim the leading and trailing quotation marks)
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                            parser.previous.length - 2)));
}

static void unary() {
    //the previous token's type is saved into the operator type !!
    TokenType operatorType = parser.previous.type;

    //we limit the compilation to the point of finding the unary operator!
    parsePrecedence(PREC_UNARY);
    
    //In case the type is a negate
    switch (operatorType) {
        case TOKEN_BANG: emitByte(OP_NOT); break;
        //the negate is stored in the bytecode chunk !
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        //default -> will return from the function !
        default: return;
    }

    // It might look weird that when you emit the chunk of bytecode (NEGATE)
    // after the expression !! (EG: 1.2 -)...
    // Also we need a way to stop the operand from being chewed in the expression format
}

/*This is the PRATT parser table which is used to fetch the functions for the respective
TOKENS. The pratt parser then fetches the precedence basis the table !*/
//The array is a ParseRule array !!!
ParseRule rules[] = {
    //Token = {Prefix (ParseFn ptr), Infix (ParseFn ptr), Precedence:)}
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,     NULL,  PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static void parsePrecedence(Precedence precedence) {
    //Advance the compiler by one token!
    advance();

    //prefix rule -> 
    // ParseFn is a typedef for a function pointer -> what it does is that
    // whatever function is fetched from the get rule function is replaced in the pointer
    // so the thing becomes void (*the function that is fetched) ();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;

    if (prefixRule == NULL) {
        //error -> give expect expression !
        error("Expect expression.");
        return;
    }

    //call the function so stored in the prefixRule !!
    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        //Advance the pointer !
        advance();
        //ParseFn infixRule -> get the inflix fn 
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        //run the inflix function
        infixRule();
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
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
    
    expression();
    
    //Consumes the last token ! -> If there is no proper end... throws an error
    consume(TOKEN_EOF, "Expect end of expression");
    
    //End the compile function with writing return in the compiled chunk !
    endCompiler();
    return !parser.hadError;
    
}