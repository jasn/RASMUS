TK_ERR = 255
TK_EOF = 254

TK_TUP = 1
TK_REL = 2
TK_FUNC = 3
TK_END = 4
TK_AT = 5
TK_SHARP = 6
TK_NOT = 7
TK_MINUS = 8
TK_AND = 9
TK_OR = 10  
TK_TRUE = 11
TK_FALSE = 12
TK_LPAREN = 13
TK_RPAREN = 14
TK_BLOCKSTART = 15
TK_BLOCKEND = 16
TK_VAL = 17
TK_IN = 18
TK_NAME = 19
TK_COLON = 20
TK_INT = 21
TK_TEXT = 22
TK_ZERO = 23
TK_ONE = 24
TK_STDBOOL = 25
TK_STDINT = 26
TK_STDTEXT = 27
TK_COMMA = 28
TK_FUNC = 29
TK_END = 30
TK_RIGHTARROW = 31
TK_TYPE_BOOL = 32
TK_TYPE_INT = 33
TK_TYPE_TEXT = 34
TK_TYPE_ATOM = 35
TK_TYPE_TUP = 36
TK_TYPE_REL = 37
TK_TYPE_FUNC = 38
TK_TYPE_ANY = 39
TK_EQUAL = 40
TK_ASSIGN = 41
TK_PLUS = 42
TK_MUL = 43
TK_DIV = 44
TK_MOD = 45
TK_SEMICOLON = 46
TK_CONCAT = 47

tokenMap = sorted(
    [(TK_TUP, "tup"), 
     (TK_REL, "rel"),
     (TK_AT, "@"),
     (TK_TRUE, "true"),
     (TK_FALSE, "false"), 
     (TK_SHARP, "#"),
     (TK_LPAREN, "("), 
     (TK_RPAREN, ")"), 
     (TK_NOT, "not"),
     (TK_MINUS, "-"),
     (TK_OR, "or"),
     (TK_AND, "and"),
     (TK_BLOCKSTART, "(+"),
     (TK_BLOCKEND, "+)"), 
     (TK_VAL, "val"),
     (TK_IN, "in"),
     (TK_COLON, ":"),
     (TK_ZERO, "zero"),
     (TK_ONE, "one"),
     (TK_STDBOOL, "?-Bool"),
     (TK_STDINT, "?-Int"),
     (TK_STDTEXT, "?-Text"),
     (TK_COMMA, ","),
     (TK_FUNC, "func"),
     (TK_END, "end"),
     (TK_RIGHTARROW, "->"),
     (TK_TYPE_BOOL, "Bool"),
     (TK_TYPE_INT, "Int"),
     (TK_TYPE_TEXT, "Text"),
     (TK_TYPE_ATOM, "Atom"),
     (TK_TYPE_TUP, "Tup"),
     (TK_TYPE_REL, "Rel"),
     (TK_TYPE_FUNC, "Func"),
     (TK_TYPE_ANY, "Any"),
     (TK_EQUAL, "="),
     (TK_ASSIGN, ":="),
     (TK_PLUS, "+"),
     (TK_MUL, "*"),
     (TK_DIV, "/"),
     (TK_MOD, "mod"),
     (TK_SEMICOLON, ";"),
     (TK_CONCAT, "++"),
     ],
    key=lambda p : -len(p[1])
    )

mapToken = {}
for k,v in tokenMap:
    mapToken[k] = "\"%s\""%v
mapToken[TK_EOF] = "End of file"
mapToken[TK_NAME] = "Name"
mapToken[TK_INT] = "Int"
mapToken[TK_TEXT] = "Text"

class Lexer:

    def __init__(self, code):
        self.code = code
        self.index = 0
        

    def getNext(self):
        c, i = self.code, self.index
        whitespace = " \t\r\n"
        while i < len(c) and c[i] in whitespace:
            i+=1

        self.index = i

        if i == len(c):
            return (TK_EOF, i, 0)

        for key,val in tokenMap:
            if c[i:i+len(val)] == val:
                self.index += len(val)
                return (key, i, len(val))

        # check if is Name
        if c[i].isalpha():
            j = 0
            while i+j < len(c) and c[i+j].isalnum():
                j+=1
            self.index += j
            return (TK_NAME, i, j)
        
        # check if it is an int
        if c[i].isdigit():
            j = 0
            while i+j < len(c) and c[i+j].isdigit():
                j+=1
            self.index += j
            return (TK_INT, i, j)    

        # check if it is a text
        if c[i] == '"':
            j = 1
            while i + j < len(c) and c[i+j] != '"':
                j+=1
            if i+j == len(c):
                self.index += j
                return (TK_ERR, i, j)
            self.index += j+1
            return (TK_TEXT, i, j+1)
        
        j = 0
        while i+j < len(c) and c[i+j] not in " \t\r\n":
            j = j + 1
        self.index += j
        return (TK_ERR, i, j)

