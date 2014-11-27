#include "tokenizer.hh"

namespace lexer {

std::ostream& operator<<(std::ostream &os, const lexer::TokenType &t) {
    switch (t) {
    case lexer::TokenType::TK_NAME:
        os << "TK_NAME";
        break;
    case lexer::TokenType::TK_FLOAT:
        os << "TK_FLOAT";
        break;
    case lexer::TokenType::TK_BADINT:
        os << "TK_BADINT";
        break;
    case lexer::TokenType::TK_INT:
        os << "TK_INT";
        break;
    case lexer::TokenType::TK_TEXT:
        os << "TK_TEXT";
        break;
    case lexer::TokenType::_TK_COMMENT:
        os << "_TK_COMMENT";
        break;
    case lexer::TokenType::_WHITESPACE:
        os << "_WHITESPACE";
        break;
    case lexer::TokenType::_NEWLINE:
        os << "_NEWLINE";
        break;
    case lexer::TokenType::TK_ASSIGN:
        os << "TK_ASSIGN";
        break;
    case lexer::TokenType::TK_AT:
        os << "TK_AT";
        break;
    case lexer::TokenType::TK_BANG:
        os << "TK_BANG";
        break;
    case lexer::TokenType::TK_BANGGT:
        os << "TK_BANGGT";
        break;
    case lexer::TokenType::TK_BANGLT:
        os << "TK_BANGLT";
        break;
    case lexer::TokenType::TK_BLOCKEND:
        os << "TK_BLOCKEND";
        break;
    case lexer::TokenType::TK_BLOCKSTART:
        os << "TK_BLOCKSTART";
        break;
    case lexer::TokenType::TK_CHOICE:
        os << "TK_CHOICE";
        break;
    case lexer::TokenType::TK_COLON:
        os << "TK_COLON";
        break;
    case lexer::TokenType::TK_COMMA:
        os << "TK_COMMA";
        break;
    case lexer::TokenType::TK_CONCAT:
        os << "TK_CONCAT";
        break;
    case lexer::TokenType::TK_DIFFERENT:
        os << "TK_DIFFERENT";
        break;
    case lexer::TokenType::TK_DIV:
        os << "TK_DIV";
        break;
    case lexer::TokenType::TK_EQUAL:
        os << "TK_EQUAL";
        break;
    case lexer::TokenType::TK_GREATER:
        os << "TK_GREATER";
        break;
    case lexer::TokenType::TK_GREATEREQUAL:
        os << "TK_GREATEREQUAL";
        break;
    case lexer::TokenType::TK_ISANY:
        os << "TK_ISANY";
        break;
    case lexer::TokenType::TK_ISATOM:
        os << "TK_ISATOM";
        break;
    case lexer::TokenType::TK_ISBOOL:
        os << "TK_ISBOOL";
        break;
    case lexer::TokenType::TK_ISFUNC:
        os << "TK_ISFUNC";
        break;
    case lexer::TokenType::TK_ISINT:
        os << "TK_ISINT";
        break;
    case lexer::TokenType::TK_ISFLOAT:
        os << "TK_ISFLOAT";
        break;
    case lexer::TokenType::TK_ISREL:
        os << "TK_ISREL";
        break;
    case lexer::TokenType::TK_ISTEXT:
        os << "TK_ISTEXT";
        break;
    case lexer::TokenType::TK_ISTUP:
        os << "TK_ISTUP";
        break;
    case lexer::TokenType::TK_LBRACKET:
        os << "TK_LBRACKET";
        break;
    case lexer::TokenType::TK_LEFT_ARROW:
        os << "TK_LEFT_ARROW";
        break;
    case lexer::TokenType::TK_LESS:
        os << "TK_LESS";
        break;
    case lexer::TokenType::TK_LESSEQUAL:
        os << "TK_LESSEQUAL";
        break;
    case lexer::TokenType::TK_LPAREN:
        os << "TK_LPAREN";
        break;
    case lexer::TokenType::TK_MINUS:
        os << "TK_MINUS";
        break;
    case lexer::TokenType::TK_MUL:
        os << "TK_MUL";
        break;
    case lexer::TokenType::TK_ONE_DOT:
        os << "TK_ONE_DOT";
        break;
    case lexer::TokenType::TK_OPEXTEND:
        os << "TK_OPEXTEND";
        break;
    case lexer::TokenType::TK_PIPE:
        os << "TK_PIPE";
        break;
    case lexer::TokenType::TK_PLUS:
        os << "TK_PLUS";
        break;
    case lexer::TokenType::TK_PROJECT_MINUS:
        os << "TK_PROJECT_MINUS";
        break;
    case lexer::TokenType::TK_PROJECT_PLUS:
        os << "TK_PROJECT_PLUS";
        break;
    case lexer::TokenType::TK_SELECT:
        os << "TK_SELECT";
        break;
    case lexer::TokenType::TK_RBRACKET:
        os << "TK_RBRACKET";
        break;
    case lexer::TokenType::TK_RIGHTARROW:
        os << "TK_RIGHTARROW";
        break;
    case lexer::TokenType::TK_RPAREN:
        os << "TK_RPAREN";
        break;
    case lexer::TokenType::TK_SEMICOLON:
        os << "TK_SEMICOLON";
        break;
    case lexer::TokenType::TK_SET_MINUS:
        os << "TK_SET_MINUS";
        break;
    case lexer::TokenType::TK_SHARP:
        os << "TK_SHARP";
        break;
    case lexer::TokenType::TK_STDBOOL:
        os << "TK_STDBOOL";
        break;
    case lexer::TokenType::TK_STDINT:
        os << "TK_STDINT";
        break;
    case lexer::TokenType::TK_STDFLOAT:
        os << "TK_STDFLOAT";
        break;
    case lexer::TokenType::TK_STDTEXT:
        os << "TK_STDTEXT";
        break;
    case lexer::TokenType::TK_TILDE:
        os << "TK_TILDE";
        break;
    case lexer::TokenType::TK_TWO_DOTS:
        os << "TK_TWO_DOTS";
        break;
    case lexer::TokenType::TK_ADD:
        os << "TK_ADD";
        break;
    case lexer::TokenType::TK_AFTER:
        os << "TK_AFTER";
        break;
    case lexer::TokenType::TK_AND:
        os << "TK_AND";
        break;
    case lexer::TokenType::TK_BEFORE:
        os << "TK_BEFORE";
        break;
    case lexer::TokenType::TK_CLOSE:
        os << "TK_CLOSE";
        break;
    case lexer::TokenType::TK_COUNT:
        os << "TK_COUNT";
        break;
    case lexer::TokenType::TK_DATE:
        os << "TK_DATE";
        break;
    case lexer::TokenType::TK_DAYS:
        os << "TK_DAYS";
        break;
    case lexer::TokenType::TK_END:
        os << "TK_END";
        break;
    case lexer::TokenType::TK_FALSE:
        os << "TK_FALSE";
        break;
    case lexer::TokenType::TK_FI:
        os << "TK_FI";
        break;
    case lexer::TokenType::TK_FUNC:
        os << "TK_FUNC";
        break;
    case lexer::TokenType::TK_HAS:
        os << "TK_HAS";
        break;
    case lexer::TokenType::TK_IF:
        os << "TK_IF";
        break;
    case lexer::TokenType::TK_IN:
        os << "TK_IN";
        break;
    case lexer::TokenType::TK_MAX:
        os << "TK_MAX";
        break;
    case lexer::TokenType::TK_MIN:
        os << "TK_MIN";
        break;
    case lexer::TokenType::TK_MOD:
        os << "TK_MOD";
        break;
    case lexer::TokenType::TK_MULT:
        os << "TK_MULT";
        break;
    case lexer::TokenType::TK_NOT:
        os << "TK_NOT";
        break;
    case lexer::TokenType::TK_ONE:
        os << "TK_ONE";
        break;
    case lexer::TokenType::TK_OPEN:
        os << "TK_OPEN";
        break;
    case lexer::TokenType::TK_OR:
        os << "TK_OR";
        break;
    case lexer::TokenType::TK_REL:
        os << "TK_REL";
        break;
    case lexer::TokenType::TK_SYSTEM:
        os << "TK_SYSTEM";
        break;
    case lexer::TokenType::TK_TODAY:
        os << "TK_TODAY";
        break;
    case lexer::TokenType::TK_TRUE:
        os << "TK_TRUE";
        break;
    case lexer::TokenType::TK_TUP:
        os << "TK_TUP";
        break;
    case lexer::TokenType::TK_TYPE_ANY:
        os << "TK_TYPE_ANY";
        break;
    case lexer::TokenType::TK_TYPE_ATOM:
        os << "TK_TYPE_ATOM";
        break;
    case lexer::TokenType::TK_TYPE_BOOL:
        os << "TK_TYPE_BOOL";
        break;
    case lexer::TokenType::TK_TYPE_FUNC:
        os << "TK_TYPE_FUNC";
        break;
    case lexer::TokenType::TK_TYPE_INT:
        os << "TK_TYPE_INT";
        break;
    case lexer::TokenType::TK_TYPE_FLOAT:
        os << "TK_TYPE_FLOAT";
        break;
    case lexer::TokenType::TK_TYPE_REL:
        os << "TK_TYPE_REL";
        break;
    case lexer::TokenType::TK_TYPE_TEXT:
        os << "TK_TYPE_TEXT";
        break;
    case lexer::TokenType::TK_TYPE_TUP:
        os << "TK_TYPE_TUP";
        break;
    case lexer::TokenType::TK_VAL:
        os << "TK_VAL";
        break;
    case lexer::TokenType::TK_WRITE:
        os << "TK_WRITE";
        break;
    case lexer::TokenType::TK_UNSET:
        os << "TK_UNSET";
        break;
    case lexer::TokenType::TK_ZERO:
        os << "TK_ZERO";
        break;
    case lexer::TokenType::TK_PRINT:
        os << "TK_PRINT";
        break;
    case lexer::TokenType::END_OF_FILE:
        os << "END_OF_FILE";
        break;
    case lexer::TokenType::INVALID:
    default:
        os << "INVALID";
    }

    return os;
}

Token Tokenizer::getNextToken() {


    const uint8_t *start;
    const uint8_t *curr = reinterpret_cast<const uint8_t*>(str);
beginning:
    start = curr;

    goto s0;

s0:
    switch (*curr) {
    case 67:
    case 68:
    case 69:
    case 71:
    case 72:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 83:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 103:
    case 106:
    case 107:
    case 108:
    case 113:
    case 120:
    case 121:
        ++curr;
        goto s68;
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s71;
    case 126:
        ++curr;
        goto s74;
    case 124:
        ++curr;
        goto s77;
    case 122:
        ++curr;
        goto s80;
    case 119:
        ++curr;
        goto s83;
    case 118:
        ++curr;
        goto s86;
    case 117:
        ++curr;
        goto s88;
    case 116:
        ++curr;
        goto s91;
    case 115:
        ++curr;
        goto s94;
    case 114:
        ++curr;
        goto s97;
    case 112:
        ++curr;
        goto s100;
    case 111:
        ++curr;
        goto s103;
    case 60:
        ++curr;
        goto s105;
    case 59:
        ++curr;
        goto s106;
    case 48:
        ++curr;
        goto s110;
    case 47:
        ++curr;
        goto s111;
    case 46:
        ++curr;
        goto s112;
    case 45:
        ++curr;
        goto s113;
    case 44:
        ++curr;
        goto s114;
    case 43:
        ++curr;
        goto s115;
    case 42:
        ++curr;
        goto s116;
    case 41:
        ++curr;
        goto s117;
    case 58:
        ++curr;
        goto s118;
    case 40:
        ++curr;
        goto s119;
    case 93:
        ++curr;
        goto s122;
    case 13:
        ++curr;
        goto s123;
    case 9:
    case 32:
        ++curr;
        goto s124;
    case 10:
        ++curr;
        goto s125;
    case 33:
        ++curr;
        goto s126;
    case 34:
        ++curr;
        goto s127;
    case 35:
        ++curr;
        goto s128;
    case 38:
        ++curr;
        goto s129;
    case 61:
        ++curr;
        goto s130;
    case 62:
        ++curr;
        goto s131;
    case 63:
        ++curr;
        goto s132;
    case 64:
        ++curr;
        goto s133;
    case 65:
        ++curr;
        goto s136;
    case 66:
        ++curr;
        goto s137;
    case 70:
        ++curr;
        goto s138;
    case 73:
        ++curr;
        goto s139;
    case 82:
        ++curr;
        goto s141;
    case 84:
        ++curr;
        goto s143;
    case 91:
        ++curr;
        goto s145;
    case 92:
        ++curr;
        goto s147;
    case 97:
        ++curr;
        goto s149;
    case 98:
        ++curr;
        goto s151;
    case 99:
        ++curr;
        goto s153;
    case 100:
        ++curr;
        goto s155;
    case 101:
        ++curr;
        goto s157;
    case 102:
        ++curr;
        goto s159;
    case 104:
        ++curr;
        goto s160;
    case 105:
        ++curr;
        goto s162;
    case 109:
        ++curr;
        goto s164;
    case 110:
        ++curr;
        goto s165;
    case 0:
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::END_OF_FILE};
    default: 
        ++curr;
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s1:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISFLOAT};
    }
