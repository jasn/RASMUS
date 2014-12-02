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
#ifndef __STRONGTYPE_HH__
#define __STRONGTYPE_HH__

#include <stdlib/refptr.hh>
#include <shared/type.hh>
#include <stdlib.h>
#include <vector>
#include <map>
#include <ostream>
namespace rasmus {
namespace frontend {

class Type {
public:
	class Container {
	public: 
		uint64_t refCnt;
	};

	enum Kind : int8_t {
		Empty, Any, Int, Float, Bool, Text, ARel, ATup, AFunc, Rel, Func, Tup, Union
	};

	Type(): bits(0) {}
	~Type() {reset(0);}
	Type(Type && o): bits(o.bits) {o.bits = 0;}
	Type(const Type & o): bits(0) {reset(o.bits); }
	Type & operator=(const Type & o) {reset(o.bits); return *this; }
	Type & operator=(Type && o) {reset(o.bits);	return *this; }

	explicit Type(PlainType t): bits(0) {
		switch (t) {
		case TAny: bits = (uint64_t)Any << shift; break;
		case TInt: bits = (uint64_t)Int << shift; break;
		case TFloat: bits = (uint64_t)Float << shift; break;
		case TBool: bits = (uint64_t)Bool << shift; break;
		case TText: bits = (uint64_t)Text << shift; break;
		case TRel: bits = (uint64_t)ARel << shift; break;
		case TTup: bits = (uint64_t)ATup << shift; break;
		case TFunc: bits = (uint64_t)AFunc << shift; break;
		case TInvalid:
			bits = (uint64_t)Empty << shift;
			break;
		}
	}
	
	Kind kind() const {
		return (Kind)(bits >> shift);
	}
	
	PlainType plain() const {
		switch (kind()) {
		case Any: return TAny;
		case Int: return TInt;
		case Float: return TFloat;
		case Bool: return TBool;
		case Text: return TText;
		case ARel: return TRel;
		case Rel: return TRel;
		case ATup: return TTup;
		case Tup: return TTup;
		case AFunc: return TFunc;
		case Func: return TFunc;
		case Union: return unionPlain();
		default: return TInvalid;
		}
	}

	bool isEmpty() const {return kind() == Empty;}
	bool isNoneEmpty() const {return kind() == Empty;}

	const Type & funcRet() const;
	const std::vector<Type> & funcArgs() const;
	// We guarentee that disjunctions are relativly minimal, and that they do not contain other disjunctions
	const std::vector<Type> & unionParts() const;
	const std::map<std::string, Type> & relTupSchema() const;
	
	static Type empty() {return Type((uint64_t)Empty << shift);}
	static Type any() {return Type((uint64_t)Any << shift);}
	static Type integer() {return Type((uint64_t)Int << shift);}
	static Type fp() {return Type((uint64_t)Float << shift);}
	static Type fpAndInt() {return join({fp(), integer()});}
	static Type boolean() {return Type((uint64_t)Bool << shift);}
	static Type text() {return Type((uint64_t)Text << shift);}
	static Type aRel() {return Type((uint64_t)ARel << shift);}
	static Type aTup() {return Type((uint64_t)ATup << shift);}
	static Type aFunc() {return Type((uint64_t)AFunc << shift);}
	static Type atomic() {return join({fp(), integer(), text(), boolean()});}
	static Type rel(std::map<std::string, Type> schema);
	static Type tup(std::map<std::string, Type> schema);
	static Type func(Type ret, std::vector<Type> args);
	static Type join(std::vector<Type> parts);
	static Type intersection(std::vector<Type> parts);

	static bool intersects(const Type & lhs, const Type & rhs);

	/**
	 * \brief Check if lhs is a specialization of rhs
	 *
	 * Check where the set of types lhs in a subset of the set of types rhs
	 */
	static bool subset(const Type & lhs, const Type & rhs);


	friend bool operator==(const Type & lhs, const Type & rhs);
	inline friend bool operator!=(const Type & lhs, const Type & rhs) {
		return !(lhs == rhs);
	}
private:
	static const size_t shift=56;

	// We assume that only the bottom 56-bits of a pointer are none-zero
	// See http://en.wikipedia.org/wiki/X86-64#Virtual_address_space_details

	Container * p() const {
		union {uint64_t bb; Container * pp; };
		bb = bits & 0x7FFFFFFFFFFFFFll;
		return pp;
	}

	void reset(uint64_t newBits) {
		if (newBits == bits) return;
		switch (kind()) {
		case Func:
		case Union:
		case Rel:
		case Tup:
			p()->refCnt--;
			if (p()->refCnt == 0) destroy();
			break;
		default:
			break;
		}
		bits = newBits;
		switch (kind()) {
		case Func:
		case Union:
		case Rel:
		case Tup:
			p()->refCnt++;
			break;
		default:
			break;
		}
	}

	static bool schemaSubset(const Type & lhs, const Type & rhs);
	static bool funcSubset(const Type & lhs, const Type & rhs);
	PlainType unionPlain() const; 
	void destroy();
	explicit Type(uint64_t b): bits(b) {}
	uint64_t bits;
};

std::ostream & operator<<(std::ostream &, const Type &);

} //namespace frontend
} //namespace rasmus

#endif //__STRONGTYPE_HH__



