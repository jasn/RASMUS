import visitor
from lexer import *
from llvm.core import *
from llvm.passes import *
from firstParse import TBool, TInt, TAny, TText, TFunc

anyType = Type.struct([Type.int(8), Type.int(64)])
funcBase = Type.struct([Type.pointer(Type.void()), Type.int(16)])

def llvmType(t):
    if t == TBool:
        return Type.int(8)
    elif t == TInt:
        return Type.int(64)
    elif t == TAny:
        return anyType

def typeRepr(t):
    if t == TBool: return Constant.int(Type.int(8), 0)
    if t == TInt: return Constant.int(Type.int(8), 1)
    if t == TAny: return Constant.int(Type.int(8), 2)
        
def intp(v):
    return Constant.int(Type.int(32), v)

def funcType(argc):
    return Type.function(anyType, [Type.pointer(funcBase)] +
                         [Type.int(8), Type.int(64)] * argc, False)


class LLVMCodeGen(visitor.Visitor):
    def cast(self, value, tfrom, tto):
        if tfrom == tto: 
            return value
        if tto == TAny:
            a = self.builder.alloca(anyType)
            self.builder.store(typeRepr(tto), self.builder.gep(a, [intp(0), intp(0)]))
            self.builder.store(value, self.builder.gep(a, [intp(0), intp(1)]))
            return a
        if tfrom == TAny:
            #TODO throw error
            return self.builder.bitcast(self.builder.extract_value(value, 1), llvmType(tto))

    def __init__(self, err, code):
        self.code = code
        self.err = err
        self.module = Module.new("Monkey")
        self.fid = 0
        self.passMgr = FunctionPassManager.new(self.module)
        # Do simple "peephole" optimizations and bit-twiddling optzns.
        self.passMgr.add(PASS_INSTCOMBINE)
        # Reassociate expressions.
        self.passMgr.add(PASS_REASSOCIATE)
        # Eliminate Common SubExpressions.
        self.passMgr.add(PASS_GVN)
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
        hats = []
        for choice in node.choices:
            cond = self.cast(self.visit(choice.condition), choice.condition.type, TBool)
            if cond == Constant.int(Type.int(8), 0) or done:
                self.err.reportWarning("Branch never taken", choice.arrowToken)
                continue
            
            if cond == Constant.int(Type.int(8), 1):
                done = True

            value = self.cast(self.visit(choice.value), choice.value.type, node.type)
            hats.append((cond, value))
            
        if not done:
            val = Constant.int(Type.int(64), 0) #TODO get undefined value for type
        else:
            _, val = hats.pop()
            
        while hats:
            cond, v = hats.pop()
            val = self.builder.select(cond, v , val)
        print val

    def visitForallExp(self, node):
        pass

    def visitFuncExp(self, node):
        # Create function type
        funct_type = funcType(len(node.args))

        # Create function object type
        t = Type.struct( [
                Type.pointer(funct_type), #Function ptr
                Type.int(16), # Number of arguments
                ] + [llvmType(cap.type) for cap in node.captures])

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
        self.function.args[0].name = "self"
        selfv = self.builder.bitcast(self.function.args[0], Type.pointer(t))
        
        for i in range(len(node.args)):
            arg = node.args[i]
            #Todo check types
            if arg.type == TInt:
                arg.value = self.builder.bitcast(
                    self.function.args[i*2+2], Type.int(64))
            elif arg.type == TBool:
                arg.value = self.builder.bitcast(
                    self.function.args[i*2+2], Type.int(8))
            self.function.args[i*2+1].name = "t_"+arg.name
            self.function.args[i*2+2].name = "v_"+arg.name

        for i in range(len(node.captures)):
            cap = node.captures[i]
            cap.value = self.builder.load(self.builder.gep(selfv, [intp(0), intp(2+i)]))
                    
        # Build function code
        self.builder.ret(
            self.cast(
                self.visit(node.body), node.type, TAny) )

        # Revert state
        self.function = f
        self.block = b
        self.builder.position_at_end(bb)
        
        # Allocate function object
        p = self.builder.malloc(t)
        # Store function ptr
        self.builder.store(func, 
                           self.builder.gep(p, [intp(0), intp(0)]))
        # Store number of arguments
        self.builder.store(Constant.int(Type.int(16), len(node.args)), 
                           self.builder.gep(p, [intp(0), intp(1)]))
        # Store captures
        for i in range(len(node.captures)):
            cap = node.captures[i]
            self.builder.store(cap.store.value, self.builder.gep(p, [intp(0), intp(2+i)]))
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
        ft = funcType(len(node.args))
        
        capture=self.builder.bitcast(self.cast(self.visit(node.funcExp), node.funcExp.type, TFunc), 
                                      Type.pointer(funcBase))
        #TODO check argument count
        args = [capture]
        for arg in node.args:
            argv = self.visit(arg)
            if arg.type == TAny:
                args.append(self.builder.extract_value(argv, 0))
                args.append(self.builder.extract_value(argv, 1))
            else:
                args.append(typeRepr(arg.type))
                args.append(self.builder.bitcast(self.visit(arg), Type.int(64)))

        voidfp = self.builder.load(self.builder.gep(capture, [intp(0), intp(0)]))
        fp = self.builder.bitcast(voidfp, Type.pointer(ft))
        return self.builder.call(fp, args)

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
        a = self.cast(self.visit(node.lhs), node.lhs.type, TInt)
        b = self.cast(self.visit(node.rhs), node.rhs.type, TInt)
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
        elif AST.type == TInt:
            funct_type = Type.function(Type.int(64), [], False)
        else:
            funct_type = Type.function(anyType, [], False)

        self.function = Function.new(self.module, funct_type, "BAR")
        self.block = self.function.append_basic_block('entry')
        self.builder = Builder.new(self.block)
        self.builder.ret(self.visit(AST))

        self.passMgr.run(self.function)
