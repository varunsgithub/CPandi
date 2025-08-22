#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include <stdlib.h>
#include <string.h>

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

typedef void (*ParseFn)(bool canAssign);

typedef struct {
    //get the function for the prefix operator !
    ParseFn prefix;
    //get the function for the infix operator !
    ParseFn infix;
    //get the function for the precedence !
    Precedence precedence;
} ParseRule;

/*The struct stores the name and the depth where the given variable is to be found*/
typedef struct {
    Token name;
    int depth;
} Local;

/*This enum helps the code distinguish between the main() function and the sub functions defined under it*/
typedef enum {
    TYPE_FUNCTION,
    //the outer layer (the main function)
    TYPE_SCRIPT
} FunctionType;


/*The struct creates an array of locals and has fields to keep a track of the length of the array
and the max depth*/
typedef struct Compiler {
    //keeps a track of the previous enclosing fn
    struct Compiler* enclosing;

    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

Parser parser;
Compiler* current = NULL;

/*This method helps return the position of the current chunk*/
static Chunk* currentChunk() {
    return &current->function->chunk;
}

Chunk* compilingChunk;


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

/*This is a helper method for the match function which checks the type of the token*/
static bool check(TokenType type) {
    return parser.current.type == type;
}

/*This method helps match the token type to a given type*/
static bool match(TokenType type) {
    //if the given type does not match the token type return false
    if (!check(type)) return false;
    //advance to the next token
    advance();
    //return true
    return true;
}


static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

/*This method helps the loop to re start from the beginning*/
static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    //take the current position subtract the loop start point and add 2 for size of OP_LOOP size
    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body is too large maccha");

    emitByte((offset>>8) & 0xff);
    emitByte(offset & 0xff);
}

/*This method emits two placeholder address locations which are updated later*/
static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    //Emit two placeholder bytes
    emitByte(0xff);
    emitByte(0xff);
    //return the chunk's position before the placeholder bytes (to be able to update them later)
    return currentChunk()-> count - 2;
}

static void emitReturn() {
    emitByte(OP_NIL);
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

static void patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset+1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(parser.previous.start, parser.previous.length);
    }

    //this is done so that the compiler's initial slot is not available for users to use
    //its reserved for the VM.
    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->name.start = "";
    local->name.length = 0;
}

static ObjFunction* endCompiler() {
    emitReturn();
    ObjFunction* function = current->function;
    #ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), function->name != NULL ? function->name->chars : "<script>");
    }
    #endif
    //when the compiler is done compiling the code, the enclsing compiler is restored.
    current = current->enclosing;
    return function;
}

/*Increments the scope depth by 1*/
static void beginScope() {
    current->scopeDepth++;
}

/*Decrements the scope depth by 1*/
static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 && 
            current->locals[current->localCount - 1].depth >
                current->scopeDepth) {
                    emitByte(OP_POP);
                    current->localCount--;
                }
}

//These are the function declarations (Feed forwards) so that they can be used by 
// the remaining function calls !
static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

/*The method takes in the string, string interns it, then it adds the constant
to the bytecode chunk !!!*/ 
static uint8_t identifierConstant(Token* name) {
    // this method takes the string -> first string interns it and then 
    // adds it to the byte code chunk !!
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

/*Returns the index position of the local variable in the given local's 
array or -1 if the local variable was not found*/
static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable in its own initializer");
            }
            return i;
        }
    }
    return -1;
}


/*Adds the local variable to the compiler's local variable list*/
static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Too many local variables in function.");
        return;
    }
    
    //We create a pointer to the local's array .
    Local* local = &current->locals[current->localCount++];
    //the name and depth of the variable is stored in the same !
    local->name = name;
    //each variable's uninitialized state depth is -1
    local->depth = -1;
}