s2:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISTEXT};
    }
s3:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISFUNC};
    }
s4:
    switch (*curr) {
    case 116:
        ++curr;
        goto s1;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s5:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISBOOL};
    }
s6:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISATOM};
    }
s7:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_STDFLOAT};
    }
s8:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_SYSTEM};
    }
s9:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISTUP};
    }
s10:
    switch (*curr) {
    case 116:
        ++curr;
        goto s2;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s11:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISREL};
    }
s12:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISINT};
    }
s13:
    switch (*curr) {
    case 99:
        ++curr;
        goto s3;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s14:
    switch (*curr) {
    case 97:
        ++curr;
        goto s4;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s15:
    switch (*curr) {
    case 108:
        ++curr;
        goto s5;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s16:
    switch (*curr) {
    case 109:
        ++curr;
        goto s6;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s17:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ISANY};
    }
s18:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BEFORE};
    }
s19:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_STDTEXT};
    }
s20:
    switch (*curr) {
    case 116:
        ++curr;
        goto s7;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s21:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_STDBOOL};
    }
s22:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_WRITE};
    }
s23:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_UNSET};
    }
s24:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TODAY};
    }
s25:
    switch (*curr) {
    case 109:
        ++curr;
        goto s8;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s26:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_PRINT};
    }
