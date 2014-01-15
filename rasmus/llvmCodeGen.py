import visitor
from lexer import *
from llvm.core import *
from llvm.passes import *
from firstParse import TBool, TInt
#RUN LIKE
#python RASMUS.py foo.rm | llvm-as | opt -O3 | llc | gcc -x assembler - && ./a.out; echo $?

def llvmType(t):
    if t == TBool:
        return Type.int(8)
    elif t == TInt:
        return Type.int(64)

def typeRepr(t):
    if t == TBool: return Constant.int(Type.int(8), 0)
    if t == TInt: return Constant.int(Type.int(8), 0)

def intp(v):
    return Constant.int(Type.int(32), v)

class LLVMCodeGen(visitor.Visitor):
    def __init__(self, err, code):
        self.code = code
        self.err = err
        self.module = Module.new("Monkey")
        self.fid = 0
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
        # Create function type
        funct_type = Type.function(llvmType(node.rtype), 
                                   map(llvmType, [arg.type for arg in node.args]), False)
        

        # Cache current state
        f = self.function
        b = self.block
        bb = self.builder.basic_block
        
        # Name new function and block
        name = "f%d"%self.fid
        self.fid += 1
        self.function = Function.new(self.module, funct_type, name)
        self.block = self.function.append_basic_block('entry')
        self.builder.position_at_end(self.block)

        func = self.function

        # Setup args
        for farg, arg in zip(self.function.args, node.args):
            arg.value = farg
            farg.name = arg.name

        # Build function code
        self.builder.ret(self.visit(node.body))

        # Revert state
        self.function = f
        self.block = b
        self.builder.position_at_end(bb)

        # Create function object type
        t = Type.struct( [
                Type.pointer(funct_type), #Function ptr
                Type.int(16), # Number of arguments
                ] + [Type.int(8)] * (len(node.args) + 1))
        # Todo add closure arguments

        # Allocate function object
        p = self.builder.malloc(t)
        self.builder.store(func, 
                           self.builder.gep(p, [intp(0), intp(0)]))
        self.builder.store(Constant.int(Type.int(16), len(node.args)), 
                           self.builder.gep(p, [intp(0), intp(1)]))
        self.builder.store(typeRepr(node.rtype), 
                           self.builder.gep(p, [intp(0), intp(2)]))
        i=3
        for arg in node.args:
            self.builder.store(typeRepr(arg.type), 
                               self.builder.gep(p, [intp(0), intp(i)]))
            i += 1

        # TODO store closure arguments
        return p

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
