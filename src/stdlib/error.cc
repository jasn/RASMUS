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
#include <mutex> 




#include <stdint.h>
#include "lib.h"
#include <shared/type.hh>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdlib/callback.hh>

extern "C" {
using namespace rasmus::stdlib;

void rm_emitTypeError [[noreturn]] (uint32_t start, uint32_t end, uint8_t got, uint8_t expect) {
	std::stringstream ss;
	ss << "Expected type " << Type(expect) << " but got " << Type(got) << ".";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitArgCntError [[noreturn]] (int32_t start, int32_t end, int16_t got, int16_t expect) {
	std::stringstream ss;
	ss << "Expected " << expect << " arguments but got " << got << ".";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void listColumnNames(std::stringstream & ss, std::vector<std::string> schemaNames){
	if(schemaNames.size() == 0)
		ss << "It contains no columns.";
	else if (schemaNames.size() == 1){
		ss << "It contains this column: "
		   << schemaNames[0] << ".";
	}
	else{
		ss << "It contains these columns: ";
		for(size_t k = 0; k < schemaNames.size(); k++){
			ss << schemaNames[k];
			if(k != schemaNames.size() - 1)
				ss << ", ";
		}
		ss << ".";
	}
}

void rm_emitColNameError [[noreturn]] (uint32_t begin, uint32_t end, std::string wantedName,
									   std::vector<std::string> schemaNames) {
	std::stringstream ss;
	ss << "The given relation's schema does not contain a column named " << wantedName << ". ";
	listColumnNames(ss, schemaNames);
	callback->reportError(begin, end, ss.str());
	__builtin_unreachable();
}			

void rm_factorMissingColError [[noreturn]] (uint32_t begin, uint32_t end, std::string wantedName,
											size_t rel_num, std::vector<std::string> schemaNames) {
	std::stringstream ss;
	ss << "Relation number " << rel_num << " does not contain a column named " << wantedName << ". ";
	listColumnNames(ss, schemaNames);
	callback->reportError(begin, end, ss.str());
	__builtin_unreachable();
}

void rm_emitTupColNameError [[noreturn]] (uint32_t begin, uint32_t end, std::string wantedName,
										  std::vector<std::string> schemaNames) {
	std::stringstream ss;
	ss << "The given tuple's schema does not contain a column named " << wantedName << ". ";
	listColumnNames(ss, schemaNames);
	callback->reportError(begin, end, ss.str());
	__builtin_unreachable();
}

void rm_emitFuncDiffSchemasError [[noreturn]] (int32_t start, int32_t end, std::string exp_type){
	std::stringstream ss;
	ss << "The subrelations returned by the " << exp_type
	   << " expression have schemas which are different from each other.";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitColCntError [[noreturn]] (int32_t start, int32_t end, size_t given, size_t max) {
	std::stringstream ss;
	ss << given << (given == 1 ? " column name was" : " column names were") << " given, but ";
	if(max == 0)
		ss << "the relation contains no columns.";
	else
		ss << "the relation only contains " << max << " column" << (max == 1 ? "" : "s") << ".";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitBadCalcTypeError [[noreturn]] (int32_t start, int32_t end, std::string name,
										   Type type, std::string calcType) {
	std::stringstream ss;
	ss << "The given column " << name << " has type " << type
	   << " but " << calcType << " is only supported for integers.";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}


void rm_emitSchemaSizeError [[noreturn]] (int32_t start, int32_t end, size_t s1, size_t s2) {
	std::stringstream ss;
	ss << "The two relations' schemas are not identical. They contain "
	   << s1 << " and " << s2 << " columns, respectively.";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitMissingColError [[noreturn]] (int32_t start, int32_t end, std::string missing_name) {
	std::stringstream ss;
	ss << "The two relations' schemas are not identical. Only one of them contains a column named "
	   << missing_name << ".";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitBadColTypeError [[noreturn]] (int32_t start, int32_t end, std::string name,
										  Type t1, Type t2) {
	std::stringstream ss;
	ss << "The two relations' schemas are not identical. The column named "
	   << name << " has varying types "
	   << t1 << " and " 
	   << t2 << ".";
	callback->reportError(start, end, ss.str());
	__builtin_unreachable();
}

void rm_emitBadCSVFormatError [[noreturn]] () {
	std::cerr << "The first row in the given CSV file could not be parsed "
			  << "because its format is wrong. It is expected that each "
			  << "field in the first row begins with I/T/B followed by a "
			  << "space, to indicate the column type." << std::endl;
	exit(EXIT_FAILURE);
}


} //extern C