s27:
    switch (*curr) {
    case 112:
        ++curr;
        goto s9;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s28:
    switch (*curr) {
    case 120:
        ++curr;
        goto s10;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s29:
    switch (*curr) {
    case 108:
        ++curr;
        goto s11;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s30:
    switch (*curr) {
    case 116:
        ++curr;
        goto s12;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s31:
    switch (*curr) {
    case 110:
        ++curr;
        goto s13;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s32:
    switch (*curr) {
    case 111:
        ++curr;
        goto s14;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s33:
    switch (*curr) {
    case 111:
        ++curr;
        goto s15;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s34:
    switch (*curr) {
    case 111:
        ++curr;
        goto s16;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s35:
    switch (*curr) {
    case 121:
        ++curr;
        goto s17;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s36:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_FALSE};
    }
s37:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_COUNT};
    }
s38:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_CLOSE};
    }
s39:
    switch (*curr) {
    case 101:
        ++curr;
        goto s18;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s40:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_AFTER};
    }
s41:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_FLOAT};
    }
s42:
    switch (*curr) {
    case 116:
        ++curr;
        goto s19;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s43:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_STDINT};
    }
s44:
    switch (*curr) {
    case 97:
        ++curr;
        goto s20;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s45:
    switch (*curr) {
    case 108:
        ++curr;
        goto s21;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s46:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ZERO};
    }
