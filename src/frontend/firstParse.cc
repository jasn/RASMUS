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
#include "tokenizer.hh"
#include <frontend/AST.hh>
#include "code.hh"
#include "error.hh"
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <frontend/visitor.hh>
#include <frontend/firstParse.hh>

using lexer::TokenType;

namespace {
using namespace rasmus::frontend;

template <typename T>
struct Reversed {
	T & list;
	Reversed(T & list): list(list) {}
	
	typedef typename T::reverse_iterator iterator;
	typedef typename T::const_reverse_iterator const_iterator;
	iterator begin() {return list.rbegin();}
	const_iterator begin() const {return list.rbegin();}
	iterator end() {return list.rend();}
	const_iterator end() const {return list.rend();}
};

template <typename T>
Reversed<T> reversed(T & list) {return Reversed<T>(list);}


struct Scope {
	NodePtr node;
	std::unordered_map<std::string, NodePtr> bind;
	Scope() {}
	Scope(NodePtr node): node(node) {}
};

class FirstParseImpl: public FirstParse, public VisitorCRTP<FirstParseImpl, void> {
public:
	//Do name lookup, type checking and constant propagation
	std::shared_ptr<Error> error;
	std::shared_ptr<Code> code;
	std::vector<Scope> scopes;
	std::shared_ptr<Callback> callback;
	GlobalId globalId;

	FirstParseImpl(std::shared_ptr<Error> error, std::shared_ptr<Code> code, std::shared_ptr<Callback> callback):
		error(error), code(code), callback(callback) {
		scopes.push_back(Scope());
		globalId=0;
	}
	
    void internalError(Token token, std::string message) {
        error->reportError(std::string("Internal error: ")+message, token);
	}
        
	bool typeCheck(Token token, NodePtr expr, const std::vector<Type> & t) {
		if (expr->type == TInvalid || expr->type == TAny) 
			return true;
		for(Type i: t) 
			if (i == TAny || i == expr->type)
				return true;

		std::stringstream ss;
		if (t.size() > 1) 
			ss << "Expected one of ";
		else
			ss << "Expected type ";
		bool first=true;
		for(Type x: t) {
			if (first) first=false;
			else ss << ", ";
			ss << x;
		}
		ss << " but found " << expr->type;
		error->reportError(ss.str(), token, {expr->charRange});
		return false;
	}
	
	bool typeMatch(Token token, NodePtr e1, NodePtr e2, 
				   const std::vector<Type> & possibleTypes={TAny}) {
		bool leftOk = typeCheck(token, e1, possibleTypes);
		bool rightOk = typeCheck(token, e2, possibleTypes);
		if (!leftOk || !rightOk) 
			return false;
		
		if (e1->type == e2->type || e1->type == TAny || e2->type == TAny)
			return true;

		std::stringstream ss;
		ss << "Expected identical types but found " << e1->type << " and " << e2->type;
		error->reportError(ss.str(), token, {e1->charRange, e2->charRange});
		return false;
	}

	Type tokenToType(Token token) {
		switch(token.id) {
		case TokenType::TK_TYPE_ANY: return TAny;
		case TokenType::TK_TYPE_ATOM: return TAtom;
        case TokenType::TK_TYPE_BOOL: return TBool;
		case TokenType::TK_TYPE_FUNC: return TFunc;
		case TokenType::TK_TYPE_INT: return TInt;
		case TokenType::TK_TYPE_REL: return TRel;
		case TokenType::TK_TYPE_TEXT: return TText;
		case TokenType::TK_TYPE_TUP: return TTup;
		default:
			internalError(token, "Invalid call to tokenToType");
			return TAny;
		}
	}

