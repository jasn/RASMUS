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
TNAMEQ = Type("name?")

class Scope:
    def __init__(self, node, bind={}):
        self.node = node
        self.bind = bind

class FirstParse(visitor.Visitor):
    """ Do name lookup, type checking and constant propagation """
    def __init__(self, err, code):
        self.err = err
        self.code = code
        self.scopes = [ Scope(None,{}) ]

    # def setLus(self, lus):
    #     self.lus = []
    #     for l in lus:
    #         x = {}
    #         for a in l:
    #             x[a] = l[a]
    #         self.lus.append(x)

    # def getLus(self):
    #     return self.lus

    def tokenToIdentifier(self, token):
        return self.code.code[token.start:token.start+token.length]

    def internalError(self, token, message):
        self.err.reportError("Internal error: %s"%message, token)

        
    def typeCheck(self, token, expr, t):
        if expr.type in [TInvalid, TAny] or (expr.type in t) or (TAny in t):
            return True
        if len(t) > 1:
            msg="Expected one of %s but found %s"%(", ".join([x.name for x in t]), expr.type.name)
        else:
            msg="Expected type %s but found %s"%(t[0].name, expr.type.name)
        self.err.reportError(msg, token, [expr.charRange])
        return False
    
    def typeMatch(self, token, e1, e2, possibleTypes = [TAny]):
        leftOk = self.typeCheck(token, e1, possibleTypes)
        rightOk = self.typeCheck(token, e2, possibleTypes)

        if leftOk and rightOk:
            if e1.type == e2.type or e1.type == TAny or e2.type == TAny:
                return True
            self.err.reportError("Expected identical types but found %s and %s"%(e1.type.name, e2.type.name), 
                                 token, 
                                 [e1.charRange, e2.charRange])
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
        name = self.tokenToIdentifier(node.token)
        funcs = []
        for lu in reversed(self.scopes):
            if name in lu.bind:
                lookedUp = lu.bind[name]
                break
            if isinstance(lu.node, FuncExp):
                funcs.append(lu)

        if not lookedUp:
            node.store = None
            node.type = TInvalid
            self.err.reportError("Name not found in scope", node.token)
            return

        for lu in reversed(funcs):
            cap = FuncCaptureValue(lookedUp.nameToken)
            cap.name = name
            cap.type = lookedUp.type
            cap.store = lookedUp
            lu.bind[name] = cap
            lu.node.captures.append(cap)
            lookedUp = cap

        node.type = lookedUp.type  
        node.store = lookedUp

    def visitAssignmentExp(self, node):
        self.visit(node.valueExp)
        # Possibly check that the type was not changes since the last binding of the same name
        self.scopes[-1].bind[self.tokenToIdentifier(node.nameToken)] = node
        node.type = node.valueExp.type

    def visitIfExp(self, node):
        for choice in node.choices:
            self.visit(choice.condition)
            self.visit(choice.value)
        texp=None
        for choice in node.choices:
            self.typeCheck(choice.arrowToken, choice.condition, [TBool])
            if not texp and choice.value.type != TInvalid:
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
        self.scopes.append(Scope(node, {}))
        node.type = TFunc
        for a in node.args:
            a.name = self.tokenToIdentifier(a.nameToken)
            self.scopes[-1].bind[a.name] = a
            a.type = self.tokenToType(a.typeToken)
            
        node.rtype = self.tokenToType(node.returnTypeToken)
        self.visit(node.body)
        self.typeCheck(node.funcToken, node.body, [node.rtype])
        self.scopes.pop()

    def visitTupExp(self, node):
        for item in node.items:
            self.visit(item.exp)
        node.type = TTup

    def visitBlockExp(self, node):
        self.scopes.append(Scope(node,{}))
        for val in node.vals: 
            self.visit(val.exp)
            self.scopes[-1].bind[self.tokenToIdentifier(val.nameToken)] = val.exp
        self.visit(node.inExp)
        node.type = node.inExp.type
        self.scopes.pop()

    def visitBuiltInExp(self, node):
        returnType, argumentTypes = None, []
        tkn = node.nameToken.id
        
        if tkn in [TK_ISATOM, TK_ISTUP, TK_ISREL, TK_ISFUNC, TK_ISANY]:
            returnType = TBool
            argumentTypes.append(TAny)
        elif tkn in  [TK_ISBOOL, TK_ISINT, TK_ISTEXT]:
            returnType = TBool
            if len(node.args) < 2:
                argumentTypes = [TAny]
            else:
                argumentTypes = [TAny, TNAMEQ]
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
            argumentTypes = [TRel, TNAMEQ]
        elif tkn in [TK_MAX, TK_MIN, TK_COUNT, TK_ADD, TK_MULT]:
            returnType = TInt
            argumentTypes = [TRel, TNAMEQ]
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
        elif tkn == TK_PRINT:
            returnType = TAny
            argumentTypes = [TBool]
        else:
            self.err.reportError("Unknown buildin", None, [node.charRange])            

        node.type=returnType
        if len(node.args) < len(argumentTypes):
            # too few args
            self.err.reportError("Too few arguments to builtin function, received %d but expected %d"%(len(node.args), len(argumentTypes)), node.nameToken)
        elif len(node.args) > len(argumentTypes):
            self.err.reportError("Too many arguments to builtin function, received %d but expected %d"%(len(node.args), len(argumentTypes)), node.nameToken)
        
        for i in xrange(len(node.args)):
            if i >= len(argumentTypes) or argumentTypes[i] != TNAMEQ:
                self.visit(node.args[i])
            if i < len(argumentTypes) and argumentTypes[i] != TNAMEQ:
                self.typeCheck(node.nameToken, node.args[i], [argumentTypes[i]])
            if i < len(argumentTypes) and argumentTypes[i] == TNAMEQ:
                if (not isinstance(node.args[i], VariableExp) 
                    and not(isinstance(node.args[i], Exp) 
                            and isinstance(node.args[i].exp, VariableExp))):
                    self.err.reportError("Expected identifier", None, [node.args[i].charRange])

    def visitConstantExp(self, node):
        if node.token.id == TK_FALSE:
            node.bool_value = False
            node.type = TBool
        elif node.token.id == TK_TRUE:
            node.bool_value = True
            node.type = TBool
        elif node.token.id == TK_INT:
            node.int_value = int(self.code.code[node.token.start: node.token.length + node.token.start])
            node.type = TInt
        elif node.token.id == TK_TEXT:
            a = node.token.start+1
            b = node.token.length + node.token.start - 1
            if a < b:  #This in only here to make python happy
                node.txt_value = str(self.code.code[a:b])
            else:
                node.txt_value = ""
            node.type = TText
        elif node.token.id == TK_ZERO:
            node.type = TRel
            # dunno what to do here.
            # it needs to be the 'empty relation'
            pass
        elif node.token.id == TK_ONE:
            node.type = TRel
            # dunno what to do here.
            # it needs to be 
            # 'the relation with the empty Schema that has exactly one tuple which is empty'
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
        if node.token.id == TK_NOT:
            self.typeCheck(node.token, node.exp, [TBool])
            node.type = TBool
        elif node.token.id == TK_MINUS:
            self.typeCheck(node.token, node.exp, [TInt])
            node.type = TInt
        else:
            self.internalError(node.token, "Bad unary operator")
            node.type = TInvalid

    def visitRelExp(self, node):
        node.type = TRel
        self.visit(node.exp)
        self.typeCheck(node.relToken, node.exp, [TTup])

    def visitLenExp(self, node):
        self.visit(node.exp)
        self.typeCheck(node.leftPipeToken, node.exp, [TText, TRel])
        node.type = TInt

    def visitFuncInvocationExp(self, node):
        self.visit(node.funcExp)
        self.visitAll(node.args)
        node.type = TAny
        self.typeCheck(node.lparenToken, node.funcExp, [TFunc])

    def visitSubstringExp(self, node):
        self.visit(node.stringExp)
        self.visit(node.fromExp)
        self.visit(node.toExp)
        #TODO rpython seems to give this the "wrong" type
        #self.typeCheck(node.lparenToken. node.stringExp, [TText])
        self.typeCheck(node.lparenToken, node.fromExp, [TInt])
        self.typeCheck(node.lparenToken, node.toExp, [TInt])
        node.type = TText

    def visitRenameExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.lbracketToken, node.lhs, [TRel])
        node.type = TRel

    def visitDotExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.token, node.lhs, [TTup])
        node.type = TAny

    def visitProjectExp(self, node):
        self.visit(node.lhs)
        self.typeCheck(node.lhs.projectionToken, node.lhs, [TRel])
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
            self.typeCheck(node.token, node.lhs, [TInt])
            self.typeCheck(node.token, node.rhs, [TInt])
            node.type = TInt
        elif node.token.id in [TK_AND, TK_OR]:
            self.typeCheck(node.token, node.lhs, [TBool])
            self.typeCheck(node.token, node.rhs, [TBool])
            node.type = TBool
        elif node.token.id == TK_CONCAT:
            self.typeCheck(node.token, node.lhs, [TText])
            self.typeCheck(node.token, node.rhs, [TText])
            node.type = TText
        elif node.token.id in [TK_LESSEQUAL, TK_LESS, TK_GREATER, TK_GREATEREQUAL]:
            self.typeCheck(node.token, node.lhs, [TInt])
            self.typeCheck(node.token, node.rhs, [TInt])
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
        self.scopes.append(Scope(node, {}))
        self.visit(node.exp)
        self.scopes.pop()
        node.type = node.exp.type
        
