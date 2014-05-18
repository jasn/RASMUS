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
#include "lexer.hh"
#include "AST.hh"

CharRange r(Token t) {
	if (!t) return CharRange();
	return CharRange(t.start, t.start+t.length);
}

CharRange u(CharRange r1, CharRange r2) {
	return CharRange(std::min(r1.lo, r2.lo), std::max(r1.hi, r2.hi));
}

class CharRanges: public Visitor {
public:
	void visit(std::shared_ptr<VariableExp> node) override {
		node->charRange = r(node->nameToken);
	}
	
    void visit(std::shared_ptr<AssignmentExp> node) override {
		Visitor::visit(node->valueExp);
        node->charRange = u(r(node->nameToken), node->valueExp->charRange);
	}

    void visit(std::shared_ptr<IfExp> node) override {
		for (auto choice: node->choices) {
			Visitor::visit(choice->condition);
			Visitor::visit(choice->value);
		}
        node->charRange = u(r(node->ifToken), r(node->fiToken));
	}
			
    void visit(std::shared_ptr<ForallExp> node) override {
        Visitor::visitAll(node->listExps);
        Visitor::visit(node->exp);
        node->charRange = u(r(node->typeToken), node->exp->charRange);
	}

    void visit(std::shared_ptr<FuncExp> node) override {
        for (auto arg: node->args) 
            arg->charRange = u(u(r(arg->nameToken),r(arg->colonToken)), r(arg->typeToken));
		Visitor::visit(node->body);
        node->charRange = u(r(node->funcToken), r(node->endToken));
	}

    void visit(std::shared_ptr<TupExp> node) override {
        for (auto item: node->items) Visitor::visit(item->exp);
        node->charRange = u(r(node->tupToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<BlockExp> node) override {
        for (auto val: node->vals) Visitor::visit(val->exp);
        Visitor::visit(node->inExp);
		node->charRange = u(r(node->blockstartToken), r(node->blockendToken));
	}

    void visit(std::shared_ptr<BuiltInExp> node) override {
        Visitor::visitAll(node->args);
        node->charRange = u(r(node->nameToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<ConstantExp> node) override {
        node->charRange = r(node->valueToken);
	}

    void visit(std::shared_ptr<UnaryOpExp> node) override {
        Visitor::visit(node->exp);
        node->charRange = u(r(node->opToken), node->exp->charRange);
	}

    void visit(std::shared_ptr<RelExp> node) override {
        Visitor::visit(node->exp);
        node->charRange = u(r(node->relToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<LenExp> node) override {
        Visitor::visit(node->exp);
        node->charRange = u(r(node->leftPipeToken), r(node->rightPipeToken));
	}

    void visit(std::shared_ptr<FuncInvocationExp> node) override {
        Visitor::visit(node->funcExp);
        Visitor::visitAll(node->args);
        node->charRange = u(node->funcExp->charRange, r(node->rparenToken));
	}

    void visit(std::shared_ptr<SubstringExp> node) override {
        Visitor::visitAll({node->stringExp, node->fromExp, node->toExp});
        node->charRange = u(node->stringExp->charRange, r(node->rparenToken));
	}

    void visit(std::shared_ptr<RenameExp> node) override {
        Visitor::visit(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->rbracketToken));
	}
        
    void visit(std::shared_ptr<DotExp> node) override {
        Visitor::visit(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->nameToken));
	}

    void visit(std::shared_ptr<ProjectExp> node) override {
        Visitor::visit(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->names[-1]));
	}

    void visit(std::shared_ptr<BinaryOpExp> node) override {
        Visitor::visit(node->lhs);
        Visitor::visit(node->rhs);
        node->charRange = u(node->lhs->charRange, node->rhs->charRange);
	}

    void visit(std::shared_ptr<SequenceExp> node) override {
        Visitor::visitAll(node->sequence);
		if (!node->sequence.empty())
			node->charRange = u(node->sequence.front()->charRange, node->sequence.back()->charRange);
	}

    void visit(std::shared_ptr<InvalidExp> node) override {	}

    void visit(std::shared_ptr<AtExp> node) override {
		Visitor::visit(node->exp);
		node->charRange = u(r(node->atToken), r(node->rparenToken));
	}
	
    void visit(std::shared_ptr<Exp> node) override {
		Visitor::visit(node->exp);
		node->charRange = node->exp->charRange;
	}


	void visit(std::shared_ptr<Choice> node) override {}
	void visit(std::shared_ptr<FuncCaptureValue> node) override {}
	void visit(std::shared_ptr<FuncArg> node) override {}
	void visit(std::shared_ptr<TupItem> node) override {}
	void visit(std::shared_ptr<Val> node) override {}
	void visit(std::shared_ptr<RenameItem> node) override {}
};


std::shared_ptr<Visitor> charRanges() {
	return std::make_shared<CharRanges>();
}
