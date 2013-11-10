import visitor
from opcodes import *
from lexer import *
from firstParse import TBool,TInt,TInvalid,TText,TRel,TTup,TFunc,TAny,TAtom

class Codegen(visitor.Visitor):
    def __init__(self):
        self.methods = [""]
        self.method = 0

    def emit(self, val):
        self.methods[self.method] += chr(val)

    def visitVariableExp(self, node):
        pass

    def visitAssignmentExp(self, node):
        pass

    def visitChoice(self, node):
        pass

    def visitIfExp(self, node):
        pass

    def visitForallExp(self, node):
        pass

    def visitFuncArg(self, node):
        pass

    def visitFuncExp(self, node):
        pass

    def visitTupItem(self, node):
        pass

    def visitTupExp(self, node):
        pass

    def visitVal(self, node):
        pass

    def visitBlockExp(self, node):
        pass

    def visitBuiltInExp(self, node):
        pass

    def visitAtExp(self, node):
        pass

    def visitConstantExp(self, node):
        if node.type == TInt:
            self.emit(OP_INT_CONST)
            v = node.int_value
            self.emit(v & 0xFF)
            v >>= 8
            self.emit(v & 0xFF)
            v >>= 8
            self.emit(v & 0xFF)
            v >>= 8
            self.emit(v & 0xFF)
            v >>= 8

    def visitUnaryOpExp(self, node):
        pass

    def visitRelExp(self, node):
        pass

    def visitLenExp(self, node):
        pass

    def visitFuncInvocationExp(self, node):
        pass
    
    def visitSubstringExp(self, node):
        pass

    def visitRenameItem(self, node):
        pass

    def visitRenameExp(self, node):
        pass

    def visitDotExp(self, node):
        pass

    def visitProjectExp(self, node):
        pass

    def visitBinaryOpExp(self, node):
        pass

    def visitSequenceExp(self, node):
        pass

    def visitInvalidExp(self, node):
        pass

    def visitExp(self, node):
        pass

