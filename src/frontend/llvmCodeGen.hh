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
#ifndef __llvmcodegen_hh__
#define __llvmcodegen_hh__

#include <frontend/lexer.hh>
#include <frontend/AST.hh>
#include <frontend/error.hh>
#include <frontend/code.hh>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <unordered_map>

namespace rasmus {
namespace frontend {

/**
 * \brief intefrace of class for emmiting code using llvm
 *
 * The AST is trasformed into llvm using the standard llvm builder,
 * since most object in rasmus special care is taken to ensure that we
 * try to keep the number of incres and decrefs low this is done using
 * the \ref anonymous_namespace{llvmCodeGen.cc}::BorrowedLLVMVal
 * "BorrowedLLVMVal", \ref
 * anonymous_namespace{llvmCodeGen.cc}::LLVMVal "LLVMVal" and \ref
 * OwnedLLVMVal classes.
 *
 * The given AST is transformed into a single llvm function
 * that when called will have the same semantics as the ast.
 */
class LLVMCodeGen {
public:
	virtual ~LLVMCodeGen() {};

	/**
	 * \brief Translate an ast tree into an llvm finction.
	 * \note This may only be called once on a given instance of an
	 * LLVMCodeGen
	 */
	virtual llvm::Function * translate(NodePtr node) = 0;
};

std::shared_ptr<LLVMCodeGen> makeLlvmCodeGen(
	std::shared_ptr<Error> error, 
	std::shared_ptr<Code> code, 
	llvm::Module * module,
	bool dumpRawFunctions,
	bool dumpOptFunctions); 


} //namespace frontend
} //namespace rasmus

#endif //__llvmcodegen_hh__
