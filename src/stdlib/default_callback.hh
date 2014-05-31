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
#ifndef __DEFAULT_CALLBACK_HH__
#define __DEFAULT_CALLBACK_HH__
#include <stdlib/callback.hh>
#include <iostream>
#include <fstream>
#include <stdlib/text.hh>
#include <stdlib/relation.hh>

namespace rasmus {
namespace stdlib {
  
/**
 * Default implementation of the callback
 */
class DefaultCallback: public Callback {
public:
	void saveRelation(rm_object * rel, const char * name) override {
		saveRelationToFile(rel, name);
	}

	rm_object * loadRelation(const char * name) override {
		return loadRelationFromFile(name);
	}

	void printInt(int64_t v) override {
		std::cout << v << std::endl;
	}

	void printBool(int8_t v) override {
		std::cout << (v?"true":"false") << std::endl;
	}
	
	void printText(rm_object * o) override {
		printTextToStream(static_cast<TextBase *>(o), std::cout);
		std::cout << std::endl;
	}

	void printFunc(rm_object * o) override {
		std::cout << "func(" << o << ")" << std::endl;
	}

	void printTup(rm_object * o) override {
		std::cout << "tup(" << o << ")" << std::endl;
	}

	void printRel(rm_object * o) override {
		printRelationToStream(o, std::cout);
	}
	
	void reportError(size_t, size_t, std::string text) override {
		std::cerr << "Error: " << text << std::endl;
		exit(EXIT_FAILURE);
	}
	
	void reportMessage(std::string text) override {
		std::cerr << text << std::endl;
	}
};

} //namespace stdlib
} //namespace rasmus

#endif //__DEFAULT_CALLBACK_HH__
