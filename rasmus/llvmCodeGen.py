import visitor
from lexer import *
from llvm.core import *
from llvm.passes import *
#RUN LIKE
#python RASMUS.py foo.rm | llvm-as | opt -O3 | llc | gcc -x assembler - && ./a.out; echo $?

class LLVMCodeGen(visitor.Visitor):
    def __init__(self, code):
        self.code = code
        self.module = Module.new("Monkey")
        
        
        self.passMgr = FunctionPassManager.new(self.module)
        # Do simple "peephole" optimizations and bit-twiddling optzns.
        self.passMgr.add(PASS_INSTRUCTION_COMBINING)
        # Reassociate expressions.
        self.passMgr.add(PASS_REASSOCIATE)
        # Eliminate Common SubExpressions.
        self.passMgr.add(PASS_GVN)
        # Simplify the control flow graph (deleting unreachable blocks, etc).
        self.passMgr.add(PASS_CFG_SIMPLIFICATION)
        
        self.passMgr.initialize()

    def visitVariableExp(self, node):
        pass

    def visitAssignmentExp(self, node):
        pass

    def visitIfExp(self, node):
        pass

    def visitForallExp(self, node):
        pass

    def visitFuncExp(self, node):
        pass

    def visitTupExp(self, node):
        pass

    def visitBlockExp(self, node):
        pass

    def visitBuiltInExp(self, node):
        pass

    def visitConstantExp(self, node):
        if node.token.id == TK_INT:
            return Constant.int(Type.int(64), int(self.code.code[
                        node.token.start: node.token.length + node.token.start]))
        pass
    
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

    def visitRenameExp(self, node):
        pass
        
    def visitDotExp(self, node):
        pass

    def visitProjectExp(self, node):
        pass

    def visitExp(self, node):
        return self.visit(node.exp)

    def visitBinaryOpExp(self, node):
        a = self.visit(node.lhs)
        b = self.visit(node.rhs)
        if node.token.id == TK_PLUS:
            return self.builder.add(a, b)
        elif node.token.id == TK_MUL:
            return self.builder.mul(a,b)
    
    def visitSequenceExp(self, node):
        pass

    def generate(self, node):
        pass

    def visitOuter(self, AST):
        funct_type = Type.function(Type.int(64), [], False)
        self.function = Function.new(self.module, funct_type, "BAR")
        self.block = self.function.append_basic_block('entry')
        self.builder = Builder.new(self.block)
        self.builder.ret(self.visit(AST))
        self.passMgr.run(self.function)
