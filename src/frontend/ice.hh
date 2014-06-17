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
#include <stdlib/callback.hh>
#include <sstream>
#include <exception>
#include <frontend/lexer.hh>
#include <frontend/ASTBase.hh>

namespace rasmus {
namespace frontend {

class CharRange {
public:
	size_t lo, hi;
	CharRange(): lo(std::numeric_limits<size_t>::max()), hi(std::numeric_limits<size_t>::min()) {}
	CharRange(size_t lo, size_t hi): lo(lo), hi(hi) {}
};


class ICEException: public std::runtime_error {
public:
	Token mainToken;
	std::vector<CharRange> ranges;
	ICEException(const std::string & msg, Token mainToken, std::vector<CharRange> ranges): 
		std::runtime_error(msg), mainToken(mainToken), ranges(ranges) {}
};


struct ice_help {
	Token mainToken;
	std::vector<CharRange> ranges;
};

inline void ice_append(std::ostream &, ice_help &) {}

template <typename T, typename ... TT>
inline void ice_append(std::ostream & o, ice_help & h, const T & t, const TT & ... tt) {
	o << " " << t;
	ice_append(o, h, tt...);
}

template <typename ... TT>
inline void ice_append(std::ostream & o, ice_help & h, Token t, const TT & ... tt) {
	if (!h.mainToken) h.mainToken=t;
	else h.ranges.push_back(CharRange(t.start, t.start+t.length));
	ice_append(o, h, tt...);
}

template <typename ... TT>
inline void ice_append(std::ostream & o, ice_help & h, CharRange r, const TT & ... tt) {
	h.ranges.push_back(r);
	ice_append(o, h, tt...);
}


template <typename ... T>
inline void ice [[noreturn]] (const char * file, 
							  const char * function,
							  size_t line,
							  const T & ... t) {
	std::stringstream ss;
	ss << file << ":" << line << "(" << function << "):";
	ice_help h;
	ice_append(ss, h, t...);
	throw ICEException(ss.str(), h.mainToken, h.ranges);
}

#define ICE(...) ice(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

} //namespace frontend
} //namespace rasmus


