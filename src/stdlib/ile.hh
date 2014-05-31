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

namespace rasmus {
namespace stdlib {

inline void ile_append(std::ostream &) {}

template <typename T, typename ... TT>
inline void ile_append(std::ostream & o, const T & t, const TT & ... tt) {
	o << " " << t;
	ile_append(o, tt...);
}

template <typename ... T>
inline void ile [[noreturn]] (const char * file, 
							  const char * function,
							  size_t line,
							  const T & ... t) {
	std::stringstream ss;
	ss << file << ":" << line << "(" << function << "):";
	ile_append(ss, t...);
	callback->reportError(0, 0, ss.str().c_str());
	__builtin_unreachable();
}

#define ILE(...) ile(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

} //namespace stdlib
} //namespace rasmus


