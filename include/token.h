#ifndef TOKEN_H
#define TOKNE_H

enum class TokenType{
    OPERAND,
    STAR,
    CONCATENATE,
    BAR,
    LEFT_PARENTTHESIS, // always pushed into stack 
    RIGHT_PARENTHESIS  // never pushed into stack
};

struct Token {
    TokenType type;
    char value;
};
#endif
