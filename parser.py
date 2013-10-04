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

    def parseType(self):
        cToken = self.currentToken[0]
        if cToken in [TK_TYPE_BOOL,TK_TYPE_INT,TK_TYPE_TEXT,TK_TYPE_ATOM,TK_TYPE_TUP,TK_TYPE_REL,TK_TYPE_FUNC,TK_TYPE_ANY]:
            self.nextToken()
            return
        raise ParserException("Expected Type", self.currentToken)

    def parseAssignment(self):       
        self.nextToken()
        if self.currentToken[0] == TK_ASSIGN:
            self.nextToken()
            self.parseExp()

    def parseBottomExp(self):
        cToken = self.currentToken[0]
        if False:
            pass
        elif cToken == TK_MINUS:
            self.nextToken()
            self.parseExp()
        elif cToken == TK_NAME:
            self.parseAssignment()
        elif cToken == TK_FUNC:
            self.nextToken()
            self.assertToken(TK_LPAREN)
            if self.currentToken[0] != TK_RPAREN:
                self.assertToken(TK_NAME)
                self.assertToken(TK_COLON)
                self.parseType()
                while self.currentToken[0] == TK_COMMA:
                    self.nextToken()
                    self.assertToken(TK_NAME)
                    self.assertToken(TK_COLON)
                    self.parseType()
            self.assertToken(TK_RPAREN)
            self.assertToken(TK_RIGHTARROW)
            self.assertToken(TK_LPAREN)
            self.parseType()
            self.assertToken(TK_RPAREN)
            self.parseExp()
            self.assertToken(TK_END)
        elif cToken == TK_REL:
            self.nextToken()
            self.assertToken(TK_LPAREN)
            self.parseExp()
            self.assertToken(TK_RPAREN)
        elif cToken == TK_TUP:
            self.nextToken()
            self.assertToken(TK_LPAREN)
            if self.currentToken[0] != TK_RPAREN:
                self.assertToken(TK_NAME)
                self.assertToken(TK_COLON)
                self.parseExp()
                while self.currentToken[0] == TK_COMMA:
                    self.nextToken()
                    self.assertToken(TK_NAME)
                    self.assertToken(TK_COLON)
                    self.parseExp()
                    
            self.assertToken(TK_RPAREN)
        elif cToken in [TK_ZERO, TK_ONE, TK_STDBOOL, TK_STDINT, TK_STDTEXT, TK_SHARP]:
            self.nextToken()
        elif cToken == TK_TEXT:
            self.nextToken()
        elif cToken == TK_INT:
            self.nextToken()
        elif cToken == TK_BLOCKSTART:
            self.nextToken()
            while self.currentToken[0] == TK_VAL:
                self.nextToken()
                self.assertToken(TK_NAME)
                self.assertToken(TK_EQUAL)
                self.parseExp()
            self.assertToken(TK_IN)
            self.parseExp()
            self.assertToken(TK_BLOCKEND)
        elif cToken == TK_AT:
            self.nextToken()
            self.assertToken(TK_LPAREN)
            self.parseExp()
            self.assertToken(TK_RPAREN)
        elif cToken == TK_NOT:
            self.nextToken()
            self.parseExp()
        elif cToken in [TK_TRUE,TK_FALSE]:
            self.nextToken()
        else:
            raise ParserException("Unexpected token", self.currentToken)

    def parseConcatExp(self):
        self.parseBottomExp()
        while self.currentToken[0] == TK_CONCAT:
            self.nextToken()
            self.parseBottomExp()

    def parseSequenceExp(self):
        self.parseOrExp()
        while self.currentToken[0] == TK_SEMICOLON:
            self.nextToken()
            self.parseOrExp()

    def parseAndExp(self):
        self.parsePlusMinusExp()
        while self.currentToken[0] == TK_AND:
            self.nextToken()
            self.parsePlusMinusExp()

    def parseOrExp(self):
        self.parseAndExp()
        while self.currentToken[0] == TK_OR:
            self.nextToken()
            self.parseAndExp()
        
    def parseMulDivExp(self):
        self.parseConcatExp()
        while self.currentToken[0] in [TK_DIV, TK_MUL, TK_MOD]:
            self.nextToken()
            self.parseConcatExp()


    def parsePlusMinusExp(self):
        self.parseMulDivExp()
        while self.currentToken[0] in [TK_PLUS, TK_MINUS]:
            self.nextToken()
            self.parseMulDivExp()

    def parse(self) :
        try: 
            self.parseExp()
            self.assertToken(TK_EOF)
        except ParserException as e:
            startOfLine = self.code.rfind("\n", 0, e.TK[1])
            startOfLine += 1
            endOfLine = self.code.find("\n", startOfLine+1)
            if endOfLine == -1:
                endOfLine = len(self.code)

            print "Parse error: %s " %e
            print self.code[startOfLine:endOfLine]
            print "%s^%s"%(" "*(e.TK[1]-startOfLine), "~"*(e.TK[2]-1))

    def parseExp(self):
        self.parseSequenceExp()     
        
    def assertToken(self, TK):
        if TK != self.currentToken[0]:
            # raise an error
            raise ParserException("Expected %s at "%mapToken[TK], self.currentToken)
        self.nextToken()
