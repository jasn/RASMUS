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
#include "error.hh"
#include <limits>
#include <string>
#include <algorithm>
#include <iostream>

namespace {

const char * boldRed = "\033[31;1m";
const char * red = "\033[31m";
const char * reset = "\x1b[0m";
const char * green = "\033[32m";
const char * blue = "\033[34m";
const char * yellow = "\033[33m";
const char * boldYellow = "\033[33;1m";
const char * warningText = "\033[33;1mwarning\x1b[0m";
const char * errorText = "\033[31merror\x1b[0m";

class TerminalError: public Error {
public:
	size_t numberOfErrors;
	std::shared_ptr<Code> code;

	TerminalError(std::shared_ptr<Code> code): numberOfErrors(0), code(code) {}
	
	virtual void reportWarning(std::string message,
							   Token mainToken,
							   std::vector<std::pair<uint32_t, uint32_t> > ranges) override {
		report(warningText, message, mainToken, ranges);
	}
	
	virtual void reportError(std::string message,
							 Token mainToken,
							 std::vector<std::pair<uint32_t, uint32_t> > ranges) override {
		report(errorText, message, mainToken, ranges);
	}

	void report(const char * type, std::string message,
				Token mainToken,
				std::vector<std::pair<uint32_t, uint32_t> > ranges);
};

void TerminalError::report(const char * type, std::string message,
					  Token mainToken,
					  std::vector<std::pair<uint32_t, uint32_t> > ranges) {
	numberOfErrors++;
	uint32_t lo = std::numeric_limits<uint32_t>::max();
	uint32_t hi = std::numeric_limits<uint32_t>::min();
	if (mainToken) {
		lo = std::min(lo, mainToken.start);
		hi = std::max(hi, mainToken.length + mainToken.start);
	}
	for (auto r: ranges) {
		lo = std::min(lo, r.first);
		hi = std::max(hi, r.second);
	}

	auto it=std::upper_bound(code->lineStarts.begin(), code->lineStarts.end(), lo);
	uint32_t line=it - code->lineStarts.begin();
	printf("%s:%d %s %s\n", code->name.c_str(), line, type, message.c_str());
	uint32_t startOfLine = code->lineStarts[line-1]+1;
    uint32_t endOfLine = code->lineStarts[line];
	//std::cout << "ABE " << startOfLine << " " << endOfLine << " " << lo << " " << hi << " " << code->code.length() <<  std::endl;
	printf("%s%s%s\n",green, code->code.substr(startOfLine,endOfLine-startOfLine).c_str(),reset);
	if (ranges.size() == 0 && mainToken) {
		//std::cout << mainToken.start << " - " << startOfLine << std::endl;
		//std::cout << mainToken.length << " " << '~' << std::endl;
		printf("%s%s^%s%s\n", std::string(mainToken.start-startOfLine, ' ').c_str(), blue, std::string(std::max<size_t>(mainToken.length, 1)-1, '~').c_str(), reset);
	} else {
		std::string i(endOfLine - startOfLine, ' ');
		for (auto r: ranges)
			for (uint32_t x=std::max(startOfLine, r.first); x < std::min(endOfLine, r.second); ++x) 
				i[x-startOfLine] = '~';
		if (mainToken)
			i[std::max(uint32_t(0), std::min(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
		printf("%s%s%s\n", blue, i.c_str(), reset);
	}
}

} //anonymous namespace

std::shared_ptr<Error> terminalError(std::shared_ptr<Code> code) {
	return std::make_shared<TerminalError>(code);
}
    
