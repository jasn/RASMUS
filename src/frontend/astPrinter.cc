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

#include <frontend/astPrinter.hh>
#include <frontend/visitor.hh>
#include <iostream>

namespace {
using namespace rasmus::frontend;

class ASTPrinterImpl: public VisitorCRTP<ASTPrinterImpl, void>, public AstPrinter {
public:
	struct StackItem {
		int indent;
		bool first;
		bool list;
		bool small;
	};

	std::shared_ptr<Code> code;
	std::ostream * s;
	std::vector<StackItem> stack;

	ASTPrinterImpl(std::shared_ptr<Code> code): code(code), s(nullptr) {}

	void doPropHead(std::string name) {
		if (stack.back().first) stack.back().first=false;
		else {
			if (!stack.back().small) *s << ",";
			else *s << ", ";
		}
		if (!stack.back().small) 
			*s << '\n' << std::string(stack.back().indent*2, ' ');
		*s << name << ": ";
	}

	void doStart(bool list, bool small) {
		if (stack.back().list && stack.back().first)
			*s << '\n' << std::string(stack.back().indent*2, ' ');
		if (list) 
			*s << "[";
		else
			*s << "{";
		stack.push_back(StackItem{stack.back().indent+1, true, list, small});
	}

	void doEnd() {
		if (!stack.back().small) 
			*s << '\n' << std::string(stack[stack.size()-2].indent*2, ' ');
		if (stack.back().list)
			*s << "]";
		else
			*s << "}";
		stack.pop_back();
	}

	struct NodeProp {
		ASTPrinterImpl & dumper;
		std::string name;
		NodePtr node;
		friend std::ostream & operator<<(std::ostream & o, const NodeProp & np) {
			np.dumper.doPropHead(np.name);
			np.dumper.visitNode(np.node);
			return o;
		}
	};

	struct StringProp {
		ASTPrinterImpl & dumper;
		std::string name;
		std::string value;
		friend std::ostream & operator<<(std::ostream & o, const StringProp & sp) {
			sp.dumper.doPropHead(sp.name);
			o << sp.value;
			return o;
		}
	};
	
	struct Start {
		ASTPrinterImpl & dumper;
		NodePtr node;
		std::string name;
		bool small;
		friend std::ostream & operator<<(std::ostream & o, const Start & s) {
			s.dumper.doStart(false, s.small);
			if (s.node) {
				s.dumper.doPropHead("node");
				o << s.name;
				s.dumper.doPropHead("type");
				o << s.node->type;
			}
			return o;
		}
	};

	struct LStart {
		ASTPrinterImpl & dumper;
		std::string name;
		bool small;
		friend std::ostream & operator<<(std::ostream & o, const LStart & s) {
			s.dumper.doPropHead(s.name);
			s.dumper.doStart(true, s.small);
			return o;
		}
	};

	struct End {
		ASTPrinterImpl & dumper;
		friend std::ostream & operator<<(std::ostream & o, const End & e) {
			e.dumper.doEnd();
			return o;
		}
	};

	StringProp p(std::string name, std::string value) {return StringProp{*this, name, value}; }
	StringProp p(std::string name, Token t) {return StringProp{*this, name, t.getText(code)}; }
	NodeProp p(std::string name, NodePtr node) {return NodeProp{*this, name, node};}
	Start smallStart(NodePtr node, std::string name) {return Start{*this, node, name, true}; }	
	Start smallStart() {return Start{*this, NodePtr(), "", true}; }	
	Start start(NodePtr node, std::string name) {return Start{*this, node, name, false}; }	
	Start start() {return Start{*this, NodePtr(), "", false}; }	
	LStart lstart(std::string name) {return LStart{*this, name, false};}
	LStart smallLStart(std::string name) {return LStart{*this, name, true};}
	
	End smallEnd() {return End{*this};}
	End smallLEnd() {return End{*this};}
	End end() {return End{*this};}
	End lend() {return End{*this};}

	void visit(std::shared_ptr<VariableExp> node) {
		*s << smallStart(node, "var")
		   << p("name", node->nameToken)
		   << smallEnd();
	}
	
    void visit(std::shared_ptr<AssignmentExp> node) {
		*s << start(node, "assignment")
		   << p("name", node->nameToken)
		   << p("value", node->valueExp)
		   << end();
	}

    void visit(std::shared_ptr<IfExp> node) {
		*s << start(node, "if")
		   << lstart("choices");
		for (auto choice: node->choices)
			*s << start()
			   << p("condition", choice->condition)
			   << p("value", choice->value)
			   << end();
		*s << lend() << end();
	}
	
    void visit(std::shared_ptr<ForallExp> node) {
		*s << start(node, "forall")
		   << lstart("names");

		for (auto name: node->names) {
			if (stack.back().first) stack.back().first=false;
			else *s << ", ";
			*s << name.getText(code);
		}
		
		*s << lend();
		*s << lstart("relations");
        visitAll(node->listExps);
		*s << lend();
		*s << p("function", node->exp);
		*s << end();
	}

