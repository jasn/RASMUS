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
#include <iostream>
#include <stdlib/anyvalue.hh>
#include <vector>
namespace {
using namespace rasmus::stdlib;

std::vector<AnyValue> globals;

} //nameless namespace

extern "C" {


void rm_loadGlobalAny(uint32_t id, 
					  AnyRet * ret) {
	AnyValue & v = globals[id];
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

void rm_saveGlobalAny(uint32_t id, int64_t value, int8_t type) {
	if (globals.size() <= id) 
		globals.resize(id*2+10);
	
	switch (type) {
	case TInt:
		globals[id] = AnyValue(value);
		break;
	case TBool:
		globals[id] = AnyValue(static_cast<int8_t>(value));
		break;
	default:
		globals[id] = AnyValue((Type)type, 
			RefPtr<rm_object>(reinterpret_cast<rm_object*>(value))
				);
		break;
	}	
}

void rm_clearGlobals() {
	globals.clear();
}

} // extern "C"

