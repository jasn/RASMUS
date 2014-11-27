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
#include <frontend/callback.hh>
#include <stdlib/relation.hh>
#include <llvm/Support/FileSystem.h>
#include <iostream>
#include <algorithm>

namespace {

//const char * boldRed = "\033[31;1m";
//const char * red = "\033[31m";
const char * reset = "\x1b[0m";
const char * green = "\033[32m";
const char * blue = "\033[34m";
//const char * yellow = "\033[33m";
//const char * boldYellow = "\033[33;1m";
const char * warningText = "\033[33;1mwarning\x1b[0m";
const char * errorText = "\033[31merror\x1b[0m";
}

namespace rasmus {
namespace frontend {

void Callback::printInt(int64_t v) {
	std::stringstream ss;
	rasmus::stdlib::printIntToStream(v, ss);
	print(TInt, ss.str());
}

void Callback::printFloat(double v) {
	std::stringstream ss;
	rasmus::stdlib::printFloatToStream(v, ss);
	print(TFloat, ss.str());
}

void Callback::printBool(int8_t v) {
	std::stringstream ss;
	rasmus::stdlib::printBoolToStream(v, ss);
	print(TBool, ss.str());
}

void Callback::printText(rm_object * o) {
	std::stringstream ss;
	rasmus::stdlib::printTextToStream(static_cast<rasmus::stdlib::TextBase *>(o), ss);
	print(TText, ss.str());
}

void Callback::printFunc(rm_object * o) {
	std::stringstream ss;
	ss << "func";
	print(TFunc, ss.str());
}

void Callback::printTup(rm_object * o) {
	std::stringstream ss;
	rasmus::stdlib::printTupleToStream(o, ss);
	print(TTup, ss.str());
}

void Callback::printRel(rm_object * o) {
	std::stringstream ss;
	rasmus::stdlib::printRelationToStream(o, ss);
	print(TRel, ss.str());
}

void TerminalCallback::report(MsgType type, 
							  std::shared_ptr<Code> code,
							  std::string message,
							  Token mainToken,
							  std::vector<CharRange> ranges, 
							  std::string additional) {
	int lo = std::numeric_limits<int>::max();
	int hi = std::numeric_limits<int>::min();
	if (mainToken) {
		lo = std::min<int>(lo, mainToken.start);
		hi = std::max<int>(hi, mainToken.length + mainToken.start);
	}
	for (auto r: ranges) {
		lo = std::min<int>(lo, r.lo);
		hi = std::max<int>(hi, r.hi);
	}
	
	auto it=std::upper_bound(code->lineStarts.begin(), code->lineStarts.end(), lo);
	int line=it - code->lineStarts.begin();

	// if we were given no ranges and mainToken is null, line will be too big
	if(it == code->lineStarts.end() && line > 0)
		line--;
	
	
	std::cerr << code->name.c_str() << ":" << line << " ";
	switch (type) {
	case MsgType::error:
		std::cerr << errorText;
		break;
	case MsgType::warning:
		std::cerr << warningText;
		break;
	default:
		break;
	}
	std::cerr << " " << message << std::endl;
	int startOfLine = code->lineStarts[line-1]+1;
    int endOfLine = code->lineStarts[line];

	std::cout << green
			  << code->code.substr(startOfLine,endOfLine-startOfLine)
			  << reset
			  << std::endl;
	if (ranges.size() == 0 && mainToken) {
		std::cerr << std::string(mainToken.start-startOfLine, ' ') 
				  << blue 
				  << "^"
				  << std::string(std::max<size_t>(mainToken.length, 1)-1, '~')
				  << reset
				  << std::endl;
	} else {
		std::string i(endOfLine - startOfLine, ' ');
		for (auto r: ranges)
			for (int x=std::max<int>(startOfLine, r.lo); x < std::min<int>(endOfLine, r.hi); ++x) 
				i[x-startOfLine] = '~';
		if (mainToken)
			i[std::max<int>(size_t(0), std::min<int>(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
		std::cerr << blue << i << reset << std::endl;
	}
	if (!additional.empty()) 
		std::cerr << additional << std::endl;
}

void TerminalCallback::report(MsgType type, std::string message) {
	switch (type) {
	case MsgType::error:
		std::cerr << errorText << ": " << message << std::endl;
		break;
	case MsgType::warning:
		std::cerr << warningText << ": " << message << std::endl;
		break;
	case MsgType::info:
		std::cerr <<  message << std::endl;
	}
}

void TerminalCallback::print(PlainType, std::string repr) {
	std::cout << repr << std::endl;
}

void TerminalCallback::saveRelation(rm_object * o, const char * name) {
	rasmus::stdlib::saveRelationToFile(o, (std::string(name)+".rdb").c_str());
}

rm_object * TerminalCallback::loadRelation(const char * name) {
	return rasmus::stdlib::loadRelationFromFile((std::string(name)+".rdb").c_str());
}

void TerminalCallback::deleteRelation(const char * name) {
	// TODO: define proper behaviour?
	llvm::sys::fs::remove(std::string(name)+".rdb");
}

bool TerminalCallback::hasRelation(const char * name) {
	return llvm::sys::fs::exists(std::string(name)+".rdb");
}

} //namespace rasmus
} //namespace frontend
