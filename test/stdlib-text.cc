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
#include <stdlib/text.hh>
#include <stdlib/refptr.hh>
#include <stdlib/lib.h>
using namespace rasmus;
using namespace rasmus::stdlib;

bool construct() {
	{
		RefPtr<rm_object> text=RefPtr<rm_object>::steal(rm_getConstText("Hello"));
		ensure_eq(textToString(text.getAs<TextBase>()), "Hello");
		ensure_eq(text->ref_cnt, 1);
	}
	{
		RefPtr<rm_object> text=RefPtr<rm_object>::steal(rm_getConstText("Hello world!"));
		ensure_eq(textToString(text.getAs<TextBase>()), "Hello world!");
		ensure_eq(text->ref_cnt, 1);
	}
	{
		RefPtr<rm_object> text=RefPtr<rm_object>::steal(rm_getConstText(" -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-"));
		ensure_eq(textToString(text.getAs<TextBase>()), " -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style \"stroustrup\") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-");
		ensure_eq(text->ref_cnt, 1);
	}
	return true;
}

bool concat() {
	{
		//Small string concat
		RefPtr<rm_object> lhs=RefPtr<rm_object>::steal(rm_getConstText("Hey"));
		RefPtr<rm_object> rhs=RefPtr<rm_object>::steal(rm_getConstText(" monkey"));
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_concatText(lhs.getAs<TextBase>(), rhs.getAs<TextBase>()));
			
			ensure_eq(ans->ref_cnt, 1);
		}
		ensure_eq(lhs->ref_cnt, 1);
		ensure_eq(rhs->ref_cnt, 1);
	}	
	{
		//Large string concat
		RefPtr<rm_object> lhs=RefPtr<rm_object>::steal(rm_getConstText("The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:"));
		RefPtr<rm_object> rhs=RefPtr<rm_object>::steal(rm_getConstText("Global: All three versions of operator delete are declared in the global namespace, not within the std namespace."));
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_concatText(lhs.getAs<TextBase>(), rhs.getAs<TextBase>()));
			ensure_eq(textToString(ans.getAs<TextBase>()), "The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:Global: All three versions of operator delete are declared in the global namespace, not within the std namespace.");
			ensure_eq(ans->ref_cnt, 1);
		}
		ensure_eq(lhs->ref_cnt, 1);
		ensure_eq(rhs->ref_cnt, 1);
	}
	return true;
}

bool substring() {
	{
		RefPtr<rm_object> txt=RefPtr<rm_object>::steal(rm_getConstText("Minime"));
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_substrText(txt.getAs<TextBase>(), 1, 4));
			ensure_eq(textToString(ans.getAs<TextBase>()), "ini");
			ensure_eq(ans->ref_cnt, 1);
		}
		ensure_eq(txt->ref_cnt, 1);
	}
	{
		RefPtr<rm_object> txt=RefPtr<rm_object>::steal(rm_getConstText("The default allocation and deallocation functions are special components of the standard library; They have the following unique properties:"));
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_substrText(txt.getAs<TextBase>(), 4, 10000));
			ensure_eq(textToString(ans.getAs<TextBase>()), "default allocation and deallocation functions are special components of the standard library; They have the following unique properties:");
			ensure_eq(ans->ref_cnt, 1);
		}
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_substrText(txt.getAs<TextBase>(), -10, 3));
			ensure_eq(textToString(ans.getAs<TextBase>()), "The");
			ensure_eq(ans->ref_cnt, 1);
		}
		{
			RefPtr<rm_object> ans=RefPtr<rm_object>::steal(rm_substrText(txt.getAs<TextBase>(), 4, 3));
			ensure_eq(textToString(ans.getAs<TextBase>()), "");
			ensure_eq(ans->ref_cnt, 1);	
		}
		ensure_eq(txt->ref_cnt, 1);
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


