import visitor
from lexer import *
from llvm.core import *
from llvm.passes import *
from firstParse import TBool, TInt
#RUN LIKE
#python RASMUS.py foo.rm | llvm-as | opt -O3 | llc | gcc -x assembler - && ./a.out; echo $?

class LLVMCodeGen(visitor.Visitor):
    def __init__(self, err, code):
        self.code = code
        self.err = err
        self.module = Module.new("Monkey")
        
        
        self.passMgr = FunctionPassManager.new(self.module)
        # Do simple "peephole" optimizations and bit-twiddling optzns.
        #self.passMgr.add(PASS_INSTRUCTION_COMBINING)
        # Reassociate expressions.
        #self.passMgr.add(PASS_REASSOCIATE)
        # Eliminate Common SubExpressions.
        #self.passMgr.add(PASS_GVN)
        # Simplify the control flow graph (deleting unreachable blocks, etc).
        #self.passMgr.add(PASS_CFG_SIMPLIFICATION)
        
        self.passMgr.initialize()

    def visitVariableExp(self, node):
        return node.store.value

    def visitAssignmentExp(self, node):
        node.value = self.visit(node.valueExp)
        return node.value

    def visitIfExp(self, node):
        done=False
        
        for choice in node.choices:
            choice.then_block = self.function.append_basic_block('then')
            choice.else_block = self.function.append_basic_block('else')

        b = self.builder.basic_block
        merge_block = self.function.append_basic_block('merge')
        self.builder.position_at_end(merge_block)
        phi = self.builder.phi(Type.int(64))

        self.builder.position_at_end(b)

        for choice in node.choices:
            cond = self.visit(choice.condition)
            if cond == Constant.int(Type.int(8), 0) or done:
                self.err.reportWarning("Branch never taken", choice.arrowToken)
            
            if cond == Constant.int(Type.int(8), 1):
                done = True

            self.builder.cbranch(cond, choice.then_block, choice.else_block)
            self.builder.position_at_end(choice.then_block)
            phi.add_incoming(self.visit(choice.value), choice.then_block)
            self.builder.branch(merge_block)
            self.builder.position_at_end(choice.else_block)
        
        self.builder.branch(merge_block)
        self.builder.position_at_end(merge_block)
        return phi

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
        if node.type == TInt:
            return Constant.int(Type.int(64), node.int_value)
        elif node.type == TBool:
            return Constant.int(Type.int(8), 1 if node.bool_value else 0)
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
        for n in node.sequence:
            x = self.visit(n)
        return x

    def generate(self, node):
        pass

    def visitOuter(self, AST):
        if AST.type == TBool:
            funct_type = Type.function(Type.int(8), [], False)
        else:
            funct_type = Type.function(Type.int(64), [], False)

        self.function = Function.new(self.module, funct_type, "BAR")
        self.block = self.function.append_basic_block('entry')
        self.builder = Builder.new(self.block)
        self.builder.ret(self.visit(AST))
        self.passMgr.run(self.function)
