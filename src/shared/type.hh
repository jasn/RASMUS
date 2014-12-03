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
#ifndef __type_hh__
#define __type_hh__
#include <cstdint>
#include <ostream>

enum class LType: std::uint16_t {
	smallText, concatText, substrText, canonicalText, function, schema, tuple, relation, undefText
};

inline const char * ltypeName(LType t) {
	switch(t) {
	case LType::undefText: return "UndefText";
	case LType::smallText: return "SmallText";
	case LType::concatText: return "ConcatText";
	case LType::substrText: return "SubstrText";
	case LType::canonicalText: return "CanonicalText";
	case LType::function: return "Function";
	case LType::schema: return "Schema";
	case LType::tuple: return "Tuple";
	case LType::relation: return "Relation";
	}
	return "Unknown";
}

inline std::ostream & operator << (std::ostream & o, LType t) {
	return o << ltypeName(t);
}

/**
 * Represents the plain user visible types
 */
enum PlainType {
/// Boolean type: true, false or ?-Bool
	TBool, 
/// Integer type e.g. -1, 5 or ?-Int
	TInt,  
/// Floating type e.g. -1.7, 5e7 or ?-Float
	TFloat,
/// Text type e.g. "monkey" or ?-Text 
	TText,
/// Relation type e.g. rel(tup(a:1))  
	TRel, 
/// Tuple type e.g. tup(a:1)
	TTup, 
/// Function type e.g. fun(a:Int)->(Any) a end
	TFunc, 
/// This type is given to a statement that cannot have a type
	TInvalid, 
/// Any type e.g. true, -5, "Monkey"
	TAny,
};

inline const char * plainTypeName(PlainType t) {
	switch(t) {
	case TBool: return "Bool";
	case TInt: return "Int";
	case TFloat: return "Float";
	case TInvalid: return "Invalid";
	case TText: return "Text";
	case TRel: return "Rel";
	case TTup: return "Tup";
	case TFunc: return "Func";
	case TAny: return "Any";
	}
	return "Unknown";
}

inline std::ostream & operator<<(std::ostream & o, PlainType t) {
	return o << plainTypeName(t);
}

#endif //__type_hh__
