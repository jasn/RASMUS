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
#include "common.hh"
#include <frontend/strongType.hh>
using namespace rasmus;
using namespace rasmus::frontend;

Type inv=Type::invalid();
Type i=Type::integer();
Type b=Type::boolean();
Type a=Type::any();
Type t=Type::text();
Type f=Type::fp();
Type af=Type::aFunc();
Type atu=Type::aTup();
Type ar=Type::aRel();

Type fu(Type ret, std::vector<Type> args) {
	return Type::func(ret, args);
}
Type tu(std::map<std::string, Type> schema) {
	return Type::tup(schema);
}
Type r(std::map<std::string, Type> schema) {
	return Type::rel(schema);
}

template <typename ...T>
Type d(T ... t) {
	return Type::disjunction({t...});
}

void eq(rasmus::teststream & ts, Type lhs,
		Type rhs, 
		bool equal=true) {
	ts << lhs << (equal?" == ":" != ") << rhs
	   << result((lhs == rhs) == equal);
} 


void equality(rasmus::teststream & ts) {
	eq(ts, i, i);
	eq(ts, fu(t, {f, i}), i, false);
	eq(ts, fu(t, {b, f}), fu(t, {b, f}));
	eq(ts, fu(t, {b, i}), fu(t, {b, f}), false);
	eq(ts, fu(f, {b, i}), fu(t, {b, f}), false);
	eq(ts, tu({{"abe", f}}), tu({{"abe", f}}));
	eq(ts, tu({{"abe", f}}), tu({{"bea", f}}), false);
	eq(ts, tu({{"abe", f}}), tu({{"abe", i}}), false);
	eq(ts, r({{"abe", f}}), r({{"abe", f}}));
	eq(ts, r({{"abe", f}}), r({{"bea", f}}), false);
	eq(ts, r({{"abe", f}}), r({{"abe", i}}), false);
	eq(ts, r({{"abe", f}}), tu({{"abe", f}}), false);
	eq(ts, r({{"abe", f}}), tu({{"bea", f}}), false);
	eq(ts, r({{"abe", f}}), tu({{"abe", i}}), false);
}

void sp(rasmus::teststream & ts, Type lhs,
		Type rhs, 
		bool comparable=true) {
	if (comparable) {
		ts << lhs << " < " << rhs
		   << result(Type::specialization(lhs,rhs));
		if (lhs != rhs) {
			ts << rhs << " !< " << lhs
			   << result(!Type::specialization(rhs,lhs));
		} 
	} else {
		ts << lhs << " !< " << rhs
		   << result(!Type::specialization(lhs,rhs));
		ts << rhs << " !< " << lhs
		   << result(!Type::specialization(rhs,lhs));
	}
} 

void specialization(rasmus::teststream & ts) {
	sp(ts, i, i);
	sp(ts, i, a);
	sp(ts, i, d(i,f));
	sp(ts, f, d(i,f));
	sp(ts, i, f, false);

	sp(ts, tu({{"abe", f}}) , tu({{"abe", a}}));
	sp(ts, tu({{"abe", f}}) , tu({{"kat", a}}), false);
	sp(ts, atu, a);
	sp(ts, tu({{"abe", f}, {"kat", b}}), atu);
	sp(ts, tu({{"abe", f}}) , tu({{"abe", f}}));

	sp(ts, r({{"abe", f}}) , r({{"abe", a}}));
	sp(ts, ar, a);
	sp(ts, r({{"abe", f}, {"kat", b}}), ar);
	sp(ts, r({{"abe", f}}) , r({{"abe", f}}));

	sp(ts, r({{"abe", f}}), tu({{"abe", f}}), false);

	sp(ts, af, a);
	sp(ts, fu(b, {a, b}), af);
	sp(ts, fu(b, {i, b}), fu(b, {a, b}));
	sp(ts, fu(i, {a, i}), fu(a, {a, i}));
	sp(ts, fu(i, {a, i}), fu(i, {i, a}), false);
	sp(ts, fu(i, {a, i}), fu(i, {i}), false);
}

void dt(rasmus::teststream & ts, std::vector<Type> lhs, 		
		Type rhs) {
	ts << "disjunction(";
	bool first=true;
	for(auto t: lhs) { 
		if (first) first = false;
		else ts << ", ";
		ts << t;
	}
	ts << ") == " << rhs << result(d(lhs) == rhs);
} 


void disjunction(rasmus::teststream & ts) {
	dt(ts, {i, a}, a);
	dt(ts, {i, i, f}, d(f, i));
	dt(ts, {atu, tu({{"abe", i}})}, atu);
	dt(ts, {tu({{"abe", d(i,f)}}), tu({{"abe", i}})}, tu({{"abe", d(i,f)}}));
}

void ct(rasmus::teststream & ts, std::vector<Type> lhs, 		
		Type rhs) {
	ts << "conjunction(";
	bool first=true;
	for(auto t: lhs) { 
		if (first) first = false;
		else ts << ", ";
		ts << t;
	}
	ts << ") == " << rhs << result(Type::conjunction(lhs) == rhs);
} 


void conjunction(rasmus::teststream & ts) {
	ct(ts, {i, d(i, f)}, i);
	ct(ts, {i, a}, i);
	ct(ts, {fu(a, {a, i}), fu(b, {b, a})}, fu(b, {b, i}) );
	ct(ts, {fu(a, {a, i}), fu(b, {b, b})}, inv);
	ct(ts, {tu({{"abe", i}}), tu({{"abe", a}}), atu, a}, tu({{"abe", i}})); 
	ct(ts, {tu({{"abe", i}}), tu({{"abe", a}}), af, a}, inv);
}

int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
	  .multi_test(equality, "equality")
	  .multi_test(specialization, "specialization")
	  .multi_test(disjunction, "disjunction")
	  .multi_test(conjunction, "conjunction");
}