s47:
    switch (*curr) {
    case 101:
        ++curr;
        goto s22;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s48:
    switch (*curr) {
    case 116:
        ++curr;
        goto s23;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s49:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TRUE};
    }
s50:
    switch (*curr) {
    case 121:
        ++curr;
        goto s24;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s51:
    switch (*curr) {
    case 101:
        ++curr;
        goto s25;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s52:
    switch (*curr) {
    case 116:
        ++curr;
        goto s26;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s53:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_OPEN};
    }
s54:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MULT};
    }
s55:
    switch (*curr) {
    case 117:
        ++curr;
        goto s27;
    case 101:
        ++curr;
        goto s28;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s56:
    switch (*curr) {
    case 101:
        ++curr;
        goto s29;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s57:
    switch (*curr) {
    case 110:
        ++curr;
        goto s30;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s58:
    switch (*curr) {
    case 117:
        ++curr;
        goto s31;
    case 108:
        ++curr;
        goto s32;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s59:
    switch (*curr) {
    case 111:
        ++curr;
        goto s33;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s60:
    switch (*curr) {
    case 116:
        ++curr;
        goto s34;
    case 110:
        ++curr;
        goto s35;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s61:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 102:
        ++curr;
        goto s214;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s62:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 100:
        ++curr;
        goto s213;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s63:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 116:
        ++curr;
        goto s212;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s64:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 100:
        ++curr;
        goto s211;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s65:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 112:
        ++curr;
        goto s210;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s66:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 120:
        ++curr;
        goto s209;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s67:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s208;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s68:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s69:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s70:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s75;
    case 43:
    case 45:
        ++curr;
        goto s78;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s71:
    switch (*curr) {
    case 101:
        ++curr;
        goto s70;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s71;
    case 46:
        ++curr;
        goto s72;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_INT};
    }
s72:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s76;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s73:
    switch (*curr) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
    case 221:
    case 222:
    case 223:
    case 224:
    case 225:
    case 226:
    case 227:
    case 228:
    case 229:
    case 230:
    case 231:
    case 232:
    case 233:
    case 234:
    case 235:
    case 236:
    case 237:
    case 238:
    case 239:
    case 240:
    case 241:
    case 242:
    case 243:
    case 244:
    case 245:
    case 246:
    case 247:
    case 248:
    case 249:
    case 250:
    case 251:
    case 252:
    case 253:
    case 254:
    case 255:
        ++curr;
        goto s73;
    default: 
        goto beginning;
    }
s74:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TILDE};
    }
s75:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s75;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_FLOAT};
    }
