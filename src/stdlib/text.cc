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

#include <cstring>
#include <iostream>
#include <stdlib/text.hh>
#include <stdlib/rm_object.hh>
#include <stdlib/ile.hh>
#include <limits>
const size_t smallLength = 20;

namespace {
using namespace rasmus::stdlib;

template <typename T>
void buildText(TextBase * o, T & out, size_t start, size_t end) {
	switch (o->type) {
	case LType::canonicalText:
		out(static_cast<CanonicalText*>(o)->data + start, end - start);
		break;
	case LType::smallText:
		out(static_cast<SmallText*>(o)->data + start, end-start);
		break;
	case LType::concatText:
	{
		TextBase * l = static_cast<ConcatText*>(o)->left.get();
		TextBase * r = static_cast<ConcatText*>(o)->right.get();
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
	default:
		ILE("Unhandled type", o->type);
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

struct OStreamQuoteBuilder {
	OStreamQuoteBuilder(std::ostream & o): o(o) {}
	void operator()(char * part, size_t length) const {
		for (size_t i=0; i < length; ++i) {
			switch(part[i]) {
			case '\\':
			case '"':
				o.put('\\');
				o.put(part[i]);
				break;
			case '\r':
				o.put('\\');
				o.put('r');
				break;
			case '\n':
				o.put('\\');
				o.put('n');
				break;
			case '\t':
				o.put('\\');
				o.put('t');
				break;
			default:
				o.put(part[i]);
			}
		}
	}
	std::ostream & o;
};

/**
 * Turn a text object into a canonical text
 */
const char * canonizeText(TextBase * o) {
	switch (o->type) {
	case LType::canonicalText:
		return static_cast<CanonicalText*>(o)->data;
	case LType::smallText:
		return static_cast<SmallText*>(o)->data;
	case LType::concatText:
	{
		registerDeallocation(o);
		size_t length=o->length;
		char * data=new char[length+1];
		MemcpyBuilder builder(data);
		buildText(o, builder, 0, length);
		data[length] = '\0';
		uint32_t rc=o->ref_cnt;
		static_cast<ConcatText*>(o)->~ConcatText();
		new(o) CanonicalText(length, data);
		o->ref_cnt=rc;
		registerAllocation(o);
		return data;
	}
	case LType::substrText:
	{
		registerDeallocation(o);
		size_t length=o->length;
		char * data=new char[length+1];
		MemcpyBuilder builder(data);
		buildText(o, builder, 0, length);
		data[length] = '\0';
		uint32_t rc=o->ref_cnt;
		static_cast<ConcatText*>(o)->~ConcatText();
		new(o) CanonicalText(length, data);
		o->ref_cnt=rc;
		registerAllocation(o);
		return data;
	}
	default:
		ILE("Unhandled type", o->type);
	}
	return nullptr;
}

template <typename T, typename ...TS>
T * makeText(TS &&... ts) {
	T * o=reinterpret_cast<T *>(operator new(std::max(sizeof(ConcatText), std::max(sizeof(SubstrText), sizeof(CanonicalText)))));
	new(o) T(std::forward<TS>(ts)...);
	registerAllocation(o);
	o->incref();
	return o;
}

SmallText * makeSmallText(size_t len) {
	void * m=operator new(sizeof(SmallText)+ len+1);
	SmallText * o = reinterpret_cast<SmallText*>(m);
	o->data[len] = '\0';
	new(o) SmallText(len);
	registerAllocation(o);
	o->incref();
	return o;
}

size_t length(rm_object * o) {
	return static_cast<TextBase*>(o)->length;
}

TextBase * toTextBase(rm_object * o) {
	// TODO check for valid type
	return static_cast<TextBase*>(o);
}


} //unnamed namespace

namespace rasmus {
namespace stdlib {

void printQuoteTextToStream(TextBase * ptr, std::ostream & stream) {
	if (ptr == &undef_text) {
		stream << "?-Text";
		return;
	}
	stream.put('"');
	OStreamQuoteBuilder b(stream);
	buildText(ptr, b, 0, length(ptr) );
	stream.put('"');
	
}

void printTextToStream(TextBase * ptr, std::ostream & stream) {
	if (ptr == &undef_text) {
		stream << "?-Text";
		return;
	}
	OStreamBuilder b(stream);
	buildText(ptr, b, 0, length(ptr) );
}

std::string textToString(TextBase * ptr) {
	if (ptr == &undef_text) return "?-Text";
	std::string text(length(ptr), ' ');
	MemcpyBuilder builder(&text[0]);
	buildText(ptr, builder, 0, text.size());
	return text;
}

template <typename C>
int8_t textCompImpl(const rm_object * lhs, const rm_object * rhs, C c) {
	if (lhs == &undef_text) 
		return (rhs == &undef_text)
			?(c(0, 0)?RM_TRUE:RM_FALSE)
			:(c(0, 1)?RM_TRUE:RM_FALSE);
	if (rhs == &undef_text) 
		return c(1, 0)?RM_TRUE:RM_FALSE;
	const char * lhst = canonizeText(toTextBase(const_cast<rm_object *>(lhs)));
	const char * rhst = canonizeText(toTextBase(const_cast<rm_object *>(rhs)));
	return c(strcmp(lhst, rhst), 0)?RM_TRUE:RM_FALSE;
}

} //stdlib
} //rasmus

extern "C" {

rm_object undef_text = rm_object(LType::undefText, 1);

rm_object * rm_getConstText(const char *cptr) {
	size_t len = strlen(cptr);
	SmallText * o = makeSmallText(len);
	memcpy(o->data, cptr, len);
	return o;
}

int8_t rm_equalText(const rm_object *lhs, const rm_object *rhs) {
	return textCompImpl(lhs, rhs, std::equal_to<int>());
}

rm_object * rm_concatText(rm_object *lhs_, rm_object *rhs_) {
	if (lhs_ == &undef_text || rhs_ == &undef_text) {
		undef_text.incref();
		return &undef_text;
	}
	TextBase * lhs = toTextBase(lhs_);
	TextBase * rhs = toTextBase(rhs_);
	size_t len=lhs->length + rhs->length;
	if (len <= smallLength) {
		SmallText * o = makeSmallText(len);
		MemcpyBuilder builder(o->data);
		buildText(lhs, builder, 0, lhs->length);
		buildText(rhs, builder, 0, rhs->length);
		return o;
	}
	return makeText<ConcatText>(RefPtr<TextBase>(lhs), RefPtr<TextBase>(rhs));
}

rm_object * rm_substrText(rm_object * str_, int64_t start, int64_t end) {
	if (str_ == &undef_text ||
		start == std::numeric_limits<int64_t>::min() ||
		end == std::numeric_limits<int64_t>::min()) {
		undef_text.incref();
		return &undef_text;
	}
	TextBase * str = toTextBase(str_);
	size_t len=str->length;;
	start = std::max<int64_t>(0, start);
	end = std::min<int64_t>(end, len);
	if (end < start) end=start;
	if (end - start <= (int64_t)smallLength) {
		SmallText * o = makeSmallText(end-start);
		MemcpyBuilder builder(o->data);
		buildText(str, builder, start, end);
		return o;
	}
	return makeText<SubstrText>(RefPtr<TextBase>(str), (size_t)start, (size_t)end);
}

int8_t rm_substringSearch(rm_object *lhs, rm_object *rhs) {
	const char * lhst = canonizeText(toTextBase(lhs));
	const char * rhst = canonizeText(toTextBase(rhs));
	std::string dummy(rhst, length(rhs));
	return dummy.find(lhst, length(lhs)) != std::string::npos;
}	

int8_t rm_textLe(const rm_object * lhs, const rm_object * rhs) {
	return textCompImpl(lhs, rhs, std::less_equal<int>());
}

int8_t rm_textLt(const rm_object * lhs, const rm_object * rhs) {
	return textCompImpl(lhs, rhs, std::less<int>());
}

} // extern "C"