/*This function is used for declaring a local variable*/
static void declareVariable() {
    //if the current scope is 0 then return (global variable)
    if (current->scopeDepth == 0) return;

    //the token name is taken from the previous token
    Token* name = &parser.previous;

    for (int i = current->localCount - 1; i >= 0; i--) {
        //all variables are checked if there is any duplicate name in the same scope
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Already a variable with this name in this scope");
        }
    }

    addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
    //consume the name of the variable
    consume(TOKEN_IDENTIFIER, errorMessage);
    
    //first declare the variable
    declareVariable();
    //if the current scope's depth is greater than 0... exit and return 0.
    if (current->scopeDepth > 0) return 0;
    
    //add it to the bytecode chunk and the string intern !!
    return identifierConstant(&parser.previous);
}

/*This helper function helps marking the object as initialized by updating the depth from -1 to their actual values*/
static void markInitialized() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {
    //if the current scope is nested -> then the variable is not global
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }
    
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList() {
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            if (argCount == 255) {
                error("Can't have more than 255 arguments.");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    return argCount;
}

/*The and will */
static void and_(bool canAssign) {
    //save the location of the place where the placeholder instructions have been saved
    int endJump = emitJump(OP_JUMP_IF_FALSE);
    //discard the value of the leftmost op
    emitByte(OP_POP);

    //compile the right side of the expression
    //get the 
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static void binary(bool canAssign) {
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

/*The call method will count the arguments and then emit the OP_CODE !*/
static void call(bool canAssign) {
    //count the number of arguments using the 
    //arguments in theh arg count !
    uint8_t argCount = argumentList();
    //emit a call OP_CODE !
    emitBytes(OP_CALL, argCount);
}

static void literal(bool canAssign) {
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

static void block() {
    //while the token right brace or the eof is not reached
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        //go to the declaration method
        declaration();
    }
    //consume the token right brace !
    consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/*The method */
static void function(FunctionType type) {
    
    Compiler compiler;
    
    initCompiler(&compiler, type);
    
    beginScope();

    consume(TOKEN_LEFT_PAREN, "Expect '(' after function name");

    //Here is where we add parameters in the function
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            //increase the parameter count
            current->function->arity++;
            //if the same is less than 255 then continue
            if (current->function->arity > 255) {
                errorAtCurrent("Can't have more than 255 parameters");
            }
            //the parameter is then saved and defined in the variable's local stack.
            uint8_t constant = parseVariable("Expect a parameter's name");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));   
    }


    consume(TOKEN_RIGHT_PAREN, "Expect ')' after parameters");
    consume(TOKEN_LEFT_BRACE, "Expect '{' after the function body.");
    block();

    ObjFunction* function = endCompiler();
    emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
}

/*This method helps in function declaration*/
/*A function declaration is considered as a variable declaration and the same is instantly marked as initialized*/
static void funDeclaration() {
    //The name is defined and read.
    uint8_t global = parseVariable("Expect function name");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

/*This method helps with variable declaration in the stataments method*/
static void varDeclaration() {
    //the variable is first parsed for its name
    uint8_t global = parseVariable("Expect variable name");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        //if there is no token equal then implicit put a nil for the value
        emitByte(OP_NIL);
    }
    //consume the semi colon 
    consume(TOKEN_SEMICOLON,
        "Expect ';' after variable declaration.");

    //define the variable which has been read.
    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

/*Method helps work on for statements*/
static void forStatement() {
    beginScope();
    
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (match(TOKEN_SEMICOLON)) {
        //do nothing
    } else if(match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        expressionStatement();
    }

    int loopStart = currentChunk()->count;
    
    int exitJump = -1;
    if (!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "Expect ';' after loop condition.");

        // Jump out of the loop if the condition is false.
        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP); // Condition.
    }

    if (!match(TOKEN_RIGHT_PAREN)) {

        int bodyJump = emitJump(OP_JUMP);
        
        int incrementStart = currentChunk()->count;
        
        expression();
        
        emitByte(OP_POP);
        
        consume(TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        
        loopStart = incrementStart;
        
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(OP_POP); // Condition.
    }


    endScope();
}

/*Helps with analyzing an if statement*/
static void ifStatement() {
    //consume the left parenthesis
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'");
    //analyse the expression
    expression();
    //consume the right token
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    /*Backpatching technique to fix the jumps*/
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    //recursively go into the statement
    statement();

    //so now we jump into the next jump 
    int elseJump = emitJump(OP_JUMP);

    //then patch jump
    patchJump(thenJump);
    emitByte(OP_POP);

    //now comes in the bad boy........
    //the else statement da
    if (match(TOKEN_ELSE)) statement();
    patchJump(elseJump);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

/*This method will compile the return statement for a function*/
static void returnStatement() {
    if (current->type == TYPE_SCRIPT) {
        error("Can't return from top level code maccha");
    }
    
    //match the semi colon token !
    if (match(TOKEN_SEMICOLON)) {
        //emit an empty return
        emitReturn();
    } else {
        //evaluate the expression
        expression();
        //consume the semicolon and emit an return.
        consume(TOKEN_SEMICOLON, "Expect ';' after return value.");
        emitByte(OP_RETURN);
    }
}

/*The while conditional statement method and its jumps*/
static void whileStatement() {
    int loopStart = currentChunk()->count;
    //consume the left parenthesis
    consume(TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    //evaluate the expression
    expression();
    //consume the right token
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

    //have an exit point ready
    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    //pop that exp
    emitByte(OP_POP);
    
    //compile the body of the while statement
    statement();
    
    emitLoop(loopStart);
    //jump here the moment you dont satisfy the condition
    patchJump(exitJump);
    //pop that stuff
    emitByte(OP_POP);
}

/*This method is an error synchronization technique where the Panic mode works by
entering a state of chewing through the entire block of code till it reaches the semi colon or any 
of the given parser types or the end of code whichever is earlier*/
static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;
        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            
            default:
                ; // do nothing :)
        }

        advance();
    }
}