s76:
    switch (*curr) {
    case 101:
        ++curr;
        goto s70;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s76;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_FLOAT};
    }
s77:
    switch (*curr) {
    case 43:
        ++curr;
        goto s195;
    case 45:
        ++curr;
        goto s196;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_PIPE};
    }
s78:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s75;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s79:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TWO_DOTS};
    }
s80:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s194;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s81:
    switch (*curr) {
    case 120:
        ++curr;
        goto s42;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s82:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_CONCAT};
    }
s83:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 114:
        ++curr;
        goto s193;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s84:
    switch (*curr) {
    case 111:
        ++curr;
        goto s44;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s85:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BLOCKEND};
    }
s86:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s192;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s87:
    switch (*curr) {
    case 116:
        ++curr;
        goto s43;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s88:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s191;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s89:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s89;
    case 41:
        ++curr;
        goto s92;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s90:
    switch (*curr) {
    case 13:
        ++curr;
        goto s123;
    default: 
        goto beginning;
    }
s91:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s188;
    case 114:
        ++curr;
        goto s189;
    case 117:
        ++curr;
        goto s190;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s92:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_AT};
    }
s93:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_LEFT_ARROW};
    }
s94:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 122:
        ++curr;
        goto s68;
    case 121:
        ++curr;
        goto s187;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s95:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_ATOM};
    }
s96:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_OPEXTEND};
    }
s97:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s186;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s98:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_BOOL};
    }
s99:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_LESSEQUAL};
    }
s100:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 114:
        ++curr;
        goto s185;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s101:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_FUNC};
    }
s102:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_DIFFERENT};
    }
s103:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s182;
    case 112:
        ++curr;
        goto s183;
    case 114:
        ++curr;
        goto s184;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s104:
    switch (*curr) {
    case 116:
        ++curr;
        goto s41;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s105:
    switch (*curr) {
    case 45:
        ++curr;
        goto s93;
    case 60:
        ++curr;
        goto s96;
    case 61:
        ++curr;
        goto s99;
    case 62:
        ++curr;
        goto s102;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_LESS};
    }
