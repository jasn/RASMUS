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

tokenMap = sorted(
    [(TK_TUP, "tup"), 
     (TK_REL, "rel"),
     (TK_AT, "@"),
     (TK_TRUE, "true"),
     (TK_FALSE, "false"), 
     (TK_LPAREN, "("), 
     (TK_RPAREN, "("), 
     (TK_NOT, "not"),
     (TK_BLOCKSTART, "(+"),
     (TK_BLOCKEND, "+)"), 
     (TK_VAL, "val"),
     (TK_IN, "in"),
     (TK_COLON, ":"),
     ],
    key=lambda p : -len(p[1])
    )

mapToken = {}
for k,v in tokenMap:
    mapToken[k] = "\"%s\""%v
mapToken[TK_EOF] = "End of file"
mapToken[TK_NAME] = "Name"

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
            
        j = 0
        while i+j < len(c) and c[i+j] not in " \t\r\n":
            j = j + 1
        self.index += j
        return (TK_ERR, i, j)

