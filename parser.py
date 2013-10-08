from lexer import *


class ParserException(Exception):
    def __init__(self, hat, TK):
        self.TK = TK
        Exception.__init__(self, hat)

class RecoverException(Exception):
    pass

class RecoverEOF(RecoverException):
    TK=TK_EOF

class RecoverBLOCKEND(RecoverException):
    TK=TK_BLOCKEND

class RecoverIN(RecoverException):
    TK=TK_IN

class RecoverVAL(RecoverException):
    TK=TK_VAL

class RecoverRPAREN(RecoverException):
    TK=TK_RPAREN

class RecoverSEMICOLON(RecoverException):
    TK=TK_SEMICOLON

class Parser:
    def __init__(self, error, code):
        self.error = error
        self.code = code
        self.lexer = Lexer(code)
        self.currentToken = self.lexer.getNext()
        self.recoverStack = []
        
    def nextToken(self):
        self.currentToken = self.lexer.getNext()

    def pushRecover(self, rec):
        self.recoverStack.append(rec)

    def popRecover(self):
        self.recoverStack.pop()

    def recover(self):
        """Recover errors at the first token in list of tokens specified on the recovery stack"""
        r = dict( (l.TK, l) for l in self.recoverStack)
        while not self.currentToken[0] in r:
            if self.currentToken[0] == TK_ERR:
                self.parseError("Invalid token")
            self.nextToken()
        raise r[self.currentToken[0]]()
        
    def parseError(self, error):
        TK=self.currentToken
        self.error.reportError("Parse error: %s" %error,
                               self.currentToken)
                
    def parseType(self):
        cToken = self.currentToken[0]
        if cToken in [TK_TYPE_BOOL,TK_TYPE_INT,TK_TYPE_TEXT,TK_TYPE_ATOM,TK_TYPE_TUP,TK_TYPE_REL,TK_TYPE_FUNC,TK_TYPE_ANY]:
            self.nextToken()
            return
        self.parseError("Expected Type");
        self.recover()

    def parseAssignment(self):       
        self.nextToken()
        if self.currentToken[0] == TK_ASSIGN:
            self.nextToken()
            self.parseExp()

    def parseBottomExp(self):
        cToken = self.currentToken[0]
        if False:
            pass
        elif cToken == TK_PIPE:
            self.nextToken()
            self.parseExp()
            self.assertTokenConsume(TK_PIPE)
        elif cToken == TK_MINUS:
            self.nextToken()
            self.parseExp()
        elif cToken == TK_NAME:
            self.parseAssignment()
        elif cToken == TK_FUNC:
            self.nextToken()
            self.assertTokenConsume(TK_LPAREN)
            self.pushRecover(RecoverRPAREN);
            try:
                if self.currentToken[0] != TK_RPAREN:
                    self.assertTokenConsume(TK_NAME)
                self.assertTokenConsume(TK_COLON)
                self.parseType()
                while self.currentToken[0] == TK_COMMA:
                    self.nextToken()
                    self.assertTokenConsume(TK_NAME)
                    self.assertTokenConsume(TK_COLON)
                    self.parseType()
                self.assertToken(TK_RPAREN)
            except RecoverRPAREN:
                pass
            finally:
                self.popRecover()
            self.nextToken()
            self.assertTokenConsume(TK_RIGHTARROW)
            self.assertTokenConsume(TK_LPAREN)
            self.parseType()
            self.assertTokenConsume(TK_RPAREN)
            self.pushRecover(RecoverEnd)
            try:
                self.parseExp()
            except RecoverEND:
                pass
            finally:
                self.popRecover()
            self.assertTokenConsume(TK_END)
        elif cToken == TK_REL:
            self.nextToken()
            self.assertTokenConsume(TK_LPAREN)
            self.pushRecover(RecoverRPAREN)
            try:
                self.parseExp()
                self.assertToken(TK_RPAREN)
            except RecoverRPAREN:
                pass
            finally:
                self.popRecover()
            self.nextToken()
        elif cToken == TK_TUP:
            self.nextToken()
            self.assertTokenConsume(TK_LPAREN)
            self.pushRecover(RecoverRPAREN)
            try:
                if self.currentToken[0] != TK_RPAREN:
                    self.assertTokenConsume(TK_NAME)
                    self.assertTokenConsume(TK_COLON)
                    self.parseExp()
                    while self.currentToken[0] == TK_COMMA:
                        self.nextToken()
                        self.assertTokenConsume(TK_NAME)
                        self.assertTokenConsume(TK_COLON)
                        self.parseExp()
                    self.assertToken(TK_RPAREN)
            except RecoverRPAREN:
                pass
            finally:
                self.popRecover()
            self.nextToken()
        elif cToken in [TK_ZERO, TK_ONE, TK_STDBOOL, TK_STDINT, TK_STDTEXT, TK_SHARP]:
            self.nextToken()
        elif cToken == TK_TEXT:
            self.nextToken()
        elif cToken == TK_INT:
            self.nextToken()
        elif cToken == TK_BLOCKSTART:
            self.nextToken()
            self.pushRecover(RecoverBLOCKEND)
            try: 
                self.pushRecover(RecoverIN)                
                try:
                    while self.currentToken[0] == TK_VAL:
                        self.nextToken()
                        self.pushRecover(RecoverVAL)
                        try:
                            self.assertTokenConsume(TK_NAME)
                            self.assertTokenConsume(TK_EQUAL)
                            self.parseExp()
                        except RecoverVAL:
                            pass
                        finally:
                            self.popRecover()
                    self.assertToken(TK_IN)
                except RecoverIN:
                    pass
                finally:
                    self.popRecover()
                self.nextToken()
                self.parseExp()
                self.assertToken(TK_BLOCKEND)
            except RecoverBLOCKEND:
                pass
            finally:
                self.popRecover()
            self.nextToken()
        elif cToken == TK_AT:
            self.nextToken()
            self.assertTokenConsume(TK_LPAREN)
            self.pushRecover(RecoverRPAREN)
            try:
                self.parseExp()
                self.assertToken(TK_RPAREN)
            except RecoverRPAREN:
                pass
            finally:
                self.popRecover()
            self.nextToken()
        elif cToken == TK_NOT:
            self.nextToken()
            self.parseExp()
        elif cToken in [TK_TRUE,TK_FALSE]:
            self.nextToken()
        else:
            self.parseError("Unexpected token")
            self.recover()


    def parseCompareExp(self):
        self.parsePlusMinusExp()
        if self.currentToken[0] in [TK_EQUAL, TK_DIFFERENT, 
                                    TK_LESS, TK_GREATER, 
                                    TK_LESSEQUAL, TK_GREATEREQUAL]:
            self.nextToken()
            self.parsePlusMinusExp()

    def parseOpExtendAndOverwriteExp(self):
        self.parseBottomExp()
        while self.currentToken[0] == TK_OPEXTEND:
            self.nextToken()
            self.parseBottomExp()

    def parseSubstringExp(self):
        self.parseBottomExp()
        if self.currentToken[0] == TK_LPAREN:
            self.nextToken()
            self.parseExp()
            self.assertTokenConsume(TK_TWO_DOTS)
            self.parseExp()
            self.assertTokenConsume(TK_RPAREN)
            
    def parseConcatExp(self):
        self.parseOpExtendAndOverwriteExp()
        while self.currentToken[0] == TK_CONCAT:
            self.nextToken()
            self.parseOpExtendAndOverwriteExp()

    def parseSequenceExp(self):
        self.parseOrExp()
        while self.currentToken[0] == TK_SEMICOLON:
            self.nextToken()
            self.pushRecover(RecoverSEMICOLON)
            try:
                self.parseOrExp()
            except RecoverSEMICOLON:
                pass
            finally:
                self.popRecover()

    def parseAndExp(self):
        self.parseCompareExp()
        while self.currentToken[0] == TK_AND:
            self.nextToken()
            self.parseCompareExp()

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
        self.pushRecover(RecoverEOF)
        try:
            self.parseExp()
            self.assertTokenConsume(TK_EOF)
        except RecoverEOF:
            pass
        finally:
            self.popRecover()
            

    def parseExp(self):
        self.parseSequenceExp()     
        
    def assertToken(self, TK):
        if TK != self.currentToken[0]:
            self.parseError("Expected %s at "%tokenNames[TK]);
            self.recover()

    def assertTokenConsume(self, TK):
        self.assertToken(TK)
        self.nextToken()

