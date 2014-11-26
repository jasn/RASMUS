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

#include <frontend/strongType.hh>
namespace rasmus {
namespace frontend {
namespace strongType {

void Base::destroy() {
	switch (type()) {
	case BaseType::Any:
	case BaseType::Int:
	case BaseType::Float:
	case BaseType::Bool:
	case BaseType::Text:
	case BaseType::ARel:
	case BaseType::ATup:
	case BaseType::AFunc:
		delete this;
		break;
	case BaseType::Func:
		delete static_cast<Func*>(this);
		break;
	case BaseType::Disjunction:
		delete static_cast<Func*>(this);
		break;
	}
}

Ptr returnType(const Ptr & type) {
	switch(type->type()) {
	case BaseType::Any:
	case BaseType::Int:
	case BaseType::Float:
	case BaseType::Bool:
	case BaseType::Text:
	case BaseType::ARel:
	case BaseType::ATup:
	case BaseType::Rel:
	case BaseType::Tup:
	case BaseType::None:
		return none();
	case BaseType::AFunc: 
		return any();
	case BaseType::Func:
		return static_cast<Func *>(type.get())->ret;
	}
}

Type plain(const Ptr & type) {
	switch(type->type()) {
	case BaseType::Any: return TAny;
	case BaseType::Int:	return TInt;
	case BaseType::Float: return TFloat;
	case BaseType::Bool: return TBool;
	case BaseType::Text: return TText;
	case BaseType::Rel: return TRel;
	case BaseType::ARel: return TRel;
	case BaseType::Tup: return TTup;
	case BaseType::ATup: return TTup;
	case BaseType::Func: return TFunc;
	case BaseType::AFunc: return TFunc;
	case BaseType::None: return TInvalid;
	}
}

Ptr strong(Type type) {
	switch (type) {
	case TAny: return any();
	case TInt: return integer();
	case TFloat: return fp();
	case TBool: return boolean();
	case TText: return text();
	case TRel: return aRel();
	case TTup: return aTup();
	case TFunc: return aFunc();
	default:
		return none();
	}
}


void check(const Ptr & lhs, const Ptr & rhs) {

}

bool match(const Ptr & lhs, const Ptr & rhs) {
	switch (rhs->type()) {
	case BaseType::Any:
		return true;
	case BaseType::Int:
		return lhs->type() == BaseType::Int;
	case BaseType::Float:
		return lhs->type() == BaseType::Float;
	case BaseType::Bool:
		return lhs->type() == BaseType::Bool;
	case BaseType::Text:
		return lhs->type() == BaseType::Text;
	case BaseType::ARel:
		return lhs->type() == BaseType::ARel || lhs->type() == BaseType::Rel;
	case BaseType::Rel:
		// TODO
		return lhs->type() == BaseType::ARel || lhs->type() == BaseType::Rel;
	case BaseType::ATup:
		return lhs->type() == BaseType::ATup || lhs->type() == BaseType::Tup;
	case BaseType::Tup:
		// TODO
		return lhs->type() == BaseType::ATup || lhs->type() == BaseType::Tup;
	case BaseType::AFunc:
		return lhs->type() == BaseType::AFunc || lhs->type() == BaseType::Func;
	case BaseType::Func:
		// TODO
		return lhs->type() == BaseType::AFunc || lhs->type() == BaseType::Func;
	case BaseType::None:
		return false;
	case BaseType::Disjunction:
		for (auto e: rhs.getAs<Disjunction>()->entries) 
			if (match(lhs, e)) return true;
		return false;
	}
}

void output(std::ostream & o, const Ptr & v) {
	if (v.get() == NULL) {
		o << "NULL";
		return;
	}
	switch (v->type()) {
	case BaseType::Any:	o << "Any"; break;
	case BaseType::Int: o << "Int"; break;
	case BaseType::Float: o << "Float"; break;
	case BaseType::Bool: o << "Bool"; break;
	case BaseType::Text: o << "Text"; break;
	case BaseType::ARel: o << "Rel"; break;
	case BaseType::Rel: o << "Rel"; break; //TODO
	case BaseType::ATup: o << "Tup"; break;
	case BaseType::Tup: o << "Tup"; break; //TODO
	case BaseType::AFunc: o << "Func"; break;
	case BaseType::Func: {
		bool first=true;
		o << "Func(";
		for (auto e: v.getAs<Func>()->args) {
			if (first) first=false;
			else o << ", ";
			output(o, e);
		}
		o << ")->(";
		output(o, v.getAs<Func>()->ret);
		o << ")";
		break;
	}
	case BaseType::None: o << "Invalid"; break;
	case BaseType::Disjunction: {
		bool first=true;
		for (auto e: v.getAs<Disjunction>()->entries) {
			if (first) first=false;
			else o << " | ";
			output(o, e);
		}
		break;
	}
	}
}


void output(std::ostream & o, const Ptr & type);


} //namespace strongType
} //namespace frontend
} //namespace rasmus