    void visit(std::shared_ptr<VariableExp> node) {
        NodePtr lookedUp;
		std::string name = node->nameToken.getText(code);
		std::vector<Scope *> funcs;
        for (auto & lu: reversed(scopes)) {
			auto it=lu.bind.find(name);
			if (it != lu.bind.end())  {
				lookedUp = it->second;
				break;
			}
			if (lu.node && lu.node->nodeType == NodeType::FuncExp)
				funcs.push_back(&lu);
		}

		// If we cannot find the variable, then it must be an external relation
		if (!lookedUp) {
			if (callback->hasRelation(name.c_str())) {
				node->type = TRel;
				return;
			} else {
				std::stringstream ss;
				ss << "Unknown variable " << name;
				error->reportError(ss.str(), node->nameToken);
				return;
			}
		}
		
		for (auto lu: reversed(funcs)) {
			std::shared_ptr<FuncCaptureValue> cap = 
				std::make_shared<FuncCaptureValue>(node->nameToken); 
			cap->type = lookedUp->type;
			cap->store = lookedUp;
			lu->bind[name] = cap;
			lu->bind[name] = cap;
			std::static_pointer_cast<FuncExp>(lu->node)->captures.push_back(cap);
			lookedUp = cap;
		}

        node->type = lookedUp->type;  
		node->store = lookedUp;
	}

    void visit(std::shared_ptr<AssignmentExp> node) {
        visitNode(node->valueExp);
		// Possibly check that the type was not changes since the last binding of the same name
		if (!(scopes.back().node)) 
			node->globalId = globalId++;
		scopes.back().bind[node->nameToken.getText(code)] = node;
        node->type = node->valueExp->type;
	}

    void visit(std::shared_ptr<IfExp> node) {
        for (auto choice: node->choices) {
            visitNode(choice->condition);
			visitNode(choice->value);
		}
				
        NodePtr texp;
		for (auto choice: node->choices) {
            typeCheck(choice->arrowToken, choice->condition, {TBool});
            if (!texp && choice->value->type != TInvalid) 
				texp = choice->value;
		}
        if (texp) {
            bool good=true;
            for (auto choice: node->choices) {
                if (!typeMatch(choice->arrowToken, texp, choice->value))
                    good=false;
			}
			if (good) node->type = texp->type;
		}
	}
	
    void visit(std::shared_ptr<ForallExp> node) {		
		visitAll(node->listExps);
		for(auto exp : node->listExps){
			typeCheck(node->typeToken, exp, {TRel});
		}
		visitNode(node->exp);
		typeCheck(node->colonToken, node->exp, {TFunc});
		node->type = TRel;

	}

    void visit(std::shared_ptr<FuncExp> node) {
        scopes.push_back(Scope(node));
        node->type = TFunc;
        for (auto a: node->args) {
			scopes.back().bind[a->nameToken.getText(code)] = a;
			a->type = tokenToType(a->typeToken);
		}
        node->rtype = tokenToType(node->returnTypeToken);
        visitNode(node->body);
        typeCheck(node->funcToken, node->body, {node->rtype});
		scopes.pop_back();
	}

    void visit(std::shared_ptr<TupExp> node) {
        for (auto item: node->items) visitNode(item->exp);
        node->type = TTup;
	}

    void visit(std::shared_ptr<BlockExp> node) {
        scopes.push_back(Scope(node));
        for (auto val: node->vals) {
            visitNode(val->exp);
            scopes.back().bind[val->nameToken.getText(code)] = val->exp;
		}
        visitNode(node->inExp);
        node->type = node->inExp->type;
		scopes.pop_back();
	}

