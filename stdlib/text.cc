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

const size_t smallLength = 20;

namespace {

template <typename T>
void buildText(rm_object * o, T & out) {
	switch (o->type) {
	case Type::canonicalText:
		out(static_cast<CanonicalText*>(o)->data, static_cast<CanonicalText*>(o)->length);
		break;
	case Type::smallText:
		out(static_cast<SmallText*>(o)->data, static_cast<SmallText*>(o)->length);
		break;
	case Type::concatText:
		buildText(static_cast<ConcatText*>(o)->left.get(), out);
		buildText(static_cast<ConcatText*>(o)->right.get(), out);
		break;
		//TODO substr
	}
}

struct MemcpyBuilder {
	void operator()(char * part, size_t length) {
		memcpy(data, part, length);
		data += length;
	}
	MemcpyBuilder(char * data): data(data) {}
	char * data;
};

struct OStreamBuilder {
	OStreamBuilder(std::ostream & o): o(o) {}
	void operator()(char * part, size_t length) const {o.write(part, length);}
	std::ostream & o;
};

/**
 * Turn a text object into a canonical text
 */
const char * canonizeText(rm_object * o) {
	switch (o->type) {
	case Type::canonicalText:
		return static_cast<CanonicalText*>(o)->data;
	case Type::smallText:
		return static_cast<SmallText*>(o)->data;
	case Type::concatText:
	{
		size_t length=static_cast<TextBase*>(o)->length;
		char * data=new char[length];
		MemcpyBuilder builder(data);
		buildText(o, builder);
		uint32_t rc=o->ref_cnt;
		static_cast<ConcatText*>(o)->~ConcatText();
		new(o) CanonicalText(length, data);
		o->ref_cnt=rc;
		return data;
	}
	case Type::substrText:
	{
		size_t length=static_cast<TextBase*>(o)->length;
		char * data=new char[length];
		MemcpyBuilder builder(data);
		buildText(o, builder);
		uint32_t rc=o->ref_cnt;
		static_cast<ConcatText*>(o)->~ConcatText();
		new(o) CanonicalText(length, data);
		o->ref_cnt=rc;
		return data;
	}
	}
}


template <typename T, typename ...TS>
T * makeText(TS &&... ts) {
	T * o=reinterpret_cast<T *>(operator new(std::max(sizeof(ConcatText), std::max(sizeof(SubstrText), sizeof(CanonicalText)))));
	new(o) T(std::forward<TS>(ts)...);
}

SmallText * makeSmallText(size_t len) {
	SmallText * o = reinterpret_cast<SmallText*>(operator new(sizeof(sizeof(SmallText)+ len)));
	new(o) SmallText(len);	
	return o;
}

size_t length(rm_object * o) {
	return static_cast<TextBase*>(o)->length;
}


} //unnamed namespace

extern "C" {

void rm_free(rm_object * o) {
	switch (o->type) {
	case Type::canonicalText:
		static_cast<CanonicalText*>(o)->~CanonicalText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case Type::concatText:
		static_cast<ConcatText*>(o)->~ConcatText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case Type::substrText:
		static_cast<SubstrText*>(o)->~SubstrText();
		operator delete(reinterpret_cast<void *>(o));
	case Type::smallText:
		static_cast<SmallText*>(o)->~SmallText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	}
}

rm_object * rm_getConstText(const char *cptr) {
	size_t len = strlen(cptr);
	SmallText * o = makeSmallText(len);
	memcpy(o->data, cptr, len);
	return o;
}

rm_object * rm_concatText(rm_object *lhs, rm_object *rhs) {
	size_t len=length(lhs) + length(rhs);
	if (len <= smallLength) {
		SmallText * o = makeSmallText(len);
		MemcpyBuilder builder(o->data);
		buildText(lhs, builder);
		buildText(rhs, builder);
		return o;
	}
	return makeText<ConcatText>(RefPtr(lhs), RefPtr(rhs));
}

int8_t rm_substringSearch(rm_object *lhs, rm_object *rhs) {
	// TODO This could be much faster
	const char * lhst = canonizeText(lhs);
	const char * rhst = canonizeText(rhs);
	std::string dummy(rhst, length(rhs));
	return dummy.find(lhst, length(lhs)) != std::string::npos;
}	

void rm_printText(rm_object * ptr) {
	OStreamBuilder b(std::cout);
	buildText(ptr, b);
	std::cout << std::endl;
}

}