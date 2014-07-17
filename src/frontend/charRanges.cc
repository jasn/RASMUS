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
#include <frontend/lexer.hh>
#include <frontend/AST.hh>
#include <frontend/visitor.hh>
#include <frontend/charRanges.hh>
namespace {
using namespace rasmus::frontend;

CharRange r(Token t) {
	if (!t || t.length == 0) return CharRange();
	return CharRange(t.start, t.start+t.length);
}

CharRange u(CharRange r1, CharRange r2) {
	return CharRange(std::min(r1.lo, r2.lo), std::max(r1.hi, r2.hi));
}

class CharRangesImpl: public CharRanges, public VisitorCRTP<CharRangesImpl, void> {
public:
	void visit(std::shared_ptr<VariableExp> node) {
		node->charRange = r(node->nameToken);
	}
	
    void visit(std::shared_ptr<AssignmentExp> node) {
		visitNode(node->valueExp);
        node->charRange = u(r(node->nameToken), node->valueExp->charRange);
	}

    void visit(std::shared_ptr<IfExp> node) {
		for (auto choice: node->choices) {
			visitNode(choice->condition);
			visitNode(choice->value);
		}
        node->charRange = u(r(node->ifToken), r(node->fiToken));
	}
			
    void visit(std::shared_ptr<ForallExp> node) {
        visitAll(node->listExps);
        visitNode(node->exp);
        node->charRange = u(r(node->typeToken), node->exp->charRange);
	}

    void visit(std::shared_ptr<FuncExp> node) {
        for (auto arg: node->args) 
            arg->charRange = u(u(r(arg->nameToken),r(arg->colonToken)), r(arg->typeToken));
		visitNode(node->body);
        node->charRange = u(r(node->funcToken), r(node->endToken));
	}

    void visit(std::shared_ptr<TupExp> node) {
        for (auto item: node->items) visitNode(item->exp);
        node->charRange = u(r(node->tupToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<BlockExp> node) {
        for (auto val: node->vals) visitNode(val->exp);
        visitNode(node->inExp);
		node->charRange = u(r(node->blockstartToken), r(node->blockendToken));
	}

    void visit(std::shared_ptr<BuiltInExp> node) {
        visitAll(node->args);
        node->charRange = u(r(node->nameToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<ConstantExp> node) {
        node->charRange = r(node->valueToken);
	}

    void visit(std::shared_ptr<UnaryOpExp> node) {
        visitNode(node->exp);
        node->charRange = u(r(node->opToken), node->exp->charRange);
	}

    void visit(std::shared_ptr<RelExp> node) {
        visitNode(node->exp);
        node->charRange = u(r(node->relToken), r(node->rparenToken));
	}

    void visit(std::shared_ptr<LenExp> node) {
        visitNode(node->exp);
        node->charRange = u(r(node->leftPipeToken), r(node->rightPipeToken));
	}

    void visit(std::shared_ptr<FuncInvocationExp> node) {
        visitNode(node->funcExp);
        visitAll(node->args);
        node->charRange = u(node->funcExp->charRange, r(node->rparenToken));
	}

    void visit(std::shared_ptr<SubstringExp> node) {
        visitAll({node->stringExp, node->fromExp, node->toExp});
        node->charRange = u(node->stringExp->charRange, r(node->rparenToken));
	}

    void visit(std::shared_ptr<RenameExp> node) {
        visitNode(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->rbracketToken));
	}
        
    void visit(std::shared_ptr<DotExp> node) {
        visitNode(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->nameToken));
	}

    void visit(std::shared_ptr<TupMinus> node) {
        visitNode(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->nameToken));
	}

    void visit(std::shared_ptr<ProjectExp> node) {
        visitNode(node->lhs);
        node->charRange = u(node->lhs->charRange, r(node->names[-1]));
	}

    void visit(std::shared_ptr<BinaryOpExp> node) {
        visitNode(node->lhs);
        visitNode(node->rhs);
        node->charRange = u(node->lhs->charRange, node->rhs->charRange);
	}

    void visit(std::shared_ptr<SequenceExp> node) {
        visitAll(node->sequence);
		if (!node->sequence.empty())
			node->charRange = u(node->sequence.front()->charRange, node->sequence.back()->charRange);
	}

    void visit(std::shared_ptr<InvalidExp>) {}

    void visit(std::shared_ptr<AtExp> node) {
		node->charRange = r(node->atToken);
	}

	void visit(std::shared_ptr<Choice>) {}
	void visit(std::shared_ptr<FuncCaptureValue>) {}
	void visit(std::shared_ptr<FuncArg>) {}
	void visit(std::shared_ptr<TupItem>) {}
	void visit(std::shared_ptr<Val>) {}
	void visit(std::shared_ptr<RenameItem>) {}

	virtual void run(NodePtr node) override {visitNode(node);}
};

} //nameless namespace

namespace rasmus {
namespace frontend {
std::shared_ptr<CharRanges > makeCharRanges() {
	return std::make_shared<CharRangesImpl>();
}
} //namespace frontend
} //namespace rasmus

