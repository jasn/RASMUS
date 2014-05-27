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
void buildText(rm_object * o, T & out, size_t start, size_t end) {
	switch (o->type) {
	case LType::canonicalText:
		out(static_cast<CanonicalText*>(o)->data + start, end - start);
		break;
	case LType::smallText:
		out(static_cast<SmallText*>(o)->data + start, end-start);
		break;
	case LType::concatText:
	{
		rm_object * l = static_cast<ConcatText*>(o)->left.get();
		rm_object * r = static_cast<ConcatText*>(o)->right.get();
		size_t llen = static_cast<TextBase*>(l)->length;
		if (start >= llen) 
			buildText(r, out, start - llen, end-llen);
		else if (end <= llen) 
			buildText(l, out, start, end);
		else {
			buildText(l, out, start, llen);
			buildText(r, out, 0, end - llen);
		}
		break;
	}
	case LType::substrText: 
	{
		SubstrText * s = static_cast<SubstrText *>(o);
		buildText(s->content.get(), out, start + s->start, end + s->start);
		break;
	}
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
	case LType::canonicalText:
		return static_cast<CanonicalText*>(o)->data;
	case LType::smallText:
		return static_cast<SmallText*>(o)->data;
	case LType::concatText:
	{
		size_t length=static_cast<TextBase*>(o)->length;
		char * data=new char[length];
		MemcpyBuilder builder(data);
		buildText(o, builder, 0, length);
		uint32_t rc=o->ref_cnt;
		static_cast<ConcatText*>(o)->~ConcatText();
		new(o) CanonicalText(length, data);
		o->ref_cnt=rc;
		return data;
	}
	case LType::substrText:
	{
		size_t length=static_cast<TextBase*>(o)->length;
		char * data=new char[length];
		MemcpyBuilder builder(data);
		buildText(o, builder, 0, length);
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
	o->incref();
	return o;
}

SmallText * makeSmallText(size_t len) {
	void * m=operator new(sizeof(SmallText)+ len);
	SmallText * o = reinterpret_cast<SmallText*>(m);
	new(o) SmallText(len);
	o->incref();
	return o;
}

size_t length(rm_object * o) {
	return static_cast<TextBase*>(o)->length;
}


} //unnamed namespace

extern "C" {

void rm_free(rm_object * o) {
	switch (o->type) {
	case LType::canonicalText:
		std::cout << "free(canonicalText)" << std::endl;
		static_cast<CanonicalText*>(o)->~CanonicalText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::concatText:
		std::cout << "free(concatText)" << std::endl;
		static_cast<ConcatText*>(o)->~ConcatText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::substrText:
		std::cout << "free(substrText)" << std::endl;
		static_cast<SubstrText*>(o)->~SubstrText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::smallText:
		std::cout << "free(smallText)" << std::endl;
		static_cast<SmallText*>(o)->~SmallText();
		operator delete(reinterpret_cast<void *>(o));
		break;
	case LType::function: 
	{		
		std::cout << "free(function)" << std::endl;
		function_object * fo = static_cast<function_object*>(o);
		fo->dtor(fo);
		::free(fo); //Functions are allocated with malloc by the codegen
	}
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
		buildText(lhs, builder, 0, length(lhs));
		buildText(rhs, builder, 0, length(rhs));
		return o;
	}
	return makeText<ConcatText>(RefPtr(lhs), RefPtr(rhs));
}

rm_object * rm_substrText(rm_object * str, int64_t start, int64_t end) {
	size_t len=length(str);
	start = std::max<int64_t>(0, start);
	end = std::min<int64_t>(end, len);
	if (end < start) end=start;
	if (end - start <= smallLength) {
		SmallText * o = makeSmallText(end-start);
		MemcpyBuilder builder(o->data);
		buildText(str, builder, start, end);
		return o;
	}
	return makeText<SubstrText>(RefPtr(str), (size_t)start, (size_t)end);
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
	buildText(ptr, b, 0, length(ptr) );
	std::cout << std::endl;
}

void rm_printTextToStream(rm_object * ptr, std::ostream & stream) {
	OStreamBuilder b(stream);
	buildText(ptr, b, 0, length(ptr) );
}

} // extern "C"

std::string rm_textToString(rm_object * ptr) {
	std::string text(length(ptr), ' ');
	MemcpyBuilder builder(&text[0]);
	buildText(ptr, builder, 0, text.size());
	return text;
}
