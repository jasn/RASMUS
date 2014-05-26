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
#include <stdlib/inner.hh>
using namespace rasmus;

std::string rm_textToString(rm_object * ptr);

bool construct() {
	{
		RefPtr text=RefPtr::steal(rm_getConstText("Hello"));
		if (rm_textToString(text.get()) != "Hello") return false;
		if (text->ref_cnt != 1) return false;
	}
	{
		RefPtr text=RefPtr::steal(rm_getConstText("Hello world!"));
		if (rm_textToString(text.get()) != "Hello world!") return false;
		if (text->ref_cnt != 1) return false;
	}
	{
		RefPtr text=RefPtr::steal(rm_getConstText(" -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-"));
		if (rm_textToString(text.get()) != " -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-") return false;
		if (text->ref_cnt != 1) return false;
	}
	return true;
}

bool concat() {
	return true;
}
bool substring() {
	return true;
}

int main(int argc, char **argv) {
  return rasmus::tests(argc, argv)
    .test(construct, "construct")
    .test(concat, "concat")
    .test(substring, "substring")
    ;
}


