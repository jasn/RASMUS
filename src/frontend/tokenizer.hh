#ifndef TOKENIZER_HH_GUARD
#define TOKENIZER_HH_GUARD

#include <iostream>
#include <stdint.h>

namespace lexer {

enum class TokenType {
    TK_NAME, TK_FLOAT, TK_BADINT, TK_INT, TK_TEXT, _TK_COMMENT, _WHITESPACE, _NEWLINE, TK_ASSIGN, TK_AT, TK_BANG, TK_BANGGT, TK_BANGLT, TK_BLOCKEND, TK_BLOCKSTART, TK_CHOICE, TK_COLON, TK_COMMA, TK_CONCAT, TK_DIFFERENT, TK_DIV, TK_EQUAL, TK_GREATER, TK_GREATEREQUAL, TK_ISANY, TK_ISATOM, TK_ISBOOL, TK_ISFUNC, TK_ISINT, TK_ISFLOAT, TK_ISREL, TK_ISTEXT, TK_ISTUP, TK_LBRACKET, TK_LEFT_ARROW, TK_LESS, TK_LESSEQUAL, TK_LPAREN, TK_MINUS, TK_MUL, TK_ONE_DOT, TK_OPEXTEND, TK_PIPE, TK_PLUS, TK_PROJECT_MINUS, TK_PROJECT_PLUS, TK_SELECT, TK_RBRACKET, TK_RIGHTARROW, TK_RPAREN, TK_SEMICOLON, TK_SET_MINUS, TK_SHARP, TK_STDBOOL, TK_STDINT, TK_STDFLOAT, TK_STDTEXT, TK_TILDE, TK_ADD, TK_AFTER, TK_AND, TK_BEFORE, TK_CLOSE, TK_COUNT, TK_DATE, TK_DAYS, TK_END, TK_FALSE, TK_FI, TK_FUNC, TK_HAS, TK_IF, TK_IN, TK_MAX, TK_MIN, TK_MOD, TK_MULT, TK_NOT, TK_ONE, TK_OPEN, TK_OR, TK_REL, TK_SYSTEM, TK_TODAY, TK_TRUE, TK_TUP, TK_TYPE_ANY, TK_TYPE_ATOM, TK_TYPE_BOOL, TK_TYPE_FUNC, TK_TYPE_INT, TK_TYPE_FLOAT, TK_TYPE_REL, TK_TYPE_TEXT, TK_TYPE_TUP, TK_VAL, TK_WRITE, TK_UNSET, TK_ZERO, TK_PRINT, END_OF_FILE, INVALID

};

std::ostream& operator<<(std::ostream &os, const lexer::TokenType &t);

struct Token {
    const char *start, *curr;
    TokenType tkn;
};


struct Tokenizer {

    const char *str;

    Tokenizer(const char *str) : str(str) {}

    Token getNextToken();

};

} // end namespace lexer

#endif // TOKENIZER_HH_GUARD
