import visitor
from lexer import *


class Evaluator(visitor.Visitor):
    def __init__(self):
        pass
      
    def visitVariableExp(self, node):
        return node.store.val

    def visitAssignmentExp(self, node):
        node.val = self.visit(node.valueExp)
        return node.val

    def visitChoice(self, node):
        pass

    def visitIfExp(self, node):
        for choice in node.choices:
            if self.visit(choice.condition):
                return self.visit(choice.value)
        return None

    def visitForallExp(self, node):
        pass
    def visitFuncArg(self, node):
        pass
    def visitFuncExp(self, node):
        return node
        pass
    def visitTupItem(self, node):
        pass
    def visitTupExp(self, node):
        pass

    def visitVal(self, node):
        node.val = self.visit(node.exp)

    def visitBlockExp(self, node):
        for val in node.vals:
            self.visit(val)
        return self.visit(node.inExp)

    def visitBuiltInExp(self, node):
        pass

    def visitAtExp(self, node):
        pass

    def visitConstantExp(self, node):
        return node.cvalue 

    def visitUnaryOpExp(self, node):
        if node.token.id == TK_MINUS:
            return - self.visit(node.exp)
        if node.token.id == TK_NOT:
            return not self.visit(node.exp)

    def visitRelExp(self, node):
        pass
    def visitLenExp(self, node):
        return len(self.visit(node.exp))

    def visitFuncInvocationExp(self, node):
        func = self.visit(node.funcExp)
        oldValues = [arg.val for arg in func.args]
        for i in range(len(node.args)):
            func.args[i].val = self.visit(node.args[i])
        res = self.visit(func.body)
        for i in range(len(node.args)):
            func.args[i].val = oldValues[i]
        return res
    
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
        if node.token.id == TK_PLUS:
            return self.visit(node.lhs) + self.visit(node.rhs)
        if node.token.id == TK_MINUS:
            return self.visit(node.lhs) - self.visit(node.rhs)
        if node.token.id == TK_MUL:
            return self.visit(node.lhs) * self.visit(node.rhs)
        if node.token.id == TK_DIV:
            return self.visit(node.lhs) / self.visit(node.rhs)
        if node.token.id == TK_LESSEQUAL:
            return self.visit(node.lhs) <= self.visit(node.rhs)

    def visitSequenceExp(self, node):
        self.visitAll(node.sequence[0:-1])
        return self.visit(node.sequence[-1])
    def visitInvalidExp(self, node):
        pass
    def visitExp(self, node):
        return self.visit(node.exp)
