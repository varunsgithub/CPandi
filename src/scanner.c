#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"


typedef struct {
    //The beginning of the current lexeme being scanned
    const char* start;
    //the current position for the scanner
    const char* current;
    //What line the current lexeme is on !!!
    int line;
} Scanner;

Scanner scanner;


void initScanner(const char* source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

//checks if the given character is an alphabet !
static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

static char advance() {
    //increment the current position
    scanner.current++;
    //return the character that you just passed 
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    //return the current + 1 position !!!!
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;
    //if it is true then advance the pointer.
    scanner.current++;
    return true;
}


static bool isAtEnd() {
    //Return if the current position of the scanner is at the end 
    // Note that the end of the scanner is always '\0'
    return *scanner.current == '\0';
}

/*This method is executed after the current position of the scanner realises that there
is a token that needs to be made from the lexeme !*/
static Token makeToken(TokenType type) {
    //Create a new token 
    Token token;
    //the token type is taken from the argument
    token.type = type;
    // the start of the token is the starting point of the lexeme !
    token.start = scanner.start;
    //The length is the end(current position) - start !
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
  Token token;
  //Set the token type to token error
  token.type = TOKEN_ERROR;
  //The start of the token has to be the error message
  token.start = message;
  //Length has to be the length of the message
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

static void skipWhitespace() {
    //Why the for loop ?
    // helps skip all the white spaces :)
    for (;;) {
        char c = peek();
        switch(c) {
            //Skip all whitespace and tabs !
            case ' ':
            case '\r':
            case '\t':
            //Advance to the next character !
                advance();
                break;
            case '\n':
                //increment the line number
                scanner.line++;
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    //A comment goes all the way to end !
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            //default -> in case the next character is not wp -> skip and return
            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    //the function checks two things
    //1) If the length of the token matches the keyword's length !
    //2) If the memory block matches the remaining keyword (memcmp) !!!
    if (scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length)==0) {
        return type;
    }
    //return the identifier
    return TOKEN_IDENTIFIER;

}




static TokenType identifierType() {
    
    //This works like a trie where the letters are checked
    //if the first char is a then check -> the remaining characters are nd...and so on !
    switch (scanner.start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        //Since the words false, for and fun all start with s
        //we manager this with checking the second alphabet.
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch(scanner.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            } break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            } break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }


    return TOKEN_IDENTIFIER;
}



static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}

static Token number() {
    //While the next character is still a digit keep advancing
    while (isDigit(peek())) advance();

    //check for fractions :)
    if (peek() == '.' && isDigit(peekNext())) {
        //consume the .
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static Token string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string");

    //This consumes the last "
    advance();

    //return makeToken (for the token -> make it a string !!)
    return makeToken(TOKEN_STRING);
}

/*Implementing the scan token method*/
Token scanToken() {
    skipWhitespace();
    //The start of the scanner is set to the current position (to record the starting position of the lexeme)
    scanner.start = scanner.current;
    //If the token's position is at end then return EOF
    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    //if the lexeme is an alphabet then check if it is one of the identifier !!
    if (isAlpha(c)) {return identifier();}


    if (isDigit(c)) {
        return number();
    }

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_PAREN);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '-':
            return makeToken(TOKEN_MINUS);
        case '+':
            return makeToken(TOKEN_PLUS);
        case '/':
            return makeToken(TOKEN_SLASH);
        case '*':
            return makeToken(TOKEN_STAR);
        case '!':
            return makeToken(
                match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return string();
        
    }

    return errorToken("Unexepected character");
}