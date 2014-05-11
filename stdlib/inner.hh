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
#ifndef __INNER_HH__
#define __INNER_HH__

#include "lib.h"

enum class types: uint16_t {
	text
};

enum class texts: uint16_t {
	leaf, concat
};

inline void incref(rm_object * o) {
	o->ref_cnt++;
}

inline void decref(rm_object * o) {
	o->ref_cnt--;
	if (o->ref_cnt == 0) {
		rm_free(o);
	}
}


template <typename T=rm_object>
struct ref_ptr {
	typedef T element_type;
	
	explicit inline ref_ptr(T * p=0): ptr(0) {
		reset(p);
	}

	inline ref_ptr(const ref_ptr & p): ptr(0) {
		reset(p.get());
	}

	template <typename Y>
	inline ref_ptr(const ref_ptr<Y> & p): ptr(0) {
		reset(p.get());
	}

	inline ref_ptr & operator=(const ref_ptr & p) {
		reset(p.get());
		return *this;
	}

	template <typename Y>
	inline ref_ptr & operator=(const ref_ptr<Y> & p) {
		reset(p.get());
		return *this;
	}

	inline ~ref_ptr() {reset(0);}

	inline T & operator*() const {return *ptr;}

	inline T * operator->() const {return ptr;}

	inline T * get() const {return ptr;}

	inline void reset(T * p=0) {
		if (p) incref(p);
		if (ptr) decref(p);
		ptr = p;
	}

	bool operator <(const ref_ptr<T> & o) const {
		return ptr < o.ptr;
	}

	bool operator ==(const ref_ptr<T> & o) const {
		return ptr == o.ptr;
	}

	bool operator!() const {return !ptr;}

	T * ptr;
};

struct rm_TextLeaf: public rm_object {
	size_t length;
	char data[0];
};

struct rm_TextConcat: public rm_object {
	/*size_t length;*/
	ref_ptr<> left;
	ref_ptr<> right;
};

#endif //__INNER_H__
