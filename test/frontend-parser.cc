// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>
#include "common.hh"
#include <frontend/lexer.hh>
#include <frontend/parser.hh>
#include <frontend/code.hh>
#include <frontend/visitor.hh>
using namespace rasmus;
using namespace rasmus::frontend;

class ASTDumper: public VisitorCRTP<ASTDumper, void> {
public:
	std::stringstream ss;
	std::shared_ptr<Code> code;
	ASTDumper(std::shared_ptr<Code> code): code(code) {}
	
	struct NodeVisitor {
		ASTDumper & dumper;
		NodePtr node;
		NodeVisitor(ASTDumper & dumper, NodePtr node): dumper(dumper), node(node) {}
		friend std::ostream & operator<<(std::ostream & o, const NodeVisitor & nv) {
			nv.dumper.visitNode(nv.node);
			return o;
		}
	};
	
	std::string t(Token t) {
		return t.getText(code);
	}

	NodeVisitor v(NodePtr node) {
		return NodeVisitor(*this, node);
	}

	void visit(std::shared_ptr<VariableExp> node) {
		ss << t(node->nameToken);
	}
	
    void visit(std::shared_ptr<AssignmentExp> node) {
		ss << "(:= " << t(node->nameToken) << " " << v(node->valueExp) << ")";
	}

    void visit(std::shared_ptr<IfExp> node) {
		ss << "(if";
		for (auto choice: node->choices)
			ss << " " << v(choice->condition) << " " << v(choice->value);
		ss << ")";
	}
			
    void visit(std::shared_ptr<ForallExp> node) {
        visitAll(node->listExps);
        visitNode(node->exp);
		//TODO
	}

    void visit(std::shared_ptr<FuncExp> node) {
		ss << "(func (";
        bool f=true;
        for (auto arg: node->args) {
            if (f) f=false;
            else ss << " ";
			ss << t(arg->nameToken) << " " << t(arg->typeToken);
		}
		ss << ") " << t(node->returnTypeToken) << " " << v(node->body) << ")";
	}
	
    void visit(std::shared_ptr<TupExp> node) {
		ss << "(tup";
		for (auto item: node->items) 
			ss << " " << t(item->nameToken) << " " << v(item->exp);
		ss << ")";
	}

    void visit(std::shared_ptr<BlockExp> node) {
		ss << "(block";
        for (auto val: node->vals) {
			ss << " " << t(val->nameToken) << " " << v(val->exp);
		}
		ss << " " << v(node->inExp) << ")";
	}

    void visit(std::shared_ptr<BuiltInExp> node) {
		ss << "(" << t(node->nameToken);
        for (auto arg: node->args)
			ss << " " << v(arg);
		ss << ")";
	}

    void visit(std::shared_ptr<ConstantExp> node) {
		ss << t(node->valueToken);
	}

    void visit(std::shared_ptr<UnsetExp> node) {
		ss << "(unset " << t(node->nameToken) << ")";
	}

    void visit(std::shared_ptr<UnaryOpExp> node) {
		ss << "(" << t(node->opToken) << " " << v(node->exp) << ")";
	}

    void visit(std::shared_ptr<RelExp> node) {
		ss << "(rel " << v(node->exp) << ")";
	}

    void visit(std::shared_ptr<LenExp> node) {
		ss << "(len " << v(node->exp) << ")";
	}

    void visit(std::shared_ptr<FuncInvocationExp> node) {
		ss << "(call " << v(node->funcExp);
        for (auto arg: node->args)
			ss << " " << v(arg);
		ss << ")";
	}

    void visit(std::shared_ptr<SubstringExp> node) {
		ss << "(.. " << v(node->stringExp) << " " << v(node->fromExp) << " " << v(node->toExp) << ")";
	}

    void visit(std::shared_ptr<RenameExp> node) {
		ss << "([ " << v(node->lhs);
        for (auto item: node->renames)
			ss << " " << t(item->fromNameToken) << " " << t(item->toNameToken);
		ss << ")";
	}
        
    void visit(std::shared_ptr<DotExp> node) {
		ss << "(. " << v(node->lhs) << " " << t(node->nameToken) << ")";
	}

