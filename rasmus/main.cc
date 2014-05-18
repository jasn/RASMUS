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
#include "code.hh"
#include "error.hh"
#include "parser.hh"
#include "lexer.hh"
#include <iostream>

std::shared_ptr<Visitor> charRanges(); //Declared in charRanges.cc

int main(int argc, char ** argv) {
	std::shared_ptr<Code> c = std::make_shared<Code>("", "Interpreted");
	std::shared_ptr<Error> e = terminalError(c);
	std::shared_ptr<Lexer> l = std::make_shared<Lexer>(c);
	std::shared_ptr<Parser> p = parser(l, e, true);
	std::shared_ptr<Visitor> cr = charRanges();

	NodePtr ast = std::make_shared<SequenceExp>();
	std::string incomplete;
	std::string theCode;
	while (std::cin) {
		std::string line;
		if (!incomplete.empty())
			std::cout << ".... " << std::flush;
		else
			std::cout << ">>>> " << std::flush;
		std::getline(std::cin, line);
		if (line.empty()) continue;
		c->set(theCode + incomplete + line);
		try {
			NodePtr r=p->parse();
			theCode = c->code+"\n";
			incomplete = "";
			cr->visit(r);
		} catch (IncompleteInputException) {
			incomplete = line + "\n";
		}
	}
	std::cout << std::endl;
}
