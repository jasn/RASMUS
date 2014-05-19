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
#include "code.hh"
#include "error.hh"
#include <unordered_map>
#include <sstream>
#include <iostream>
#include "visitor.hh"

namespace {

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

class FirstParse: public Visitor<void>, public VisitorCRTP<FirstParse, void> {
public:
	//Do name lookup, type checking and constant propagation
	std::shared_ptr<Error> error;
	std::shared_ptr<Code> code;

	std::vector<Scope> scopes;


	FirstParse(std::shared_ptr<Error> error, std::shared_ptr<Code> code):
		error(error), code(code) {
		scopes.push_back(Scope());
	}
	
	std::string tokenToIdentifier(Token token) const {
		return code->code.substr(token.start, token.length);
	}
	
    void internalError(Token token, std::string message) {
        error->reportError(std::string("Internal error: ")+message, token);
	}
        
	bool typeCheck(Token token, NodePtr expr, std::initializer_list<Type> t) {
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
				   std::initializer_list<Type> possibleTypes={TAny}) {
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
		case TK_TYPE_ANY: return TAny;
		case TK_TYPE_ATOM: return TAtom;
        case TK_TYPE_BOOL: return TBool;
		case TK_TYPE_FUNC: return TFunc;
		case TK_TYPE_INT: return TInt;
		case TK_TYPE_REL: return TRel;
		case TK_TYPE_TEXT: return TText;
		case TK_TYPE_TUP: return TTup;
		default:
			internalError(token, "Invalid call to tokenToType");
			return TAny;
		}
	}

    void visit(std::shared_ptr<VariableExp> node) {
        NodePtr lookedUp;
		std::string name = tokenToIdentifier(node->nameToken);
		std::vector<std::shared_ptr<FuncExp> > funcs;
        for (auto & lu: reversed(scopes)) {
			auto it=lu.bind.find(name);
			if (it != lu.bind.end())  {
				lookedUp = it->second;
				break;
			}
			if (lu.node->nodeType == NodeType::FuncExp)
				funcs.push_back(std::static_pointer_cast<FuncExp>(lu.node));
		}

		if (!lookedUp) {
            node->type = TInvalid;
            error->reportError("Name not found in scope", node->nameToken);
            return;
		}

		// TODO Fixme
        // for (auto &lu: reversed(funcs)) {
		// 	std::shared_ptr<FuncCaptureValue> cap = 
		// 		std::make_shared<FuncCaptureValue>(getNameToken(lookedUp)); //TODO FIXME
        //     //cap->name = name;
        //     cap->type = lookedUp->type;
        //     cap->store = lookedUp;
        //     lu.bind[name] = cap;
        //     lu.node->captures.append(cap);
        //     lookedUp = cap;
		// }

        node->type = lookedUp->type;  
		node->store = lookedUp;
	}

    void visit(std::shared_ptr<AssignmentExp> node) {
        visitNode(node->valueExp);
		// Possibly check that the type was not changes since the last binding of the same name
		scopes.back().bind[tokenToIdentifier(node->nameToken)] = node;
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
		//TODO
		visitAll(node->listExps);
		visitNode(node->exp);
	}

    void visit(std::shared_ptr<FuncExp> node) {
        scopes.push_back(Scope(node));
        node->type = TFunc;
        for (auto a: node->args) {
			// a->name = tokenToIdentifier(a->nameToken);
			// self.scopes.back().bind[a->name] = a;
			scopes.back().bind[tokenToIdentifier(a->nameToken)] = a;
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
            scopes.back().bind[tokenToIdentifier(val->nameToken)] = val->exp;
		}
        visitNode(node->inExp);
        node->type = node->inExp->type;
		scopes.pop_back();
	}