    void visit(std::shared_ptr<TupMinus> node) {
		ss << "(\\ " << v(node->lhs) << " " << t(node->nameToken) << ")";
	}

    void visit(std::shared_ptr<ProjectExp> node) {
		ss << "(" << t(node->projectionToken) << " " << v(node->lhs);
        for (auto name: node->names)
			ss << " " << t(name);
		ss << ")";
	}

    void visit(std::shared_ptr<BinaryOpExp> node) {
		ss << "(" << t(node->opToken) << " " << v(node->lhs) << " " << v(node->rhs) << ")";
	}

    void visit(std::shared_ptr<SequenceExp> node) {
		ss << "(;";
		for (auto e: node->sequence)
			ss << " " << v(e);
		ss << ")";
	}

    void visit(std::shared_ptr<InvalidExp>) {	
		ss << "invalid";
	}

    void visit(std::shared_ptr<AtExp> node) {
		ss << t(node->atToken);
	}

	void visit(std::shared_ptr<Choice>) {}
	void visit(std::shared_ptr<FuncCaptureValue>) {}
	void visit(std::shared_ptr<FuncArg>) {}
	void visit(std::shared_ptr<TupItem>) {}
	void visit(std::shared_ptr<Val>) {}
	void visit(std::shared_ptr<RenameItem>) {}
};


bool pt(const char * txt, const char * exp, size_t errors=0) {
	std::shared_ptr<Code> code = std::make_shared<Code>(txt, "monkey");
	std::shared_ptr<Lexer> lexer = std::make_shared<Lexer>(code, 0);
	std::shared_ptr<Error> error = makeCountError();
	std::shared_ptr<Parser> p = makeParser(lexer, error, false);
	
	NodePtr ast=p->parse();
	ASTDumper ad(code);
	ad.visitNode(ast);
	if (ad.ss.str() != exp) {
		log_error() << "Parsed: " << ad.ss.str() << "; Expected: " << exp << std::endl;
		return false;
	}
	return errors == error->count();
}

