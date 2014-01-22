import visitor
from lexer import *
from llvm.core import *
from llvm.passes import *
from firstParse import TBool, TInt, TAny, TText, TFunc

class ICEException(Exception):
    def __init__(self, message): 
        self.message = message


anyRetType = Type.struct([Type.int(8), Type.int(64)])
funcBase = Type.struct([Type.pointer(Type.void()), Type.int(16)])

def llvmType(t):
    if t == TBool: return Type.int(8)
    elif t == TInt: return Type.int(64)
    elif t == TAny: return anyType
    elif t == TFunc: return Type.pointer(funcBase)
    raise ICEException("Unhandled type %s"%str(t))

def typeRepr(t):
    if t == TBool: return Constant.int(Type.int(8), 0)
    elif t == TInt: return Constant.int(Type.int(8), 1)
    elif t == TAny: raise ICEException("Any does not have a repr")
    elif t == TFunc: return Constant.int(Type.int(8), 2)
    raise ICEException("Unhandled type %s"%str(t))

def genUndef(self, t):
    if t == TBool: return Constant.int(Type.int(8), 255)
    elif t == TInt: return Constant.int(Type.int(64), 2**63-1)
    elif t == TAny: return Constant.int(Type.int(64), 2**63-1)
    elif t == TFunc: return Constant.null(Type.pointer(funcBase))
    raise ICEException("Unhandled type %s"%str(t))

def intp(v):
    return Constant.int(Type.int(32), v)

def funcType(argc):
    return Type.function(Type.void(), 
                         [Type.pointer(funcBase), Type.pointer(anyRetType)] +
                         [Type.int(8), Type.int(64)] * argc, False)

