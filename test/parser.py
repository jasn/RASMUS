import unittest
import rasmus.parser
import rasmus.visitor
from StringIO import StringIO

class AstDumper(rasmus.visitor.Visitor):
    def __init__(self, code, f):
        self.code = code
        self.f = f

    def w(self, m):
        self.f.write(m)
        
    def t(self, token):
        return self.code[token.start: token.start+token.length]

    def visitVariableExp(self, node):
        self.w(self.t(node.token))

    def visitAssignmentExp(self, node):
        self.w("(:= %s "%self.t(node.nameToken))
        self.visit(node.valueExp)
        self.w(")")

    def visitIfExp(self, node):
        self.w("(if")
        for choice in node.choices:
            self.w(" ")
            self.visit(choice.condition)
            self.w(" ")
            self.visit(choice.value)
        self.w(")")

    def visitForallExp(self, node):
        self.visitAll(node.listExps)
        self.visit(node.expr)
        #TODO

    def visitFuncExp(self, node):
        self.w("(func (")
        f=True
        for arg in node.args:
            if f: f=False
            else: self.w(" ")
            self.w("%s %s"%(self.t(arg.nameToken), self.t(arg.typeToken)))
        self.w(") %s "%self.t(node.returnTypeToken))
        self.visit(node.body)
        self.w(")")

    def visitTupExp(self, node):
        self.w("(tup")
        for item in node.items: 
            self.w(" %s "%self.t(item.nameToken))
            self.visit(item.exp)
        self.w(")")    

    def visitBlockExp(self, node):
        self.w("(block")
        for val in node.vals: 
            self.w(" %s "%self.t(val.nameToken))
            self.visit(val.exp)
        self.w(" ")
        self.visit(node.inExp)
        self.w(")")

    def visitBuiltInExp(self, node):
        self.w("(%s"%self.t(node.nameToken))
        for arg in node.args:
            self.w(" ")
            self.visit(arg)
        self.w(")")

    def visitConstantExp(self, node):
        self.w(self.t(node.token))

    def visitUnaryOpExp(self, node):
        self.w("(%s "%self.t(node.token))
        self.visit(node.exp)
        self.w(")")

    def visitRelExp(self, node):
        self.w("(rel ")
        self.visit(node.exp)
        self.w(")")

    def visitLenExp(self, node):
        self.w("(len ")
        self.visit(node.exp)
        self.w(")")

    def visitFuncInvocationExp(self, node):
        self.w("(call ")
        self.visit(node.funcExp)
        for arg in node.args:
            self.w(" ")
            self.visit(arg)
        self.w(")")

    def visitSubstringExp(self, node):
        self.w("(.. ")
        self.visit(node.stringExp)
        self.w(" ")
        self.visit(node.fromExp)
        self.w(" ")
        self.visit(node.toExp)
        self.w(")")

    def visitRenameExp(self, node):
        self.w("([ ")
        self.visit(node.lhs)
        for item in node.renames:
            self.w(" %s %s"%(self.t(item.fromName), self.t(item.toName)))
        self.w(")")
        
    def visitDotExp(self, node):
        self.w("(. ")
        self.visit(node.lhs)
        self.w(" %s)"%self.t(node.nameToken));

    def visitProjectExp(self, node):
        self.w("(%s "%self.t(node.projectionToken))
        self.visit(node.lhs)
        for name in node.names:
            self.w(" %s"%self.t(name))
        self.w(")")

    def visitBinaryOpExp(self, node):
        self.w("(%s "%self.t(node.token))
        self.visit(node.lhs)
        self.w(" ")
        self.visit(node.rhs)
        self.w(")")

    def visitSequenceExp(self, node):
        self.w("(;")
        for e in node.sequence:
            self.w(" ")
            self.visit(e)
        self.w(")")

    def visitInvalidExp(self, node):
        self.w("invalid");

    def visitExp(self, node):
        self.visit(node.exp)

class Error:
    def __init__(self):
        self.count = 0

    def reportError(self, *_):
        self.count += 1

