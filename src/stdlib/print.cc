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
#include <iostream>
#include <stdint.h>
#include "lib.h"
#include <shared/type.hh>
#include <stdlib/callback.hh>

extern "C" {
using namespace rasmus::stdlib;

void rm_print(uint8_t t, int64_t v) {
    switch (Type(t)) {
    case TBool:
		callback->printBool(v);
		break;
    case TInt:
		callback->printInt(v);
		break;
    case TText: //It's a Text
		callback->printText((rm_object*)v);
		break;
    case TRel:
		callback->printRel((rm_object*)v);
		break;
    case TTup:
		callback->printTup((rm_object*)v);
		break;
    case TFunc:
		callback->printFunc((rm_object*)v);
		break;
    default:
		callback->reportMessage("ILE: rm_print called on unhandled type");
		rm_abort();
		break;
    }
}

} //extern "C"