    void visit(std::shared_ptr<BuiltInExp> node) {
		Type returnType=TInvalid;
		std::vector<std::vector<Type> > argumentTypes;
		switch (node->nameToken.id) {
		case TokenType::TK_ISATOM:
		case TokenType::TK_ISTUP:
		case TokenType::TK_ISREL:
		case TokenType::TK_ISFUNC:
		case TokenType::TK_ISANY:
            returnType = TBool;
            argumentTypes.push_back({TAny});
			break;
        case TokenType::TK_ISBOOL:
		case TokenType::TK_ISINT:
		case TokenType::TK_ISTEXT:
            returnType = TBool;
			argumentTypes.push_back({TAny});
            if (node->args.size() >= 2) argumentTypes.push_back({TNAMEQ});
			break;
        case TokenType::TK_SYSTEM:
            returnType = TInt;
            argumentTypes.push_back({TText});
			break;
        case TokenType::TK_OPEN:
		case TokenType::TK_WRITE:
            returnType = TBool;
            argumentTypes.push_back({TText});
			break;
        case TokenType::TK_CLOSE:
            returnType = TBool;
			break;
        case TokenType::TK_HAS:
            returnType = TBool;
            argumentTypes.push_back({TTup, TRel});
			argumentTypes.push_back({TNAMEQ});
			break;
		case TokenType::TK_MAX:
		case TokenType::TK_MIN:
		case TokenType::TK_COUNT:
		case TokenType::TK_ADD:
		case TokenType::TK_MULT:
            returnType = TInt;
			argumentTypes.push_back({TRel});
			argumentTypes.push_back({TNAMEQ});
			break;
		case TokenType::TK_DAYS:
            returnType = TInt;
            argumentTypes.push_back({TText});
			argumentTypes.push_back({TText});
			break;
		case TokenType::TK_BEFORE:
		case TokenType::TK_AFTER:
            returnType = TText;
            argumentTypes.push_back({TText});
			argumentTypes.push_back({TText});
			break;
		case TokenType::TK_DATE:
            returnType = TText;
            argumentTypes.push_back({TText});
			argumentTypes.push_back({TInt});
			break;
        case TokenType::TK_TODAY:
            returnType = TText;
			break;
		case TokenType::TK_PRINT:
            returnType = TBool;
            argumentTypes.push_back({TAny});
			break;
		default:
            error->reportError("Unknown buildin", node->nameToken, {node->charRange});
			break;
		}
		
        node->type=returnType;
        if (node->args.size() != argumentTypes.size()) {
			// too few args
			std::stringstream ss;
			ss << "Too "
			   << (node->args.size() < argumentTypes.size()?"few":"many")
			   << " arguments to builtin function, received " << node->args.size() 
			   << " but expected " << argumentTypes.size();
			error->reportError(ss.str(), node->nameToken, {node->charRange});
		}
		
		
		for (size_t i=0; i < node->args.size(); ++i) {
            if (i >= argumentTypes.size() || argumentTypes[i] != std::vector<Type>{TNAMEQ})
                visitNode(node->args[i]);
            if (i < argumentTypes.size() && argumentTypes[i] != std::vector<Type>{TNAMEQ})
                typeCheck(node->nameToken, node->args[i], argumentTypes[i]);
            if (i < argumentTypes.size() && argumentTypes[i] == std::vector<Type>{TNAMEQ}) {
				if (node->args[i]->nodeType != NodeType::VariableExp) 
					error->reportError("Expected identifier", Token(), {node->args[i]->charRange});
			}
		}
	}

    void visit(std::shared_ptr<ConstantExp> node) {
		switch (node->valueToken.id) {
		case TokenType::TK_FALSE:
        case TokenType::TK_TRUE:
        case TokenType::TK_STDBOOL:
            node->type = TBool;
			break;
		case TokenType::TK_TEXT:
        case TokenType::TK_STDTEXT:
			node->type = TText;
			break;
        case TokenType::TK_ZERO:
        case TokenType::TK_ONE:
			node->type = TRel;
			break;
        case TokenType::TK_STDINT:
			node->type = TInt;
			break;
		case TokenType::TK_BADINT:
			error->reportWarning(
				"Integers should not start with 0 (we do not support octals)",
				node->valueToken);
			node->type = TInt;
			break;
		case TokenType::TK_INT:
			//TODO Validate the integer and check its range
            //atoi(code->code.substr(node->valueToken.start, node->valueToken.length).c_str());
			node->type = TInt;
			break;
		default:
			internalError(node->valueToken, std::string("Invalid constant type ")+getTokenName(node->valueToken.id));
            node->type = TInvalid;
			break;
		}
	}   