    void visit(std::shared_ptr<FuncExp> node) {
		*s << start(node, "func")
		   << lstart("args");
		for (auto arg: node->args) {
			*s << smallStart()
			   << p("name", arg->nameToken)
			   << p("type", arg->typeToken)
			   << smallEnd();
		}
		*s << lend()
		   << p("returnType", node->returnTypeToken)
		   << p("body", node->body)
		   << end();
	}
	
    void visit(std::shared_ptr<TupExp> node) {
		*s << start(node, "tup") 
		   << lstart("items");
		for (auto item: node->items) 
			*s << start()
			   << p("name", item->nameToken)
			   << p("exp", item->exp)
			   << end();
		*s << lend() << end();
	}
	
    void visit(std::shared_ptr<BlockExp> node) {
		*s << start(node, "block")
		   << lstart("vals");
        for (auto val: node->vals) 
			*s << start() << p("name", val->nameToken) << p("value", val->exp) << end();
		*s << lend() << p("in", node->inExp) << end();
	}

    void visit(std::shared_ptr<BuiltInExp> node) {
		*s << start(node, "buildin")
		   << p("name", node->nameToken)
		   << lstart("args");
		for (auto arg: node->args)
			visitNode(arg);
		*s << lend() << end();
	}

    void visit(std::shared_ptr<ConstantExp> node) {
		*s << smallStart(node, "const") << p("value", node->valueToken) << smallEnd();
	}

    void visit(std::shared_ptr<UnaryOpExp> node) {
		*s << start(node, "unary")
		   << p("op", node->opToken)
		   << p("exp", node->exp)
		   << end();
	}

    void visit(std::shared_ptr<RelExp> node) {
		*s << start(node, "rel") << p("exp", node->exp) << end();
	}

    void visit(std::shared_ptr<LenExp> node) {
		*s << start(node, "len") << p("exp", node->exp) << end();
	}

    void visit(std::shared_ptr<FuncInvocationExp> node) {
		*s << start(node, "call")
		   << lstart("args");
		for (auto arg: node->args)
			visitNode(arg);
		*s << lend() << end();
	}

    void visit(std::shared_ptr<SubstringExp> node) {
		*s << start(node, "substr") << p("str", node->stringExp) << p("from", node->fromExp) << p("to", node->toExp) << end();
	}

    void visit(std::shared_ptr<RenameExp> node) {
		*s << start(node, "rename")
		   << p("rel", node->lhs)
		   << lstart("renames");
		for (auto item: node->renames)
			*s << smallStart() << p("from", item->fromNameToken) << p("to", item->toNameToken);
		*s << lend() << end();
	}
        
    void visit(std::shared_ptr<DotExp> node) {
		*s << start(node, "lookup") << p("tup", node->lhs) << p("name", node->nameToken) << end();
	}

    void visit(std::shared_ptr<TupMinus> node) {
		*s << start(node, "remove") << p("tup", node->lhs) << p("name", node->nameToken) << end();
	}

    void visit(std::shared_ptr<ProjectExp> node) {
		*s << start(node, "project")
		   << p("op", node->projectionToken)
		   << p("rel", node->lhs)
		   << smallLStart("names");
		for (auto name: node->names) {
			if (stack.back().first) stack.back().first=false;
			else *s << ", ";
			*s << name.getText(code);
		}
		*s << smallLEnd() << end();
	}

    void visit(std::shared_ptr<BinaryOpExp> node) {
		*s << start(node, "binop") 
		   << p("op", node->opToken)
		   << p("lhs", node->lhs) << p("rhs", node->rhs) << end();
	}

    void visit(std::shared_ptr<SequenceExp> node) {
		*s << start(node, "sequence") << lstart("exps");
		for (auto e: node->sequence)
			visitNode(e);
		*s << lend() << end();
	}

    void visit(std::shared_ptr<InvalidExp> node) {
		*s << smallStart(node, "invalid") << smallEnd();
	}

    void visit(std::shared_ptr<AtExp> node) {
		*s << start(node, "at") << p("atToken", node->atToken) << end();
	}

	void visit(std::shared_ptr<Choice>) {}
	void visit(std::shared_ptr<FuncCaptureValue>) {}
	void visit(std::shared_ptr<FuncArg>) {}
	void visit(std::shared_ptr<TupItem>) {}
	void visit(std::shared_ptr<Val>) {}
	void visit(std::shared_ptr<RenameItem>) {}

	virtual void run(NodePtr node, std::ostream & os) {
		stack.push_back(StackItem{0, true, false, false});
		s=&os;
		visitNode(node);
		s=nullptr;
		os << std::endl;
		stack.clear();
	}
};

} //unnamed namespace

namespace rasmus {
namespace frontend {

std::shared_ptr<AstPrinter> makeAstPrinter(std::shared_ptr<Code> code) {
	return std::make_shared<ASTPrinterImpl>(code);
}

} //namespace frontend
} //namespace rasmus

