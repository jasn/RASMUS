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
	bool missingIsAny;

	FirstParseImpl(std::shared_ptr<Error> error, 
				   std::shared_ptr<Code> code, 
				   std::shared_ptr<Callback> callback,
				   bool missingIsAny)
		: error(error), code(code)
		, callback(callback), missingIsAny(missingIsAny) {
		scopes.push_back(Scope());
		globalId=0;
	}
	
    void internalError(Token token, std::string message) {
        error->reportError(std::string("Internal error: ")+message, token);
	}
	
	bool typeCheck(Token token, NodePtr expr, const Type & t) {
		if (Type::match(expr->type, t)) 
			return true;

		std::stringstream ss;
		if (t.base() == Type::Disjunction)
			ss << "Expected one of ";
		else
			ss << "Expected type ";
		
		ss << t << " but found " << expr->type;
		error->reportError(ss.str(), token, {expr->charRange});
		return false;
	}

	Type tokenToType(Token token) {
		switch(token.id) {
		case TokenType::TK_TYPE_ANY: return Type::any();
        case TokenType::TK_TYPE_BOOL: return Type::boolean();
		case TokenType::TK_TYPE_FUNC: return Type::aFunc();
		case TokenType::TK_TYPE_INT: return Type::integer();
		case TokenType::TK_TYPE_FLOAT: return Type::fp();
		case TokenType::TK_TYPE_REL: return Type::aRel();
		case TokenType::TK_TYPE_TEXT: return Type::text();
		case TokenType::TK_TYPE_TUP: return Type::aTup();
		default:
			internalError(token, "Invalid call to tokenToType");
			return Type::invalid();
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
			if (missingIsAny) {
				node->type = Type::any();
				return;
			}
			if (callback->hasRelation(name.c_str())) {
				node->type = Type::aRel(); // Todo find the actual relation type
				return;
			} else {
				if (false) ;
				else if (name == "acos") {
					node->buildin = BuildIn::acos;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "asin") {
					node->buildin = BuildIn::asin;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "atan") {
					node->buildin = BuildIn::atan;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "atan2") {
					node->buildin = BuildIn::atan2;
					node->type = Type::func(Type::fp(), {Type::fpOrInt(), Type::fpOrInt()});
				} else if (name == "ceil") {
					node->buildin = BuildIn::ceil;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "cos") {
					node->buildin = BuildIn::cos;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "floor") {
					node->buildin = BuildIn::floor;
					node->type = Type::func(Type::integer(), {Type::fpOrInt()});
				} else if (name == "pow") {
					node->buildin = BuildIn::pow;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "round") {
					node->buildin = BuildIn::round;
					node->type = Type::func(Type::integer(), {Type::fpOrInt()});
				} else if (name == "sin") {
					node->buildin = BuildIn::sin;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "sqrt") {
					node->buildin = BuildIn::sqrt;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else if (name == "tan") {
					node->buildin = BuildIn::tan;
					node->type = Type::func(Type::fp(), {Type::fpOrInt()});
				} else {
					std::stringstream ss;
					ss << "Unknown variable " << name;
					error->reportError(ss.str(), node->nameToken);
					node->type = Type::invalid();
				}
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

		if (scopes.back().node) {
            error->reportError("Assignment is currently only supported in global scope", node->nameToken, {node->charRange});
		}
			

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
				
		std::vector<Type> types;
		for (auto choice: node->choices) {
            typeCheck(choice->arrowToken, choice->condition, Type::boolean());
			types.push_back(choice->value->type);
		}

		node->type = Type::disjunction(types);
	}
	
    void visit(std::shared_ptr<ForallExp> node) {		
		visitAll(node->listExps);
		for(auto exp : node->listExps)
			typeCheck(node->typeToken, exp, Type::aRel());
		visitNode(node->exp);
		// TODO we can do much better typechecking here
		typeCheck(node->colonToken, node->exp, Type::aFunc()); 
		// TODO use the return type of the func in node
		node->type = Type::aRel();
		
	}

    void visit(std::shared_ptr<FuncExp> node) {
        scopes.push_back(Scope(node));
		std::vector<Type> argTypes;
        for (auto a: node->args) {
			scopes.back().bind[a->nameToken.getText(code)] = a;
			a->type = tokenToType(a->typeToken);
			argTypes.push_back(a->type);
		}
		Type rtype=tokenToType(node->returnTypeToken);
        visitNode(node->body);
		typeCheck(node->funcToken, node->body, rtype);
		// TODO make rtype the conjunction of the types of rtype and node->body type
				
		node->type = Type::func(rtype, argTypes);
		scopes.pop_back();
	}

    void visit(std::shared_ptr<TupExp> node) {
        for (auto item: node->items) visitNode(item->exp);
		// TODO make strong type
        node->type = Type::aTup();
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
		Type returnType=Type::invalid();
		std::vector<Type> argumentTypes;
		bool secondIsName=false;
		switch (node->nameToken.id) {
		case TokenType::TK_ISATOM:
		case TokenType::TK_ISTUP:
		case TokenType::TK_ISREL:
		case TokenType::TK_ISFUNC:
		case TokenType::TK_ISANY:
            returnType = Type::boolean();;
            argumentTypes.push_back(Type::any());
			break;
        case TokenType::TK_ISBOOL:
		case TokenType::TK_ISINT:
		case TokenType::TK_ISFLOAT:
		case TokenType::TK_ISTEXT:
            returnType = Type::boolean();
            argumentTypes.push_back(Type::any());
            if (node->args.size() >= 2) secondIsName = true;
			break;
        case TokenType::TK_SYSTEM:
			returnType = Type::integer();
            argumentTypes.push_back(Type::text());
			break;
        case TokenType::TK_OPEN:
		case TokenType::TK_WRITE:
			returnType = Type::boolean();
            argumentTypes.push_back(Type::text());
			break;
        case TokenType::TK_CLOSE:
			returnType = Type::boolean();
			break;
        case TokenType::TK_HAS:
			returnType = Type::boolean();
            argumentTypes.push_back(Type::disjunction({Type::aRel(), Type::aTup()}));
			secondIsName = true;
			break;
		case TokenType::TK_MAX:
		case TokenType::TK_MIN:
		case TokenType::TK_COUNT:
		case TokenType::TK_ADD:
		case TokenType::TK_MULT:
			returnType = Type::integer();
            argumentTypes.push_back(Type::aRel());
			secondIsName = true;
			break;
		case TokenType::TK_DAYS:
			returnType = Type::integer();
            argumentTypes.push_back(Type::text());
			argumentTypes.push_back(Type::text());
			break;
		case TokenType::TK_BEFORE:
		case TokenType::TK_AFTER:
			returnType = Type::text();
            argumentTypes.push_back(Type::text());
			argumentTypes.push_back(Type::text());
			break;
		case TokenType::TK_DATE:
			returnType = Type::text();
            argumentTypes.push_back(Type::text());
			argumentTypes.push_back(Type::integer());
			break;
        case TokenType::TK_TODAY:
			returnType = Type::text();
			break;
		case TokenType::TK_PRINT:
			returnType = Type::boolean();
            argumentTypes.push_back(Type::any());
			break;
		default:
            error->reportError("Unknown buildin", node->nameToken, {node->charRange});
			break;
		}
		
        node->type=returnType;
		size_t expectedArgs=argumentTypes.size() + (secondIsName?1:0);
        if (node->args.size() != expectedArgs) {
			// too few args
			std::stringstream ss;
			ss << "Too "
			   << (node->args.size() < argumentTypes.size()?"few":"many")
			   << " arguments to builtin function, received " << node->args.size() 
			   << " but expected " << expectedArgs;
			error->reportError(ss.str(), node->nameToken, {node->charRange});
		}
		
		for (size_t i=0; i < node->args.size(); ++i) {
			if (i == 1 && secondIsName) {
				// TODO check if the name in actually in the schema
				if (node->args[i]->nodeType != NodeType::VariableExp) 
					error->reportError("Expected identifier", Token(), {node->args[i]->charRange});
				continue;
			}
			visitNode(node->args[i]);
			if (i < argumentTypes.size())
				typeCheck(node->nameToken, node->args[i], argumentTypes[i]);
		}
	}

    void visit(std::shared_ptr<ConstantExp> node) {
		switch (node->valueToken.id) {
		case TokenType::TK_FALSE:
        case TokenType::TK_TRUE:
        case TokenType::TK_STDBOOL:
            node->type = Type::boolean();
			break;
		case TokenType::TK_TEXT:
        case TokenType::TK_STDTEXT:
            node->type = Type::text();
			break;
        case TokenType::TK_ZERO:
        case TokenType::TK_ONE:
            node->type = Type::rel({});
			break;
        case TokenType::TK_STDINT:
			node->type = Type::integer();
			break;
        case TokenType::TK_STDFLOAT:
			node->type = Type::fp();
			break;
		case TokenType::TK_BADINT:
			error->reportWarning(
				"Integers should not start with 0 (we do not support octals)",
				node->valueToken);
			node->type = Type::integer();
			break;
		case TokenType::TK_INT:
			//TODO Validate the integer and check its range
            //atoi(code->code.substr(node->valueToken.start, node->valueToken.length).c_str());
			node->type = Type::integer();
			break;
		case TokenType::TK_FLOAT:
			// TODO Validate range
			node->type = Type::fp();
			break;
		default:
			internalError(node->valueToken, std::string("Invalid constant type ")+getTokenName(node->valueToken.id));
			node->type = Type::invalid();
			break;
		}
	}   

    void visit(std::shared_ptr<UnsetExp> node) {
		node->type = Type::boolean();
		if (scopes.size() == 1) {
			node->isGlobal = true;
		}

		std::string name = node->nameToken.getText(code);
		auto &lu = scopes.back();
		auto it = lu.bind.find(name);
		if (it != lu.bind.end())  {
			lu.bind.erase(it);
			return;
		}


		// If we cannot find the variable, then it must be an external relation

		if (scopes.size() != 1 || !callback->hasRelation(name.c_str())) {
			std::stringstream ss;
			ss << "Unknown variable " << name;
			error->reportError(ss.str(), node->nameToken);
			return;
		}

	}

    void visit(std::shared_ptr<UnaryOpExp> node) {
        visitNode(node->exp);
		switch (node->opToken.id) {
		case TokenType::TK_NOT:
			typeCheck(node->opToken, node->exp, Type::boolean());
			node->type = Type::boolean();
			break;
		case TokenType::TK_MINUS:
            typeCheck(node->opToken, node->exp, Type::fpOrInt());
            node->type = node->exp->type;
			break;
		default:
            internalError(node->opToken, "Bad unary operator");
            node->type = Type::invalid();
		}
	}

    void visit(std::shared_ptr<RelExp> node) {
		// TODO stronger type here
        node->type = Type::aRel(); 
        visitNode(node->exp);
        typeCheck(node->relToken, node->exp, Type::aTup());
	}
	
    void visit(std::shared_ptr<LenExp> node) {
        visitNode(node->exp);
		typeCheck(node->leftPipeToken, node->exp, 
				  Type::disjunction({Type::text(), Type::aRel(), Type::aTup()}));
        node->type = Type::integer();
	}
	
	bool getFunctions(Type ft, std::vector<Type> & ans) {
		bool afunc=false;
		switch (ft.base()) {
		case Type::AFunc:
			afunc=true;
			break;
		case Type::Func:
			ans.push_back(ft);
			break;
		case Type::Disjunction:
			for (const auto & e: ft.disjunctionParts())
				afunc = afunc || getFunctions(e, ans);
			break;
		default:
			break;
		}
		return afunc;
	}
	
    void visit(std::shared_ptr<FuncInvocationExp> node) {
        visitNode(node->funcExp);
        visitAll(node->args);

		if (node->funcExp->type.base() == Type::Any) {
			node->type = Type::any();
			return;
		}
		
		if (node->funcExp->type.base() == Type::Invalid) {
			node->type = Type::invalid();
			return;
		}
		
		std::vector<Type> argTypes;
		for (auto arg: node->args)
			argTypes.push_back(arg->type);
		std::vector<Type> fts;
		if (getFunctions(node->funcExp->type, fts)) { //One of the functions is an any func
			node->type = Type::any();
			
		} else if(fts.size() == 0) {
			error->reportError("Tried to call none function", node->lparenToken, {node->funcExp->charRange});
			node->type = Type::invalid();
		} else {
			std::vector<Type> matchTypes;
			for (const auto & f: fts) {
				if (f.funcArgs().size() != argTypes.size()) continue;
				bool ok=true;
				for (size_t i=0; i < argTypes.size(); ++i) {
					if (!Type::match(argTypes[i], f.funcArgs()[i])) {
						ok=false;
						break;
					}
				}
				if (!ok) continue;
				matchTypes.push_back(f.funcRet());
			}
			
			if (matchTypes.size() == 0) { //There where no functions we could call
				std::stringstream ss;
				ss << "Call (";
				for (size_t i=0; i != argTypes.size(); ++i) {
					if (i != 0) ss << ", ";
					ss << argTypes[i];
				}
				ss << "), does not match:\n";
				for (size_t i=0; i < fts.size(); ++i) {
					if (i != 0) ss << " or \n";
					ss << fts[i];
				}
				error->reportError("Invalid function call", node->lparenToken, {node->charRange}, ss.str());
				
			} else {
				node->type = Type::disjunction(matchTypes);
			}
		}
	}

    void visit(std::shared_ptr<SubstringExp> node) {
        visitNode(node->stringExp);
        visitNode(node->fromExp);
        visitNode(node->toExp);
		typeCheck(node->lparenToken, node->stringExp, Type::text());
		typeCheck(node->lparenToken, node->fromExp, Type::integer());
		typeCheck(node->lparenToken, node->toExp, Type::integer());
		
		node->type = Type::text();
	}

    void visit(std::shared_ptr<RenameExp> node) {
        visitNode(node->lhs);
		// TODO Stronger type check
        typeCheck(node->lbracketToken, node->lhs, Type::aRel());
		// TODO Stronger return type
        node->type = Type::aRel();
	}

    void visit(std::shared_ptr<DotExp> node) {
        visitNode(node->lhs);
		// TODO Stronger type check
        typeCheck(node->dotToken, node->lhs, Type::aTup());
		// TODO Stronger return type
        node->type = Type::any();
	}

    void visit(std::shared_ptr<TupMinus> node) {
        visitNode(node->lhs);
		// TODO Stronger type check
        typeCheck(node->opToken, node->lhs, Type::aTup());
		// TODO Stronger return type
        node->type = Type::aTup();
	}

    void visit(std::shared_ptr<ProjectExp> node) {
        visitNode(node->lhs);
		// TODO Stronger type check
        typeCheck(node->projectionToken, node->lhs, Type::aRel());
		// TODO Stronger return type
        node->type = Type::aRel();
	}

    void visit(std::shared_ptr<InvalidExp> node) {
        node->type = Type::invalid();
	}

	struct BinopHelp {
		Type lhsType;
		Type rhsType;
		Type resType;
	};


	void binopTypeCheck(std::shared_ptr<BinaryOpExp> node,
						std::initializer_list<BinopHelp> ops) {
        visitNode(node->lhs);
        visitNode(node->rhs);
		Type lhst=node->lhs->type;
		Type rhst=node->rhs->type;
		
		std::vector<Type> matches;
		for(auto h: ops) {
			if (!Type::match(h.lhsType, lhst)) continue;
			if (!Type::match(h.rhsType, rhst)) continue;
			matches.push_back(h.resType);
		}
		
		if (matches.size() == 0) {
			if (ops.size() == 1) {
				typeCheck(node->opToken, node->lhs, ops.begin()->lhsType);
				typeCheck(node->opToken, node->rhs, ops.begin()->rhsType);
			} else {
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
			return;
		}
		node->type = Type::disjunction(matches);
	}

	
    void visit(std::shared_ptr<BinaryOpExp> node) {
		switch(node->opToken.id) {
		case TokenType::TK_PLUS:
		case TokenType::TK_MUL:
		case TokenType::TK_MINUS:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::fp()},
					{Type::fp(), Type::fp(), Type::fp()},
					{Type::integer(),   Type::fp(), Type::fp()},
					{Type::integer(), Type::integer(), Type::integer()},
						// TODO better type checking
					{Type::aRel(), Type::aRel(), Type::aRel()}
				});
			break;
		case TokenType::TK_DIV:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::fp()},
					{Type::fp(), Type::fp(), Type::fp()},
					{Type::integer(),   Type::fp(), Type::fp()},
					{Type::integer(),   Type::integer(),   Type::integer()} });
			break;
		case TokenType::TK_MOD:
			binopTypeCheck(node, { 
					{Type::fp(), Type::integer(),   Type::fp()},
					{Type::fp(), Type::fp(), Type::fp()},
					{Type::integer(),   Type::fp(), Type::fp()},
					{Type::integer(), Type::integer(), Type::integer()} });
			break;
		case TokenType::TK_AND:
		case TokenType::TK_OR:
			binopTypeCheck(node, { {Type::boolean(), Type::boolean(), Type::boolean()} });
			break;
        case TokenType::TK_CONCAT:
			binopTypeCheck(node, { {Type::text(), Type::text(), Type::text()} });
			break;
		case TokenType::TK_LESSEQUAL:
		case TokenType::TK_LESS:
		case TokenType::TK_GREATER:
		case TokenType::TK_GREATEREQUAL:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::boolean()},
					{Type::fp(), Type::fp(), Type::boolean()},
					{Type::integer(),   Type::fp(), Type::boolean()},
					{Type::integer(), Type::integer(), Type::boolean()},
					{Type::boolean(), Type::boolean(), Type::boolean()} });
			break;
		case TokenType::TK_EQUAL:
		case TokenType::TK_DIFFERENT:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::boolean()},
					{Type::fp(), Type::fp(), Type::boolean()},
					{Type::integer(),   Type::fp(), Type::boolean()},
					{Type::integer(), Type::integer(), Type::boolean()},
					{Type::boolean(), Type::boolean(), Type::boolean()},
					{Type::text(), Type::text(), Type::boolean()},
					// TODO better type checking
					{Type::aTup(), Type::aTup(), Type::boolean()},
					// TODO better type checking
					{Type::aRel(), Type::aRel(), Type::boolean()}});
			break;
		case TokenType::TK_TILDE:
			binopTypeCheck(node, { {Type::text(), Type::text(), Type::boolean()} });
			break;
		case TokenType::TK_SELECT:
			// TODO better type checking
			binopTypeCheck(node, { {Type::aRel(), Type::aFunc(), Type::aRel()} });
			break;
		case TokenType::TK_OPEXTEND:
			// TODO better type checking
			binopTypeCheck(node, { {Type::aTup(), Type::aTup(), Type::aTup()} });
			break;
		default:
            internalError(node->opToken, std::string("Invalid operator")+getTokenName(node->opToken.id));
            node->type = Type::invalid();
			break;
		}
	}

    void visit(std::shared_ptr<SequenceExp> node) {
        visitAll(node->sequence);
        if (node->sequence.empty())
			node->type = Type::invalid();
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
										   std::shared_ptr<Callback> callback,
										   bool missingIsAny) {
	return std::make_shared<FirstParseImpl>(error, code, callback, missingIsAny);
}

} //namespace rasmus
} //namespace frontend
