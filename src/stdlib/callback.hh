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
#ifndef __CALLBACK_HH__
#define __CALLBACK_HH__
#include <stdlib/rm_object.hh>
#include <memory>

namespace rasmus {
namespace stdlib {

class IOException: public std::runtime_error {
public:
	using std::runtime_error::runtime_error;
};

/**
 * Callback used by the stdlib to interact with the environment
 */
class Callback {
public:
	~Callback() {}
	virtual void saveRelation(rm_object * o, const char * name) = 0;
	virtual rm_object * loadRelation(const char * name) = 0;

	virtual void printInt(int64_t v) = 0;
	virtual void printBool(int8_t v) = 0;
	virtual void printText(rm_object * o) = 0;
	virtual void printFunc(rm_object * o) = 0;
	virtual void printTup(rm_object * o) = 0;
	virtual void printRel(rm_object * o) = 0;
	
	virtual void reportError [[noreturn]] (size_t start, size_t end, std::string text) = 0;
	virtual void reportMessage(std::string text) = 0;
};

extern std::shared_ptr<Callback> callback;

} //namespace stdlib
} //namespace rasmus

#endif //__CALLBACK_HH__
