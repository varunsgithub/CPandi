#ifndef cpandi_scanner_h
#define cpandi_scanner_h

/*The initializer sets all the parameter to the source and line to 1*/
void initScanner(const char* source);

/*Scans the tokens*/
Token scanToken();

/*The method helps check if the pointer is at the end of the lexeme*/
static bool isAtEnd();

/*Helps convert the lexeme into a cpandi token*/
static Token makeToken(TokenType type);

/*Helps create a error token*/
static Token errorToken(const char* message);

/*advances the next lexeme*/
static char advance();

/*Matches characters !!*/
static bool match(char expected);

/*Helps skipping whitespaces :)*/
static void skipWhitespace();

/*Peeks the character in front*/
static char peek();

/*Helps look ahead (1 character)*/
static char peekNext();

/*The string function reads literal values*/
static Token string();

/*Function to check for numbers*/
static bool isDigit(char c);

/*Check if the given character is an alphabet*/
static bool isAlpha(char c);

/*Check if the given lexeme is an identifier*/
static Token identifier();

/*return the identifier type*/
static TokenType identifierType();

/*Converts numbers*/
static Token number();

/*Enum for tokentype*/
typedef enum {
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,
  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
  // Keywords.
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

  TOKEN_ERROR, TOKEN_EOF
} TokenType;


/*Struct for token*/
typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

#endif