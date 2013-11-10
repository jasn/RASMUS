__nn = iter(xrange(20000)).next

TK_ADD = __nn()
TK_AFTER = __nn()
TK_AND = __nn()
TK_ASSIGN = __nn()
TK_AT = __nn()
TK_BANG = __nn()
TK_BANGGT = __nn()
TK_BANGLT = __nn()
TK_BEFORE = __nn()
TK_BLOCKEND = __nn()
TK_BLOCKSTART = __nn()
TK_CHOICE = __nn()
TK_CLOSE = __nn()
TK_COLON = __nn()
TK_COMMA = __nn()
TK_CONCAT = __nn()
TK_COUNT = __nn()
TK_DATE = __nn()
TK_DAYS = __nn()
TK_DIFFERENT = __nn()
TK_DIV = __nn()
TK_END = __nn()
TK_EOF = __nn()
TK_EQUAL = __nn()
TK_ERR = __nn()
TK_FALSE = __nn()
TK_FI = __nn()
TK_FUNC = __nn()
TK_GREATER = __nn()
TK_GREATEREQUAL = __nn()
TK_HAS = __nn()
TK_IF = __nn()
TK_IN = __nn()
TK_INT = __nn()
TK_ISANY = __nn()
TK_ISATOM = __nn()
TK_ISBOOL = __nn()
TK_ISFUNC = __nn()
TK_ISINT = __nn()
TK_ISREL = __nn()
TK_ISTEXT = __nn()
TK_ISTUP = __nn()
TK_LBRACKET = __nn()
TK_LEFT_ARROW = __nn()
TK_LESS = __nn()
TK_LESSEQUAL = __nn()
TK_LPAREN = __nn()
TK_MAX = __nn()
TK_MIN = __nn()
TK_MINUS = __nn()
TK_MOD = __nn()
TK_MUL = __nn()
TK_MULT = __nn()
TK_NAME = __nn()
TK_NOT = __nn()
TK_ONE = __nn()
TK_ONE_DOT = __nn()
TK_OPEN = __nn()
TK_OPEXTEND = __nn()
TK_OR = __nn()
TK_PIPE = __nn()
TK_PLUS = __nn()
TK_PROJECT_MINUS = __nn()
TK_PROJECT_PLUS = __nn()
TK_QUESTION = __nn()
TK_RBRACKET = __nn()
TK_REL = __nn()
TK_RIGHTARROW = __nn()
TK_RPAREN = __nn()
TK_SEMICOLON = __nn()
TK_SET_MINUS = __nn()
TK_SHARP = __nn()
TK_STDBOOL = __nn()
TK_STDINT = __nn()
TK_STDTEXT = __nn()
TK_SYSTEM = __nn()
TK_TEXT = __nn()
TK_TILDE = __nn()
TK_TODAY = __nn()
TK_TRUE = __nn()
TK_TUP = __nn()
TK_TWO_DOTS = __nn()
TK_TYPE_ANY = __nn()
TK_TYPE_ATOM = __nn()
TK_TYPE_BOOL = __nn()
TK_TYPE_FUNC = __nn()
TK_TYPE_INT = __nn()
TK_TYPE_REL = __nn()
TK_TYPE_TEXT = __nn()
TK_TYPE_TUP = __nn()
TK_VAL = __nn()
TK_WRITE = __nn()
TK_ZERO = __nn()

operators = [
    (TK_ASSIGN, ":="),
    (TK_AT, "@"),
    (TK_BANG, "!"),
    (TK_BANGGT, "!>"),
    (TK_BANGLT, "!<"),
    (TK_BLOCKEND, "+)"), 
    (TK_BLOCKSTART, "(+"),
    (TK_CHOICE, "&"),
    (TK_COLON, ":"),
    (TK_COMMA, ","),
    (TK_CONCAT, "++"),
    (TK_DIFFERENT, "<>"),
    (TK_DIV, "/"),
    (TK_EQUAL, "="),
    (TK_GREATER, ">"),
    (TK_GREATEREQUAL, ">="),
    (TK_ISANY, "is-Any"),
    (TK_ISATOM, "is-Atom"),
    (TK_ISBOOL, "is-Bool"),
    (TK_ISFUNC, "is-Func"),
    (TK_ISINT, "is-Int"),
    (TK_ISREL, "is-Rel"),
    (TK_ISTEXT, "is-Text"),
    (TK_ISTUP, "is-Tup"),
    (TK_LBRACKET, "["),
    (TK_LEFT_ARROW, "<-"),
    (TK_LESS, "<"),
    (TK_LESSEQUAL, "<="),
    (TK_LPAREN, "("), 
    (TK_MINUS, "-"),
    (TK_MUL, "*"),
    (TK_ONE_DOT, "."),
    (TK_OPEXTEND, "<<"),
    (TK_PIPE, "|"),
    (TK_PLUS, "+"),
    (TK_PROJECT_MINUS, "|-"),
    (TK_PROJECT_PLUS, "|+"),
    (TK_QUESTION, "?"),
    (TK_RBRACKET, "]"),
    (TK_RIGHTARROW, "->"),
    (TK_RPAREN, ")"), 
    (TK_SEMICOLON, ";"),
    (TK_SET_MINUS, "\\"),
    (TK_SHARP, "#"),
    (TK_STDBOOL, "?-Bool"),
    (TK_STDINT, "?-Int"),
    (TK_STDTEXT, "?-Text"),
    (TK_TILDE, "~"),
    (TK_TWO_DOTS, ".."),
    ]

