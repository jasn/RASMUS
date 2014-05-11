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

#include "inner.hh"
#include <cstring>
#include <iostream>

extern "C" {

  // struct rm_Text {
  //   rm_Text(const char *cptr) : str(cptr) { }
  //   static rm_Text* concat(const rm_Text *lhs, const rm_Text *rhs) {
  //     return new rm_Text(((*lhs).str + (*rhs).str).c_str());
  //   }
  //   bool find(const rm_Text *needle) const {
  //     return str.find(needle->str) != std::string::npos;
  //   }
  //   std::string str;
  // };
  //   return new rm_Text(cptr);    
  // }

rm_object * rm_getConstText(const char *cptr) {
	size_t len = strlen(cptr);
	rm_TextLeaf * o = reinterpret_cast<rm_TextLeaf*>(malloc(sizeof(rm_TextLeaf)+ len));
	new(o) rm_TextLeaf;
	o->type = (uint16_t)types::text;
	o->subtype = (uint16_t)texts::leaf;
	o->length = len;
	memcpy(o->data, cptr, len);
	return o;
}

rm_object * rm_concatText(rm_object *lhs, rm_object *rhs) {
	//TODO if the objects are small or have about the same size then we could just do the concat
	rm_TextConcat * cc=new rm_TextConcat();
	cc->type = (uint16_t)types::text;
	cc->subtype = (uint16_t)texts::concat;
	cc->left = ref_ptr<>(lhs);
	cc->right = ref_ptr<>(rhs);
	return cc;
}

// int8_t rm_substringSearch(const void *lhs, const void *rhs) {
//     const rm_Text *lhst = reinterpret_cast<const rm_Text*>(lhs);
//     const rm_Text *rhst = reinterpret_cast<const rm_Text*>(rhs);
// 	return rhst->find(lhst);
// }

void rm_printText(rm_object * ptr) {
	
    /*std::cout << reinterpret_cast<rm_Text*>(ptr)->str << std::endl;*/
}

}