void base(rasmus::teststream & ts) {
    ts << "variable" << result(pt("hat", "hat"));
    ts << "assign" << result(pt("hat:=4", "(:= hat 4)"));
    ts << "if" << result(pt("if false -> 4 & true -> 5 fi", "(if false 4 true 5)"));
    //todo forall
    ts << "buildin1" << result(pt("today", "(today)"));
    ts << "buildin2" << result(pt("close", "(close)"));
    ts << "buildin3" << result(pt("is-Atom(a)", "(is-Atom a)"));
    ts << "buildin4" << result(pt("is-Tup(a)", "(is-Tup a)"));
    ts << "buildin5" << result(pt("is-Rel(a)", "(is-Rel a)"));
    ts << "buildin6" << result(pt("is-Func(a)", "(is-Func a)"));
    ts << "buildin7" << result(pt("is-Any(a)", "(is-Any a)"));
    ts << "buildin8" << result(pt("is-Bool(a)", "(is-Bool a)"));
    ts << "buildin9" << result(pt("is-Int(a)", "(is-Int a)"));
    ts << "buildin10" << result(pt("is-Text(a)", "(is-Text a)"));
    ts << "buildin11" << result(pt("system(\"ls\")", "(system \"ls\")"));
    ts << "buildin12" << result(pt("write(\"ls\")", "(write \"ls\")"));
    ts << "buildin13" << result(pt("open(\"ls\")", "(open \"ls\")"));
    ts << "buildin14" << result(pt("has(\"ls\")", "(has \"ls\")"));
    ts << "buildin15" << result(pt("min(1)", "(min 1)"));
    ts << "buildin16" << result(pt("count(1)", "(count 1)"));
    ts << "buildin17" << result(pt("add(1)", "(add 1)"));
    ts << "buildin18" << result(pt("mult(1)", "(mult 1)"));
    ts << "buildin19" << result(pt("days(1)", "(days 1)"));
    ts << "buildin20" << result(pt("before(1)", "(before 1)"));
    ts << "buildin21" << result(pt("after(1)", "(after 1)"));
    ts << "buildin22" << result(pt("date(1)", "(date 1)"));
    ts << "func1" << result(pt("func (a:Int, b:Text) -> (Bool) false end", "(func (a Int b Text) Bool false)"));
    ts << "func2" << result(pt("func () -> (Bool) false end", "(func () Bool false)"));
    ts << "tup1" << result(pt("tup()", "(tup)"));
    ts << "tup1" << result(pt("tup(abe: 4, kat: 5)", "(tup abe 4 kat 5)"));
    ts << "block1" << result(pt("(+ in 42 +)", "(block 42)"));
    ts << "block2" << result(pt("(+ val a = 1 val b = 2 in 42 +)", "(block a 1 b 2 42)"));
    ts << "rel" << result(pt("rel(x)", "(rel x)"));
    ts << "at" << result(pt("@(4)", "@(4)"));
    ts << "len" << result(pt("|\"hat\"|", "(len \"hat\")"));
    ts << "minus" << result(pt("-4", "(- 4)"));
    ts << "zero" << result(pt("zero", "zero"));
    ts << "one" << result(pt("one", "one"));
    ts << "stdbool" << result(pt("?-Bool", "?-Bool"));
    ts << "stdint" << result(pt("?-Int", "?-Int"));
    ts << "stdtext" << result(pt("?-Text", "?-Text"));
    ts << "text" << result(pt("\"hat\"", "\"hat\""));
    ts << "int" << result(pt("4", "4"));
    ts << "true" << result(pt("true", "true"));
    ts << "false" << result(pt("false", "false"));
    ts << "sharp" << result(pt("#", "#"));
    ts << "not" << result(pt("not false", "(not false)"));
    ts << "substr" << result(pt("\"hat\"(1 .. 4)", "(.. \"hat\" 1 4)"));
    ts << "call1" << result(pt("hat(1, 4)", "(call hat 1 4)"));
    ts << "call2" << result(pt("hat()", "(call hat)"));
    ts << "call3" << result(pt("hat(1)", "(call hat 1)"));
    ts << "rename1" << result(pt("a[x<-y, a<-b]", "([ a x y a b)"));
    ts << "rename2" << result(pt("a[x<-y]", "([ a x y)"));
    ts << "dot1" << result(pt("a.b", "(. a b)"));
    ts << "dot2" << result(pt("tup(b: 4).b", "(. (tup b 4) b)"));
    ts << "remove1" << result(pt("a\\b", "(\\ a b)"));
    ts << "remove2" << result(pt("tup(b: 4)\\b", "(\\ (tup b 4) b)"));
    ts << "remove3" << result(pt("a\\b\\c", "(\\ (\\ a b) c)"));
    //ts << "extend(self):
    ts << "concat" << result(pt("a++b", "(++ a b)"));
    ts << "proj1" << result(pt("a |+ a, b", "(|+ a a b)"));
    ts << "proj2" << result(pt("a |- b", "(|- a b)"));
    ts << "mul" << result(pt("a * b", "(* a b)"));
    ts << "div" << result(pt("a / b", "(/ a b)"));
    ts << "mod" << result(pt("a mod b", "(mod a b)"));
    ts << "and" << result(pt("true and false", "(and true false)"));
    ts << "plus" << result(pt("a + b", "(+ a b)"));
    ts << "minus" << result(pt("a - b", "(- a b)"));
    ts << "or" << result(pt("a or b", "(or a b)"));
    ts << "select" << result(pt("a ?(b)", "(? a (func (# Tup) Bool b))"));
    //compares
    ts << "sequence" << result(pt("a;b;c", "(; a b c)"));

    ts << "ass1" << result(pt("a+b+c", "(+ (+ a b) c)"));
    ts << "ass2" << result(pt("a*b+c*d", "(+ (* a b) (* c d))"));
    ts << "ass3" << result(pt("a+b*c+d", "(+ (+ a (* b c)) d)"));
    ts << "paren" << result(pt("(a)", "a"));

    ts << "only_block_start" << result(pt("(+", "invalid", 1));
    ts << "invalid2" << result(pt("system(\"ls\"))", "(system \"ls\")", 1));
}


int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
		.multi_test(base, "base");
}