class Testparser(unittest.TestCase):
    def do(self, code, lisp, errs=0):
        e = Error()
        p = rasmus.parser.Parser(e, code)
        AST = p.parse()
        f = StringIO()
        AstDumper(code, f).visit(AST)
        self.assertEqual(lisp, f.getvalue())
        self.assertEqual(e.count, errs)
    
    def test_variable(self): self.do("hat", "hat")
    def test_assign(self): self.do("hat:=4", "(:= hat 4)")
    def test_if(self): self.do("if false -> 4 & true -> 5 fi", "(if false 4 true 5)")
    #todo forall
    def test_buildin1(self): self.do("today", "(today)")
    def test_buildin2(self): self.do("close", "(close)")
    def test_buildin3(self): self.do("is-Atom(a)", "(is-Atom a)")
    def test_buildin4(self): self.do("is-Tup(a)", "(is-Tup a)")
    def test_buildin5(self): self.do("is-Rel(a)", "(is-Rel a)")
    def test_buildin6(self): self.do("is-Func(a)", "(is-Func a)")
    def test_buildin7(self): self.do("is-Any(a)", "(is-Any a)")
    def test_buildin8(self): self.do("is-Bool(a)", "(is-Bool a)")
    def test_buildin9(self): self.do("is-Int(a)", "(is-Int a)")
    def test_buildin10(self): self.do("is-Text(a)", "(is-Text a)")
    def test_buildin11(self): self.do("system(\"ls\")", "(system \"ls\")")
    def test_buildin12(self): self.do("write(\"ls\")", "(write \"ls\")")
    def test_buildin13(self): self.do("open(\"ls\")", "(open \"ls\")")
    def test_buildin14(self): self.do("has(\"ls\")", "(has \"ls\")")
    def test_buildin15(self): self.do("min(1)", "(min 1)")
    def test_buildin16(self): self.do("count(1)", "(count 1)")
    def test_buildin17(self): self.do("add(1)", "(add 1)")
    def test_buildin18(self): self.do("mult(1)", "(mult 1)")
    def test_buildin19(self): self.do("days(1)", "(days 1)")
    def test_buildin20(self): self.do("before(1)", "(before 1)")
    def test_buildin21(self): self.do("after(1)", "(after 1)")
    def test_buildin22(self): self.do("date(1)", "(date 1)")
    def test_func1(self): self.do("func (a:Int, b:Text) -> (Bool) false end", "(func (a Int b Text) Bool false)")
    def test_func2(self): self.do("func () -> (Bool) false end", "(func () Bool false)")
    def test_tup1(self): self.do("tup()", "(tup)")
    def test_tup1(self): self.do("tup(abe: 4, kat: 5)", "(tup abe 4 kat 5)")
    def test_block1(self): self.do("(+ in 42 +)", "(block 42)")
    def test_block2(self): self.do("(+ val a = 1 val b = 2 in 42 +)", "(block a 1 b 2 42)")
    def test_rel(self): self.do("rel(x)", "(rel x)")
    def test_at(self): self.do("@4", "(@ 4)")
    def test_len(self): self.do("|\"hat\"|", "(len \"hat\")")
    def test_minus(self): self.do("-4", "(- 4)")
    def test_zero(self): self.do("zero", "zero")
    def test_one(self): self.do("one", "one")
    def test_stdbool(self): self.do("?-Bool", "?-Bool")
    def test_stdint(self): self.do("?-Int", "?-Int")
    def test_stdtext(self): self.do("?-Text", "?-Text")
    def test_text(self): self.do("\"hat\"", "\"hat\"")
    def test_int(self): self.do("4", "4")
    def test_true(self): self.do("true", "true")
    def test_false(self): self.do("false", "false")
    def test_sharp(self): self.do("#", "#")
    def test_not(self): self.do("not false", "(not false)")
    def test_substr(self): self.do("\"hat\"(1..4)", "(.. \"hat\" 1 4)")
    def test_call1(self): self.do("hat(1, 4)", "(call hat 1 4)")
    def test_call2(self): self.do("hat()", "(call hat 1 4)")
    def test_call3(self): self.do("hat(1)", "(call hat 1)")
    def test_rename1(self): self.do("a[x->y, a->b]", "([ a x y a b)")
    def test_rename2(self): self.do("a[x->y]", "([ a x y)")
    def test_dot(self): self.do("a.b", "(. a b)")
    #def test_extend(self):
    def test_concat(self): self.do("a++b", "(++ a b)")
    def test_proj1(self): self.do("a |+ a, b", "(|+ a a b)")
    def test_proj2(self): self.do("a |- b", "(|- a b)")
    def test_mul(self): self.do("a * b", "(* a b)")
    def test_div(self): self.do("a / b", "(/ a b)")
    def test_mod(self): self.do("a mod b", "(mod a b)")
    def test_and(self): self.do("true and false", "(and true false)")
    def test_plus(self): self.do("a + b", "(+ a b)")
    def test_minus(self): self.do("a - b", "(- a b)")
    def test_or(self): self.do("a or b", "(or a b)")
    def test_select(self): self.do("a ? b", "(? a b)")
    #compares
    def test_sequence(self): self.do("a;b;c", "(; a b c)")

    def test_ass1(self): self.do("a+b+c", "(+ (+ a b) c)")
    def test_ass2(self): self.do("a*b+c*d", "(+ (* a b) (* c d))")
    def test_ass3(self): self.do("a+b*c+d", "(+ (+ a (* b c)) d)")
    def test_paren(self): self.do("(a)", "a")
