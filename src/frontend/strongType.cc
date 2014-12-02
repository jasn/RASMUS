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
#include <iostream>
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

class UnionC: public Type::Container {
public:
	UnionC(std::vector<Type> && parts)
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

const std::vector<Type> & Type::unionParts() const {
	assert(kind() == Union);
	return static_cast<const UnionC*>(p())->parts;
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


Type Type::join(std::vector<Type> parts) {
	std::vector<Type> rparts;
	for (auto && part: parts) {
		if (part.kind() == Union) 
			for (const auto & p: part.unionParts())
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
		case Empty: return empty();
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
		case Union:
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
	
	std::vector<Type> rels;
	std::vector<Type> tups;
	std::vector<Type> funcs;

	for (auto && part: rparts) {
		switch(part.kind()) {
		case Rel: 
			if ((has & HAREL) == 0) rels.emplace_back(std::move(part));
			break;
		case Tup:
			if ((has & HATUP) == 0) tups.emplace_back(std::move(part));
			break;
		case Func:
			if ((has & HAFUNC) == 0) funcs.emplace_back(std::move(part));
			break;
		default:
			break;
		}
	}

	
	for (size_t x=0; x != rels.size();) {
		for (size_t y=0; y != rels.size(); ++y) {
			if (x == y || !schemaSubset(rels[x], rels[y])) continue;
			rels.erase(rels.begin()+x);
			goto next1;
		}
		++x;
	next1:	
		;
	}

	for (size_t x=0; x != tups.size();) {
		for (size_t y=0; y != tups.size(); ++y) {
			if (x == y || !schemaSubset(tups[x], tups[y])) continue;
			tups.erase(tups.begin()+x);
			goto next2;
		}
		++x;
	next2:	
		;
	}

	for (size_t x=0; x != funcs.size();) {
		for (size_t y=0; y != funcs.size(); ++y) {
			if (x == y || !funcSubset(funcs[x], funcs[y])) continue;
			funcs.erase(funcs.begin()+x);
			goto next3;
		}
		++x;
	next3:	
		;
	}
	
	ret.insert(ret.end(), rels.begin(), rels.end());
	ret.insert(ret.end(), tups.begin(), tups.end());
	ret.insert(ret.end(), funcs.begin(), funcs.end());

	if (ret.size() == 0) 
		return empty();
	if (ret.size() == 1) 
		return std::move(ret[0]);
	
	union {uint64_t bb; Container * pp; };
	pp = new UnionC(std::move(ret));
	assert((bb >> 56) == 0);
	bb |= (uint64_t)Union << 56;
	return Type(bb);
}

static Type tupIntersection(const std::vector<Type> & parts) {
	bool first=true;
	std::map<std::string, std::vector<Type> > schemas;
	for (const auto & part: parts) {
		if (part.kind() != Type::Tup) continue;
		const std::map<std::string, Type> & schema=part.relTupSchema();
		if (first) {
			for(const auto & p: schema)
				schemas.insert(schemas.end(), std::make_pair(p.first, std::vector<Type>({p.second})));
			first = false;
		} else {
			auto i=schema.begin();
			auto j=schemas.begin();
			for (; i != schema.end() && j != schemas.end(); ++i, ++j) {
				if (i->first != i->first) return Type::empty();
				j->second.push_back(i->second);
			}
			if (i != schema.end() || j != schemas.end()) return Type::empty();
		}
	}

	std::map<std::string, Type> schema;
	for (const auto & p: schemas) {
		Type t=Type::intersection(p.second);
		if (t.isEmpty()) return Type::empty();
		schema.insert(schema.end(), std::make_pair(p.first, t));
	}
	return Type::tup(schema);
}


static Type relIntersection(const std::vector<Type> & parts) {
	bool first=true;
	std::map<std::string, std::vector<Type> > schemas;
	for (const auto & part: parts) {
		if (part.kind() != Type::Rel) continue;
		const std::map<std::string, Type> & schema=part.relTupSchema();
		if (first) {
			for(const auto & p: schema)
				schemas.insert(schemas.end(), std::make_pair(p.first, std::vector<Type>({p.second})));
			first = false;
		} else {
			auto i=schema.begin();
			auto j=schemas.begin();
			for (; i != schema.end() && j != schemas.end(); ++i, ++j) {
				if (i->first != i->first) return Type::empty();
				j->second.push_back(i->second);
			}
			if (i != schema.end() || j != schemas.end()) return Type::empty();
		}
	}

	std::map<std::string, Type> schema;
	for (const auto & p: schemas) {
		Type t=Type::intersection(p.second);
		if (t.isEmpty()) return Type::empty();
		schema.insert(schema.end(), std::make_pair(p.first, t));
	}
	return Type::rel(schema);
}

static Type funcIntersection(const std::vector<Type> & parts) {
	bool first=true;
	std::vector<std::vector<Type> > argss;
	std::vector<Type> rets;
	
	for (const auto & part: parts) {
		if (part.kind() != Type::Func) continue;
		const std::vector<Type> & args=part.funcArgs();
		if (first) {
			argss.resize(args.size());
			first = false;
		} else if (args.size() != argss.size()) 
			return Type::empty();
		rets.push_back(part.funcRet());
		for (size_t i=0; i < args.size(); ++i)
			argss[i].push_back(args[i]);
	}
	
	std::vector<Type> args;
	for (const auto & arg: argss) {
		Type t=Type::intersection(arg);
		if (t.isEmpty()) return Type::empty();
		args.push_back(t);
	}
	Type ret=Type::intersection(rets);
	if (ret.isEmpty()) return Type::empty();
	
	return Type::func(ret, args);
}

static Type intersectionInner(std::vector<Type> & parts) {
	// Recursive sub expression to get rid of disjunctions
	for (auto & part: parts) {
		if (part.kind() != Type::Union) continue;
		std::vector<Type> ans;
		Type t=part;
		for (const auto & p: t.unionParts()) {
			part=p; //Owerride the disjunction with one of its members
			Type r=intersectionInner(parts);
			if (!r.isEmpty()) ans.emplace_back(std::move(r));
		}
		return Type::join(ans);
	}

	Type::Kind kind=Type::Any;
	
	for (const auto & part: parts) {
		switch(part.kind()) {
		case Type::Any:
			break;
		case Type::Empty: 
			kind=Type::Empty; 
			break;
		case Type::Int: 
			kind = (kind == Type::Int || kind == Type::Any) ? Type::Int : Type::Empty; 
			break;
		case Type::Float: 
			kind = (kind == Type::Float || kind == Type::Any) ? Type::Float : Type::Empty; 
			break;
		case Type::Bool: 
			kind = (kind == Type::Bool || kind == Type::Any) ? Type::Bool : Type::Empty; 
			break;
		case Type::Text: 
			kind = (kind == Type::Text || kind == Type::Any) ? Type::Text : Type::Empty; 
			break;
		case Type::ARel: 
			if (kind == Type::Rel) kind = Type::Rel;
			else if (kind == Type::ARel || kind == Type::Any) kind = Type::ARel;
			else kind = Type::Empty;
			break;
		case Type::ATup: 
			if (kind == Type::Tup) kind = Type::Tup;
			else if (kind == Type::ATup || kind == Type::Any) kind = Type::ATup;
			else kind = Type::Empty;
			break;
		case Type::AFunc: 
			if (kind == Type::Func) kind = Type::Func;
			else if (kind == Type::AFunc || kind == Type::Any) kind = Type::AFunc;
			else kind = Type::Empty;
			break;
		case Type::Rel:
			kind = (kind == Type::Rel || kind == Type::ARel || kind == Type::Any) ? Type::Rel : Type::Empty; 
			break;
		case Type::Tup: 
			kind = (kind == Type::Tup || kind == Type::ATup || kind == Type::Any) ? Type::Tup : Type::Empty; 
			break;
		case Type::Func: 
			kind = (kind == Type::Func || kind == Type::AFunc || kind == Type::Any) ? Type::Func : Type::Empty; 
			break;
		case Type::Union:
			assert(false);
			break;
		}
	}

	switch (kind) {
	case Type::Empty: return Type::empty();
	case Type::Any: return Type::any();
	case Type::Int: return Type::integer();
	case Type::Float: return Type::fp();
	case Type::Bool: return Type::boolean();
	case Type::Text: return Type::text();
	case Type::ARel: return Type::aRel();
	case Type::ATup: return Type::aTup();
	case Type::AFunc: return Type::aFunc();
	case Type::Union: assert(false); break;
	case Type::Tup: return tupIntersection(parts);
	case Type::Rel: return relIntersection(parts);
	case Type::Func: return funcIntersection(parts);
	}
	assert(false);
	return Type::empty();
}

Type Type::intersection(std::vector<Type> parts) {
	return intersectionInner(parts);
}

PlainType Type::unionPlain() const {
	assert(kind() == Union);
	const int HBOOL=1;
	const int HFLOAT=2;
	const int HINT=4;
	const int HTEXT=8;
	const int HREL=16;
	const int HTUP=32;
	const int HFUNC=64;
	const int HANY=127;
	int has=0;
	for (const auto & part: unionParts()) {
		switch(part.kind()) {
		case Empty: return TInvalid;
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
		case Union:
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
	case Union:
		assert(p()->refCnt==0);
		delete static_cast<UnionC*>(p());
		break;
	default:
		assert(false);
	}
}

std::ostream & operator<<(std::ostream & o, const Type & t) {
	switch (t.kind()) {
	case Type::Empty: return o << "Invalid";
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
	case Type::Union: {
 		bool first=true;
 		for (const auto & e: t.unionParts()) {
 			if (first) first=false;
 			else o << " | ";
			o << e;
		}
		return o;
	}
	}
	return o;
}

bool intersectsSchema(
	const std::map<std::string, Type> & lhs, 
	const std::map<std::string, Type> & rhs) {

	auto il=lhs.begin();
	auto ir=rhs.begin();
	while (il != lhs.end() && ir != rhs.end()) {
		if (il->first != ir->first) return false;
		if (!Type::intersects(il->second, ir->second)) return false;
		++il;
		++ir;
	}
	return il == lhs.end() && ir == rhs.end();
}
						

bool Type::intersects(const Type & lhs, const Type & rhs) {
	const Kind lb=lhs.kind();
	const Kind rb=rhs.kind();
	if (lb == Union) {
		for (const auto & part: lhs.unionParts()) 
			if (intersects(part, rhs)) return true;
		return false;
	}
	if (rb == Union) {
		for (const auto & part: rhs.unionParts()) 
			if (intersects(lhs, part)) return true;
		return false;
	}
	if (lb == Any || rb == Any || rb == Empty || rb == Empty) 
		return true;

	switch (lb) {
	case Union:
	case Empty:
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
		return rb == ARel || (rb == Rel && intersectsSchema(lhs.relTupSchema(), rhs.relTupSchema()));
	case Tup:
		return rb == ATup || (rb == Tup && intersectsSchema(lhs.relTupSchema(), rhs.relTupSchema()));
	case Func: {
		if (rb == AFunc) return true;
		if (rb != Func) return false;
		if (!intersects(lhs.funcRet(), rhs.funcRet())) return false;
		const auto & la = lhs.funcArgs();
		const auto & ra = rhs.funcArgs();
		if (la.size() != ra.size()) return false;
		for (size_t i=0; i < la.size(); ++i) 
			if (!intersects(la[i], ra[i])) return false;
		return true;
	}
	}
	assert(false);
	return true;
}

bool Type::schemaSubset(const Type & lhs, const Type & rhs) {
	const std::map<std::string, Type> & l = lhs.relTupSchema();
	const std::map<std::string, Type> & r = rhs.relTupSchema();
	auto li=l.begin();
	auto ri=r.begin();
	for(; li != l.end() && ri != r.end(); ++li, ++ri) {
		if (li->first != ri->first) return false;
		Kind lk=li->second.kind();
		Kind rk=ri->second.kind();
		switch (rk) {
		case Any: 
			break;
		case Int:
			if (lk != Int) return false;
			break;
		case Float:
			if (lk != Float) return false;
			break;
		case Text:
			if (lk != Text) return false;
			break;
		case Bool:
			if (lk != Bool) return false;
			break;
		default:
			assert(false);
		}
	}
	return li == l.end() && ri == r.end();
}

bool Type::funcSubset(const Type & lhs, const Type & rhs) {
	assert(lhs.kind() = Func && rhs.kind() == Func);
	const auto & largs=lhs.funcArgs();
	const auto & rargs=rhs.funcArgs();

	if (largs.size() != rargs.size()) 
		return false;

	for (size_t i=0; i < largs.size(); ++i)
		if (!subset(largs[i], rargs[i]))
			return false;
	
	if (!subset(lhs.funcRet(), rhs.funcRet())) 
		return false;
	
	return true;
}


bool Type::subset(const Type & lhs, const Type & rhs) {
	std::vector<Type> lefts, rights; 
	if (lhs.kind() == Union)
		lefts=lhs.unionParts();
	else
		lefts.push_back(lhs); 
	if (rhs.kind() == Union)
		rights = rhs.unionParts();
	else 
		rights.push_back(rhs);

	const int HBOOL=1;
	const int HFLOAT=2;
	const int HINT=4;
	const int HTEXT=8;
	const int HAREL=16;
	const int HATUP=32;
	const int HAFUNC=64;
	const int HANY=127;
	int has=0;
	for (const auto & part: rights) {
		switch(part.kind()) {
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
		case Empty:
			break;
		case Union:
			assert(false);
			break;
		}
	}
	
	// If right is any then we are happy
	if ((has & HANY) == HANY) return true;

	// Every member in left must be a subset of one in right
	for (const auto & l: lefts) {
		switch (l.kind()) {
		case Union:
			assert(false);
		case Empty:
			return true;
		case Any:
			return false;
		case Int:
			if ((has & HINT) == 0) return false;
			break;
		case Float: 
			if ((has & HFLOAT) == 0) return false;
			break;
		case Bool: 
			if ((has & HBOOL) == 0) return false;
			break;
		case Text:
			if ((has & HTEXT) == 0) return false;
			break;
		case ARel: 
			if ((has & HAREL) == 0) return false;
			break;
		case ATup: 
			if ((has & HATUP) == 0) return false;
			break;
		case AFunc:
			if ((has & HAFUNC) == 0) return false;
			break;
		case Rel: {
			if (has & HAREL) break;
			bool ok=false;
			for (const auto & r: rights) {
				if (r.kind() != Rel) continue;
				if (!schemaSubset(l, r)) continue;
				ok=true;
				break;
			}
			if (!ok) return false;
			break;
		}
		case Tup: {
			if (has & HATUP) break;
			if (has & HAREL) break;
			bool ok=false;
			for (const auto & r: rights) {
				if (r.kind() != Tup) continue;
				if (!schemaSubset(l, r)) continue;
				ok=true;
				break;
			}
			if (!ok) return false;
			break;
		}
		case Func: { 
			if (has & HAFUNC) break;
			bool ok=false;
			for (const auto & r: rights) {
				if (r.kind() != Func) continue;
				if (!funcSubset(l, r)) continue;
				ok=true;
				break;
			}
			if (!ok) return false;
		}
		}
	}
	return true;
}

bool operator==(const Type & lhs, const Type & rhs) {
	Type::Kind lk=lhs.kind();
	Type::Kind rk=rhs.kind();
	if (lk != rk) 
		return false;
	switch (lk) {
	case Type::Rel: 
	case Type::Tup:
		return lhs.relTupSchema() == rhs.relTupSchema();
	case Type::Func:
		return lhs.funcRet() == rhs.funcRet() &&
			lhs.funcArgs() == rhs.funcArgs();
	case Type::Union:
		for(const auto & l: lhs.unionParts()) {
			bool ok=false;
			for(const auto & r: rhs.unionParts()) {
				if (l != r) continue;
				ok=true;
				break;
			} 
			if (!ok) return false;
		}
		return true;
	default:
		return true;
	}
	return true;
}

} //namespace frontend
} //namespace rasmus

