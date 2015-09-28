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
#ifndef __TEXT_HH__
#define __TEXT_HH__
#include "lib.h"
#include <stdlib/rm_object.hh>
#include <stdlib/refptr.hh>

namespace rasmus {
namespace stdlib {

struct TextBase: public rm_object {
	const size_t length;
	TextBase(LType type, size_t length):
		rm_object(type), length(length) {}
};

struct SmallText: public TextBase {
	SmallText(size_t length): TextBase(LType::smallText, length) {}
	char data[];
};

struct ConcatText: public TextBase {
	ConcatText(RefPtr<TextBase> left, RefPtr<TextBase> right): 
		TextBase(LType::concatText, 
				 left.get()->length +
				 right.get()->length),
		left(std::move(left)), right(std::move(right)) {}
	const RefPtr<TextBase> left;
	const RefPtr<TextBase> right;
};

struct SubstrText: public TextBase {
	SubstrText(RefPtr<TextBase> content, size_t start, size_t end):
		TextBase(LType::substrText, end-start),
		content(std::move(content)),
		start(start) {}
	
	const RefPtr<TextBase> content;
	const size_t start;
};

struct CanonicalText: public TextBase {
	CanonicalText(size_t length): TextBase(LType::canonicalText, length), data(nullptr) {
		data=new char[length];
	}
	CanonicalText(size_t length, char * data): TextBase(LType::canonicalText, length), data(data) {}
	
	~CanonicalText() {
		delete[] data;
	}
	char * data;
};

/**
 * Convert a text object to a std::string
 */
std::string textToString(TextBase * text);

/**
 * Print a text to a stream
 */
void printTextToStream(TextBase * text, std::ostream & out);

/**
 * Print a text to a stream if it is not ?-Text it will be wrapped in "",s and quoted
 */
void printQuoteTextToStream(TextBase * ptr, std::ostream & stream);

} //namespace rasmus
} //namespace stdlib
#endif //__TEXT_HH__

