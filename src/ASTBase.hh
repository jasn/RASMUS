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

#include "lexer.hh"
#include <memory>
#include <vector>
#include <limits>
#include <ostream>
#include "nodetype.hh"
#include "type.hh"

class CharRange {
public:
	size_t lo, hi;
	CharRange(): lo(std::numeric_limits<size_t>::max()), hi(std::numeric_limits<size_t>::min()) {}
	CharRange(size_t lo, size_t hi): lo(lo), hi(hi) {}
};


namespace llvm {
struct Value;
struct GlobalVariable;
};

struct LLVMVal {
	llvm::Value * value;
	llvm::Value * type; //Only valid if val is of any type
	LLVMVal(): value(nullptr), type(nullptr) {}
	LLVMVal(llvm::Value * value): value(value), type(nullptr) {}
	LLVMVal(llvm::Value * value, llvm::Value * type): value(value), type(type) {}
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

template <typename RT>
class Visitor {
public:
  virtual ~Visitor() {};
  virtual RT run(NodePtr node) = 0;
};

#endif //__ast_base_hh__


