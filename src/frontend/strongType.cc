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
#include <cassert>

namespace rasmus {
namespace frontend {

namespace {

class FuncC: public Type::Container {
public:
	Type ret;
	std::vector<Type> args;
	FuncC(Type ret, std::vector<Type> && args)
		: ret(ret)
		, args(std::move(args)) {
		refCnt=1;
	}
};

class SchemaC: public Type::Container {
public:
	std::map<std::string, Type> entries;
	SchemaC(std::map<std::string, Type> && entries)
		: entries(std::move(entries)) {
		refCnt=1;
	}
};

class DisjunctionC: public Type::Container {
public:
	DisjunctionC(std::vector<Type> && parts)
		: parts(std::move(parts)) {
		refCnt=1;
	}
	std::vector<Type> parts;
};

void outputSchema(std::ostream &o, const std::map<std::string, Type> & entries) {
	o << "(";
	bool first = true;
	for (const auto & p: entries) {
		if (first) first=false;
		else o << ", ";
		o << p.first << ": " << p.second;
	}
	o << ")";
}

} //Nameless namespace

const Type & Type::funcRet() const {
	assert(kind() == Func);
	return static_cast<const FuncC*>(p())->ret;
}

const std::vector<Type> & Type::funcArgs() const {
	assert(kind() == Func);
	return static_cast<const FuncC*>(p())->args;
}

const std::vector<Type> & Type::disjunctionParts() const {
	assert(kind() == Disjunction);
	return static_cast<const DisjunctionC*>(p())->parts;
}

const std::map<std::string, Type> & Type::relTupSchema() const {
	assert(kind() == Tup || kind() == Rel);
	return static_cast<const SchemaC*>(p())->entries;
}

Type Type::rel(std::map<std::string, Type> schema) {
	union {uint64_t bb; Container * pp; };
	pp = new SchemaC(std::move(schema));
	assert((bb >> 56) == 0);
	bb |= (uint64_t)Rel << 56;
	return Type(bb);
}

Type Type::tup(std::map<std::string, Type> schema) {
	union {uint64_t bb; Container * pp; };
	pp = new SchemaC(std::move(schema));
	assert((bb >> 56) == 0);
	bb |= (uint64_t)Tup << 56;
	return Type(bb);
}

Type Type::func(Type ret, std::vector<Type> args) {
	union {uint64_t bb; Container * pp; };
	pp = new FuncC(std::move(ret), std::move(args));
	assert((bb >> 56) == 0);
	bb |= (uint64_t)Func << 56;
	return Type(bb);
}


Type Type::disjunction(std::vector<Type> parts) {
	std::vector<Type> rparts;
	for (auto && part: parts) {
		if (part.kind() == Disjunction) 
			for (const auto & p: part.disjunctionParts())
				rparts.push_back(p);
		else
			rparts.emplace_back(std::move(part));
	}
	
	const int HBOOL=1;
	const int HFLOAT=2;
	const int HINT=4;
	const int HTEXT=8;
	const int HAREL=16;
	const int HATUP=32;
	const int HAFUNC=64;
	const int HANY=127;
	int has=0;
	for (const auto & part: rparts) {
		switch(part.kind()) {
		case Invalid: return invalid();
		case Any: has |= HANY; break;
		case Int: has |= HINT; break;
		case Float: has |= HFLOAT; break;
		case Bool: has |= HBOOL; break;
		case Text: has |= HTEXT; break;
		case ARel: has |= HAREL; break;
		case ATup: has |= HATUP; break;
		case AFunc: has |= HAFUNC; break;
		case Rel: 
		case Tup:
		case Func:
			break;
		case Disjunction:
			assert(false);
			break;
		}
	}
	std::vector<Type> ret;
	if ((has & HANY) == HANY) return any();
	if (has & HBOOL) ret.push_back(boolean());
	if (has & HFLOAT) ret.push_back(fp());
	if (has & HINT) ret.push_back(integer());
	if (has & HTEXT) ret.push_back(text());
	if (has & HAREL) ret.push_back(aRel());
	if (has & HATUP) ret.push_back(aTup());
	if (has & HAFUNC) ret.push_back(aFunc());

	for (auto && part: rparts) {
		switch(part.kind()) {
		case Rel: 
			if ((has & HAREL) == 0) ret.emplace_back(std::move(part));
			break;
		case Tup:
			if ((has & HATUP) == 0) ret.emplace_back(std::move(part));
			break;
		case Func:
			if ((has & HAFUNC) == 0) ret.emplace_back(std::move(part));
			break;
		default:
			break;
		}
	}

	if (ret.size() == 0) 
		return invalid();
	if (ret.size() == 1) 
		return std::move(ret[0]);
	
	union {uint64_t bb; Container * pp; };
	pp = new DisjunctionC(std::move(ret));
	assert((bb >> 56) == 0);
	bb |= (uint64_t)Disjunction << 56;
	return Type(bb);
}

PlainType Type::disjunctionPlain() const {
	assert(kind() == Disjunction);
	const int HBOOL=1;
	const int HFLOAT=2;
	const int HINT=4;
	const int HTEXT=8;
	const int HREL=16;
	const int HTUP=32;
	const int HFUNC=64;
	const int HANY=127;
	int has=0;
	for (const auto & part: disjunctionParts()) {
		switch(part.kind()) {
		case Invalid: return TInvalid;
		case Any: has |= HANY; break;
		case Int: has |= HINT; break;
		case Float: has |= HFLOAT; break;
		case Bool: has |= HBOOL; break;
		case Text: has |= HTEXT; break;
		case Rel: has |= HREL; break;
		case ARel: has |= HREL; break;
		case Func: has |= HTUP; break;
		case ATup: has |= HTUP; break;
		case Tup: has |= HTUP; break;
		case AFunc: has |= HFUNC; break;
		case Disjunction:
			assert(false);
			break;
		}
	}

	switch(has) {
	case 0: return TInvalid;
	case HBOOL:	return TBool;
	case HFLOAT: return TFloat;
	case HINT: return TInt;
	case HTEXT: return TText;
	case HREL: return TRel;
	case HTUP: return TTup;
	case HFUNC: return TFunc;
	default: return TAny;
	}
}

void Type::destroy() {
	switch(kind()) {
	case Func:
		assert(p()->refCnt==0);
		delete static_cast<FuncC*>(p());
		break;
	case Tup:
	case Rel:
		assert(p()->refCnt==0);
		delete static_cast<SchemaC*>(p());
		break;
	case Disjunction:
		assert(p()->refCnt==0);
		delete static_cast<DisjunctionC*>(p());
		break;
	default:
		assert(false);
	}
}

std::ostream & operator<<(std::ostream & o, const Type & t) {
	switch (t.kind()) {
	case Type::Invalid: return o << "Invalid";
	case Type::Any: return o << "Any";
	case Type::Int: return o << "Int";
	case Type::Float: return o << "Float";
	case Type::Bool: return o << "Bool";
	case Type::Text: return o << "Text";
	case Type::ARel: return o << "Rel";
	case Type::ATup: return o << "Tup";
	case Type::AFunc: return o << "Func";
	case Type::Rel: 
		o << "Rel";
		outputSchema(o, t.relTupSchema());
		return o;
	case Type::Func: {
		o << "Func(";
		bool first=true;
		for (const auto & e: t.funcArgs()) {
			if (first) first=false;
			else o << ", ";
			o << e;
		}
		return o << ")->(" << t.funcRet() << ")";
	}
	case Type::Tup: 
		o << "Tup";
		outputSchema(o, t.relTupSchema());
		return o;
	case Type::Disjunction: {
 		bool first=true;
 		for (const auto & e: t.disjunctionParts()) {
 			if (first) first=false;
 			else o << " | ";
			o << e;
		}
		return o;
	}
	}
	return o;
}

bool matchSchema(
	const std::map<std::string, Type> & lhs, 
	const std::map<std::string, Type> & rhs) {

	auto il=lhs.begin();
	auto ir=rhs.begin();
	while (il != lhs.end() && ir != rhs.end()) {
		if (il->first != ir->first) return false;
		if (!Type::match(il->second, ir->second)) return false;
		++il;
		++ir;
	}
	return il == lhs.end() && ir == rhs.end();
}
						

bool Type::match(const Type & lhs, const Type & rhs) {
	const Kind lb=lhs.kind();
	const Kind rb=rhs.kind();
	if (lb == Disjunction) {
		for (const auto & part: lhs.disjunctionParts()) 
			if (match(part, rhs)) return true;
		return false;
	}
	if (rb == Disjunction) {
		for (const auto & part: rhs.disjunctionParts()) 
			if (match(lhs, part)) return true;
		return false;
	}
	if (lb == Any || rb == Any || rb == Invalid || rb == Invalid) 
		return true;

	switch (lb) {
	case Disjunction:
	case Invalid:
	case Any:
		assert(false);
		return true;
	case Int: return rb == Int;
	case Float: return rb == Float;
	case Bool: return rb == Bool;
	case Text: return rb == Text;
	case ARel: return rb == Rel || rb == ARel;
	case ATup: return rb == Tup || rb == ATup;
	case AFunc: return rb == Func || rb == AFunc;
	case Rel:
		return rb == ARel || (rb == Rel && matchSchema(lhs.relTupSchema(), rhs.relTupSchema()));
	case Tup:
		return rb == ATup || (rb == Tup && matchSchema(lhs.relTupSchema(), rhs.relTupSchema()));
	case Func: {
		if (rb == AFunc) return true;
		if (rb != Func) return false;
		if (!match(lhs.funcRet(), rhs.funcRet())) return false;
		const auto & la = lhs.funcArgs();
		const auto & ra = rhs.funcArgs();
		if (la.size() != ra.size()) return false;
		for (size_t i=0; i < la.size(); ++i) 
			if (!match(la[i], ra[i])) return false;
		return true;
	}
	}
	assert(false);
	return true;
}

} //namespace frontend
} //namespace rasmus