    void visit(std::shared_ptr<UnaryOpExp> node) {
        visitNode(node->exp);
		switch (node->opToken.id) {
		case TokenType::TK_NOT:
			typeCheck(node->opToken, node->exp, {TBool});
			node->type = TBool;
			break;
		case TokenType::TK_MINUS:
            typeCheck(node->opToken, node->exp, {TInt});
            node->type = TInt;
			break;
		default:
            internalError(node->opToken, "Bad unary operator");
            node->type = TInvalid;
		}
	}

    void visit(std::shared_ptr<RelExp> node) {
        node->type = TRel;
        visitNode(node->exp);
        typeCheck(node->relToken, node->exp, {TTup});
	}
	
    void visit(std::shared_ptr<LenExp> node) {
        visitNode(node->exp);
		typeCheck(node->leftPipeToken, node->exp, {TText, TRel, TTup});
        node->type = TInt;
	}

    void visit(std::shared_ptr<FuncInvocationExp> node) {
        visitNode(node->funcExp);
        visitAll(node->args);
        node->type = TAny;
        typeCheck(node->lparenToken, node->funcExp, {TFunc});
	}

    void visit(std::shared_ptr<SubstringExp> node) {
        visitNode(node->stringExp);
        visitNode(node->fromExp);
        visitNode(node->toExp);
		typeCheck(node->lparenToken, node->stringExp, {TText});
		typeCheck(node->lparenToken, node->fromExp, {TInt});
		typeCheck(node->lparenToken, node->toExp, {TInt});
        node->type = TText;
	}

    void visit(std::shared_ptr<RenameExp> node) {
        visitNode(node->lhs);
        typeCheck(node->lbracketToken, node->lhs, {TRel});
        node->type = TRel;
	}

    void visit(std::shared_ptr<DotExp> node) {
        visitNode(node->lhs);
        typeCheck(node->dotToken, node->lhs, {TTup});
        node->type = TAny;
	}

    void visit(std::shared_ptr<TupMinus> node) {
        visitNode(node->lhs);
        typeCheck(node->opToken, node->lhs, {TTup});
        node->type = TTup;
	}

    void visit(std::shared_ptr<ProjectExp> node) {
        visitNode(node->lhs);
        typeCheck(node->projectionToken, node->lhs, {TRel});
        node->type = TRel;
	}

    void visit(std::shared_ptr<InvalidExp> node) {
        node->type = TInvalid;
	}

	struct BinopHelp {
		::Type lhsType;
		::Type rhsType;
		::Type resType;
	};