static void declaration() {
    //if the token matches the function declaration
    if (match(TOKEN_FUN)) {
        //start the function declaration !
        funDeclaration();
    }
    //if you end up matching a variable declaration
    else if (match(TOKEN_VAR)) {
        //go to the variable declaration method
        varDeclaration();
    } else {
        //else go to the statement declaration
        statement();
    }

    //error synchronisation
    if (parser.panicMode) synchronize();
}

/*Calls the appropriate statement methods*/
static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else if (match(TOKEN_FOR)) {
        forStatement();
    } else if (match(TOKEN_IF)) {
        ifStatement();
    } else if (match(TOKEN_RETURN)) {
        returnStatement();
    } else if (match(TOKEN_WHILE)) {
        whileStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        //begin the scope !
        beginScope();
        //enter block
        block();
        //end the scope
        endScope();
    } else {
        expressionStatement();
    }
}


static void grouping(bool canAssign) {
    //The expression is ... ?
    expression();
    //The right parenthesis is consumed !
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}


static void number(bool canAssign) {
    //strtod converts a string to a double int 
    double value = strtod(parser.previous.start, NULL);
    //emit the value but first use the number casting macro to cast the value to a number !
    emitConstant(NUMBER_VAL(value));
}

/*The or function helps compile and short circuit the operator*/
static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

/*This function helps form a bytecode for strings*/
static void string(bool canAssign) {
    //(The +1 and -2 trim the leading and trailing quotation marks)
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                            parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }


    //if the token is an = then evaluate the expression
    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        //emit the setter bytecode chunk
        emitBytes(setOp, (uint8_t) arg);
    } else {
        //else emit the getter bytecode chunk
        emitBytes(getOp, (uint8_t) arg);
    }
}

/*This function helps form a bytecode for variable declarations*/
static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
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
  [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
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
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
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

    
    bool canAssign = precedence <= PREC_ASSIGNMENT;
    //call the function so stored in the prefixRule !!
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        //Advance the pointer !
        advance();
        //ParseFn infixRule -> get the inflix fn 
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        //run the infix function
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}


static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

/*When compiling, the tokenized source code is passed as args to the function*/
ObjFunction* compile(const char* source) {
    
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    //Initialization for the bool variables in the parser !
    parser.hadError = false;
    parser.panicMode = false;

    //Reads the error free tokens !!
    advance();
    
    while (!match(TOKEN_EOF)) {
        declaration();
    }
    
    ObjFunction* function = endCompiler();
    return parser.hadError ? NULL : function;
}