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
#ifndef __ast_base_hh__
#define __ast_base_hh__

#include <frontend/lexer.hh>
#include <memory>
#include <vector>
#include <limits>
#include <ostream>
#include <frontend/nodetype.hh>
#include <shared/type.hh>

namespace llvm {
struct Value;
};

namespace rasmus {
namespace frontend {

typedef size_t GlobalId;
const GlobalId NOT_GLOBAL=std::numeric_limits<GlobalId>::max();

class ICEException: public std::runtime_error {
public:
	ICEException(const std::string & msg): std::runtime_error(msg) {}
};

class CharRange {
public:
	size_t lo, hi;
	CharRange(): lo(std::numeric_limits<size_t>::max()), hi(std::numeric_limits<size_t>::min()) {}
	CharRange(size_t lo, size_t hi): lo(lo), hi(hi) {}
};



struct LLVMVal {
	llvm::Value * value;
	llvm::Value * type; //Only valid if val is of any type
	bool owned;
	LLVMVal(): value(nullptr), type(nullptr), owned(false) {}
	LLVMVal(llvm::Value * value, bool owned): value(value), type(nullptr), owned(owned) {}
	LLVMVal(llvm::Value * value, llvm::Value * type, bool owned): value(value), type(type), owned(owned) {}
	LLVMVal(const LLVMVal & o): value(o.value), type(o.type), owned(o.owned) {
		if (owned) throw ICEException("Owned was copied");
	}

	LLVMVal & operator=(const LLVMVal & o) {
		value = o.value;
		type = o.type;
		owned = o.owned;
		if (owned) throw ICEException("Owned was copied");
		return *this;
	}

	LLVMVal(LLVMVal && o): value(o.value), type(o.type), owned(o.owned) {
		o.value = nullptr;
		o.type = nullptr;
		o.owned = false;
	}
	LLVMVal & operator=(LLVMVal && o) {
		if (owned) throw ICEException("Moved into owned");
		value = o.value; o.value=nullptr;
		type = o.type; o.type=nullptr;
		owned = o.owned; o.owned=false;
		return *this;
	}

	~LLVMVal() {
		if (owned) throw ICEException("Owned was freed");
	}
};

class Node {
public:
	const NodeType nodeType;
	bool tainted;
	CharRange charRange;
	Type type;
	LLVMVal llvmVal;
	Node(NodeType t): nodeType(t), tainted(false), type(TInvalid) {}
	virtual ~Node() {}
};
typedef std::shared_ptr<Node> NodePtr;

} //namespace frontend
} //namespace rasmus

#endif //__ast_base_hh__


