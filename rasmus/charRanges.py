import visitor
from collections import namedtuple

CharRange=namedtuple("CharRange", ['lo','hi'])

def r(token):
    return CharRange(token.start, token.start+token.length)

def u(r1, r2):
    return CharRange(min(r1.lo, r2.lo), max(r1.hi, r2.hi))

class CharRanges(visitor.Visitor):
    def visitVariableExp(self, node):
        node.charRange = r(node.token)

    def visitAssignmentExp(self, node):
        self.visit(node.valueExp)
        node.charRange = u(r(node.nameToken), node.valueExp.charRange)

    def visitIfExp(self, node):
        for choice in node.choices:
            self.visit(choice.condition)
            self.visit(choice.value)
        node.charRange = u(r(node.ifToken), r(node.fiToken))

    def visitForallExp(self, node):
        self.visitAll(node.listExps)
        self.visit(node.expr)
        node.charRange = u(r(node.typeToken), node.expr.charRange)

    def visitFuncExp(self, node):
        self.visit(node.body)
        node.charRange = u(r(node.funcToken), r(node.endToken))

    def visitTupExp(self, node):
        for item in node.items: self.visit(item.exp)
        node.charRange = u(r(node.tupToken), r(node.rparenToken))

    def visitBlockExp(self, node):
        for val in node.vals: self.visit(val.exp)
        self.visit(node.inExp)
        node.charRange = u(r(node.blockstartToken), r(node.blockendToken))

    def visitBuiltInExp(self, node):
        self.visitAll(node.args)
        node.charRange = u(r(node.nameToken), r(node.rparenToken))

    def visitConstantExp(self, node):
        node.charRange = r(node.token)

    def visitUnaryOpExp(self, node):
        self.visit(node.exp)
        node.charRange = u(r(node.token), node.exp.charRange)

    def visitRelExp(self, node):
        self.visit(node.exp)
        node.charRange = u(r(node.relToken), r(node.rparenToken))

    def visitLenExp(self, node):
        self.visit(node.exp)
        node.charRange = u(r(node.leftPipeToken), r(node.rightPipeToken))

    def visitFuncInvocationExp(self, node):
        self.visit(node.funcExp)
        self.visitAll(node.args)
        node.charRange = u(node.funcExp.charRange, r(node.rparenToken))

    def visitSubstringExp(self, node):
        self.visitAll([node.stringExp, node.fromExp, node.toExp])
        node.charRange = u(node.stringExp.charRange, r(node.rparenToken))

    def visitRenameExp(self, node):
        self.visit(node.lhs)
        node.charRange = u(node.lhs.charRange, r(node.rbracketToken))
        
    def visitDotExp(self, node):
        self.visit(node.lhs)
        node.charRange = u(node.lhs.charRange, r(node.nameToken))

    def visitProjectExp(self, node):
        self.visit(node.lhs)
        node.charRange = u(node.lhs.charRange, r(node.names[-1]))

    def visitBinaryOpExp(self, node):
        self.visit(node.lhs)
        self.visit(node.rhs)
        node.charRange = u(node.lhs.charRange, node.rhs.charRange)

    def visitSequenceExp(self, node):
        self.visitAll(node.sequence)
        if node.sequence:
            node.charRange = u(node.sequence[0].charRange, node.sequence[-1].charRange)

    def visitInvalidExp(self, node):
        pass

    def visitExp(self, node):
        self.visit(node.exp)
        node.charRange = node.exp.charRange
