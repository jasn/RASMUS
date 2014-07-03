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
#include <limits>

static const int8_t RM_TRUE = 3;
static const int8_t RM_FALSE = 0;

// TODO Q what should we call these constants?
static const int64_t RM_NULLINT = std::numeric_limits<int64_t>::min();
static const int8_t RM_NULLBOOL = 2;


/**
 * This header contains the public C interface boundery between the stdlib and
 * the compiler (codegen)
 */
extern "C" {

/**
 * Baseclass of all objecs used by rasmus
 */
struct rm_object;

/**
 * Type used to encapsulate an any return value
 */
struct AnyRet {
	int64_t value;
	int8_t type;
};

struct TupEntry {
	const char * name;
	int64_t value;
	int8_t type;
};

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  text.cc  //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Generate a text constant, the returned value has refcnt 1
 *
 * @param cptr A zero terminated cstring
 */
rm_object * rm_getConstText(const char *cptr); 

/**
 * Concatinate two text strings, the returned value has refcnt 1
 */
rm_object * rm_concatText(rm_object * p1, rm_object * p2);

/**
 * Get the substring of a text, the returned value has refcnt 1
 */
rm_object * rm_substrText(rm_object * str, int64_t start, int64_t end);

extern rm_object undef_text;

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////  rm_object.cc  ////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Free an arbetrery rasmus object
 * Note that ptr->ref_ctr must be zero
 */
void rm_free(rm_object * ptr);

/**
 * Create a new function rm_object, the returned object
 * will have ref_cnt=0
 * 
 * @param The size in bytes of the function object
 */
rm_object * rm_createFunction(uint32_t size);

int64_t rm_length(rm_object * obj);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  print.cc  /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Print an object of type t
 * the value is any castet into v
 */
void rm_print(uint8_t t, int64_t v);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  error.cc  /////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * Emit a type error
 *
 * @param start the first character in the source where the error occurs
 * @param end the last character in the source where the error occurs
 * @param got the Type we found
 * @param expected the Type we expected
 */
void rm_emitTypeError [[ noreturn ]] (uint32_t start, uint32_t end, uint8_t got, uint8_t expect);

/**
 * Emit an argument count error
 *
 * @param start the first character in the source where the error occurs
 * @param end the last character in the source where the error occurs
 * @param got The number of arguments we got
 * @param expected the number of arguments we expected
 */
void rm_emitArgCntError [[ noreturn ]] (int32_t start, int32_t end, int16_t got, int16_t expect);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  relation.cc  //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
void rm_saveRel(rm_object * o, const char * name);

rm_object * rm_joinRel(rm_object * lhs, rm_object * rhs);

rm_object * rm_unionRel(rm_object * lhs, rm_object * rhs);

rm_object * rm_loadRel(const char * name);

rm_object * rm_select(rm_object * rel, rm_object * func);

rm_object * rm_createTup(uint32_t count, TupEntry * entries);

rm_object * rm_extendTup(rm_object * lhs, rm_object * rhs);

rm_object * rm_tupRemove(rm_object * tup, const char * name);

uint8_t rm_tupHasEntry(rm_object * tup, const char * name);

void rm_tupEntry(rm_object * tup, const char * name, AnyRet * ret);

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////  global.cc  //////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

/**
 * load a global from the globals database, in case the return object
 * is a rm_object the refcnt is NOT increased
 *
 * @parm id The id of the global to load
 * @param ret Pointer of where to store the result
 */
void rm_loadGlobalAny(uint32_t id, AnyRet * ret);

/**
 * save a global to the globals database. 
 *
 * @param id The if of the global to save
 * @param value The value of the global casted to a int64_t
 * @param type The Type of the global cast to a int8_t
 */
void rm_saveGlobalAny(uint32_t id, int64_t value, int8_t type);

/**
 * clear all storred globals
 */
void rm_clearGlobals();

} //extern C
#endif //__LIB_H__