s106:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_SEMICOLON};
    }
s107:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s107;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BADINT};
    }
s108:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_TEXT};
    }
s109:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 116:
        ++curr;
        goto s207;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s110:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s107;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_INT};
    }
s111:
    switch (*curr) {
    case 47:
        ++curr;
        goto s73;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_DIV};
    }
s112:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s76;
    case 46:
        ++curr;
        goto s79;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ONE_DOT};
    }
s113:
    switch (*curr) {
    case 62:
        ++curr;
        goto s134;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MINUS};
    }
s114:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_COMMA};
    }
s115:
    switch (*curr) {
    case 43:
        ++curr;
        goto s82;
    case 41:
        ++curr;
        goto s85;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_PLUS};
    }
s116:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MUL};
    }
s117:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_RPAREN};
    }
s118:
    switch (*curr) {
    case 61:
        ++curr;
        goto s152;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_COLON};
    }
s119:
    switch (*curr) {
    case 43:
        ++curr;
        goto s154;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_LPAREN};
    }
s120:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s205;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s121:
    switch (*curr) {
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
        ++curr;
        goto s89;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s122:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_RBRACKET};
    }
s123:
    switch (*curr) {
    case 10:
        ++curr;
        goto s90;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s124:
    switch (*curr) {
    case 9:
    case 32:
        ++curr;
        goto s124;
    default: 
        goto beginning;
    }
s125:
    switch (*curr) {
    case 10:
        ++curr;
        goto s125;
    default: 
        goto beginning;
    }
s126:
    switch (*curr) {
    case 62:
        ++curr;
        goto s161;
    case 60:
        ++curr;
        goto s163;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BANG};
    }
s127:
    switch (*curr) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
    case 221:
    case 222:
    case 223:
    case 224:
    case 225:
    case 226:
    case 227:
    case 228:
    case 229:
    case 230:
    case 231:
    case 232:
    case 233:
    case 234:
    case 235:
    case 236:
    case 237:
    case 238:
    case 239:
    case 240:
    case 241:
    case 242:
    case 243:
    case 244:
    case 245:
    case 246:
    case 247:
    case 248:
    case 249:
    case 250:
    case 251:
    case 252:
    case 253:
    case 254:
    case 255:
        ++curr;
        goto s127;
    case 34:
        ++curr;
        goto s156;
    case 92:
        ++curr;
        goto s158;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s128:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_SHARP};
    }
s129:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_CHOICE};
    }
s130:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_EQUAL};
    }
s131:
    switch (*curr) {
    case 61:
        ++curr;
        goto s150;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_GREATER};
    }
s132:
    switch (*curr) {
    case 45:
        ++curr;
        goto s148;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_SELECT};
    }
s133:
    switch (*curr) {
    case 40:
        ++curr;
        goto s121;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s134:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_RIGHTARROW};
    }
s135:
    switch (*curr) {
    case 111:
        ++curr;
        goto s45;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s136:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s144;
    case 116:
        ++curr;
        goto s146;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s137:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s142;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s138:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s120;
    case 117:
        ++curr;
        goto s140;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s139:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s109;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s140:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s206;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s141:
    switch (*curr) {
    case 101:
        ++curr;
        goto s67;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s142:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s204;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s143:
    switch (*curr) {
    case 117:
        ++curr;
        goto s65;
    case 101:
        ++curr;
        goto s66;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s144:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 122:
        ++curr;
        goto s68;
    case 121:
        ++curr;
        goto s202;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s145:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_LBRACKET};
    }
s146:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s203;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s147:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_SET_MINUS};
    }
s148:
    switch (*curr) {
    case 66:
        ++curr;
        goto s198;
    case 70:
        ++curr;
        goto s199;
    case 73:
        ++curr;
        goto s200;
    case 84:
        ++curr;
        goto s201;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s149:
    switch (*curr) {
    case 110:
        ++curr;
        goto s62;
    case 102:
        ++curr;
        goto s63;
    case 100:
        ++curr;
        goto s64;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s150:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_GREATEREQUAL};
    }