    void visit(std::shared_ptr<BuiltInExp> node) {
		Type returnType;
		std::vector<Type> argumentTypes;
        TokenId tkn = node->nameToken.id;
		switch (node->nameToken.id) {
		case TK_ISATOM:
		case TK_ISTUP:
		case TK_ISREL:
		case TK_ISFUNC:
		case TK_ISANY:
            returnType = TBool;
            argumentTypes.push_back(TAny);
			break;
        case TK_ISBOOL:
		case TK_ISINT:
		case TK_ISTEXT:
            returnType = TBool;
			argumentTypes.push_back(TAny);
            if (node->args.size() >= 2) argumentTypes.push_back(TNAMEQ);
			break;
        case TK_SYSTEM:
            returnType = TInt;
            argumentTypes.push_back(TText);
			break;
        case TK_OPEN:
		case TK_WRITE:
            returnType = TBool;
            argumentTypes.push_back(TText);
			break;
        case TK_CLOSE:
            returnType = TBool;
			break;
        case TK_HAS:
            returnType = TBool;
            argumentTypes.push_back(TRel);
			argumentTypes.push_back(TNAMEQ);
			break;
		case TK_MAX:
		case TK_MIN:
		case TK_COUNT:
		case TK_ADD:
		case TK_MULT:
            returnType = TInt;
			argumentTypes.push_back(TRel);
			argumentTypes.push_back(TNAMEQ);
			break;
		case TK_DAYS:
            returnType = TInt;
            argumentTypes.push_back(TText);
			argumentTypes.push_back(TText);
			break;
		case TK_BEFORE:
		case TK_AFTER:
            returnType = TText;
            argumentTypes.push_back(TText);
			argumentTypes.push_back(TText);
			break;
		case TK_DATE:
            returnType = TText;
            argumentTypes.push_back(TText);
			argumentTypes.push_back(TInt);
			break;
        case TK_TODAY:
            returnType = TText;
			break;
		case TK_PRINT:
            returnType = TBool;
            argumentTypes.push_back(TAny);
			break;
		default:
            error->reportError("Unknown buildin", node->nameToken, {node->charRange});
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
            if (i >= argumentTypes.size() || argumentTypes[i] != TNAMEQ)
                visitNode(node->args[i]);
            if (i < argumentTypes.size() && argumentTypes[i] != TNAMEQ)
                typeCheck(node->nameToken, node->args[i], {argumentTypes[i]});
            if (i < argumentTypes.size() && argumentTypes[i] == TNAMEQ) {
                // if (not isinstance(node->args[i], VariableExp) 
                //     && not(isinstance(node->args[i], Exp) 
                //             && isinstance(node->args[i].exp, VariableExp))):
                //     err.reportError("Expected identifier", None, [node->args[i].charRange])
				// 		}
				//TODO FIX ME
			}
		}
	}

    void visit(std::shared_ptr<ConstantExp> node) {
		switch (node->valueToken.id) {
		case TK_FALSE:
            node->bool_value = false;
            node->type = TBool;
			break;
        case TK_TRUE:
            node->bool_value = true;
            node->type = TBool;
			break;
        case TK_INT:
			//TODO
            node->int_value = atoi(code->code.substr(node->valueToken.start, node->valueToken.length).c_str());
			node->type = TInt;
			break;
		case TK_TEXT:
			node->txt_value = code->code.substr(node->valueToken.start+1, node->valueToken.length-2);
            node->type = TText;
			break;
        case TK_ZERO:
			node->type = TRel;
			//  dunno what to do here.
            // it needs to be the 'empty relation'
            break;
        case TK_ONE:
            node->type = TRel;
            // dunno what to do here.
            // it needs to be 
            // 'the relation with the empty Schema that has exactly one tuple which is empty'
            break;
        case TK_STDBOOL:
			// TODO
            node->type = TBool;
			break;
        case TK_STDINT:
			// TODO
            node->type = TInt;
			break;
        case TK_STDTEXT:
			// TODO
            node->type = TText;
			break;
		default:
			internalError(node->valueToken, std::string("Invalid constant type ")+getTokenName(node->valueToken.id));
            node->type = TInvalid;
		}
	}   

