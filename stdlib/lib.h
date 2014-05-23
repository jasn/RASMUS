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
#ifndef __LIB_H__
#define __LIB_H__

#include <stdint.h>
#include <stddef.h>

/**
 * This header contains the intier public interface of the RASMUS stdlib
 */
extern "C" {

/**
 * Baseclass of all objecs used by rasmus
 */
struct rm_object;
// struct rm_object {
// 	uint32_t ref_cnt;
// ...
// };

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  text.cc  //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Generate a text constant
 */
rm_object * rm_getConstText(const char *cptr); 

/**
 * Print a text
 */
void rm_printText(rm_object * ptr);

/**
 * Concatinate two text strings
 */
rm_object * rm_concatText(rm_object * p1, rm_object * p2);

/**
 * Free an arbetrery rasmus object
 * Note that ptr->ref_ctr must be zero
 */
void rm_free(rm_object * ptr);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  print.cc  /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void rm_print(uint8_t t, int64_t v);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  error.cc  /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

void rm_emitTypeError(uint32_t start, uint32_t end, uint8_t got, uint8_t expect);

void rm_emitArgCntError(int32_t start, int32_t end, int16_t got, int16_t expect);

int8_t rm_interactiveWrapper(char * txt, void (* fct)() );
  
void rm_printRel(rm_object * ptr);
void rm_saveRel(rm_object * o, const char * name);
rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs);
rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs);
rm_object * rm_loadRel(const char * name);

} //extern C
#endif //__LIB_H__