s151:
    switch (*curr) {
    case 101:
        ++curr;
        goto s61;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s152:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ASSIGN};
    }
s153:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s166;
    case 111:
        ++curr;
        goto s167;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s154:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BLOCKSTART};
    }
s155:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s168;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s156:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TEXT};
    }
s157:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s169;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s158:
    switch (*curr) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
    case 221:
    case 222:
    case 223:
    case 224:
    case 225:
    case 226:
    case 227:
    case 228:
    case 229:
    case 230:
    case 231:
    case 232:
    case 233:
    case 234:
    case 235:
    case 236:
    case 237:
    case 238:
    case 239:
    case 240:
    case 241:
    case 242:
    case 243:
    case 244:
    case 245:
    case 246:
    case 247:
    case 248:
    case 249:
    case 250:
    case 251:
    case 252:
    case 253:
    case 254:
    case 255:
        ++curr;
        goto s127;
    case 92:
        ++curr;
        goto s158;
    case 34:
        ++curr;
        goto s197;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s159:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s170;
    case 105:
        ++curr;
        goto s171;
    case 117:
        ++curr;
        goto s172;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s160:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s173;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s161:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BANGGT};
    }
s162:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 102:
        ++curr;
        goto s174;
    case 110:
        ++curr;
        goto s175;
    case 115:
        ++curr;
        goto s176;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s163:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_BANGLT};
    }
s164:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s177;
    case 105:
        ++curr;
        goto s178;
    case 111:
        ++curr;
        goto s179;
    case 117:
        ++curr;
        goto s180;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s165:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s181;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s166:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s215;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s167:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 117:
        ++curr;
        goto s216;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s168:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 122:
        ++curr;
        goto s68;
    case 116:
        ++curr;
        goto s217;
    case 121:
        ++curr;
        goto s218;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s169:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 100:
        ++curr;
        goto s219;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s170:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s220;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s171:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_FI};
    }
s172:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s221;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s173:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s222;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s174:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_IF};
    }
s175:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_IN};
    }
s176:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 45:
        ++curr;
        goto s223;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s177:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 120:
        ++curr;
        goto s224;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s178:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s225;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s179:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 100:
        ++curr;
        goto s226;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s180:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s227;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s181:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 116:
        ++curr;
        goto s228;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s182:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s229;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s183:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s230;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s184:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_OR};
    }
s185:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 105:
        ++curr;
        goto s231;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s186:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s232;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s187:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s233;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s188:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 100:
        ++curr;
        goto s234;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s189:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 117:
        ++curr;
        goto s235;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s190:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 112:
        ++curr;
        goto s236;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s191:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s237;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s192:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s238;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s193:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 105:
        ++curr;
        goto s239;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s194:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 114:
        ++curr;
        goto s240;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s195:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_PROJECT_PLUS};
    }
s196:
    switch (*curr) {
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_PROJECT_MINUS};
    }
s197:
    switch (*curr) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
    case 132:
    case 133:
    case 134:
    case 135:
    case 136:
    case 137:
    case 138:
    case 139:
    case 140:
    case 141:
    case 142:
    case 143:
    case 144:
    case 145:
    case 146:
    case 147:
    case 148:
    case 149:
    case 150:
    case 151:
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 184:
    case 185:
    case 186:
    case 187:
    case 188:
    case 189:
    case 190:
    case 191:
    case 192:
    case 193:
    case 194:
    case 195:
    case 196:
    case 197:
    case 198:
    case 199:
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
    case 210:
    case 211:
    case 212:
    case 213:
    case 214:
    case 215:
    case 216:
    case 217:
    case 218:
    case 219:
    case 220:
    case 221:
    case 222:
    case 223:
    case 224:
    case 225:
    case 226:
    case 227:
    case 228:
    case 229:
    case 230:
    case 231:
    case 232:
    case 233:
    case 234:
    case 235:
    case 236:
    case 237:
    case 238:
    case 239:
    case 240:
    case 241:
    case 242:
    case 243:
    case 244:
    case 245:
    case 246:
    case 247:
    case 248:
    case 249:
    case 250:
    case 251:
    case 252:
    case 253:
    case 254:
    case 255:
        ++curr;
        goto s127;
    case 34:
        ++curr;
        goto s156;
    case 92:
        ++curr;
        goto s158;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TEXT};
    }