    void visit(std::shared_ptr<UnaryOpExp> node) {
        visitNode(node->exp);
		switch (node->opToken.id) {
		case TK_NOT:
			typeCheck(node->opToken, node->exp, {TBool});
			node->type = TBool;
			break;
		case TK_MINUS:
            typeCheck(node->opToken, node->exp, {TInt});
            node->type = TInt;
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
		typeCheck(node->leftPipeToken, node->exp, {TText, TRel});
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

    void visit(std::shared_ptr<ProjectExp> node) {
        visitNode(node->lhs);
        typeCheck(node->projectionToken, node->lhs, {TRel});
        node->type = TRel;
	}

    void visit(std::shared_ptr<InvalidExp> node) {
        node->type = TInvalid;
	}
	
    void visit(std::shared_ptr<BinaryOpExp> node) {
        visitNode(node->lhs);
        visitNode(node->rhs);
		switch(node->opToken.id) {
		case TK_PLUS:
		case TK_MUL:
		case TK_MINUS:
            typeMatch(node->opToken, node->lhs, node->rhs, {TRel, TInt});
			if (node->lhs->type == TInt && node->rhs->type != TRel)
                node->type = TInt;
			else if (node->lhs->type != TRel && node->rhs->type == TInt)
                node->type = TInt;
            else if (node->lhs->type == TRel && node->rhs->type != TInt)
                node->type = TRel;
            else if (node->lhs->type != TInt && node->rhs->type == TRel)
                node->type = TRel;
			else if (node->lhs->type == TAny && node->rhs->type == TAny)
				node->type = TAny;
			break;
		case TK_DIV:
		case TK_MOD:
            typeCheck(node->opToken, node->lhs, {TInt});
			typeCheck(node->opToken, node->rhs, {TInt});
            node->type = TInt;
			break;
		case TK_AND:
		case TK_OR:
            typeCheck(node->opToken, node->lhs, {TBool});
			typeCheck(node->opToken, node->rhs, {TBool});
            node->type = TBool;
			break;
        case TK_CONCAT:
            typeCheck(node->opToken, node->lhs, {TText});
			typeCheck(node->opToken, node->rhs, {TText});
            node->type = TText;
			break;
		case TK_LESSEQUAL:
		case TK_LESS:
		case TK_GREATER:
		case TK_GREATEREQUAL:
            typeCheck(node->opToken, node->lhs, {TInt});
			typeCheck(node->opToken, node->rhs, {TInt});
            node->type = TBool;
			break;
		case TK_EQUAL:
		case TK_DIFFERENT:
            // informal semantics page 17-18: 
            // Values of the same type can be compared && the result of a comparison
            // is a boolean.
            // 
            // Any two values can be compared using = or <>.
            typeMatch(node->opToken, node->lhs, node->rhs, {TAny});
			node->type = TBool;
			break;
		case TK_TILDE:
			node->type = TBool;
			typeMatch(node->opToken, node->lhs, node->rhs, {TText});
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
    
    void visit(std::shared_ptr<Exp> node) {
        scopes.push_back(Scope(node));
        visitNode(node->exp);
        scopes.pop_back();
        node->type = node->exp->type;
	}

	void visit(std::shared_ptr<Choice> node) {}
	void visit(std::shared_ptr<FuncCaptureValue> node) {}
	void visit(std::shared_ptr<FuncArg> node) {}
	void visit(std::shared_ptr<TupItem> node) {}
	void visit(std::shared_ptr<Val> node) {}
	void visit(std::shared_ptr<RenameItem> node) {}

	void visit(std::shared_ptr<AtExp> node) {/*TODO*/	}

	virtual void run(NodePtr node) override {visitNode(node);}

};

} //nameless namespace
        
std::shared_ptr<Visitor<void> > firstParse(std::shared_ptr<Error> error, std::shared_ptr<Code> code) {
	return std::make_shared<FirstParse>(error, code);
}
