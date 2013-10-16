import visitor

class Dict:
    def __init__(self, printer):
        self.p = printer

    def __enter__(self):
        if self.p.inList[-1]:
            self.p.putComma()
        self.p.f.write("{")
        self.p.incIndent()

    def __exit__(self, *args):
        self.p.decIndent()
        self.p.f.write("\n%s}"%("\t"*self.p.indent))

class Node(Dict):
    def __init__(self, printer, node, name):
        Dict.__init__(self, printer)
        self.node = node
        self.name = name

    def __enter__(self):
        Dict.__enter__(self)
        with self.p.attribute("class"): 
            self.p.value(self.name)
        # if self.node.charRange:
        #     with self.p.attribute("charRange"): 
        #         self.p.value(self.node.charRange)
        if self.node.cvalue:
            with self.p.attribute("cvalue"): 
                self.p.value(self.node.cvalue)
        if self.node.type:
            with self.p.attribute("type"): 
                self.p.value(self.node.type)

class Attribute:
    def __init__(self, printer, name, l=False):
        self.p = printer
        self.name = name
        self.l = l

    def __enter__(self):
        self.p.inList.append(self.l)
        self.p.putComma()
        self.p.f.write("\n%s\"%s\": "%("\t"*self.p.indent, self.name))

    def __exit__(self, *args):
        self.p.inList.pop()
        
class List(Attribute):
    def __init__(self, printer, name):
        Attribute.__init__(self, printer, name, True)

    def __enter__(self):
        Attribute.__enter__(self)
        self.p.incIndent()
        self.p.f.write("[\n%s"%("\t"*self.p.indent));

    def __exit__(self, *args):
        self.p.decIndent()
        self.p.f.write("\n%s]"%("\t"*self.p.indent))
        Attribute.__exit__(self)


