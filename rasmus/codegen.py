import visitor
from opcodes import *
from lexer import *
from firstParse import TBool,TInt,TInvalid,TText,TRel,TTup,TFunc,TAny,TAtom

class Codegen(visitor.Visitor):
    def __init__(self, codeStore):
        self.store = codeStore
        self.method = 0

    def emitUInt8(self, byte):
        self.store.methods[self.method] += chr(byte);

    def emitInt32(self, word):
        self.store.methods[self.method] += chr(word & 0xFF);
        word >>= 8
        self.store.methods[self.method] += chr(word & 0xFF);
        word >>= 8
        self.store.methods[self.method] += chr(word & 0xFF);
        word >>= 8
        self.store.methods[self.method] += chr(word & 0xFF);

    def emitPrint(self):
        self.emitUInt8(OP_PRINT)

    def emitHalt(self):
        self.emitUInt8(OP_HALT)

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
            self.emitUInt8(OP_INT_CONST)
            self.emitInt32(node.int_value)

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
        #TODO add type checking
        self.visit(node.lhs)
        self.visit(node.rhs)
        if node.token.id == TK_PLUS:
            self.emitUInt8(OP_ADD)
        elif node.token.id == TK_MINUS:
            self.emitUInt8(OP_MINUS)
        elif node.token.id == TK_MUL:
            self.emitUInt8(OP_MUL)
        elif node.token.id == TK_DIV:
            self.emitUInt8(OP_DIV)
        else:
            print "ICE"

    def visitSequenceExp(self, node):
        pass

    def visitInvalidExp(self, node):
        pass

    def visitExp(self, node):
        pass

