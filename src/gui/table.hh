#ifndef TABLE_HH_GUARD
#define TABLE_HH_GUARD

#include <iostream>
#include <stdint.h>

namespace lexer {

enum class TableTokenType {
    INVALID=249,
    TK_NAME=250,
    TK_FLOAT=251,
    TK_BADINT=252,
    TK_INT=253,
    TK_TEXT=254,
    _TK_COMMENT=255,
    _WHITESPACE=256,
    _NEWLINE=257,
    TK_ASSIGN=258,
    TK_AT=259,
    TK_BANG=260,
    TK_BANGGT=261,
    TK_BANGLT=262,
    TK_BLOCKEND=263,
    TK_BLOCKSTART=264,
    TK_CHOICE=265,
    TK_COLON=266,
    TK_COMMA=267,
    TK_CONCAT=268,
    TK_DIFFERENT=269,
    TK_DIV=270,
    TK_EQUAL=271,
    TK_GREATER=272,
    TK_GREATEREQUAL=273,
    TK_ISANY=274,
    TK_ISATOM=275,
    TK_ISBOOL=276,
    TK_ISFUNC=277,
    TK_ISINT=278,
    TK_ISFLOAT=279,
    TK_ISREL=280,
    TK_ISTEXT=281,
    TK_ISTUP=282,
    TK_LBRACKET=283,
    TK_LEFT_ARROW=284,
    TK_LESS=285,
    TK_LESSEQUAL=286,
    TK_LPAREN=287,
    TK_MINUS=288,
    TK_MUL=289,
    TK_ONE_DOT=290,
    TK_OPEXTEND=291,
    TK_PIPE=292,
    TK_PLUS=293,
    TK_PROJECT_MINUS=294,
    TK_PROJECT_PLUS=295,
    TK_SELECT=296,
    TK_RBRACKET=297,
    TK_RIGHTARROW=298,
    TK_RPAREN=299,
    TK_SEMICOLON=300,
    TK_SET_MINUS=301,
    TK_SHARP=302,
    TK_STDBOOL=303,
    TK_STDINT=304,
    TK_STDFLOAT=305,
    TK_STDTEXT=306,
    TK_TILDE=307,
    TK_TWO_DOTS=308,
    TK_ADD=309,
    TK_AFTER=310,
    TK_AND=311,
    TK_BEFORE=312,
    TK_CLOSE=313,
    TK_COUNT=314,
    TK_DATE=315,
    TK_DAYS=316,
    TK_END=317,
    TK_FALSE=318,
    TK_FI=319,
    TK_FUNC=320,
    TK_HAS=321,
    TK_IF=322,
    TK_IN=323,
    TK_MAX=324,
    TK_MIN=325,
    TK_MOD=326,
    TK_MULT=327,
    TK_NOT=328,
    TK_ONE=329,
    TK_OPEN=330,
    TK_OR=331,
    TK_REL=332,
    TK_SYSTEM=333,
    TK_TODAY=334,
    TK_TRUE=335,
    TK_TUP=336,
    TK_TYPE_ANY=337,
    TK_TYPE_ATOM=338,
    TK_TYPE_BOOL=339,
    TK_TYPE_FUNC=340,
    TK_TYPE_INT=341,
    TK_TYPE_FLOAT=342,
    TK_TYPE_REL=343,
    TK_TYPE_TEXT=344,
    TK_TYPE_TUP=345,
    TK_VAL=346,
    TK_WRITE=347,
    TK_UNSET=348,
    TK_ZERO=349,
    TK_PRINT=350};
extern int table[][256];
const int initialState=0;
} // end namespace lexer

#endif // TABLE_HH_GUARD