class JSONPrinter(visitor.Visitor):
    def __init__(self, code, f):
        self.code = code
        self.f = f
        self.indent = 0
        self.firstStack = [True]
        self.inList = [False]

    def incIndent(self):
        self.indent += 1
        self.firstStack.append(True)

    def decIndent(self):
        self.indent -= 1
        self.firstStack.pop()

    def putComma(self):
        if self.firstStack[-1]:
            self.firstStack[-1] = False
        else:
            self.f.write(",")

    def node(self, node, name):
        return Node(self, node, name)

    def list(self, name):
        return List(self, name)

    def attribute(self, name):
        return Attribute(self, name)

    def valueInner(self, s):
        if s is None:
            return "null"
        elif s is True or s == "true":
            return "true"
        elif s is False or s == "false":
            return "false"
        elif isinstance(s, ( int, long )) or str(s).isdigit():
            return str(s)
        elif (hasattr(s, '__iter__') or hasattr(s, '__getitem__')) and not isinstance(s, str):
            return "[%s]"%(",".join([self.valueInner(i) for i in s]))
        else:
            return("\"%s\""%str(s).replace("\\", "\\\\").replace("\"", "\\\""))

    def value(self, s):
        if self.inList[-1]:
            if self.firstStack[-1]:
                self.firstStack[-1] = False
            else:
                self.f.write(",\n%s"%("\t"*self.indent))
        self.f.write(self.valueInner(s))
    
    def tokenAttribute(self, name, token):
        with self.attribute(name):
            self.token(token)

    def token(self, token):
        self.value(self.code[token.start: token.start+token.length])

    def dict(self):
        return Dict(self)

    def visitVariableExp(self, node):
        with self.node(node, "variable"):
            self.tokenAttribute("value", node.token)

    def visitAssignmentExp(self, node):
        with self.node(node, "assignment"):
            self.tokenAttribute("name", node.nameToken)
            with self.attribute("value"):
                self.visit(node.valueExp)

    def visitIfExp(self, node):
        with self.node(node, "if"):
            with self.list("choices"):
                for choice in node.choices:
                    with self.dict():
                        with self.attribute("condition"):
                            self.visit(choice.condition)
                        with self.attribute("value"):
                            self.visit(choice.value)

    def visitForallExp(self, node):
        with self.node(node, "forall"):
            with self.list("list"):
                self.visitAll(node.listExps)
            if len(node.names):
                with self.list("names"):
                    for name in node.names:
                        self.token(name)
            with self.attribute("expr"):
                self.visit(node.expr)

    def visitFuncExp(self, node):
        with self.node(node, "func"):
            with self.list("args"):
                for arg in node.args:
                    with self.dict():
                        self.tokenAttribute("name", arg.nameToken)
                        self.tokenAttribute("type", arg.typeToken)
            self.tokenAttribute("rtype", node.returnTypeToken)
            with self.attribute("body"):
                self.visit(node.body)

    def visitTupExp(self, node):
        with self.node(node, "tup"):
            with self.list("items"):
                for item in node.items:
                    with self.dict():
                        self.tokenAttribute("name", item.nameToken)
                        with self.attribute("value"):
                            self.visit(item.exp)

    def visitBlockExp(self, node):
        with self.node(node, "block"):
            with self.list("vals"):
                for val in node.vals: 
                    with self.dict():
                        self.tokenAttribute("name", val.nameToken)
                        with self.attribute("value"):
                            self.visit(val.exp)
            with self.attribute("in"):
                self.visit(node.inExp)

    def visitBuiltInExp(self, node):
        with self.node(node, "buildin"):
            self.tokenAttribute("name", node.nameToken)
            with self.list("args"):
                self.visitAll(node.args)

    def visitConstantExp(self, node):
        with self.node(node, "constant"):
            self.tokenAttribute("value", node.token)

    def visitUnaryOpExp(self, node):
        with self.node(node, "unary"):
            self.tokenAttribute("operator", node.token)
            with self.attribute("exp"):
                self.visit(node.exp)

    def visitRelExp(self, node):
        with self.node(node, "rel"):
            with self.attribute("exp"):
                self.visit(node.exp)

    def visitLenExp(self, node):
        with self.node(node, "len"):
            with self.attribute("exp"):
                self.visit(node.exp)

    def visitFuncInvocationExp(self, node):
        with self.node(node, "invoke"):
            with self.attribute("func"):
                self.visit(node.funcExp)
            with self.list("args"):
                self.visitAll(node.args)

    def visitSubstringExp(self, node):
        with self.node(node, "substr"):
            with self.attribute("str"):
                self.visit(node.stringExp)
            with self.attribute("from"):
                self.visit(node.fromeXP)
            with self.attribute("to"):
                self.visit(node.toExp)

    def visitRenameExp(self, node):
        with self.node(node, "rename"):
            with self.attribute("lhs"):
                self.visit(node.lhs)
            with self.list("items"):
                for item in node.renames:
                    with self.dict():
                        self.tokenAttribute("from", item.fromName)
                        self.tokenAttribute("to", item.toName)
        
    def visitDotExp(self, node):
        with self.node(node, "dot"):
            with self.attribute("lhs"):
                self.visit(node.lhs)
            self.tokenAttribute("name", node.nameToken)

    def visitProjectExp(self, node):
        with self.node(node, "project"):
            self.tokenAttribute("op", node.projectionToken)
            with self.attribute("lhs"):
                self.visit(node.lhs)
            with self.list("names"):
                for name in node.names:
                    self.token(name)

    def visitBinaryOpExp(self, node):
        with self.node(node, "binary"):
            self.tokenAttribute("operator", node.token)
            with self.attribute("lhs"):
                self.visit(node.lhs)
            with self.attribute("rhs"):
                self.visit(node.rhs)

    def visitSequenceExp(self, node):
        with self.node(node, "sequence"):
            with self.list("items"):
                self.visitAll(node.sequence)

    def visitInvalidExp(self, node):
        with self.node(node, "invalid"):
            pass

    def visitExp(self, node):
        with self.node(node, "scope"):
            with self.attribute("value"):
                self.visit(node.exp)
        pass
