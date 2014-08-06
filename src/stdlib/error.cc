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
#include <stdint.h>
#include "lib.h"
#include <shared/type.hh>
#include <sstream>
#include <stdlib/callback.hh>
#include <vector>

extern "C" {
using namespace rasmus::stdlib;

void rm_emitTypeError [[noreturn]] (uint32_t start, uint32_t end, uint8_t got, uint8_t expect) {
	std::stringstream ss;
	ss << "Expected type " << Type(expect) << " but got " << Type(got);
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitArgCntError [[noreturn]] (int32_t start, int32_t end, int16_t got, int16_t expect) {
	std::stringstream ss;
	ss << "Expected " << expect << " arguments but got " << got;
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitColNameError [[noreturn]] (uint32_t begin, uint32_t end, std::string wantedName,
									   std::vector<std::pair<std::string, size_t>> schemaNames) {
	std::stringstream ss;
	ss << "A column with name " << wantedName << " does not exist in the given schema. Valid column names are: ";
	for(size_t k = 0; k < schemaNames.size(); k++){
		ss << schemaNames[k].first;
		if(k != schemaNames.size() - 1)
			ss << ", ";
	}
	callback->reportError(begin, end, ss.str());
	__builtin_unreachable();
}			



} //extern C
