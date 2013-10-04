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

class Lexer:

    def __init__(self, code):
        self.code = code
        self.index = 0
        self.tokenMap = sorted(
            [(TK_TUP, "tup"), (TK_REL, "rel")],
            key=lambda p : len(p[1])
            )

    def getNext(self):
        c, i = self.code, self.index
        whitespace = " \t\r\n"
        while i < len(c) and c[i] in whitespace:
            i+=1

        self.index = i

        if i == len(c):
            return (TK_EOF, i, 0)

        for key,val in self.tokenMap:
            if c[i:i+len(val)] == val:
                self.index += len(val)
                return (key, i, len(val))

        j = 0
        while i+j < len(c) and c[i+j] not in " \t\r\n":
            j = j + 1
        self.index += j
        return (TK_ERR, i, j)

