import visitor
from copy import copy
from AST import *
from lexer import *

class Type:
    def __init__(self, name):
        self.name = name
        
    def __str__(self):
        return self.name

TBool = Type("bool")
TInt = Type("int")
TInvalid = Type("invalid")
TText = Type("text")
TRel = Type("relation")
TTup = Type("tuple")
TFunc = Type("function")
TAny = Type("any")
TAtom = Type("atom")

class FirstParse(visitor.Visitor):
    """ Do name lookup, type checking and constant propagation """
    def __init__(self, err, code):
        self.err = err
        self.code = code
        self.lus = []

    def setCode(self, code):
        self.code = code

    def setLus(self, lus):
        self.lus = map(copy,lus)

    def getLus(self):
        return self.lus

    def tokenToIdentifier(self, token):
        _, i, j = token
        return self.code[i:i+j]

    def internalError(self, token, message):
        self.err.reportError("Internal error: %s"%message, token)

        
    def typeCheck(self, token, expr, t):
        if expr.type == TInvalid or expr.type == t or (isinstance(t, list) and expr.type in t) or expr.type == TAny or t == TAny:
            return True
        if isinstance(t, list):
            self.err.reportError("Expected one of %s but found %s"%(", ".join(map(str,t)), expr.type), token, expr.charRange)
        else:
            self.err.reportError("Expected type %s but found %s"%(str(t), expr.type), token, expr.charRange)
        return False
    
    def typeMatch(self, token, e1, e2, possibleTypes = TAny):
        
        leftOk = self.typeCheck(token, e1, possibleTypes)
        rightOk = self.typeCheck(token, e2, possibleTypes)

        if leftOk and rightOk:
            if e1.type == e2.type or e1.type == TAny or e2.type == TAny:
                return True
            self.err.reportError("Expected identical types but found %s and %s"%(e1.type, e2.type), 
                                 token, 
                                 e1.charRange, e2.charRange)
            return False

        return False

    def tokenToType(self, token):
        if token.id == TK_TYPE_ANY:
            return TAny
        elif token.id == TK_TYPE_ATOM:
            return TAtom
        elif token.id == TK_TYPE_BOOL:
            return TBool
        elif token.id == TK_TYPE_FUNC:
            return TFunc
        elif token.id == TK_TYPE_INT:
            return TInt
        elif token.id == TK_TYPE_REL:
            return TRel
        elif token.id == TK_TYPE_TEXT:
            return TText
        elif token.id == TK_TYPE_TUP:
            return TTup

    def visitVariableExp(self, node):
        lookedUp = None
        for lu in reversed(self.lus):
            name = self.tokenToIdentifier(node.token)
            if name in lu:
                lookedUp = lu[name]
                break
        if lookedUp == None:
            node.type = TInvalid
            self.err.reportError("Name not found in scope", node.token)
        else:
            node.type = lookedUp.type  
        node.store = lookedUp

    def visitAssignmentExp(self, node):
        self.visit(node.valueExp)
        # Possibly check that the type was not changes since the last binding of the same name
        self.lus[-1][self.tokenToIdentifier(node.nameToken)] = node
        node.type = node.valueExp.type

    def visitIfExp(self, node):
        for choice in node.choices:
            self.visit(choice.condition)
            self.visit(choice.value)
        texp=None
        for choice in node.choices:
            self.typeCheck(choice.arrowToken, choice.condition, TBool)
            if texp == None and choice.value.type != TInvalid:
                texp = choice.value
        node.type = TInvalid
        if texp:
            good=True
            for choice in node.choices:
                if not self.typeMatch(choice.arrowToken, texp, choice.value):
                    good=False
            if good:
                node.type = texp.type

    def visitForallExp(self, node):
        #TODO
        self.visitAll(node.listExps)
        self.visit(node.expr)

    def visitFuncExp(self, node):
        self.lus.append({})
        node.type = TFunc
        for a in node.args:
            self.lus[-1][self.tokenToIdentifier(a.nameToken)] = a
            a.type = self.tokenToType(a.typeToken)
        self.visit(node.body)
        self.typeCheck(node.funcToken, node.body, self.tokenToType(node.returnTypeToken))
        self.lus.pop()

    def visitTupExp(self, node):
        for item in node.items:
            self.visit(item.exp)
        node.type = TTup

    def visitBlockExp(self, node):
        self.lus.append({})
        for val in node.vals: 
            self.visit(val.exp)
            self.lus[-1][self.tokenToIdentifier(val.nameToken)] = val.exp
        self.visit(node.inExp)
        node.type = node.inExp.type
        self.lus.pop()

    def visitBuiltInExp(self, node):
        returnType, argumentTypes = None, []
        tkn = node.nameToken.id
        
        if tkn in [TK_ISATOM, TK_ISTUP, TK_ISREL, TK_ISFUNC, TK_ISANY]:
            returnType = TBool
            argumentTypes.append(TAny)
        elif tkn in  [TK_ISBOOL, TK_ISINT, TK_ISTEXT]:
            returnType = TBool
            if len(nodes.args) < 2:
                argumentTypes = [TAny]
            else:
                argumentTypes = [TAny, "NAME?"]
        elif tkn == TK_SYSTEM:
            returnType = TInt
            argumentTypes = [TText]
        elif tkn in [TK_OPEN, TK_WRITE]:
            returnType = TBool
            argumentTypes = [TText]
        elif tkn == TK_CLOSE:
            returnType = TBool
        elif tkn == TK_HAS:
            returnType = TBool
            argumentTypes = [TRel,  "NAME?"]
        elif tkn in [TK_MAX, TK_MIN, TK_COUNT, TK_ADD, TK_MULT]:
            returnType = TInt
            argumentTypes = [TRel,  "NAME?"]
        elif tkn == TK_DAYS:
            returnType = TInt
            argumentTypes = [TText, TText]
        elif tkn in [TK_BEFORE, TK_AFTER]:
            returnType = TText
            argumentTypes = [TText, TText]
        elif tkn == TK_DATE:
            returnType = TText
            argumentTypes = [TText, TInt]
        elif tkn == TK_TODAY:
            returnType = TText

        if len(node.args) < len(argumentTypes):
            # too few args
            self.err.reportError("Too few arguments to builtin function, received %d but expected %d"%(len(node.args), len(argumentTypes)), node.nameToken)
        elif len(node.args) > len(argumentTypes):
            self.err.reportError("Too many arguments to builtin function, received %d but expected %d"%(len(node.args), len(argumentTypes)), node.nameToken)
        
        for i in xrange(len(node.args)):
            if i >= len(argumentTypes) or argumentTypes[i] != "NAME?":
                self.visit(node.args[i])
            if i < len(argumentTypes) and argumentTypes[i] != "NAME?":
                self.typeCheck(node.nameToken, node.args[i], argumentTypes[i])
            if i < len(argumentTypes) and argumentTypes[i] == "NAME?":
                if (not isinstance(node.args[i], VariableExp) 
                    and not(isinstance(node.args[i], Exp) 
                            and isinstance(node.args[i].exp, VariableExp))):
                    self.err.reportError("Expected identifier", None, node.args[i].charRange)

    def visitConstantExp(self, node):
        if node.token.id == TK_FALSE:
            node.cvalue = False
            node.type = TBool
        elif node.token.id == TK_TRUE:
            node.cvalue = True
            node.type = TBool
        elif node.token.id == TK_INT:
            node.cvalue = int(self.code[node.token.start: node.token.length + node.token.start])
            node.type = TInt
        elif node.token.id == TK_TEXT:
            node.cvalue = str(self.code[node.token.start+1: node.token.length + node.token.start - 1])
            node.type = TText
        elif node.token.id == TK_ZERO:
            node.type = TRel
            # dunno what to do here.
            # it needs to be the 'empty relation'
            node.cvalue = None
            pass
        elif node.token.id == TK_ONE:
            node.type = TRel
            # dunno what to do here.
            # it needs to be 
            # 'the relation with the empty Schema that has exactly one tuple which is empty'
            node.cvalue = None            
            pass
        elif node.token.id == TK_STDBOOL:
            node.type = TBool
        elif node.token.id == TK_STDINT:
            node.type = TInt
        elif node.token.id == TK_STDTEXT:
            node.type = TText
        else:
            self.internalError(node.token, "Invalid constant type (%s)"%(tokenNames[node.token.id]))
            node.type = TInvalid
            

    def visitUnaryOpExp(self, node):
        self.visit(node.exp)
        if node.token == TK_NOT:
            self.typeCheck(node.token, node.exp, TBool)
            node.type = TBool
        elif node.token == TK_MINUS:
            self.typeCheck(node.token, node.exp, TInt)
            node.type = TInt
        else:
            self.internalError(node.token, "Bad unary operator")
            node.type = TInvalid

    def visitRelExp(self, node):
        node.type = TRel
        self.visit(node.exp)
        self.typeCheck(node.relToken, node.exp, TTup)

    def visitLenExp(self, node):
        self.visit(node.exp)
        self.typeCheck(node.leftPipeToken, node.exp, [TText, TRel])
        node.type = TInt

    def visitFuncInvocationExp(self, node):
        self.visit(node.funcExp)
        self.visitAll(node.args)
        node.type = TAny
        self.typeCheck(node.lparenToken, node.funcExp, TFunc)

    def visitSubstringExp(self, node):
        self.visit(node.stringExp)
        self.visit(node.fromExp)
        self.visit(node.toExp)
        self.typeCheck(node.lparenToken, node.fromExp, TInt)
        self.typeCheck(node.lparenToken, node.toExp, TInt)
        self.typeCheck(node.lparenToken. node.stringExp, TText)
        node.type = TText

    def visitRenameExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.lbracketToken, node.lhs, TRel)
        node.type = TRel

    def visitDotExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.token, node.lhs, TTup)
        node.type = TAny

    def visitProjectExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.lhs.projectionToken, node.lhs, TRel)
        node.type = TRel

    def visitInvalidExp(self, node):
        node.type = TInvalid
        
    def visitBinaryOpExp(self, node):
        self.visit(node.lhs)
        self.visit(node.rhs)
        if node.token.id in [TK_PLUS, TK_MUL, TK_MINUS]:
            self.typeMatch(node.token, node.lhs, node.rhs, [TRel, TInt])
            if node.lhs.type == TInt and node.rhs.type != TRel:
                node.type = TInt
            elif node.lhs.type != TRel and node.rhs.type == TInt:
                node.type = TInt
            elif node.lhs.type == TRel and node.rhs.type != TInt:
                node.type = TRel
            elif node.lhs.type != TInt and node.rhs.type == TRel:
                node.type = TRel
            elif node.lhs.type == TAny and node.rhs.type == TAny:
                node.type = TAny
        elif node.token.id in [TK_DIV, TK_MOD]:
            self.typeCheck(node.token, node.lhs, TInt)
            self.typeCheck(node.token, node.rhs, TInt)
            node.type = TInt
        elif node.token.id in [TK_AND, TK_OR]:
            self.typeCheck(node.token, node.lhs, TBool)
            self.typeCheck(node.token, node.rhs, TBool)
            node.type = TBool
        elif node.token.id == TK_CONCAT:
            self.typeCheck(node.token, node.lhs, TText)
            self.typeCheck(node.token, node.rhs, TText)
            node.type = TText
        elif node.token.id in [TK_LESSEQUAL, TK_LESS, TK_GREATER, TK_GREATEREQUAL]:
            self.typeCheck(node.token, node.lhs, TInt)
            self.typeCheck(node.token, node.rhs, TInt)
            node.type = TBool
        else:
            self.internalError(node.token, "Invalid operator (%s)"%(tokenNames[node.token.id]))
            node.type = TInvalid

    def visitSequenceExp(self, node):
        self.visitAll(node.sequence)
        if len(node.sequence) == 0:
            node.type = TInvalid
        else:
            node.type = node.sequence[-1].type
    
    def visitExp(self, node):
        self.lus.append({})
        self.visit(node.exp)
        self.lus.pop()
        node.type = node.exp.type
        
