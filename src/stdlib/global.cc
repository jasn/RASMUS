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
#include <cstring>
#include <callback.hh>
#include <iostream>
#include <map>
#include <stdlib/anyvalue.hh>
#include <vector>
namespace {

using namespace rasmus::stdlib;

struct mystrcmp {

	bool operator() (const char * lhs, const char * rhs) const {
		return strcmp(lhs, rhs) < 0;
	}

};

std::map<const char *, AnyValue, mystrcmp> globals;

} //nameless namespace

extern "C" {

void rm_loadGlobalAny(const char * name, 
					  AnyRet * ret) {

	AnyValue & v = globals[name];
	ret->type = v.type;
	switch (v.type) {
	case TInt:
		ret->value = v.intValue;
		break;
	case TBool:
		ret->value = v.boolValue;
		break;
	default:
		ret->value = reinterpret_cast<int64_t>(v.objectValue.get());
		break;
	}

}


void rm_saveGlobalAny(const char * name, int64_t value, int8_t type) {
	switch (type) {
	case TInt:
		globals[name] = AnyValue(value);
		break;
	case TBool:
		globals[name] = AnyValue(static_cast<int8_t>(value));
		break;
	default:
		globals[name] = AnyValue((Type)type, 
			RefPtr<rm_object>(reinterpret_cast<rm_object*>(value))
				);
		break;
	}

	callback.environmentChange(name);

}

void rm_clearGlobals() {
	globals.clear();
}

} // extern "C"