	void binopTypeCheck(std::shared_ptr<BinaryOpExp> node,
						std::initializer_list<BinopHelp> ops) {
        visitNode(node->lhs);
        visitNode(node->rhs);
		::Type lhst=node->lhs->type;
		::Type rhst=node->rhs->type;
		
		std::vector<BinopHelp> matches;
		for(auto h: ops) {
			if (h.lhsType != lhst && lhst != TAny && lhst != TInvalid) continue;
			if (h.rhsType != rhst && rhst != TAny && rhst != TInvalid) continue;
			matches.push_back(h);
		}
		
		if (matches.size() == 0) {
			if (ops.size() == 1) {
				typeCheck(node->opToken, node->lhs, {ops.begin()->lhsType});
				typeCheck(node->opToken, node->rhs, {ops.begin()->rhsType});
			} else {
				bool allMatch = true;
				std::vector<Type> matchTypes;
				for(auto h: ops) {
					if (h.lhsType != h.rhsType) {
						allMatch=false;
						break;
					}
					matchTypes.push_back(h.lhsType);
				}
				if (allMatch) 
					typeMatch(node->opToken, node->lhs, node->rhs, matchTypes);
				else {
					std::stringstream ss;
					ss << "Invalid operator use, the types (" << lhst << ", " << rhst
					   << ") does not match any of ";
					bool first=true;
					for(auto h: ops) {
						if (first) first=false;
						else ss << ", ";
						ss << "(" << h.lhsType << ", " << h.rhsType << ")";
					}
					error->reportError(ss.str(), node->opToken, {node->lhs->charRange, node->rhs->charRange});
				}
			}
			return;
		}

		::Type rtype=matches[0].resType;
		for (auto h: matches) {
			if (h.resType == rtype) continue;
			rtype = TAny;
		}
		node->type = rtype;
	}

	
    void visit(std::shared_ptr<BinaryOpExp> node) {
		switch(node->opToken.id) {
		case TokenType::TK_PLUS:
		case TokenType::TK_MUL:
		case TokenType::TK_MINUS:
			binopTypeCheck(node, {
					{TInt, TInt, TInt},
					{TRel, TRel, TRel}
				});
			break;
		case TokenType::TK_DIV:
		case TokenType::TK_MOD:
			binopTypeCheck(node, { {TInt, TInt, TInt} });
			break;
		case TokenType::TK_AND:
		case TokenType::TK_OR:
			binopTypeCheck(node, { {TBool, TBool, TBool} });
			break;
        case TokenType::TK_CONCAT:
			binopTypeCheck(node, { {TText, TText, TText} });
			break;
		case TokenType::TK_LESSEQUAL:
		case TokenType::TK_LESS:
		case TokenType::TK_GREATER:
		case TokenType::TK_GREATEREQUAL:
			binopTypeCheck(node, { 
					{TInt, TInt, TBool},
					{TBool, TBool, TBool} });
			break;
		case TokenType::TK_EQUAL:
		case TokenType::TK_DIFFERENT:
			binopTypeCheck(node, { 
					{TInt, TInt, TBool},
					{TBool, TBool, TBool},
					{TText, TText, TBool},
					{TFunc, TFunc, TBool},
					{TTup, TTup, TBool},
					{TRel, TRel, TBool}});
			break;
		case TokenType::TK_TILDE:
			binopTypeCheck(node, { {TText, TText, TBool} });
			break;
		case TokenType::TK_QUESTION:
			binopTypeCheck(node, { {TRel, TFunc, TRel} });
			break;
		case TokenType::TK_OPEXTEND:
			binopTypeCheck(node, { {TTup, TTup, TTup} });
			break;
		default:
            internalError(node->opToken, std::string("Invalid operator")+getTokenName(node->opToken.id));
            node->type = TInvalid;
			break;
		}
	}

    void visit(std::shared_ptr<SequenceExp> node) {
        visitAll(node->sequence);
        if (node->sequence.empty())
			node->type = TInvalid;
        else
            node->type = node->sequence.back()->type;
	}
    

	void visit(std::shared_ptr<Choice>) {ICE("Choice");}
	void visit(std::shared_ptr<FuncCaptureValue>) {ICE("FCV");}
	void visit(std::shared_ptr<FuncArg>) {ICE("FuncArg");}
	void visit(std::shared_ptr<TupItem>) {ICE("TupItem");}
	void visit(std::shared_ptr<Val>) {ICE("Val");}
	void visit(std::shared_ptr<RenameItem>) {ICE("RenameItem");}
	void visit(std::shared_ptr<AtExp>) {ICE("AtExp");}

	virtual void run(NodePtr node) override {
		visitNode(node);
	}

};

} //nameless namespace

namespace rasmus {
namespace frontend {
        
std::shared_ptr<FirstParse> makeFirstParse(std::shared_ptr<Error> error, 
										   std::shared_ptr<Code> code,
										   std::shared_ptr<Callback> callback) {
	return std::make_shared<FirstParseImpl>(error, code, callback);
}

} //namespace rasmus
} //namespace frontend
