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
#include "common.hh"
#include <stdlib/relation.hh>
#include <stdlib/lib.h>
using namespace rasmus;
using namespace rasmus::stdlib;

std::string rm_textToString(rm_object * ptr);


bool printRel(){

	std::cout << std::endl;

	{
		std::string input = "2\n"
			"T Name\n"
			"I Age\n" 
			"Bruce Jones\n" 
			"5\n" 
			"Mary Ross\n" 
			"12\n" 
			"Ann Bird\n" 
			"12\n" 
			"Kenneth Lewis\n" 
			"17\n";

		std::stringstream ss(input);

		rm_object * object = loadRelationFromStream(ss);
	
		printRelationToStream(object, std::cout);

	}

	{
		std::string input = "3\n" 
			"T Name\n"
			"I Age\n" 
			"B Ugly\n" 
			"Bruce Jones\n" 
			"5\n" 
			"true\n" 
			"Mary Ross\n" 
			"12\n" 
  			"true\n" 
			"Ann Bird\n" 
			"12\n" 
			"false\n" 
			"Kenneth Lewis\n" 
			"17\n"
			"true\n";

		std::stringstream ss(input);

		rm_object * object = loadRelationFromStream(ss);
		printRelationToStream(object, std::cout);

	}


	return true; // todo implement actual testing

}


bool loadAndSave(){

	{
		std::string input = "2\n"
			"0 1\n"
			"T Name\n"
			"I Age\n" 
			"Bruce Jones\n" 
			"5\n" 
			"Mary Ross\n" 
			"12\n" 
			"Ann Bird\n" 
			"12\n" 
			"Kenneth Lewis\n" 
			"17\n";

		std::stringstream ss(input);

		rm_object * object = loadRelationFromStream(ss);
	
		std::stringstream ss2;

		saveRelationToStream(object, ss2);

		ensure_eq(ss2.str(), input);

	}

   
	{
		std::string input = "3\n"
			"0 1 2\n"
			"T Name\n"
			"I Age\n" 
			"B Ugly\n" 
			"Bruce Jones\n" 
			"5\n" 
			"true\n" 
			"Mary Ross\n" 
			"12\n" 
  			"true\n" 
			"Ann Bird\n" 
			"12\n" 
			"false\n" 
			"Kenneth Lewis\n" 
			"17\n"
			"true\n";

		std::stringstream ss(input);

		rm_object * object = loadRelationFromStream(ss);
	
		std::stringstream ss2;

		saveRelationToStream(object, ss2);

		ensure_eq(ss2.str(), input);

	}

	return true;
	
}


int main(int argc, char **argv) {
  return rasmus::tests(argc, argv)
    .test(loadAndSave, "loadAndSave")
    .test(printRel, "printRel")
    ;
}


