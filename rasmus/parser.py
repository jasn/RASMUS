from lexer import *
from AST import *

thingsThatMayComeAfterParseExp = [TK_RIGHTARROW, TK_RPAREN, TK_COMMA, TK_FI,
                                  TK_PIPE, TK_COLON, TK_END, TK_IN, TK_CHOICE,
                                  TK_VAL, TK_TWO_DOTS, TK_EOF, TK_SEMICOLON, TK_BLOCKEND]

class RecoverException(Exception):
    def __init__(self, token): 
        self.token = token

class Recover:
     def __init__(self, parser, token):
         self.parser = parser
         self.token = token
    
     def __enter__(self):
         self.parser.recoverStack.append(self.token)

     def __exit__(self):
         self.parser.recoverStack.pop()

     def __exit__(self, type, value, trace):
         self.parser.recoverStack.pop()
         if isinstance(value, RecoverException):
             return value.token == self.token
         return False

class IncompleteInputException(Exception):
    pass

class Parser:
    def __init__(self, error, code, interactiveMode = False, start=0):
        self.interactiveMode = interactiveMode
        self.error = error
        self.code = code
        self.lexer = Lexer(code, start)
        self.currentToken = self.lexer.getNext()
        self.recoverStack = []
        
    def consumeToken(self):
        token = self.currentToken
        self.currentToken = self.lexer.getNext()
        return token

    def recover(self):
        """Recover errors at the first token in list of tokens specified on the recovery stack"""
        r = {}
        for x in self.recoverStack:
            r[x] = True
        if self.currentToken.id in r:
            raise RecoverException(self.currentToken.id)
        self.consumeToken()
        while not self.currentToken.id in r:
            if self.currentToken.id == TK_EOF:
                raise RecoverException(TK_EOF)
            elif self.currentToken.id == TK_ERR:
                self.parseError("Invalid token")
            self.consumeToken()
        raise RecoverException(self.currentToken.id)
        
    def parseError(self, error):
        TK=self.currentToken
        self.error.reportError("Parse error: %s" %error,
                               self.currentToken)
                
    def parseType(self):
        cToken = self.currentToken.id
        if cToken in [TK_TYPE_BOOL,TK_TYPE_INT,
                      TK_TYPE_TEXT,TK_TYPE_ATOM,
                      TK_TYPE_TUP,TK_TYPE_REL,
                      TK_TYPE_FUNC,TK_TYPE_ANY]:
            return self.consumeToken()
        self.parseError("Expected Type");
        self.recover()

    def parseAssignment(self):
        nameToken = self.consumeToken()
        if self.currentToken.id == TK_ASSIGN:
            return AssignmentExp(
                nameToken,
                self.consumeToken(),
                self.parseCompareExp())
        else:
            return VariableExp(nameToken)

    def parseIfExp(self):
        n = IfExp(self.assertTokenConsume(TK_IF))
        with Recover(self, TK_FI):
            with Recover(self, TK_CHOICE):
                n.choices.append(Choice(
                        self.parseExp(),
                        self.assertTokenConsume(TK_RIGHTARROW),
                        self.parseExp()))
            while self.currentToken.id == TK_CHOICE:
                self.consumeToken()
                with Recover(self, TK_CHOICE):
                    n.choices.append(Choice(
                            self.parseExp(),
                            self.assertTokenConsume(TK_RIGHTARROW),
                            self.parseExp()))
            self.assertToken(TK_FI)
        n.fiToken = self.assertTokenConsume(TK_FI)
        return n

    def parseForallExp(self):
        n = ForallExp(
            self.consumeToken(),
            self.assertTokenConsume(TK_LPAREN),
            )
        with Recover(self, TK_RPAREN):
            n.listExps.append(self.parseExp())
            while self.currentToken.id == TK_COMMA:
                self.consumeToken()
                n.listExps.append(self.parseExp())
            self.assertToken(TK_RPAREN)
        n.rparenToken = self.assertTokenConsume(TK_RPAREN)
        if self.currentToken.id == TK_PIPE:
            n.pipeToken = self.consumeToken()
            n.names.append(self.assertTokenConsume(TK_NAME))
            while self.currentToken.id == TK_COMMA:
                self.consumeToken()
                n.names.append(self.assertTokenConsume(TK_NAME))
        n.colonToken = self.assertTokenConsume(TK_COLON)
        n.exp = self.parseExp()
        return n

    def parseBuiltIn(self):
        n = BuiltInExp(self.consumeToken(),
                       self.assertTokenConsume(TK_LPAREN))
        with Recover(self, TK_RPAREN):
            n.args.append(self.parseExp())
            while self.currentToken.id == TK_COMMA:
                self.consumeToken()
                n.args.append(self.parseExp())
            self.assertToken(TK_RPAREN)
        n.rparenToken = self.assertTokenConsume(TK_RPAREN)
        return n
    
    def parseFuncExp(self):
        n = FuncExp(self.consumeToken(),self.assertTokenConsume(TK_LPAREN))
        with Recover(self, TK_RPAREN):
            if self.currentToken.id != TK_RPAREN:
                n.args.append(FuncArg(
                        self.assertTokenConsume(TK_NAME),
                        self.assertTokenConsume(TK_COLON),
                        self.parseType()))
                while self.currentToken.id == TK_COMMA:
                    self.consumeToken()
                    n.args.append(FuncArg(
                            self.assertTokenConsume(TK_NAME),
                            self.assertTokenConsume(TK_COLON),
                            self.parseType()))
            self.assertToken(TK_RPAREN)
        n.rparenToken = self.consumeToken()
        n.arrowToken = self.assertTokenConsume(TK_RIGHTARROW)
        n.lparenToken2 = self.assertTokenConsume(TK_LPAREN)
        n.returnTypeToken = self.parseType()
        n.rparenToken2 = self.assertTokenConsume(TK_RPAREN)
        with Recover(self, TK_END):
            n.body = self.parseExp()
        n.endToken = self.assertTokenConsume(TK_END)
        return n

    def parseTupExp(self):
        n = TupExp(self.consumeToken(), self.assertTokenConsume(TK_LPAREN))
        with Recover(self, TK_RPAREN):
            if self.currentToken.id != TK_RPAREN:
                n.items.append(TupItem(
                        self.assertTokenConsume(TK_NAME),
                        self.assertTokenConsume(TK_COLON),
                        self.parseExp()))
                while self.currentToken.id == TK_COMMA:
                    self.consumeToken()
                    n.items.append(TupItem(
                            self.assertTokenConsume(TK_NAME),
                            self.assertTokenConsume(TK_COLON),
                            self.parseExp()))
                self.assertToken(TK_RPAREN)
        n.rparenToken = self.consumeToken()
        return n

    def parseBlockExp(self):
        n = BlockExp(self.consumeToken())
        with Recover(self, TK_BLOCKEND):
            with Recover(self, TK_IN):
                while self.currentToken.id == TK_VAL:
                    valToken = self.consumeToken()
                    with Recover(self, TK_VAL):
                        n.vals.append(Val(
                                valToken,
                                self.assertTokenConsume(TK_NAME), 
                                self.assertTokenConsume(TK_EQUAL),
                                self.parseExp()))
                self.assertToken(TK_IN)
            n.inToken = self.consumeToken()
            n.inExp = self.parseExp()
            self.assertToken(TK_BLOCKEND)
        n.blockendToken = self.consumeToken()
        return n

    def parseParenthesisExp(self):
        n = InvalidExp()
        self.consumeToken()
        with Recover(self, TK_RPAREN):
            n = self.parseExp()
            self.assertToken(TK_RPAREN)
        self.assertTokenConsume(TK_RPAREN)
        return n

    def parseRelExp(self):
        n = RelExp(self.consumeToken(), self.assertTokenConsume(TK_LPAREN))
        with Recover(self, TK_RPAREN):
            n.exp = self.parseExp()
            self.assertToken(TK_RPAREN)
        n.rparenToken = self.consumeToken()
        return n

    def parseAtExp(self):
        n = AtExp(self.consumeToken(), self.assertTokenConsume(TK_LPAREN))
        with Recover(self, TK_RPAREN):
            n.exp = self.parseExp()
            self.assertToken(TK_RPAREN)
        n.rparenToken = self.consumeToken()
        return n

    def parseBottomExp(self):
        cToken = self.currentToken.id
        if False:
            pass
        elif cToken == TK_IF:
            return self.parseIfExp()
        elif cToken == TK_LPAREN:
            return self.parseParenthesisExp()
        elif cToken in [TK_TODAY, TK_CLOSE]:
            return BuiltInExp(self.consumeToken(), None)
        elif cToken in [TK_BANG, TK_BANGLT, TK_BANGGT]:
            return self.parseForallExp()
        elif cToken in [TK_ISATOM, TK_ISTUP, TK_ISREL, 
                        TK_ISFUNC, TK_ISANY, TK_SYSTEM, 
                        TK_WRITE, TK_OPEN, TK_HAS, TK_MAX,
                        TK_MIN, TK_COUNT, TK_ADD, TK_MULT,
                        TK_DAYS, TK_BEFORE, TK_AFTER, TK_DATE,
                        TK_ISBOOL, TK_ISINT, TK_ISTEXT]:
            return self.parseBuiltIn()
        elif cToken == TK_PIPE:
            return LenExp(self.consumeToken(), self.parseExp(), self.assertTokenConsume(TK_PIPE))
        elif cToken == TK_MINUS:
            return UnaryOpExp(self.consumeToken(), self.parseExp())
        elif cToken == TK_NAME:
            return self.parseAssignment()
        elif cToken == TK_FUNC:
            return self.parseFuncExp()
        elif cToken == TK_REL:
            return self.parseRelExp()
        elif cToken == TK_TUP:
            return self.parseTupExp()
        elif cToken in [TK_ZERO, TK_ONE, TK_STDBOOL, TK_STDINT, TK_STDTEXT,
                        TK_TEXT, TK_INT, TK_TRUE,TK_FALSE]:
            return ConstantExp(self.consumeToken())
        elif cToken in [TK_SHARP]:
            return VariableExp(self.consumeToken())
        elif cToken == TK_BLOCKSTART:
            return self.parseBlockExp()
        elif cToken == TK_AT:
            return self.parseAtExp()
        elif cToken == TK_NOT:
            return UnaryOpExp(self.consumeToken(), self.parseExp())
        else:
            self.unexpectedToken()

    def parseSubstringOrFuncInvocationExp(self):
        n = self.parseBottomExp()
        if self.currentToken.id == TK_LPAREN:
            lparenToken = self.consumeToken()

            if self.currentToken.id == TK_RPAREN:
                n = FuncInvocationExp(n, lparenToken)
            else:
                with Recover(self, TK_RPAREN):
                    e1 = self.parseExp()
                    if self.currentToken.id == TK_TWO_DOTS:
                        n = SubstringExp(n, lparenToken, e1, self.consumeToken(), self.parseExp())
                    else:
                        n = FuncInvocationExp(n, lparenToken)
                        n.args.append(e1)
                        while self.currentToken.id == TK_COMMA:
                            self.consumeToken()
                            n.args.append(self.parseExp())
                        self.assertToken(TK_RPAREN)
            n.rparenToken = self.assertTokenConsume(TK_RPAREN)
        return n

    def parseRenameExp(self):
        n =self.parseSubstringOrFuncInvocationExp()
        while self.currentToken.id == TK_LBRACKET:
            n = RenameExp(n, self.consumeToken())
            with Recover(self, TK_RBRACKET):
                n.renames.append(RenameItem(self.assertTokenConsume(TK_NAME),
                                            self.assertTokenConsume(TK_LEFT_ARROW),
                                            self.assertTokenConsume(TK_NAME)))

                while self.currentToken.id == TK_COMMA:
                    self.consumeToken()
                    n.renames.append(RenameItem(self.assertTokenConsume(TK_NAME),
                                                self.assertTokenConsume(TK_LEFT_ARROW),
                                                self.assertTokenConsume(TK_NAME)))
                self.assertToken(TK_RBRACKET)
            n.rbracketToken = self.assertTokenConsume(TK_RBRACKET)
        return n

    def parseDotExp(self):
        n = self.parseRenameExp()
        if self.currentToken.id == TK_ONE_DOT:
            n = DotExp(n, self.consumeToken(), self.assertTokenConsume(TK_NAME))
        return n

    def parseOpExtendAndOverwriteExp(self):
        n = self.parseDotExp()
        while self.currentToken.id == TK_OPEXTEND:
            n = BinaryOpExp(self.consumeToken(), n, self.parseDotExp())
        return n

    def parseConcatExp(self):
        n = self.parseOpExtendAndOverwriteExp()
        while self.currentToken.id == TK_CONCAT:
            n = BinaryOpExp(self.consumeToken(), n, self.parseOpExtendAndOverwriteExp())
        return n

    def parseProjectionExp(self):
        n = self.parseConcatExp()
        while self.currentToken.id in [TK_PROJECT_PLUS, TK_PROJECT_MINUS]:
            n = ProjectExp(n, self.consumeToken())
            n.names.append(self.assertTokenConsume(TK_NAME))
            while self.currentToken.id == TK_COMMA:
                self.consumeToken()
                n.names.append(self.assertTokenConsume(TK_NAME))
        return n

    def parseMulDivModAndExp(self):
        n = self.parseProjectionExp()
        while self.currentToken.id in [TK_DIV, TK_MUL, TK_MOD, TK_AND]:
            n = BinaryOpExp(self.consumeToken(), n, self.parseProjectionExp())
        return n

    def parsePlusMinusOrExp(self):
        n = self.parseMulDivModAndExp()
        while self.currentToken.id in [TK_PLUS, TK_MINUS, TK_OR, TK_SET_MINUS]:
            n = BinaryOpExp(self.consumeToken(), n, self.parseMulDivModAndExp())
        return n

    def parseSelectExp(self):
        n = self.parsePlusMinusOrExp()
        while self.currentToken.id == TK_QUESTION:
            n = BinaryOpExp(self.consumeToken(), n, self.parsePlusMinusOrExp())
        return n

    def parseCompareExp(self):
        n = self.parseSelectExp()
        if self.currentToken.id in [TK_EQUAL, TK_DIFFERENT, 
                                    TK_LESS, TK_GREATER, 
                                    TK_LESSEQUAL, TK_GREATEREQUAL, TK_TILDE]:
            n = BinaryOpExp(self.consumeToken(), n, self.parseSelectExp())
        return n

    def parseSequenceExp(self):
        n=InvalidExp()
        with Recover(self, TK_SEMICOLON):
            n=self.parseCompareExp()
            if not self.currentToken.id in thingsThatMayComeAfterParseExp:
                self.unexpectedToken()
        if self.currentToken.id == TK_SEMICOLON:
            n2 = n
            n = SequenceExp()
            if n2: n.sequence.append(n2)
        while self.currentToken.id == TK_SEMICOLON:
            self.consumeToken()
            with Recover(self, TK_SEMICOLON):
                n.sequence.append(self.parseCompareExp())
                if not self.currentToken.id in thingsThatMayComeAfterParseExp:
                    self.unexpectedToken()
        return n

    def parseExp(self):
        return Exp(self.parseSequenceExp())
            
    def parse(self, doNotRecover = False):
        n = InvalidExp()
        with Recover(self, TK_EOF):
            n = self.parseExp()
        self.assertTokenConsume(TK_EOF)
        return n

    def unexpectedToken(self):
        if self.interactiveMode and self.currentToken.id == TK_EOF:
            raise IncompleteInputException()

        self.parseError("Unexpected token")
        self.recover()        

    def assertToken(self, TK):
        if (self.interactiveMode and
            self.currentToken.id == TK_EOF and 
            TK != TK_EOF):
            raise IncompleteInputException()
        
        if TK != self.currentToken.id:
            self.parseError("Expected %s at "%tokenNames[TK])
            self.recover()

    def assertTokenConsume(self, TK):
        token=self.currentToken
        self.assertToken(TK)
        self.consumeToken()
        return token


