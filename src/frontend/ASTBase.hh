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
#include <frontend/ice.hh>
#include <frontend/strongType.hh>
namespace llvm {
struct Value;
};

namespace rasmus {
namespace frontend {

enum class BuildIn {
	invalid, 
		acos,
		asin,
		atan,
		atan2,
		ceil,
		cos,
		floor,
		pow,
		round,
		sin,
		sqrt, 
		tan,
		substr
};


typedef size_t GlobalId;
const GlobalId NOT_GLOBAL=std::numeric_limits<GlobalId>::max();

struct OwnedLLVMVal {
public:
	llvm::Value * value;
	llvm::Value * type; //Only valid if val is of any type

	OwnedLLVMVal(): value(nullptr), type(nullptr) {};
	OwnedLLVMVal(llvm::Value * value): value(value), type(nullptr) {}
	OwnedLLVMVal(llvm::Value * value, llvm::Value * type): value(value), type(type) {}
	
	OwnedLLVMVal(const OwnedLLVMVal & o) = delete;
	OwnedLLVMVal & operator=(const OwnedLLVMVal & o) = delete;
	OwnedLLVMVal(OwnedLLVMVal && o): value(o.value), type(o.type) {
		o.value=nullptr;
		o.type=nullptr;
	}

	OwnedLLVMVal & operator=(OwnedLLVMVal && o) {
		if (value) ICE("Write to tainted OwnedLLVMVAL");
		value=o.value;
		type=o.type;
		o.value=nullptr;
		o.type=nullptr;
		return *this;
	}
	
	~OwnedLLVMVal() {
		if (!std::uncaught_exception() && (value || type) ) ICE("Free of owned LLVMValue");
	}
};

class Node {
public:
	const NodeType nodeType;
	bool tainted;
	CharRange charRange;
	Type type;
	OwnedLLVMVal llvmVal;
	BuildIn buildin;
	Node(NodeType t): nodeType(t), tainted(false), buildin(BuildIn::invalid) {}
	virtual ~Node() {}
};
typedef std::shared_ptr<Node> NodePtr;

template <typename ... TT>
inline void ice_append(std::ostream & o, ice_help & h, NodePtr n, const TT & ... tt) {
	h.ranges.push_back(n->charRange);
	ice_append(o, h, tt...);
}


} //namespace frontend
} //namespace rasmus

#endif //__ast_base_hh__