keywords = [
    (TK_ADD, "add"),
    (TK_AFTER, "after"),
    (TK_AND, "and"),
    (TK_BEFORE, "before"),
    (TK_CLOSE, "close"),
    (TK_COUNT, "count"),
    (TK_DATE, "date"),
    (TK_DAYS, "days"),
    (TK_END, "end"),
    (TK_FALSE, "false"), 
    (TK_FI, "fi"),
    (TK_FUNC, "func"),
    (TK_HAS, "has"),
    (TK_IF, "if"),
    (TK_IN, "in"),
    (TK_MAX, "max"),
    (TK_MIN, "min"),
    (TK_MOD, "mod"),
    (TK_MULT, "mult"),
    (TK_NOT, "not"),
    (TK_ONE, "one"),
    (TK_OPEN, "open"),
    (TK_OR, "or"),
    (TK_REL, "rel"),
    (TK_SYSTEM, "system"),
    (TK_TODAY, "today"),
    (TK_TRUE, "true"),
    (TK_TUP, "tup"), 
    (TK_TYPE_ANY, "Any"),
    (TK_TYPE_ATOM, "Atom"),
    (TK_TYPE_BOOL, "Bool"),
    (TK_TYPE_FUNC, "Func"),
    (TK_TYPE_INT, "Int"),
    (TK_TYPE_REL, "Rel"),
    (TK_TYPE_TEXT, "Text"),
    (TK_TYPE_TUP, "Tup"),
    (TK_VAL, "val"),
    (TK_WRITE, "write"),
    (TK_ZERO, "zero"),
    ]

tokenNames = {}
for k,v in operators:
    tokenNames[k] = "\"%s\""%v
for k,v in keywords:
    tokenNames[k] = "\"%s\""%v
tokenNames[TK_EOF] = "End of file"
tokenNames[TK_ERR] = "Bad token"
tokenNames[TK_NAME] = "Name"
tokenNames[TK_INT] = "Int"
tokenNames[TK_TEXT] = "Text"

maxLength = max([len(value) for _,value in operators])

operators_map = [{} for _ in range(maxLength+1)]

for key, value in operators:
    operators_map[len(value)][value] = key

keywords_map = {}
for key, value in keywords:
    keywords_map[value] = key

class Token:
    def __init__(self, id, start, length):
        self.id = id
        self.start = start
        self.length = length

whitespace = " \t\r\n"
class Lexer:
    def __init__(self, code, start=0):
        self.code = code
        self.index = start

    def getNext(self):
        c, i = self.code.code, self.index
        while i < len(c) and c[i] in whitespace:
            i+=1

        self.index = i

        if i == len(c):
           return Token(TK_EOF, i, 0)

        # check if is operator
        for l in range(len(operators_map)-1, -1, -1):
            if c[i:i+l] in operators_map[l]:
                self.index += l
                return Token(operators_map[l][c[i:i+l]], i, l)

        # check if is Name or keyword
        if c[i].isalpha():
            j = 0
            while i+j < len(c) and c[i+j].isalnum():
                j+=1
            self.index += j
            # check if is keyword
            if c[i:i+j] in keywords_map: 
                return Token(keywords_map[c[i:i+j]], i, j)
            return Token(TK_NAME, i, j)
        
        # check if it is an int
        if c[i].isdigit():
            j = 0
            while i+j < len(c) and c[i+j].isdigit():
                j+=1
            self.index += j
            return Token(TK_INT, i, j)    

        # check if it is a text
        if c[i] == '"':
            j = 1
            while i + j < len(c) and c[i+j] != '"':
                j+=1
            if i+j == len(c):
                self.index += j
                return Token(TK_ERR, i, j)
            self.index += j+1
            return Token(TK_TEXT, i, j+1)
        
        # skip invalid token
        j = 0
        while i+j < len(c) and c[i+j] not in " \t\r\n":
            j = j + 1
        self.index += j
        return Token(TK_ERR, i, j)
