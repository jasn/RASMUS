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

enum class Type: uint16_t {
	smallText, concatText, substrText, canonicalText
};

struct rm_object {
	uint32_t ref_cnt;
	const Type type;
	rm_object(Type type): ref_cnt(0), type(type) {}

	void incref()  {ref_cnt++;}
	
	void decref() {
		ref_cnt--;
		if (ref_cnt == 0) rm_free(this);
	}
};


class RefPtr {
public:
	explicit RefPtr(rm_object * p=0): ptr(0) {reset(p);}
	RefPtr(const RefPtr & p): ptr(0) {reset(p.get());}
	RefPtr(RefPtr && p): ptr(p.ptr) {p.ptr=nullptr;}
	~RefPtr() {reset(0);}
	RefPtr & operator=(const RefPtr & p)  {reset(p.get());	return *this; }
	RefPtr & operator=(RefPtr && p)  {
		if (ptr) ptr->decref();
		ptr = p.ptr;
		p.ptr = nullptr;
		return *this;
	}
	rm_object & operator*() const {return *ptr;}
	rm_object * operator->() const {return ptr;}
	rm_object * get() const  {return ptr;}

	bool operator <(const RefPtr & o) const {return ptr < o.ptr;}
	bool operator >(const RefPtr & o) const {return ptr > o.ptr;}
	bool operator <=(const RefPtr & o) const {return ptr <= o.ptr;}
	bool operator >=(const RefPtr & o) const {return ptr >= o.ptr;}
	bool operator ==(const RefPtr & o) const {return ptr == o.ptr;}
	bool operator !=(const RefPtr & o) const {return ptr == o.ptr;}
	bool operator!() const {return !ptr;}
	
	void reset(rm_object * p=nullptr) {
		if (p) p->incref();
		if (ptr) ptr->decref();
		ptr = p;
	}

	static RefPtr steal(rm_object * p) {
		RefPtr r(p);
		p->decref();
		return r;
	}
private:
	rm_object * ptr;
};

struct TextBase: public rm_object {
	const size_t length;
	TextBase(Type type, size_t length):
		rm_object(type), length(length) {}
};

struct SmallText: public TextBase {
	SmallText(size_t length): TextBase(Type::smallText, length) {}
	char data[0];
};

struct ConcatText: public TextBase {
	ConcatText(RefPtr left, RefPtr right): 
		TextBase(Type::concatText, 
				 static_cast<TextBase*>(left.get())->length +
				 static_cast<TextBase*>(right.get())->length),
		left(left), right(right) {}
	const RefPtr left;
	const RefPtr right;
};

struct SubstrText: public TextBase {
	SubstrText(RefPtr content, size_t start, size_t end):
		TextBase(Type::substrText, end-start),
		content(content),
		start(start) {}
	
	const RefPtr content;
	const size_t start;
};

struct CanonicalText: public TextBase {
	CanonicalText(size_t length): TextBase(Type::canonicalText, length), data(nullptr) {
		data=new char[length];
	}
	CanonicalText(size_t length, char * data): TextBase(Type::canonicalText, length), data(data) {}
	
	~CanonicalText() {
		delete[] data;
	}
	char * data;
};

#endif //__INNER_H__
