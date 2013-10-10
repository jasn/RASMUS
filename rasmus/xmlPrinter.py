from sys import stdout
from xml.sax.saxutils import escape, quoteattr

class XmlPrinter:
   
    def __init__(self, code):
        self.code = code
        self.indent = 0

    def startAny(self, s, keys):
        stdout.write("%s<%s"%("\t"*self.indent,s))
        for k,v in keys.iteritems():
            stdout.write(" %s=%s"%(escape(k),quoteattr(self.code[v.start:v.start+v.length])))

    def start(self,s, **keys):
        self.startAny(s, keys)
        stdout.write(">\n")
        self.indent += 1
        pass
    
    def end(self, s):
        self.indent -= 1
        print "%s</%s>"%("\t"*self.indent,s)
        pass

    def startend(self, s, **keys):
        self.startAny(s, keys)
        stdout.write(" />\n")
        pass
        
    def visit(self, node):
        if isinstance(node, list):
            for i in node:
                self.visit(i)
        else:
            node.visit(self)

    def visitVariableExp(self, node):
        self.startend("variable", name=node.token)
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

    def visitAtExp(self, node):
        pass

    def visitConstantExp(self, node):
        self.startend("constant", value=node.token)
        pass

    def visitUnaryOpExp(self, node):
        pass

    def visitRelExp(self, node):
        pass

    def visitLenExp(self, node):
        pass

    def visitFuncInvocationExp(self, node):
        self.start("func")
        self.visit(node.funcExp)
        self.start("args")
        self.visit(node.args)
        self.end("args")
        self.end("func")
        pass

    def visitSubstringExp(self, node):
        self.start("substring")
        self.visit([node.stringExp, node.fromExp, node.toExp])
        self.end("substring")
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
        self.start("binaryOp", op=node.token)
        self.visit(node.lhs)
        self.visit(node.rhs)
        self.end("binaryOp")
        pass

    def visitSequenceExp(self, node):
        self.start("sequence")
        self.visit(node.sequence)
        self.end("sequence")
