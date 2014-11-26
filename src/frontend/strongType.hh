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

namespace rasmus {
namespace frontend {
namespace strongType {

enum class BaseType {
	Any, Int, Float, Bool, Text, ARel, ATup, AFunc, Func, Rel, Tup, Disjunction, None
};

class Base {
public:
	Base(BaseType type): m_type(type), ref_cnt(0) {}
	
	void incref() {
		ref_cnt++;
	}
	
	void decref() {
		--ref_cnt;
		if (ref_cnt == 0)
			destroy();
	}
	
	void destroy();
	
	BaseType type() const {
		return m_type;
	}
	
private:
	BaseType m_type;
	size_t ref_cnt;
};

typedef stdlib::RefPtr<Base> Ptr;

class Func: public Base {
public:
	Ptr ret;
	std::vector<Ptr> args;  
	template <typename ...T>
	Func(Ptr ret, T...ts): Base(BaseType::Func), ret(ret), args({ts...}) {}
};

// Logical constructs
class Disjunction: public Base {
public:
	std::vector<Ptr> entries;
  
	Disjunction(std::vector<Ptr> entries): Base(BaseType::Disjunction), entries(entries) {}
};

template <typename ...T>
Ptr func(Ptr ret, T ... args) {return Ptr(new Func(ret, args...));}

inline Ptr fp() {return Ptr(new Base(BaseType::Float));}
inline Ptr none() {return Ptr(new Base(BaseType::None));}
inline Ptr any() {return Ptr(new Base(BaseType::Any));}
inline Ptr integer() {return Ptr(new Base(BaseType::Int));}
inline Ptr boolean() {return Ptr(new Base(BaseType::Bool));}
inline Ptr text() {return Ptr(new Base(BaseType::Text));}
inline Ptr aTup() {return Ptr(new Base(BaseType::ATup));}
inline Ptr aFunc() {return Ptr(new Base(BaseType::AFunc));}
inline Ptr aRel() {return Ptr(new Base(BaseType::ARel));}
inline Ptr disjunction(std::vector<Ptr> p) {
	if (p.size() == 0) 
		return none();
	else if (p.size() == 1)
		return p.front();
	else
		return Ptr(new Disjunction(p));
}

Ptr returnType(const Ptr & type);
Type plain(const Ptr & type);
Ptr strong(Type type);
void output(std::ostream & o, const Ptr & type);
bool match(const Ptr & lhs, const Ptr & rhs);
void check(const Ptr & lhs, const Ptr & rhs);

} //namespace strongType
} //namespace frontend
} //namespace rasmus

#endif //__STRONGTYPE_HH__



