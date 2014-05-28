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
#include <stdlib/rm_object.hh>
#include <stdlib/function.hh>
#include <stdlib/text.hh>
#include <stdlib/callback.hh>
#include <iostream>

namespace rasmus {
namespace stdlib {

size_t objectCount=0;
size_t getObjectCount() {return objectCount;}
bool debugAllocations = false;


} //namespace stdlib
} //namespace rasmus

extern "C" {
using namespace rasmus::stdlib;

void rm_free(rm_object * o) {
	--rasmus::stdlib::objectCount;
	switch (o->type) {
	case LType::canonicalText:
		if (debugAllocations) callback->reportMessage("free(canonicalText)");
		static_cast<CanonicalText*>(o)->~CanonicalText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::concatText:
		if (debugAllocations) callback->reportMessage("free(concatText)");
		static_cast<ConcatText*>(o)->~ConcatText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::substrText:
		if (debugAllocations) callback->reportMessage("free(substrText)");
		static_cast<SubstrText*>(o)->~SubstrText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::smallText:
		if (debugAllocations) callback->reportMessage("free(smallText)");
		static_cast<SmallText*>(o)->~SmallText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::function: 
	{		
		if (debugAllocations) callback->reportMessage("free(function)");
		function_object * fo = static_cast<function_object*>(o);
		fo->dtor(fo);
		::free(fo); //Functions are allocated with malloc by the codegen
	}
	}
}


void rm_abort() {
	exit(EXIT_FAILURE);
}

} //extern "C"

