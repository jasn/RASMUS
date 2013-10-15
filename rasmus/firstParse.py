import visitor
from lexer import *

class Type:
    def __init__(self, name):
        self.name = name
        
    def __str__(self):
        return self.name

TBool = Type("bool")
TString = Type("string")
TInt = Type("int")
TInvalid = Type("invalid")


class FirstParse(visitor.Visitor):
    """ Do name lookup, type checking and constant propagation """
    def __init__(self, err, code):
        self.err = err
        self.code = code

    def internalError(self, token, message):
        self.err.reportError("Internal error: %s"%message, token)

        
    def typeCheck(self, token, expr, t):
        if expr.type == TInvalid or expr.type == t: 
            return True
        self.err.reportError("Expected type %s but found %s"%(t, expr.type), token, expr.charRange)
        return False
    
    def typeMatch(self, token, e1, e2):
        if e1.type == TInvalid or e2.type == TInvalid or e1.type == e2.type:
            return True
        self.err.reportError("Expected identical types but found %s and %s"%(e1.type, e2.type), 
                             token, 
                             e1.charRange, e2.charRange)
        return False

    def visitVariableExp(self, node):
        # Do name lookup, 
        # Copy type info
        # Propagate constant value
        pass
    
    def visitAssignmentExp(self, node):
        self.visit(node.valueExp)
        # Bind to name in scope
        # Possibly check that the type was not changes since the last binding of the same name

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
        #TODO
        self.visit(node.body)

    def visitTupExp(self, node):
        #TODO
        for item in node.items:
            self.visit(item.exp)

    def visitBlockExp(self, node):
        #TODO
        for val in node.vals: 
            self.visit(val.exp)
        self.visit(node.inExp)

    def visitBuiltInExp(self, node):
        #TODO
        self.visitAll(node.args)

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
        else:
            self.internalError(node.token, "Invalid constant type (%s)"%(tokenNames[node.token.id]))
            node.type = TInvalid
        #TK_ZERO, TK_ONE, TK_STDBOOL, TK_STDINT, TK_STDTEXT, TK_TEXT:
        pass

    def visitUnaryOpExp(self, node):
        self.visit(node.exp)
        if node.token == TK_NOT:
            self.typeCheck(node.token, node.exp, TBool)
            self.type = TBool
        elif node.token == TK_MINUS:
            self.typeCheck(node.token, node.exp, TInt)
            self.type = TInt
        else:
            self.internalError(node.token, "Bad unary operator")
            self.type = TInvalid

    def visitRelExp(self, node):
        #TODO
        self.visit(node.exp)

    def visitLenExp(self, node):
        self.visit(node.exp)
        self.typeCheck(node.leftPipeToken, node.exp, TString)
        self.type = TInt

    def visitFuncInvocationExp(self, node):
        self.visit(node.funcExp)
        self.visitAll(node.args)

    def visitSubstringExp(self, node):
        self.visit(node.stringExp)
        self.visit(node.fromExp)
        self.visit(node.toExp)
        self.typeCheck(node.lparenToken, node.fromExp, TInt)
        self.typeCheck(node.lparenToken, node.toExp, TInt)
        self.typeCheck(node.lparenToken. node.stringExp, TString)
        self.type = TString

    def visitRenameExp(self, node):
        #TODO
        self.visit(node.lhs)
        
    def visitDotExp(self, node):
        #TODO
        self.visit(node.lhs)

    def visitProjectExp(self, node):
        #TODO
        self.visit(node.lhs)

    def visitInvalidExp(self, node):
        self.type = TInvalid
        
    def visitBinaryOpExp(self, node):
        self.visit(node.lhs)
        self.visit(node.rhs)
        if node.token.id in [TK_PLUS, TK_DIV, TK_MUL, TK_MINUS, TK_MOD]:
            self.typeCheck(node.token, node.lhs, TInt)
            self.typeCheck(node.token, node.rhs, TInt)
            node.type = TInt
        elif node.token.id in [TK_AND, TK_OR]:
            self.typeCheck(node.token, node.lhs, TBool)
            self.typeCheck(node.token, node.rhs, TBool)
            node.type = TBool
        elif node.token.id == TK_CONCAT:
            self.typeCheck(node.token, node.lhs, TString)
            self.typeCheck(node.token, node.rhs, TString)
            node.type = TString
        else:
            self.internalError(node.token, "Invalid operator (%s)"%(tokenNames[node.token.id]))
            node.type = TInvalid

    def visitSequenceExp(self, node):
        self.visitAll(node.sequence)
        if len(node.sequence):
            self.type = TInvalid
        else:
            self.type = node.sequence[-1].type
    

