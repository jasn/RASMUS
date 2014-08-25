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
#include "callback.hh"
#include <iostream>
#include <map>
#include <stdlib/anyvalue.hh>
#include <vector>
#include <llvm/Support/FileSystem.h>

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

void rm_registerGlobal(const char * name, rm_object * rel) {
	globals[name] = AnyValue(TRel, RefPtr<rm_object>(rel));
	callback->environmentChanged(name);
}

bool rm_existsGlobalAny(const char * name) {
	return globals.count(name) > 0;
}

void rm_deleteGlobalAny(const char * name) {
	if (globals.count(name) == 0 || TRel == Type(globals[name].type)) {
		rm_deleteRel(name);
	}
	globals.erase(name);
	callback->environmentChanged(name);
}

void rm_loadGlobalAny(const char * name, 
					  AnyRet * ret) {

	if (!globals.count(name)) {
		rm_object * rel = rm_loadRel(name);
		AnyValue av(TRel, RefPtr<rm_object>(rel));
		globals[name] = av;
		callback->environmentChanged(name);
	}
	
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
	case TRel:
		rm_saveRel(reinterpret_cast<rm_object*>(value), name);
		globals[name] = AnyValue(TRel, 
								 RefPtr<rm_object>(reinterpret_cast<rm_object*>(value)));
		break;
	default:
		globals[name] = AnyValue((Type)type, 
			RefPtr<rm_object>(reinterpret_cast<rm_object*>(value))
				);
		break;
	}

	callback->environmentChanged(name);
}

void rm_clearGlobals() {
	globals.clear();
}

} // extern "C"