s198:
    switch (*curr) {
    case 111:
        ++curr;
        goto s135;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s199:
    switch (*curr) {
    case 108:
        ++curr;
        goto s84;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s200:
    switch (*curr) {
    case 110:
        ++curr;
        goto s87;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s201:
    switch (*curr) {
    case 101:
        ++curr;
        goto s81;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s202:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_ANY};
    }
s203:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 109:
        ++curr;
        goto s95;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s204:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 108:
        ++curr;
        goto s98;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s205:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 97:
        ++curr;
        goto s104;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s206:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 99:
        ++curr;
        goto s101;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s207:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_INT};
    }
s208:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_REL};
    }
s209:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 116:
        ++curr;
        goto s108;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s210:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TYPE_TUP};
    }
s211:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ADD};
    }
s212:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s241;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s213:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_AND};
    }
s214:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 111:
        ++curr;
        goto s242;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s215:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s243;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s216:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 110:
        ++curr;
        goto s244;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s217:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 101:
        ++curr;
        goto s245;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s218:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s246;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s219:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_END};
    }
s220:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 115:
        ++curr;
        goto s247;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s221:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    case 99:
        ++curr;
        goto s248;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s222:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_HAS};
    }
s223:
    switch (*curr) {
    case 84:
        ++curr;
        goto s55;
    case 82:
        ++curr;
        goto s56;
    case 73:
        ++curr;
        goto s57;
    case 70:
        ++curr;
        goto s58;
    case 66:
        ++curr;
        goto s59;
    case 65:
        ++curr;
        goto s60;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::INVALID};
    }
s224:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MAX};
    }
s225:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MIN};
    }
s226:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_MOD};
    }
s227:
    switch (*curr) {
    case 116:
        ++curr;
        goto s54;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s228:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NOT};
    }
s229:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_ONE};
    }
s230:
    switch (*curr) {
    case 110:
        ++curr;
        goto s53;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s231:
    switch (*curr) {
    case 110:
        ++curr;
        goto s52;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s232:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_REL};
    }
s233:
    switch (*curr) {
    case 116:
        ++curr;
        goto s51;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s234:
    switch (*curr) {
    case 97:
        ++curr;
        goto s50;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s235:
    switch (*curr) {
    case 101:
        ++curr;
        goto s49;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s236:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_TUP};
    }
s237:
    switch (*curr) {
    case 101:
        ++curr;
        goto s48;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s238:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_VAL};
    }
s239:
    switch (*curr) {
    case 116:
        ++curr;
        goto s47;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s240:
    switch (*curr) {
    case 111:
        ++curr;
        goto s46;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s241:
    switch (*curr) {
    case 114:
        ++curr;
        goto s40;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s242:
    switch (*curr) {
    case 114:
        ++curr;
        goto s39;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s243:
    switch (*curr) {
    case 101:
        ++curr;
        goto s38;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s244:
    switch (*curr) {
    case 116:
        ++curr;
        goto s37;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s245:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_DATE};
    }
s246:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_DAYS};
    }
s247:
    switch (*curr) {
    case 101:
        ++curr;
        goto s36;
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_NAME};
    }
s248:
    switch (*curr) {
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 65:
    case 66:
    case 67:
    case 68:
    case 69:
    case 70:
    case 71:
    case 72:
    case 73:
    case 74:
    case 75:
    case 76:
    case 77:
    case 78:
    case 79:
    case 80:
    case 81:
    case 82:
    case 83:
    case 84:
    case 85:
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 95:
    case 97:
    case 98:
    case 99:
    case 100:
    case 101:
    case 102:
    case 103:
    case 104:
    case 105:
    case 106:
    case 107:
    case 108:
    case 109:
    case 110:
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
        ++curr;
        goto s68;
    default: 
        str = reinterpret_cast<const char*>(curr);
        return Token{reinterpret_cast<const char*>(start), reinterpret_cast<const char*>(curr), TokenType::TK_FUNC};
    }

}

} // end namespace lexer