class LLVMCodeGen(visitor.Visitor):
    def cast(self, value, tfrom, tto, node):
        print "CAST", tfrom, tto, value
        if tfrom == tto:
            return value
        if tto == TAny:
            return (typeRepr(tfrom), self.builder.bitcast(value, Type.int(64)))
        if tfrom == TAny:
            t, v = value
            fblock = self.function.append_basic_block("b%d"%self.uid)
            nblock = self.function.append_basic_block("b%d"%(self.uid+1))
            self.uid += 2
            self.builder.cbranch(self.builder.icmp(ICMP_EQ, t, typeRepr(tto)), nblock, fblock)
            self.builder.position_at_end(fblock)
            self.builder.call(self.typeErr, 
                              [Constant.int(Type.int(32), node.charRange.lo),
                               Constant.int(Type.int(32), node.charRange.hi),
                               t,
                               typeRepr(tto)])
            self.builder.unreachable()
            self.block = nblock
            self.builder.position_at_end(self.block)
            return self.builder.bitcast(v, llvmType(tto))

            
            
    def __init__(self, err, code):
        self.code = code
        self.err = err
        self.module = Module.new("Monkey")
        #self.module.link_in("stdlib")
        self.uid = 0
        self.passMgr = FunctionPassManager.new(self.module)
        
        typeErrType = Type.function(Type.void(),
                                    [Type.int(32), Type.int(32), Type.int(8), Type.int(8)])
        argCntErrType = Type.function(Type.void(), 
                                        [Type.int(32), Type.int(32), Type.int(16), Type.int(16)])

        self.typeErr = Function.new(self.module, typeErrType, "emit_type_error")
        self.argCntErr = Function.new(self.module, argCntErrType, "emit_arg_cnt_error")
        
        # Do simple "peephole" optimizations and bit-twiddling optzns.
        self.passMgr.add(PASS_INSTCOMBINE)
        # Reassociate expressions.
        self.passMgr.add(PASS_REASSOCIATE)
        # Eliminate Common SubExpressions.
        self.passMgr.add(PASS_GVN)

        self.passMgr.add(PASS_DCE)
        # Simplify the control flow graph (deleting unreachable blocks, etc).
        #self.passMgr.add(PASS_CFG_SIMPLIFICATION)
        self.passMgr.initialize()

    def visitVariableExp(self, node):
        return node.store.value

    def visitAssignmentExp(self, node):
        node.value = self.visit(node.valueExp)
        return node.value

    def visitIfExp(self, node):
        #TODO I DO NOT WORK FOR ANY TYPE do to value beeing a pair
        
        done=False
        hats = []
        for choice in node.choices:
            # Evaluate condition and cast value to bool
            cond = self.cast(self.visit(choice.condition), 
                             choice.condition.type, 
                             TBool, 
                             choice.condition)
            if cond == Constant.int(Type.int(8), 0) or done:
                self.err.reportWarning("Branch never taken", choice.arrowToken)
                continue
            
            if cond == Constant.int(Type.int(8), 1):
                done = True

            # Evaluate value and cast to result type
            value = self.cast(self.visit(choice.value), choice.value.type, node.type, 
                              choice.value)
            hats.append((cond, value))
            
        if not done:
            val = getUndef(node.type)
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
        captureType = Type.struct( [
            Type.pointer(funct_type), #Function ptr
            Type.int(16), # Number of arguments
        ] + [llvmType(cap.type) for cap in node.captures])

        # Cache current state
        f = self.function
        b = self.block
        bb = self.builder.basic_block
        
        # Name new function and block
        name = "f%d"%self.uid
        self.uid += 1
        self.function = Function.new(self.module, funct_type, name)
        self.block = self.function.append_basic_block('entry')
        self.builder.position_at_end(self.block)

        func = self.function

        # Setup args
        self.function.args[0].name = "self"
        self.function.args[1].name = "ret"
        selfv = self.builder.bitcast(self.function.args[0], Type.pointer(captureType))
        ret = self.function.args[1]
        for i in range(len(node.args)):
            arg = node.args[i]
            t = self.function.args[i*2+2]
            v = self.function.args[i*2+3]
            fblock = self.function.append_basic_block('check_fail_%d'%i)
            nblock = self.function.append_basic_block('check_succ_%d'%i)
            self.builder.cbranch(self.builder.icmp(ICMP_EQ, t, typeRepr(arg.type)),
                                 nblock, fblock)
            
            self.builder.position_at_end(fblock)
            self.builder.call(self.typeErr,
                              [Constant.int(Type.int(32), arg.charRange.lo),
                               Constant.int(Type.int(32), arg.charRange.hi),
                               t,
                               typeRepr(arg.type)])
            self.builder.unreachable()
            self.block = nblock
            self.builder.position_at_end(self.block)

            if arg.type == TInt:
                val = self.builder.bitcast(v, Type.int(64))
            elif arg.type == TBool:
                val = self.builder.bitcast(v, Type.int(8))
            else:
                raise ICEException("Unhandled type %s"%str(arg.type))
            arg.value = (t, val)    
            self.function.args[i*2+2].name = "t_"+arg.name
            self.function.args[i*2+3].name = "v_"+arg.name

        for i in range(len(node.captures)):
            cap = node.captures[i]
            cap.value = self.builder.load(self.builder.gep(selfv, [intp(0), intp(2+i)]))
                    
        # Build function code
        t, v = self.cast(self.visit(node.body), node.body.type, TAny, node.body)
        self.builder.store(t, self.builder.gep(ret, [intp(0), intp(0)]))
        self.builder.store(self.builder.bitcast(v, Type.int(64)), 
                           self.builder.gep(ret, [intp(0), intp(1)]))
        self.builder.ret_void()

        self.passMgr.run(self.function)

        # Revert state
        self.function = f
        self.block = b
        self.builder.position_at_end(bb)
        
        # Allocate function object
        p = self.builder.malloc(captureType)

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
        
        capture=self.builder.bitcast(self.cast(self.visit(node.funcExp), node.funcExp.type, 
                                               TFunc, node.funcExp), 
                                     Type.pointer(funcBase))
        rv = self.builder.alloca(anyRetType)

        fblock = self.function.append_basic_block('check_fail_%d'%self.uid)
        nblock = self.function.append_basic_block('check_succ_%d'%(self.uid+1))
        self.uid += 2
        argc = self.builder.load(self.builder.gep(capture, [intp(0), intp(1)]))
        margc = Constant.int(Type.int(16), len(node.args))
        self.builder.cbranch(self.builder.icmp(ICMP_EQ, argc, margc),
                             nblock, fblock)
        
        self.builder.position_at_end(fblock)
        self.builder.call(self.argCntErr,
                          [Constant.int(Type.int(32), node.charRange.lo),
                           Constant.int(Type.int(32), node.charRange.hi),
                           argc,
                           margc])
        self.builder.unreachable()
        self.block = nblock
        self.builder.position_at_end(self.block)

        args = [capture, rv]

        for arg in node.args:
            argv = self.visit(arg)
            if arg.type == TAny:
                args.append(argv[0])
                args.append(argv[1])
            else:
                args.append(typeRepr(arg.type))
                args.append(self.builder.bitcast(argv, Type.int(64)))
        voidfp = self.builder.load(self.builder.gep(capture, [intp(0), intp(0)]))
        fp = self.builder.bitcast(voidfp, Type.pointer(ft))
        self.builder.call(fp, args)
        
        return (self.builder.load(self.builder.gep(rv, [intp(0), intp(0)])),
                self.builder.load(self.builder.gep(rv, [intp(0), intp(1)])))
                            

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
        a = self.cast(self.visit(node.lhs), node.lhs.type, TInt, node.lhs)
        b = self.cast(self.visit(node.rhs), node.rhs.type, TInt, node.rhs)
        if node.token.id == TK_PLUS:
            return self.cast(self.builder.add(a, b), TInt, node.type, node)
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
