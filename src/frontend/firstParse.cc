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
#include <cassert>

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


bool schemaHasName(const Type & t, const std::string & name) {
	switch (t.kind()) {
	case Type::Any:
	case Type::ATup:
	case Type::ARel:
	case Type::Empty:
		return true;
	case Type::Int:
	case Type::Float:
	case Type::Bool:
	case Type::Text:
	case Type::Func:
	case Type::AFunc:
		return false;
	case Type::Rel:
	case Type::Tup:
		return t.relTupSchema().count(name);
	case Type::Union:
		for (const auto & tt: t.unionParts())
			if (schemaHasName(tt, name))
				return true;
		return false;
	}
	assert(false);
	return false;
}

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
		if (Type::canCastSucceed(t, expr->type))
			return true;

		std::stringstream ss;
		if (t.kind() == Type::Union)
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
			return Type::empty();
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
				std::vector<std::pair<std::string, PlainType> > schema;
				if (!callback->relationSchema(name.c_str(), schema)) {
					node->type = Type::aRel(); 
					return;
				} 
				std::map<std::string, Type> s;
				for (const auto & p: schema)
					s.insert(std::make_pair(p.first, Type(p.second)));
				node->type = Type::rel(s);
				return;
			} else {
				if (false) ;
				else if (name == "acos") {
					node->buildin = BuildIn::acos;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "asin") {
					node->buildin = BuildIn::asin;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "atan") {
					node->buildin = BuildIn::atan;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "atan2") {
					node->buildin = BuildIn::atan2;
					node->type = Type::func(Type::fp(), {Type::fpAndInt(), Type::fpAndInt()});
				} else if (name == "ceil") {
					node->buildin = BuildIn::ceil;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "cos") {
					node->buildin = BuildIn::cos;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "floor") {
					node->buildin = BuildIn::floor;
					node->type = Type::func(Type::integer(), {Type::fpAndInt()});
				} else if (name == "pow") {
					node->buildin = BuildIn::pow;
					node->type = Type::func(Type::fp(), {Type::fpAndInt(), Type::fpAndInt()});
				} else if (name == "round") {
					node->buildin = BuildIn::round;
					node->type = Type::func(Type::integer(), {Type::fpAndInt()});
				} else if (name == "sin") {
					node->buildin = BuildIn::sin;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "sqrt") {
					node->buildin = BuildIn::sqrt;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "substr") {
					node->buildin = BuildIn::substr;
					node->type = Type::func(Type::text(), {Type::text(), Type::integer(), Type::integer()});
				} else if (name == "tan") {
					node->buildin = BuildIn::tan;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "log") {
					node->buildin = BuildIn::log;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "log2") {
					node->buildin = BuildIn::log2;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "log10") {
					node->buildin = BuildIn::log10;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else if (name == "exp") {
					node->buildin = BuildIn::exp;
					node->type = Type::func(Type::fp(), {Type::fpAndInt()});
				} else {
					std::stringstream ss;
					ss << "Unknown variable " << name;
					error->reportError(ss.str(), node->nameToken);
					node->type = Type::empty();
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

		node->type = Type::join(types);
	}
	
    void visit(std::shared_ptr<ForallExp> node) {		
		visitAll(node->listExps);
		std::map<std::string, Token> names;
		for (const auto & token: node->names) {
			std::string name = token.getText(code);			
			auto x=names.insert(std::make_pair(name, token));
			if (x.second) continue;
			
			std::stringstream ss;
			ss << "The attribute '" << name << "' has been specified twice";
			error->reportError(ss.str(), token, {r(x.first->second)});
		}

		std::vector<Type> types;
		
		if (names.size() == 0) {
			// Forall
			std::vector<Type> rels;
			if (node->listExps.size() != 1) {
				error->reportError("Forall can only be used on one exactly one relation", node->typeToken);
				types.push_back(Type::aTup());
			} else if (!typeCheck(node->typeToken, node->listExps[0], Type::aRel())
					   || getRels(node->listExps[0]->type, rels)) {
				types.push_back(Type::aTup());
			} else {
				std::vector<Type> tups;
				for (const auto & rel: rels) 
					tups.push_back(Type::tup(rel.relTupSchema()));
				types.push_back(Type::join(tups));
			}
		} else {
			// Factor
			std::map<std::string, std::vector<Type> > nameTypes;
			
			types.push_back(Type::empty());
			
			for(auto exp : node->listExps) {
				std::vector<Type> rels;
				if (!typeCheck(node->typeToken, exp, Type::aRel())
					|| getRels(exp->type, rels)) {
					types.push_back(Type::aRel());
					continue;
				}
				std::vector<Type> type;
				
				std::map<std::string, std::vector<Type> > expNameTypes;
				
				for (const auto & rel: rels) {
					std::map<std::string, Type> schema=rel.relTupSchema();
					for (const auto & p: names) {
						auto x=schema.find(p.first);
						if (x != schema.end()) {
							expNameTypes[p.first].push_back(x->second);
							schema.erase(x);
							continue;
						}
						if (rels.size() != 1) continue;
						std::stringstream ss;
						ss << "The attribute '" << p.first << "' does not exist in "
						   << rel;
						error->reportError(ss.str(), p.second, {exp->charRange});
					}
					type.push_back(Type::rel(schema));
				}
				
				for (const auto & p: names) {
					const std::vector<Type> & t=expNameTypes[p.first];
					if (!t.empty()) {
						nameTypes[p.first].insert(
							nameTypes[p.first].end(), t.begin(), t.end());
						continue;
					}
					nameTypes[p.first].push_back(Type::atomic());


					if (rels.size() == 1) continue;
					if (rels.size() == 0) continue;
					std::stringstream ss1;
					ss1 << "The attribute '" << p.first << "' is not pressent in any of the possible types";
					std::stringstream ss2;
					ss2 << "The possible types are:";
					for (const auto & rel: rels)
						ss2 << "\n  " << rel;
					error->reportError(ss1.str(), p.second, {exp->charRange}, ss2.str());
				}
				types.push_back(Type::join(std::move(type)));
			}

			std::map<std::string, Type> schema;
			for (const auto & p:names) {
				Type t=Type::intersection(nameTypes[p.first]);
				if (t.isEmpty()) {
					t = Type::aRel();
					std::stringstream ss1;
					ss1 << "The attribute '" << p.first << "' does not have a valid type.";
					std::stringstream ss2;
					ss2 << "The type should be all of the below at the same time:";
					for (const auto & r: nameTypes[p.first])
						ss2 << "\n* " << r;
					error->reportError(ss1.str(), p.second, {}, ss2.str());
					
				} 
				schema.insert(schema.end(), std::make_pair(p.first, std::move(t)));
			}
			types[0] = Type::tup(std::move(schema));
		}
		assert(node->exp->nodeType == NodeType::FuncExp);
		std::shared_ptr<FuncExp> f = std::static_pointer_cast<FuncExp>(node->exp);
		assert(types.size() == f->args.size());
		for (size_t i=0; i < types.size(); ++i)
			f->args[i]->type = std::move(types[i]);
		
		visitNode(node->exp);		
		if (typeCheck(node->colonToken, node->exp, Type::aFunc()))
			node->type = node->exp->type.funcRet();
		else
			node->type = Type::aRel();
	}

    void visit(std::shared_ptr<FuncExp> node) {
        scopes.push_back(Scope(node));
		std::vector<Type> argTypes;
        for (auto a: node->args) {
			scopes.back().bind[a->nameToken.getText(code)] = a;
			if (a->type.isEmpty()) a->type = tokenToType(a->typeToken);
			argTypes.push_back(a->type);
		}
		Type rtype=tokenToType(node->returnTypeToken);
        visitNode(node->body);
		typeCheck(node->funcToken, node->body, rtype);

		Type nrtype=Type::intersection({rtype, node->body->type});
		if (!nrtype.isEmpty()) rtype=nrtype;
		node->type = Type::func(rtype, argTypes);
		scopes.pop_back();
	}

    void visit(std::shared_ptr<TupExp> node) {
		std::map<std::string, Type> schema;
        for (auto item: node->items) {
			visitNode(item->exp);
			Type it=Type::empty();
			if (typeCheck(item->colonToken, item->exp, Type::atomic()))
				it = item->exp->type;
			std::string name = item->nameToken.getText(code);
			if (!schema.insert(std::make_pair(name, it)).second)
				error->reportError("Duplicate attribute name", item->nameToken);
		}
        node->type = Type::tup(schema);
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
		Type returnType=Type::empty();
		std::vector<Type> argumentTypes;
		bool secondIsName=false;
		bool mustExist=true;
		bool allowTup=false;
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
			allowTup = true;
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
            argumentTypes.push_back(Type::join({Type::aRel(), Type::aTup()}));
			allowTup = true;
			mustExist=false;
			secondIsName = true;
			break;
		case TokenType::TK_MAX:
		case TokenType::TK_MIN:
			returnType = Type::atomic();
			argumentTypes.push_back(Type::aRel());
			secondIsName = true;
			break;
		case TokenType::TK_ADD:
		case TokenType::TK_MULT:
			returnType = Type::fpAndInt();
            argumentTypes.push_back(Type::aRel());
			secondIsName = true;
			break;
		case TokenType::TK_COUNT:
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
			   << ((node->args.size() < expectedArgs)?"few":"many")
			   << " arguments to builtin function, received " << node->args.size() 
			   << " but expected " << expectedArgs;
			error->reportError(ss.str(), node->nameToken, {node->charRange});
		}
		
		for (size_t i=0; i < node->args.size(); ++i) {
			if (i == 1 && secondIsName) {
				if (node->args[i]->nodeType != NodeType::VariableExp) 
					error->reportError("Expected identifier", Token(), {node->args[i]->charRange});
				else if (mustExist) {
					Token nt=std::static_pointer_cast<VariableExp>(node->args[i])->nameToken;
					std::string name = nt.getText(code);
					std::vector<Type> rels;
					if (!getRels(node->args[0]->type, rels) &&
						(!allowTup || !getTups(node->args[0]->type, rels))) {
						bool found=false;
						for (const auto & rel: rels)
							if (rel.relTupSchema().count(name))
								found = true;
						if (!found)
							error->reportError("Attribute name not in schema", nt, {node->args[0]->charRange});
					}
				}
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
            node->type = Type::rel(std::map<std::string, Type>());
			break;
        case TokenType::TK_STDINT:
			node->type = Type::integer();
			break;
        case TokenType::TK_STDFLOAT:
			node->type = Type::fp();
			break;
		case TokenType::TK_BADINT: {
			error->reportWarning(
				"Integers should not start with 0 (we do not support octals)",
				node->valueToken);
			//Fall through
		}
		case TokenType::TK_INT:
		{
			std::string text = node->valueToken.getText(code);
			char * end=nullptr;
			const char * start=text.c_str();
			errno = 0;
			strtol(start, &end, 10) ;
			if (errno == ERANGE) 
				error->reportError("Integer outside valid range", node->valueToken, {node->charRange});
			else if (errno != 0 || start+text.size() != end) 
				error->reportError("Invalid integer", node->valueToken, {node->charRange});
			node->type = Type::integer();
			break;
		}
		case TokenType::TK_FLOAT:
		{
			std::string text = node->valueToken.getText(code);
			char * end=nullptr;
			const char * start = text.c_str();
			errno = 0;
			strtod(start, &end) ;
			if (errno == ERANGE) 
				error->reportError("Float outside valid range", node->valueToken, {node->charRange});
			else if (errno != 0 || start+text.size() != end) 
				error->reportError("Invalid float", node->valueToken, {node->charRange});
			node->type = Type::fp();
			break;
		}
		default:
			internalError(node->valueToken, std::string("Invalid constant type ")+getTokenName(node->valueToken.id));
			node->type = Type::empty();
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
            typeCheck(node->opToken, node->exp, Type::fpAndInt());
            node->type = node->exp->type;
			break;
		default:
            internalError(node->opToken, "Bad unary operator");
            node->type = Type::empty();
		}
	}

	static bool getTups(Type ft, std::vector<Type> & ans) {
		bool atup=false;
		switch (ft.kind()) {
		case Type::Empty:
		case Type::Any:
		case Type::ATup:
			atup=true;
			break;
		case Type::Tup:
			ans.push_back(ft);
			break;
		case Type::Union:
			for (const auto & e: ft.unionParts())
				atup = atup || getTups(e, ans);
			break;
		default:
			break;
		}
		return atup;
	}

	static bool getRels(Type ft, std::vector<Type> & ans) {
		bool arel=false;
		switch (ft.kind()) {
		case Type::Empty:
		case Type::Any:
		case Type::ARel:
			arel=true;
			break;
		case Type::Rel:
			ans.push_back(ft);
			break;
		case Type::Union:
			for (const auto & e: ft.unionParts())
				arel = arel || getRels(e, ans);
			break;
		default:
			break;
		}
		return arel;
	}

    void visit(std::shared_ptr<RelExp> node) {
        visitNode(node->exp);
        if (!typeCheck(node->relToken, node->exp, Type::aTup())) {
			node->type = Type::aRel();
			return;
		}
		
		std::vector<Type> tups;
		if (getTups(node->exp->type, tups)) {
			node->type = Type::aRel();
			return;
		}
		assert(!tups.empty());
		std::vector<Type> rels;
		for (const auto & tup: tups)
			rels.push_back(Type::rel(tup.relTupSchema()));
		node->type = Type::join(rels);
	}
	
    void visit(std::shared_ptr<LenExp> node) {
        visitNode(node->exp);
		typeCheck(node->leftPipeToken, node->exp, 
				  Type::join({Type::text(), Type::aRel(), Type::aTup()}));
        node->type = Type::integer();
	}
	
	bool getFunctions(Type ft, std::vector<Type> & ans) {
		bool afunc=false;
		switch (ft.kind()) {
		case Type::AFunc:
			afunc=true;
			break;
		case Type::Func:
			ans.push_back(ft);
			break;
		case Type::Union:
			for (const auto & e: ft.unionParts())
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

		if (node->funcExp->type.kind() == Type::Any) {
			node->type = Type::any();
			return;
		}
		
		if (node->funcExp->type.kind() == Type::Empty) {
			node->type = Type::empty();
			return;
		}
		
		std::vector<Type> argTypes;
		for (auto arg: node->args)
			argTypes.push_back(arg->type);
		std::vector<Type> fts;
		if (getFunctions(node->funcExp->type, fts)) { //One of the functions is an any func
			node->type = Type::any();
			
		} else if(fts.size() == 0) {
			std::stringstream ss;
			ss << "The type is " << node->funcExp->type;
			error->reportError("Tried to call none function", node->lparenToken, {node->funcExp->charRange},
							   ss.str());
			node->type = Type::empty();
		} else {
			std::vector<Type> matchTypes;
			for (const auto & f: fts) {
				if (f.funcArgs().size() != argTypes.size()) continue;
				bool ok=true;
				for (size_t i=0; i < argTypes.size(); ++i) {
					if (!Type::canCastSucceed(argTypes[i], f.funcArgs()[i])) {
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
				node->type = Type::join(matchTypes);
			}
		}
	}

	static CharRange r(Token t) {
		if (!t || t.length == 0) return CharRange();
		return CharRange(t.start, t.start+t.length);
	}
	
	void visit(std::shared_ptr<RenameExp> node) {
		visitNode(node->lhs);
		typeCheck(node->lbracketToken, node->lhs, Type::aRel());
		std::vector<Type> rels;
		if (getRels(node->lhs->type, rels)) {
			node->type = Type::aRel();
			return;
		}

		bool bad=false;
		std::map<std::string, Token> froms;
		std::map<std::string, Token> tos;
		for (const auto & p: node->renames) {
			Token from=p->fromNameToken;
			Token to=p->toNameToken;
			auto x=froms.insert(std::make_pair(from.getText(code), from));
			if (!x.second) {
				std::stringstream ss;
				ss << "The attribute '" << from.getText(code) << "' has already been renamed";
				error->reportError(ss.str(), from, {r(x.first->second)});
				bad = true;
			}
			auto y=tos.insert(std::make_pair(to.getText(code), to));
			if (!y.second) {
				std::stringstream ss;
				ss << "The attribute '" << to.getText(code) << "' has already been renamed to";
				error->reportError(ss.str(), to, {r(y.first->second)});
				bad = true;
			}
		};

		if (bad) {
			node->type = Type::aRel();
			return;
		}

		std::vector<Type> newRels;
		for (const Type & rel: rels) {
			const auto & schema = rel.relTupSchema();
			std::map<std::string, Type> newSchema = schema;
			
			bool bad=false;
			for (const auto & p: froms) {
				if (newSchema.erase(p.first)) continue;
				bad=true;
				if (rels.size() != 1) continue;
				std::stringstream ss;
				ss << "The attribue '" << p.first << "' is not defined in "
				   << rels[0];
				error->reportError(ss.str(), p.second, {node->lhs->charRange});
			}

			for (const auto & p: node->renames) {
				Token from=p->fromNameToken;
				Token to=p->toNameToken;
				
				auto x = schema.find(from.getText(code));
				if (x == schema.end()) {
					bad=true;
					//This should also have been caught by the above loop}
					continue;
				}
				
				if (newSchema.insert(std::make_pair(to.getText(code), x->second)).second)
					continue;
								
				bad=true;
				if (rels.size() == 1) {
					std::stringstream ss;
					ss << "The attribute '" << to.getText(code) << "' is allredy in "
					   << rels[0];
					error->reportError(ss.str(), to, {node->lhs->charRange});
				}
			}
			if (!bad)
				newRels.push_back(Type::rel(std::move(newSchema)));
		}

		if (newRels.empty()) {
			node->type = Type::aRel();
			if (rels.size() != 1) {
				std::stringstream ss;
				ss << "The rename cannot be done on any of the possible relation types:";
				for (const Type & rel: rels) 
					ss << "\n  " << rel;
				error->reportError("Invalid rename", node->lbracketToken, {node->charRange}, ss.str());
			}
		} else {
			node->type = Type::join(std::move(newRels));
		}
	}

    void visit(std::shared_ptr<DotExp> node) {
        visitNode(node->lhs);
        if (!typeCheck(node->dotToken, node->lhs, Type::aTup())) {
			node->type = Type::empty();
			return;
		}
		
		std::vector<Type> rels;
		if (getTups(node->lhs->type, rels)) {
			// If the type is any tup then our type is atomic
			node->type = Type::atomic();
			return;
		}

		std::vector<Type> types;
		std::string name=node->nameToken.getText(code);
		for (const auto & t: rels) {
			auto x=t.relTupSchema().find(name);
			if (x == t.relTupSchema().end()) continue;
			types.push_back(x->second);
		}
		
		if (types.empty()) {
			std::stringstream ss;
			ss << "Attribute '" << name << "' does not exist in schema";
			if (rels.size() == 1) 
				ss << ": " << rels[0];
			error->reportError(ss.str(), node->nameToken, {node->lhs->charRange});
			node->type = Type::empty();
			return;
		}
		
		node->type = Type::join(types);
	}

    void visit(std::shared_ptr<TupMinus> node) {
        visitNode(node->lhs);
		if (!typeCheck(node->opToken, node->lhs, Type::aTup())) {
			node->type = Type::aTup();
			return;
		}
		
		std::vector<Type> tups;
		if (getTups(node->lhs->type, tups)) {
			// In case of aTub
			node->type = Type::aTup();
			return;
		}
		
		std::string name=node->nameToken.getText(code);
		
		std::vector<Type> ntups;
		for (const auto & tup: tups) {
			std::map<std::string, Type> schema=tup.relTupSchema();
			if (schema.erase(name)) {
				ntups.push_back(Type::tup(std::move(schema)));
				continue;
			}
			if (tups.size() == 1) {
				std::stringstream ss;
				ss << "The attribue '" << name << "' is not in the schema of "
				   << tup;
				error->reportError(ss.str(), node->nameToken, {node->lhs->charRange});
			}
		}
		
		if (!ntups.empty()) {
			node->type = Type::join(ntups);
			return;
		}
		
		node->type = Type::aTup();
		if (tups.size() == 1) return;
		
		std::stringstream ss;
		ss << "The attribute '" << name << "' is not in any of the possible tuple types:";
		for (const Type & tup: tups) 
			ss << "\n  " << tup;
		error->reportError("Bad tuple remove", node->nameToken, {node->lhs->charRange}, ss.str());
	}

    void visit(std::shared_ptr<ProjectExp> node) {
        visitNode(node->lhs);
		
		std::map<std::string, Token> names;
		for (const auto & t: node->names) {
			std::string name=t.getText(code);
			auto x=names.insert(std::make_pair(name, t));
			if (x.second) continue;
			
			std::stringstream ss;
			ss << "The attribute '" << name << "' has been specified twice";
			error->reportError(ss.str(), t, {r(x.first->second)});
		}

        if (!typeCheck(node->projectionToken, node->lhs, Type::aRel())) {
			node->type = Type::aRel();
			return;
		}
		
		std::vector<Type> rels;
		if (getRels(node->lhs->type, rels)) {
			// If we are anyrel and it is a project minus we know nothing
			if (node->projectionToken.id == TokenType::TK_PROJECT_MINUS) {
				node->type = Type::aRel();
				return;
			} 
			// In the case of project plus atleast we know the names.
			std::map<std::string, Type> schema;
			for (const auto & p: names) 
				schema.insert(schema.end(), std::make_pair(p.first, Type::atomic()));
			node->type = Type::rel(std::move(schema));
			return;
		}

		std::vector<Type> nrels;
		if (node->projectionToken.id == TokenType::TK_PROJECT_MINUS) {
			for(const auto & rel: rels) {
				bool bad=false;
				std::map<std::string, Type> schema=rel.relTupSchema();
				for (const auto p: names) {
					if (schema.erase(p.first)) continue;
					bad=true;
					if (rels.size() == 1) {
						std::stringstream ss;
						ss << "The attribute '" << p.first << "' is not in the relation of type " << rel;
						error->reportError(ss.str(), p.second, {node->lhs->charRange});
					}
				}
				if (!bad) nrels.push_back(Type::rel(std::move(schema)));
			}
		} else {
			for(const auto & rel: rels) {
				bool bad=false;
				const std::map<std::string, Type> & schema=rel.relTupSchema();
				std::map<std::string, Type> nschema;
				for (const auto p: names) {
					auto x=schema.find(p.first);
					if (x != schema.end()) {
						nschema.insert(nschema.end(), *x);
						continue;
					}
					bad=true;
					if (rels.size() == 1) {
						std::stringstream ss;
						ss << "The attribute '" << p.first << "' is not in the relation of type " << rel;
						error->reportError(ss.str(), p.second, {node->lhs->charRange});
					}
				}
				if (!bad) nrels.push_back(Type::rel(std::move(nschema)));
			}
		}
		
		if (!nrels.empty()) {
			node->type = Type::join(std::move(nrels));
			return;
		}
		
		if (rels.size() != 1) {
			std::stringstream ss;
			ss << "Possible schemas are:";
			for (const auto & rel: rels) 
				ss << "\n  " << rel;
			error->reportError("The project does not match any of the possible schemas", 
							   node->projectionToken, {node->charRange}, ss.str());
		}
	}

    void visit(std::shared_ptr<InvalidExp> node) {
        node->type = Type::empty();
	}

	struct BinopHelp {
		Type lhsType;
		Type rhsType;
		Type resType;
		std::function<Type(Type, Type)> func;

		BinopHelp(Type lhsType, Type rhsType, Type resType, 
				  std::function<Type(Type, Type)> func=std::function<Type(Type, Type)>()):
			lhsType(lhsType), rhsType(rhsType), resType(resType), func(func) {}
	};


	void binopTypeCheck(std::shared_ptr<BinaryOpExp> node,
						std::initializer_list<BinopHelp> ops) {
        visitNode(node->lhs);
        visitNode(node->rhs);
		Type lhst=node->lhs->type;
		Type rhst=node->rhs->type;
		
		std::vector<BinopHelp> matches;
		for(auto h: ops) {
			if (!Type::canCastSucceed(h.lhsType, lhst)) continue;
			if (!Type::canCastSucceed(h.rhsType, rhst)) continue;
			matches.push_back(h);
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

		std::vector<Type> types;
		for(auto h: matches) {
			Type t = h.func? h.func(lhst, rhst): h.resType;
			if (!t.isEmpty()) 
				types.push_back(t);
		}
		if (types.empty()) {
			for(auto h: matches) 
				types.push_back(h.resType);
			std::stringstream ss;
			ss << "Invalid operator use with types " << lhst << " and " << rhst << ".";
			error->reportError(ss.str(), node->opToken, {node->lhs->charRange, node->rhs->charRange});			
		}
		node->type = Type::join(types);
	}

	void visitSelect(std::shared_ptr<BinaryOpExp> node) { 
		visitNode(node->lhs);
		if (!typeCheck(node->opToken, node->lhs, Type::aRel())) {
			node->type = Type::aRel();
			visitNode(node->rhs);
			typeCheck(node->opToken, node->rhs, Type::func(Type::boolean(), {Type::aTup()}));
			return;
		} 
	
		std::vector<Type> rels;
		if (getRels(node->lhs->type, rels)) {
			node->type = Type::aRel();
			visitNode(node->rhs);
			typeCheck(node->opToken, node->rhs, Type::func(Type::boolean(), {Type::aTup()}));
			return;
		}

		std::vector<Type> tups;
		for (const auto & rel: rels) 
			tups.push_back(Type::tup(rel.relTupSchema()));
		Type tup=Type::join(tups);

		assert(node->rhs->nodeType == NodeType::FuncExp);
		std::shared_ptr<FuncExp> f = std::static_pointer_cast<FuncExp>(node->rhs);
		assert(f->args.size() == 1);
		f->args[0]->type = tup;

		visitNode(node->rhs);
		typeCheck(node->opToken, node->rhs, Type::func(Type::boolean(), {tup}));
		node->type = node->lhs->type;
	}

	static Type typeNaturalJoin(Type lhs, Type rhs) {
		std::vector<Type> lrels;
		std::vector<Type> rrels;
		if (getRels(lhs, lrels)) {
			// If the lhs is anyrel
			return Type::aRel();
		}
		if (getRels(rhs, rrels)) {
			// If the rhs is anyrel
			return Type::aRel();
		}
		
		std::vector<Type> rels;
		for (const auto lrel: lrels) {
			const std::map<std::string, Type> & lschema=lrel.relTupSchema();
			for (const auto rrel: rrels) {
				const std::map<std::string, Type> & rschema=rrel.relTupSchema();
				bool ok=true;
				std::map<std::string, Type> schema;
				
				auto li=lschema.begin();
				auto ri=rschema.begin();
				while (li != lschema.end() && ri != rschema.end()) {
					if (li->first < ri->first) {
						schema.insert(schema.end(), *li);
						++li;
					} else if (ri->first < li->first) {
						schema.insert(schema.end(), *ri);
						++ri;
					} else {
						Type t=Type::intersection({li->second, ri->second});
						if (t.isEmpty()) 
							ok=false;
						else 
							schema.insert(schema.end(), std::make_pair(li->first, t));
						++li;
						++ri;
					}
				}
				
				if (!ok) continue;
				for (;li != lschema.end(); ++li) schema.insert(schema.end(), *li);
				for (;ri != rschema.end(); ++ri) schema.insert(schema.end(), *ri);
				rels.push_back(Type::rel(schema));
			}
		}

		return Type::join(rels);		
	}

    void visit(std::shared_ptr<BinaryOpExp> node) {
		switch(node->opToken.id) {
		case TokenType::TK_PLUS:
		case TokenType::TK_MINUS:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::fp()},
					{Type::fp(), Type::fp(), Type::fp()},
					{Type::integer(),   Type::fp(), Type::fp()},
					{Type::integer(), Type::integer(), Type::integer()},
					{Type::aRel(), Type::aRel(), Type::aRel(), [](Type lhs, Type rhs)->Type {
							return Type::intersection({lhs, rhs});
						}}
				});
			break;
		case TokenType::TK_MUL:
			binopTypeCheck(node, {
					{Type::fp(), Type::integer(),   Type::fp()},
					{Type::fp(), Type::fp(), Type::fp()},
					{Type::integer(),   Type::fp(), Type::fp()},
					{Type::integer(), Type::integer(), Type::integer()},
					{Type::aRel(), Type::aRel(), Type::aRel(), typeNaturalJoin}
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
					{Type::aTup(), Type::aTup(), Type::boolean()},
					{Type::aRel(), Type::aRel(), Type::boolean()}});
			break;
		case TokenType::TK_TILDE:
			binopTypeCheck(node, { {Type::text(), Type::text(), Type::boolean()} });
			break;
		case TokenType::TK_SELECT:
			visitSelect(node);
			break;
		case TokenType::TK_OPEXTEND:
			binopTypeCheck(node, { {Type::aTup(), Type::aTup(), Type::aTup(), [](Type lhs, Type rhs)->Type {
							std::vector<Type> lt;
							std::vector<Type> rt;
							if (getTups(lhs, lt) || getTups(rhs, rt)) 
								return Type::aTup();
							std::vector<Type> ans;
							for (const auto & l: lt)
								for (const auto & r: rt) {
									std::map<std::string, Type> schema=r.relTupSchema();
									for (const auto & p: l.relTupSchema()) 
										schema.insert(p);
									ans.push_back(Type::tup(schema));
								}
							return Type::join(ans);
						}}});
			break;
		default:
            internalError(node->opToken, std::string("Invalid operator")+getTokenName(node->opToken.id));
            node->type = Type::empty();
			break;
		}
	}

    void visit(std::shared_ptr<SequenceExp> node) {
        visitAll(node->sequence);
        if (node->sequence.empty())
			node->type = Type::empty();
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
