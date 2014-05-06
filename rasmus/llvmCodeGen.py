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
    elif t == TText: return Constant.int(Type.int(8), 3)
    raise ICEException("Unhandled type %s"%str(t))

def genUndef(t):
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
        if tfrom == tto:
            return value
        if tto == TAny:
            if tfrom == TInt:
                return (typeRepr(tfrom), value)
            elif tfrom == TBool:
                return (typeRepr(tfrom), self.builder.zext(value, Type.int(64)))
            elif tfrom == TText:
                return (typeRepr(tfrom), self.builder.ptrtoint(value, Type.int(64)))
            else:
                raise ICEException("Unhandled type1")
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
            if tto == TInt:
                return v
            elif tto == TBool:
                return self.builder.trunc(v)
            else:
                raise ICEException("Unhandled type2")
        raise ICEException("Unhandled type 3")
            
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
        printType = Type.function(Type.void(), [Type.int(8), Type.int(64)])

        self.innerType = Type.function(Type.void(), [])
        interactiveWrapperType = Type.function(Type.int(8), 
                                               [Type.pointer(Type.int(8)), Type.pointer(self.innerType)])

        getConstTextType = Type.function(Type.pointer(Type.void()), [Type.pointer(Type.int(8))])

        fs = [
            ('rm_getConstText', getConstTextType),
            ('rm_print',printType), 
            ('rm_emitTypeError', typeErrType), 
            ('rm_emitArgCntError', argCntErrType),
            ('rm_interactiveWrapper', interactiveWrapperType)
        ]

        self.stdlib = {}
        for name, type in fs:
            self.stdlib[name] = Function.new(self.module, type, name)
             
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
            val = genUndef(node.type)
        else:
            _, val = hats.pop()
            
        while hats:
            cond, v = hats.pop()
            val = self.builder.select(cond, v , val)

        return val

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
            arg.value = self.cast(
                (self.function.args[i*2+2], self.function.args[i*2+3]),
                TAny, arg.type, arg)
            self.function.args[i*2+2].name = "t_"+arg.name
            self.function.args[i*2+3].name = "v_"+arg.name

        for i in range(len(node.captures)):
            cap = node.captures[i]
            cap.value = self.builder.load(self.builder.gep(selfv, [intp(0), intp(2+i)]))
                    
        # Build function code
        x = self.visit(node.body)
        t, v = self.cast(x, node.body.type, TAny, node.body)
        print node.body.type ,x, t,v
        self.builder.store(t, self.builder.gep(ret, [intp(0), intp(0)]))
        self.builder.store(v, self.builder.gep(ret, [intp(0), intp(1)]))
        self.builder.ret_void()
        print self.function
        self.function.verify()
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
        raise ICEException("Tub not implemented")

    def visitBlockExp(self, node):
        raise ICEException("Block not implemented")

    def visitBuiltInExp(self, node):
        tkn = node.nameToken.id
        if tkn == TK_PRINT:
            t, v = self.cast(self.visit(node.args[0]), node.args[0].type, TAny, node.args[0])
            self.builder.call(self.stdlib['rm_print'], [t, v])
            return Constant.int(Type.int(8), 1)
        else:
            raise ICEException("BuildIn not implemented")

    def visitConstantExp(self, node):
        if node.type == TInt:
            return Constant.int(Type.int(64), node.int_value)
        elif node.type == TBool:
            return Constant.int(Type.int(8), 1 if node.bool_value else 0)
        elif node.type == TText:
            
#            print #self.builder.call(
#            print self.stdlib['rm_getConstText']
#            print self.builder.gep(Constant.stringz(node.txt_value), [intp(0), intp(0)])

            x = Constant.stringz(node.txt_value)

            gv = self.module.add_global_variable(x.type, "gv1")
            gv.initializer = x

            return self.builder.call(
                self.stdlib['rm_getConstText'],
                [self.builder.gep(gv, [intp(0), intp(0)])]
            )
        else:
            raise ICEException("Const")
    
    def visitUnaryOpExp(self, node):
        raise ICEException("Unary")

    def visitRelExp(self, node):
        raise ICEException("Rel")

    def visitLenExp(self, node):
        raise ICEException("Len")

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
                           margc,
                           argc])
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
        raise ICEException("Substr")

    def visitRenameExp(self, node):
        raise ICEException("Rename")
        
    def visitDotExp(self, node):
        raise ICEException("DotExp")

    def visitProjectExp(self, node):
        raise ICEException("Project")

    def visitExp(self, node):
        return self.visit(node.exp)

    def visitBinaryOpExp(self, node):
        a = self.cast(self.visit(node.lhs), node.lhs.type, TInt, node.lhs)
        b = self.cast(self.visit(node.rhs), node.rhs.type, TInt, node.rhs)
        if node.token.id == TK_PLUS:
            return self.cast(self.builder.add(a, b), TInt, node.type, node)
        elif node.token.id == TK_MUL:
            return self.builder.mul(a,b)
        elif node.token.id == TK_MINUS:
            return self.cast(self.builder.sub(a, b), TInt, node.type, node)
        elif node.token.id == TK_DIV:
            return self.cast(self.builder.sdiv(a, b), TInt, node.type, node)
        elif node.token.id == TK_MOD:
            return self.cast(self.builder.srem(a, b), TInt, node.type, node)
        elif node.token.id == TK_GREATER:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_SGT, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_LESS:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_SLT, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_GREATEREQUAL:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_UGE, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_LESSEQUAL:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_ULE, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_DIFFERENT:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_NE, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_EQUAL:
            if node.lhs.type == TInt and node.rhs.type == TInt:
                return self.builder.icmp(ICMP_EQ, a, b)
            else:
                raise ICEException("Only Integer comparison implemented")
        elif node.token.id == TK_TILDE:
            # text comparison: true if a is a substring of b
            raise ICEExpcetion("Substring test not implemented")
        elif node.token.id == TK_AND:
            return self.cast(self.builder.and_(a, b), TBool, node.type, node)
        elif node.token.id == TK_OR:
            return self.cast(self.builder.or_(a, b), TBool, node.type, node)
        elif node.token.id == TK_CONCAT:
            raise ICEException("Binary operation: Concatenation not implemented")
        else:
            raise ICEException("Binop not implemented")
    
    def visitSequenceExp(self, node):
        for n in node.sequence:
            x = self.visit(n)
        return x

    def generate(self, node):
        pass

    def visitOuter(self, AST):
        
        id = self.uid
        self.uid += 1

        funct_type = Type.function(Type.void(), [], False)
        self.function = Function.new(self.module, funct_type, "INNER_%d"%id)
        self.block = self.function.append_basic_block('entry')
        self.builder = Builder.new(self.block)
        self.visit(AST)
        self.builder.ret_void()
        self.function.verify()
        self.passMgr.run(self.function)
        print self.function
        inner = self.function

        outer_type = Type.function(Type.int(8), [Type.pointer(Type.int(8))], False)
        self.function = Function.new(self.module, outer_type, "OUTER_%d"%id)
        self.block = self.function.append_basic_block('entry')
        self.builder = Builder.new(self.block)
        self.builder.ret( self.builder.call(self.stdlib['rm_interactiveWrapper'], [self.function.args[0], inner]) )
        self.function.verify()
        self.passMgr.run(self.function)
