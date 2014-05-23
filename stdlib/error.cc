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
#include <iostream>
#include <stdexcept>
#include "lib.h"
#include "type.hh"

static char * text;

struct TypeError: public std::runtime_error {
	int32_t start;
	int32_t end;
	int8_t got;
	int8_t expect;
	TypeError(int32_t start, int32_t end, int8_t got, int8_t expect)
		: std::runtime_error("TypeError")
		, start(start)
		, end(end)
		, got(got)
		, expect(expect) {}
};

struct ArgCntError: public std::runtime_error {
	int32_t start;
	int32_t end;
	int16_t got;
	int16_t expect;
	ArgCntError(int32_t start, int32_t end, int16_t got, int16_t expect)
		: std::runtime_error("ArgCntError")
		, start(start)
		, end(end)
		, got(got)
		, expect(expect) {}
};


void errorRange(size_t start, size_t end) {
	size_t ls=0;
	size_t line=1;
	size_t cur=0;
	size_t le=0;
	for(;cur < start && text[cur]; ++cur) {
		if (text[cur] != '\n') continue;
		ls=cur+1;
		line++;
	}
	for (le=ls; text[le] && le < end; ++le); //Skip until text end
	for (; text[le] && text[le] != '\n'; ++le); //Skip undil line end
	std::cout << std::string(text+ls,le-ls) << std::endl; //Print the line
	std::cout << std::string(start-ls, ' ') 
			  << "\033[34m" //Blue
			  << std::string(end-start, '~') 
			  << "\x1b[0m"  //Reset 
			  << std::endl;
}

extern "C" {

void rm_emitTypeError(uint32_t start, uint32_t end, uint8_t got, uint8_t expect) {
	std::cout << "Interpreted: \033[31;1merror\x1b[0m excepted type "
			  << Type(expect) << " but got " << Type(got) << std::endl;
	errorRange(start, end);
	throw TypeError(start, end, got, expect);
}

void rm_emitArgCntError(int32_t start, int32_t end, int16_t got, int16_t expect) {
	std::cout << "Interpreted: \033[31;1merror\x1b[0m excepted " 
			  << expect
			  << " arguments but got " << got << std::endl;
	errorRange(start, end);
	throw ArgCntError(start, end, got, expect);
}

int8_t rm_interactiveWrapper(char * txt, void (* fct)() ) {
	try {
		text = txt;
		fct();
    } catch(TypeError) {
		return 1;
    } catch(ArgCntError) {
		return 1;
    }
    return 0;
}

} //extern C
