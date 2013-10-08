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

operators = [
        (TK_AT, "@"),
        (TK_SHARP, "#"),
        (TK_LPAREN, "("), 
        (TK_RPAREN, ")"), 
        (TK_MINUS, "-"),
        (TK_BLOCKSTART, "(+"),
        (TK_BLOCKEND, "+)"), 
        (TK_COLON, ":"),
        (TK_STDBOOL, "?-Bool"),
        (TK_STDINT, "?-Int"),
        (TK_STDTEXT, "?-Text"),
        (TK_COMMA, ","),
        (TK_RIGHTARROW, "->"),
        (TK_EQUAL, "="),
        (TK_ASSIGN, ":="),
        (TK_PLUS, "+"),
        (TK_MUL, "*"),
        (TK_DIV, "/"),
        (TK_SEMICOLON, ";"),
        (TK_CONCAT, "++"),
        ]

keywords = [(TK_TUP, "tup"), 
            (TK_REL, "rel"),
            (TK_TRUE, "true"),
            (TK_FALSE, "false"), 
            (TK_NOT, "not"),
            (TK_OR, "or"),
            (TK_AND, "and"),
            (TK_VAL, "val"),
            (TK_IN, "in"),
            (TK_ZERO, "zero"),
            (TK_ONE, "one"),
            (TK_FUNC, "func"),
            (TK_END, "end"),
            (TK_TYPE_BOOL, "Bool"),
            (TK_TYPE_INT, "Int"),
            (TK_TYPE_TEXT, "Text"),
            (TK_TYPE_ATOM, "Atom"),
            (TK_TYPE_TUP, "Tup"),
            (TK_TYPE_REL, "Rel"),
            (TK_TYPE_FUNC, "Func"),
            (TK_TYPE_ANY, "Any"),
            (TK_MOD, "mod"),
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

operators_map = [{} for _ in range(7)]

for key, value in operators:
    operators_map[len(value)][value] = key

keywords_map = {}
for key, value in keywords:
    keywords_map[value] = key

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

        # check if is operator
        for l in reversed(range(len(operators_map))):
            if c[i:i+l] in operators_map[l]:
                self.index += l
                return (operators_map[l][c[i:i+l]], i, l)

        # check if is Name or keyword
        if c[i].isalpha():
            j = 0
            while i+j < len(c) and c[i+j].isalnum():
                j+=1
            self.index += j
            # check if is keyword
            if c[i:i+j] in keywords_map: 
                return (keywords_map[c[i:i+j]], i, j)
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
        
        # skip invalid token
        j = 0
        while i+j < len(c) and c[i+j] not in " \t\r\n":
            j = j + 1
        self.index += j
        return (TK_ERR, i, j)

