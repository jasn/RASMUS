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
#include <stdlib/lib.h>
using namespace rasmus;

std::string rm_textToString(rm_object * ptr);

bool construct() {
	{
		RefPtr text=RefPtr::steal(rm_getConstText("Hello"));
		ENSURE_EQ(rm_textToString(text.get()), "Hello");
		ENSURE_EQ(text->ref_cnt, 1);
	}
	{
		RefPtr text=RefPtr::steal(rm_getConstText("Hello world!"));
		ENSURE_EQ(rm_textToString(text.get()), "Hello world!");
		ENSURE_EQ(text->ref_cnt, 1);
	}
	{
		RefPtr text=RefPtr::steal(rm_getConstText(" -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-"));
		ENSURE_EQ(rm_textToString(text.get()), " -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-");
		ENSURE_EQ(text->ref_cnt, 1);
	}
	return true;
}

bool concat() {
	{
		//Small string concat
		RefPtr lhs=RefPtr::steal(rm_getConstText("Hey"));
		RefPtr rhs=RefPtr::steal(rm_getConstText(" monkey"));
		{
			RefPtr ans=RefPtr::steal(rm_concatText(lhs.get(), rhs.get()));
			ENSURE_EQ(rm_textToString(ans.get()), "Hey monkey");
			ENSURE_EQ(ans->ref_cnt, 1);
		}
		ENSURE_EQ(lhs->ref_cnt, 1);
		ENSURE_EQ(rhs->ref_cnt, 1);
	}	
	{
		//Large string concat
		RefPtr lhs=RefPtr::steal(rm_getConstText("The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:"));
		RefPtr rhs=RefPtr::steal(rm_getConstText("Global: All three versions of operator delete are declared in the global namespace, not within the std namespace."));
		{
			RefPtr ans=RefPtr::steal(rm_concatText(lhs.get(), rhs.get()));
			ENSURE_EQ(rm_textToString(ans.get()), "The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:Global: All three versions of operator delete are declared in the global namespace, not within the std namespace.");
			ENSURE_EQ(ans->ref_cnt, 1);
		}
		ENSURE_EQ(lhs->ref_cnt, 1);
		ENSURE_EQ(rhs->ref_cnt, 1);
	}
	return true;
}

bool substring() {
	{
		RefPtr txt=RefPtr::steal(rm_getConstText("Minime"));
		{
			RefPtr ans=RefPtr::steal(rm_substrText(txt.get(), 1, 4));
			if (rm_textToString(ans.get()) != "ini") return false;
			ENSURE_EQ(ans->ref_cnt, 1);
		}
		ENSURE_EQ(txt->ref_cnt, 1);
	}
	{
		RefPtr txt=RefPtr::steal(rm_getConstText("The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:"));
		{
			RefPtr ans=RefPtr::steal(rm_substrText(txt.get(), 4, 10000));
			ENSURE_EQ(rm_textToString(ans.get()), "default allocation and deallocation functions are special components of the standard library; They have the following unique properties:");
			ENSURE_EQ(ans->ref_cnt, 1);
		}
		{
			RefPtr ans=RefPtr::steal(rm_substrText(txt.get(), -10, 3));
			ENSURE_EQ(rm_textToString(ans.get()), "The");
			ENSURE_EQ(ans->ref_cnt, 1);
		}
		{
			RefPtr ans=RefPtr::steal(rm_substrText(txt.get(), 4, 3));
			ENSURE_EQ(rm_textToString(ans.get()), "");
			ENSURE_EQ(ans->ref_cnt, 1);	
		}
		ENSURE_EQ(txt->ref_cnt, 1);
	}
	return true;
}

int main(int argc, char **argv) {
  return rasmus::tests(argc, argv)
    .test(construct, "construct")
    .test(concat, "concat")
    .test(substring, "substring")
    ;
}


