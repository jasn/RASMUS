from lexer import *

class ParserException(Exception):

    def __init__(self, hat, TK):
        self.TK = TK
        Exception.__init__(self, hat)

class Parser:

    def __init__(self, code):
        self.code = code
        self.lexer = Lexer(code)
        self.currentToken = self.lexer.getNext()

    def nextToken(self):
        self.currentToken = self.lexer.getNext()

    def parseBottomExp(self):
        cToken = self.currentToken[0]
        if False:
            pass
        elif cToken == TK_BLOCKSTART:
            self.nextToken()
            while self.currentToken[0] == TK_VAL:
                self.nextToken()
                self.assertToken(TK_NAME)
                self.assertToken(TK_COLON)
                self.parseExp()
            self.assertToken(TK_IN)
            self.parseExp()
            self.assertToken(TK_BLOCKEND)
            pass
        elif cToken == TK_AT:
            self.nextToken()
            self.assertToken(TK_LPAREN)
            self.parseExp()
            self.assertToken(TK_RPAREN)
        elif cToken == TK_NOT:
            self.nextToken()
            self.parseExp()
        elif cToken == TK_TRUE:
            self.nextToken()
        elif cToken == TK_FALSE:
            self.nextToken()
        else:
            raise ParserException("Unexpected token", self.currentToken)
            
    def parse(self) :
        try: 
            self.parseExp()
            self.assertToken(TK_EOF)
        except ParserException as e:
            startOfLine = self.code.rfind("\n", 0, e.TK[1])
            if startOfLine == -1:
                startOfLine = 0

            endOfLine = self.code.find("\n", startOfLine+1)
            if endOfLine == -1:
                endOfLine = len(self.code)

            print "Parse error: %s " %e
            print self.code[startOfLine:endOfLine]
            print "%s^%s"%(" "*(e.TK[1]-startOfLine), "~"*(e.TK[2]-1))

    def parseExp(self):
        self.parseBottomExp();
        

    def assertToken(self, TK):
        if TK != self.currentToken[0]:
            # raise an error
            raise ParserException("Expected %s at "%mapToken[TK], self.currentToken)
        self.nextToken()
