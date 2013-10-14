import visitor
from lexer import *

#RUN LIKE
#python RASMUS.py foo.rm | llvm-as | opt -O3 | llc | gcc -x assembler - && ./a.out; echo $?

class LLVMCodeGen(visitor.Visitor):
    def __init__(self, code, f):
        self.code = code
        self.cnt = 0
        self.f = f

    def n(self):
        x = self.cnt
        self.cnt += 1
        return x

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
            a=self.n()
            self.f.write("  %%%d = add i32 %d, 0\n"%(a, int(self.code[
                        node.token.start: node.token.length + node.token.start])))
            return a

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

    def visitBinaryOpExp(self, node):
        a = self.visit(node.lhs)
        b = self.visit(node.rhs) 
        c = self.n()
        self.f.write("  %%%d = add i32 %%%d, %%%d\n"%(c, a, b))
        return c

    def visitSequenceExp(self, node):
        pass

    def generate(self, node):
        self.f.write("define i32 @main() ssp {\n");
        self.f.write("entry:\n");
        a = self.visit(node)
        self.f.write("  ret i32 %%%d\n"%a)
        self.f.write("}\n")

# @.str = private constant [13 x i8] c"Hello World!\00", align 1 ;

# define i32 @main() ssp {
# entry:
#   %retval = alloca i32
#   %0 = alloca i32
#   %"alloca point" = bitcast i32 0 to i32
#   %1 = call i32 @puts(i8* getelementptr inbounds ([13 x i8]* @.str, i64 0, i64 0))
#   store i32 0, i32* %0, align 4
#   %2 = load i32* %0, align 4
#   store i32 %2, i32* %retval, align 4
#   br label %return
# return:
#   %retval1 = load i32* %retval
#   ret i32 %retval1
# }

# declare i32 @puts(i8*)
