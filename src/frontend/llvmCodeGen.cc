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
#include "lexer.hh"
#include <frontend/AST.hh>
#include "code.hh"
#include "error.hh"
#include <frontend/visitor.hh>
#include "llvmCodeGen.hh"
#include <sstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <array>
#include <frontend/tokenizer.hh>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#if LLVM_VERSION_MAJOR > 3 || LLVM_VERSION_MINOR >= 5
#include <llvm/IR/Verifier.h>
#else
#include <llvm/Analysis/Verifier.h>
#include <llvm/Assembly/PrintModulePass.h>
#endif
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/PassManager.h>
#include <llvm/IR/Instructions.h>

#include <llvm/Transforms/Scalar.h>
#include <llvm/Analysis/Passes.h>

using lexer::TokenType;

/**
 * \file llwmCodeGen.cc
 * This file implements rasmus code generation using llvm
 */
namespace {
using namespace llvm;
using namespace rasmus::frontend;

/**
 * \brief template magic to construct an argument pack from an array
 */
template<int ...> struct seq {};
/**
 * \brief template magic to construct an argument pack from an array
 */
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
/**
 * \brief template magic to construct an argument pack from an array
 */
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

/**
 * \brief Class representing a borrow value.
 * When we dispose of this value we should not defer the values
 */
struct BorrowedLLVMVal {
public:
	/**
	 * \brief The actual value, its type may vary depending on the static type
	 */
	llvm::Value * value; 

	/**
	 * \brief If the static type of this value is TAny, this will contain the runtime type
	 */
	llvm::Value * type;
	
	BorrowedLLVMVal(): value(nullptr), type(nullptr) {};
	BorrowedLLVMVal(llvm::Value * value): value(value), type(nullptr) {}
	BorrowedLLVMVal(llvm::Value * value, llvm::Value * type): value(value), type(type) {}
};

/**
 * \Brief Class represetting an owned or borrowed llvm value
 *
 * If the value is destructed on an owned type an exception will be thrown
 */
struct LLVMVal {
public:
	/**
	 * \brief The actual value, its type may vary depending on the static type
	 */
	llvm::Value * value;

	/**
	 * \brief If the static type of this value is TAny, this will contain the runtime type
	 */
	llvm::Value * type;
	
	/**
	 * \brief Indicate if this value is owned (or borrowed
	 */
	bool owned;

	LLVMVal(): value(nullptr), type(nullptr), owned(false) {}
	LLVMVal(const BorrowedLLVMVal & v): value(v.value), type(v.type), owned(false) {} 
	
	LLVMVal(OwnedLLVMVal && v): value(v.value), type(v.type), owned(true) {
		v.value=nullptr; v.type=nullptr;
	}

	LLVMVal(LLVMVal && v): value(v.value), type(v.type), owned(v.owned) {
		v.value=nullptr;
		v.type=nullptr;
		v.owned=false;
	}
	
	LLVMVal & operator=(LLVMVal && v) {
		if (value || type || owned) ICE("Set of tainted LLVMVal");
		value=v.value;
		type=v.type;
		owned=v.owned;
		v.value=nullptr;
		v.type=nullptr;
		v.owned=false;
		return *this;
	}
	
	LLVMVal(const LLVMVal & o) = delete;
	LLVMVal & operator=(const LLVMVal & o) = delete;
	
	~LLVMVal() {
		if (!std::uncaught_exception() && (value || type || owned)) ICE("LLVMValue not propperly disposed of");
	}
};

/**
 * \brief Main llvm codegenerator class
 */
class CodeGen: public LLVMCodeGen, public VisitorCRTP<CodeGen, LLVMVal> {
public:
	/**
	 * \brief unique id used to generate various unique identifiers for the llvmir
	 */
	size_t uid=0;
	std::shared_ptr<Error> error;
	std::shared_ptr<Code> code;

	std::unordered_map<std::string, llvm::Function *> stdlib;

	Module * module;
	IRBuilder<> builder;
	FunctionPassManager fpm;
	bool dumpRawFunctions;
	bool dumpOptFunctions;

	llvm::Type * voidType = llvm::Type::getVoidTy(getGlobalContext());
	llvm::IntegerType * int8Type = llvm::Type::getInt8Ty(getGlobalContext());
	llvm::IntegerType * int16Type = llvm::Type::getInt16Ty(getGlobalContext());
	llvm::IntegerType * int32Type = llvm::Type::getInt32Ty(getGlobalContext());
	llvm::IntegerType * int64Type = llvm::Type::getInt64Ty(getGlobalContext());
	llvm::PointerType * pointerType(llvm::Type * t) {return PointerType::getUnqual(t);}
	llvm::PointerType * voidPtrType = pointerType(int8Type);

	/**
	 * \brief Simple wrapper to define an llvm struct type
	 */
	llvm::StructType * structType(std::string name, 
								  std::initializer_list<llvm::Type *> types) {
		return StructType::create(getGlobalContext(), ArrayRef<llvm::Type * >(types.begin(), types.end()), name);
	}
	
	/**
	 * \brief Simple wrapper to define a llvm function type
	 */
	llvm::FunctionType * functionType(llvm::Type * ret, std::initializer_list<llvm::Type *> args) {
		return FunctionType::get(ret, ArrayRef<llvm::Type * >(args.begin(), args.end()), false);
	}
	
	llvm::FunctionType * dtorType = functionType(voidType, {voidPtrType});;
	llvm::StructType * anyRetType = structType("AnyRet", {int64Type, int8Type});
	llvm::StructType * tupleEntryType = structType("TupleEntry", {pointerType(int8Type), int64Type, int8Type});

	// refcnt, type, argc, dtorptr, funcptr
	// arguments to the function given by funcptr are: FuncBase *, anyRetType *, (uint64_t value, uint8_t type) zero or more times
	llvm::StructType * funcBase = structType("FuncBase", {int32Type, int16Type, int16Type, voidPtrType, voidPtrType} );;
	llvm::StructType * objectBaseType = structType("ObjectBase", {int32Type, int16Type});

	
	CodeGen(std::shared_ptr<Error> error, std::shared_ptr<Code> code,
			llvm::Module * module, bool dumpRawFunctions,
			bool dumpOptFunctions): error(error), code(code), module(module),
									builder(getGlobalContext()), fpm(module), 
									dumpRawFunctions(dumpRawFunctions),
									dumpOptFunctions(dumpOptFunctions) {
//		fpm.add(new DataLayout(module));
//		fpm.add(createBasicAliasAnalysisPass());
//		fpm.add(createInstructionCombiningPass());
//		fpm.add(createReassociatePass());
		//fpm.add(createGVNPass());
//		fpm.add(createCFGSimplificationPass());
		fpm.doInitialization();

		//Define all functions in the standard library
		std::vector< std::pair<std::string, FunctionType *> > fs =
		{
			{"rm_print", functionType(voidType, {int8Type, int64Type})},
			{"rm_checkAbort", functionType(voidType, {})},
			{"rm_enterFunction", functionType(voidType, {})},
			{"rm_exitFunction", functionType(voidType, {})},
			{"rm_free", functionType(voidType, {voidPtrType})},
			{"rm_concatText", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_getConstText", functionType(voidPtrType, {pointerType(int8Type)})},
			{"rm_emitTypeError", functionType(voidType, {int32Type, int32Type, int8Type, int8Type})},
			{"rm_emitIfError", functionType(voidType, {int32Type, int32Type})},
			{"rm_emitIsTypeError", functionType(voidType, {int32Type, int32Type, int8Type})},
			{"rm_emitArgCntError", functionType(voidType, {int32Type, int32Type, int16Type, int16Type})},
			{"rm_unionRel", functionType(voidPtrType, {voidPtrType, voidPtrType, int64Type})},
			{"rm_joinRel", functionType(voidPtrType, {voidPtrType, voidPtrType, int64Type})},
			{"rm_diffRel", functionType(voidPtrType, {voidPtrType, voidPtrType, int64Type})},
			{"rm_loadRel", functionType(voidPtrType, {pointerType(int8Type)})},
			{"rm_saveRel", functionType(voidType, {voidPtrType, pointerType(int8Type)})},
			{"rm_maxRel", functionType(int64Type, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_minRel", functionType(int64Type, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_addRel", functionType(int64Type, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_multRel", functionType(int64Type, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_countRel", functionType(int64Type, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_selectRel", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_projectPlusRel", functionType(voidPtrType, {voidPtrType, int32Type, pointerType(pointerType(int8Type)), int64Type})},
			{"rm_renameRel", functionType(voidPtrType, {voidPtrType, int32Type, pointerType(pointerType(int8Type)), int64Type})},
			{"rm_projectMinusRel", functionType(voidPtrType, {voidPtrType, int32Type, pointerType(pointerType(int8Type)), int64Type})},
			{"rm_substrText", functionType(voidPtrType, {voidPtrType, int64Type, int64Type})},
			{"rm_createFunction", functionType(voidPtrType, {int32Type})},
			{"rm_loadGlobalAny", functionType(voidType, {pointerType(int8Type), pointerType(anyRetType)})},
			{"rm_saveGlobalAny", functionType(voidType, {pointerType(int8Type), int64Type, int8Type})},
			{"rm_length", functionType(int64Type, {voidPtrType})},
			{"rm_tupEntry", functionType(voidType, {voidPtrType, pointerType(int8Type), pointerType(anyRetType), int64Type})},
			{"rm_createTup", functionType(voidPtrType, {int32Type, pointerType(tupleEntryType), int64Type})},
			{"rm_extendTup", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_tupRemove", functionType(voidPtrType, {voidPtrType, pointerType(int8Type), int64Type})},
			{"rm_tupHasEntry", functionType(int8Type, {voidPtrType, pointerType(int8Type)})},
			{"rm_relHasEntry", functionType(int8Type, {voidPtrType, pointerType(int8Type)})},
			{"rm_tupEntryType", functionType(int8Type, {voidPtrType, pointerType(int8Type), int8Type, int64Type})},
			{"rm_relEntryType", functionType(int8Type, {voidPtrType, pointerType(int8Type), int8Type, int64Type})},
			{"rm_equalText", functionType(int8Type, {voidPtrType, voidPtrType})},
			{"rm_equalRel", functionType(int8Type, {voidPtrType, voidPtrType})},
			{"rm_equalTup", functionType(int8Type, {voidPtrType, voidPtrType})},
			{"rm_createRel", functionType(voidPtrType, {voidPtrType})},
			{"rm_factorRel", functionType(voidPtrType, {int32Type, pointerType(pointerType(int8Type)), int32Type, pointerType(voidPtrType), voidPtrType, int64Type})},
			{"rm_deleteGlobalAny", functionType(voidType, {pointerType(int8Type)})}
		};

		for(auto p: fs)
			stdlib[p.first] = Function::Create(p.second, Function::ExternalLinkage, p.first, module);
	}

	/**
	 * \brief Fetch an function from the standard library or throw an error if it does not exist
	 */
	Function * getStdlibFunc(std::string name) {
		auto x=stdlib.find(name);
		if (x == stdlib.end()) ICE("Unknow stdlib function", name);
		return x->second;
	}

	/**
	 * \brief Get the llvm type of a rasmus type
	 */
	llvm::Type * llvmType(::Type t) {
		switch (t) {
		case TBool: return int8Type;
		case TInt: return int64Type;
		case TFunc: 
		case TText: 
		case TRel: 
			return voidPtrType;
		case TAny: return anyRetType;
		default:
			ICE("llvmType - Unhandled type ", t);
		}
	}
	
	llvm::ConstantInt * int8(uint8_t value) {return llvm::ConstantInt::get(int8Type, value);}
	llvm::ConstantInt * int16(uint16_t value) {return llvm::ConstantInt::get(int16Type, value);}
	llvm::ConstantInt * int32(uint32_t value) {return llvm::ConstantInt::get(int32Type, value);}
	llvm::ConstantInt * int64(int64_t value) {return llvm::ConstantInt::get(int64Type, value);}

	llvm::ConstantInt * undefInt = int64(std::numeric_limits<int64_t>::min());
	llvm::ConstantInt * trueBool = int8(3);
	llvm::ConstantInt * undefBool = int8(2);
	llvm::ConstantInt * falseBool = int8(0);


	llvm::ConstantInt * packCharRange(std::shared_ptr<Node> node) {
		return int64(node->charRange.lo | (uint64_t(node->charRange.hi) << 32));
	}

	
	/**
	 * \Brief get the runtime type represetnation for a static type
	 */
	llvm::Value * typeRepr(::Type t) {
		return int8(t);
	}
	
	/**
	 * \Brief get the function type of a rasmus function with argc arguments
	 */
	llvm::FunctionType * funcType(uint16_t argc) {
		std::vector<llvm::Type *> t;
		t.push_back(pointerType(funcBase));
		t.push_back(pointerType(anyRetType));
		for (size_t i=0; i < argc; ++i) {
			t.push_back(int64Type);
			t.push_back(int8Type);
		}
		return llvm::FunctionType::get(voidType, t, false);
	}

	/**
	 * \Brief get the current llvm function we are creating
	 */
	Function * getFunction() {
		return builder.GetInsertBlock()->getParent();
	}

	/**
	 * \Brief create a new block with a unique name
	 */
	BasicBlock * newBlock(std::string name = "b") {
		std::stringstream ss;
		ss << name << uid++;
		return BasicBlock::Create(getGlobalContext(), ss.str(), getFunction());
	}

	/**
	 * \Brief forget the ownership of a value, assuming it has been passed on to some external entity
	 */
	void forgetOwnership(LLVMVal & val) {
		val.value=nullptr;
		val.type=nullptr;
		val.owned=false;
	}

	/**
	 * \Brief forget the ownership of a value, assuming it has been passed on to some external entity
	 */
	void forgetOwnership(OwnedLLVMVal & val) {
		val.value=nullptr;
		val.type=nullptr;
	}

	/**
	 * \Brief turn a borrowed value into an owned value without increffing it,
	 * This assumes that the ownership was forgot earlier
	 */
	OwnedLLVMVal stealOwnership(BorrowedLLVMVal val) {
  		return OwnedLLVMVal(val.value, val.type);
	}

	/**
	 * \Brief borrow a reference to a llvm value, the borrowee must be alive while the borrowed value lives
	 */
	BorrowedLLVMVal borrow(const LLVMVal & v) {
		return BorrowedLLVMVal(v.value, v.type);
	}

	/**
	 * \Brief borrow a reference to a llvm value, the borrowee must be alive while the borrowed value lives
	 */
	BorrowedLLVMVal borrow(const OwnedLLVMVal & v) {
		return BorrowedLLVMVal(v.value, v.type);
	}

	/**
	 * \Brief take ownership of a value of a given static type, increffing when needed
	 */
	OwnedLLVMVal takeOwnership(LLVMVal val, ::Type type) {
 		OwnedLLVMVal value(val.value, val.type);
		bool owned=val.owned;
		forgetOwnership(val);
		if (owned) return value;
		switch (type) {
		case TInt:
		case TBool:
			break;
		case TFunc:
		case TRel:
		case TText:
		case TTup:
		{
			Value * v = builder.CreatePointerCast(value.value, pointerType(objectBaseType));
			Value * rc = builder.CreateAdd(builder.CreateLoad(builder.CreateConstGEP2_32(v, 0, 0)), int32(1));
			builder.CreateStore(rc, builder.CreateConstGEP2_32(v, 0, 0));
			break;
		}
		case TAny:
		{
			BasicBlock * b1 = newBlock();
			BasicBlock * b2 = newBlock();
			builder.CreateCondBr(builder.CreateICmpUGE(value.type, int8((int)TText)), b1, b2);
			builder.SetInsertPoint(b1);
			Value * p = builder.CreateIntToPtr(value.value, voidPtrType);
			Value * v = builder.CreatePointerCast(p, pointerType(objectBaseType));
			Value * rc = builder.CreateAdd(builder.CreateLoad(builder.CreateConstGEP2_32(v, 0, 0)), int32(1));
			builder.CreateStore(rc, builder.CreateConstGEP2_32(v, 0, 0));
			builder.CreateBr(b2);
			builder.SetInsertPoint(b2);
			break;
		}
		default:
			ICE("Unhandled type in takeOwnership", type);
		}
		return value;
	}

	/**
	 * \Brief take ownership of a value of a given static type, increffing when needed
	 */
	OwnedLLVMVal takeOwnership(BorrowedLLVMVal value, ::Type type) {
		return takeOwnership(LLVMVal(value), type);
	}

	/**
	 * \Brief drop ownership of a value of a given static type, decreffing when needed
	 */
	void disown(OwnedLLVMVal & value, ::Type type) {
		switch (type) {
		case TInt:
		case TBool:
			break;
		case TFunc:
		case TRel:
		case TText:
		case TTup:
		{
			BasicBlock * b1 = newBlock();
			BasicBlock * b2 = newBlock();
			Value * v = builder.CreatePointerCast(value.value, pointerType(objectBaseType));
			Value * rc = builder.CreateSub(builder.CreateLoad(builder.CreateConstGEP2_32(v, 0, 0)), int32(1));
			builder.CreateStore(rc, builder.CreateConstGEP2_32(v, 0, 0));
			builder.CreateCondBr(builder.CreateICmpEQ(rc, int32(0)), b1, b2);
			builder.SetInsertPoint(b1);
			builder.CreateCall(getStdlibFunc("rm_free"), builder.CreatePointerCast(value.value, voidPtrType));
			builder.CreateBr(b2);
			builder.SetInsertPoint(b2);
		}
			break;
		case TAny:
		{
			BasicBlock * b1 = newBlock();
			BasicBlock * b2 = newBlock();
			BasicBlock * b3 = newBlock();
			builder.CreateCondBr(builder.CreateICmpUGE(value.type, int8((int)TText)), b1, b3);
			builder.SetInsertPoint(b1);
			Value * p = builder.CreateIntToPtr(value.value, voidPtrType);
			Value * v = builder.CreatePointerCast(p, pointerType(objectBaseType));
			Value * rc = builder.CreateSub(builder.CreateLoad(builder.CreateConstGEP2_32(v, 0, 0)), int32(1));
			builder.CreateStore(rc, builder.CreateConstGEP2_32(v, 0, 0));
			builder.CreateCondBr(builder.CreateICmpEQ(rc, int32(0)), b2, b3);
			builder.SetInsertPoint(b2);
			builder.CreateCall(getStdlibFunc("rm_free"), p);
			builder.CreateBr(b3);
			builder.SetInsertPoint(b3);
			break;
		}
		default:
			ICE("Unhandled type in abandon", type);
		}
		forgetOwnership(value);
	}

	/**
	 * \Brief drop ownership of a value of a given static type, decreffing when needed
	 */
	void disown(LLVMVal & value, ::Type type) {
		if (value.owned) {
			OwnedLLVMVal v(value.value, value.type);
			disown(v, type);
		}
		forgetOwnership(value);
	}

	/**
	 * \Brief return the undefined value of a given static type
	 */
	BorrowedLLVMVal getUndef(::Type t) {
		switch(t) {
		case TInt:
			return BorrowedLLVMVal(undefInt);
		case TBool:
			return BorrowedLLVMVal(undefBool);
		case TAny:
			return BorrowedLLVMVal(undefInt, typeRepr(TInt));
		case TFunc:
			return BorrowedLLVMVal(llvm::Constant::getNullValue(pointerType(funcBase)));
		case TText:
			return extGlobalObject("undef_text");
		case TRel:
		case TTup:
		default:
			ICE("Unhandled undef", t);
		}
	}

	bool hasUndef(::Type t) {
		switch(t) {
		case TInt:
		case TBool:
		case TText:
			return true;
		case TFunc:
		case TAny:
		case TRel:
		case TTup:
		default:
			return false;
		}
	}

	/**
	 * \Brief cast a value a given static type to another.
	 * In case of errors error messages report the error in the range of node
	 * 
	 * Currently we can only cast form a type to it self
	 * from a TAny to anything else
	 * and from anything to a TAny
	 */
	BorrowedLLVMVal cast(BorrowedLLVMVal value, ::Type tfrom, ::Type tto, NodePtr node) {
		if (tfrom == tto) return BorrowedLLVMVal(value.value, value.type);
		if (tto == TAny) {
			switch(tfrom) {
			case TInt:
				return BorrowedLLVMVal(value.value, typeRepr(tfrom));
			case TBool:
				return BorrowedLLVMVal(builder.CreateZExt(value.value, int64Type), typeRepr(tfrom));
			case TText:
			case TFunc:
			case TRel:
			case TTup:
				return BorrowedLLVMVal(builder.CreatePtrToInt(value.value, int64Type), typeRepr(tfrom));
            default:
                ICE("Unhandled type", tfrom, tto, node);
			};
		}

		if (tfrom == TAny) {
			BasicBlock * fblock = newBlock("badtype");
			BasicBlock * nblock = newBlock("goodtype");
			std::stringstream ss;
			ss << "is" << tto;
			builder.CreateCondBr(builder.CreateICmpEQ(value.type, typeRepr(tto), ss.str()), nblock, fblock);
			builder.SetInsertPoint(fblock);
			builder.CreateCall4(getStdlibFunc("rm_emitTypeError"),
								int32(node->charRange.lo), int32(node->charRange.hi),
								value.type,
								typeRepr(tto));
			builder.CreateUnreachable();
			builder.SetInsertPoint(nblock);
			switch (tto) {
			case TInt:
				return BorrowedLLVMVal(value.value);
			case TBool:
				return BorrowedLLVMVal(builder.CreateTruncOrBitCast(value.value, llvmType(tto)));
			case TText:
				return BorrowedLLVMVal(builder.CreateIntToPtr(value.value, voidPtrType, "text"));
			case TRel:
				return BorrowedLLVMVal(builder.CreateIntToPtr(value.value, voidPtrType, "rel"));
			case TTup:
				return BorrowedLLVMVal(builder.CreateIntToPtr(value.value, voidPtrType, "tup"));
			case TFunc:
				return BorrowedLLVMVal(builder.CreateIntToPtr(value.value, voidPtrType, "func"));
			default:
				ICE("Unhandled type", tfrom, tto, node);
			}
		}
		ICE("Unhandled type", tfrom, tto, node);
	}

	/**
	 * \brief Cast a llvmval
	 */
	LLVMVal cast(LLVMVal value, ::Type tfrom, ::Type tto, NodePtr node) {
		BorrowedLLVMVal s=cast(borrow(value), tfrom, tto, node);
		value.value = s.value;
		value.type = s.type;
		return value;
	}

	/**
	 * \brief Cast an owned llvmcal
	 */
	OwnedLLVMVal cast(OwnedLLVMVal value, ::Type tfrom, ::Type tto, NodePtr node) {
		BorrowedLLVMVal s=cast(borrow(value), tfrom, tto, node);
		value.value = s.value;
		value.type = s.type;
		return value;
	}

	/**
	 * \brief Visit a child node, and cast the result to a given type
	 */
	LLVMVal castVisit(NodePtr node, ::Type tto) {
		return cast(visitNode(node), node->type, tto, node);
	}

	/**
	 * \brief When we are done generating a function call the method to optimize and output debug info
	 */
	void finishFunction(Function * func) {
		if (dumpRawFunctions) func->dump();
		llvm::verifyFunction(*func);
		fpm.run(*func);
		if (dumpOptFunctions) func->dump();
	}

	
	BorrowedLLVMVal loadValue(Value * v, std::initializer_list<int> gep, ::Type type) {
		std::vector<Value *> GEP;
		for (int x: gep) GEP.push_back(int32(x));
		switch (type) {
		case TInt:
			return BorrowedLLVMVal(builder.CreateLoad(builder.CreateGEP(v, GEP, "int_addr"), "int"));
		case TBool:
			return BorrowedLLVMVal(builder.CreateLoad(builder.CreateGEP(v, GEP, "bool_addr"), "bool"));
		case TText:
			return BorrowedLLVMVal(builder.CreateLoad(builder.CreateGEP(v, GEP, "text_addr"), "text"));
		case TFunc:
			return BorrowedLLVMVal(builder.CreateLoad(builder.CreateGEP(v, GEP, "func_addr"), "func"));
		case TRel:
			return BorrowedLLVMVal(builder.CreateLoad(builder.CreateGEP(v, GEP, "rel_addr"), "rel"));
		case TAny:
		{
			GEP.push_back(int32(0));
			Value * value=builder.CreateGEP(v, GEP, "value_addr");
			GEP.pop_back();
			GEP.push_back(int32(1));
			Value * type=builder.CreateGEP(v, GEP, "type_addr");
			return BorrowedLLVMVal(builder.CreateLoad(value, "value"), builder.CreateLoad(type,"type"));
		}
		default:
			ICE("Unhandled type", type);
		}
	}

	void saveValue(Value * dst, std::initializer_list<int> gep, LLVMVal _v, ::Type type) {
		OwnedLLVMVal v=takeOwnership(std::move(_v), type);
		std::vector<Value *> GEP;
		for (int x: gep) GEP.push_back(int32(x));
		switch (type) {
		case TInt:
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "int_addr"));
			break;
		case TBool:
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "bool_addr"));
			break;
		case TText:
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "text_addr"));
			break;
		case TFunc:
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "func_addr"));
			break;
		case TRel:
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "rel_addr"));
			break;
		case TAny:
		{
			GEP.push_back(int32(0));
			builder.CreateStore(v.value, builder.CreateGEP(dst, GEP, "value_addr"));
			GEP.pop_back();
			GEP.push_back(int32(1));
			builder.CreateStore(v.type, builder.CreateGEP(dst, GEP, "type_addr"));
			break;
		}
		default:
			ICE("Unhandled type", type);
		}
		forgetOwnership(v);
	}

	/**
	 * \Create a global string object
	 */
	Value * globalString(std::string s) {
		Constant * c = ConstantDataArray::getString(getGlobalContext(), s);
		GlobalVariable * gv = new GlobalVariable(*module,
												 c->getType(),
												 true,
												 llvm::GlobalValue::PrivateLinkage,
												 c);
		return builder.CreateConstGEP2_32(gv, 0, 0);
	}

	/**
	 * \Create a global string object
	 */
	Value * globalString(Token t) {
		return globalString(t.getText(code));
	}

	/**
	 * \Create return a reference to a global external symbol
	 */
	BorrowedLLVMVal extGlobalObject(const char * name) {
		if (module->getGlobalVariable(name)) return BorrowedLLVMVal(module->getGlobalVariable(name));
		return BorrowedLLVMVal(new GlobalVariable(*module,
												  int8Type,
												  true,
												  llvm::GlobalValue::ExternalLinkage,
												  nullptr,
												  name));
	}


	/**
	 * \brief template magic used by dOp and friends
	 * \internal
	 */
	template <typename T>
	struct bwh {
		typedef BorrowedLLVMVal t;
	};

	/**
	 * \brief template magic used by dOp and friends
	 * \internal
	 */
	template <typename T>
	struct th {
		typedef ::Type t;
	};


	/**
	 * \brief Defines a typed operation
	 */
	template <typename ...T>
	struct OpImpl {
		/** \brief The return type */
		::Type ret;
		/** \brief The argument types */
		std::array<::Type, sizeof...(T)> types;
		/** \brief The function to call to emit code for the operation */
		std::function<LLVMVal(typename bwh<T>::t ...)> func;
	};

	/**
	 * \brief Declare a typed operation
	 * \param func The function that exmits code for the operation
	 * \param rtype The return type of the operation
	 * \param types The argument types of the operation
	 */
	template <typename F, typename ...T>
	OpImpl<typename th<T>::t...> dOp(
		F func, 
		::Type rtype, 
		T... types) {
		typedef OpImpl<typename th<T>::t...> rt; 
		return rt{rtype, {types...}, func};
	}

	/**
	 * \brief Declare a typed operation which is a simple comparator
	 * \parm in The type the comparator operates on
	 * \param pred The predicate to use for comparison
	 */
	OpImpl<::Type, ::Type> dComp(::Type in, llvm::CmpInst::Predicate pred) {
		return OpImpl<::Type, ::Type>{TBool, {in, in}, 
				[this, in, pred](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs) -> OwnedLLVMVal {
					return builder.CreateSelect(
						builder.CreateICmpEQ(lhs.value, getUndef(in).value),
						undefBool,
						builder.CreateSelect(
							builder.CreateICmpEQ(rhs.value, getUndef(in).value),
							undefBool,
							builder.CreateSelect(
								builder.CreateICmp(pred, lhs.value, rhs.value),
								trueBool,
								falseBool)));
				}};
	}

	/**
	 * \brief Declare a typed operation which returns undef in any of the arguments are undef
	 * See \ref{dOp}
	 */
	template <typename Func>
	OpImpl<::Type, ::Type> dOpU(Func func, ::Type ret, ::Type tlhs, ::Type trhs) {
		return OpImpl<::Type, ::Type>{ret, {tlhs, trhs}, 
				[this, func, tlhs, trhs, ret](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs) -> OwnedLLVMVal {
					LLVMVal v=func(lhs, rhs);
					OwnedLLVMVal r=builder.CreateSelect(
						builder.CreateICmpEQ(lhs.value, getUndef(tlhs).value),
						getUndef(ret).value,
						builder.CreateSelect(
							builder.CreateICmpEQ(rhs.value, getUndef(trhs).value),
							getUndef(ret).value,
							v.value));
					disown(v, ret);
					return r;
				}};
	}

	/**
	 * \brief Declare a typed operation which is a standard library call
	 * See \ref{dOp}
	 */
	template <typename ...T>
	OpImpl<typename th<T>::t...> dCall(
		const char * name, 
		::Type rtype, 
		T ... types) {
		typedef OpImpl<typename th<T>::t...> rt;
		return rt{rtype, {types...},
				[name, this](typename bwh<T>::t... vals) -> OwnedLLVMVal {
					return builder.CreateCall(
						getStdlibFunc(name), 
						std::vector<Value*>{vals.value...});
				}};
	}

	/**
	 * \brief Declare a typed operation which is a standard library call
	 * See \ref{dOp}
	 */
	template <typename ...T>
	OpImpl<typename th<T>::t...> dCallR(
		const char * name, 
		std::shared_ptr<Node> node, 
		::Type rtype, 
		T ... types) {
		typedef OpImpl<typename th<T>::t...> rt;
		return rt{rtype, {types...},
				[name, this, node](typename bwh<T>::t... vals) -> OwnedLLVMVal {
					return builder.CreateCall(
						getStdlibFunc(name), 
						std::vector<Value*>{vals.value..., packCharRange(node)});
				}};
	}

	/**
	 * \brief Declare a typed operation
	 * \param func The function that exmits code for the operation
	 * \param rtype The return type of the operation
	 * \param types The argument types of the operation
	 */
	template <typename ...T>
	OpImpl<T...> dBInv(OpImpl<T...> op) {
		typedef OpImpl<T...> rt;
		return rt{op.ret, op.types, [op,this](typename bwh<T>::t... vals) -> LLVMVal {
				LLVMVal v=op.func(vals...);
				LLVMVal r=OwnedLLVMVal(builder.CreateSub(trueBool, v.value));
				disown(v, op.ret);
				return r;
			}};
	}


	/**
	 * \brief template magic used by dOpImp
	 * \internal
	 */
	template <int i>
	struct bwi {
		typedef BorrowedLLVMVal t;
	};

	/**
	 * \brief template magic used by dOpImp
	 * \internal
	 */
	template <int ...S>
	LLVMVal doCall(seq<S...>, 
				   std::function<LLVMVal(typename bwi<S>::t ...)> func,
				   std::array<BorrowedLLVMVal, sizeof...(S)> & v) {
		return func(v[S]...);
	}

	/**
	 * \brief template magic used by dOpImp
	 * \internal
	 */
	template <typename oi_t,
			  int N> 
	LLVMVal opImp_(int i,
				   std::vector<oi_t> ops,
				   NodePtr node,
				   const std::array<LLVMVal, N> & vals,
				   std::array<NodePtr, N> childs,
				   std::array<BorrowedLLVMVal, N> args) {
		if (i == N) {
			if (ops.size() > 1) ICE("Multiple identical binops", node);
			LLVMVal v=doCall(typename gens<N>::type(), ops[0].func, args);
			LLVMVal r(cast(std::move(v), ops[0].ret, node->type, node));
			return r;
		}
		
		::Type type=childs[i]->type;
		
		if (type != TAny) {
			std::vector<oi_t> matches;
			for (auto op: ops) {
				if (type != op.types[i]) continue;
				matches.push_back(op);
			}
			if (matches.size() == 0)
				ICE("Infeasible types in binopImpl, there is a bug in the typechecker!!", node);
			args[i] = borrow(vals[i]);
			LLVMVal r=opImp_<oi_t, N>(i+1, matches, node, vals, childs, args);
			return r;
		} else {
			BasicBlock * errTarget = newBlock();
			BasicBlock * end = newBlock();
			std::sort(ops.begin(), ops.end(), [i](const oi_t & a, const oi_t & b) {
					return a.types[i] < b.types[i];
				});
			
			Value * value=nullptr;
			Value * type=nullptr;
			switch (node->type) {
			case TText:
			case TRel:
			case TTup:
				value = builder.CreateAlloca(voidPtrType);
				break;
			case TBool:
				value = builder.CreateAlloca(int8Type);
				break;
			case TInt:
				value = builder.CreateAlloca(int64Type);
				break;
			case TAny:
				value = builder.CreateAlloca(int64Type);
				type = builder.CreateAlloca(int8Type);
				break;
			default:
				ICE("Unhandled", node->type, node);
			}


			SwitchInst * sw=builder.CreateSwitch(vals[i].type, errTarget);

			auto a=ops.begin();
			while (a != ops.end()) {
				auto b=a;
				do {
					++b;
				} while(b != ops.end() && b->types[i] == a->types[i]);

				BasicBlock * block = newBlock();
				sw->addCase(int8(a->types[i]), block);
				builder.SetInsertPoint(block);
				
				switch (a->types[i]) {
				case TInt:
					args[i] = BorrowedLLVMVal(vals[i].value);
					break;
				case TBool:
					args[i] = BorrowedLLVMVal(builder.CreateTruncOrBitCast(vals[i].value, int8Type));
					break;
				case TText:
				case TFunc:
				case TRel:
				case TTup:
					args[i] = BorrowedLLVMVal(builder.CreateIntToPtr(vals[i].value, voidPtrType));
					break;
				default:
					ICE("Unhandled", node->type, node);
			
				};
				
				std::vector<oi_t> matches(a, b);
				LLVMVal r=opImp_<oi_t, N>(i+1, matches, node, vals, childs, args);
				builder.CreateStore(r.value, value);
				if (type) builder.CreateStore(r.type, type);
				builder.CreateBr(end);
				
				forgetOwnership(r);	
				a=b;
			}

			
			builder.SetInsertPoint(errTarget);
			
			builder.CreateCall4(getStdlibFunc("rm_emitTypeError"),
								int32(node->charRange.lo), int32(node->charRange.hi),
								vals[i].type,
								typeRepr(ops.begin()->types[i])); // TODO FIXME this only states one even though there might be more allowed
			builder.CreateUnreachable();
			builder.SetInsertPoint(end);
			LLVMVal r=OwnedLLVMVal(
				builder.CreateLoad(value),
				type?builder.CreateLoad(type):nullptr);
			return r;
		}
	}

	/**
	 * \brief Create codegen for a set of typed operations on the given nodes
	 *
	 * A number of operations will be given in the ops list, each of the child nodes
	 * c of node, is codegened, and the appropriate operation is codegened depending on
	 * the types, in case this cannot be figured out on compile time (too mary TAnys), 
	 * the right operation is selected on runtime
	 *
	 * This is usefull for implementing type overloaded unary and binary operators, 
	 * as well as type overloaded buildin functions
	 */
	template <typename ...T>
	LLVMVal opImp(std::initializer_list<OpImpl<typename th<T>::t...> > ops,
				  std::shared_ptr<Node> node,
				  T ... c
		) {
		const int N=sizeof...(T);
		typedef OpImpl<typename th<T>::t...> oi_t;

		std::array<std::shared_ptr<Node>, N> childs={std::static_pointer_cast<Node>(c)...};
		
		std::vector<oi_t> matches;
		for (auto op: ops) {
			bool ok=true;
			for (size_t i=0; i < N; ++i)
				ok = ok && (childs[i]->type == op.types[i] || childs[i]->type == TAny);
			if (!ok) continue;
			matches.push_back(op);
		}
		std::array<BorrowedLLVMVal, N> args;
		std::array<LLVMVal, N> vals;
		for (size_t i=0; i < N; ++i) 
			vals[i]=visitNode(childs[i]);
		LLVMVal r=opImp_<oi_t, N>(0, matches, node, vals, childs, args);
		for (size_t i=0; i < N; ++i)
			disown(vals[i], childs[i]->type);
		return  r;
	}

	LLVMVal readNamedVariable(Token nameToken, std::weak_ptr<Node> storePtr,
							  const NodePtr & node) {
		if (NodePtr store = storePtr.lock()) {
			if (store->nodeType != NodeType::AssignmentExp) return borrow(store->llvmVal);
			std::shared_ptr<AssignmentExp> st=std::static_pointer_cast<AssignmentExp>(store);
			
			if (st->globalId == NOT_GLOBAL)
				return borrow(store->llvmVal);
		}
		Value * rv = builder.CreateAlloca(anyRetType, nullptr, "globalVar");
			
		builder.CreateCall2(getStdlibFunc("rm_loadGlobalAny"),
							globalString(nameToken),
							rv);
			
		return takeOwnership(cast(BorrowedLLVMVal(
								 builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 0, "value_addr"), "value"), 
								 builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 1, "type_addr"), "type")),
								  TAny, node->type, node), node->type);
			
	}
	
	/** \brief Codegen for a valiable */
	LLVMVal visit(std::shared_ptr<VariableExp> node) {
		return readNamedVariable(node->nameToken, node->store, node);
	} 
	
	/** \brief Codegen for an assignment */
	LLVMVal visit(std::shared_ptr<AssignmentExp> node) {
		LLVMVal val = castVisit(node->valueExp, node->type);
		if (node->globalId == NOT_GLOBAL) {
			node->llvmVal=takeOwnership(borrow(val), node->type);
			return val;
		}

		{
			BorrowedLLVMVal v2 = cast(borrow(val), node->type, TAny, node);
			builder.CreateCall3(getStdlibFunc("rm_saveGlobalAny"), 
								globalString(node->nameToken),
								v2.value,
								v2.type);
		}

		return val;
	}

	/** \brief Codegen for if */
	LLVMVal visit(std::shared_ptr<IfExp> node) {
		Value * value=nullptr;
		Value * type=nullptr;
		switch (node->type) {
		case TText:
		case TRel:
		case TTup:
		case TFunc:
			value = builder.CreateAlloca(voidPtrType);
			break;
		case TBool:
			value = builder.CreateAlloca(int8Type);
			break;
		case TInt:
			value = builder.CreateAlloca(int64Type);
			break;
		case TAny:
			value = builder.CreateAlloca(int64Type);
			type = builder.CreateAlloca(int8Type);
			break;
		default:
			ICE("Unhandled", node->type, node);
		}


		BasicBlock * end = newBlock();
		bool done=false;

		for (auto choice: node->choices) {
			LLVMVal cond = castVisit(choice->condition, TBool);
		 	llvm::ConstantInt * ci = dyn_cast<llvm::ConstantInt>(cond.value);
		 	if (done || (ci && ci->isZeroValue())) {
		 		error->reportWarning("Branch never taken", choice->arrowToken);
				forgetOwnership(cond);
		 		continue;
		 	}

			if (ci && ci->equalsInt(3)) {
				LLVMVal v=takeOwnership(castVisit(choice->value, node->type), node->type);
				builder.CreateStore(v.value, value);
				if (type) builder.CreateStore(v.type, type);
				forgetOwnership(v);
				builder.CreateBr(end);
				done=true;
			} else {
				BasicBlock * b1 = newBlock();		
				BasicBlock * b2 = newBlock();
				builder.CreateCondBr(builder.CreateICmpEQ(cond.value, trueBool), b1, b2);
				
				builder.SetInsertPoint(b1);
				LLVMVal v=takeOwnership(castVisit(choice->value, node->type), node->type);
				builder.CreateStore(v.value, value);
				if (type) builder.CreateStore(v.type, type);
				forgetOwnership(v);
				builder.CreateBr(end);
				
				builder.SetInsertPoint(b2);
			}
			forgetOwnership(cond);
		}
		
		if (!done) {
			if (hasUndef(node->type)) {
				LLVMVal v=takeOwnership(getUndef(node->type), node->type);
				builder.CreateStore(v.value, value);
				if (type) builder.CreateStore(v.type, type);
				forgetOwnership(v);			
				builder.CreateBr(end);
				error->reportWarning("if might have none of the branches taken, and the return type has no default value.", node->ifToken, {node->charRange});
			} else {
				builder.CreateCall2(getStdlibFunc("rm_emitIfError"),
									int32(node->charRange.lo), int32(node->charRange.hi));
				builder.CreateUnreachable();
				
			}
		}
		builder.SetInsertPoint(end);
	
		return OwnedLLVMVal(builder.CreateLoad(value), type?builder.CreateLoad(type):nullptr);
	}

	/** \brief Codegen for factor */
	LLVMVal visit(std::shared_ptr<ForallExp> node) {
		Value * names = builder.CreateAlloca(voidPtrType, int32(node->names.size()) );
		for (size_t i=0; i < node->names.size(); ++i)
			builder.CreateStore(globalString(node->names[i]), builder.CreateConstGEP1_32(names, i));

		Value * relations = builder.CreateAlloca(voidPtrType, int32(node->listExps.size()) );
		std::vector<LLVMVal> possibly_owned_relations;
		for (size_t i=0; i < node->listExps.size(); ++i){
			possibly_owned_relations.push_back(castVisit(node->listExps[i], TRel));
			builder.CreateStore(possibly_owned_relations.back().value, builder.CreateConstGEP1_32(relations, i));
		}

		LLVMVal func = castVisit(node->exp, TFunc);
		std::vector<Value *> args = {int32(node->names.size()), names, int32(node->listExps.size()),
									 relations, builder.CreatePointerCast(func.value, voidPtrType),
									 packCharRange(node)};
			
		LLVMVal ret = OwnedLLVMVal(builder.CreateCall(getStdlibFunc("rm_factorRel"), args));

		disown(func, TFunc);
		for(auto & rel : possibly_owned_relations)
		    disown(rel, TRel); 

		return ret;
	}

	/** \brief Codegen for function declaration */
	LLVMVal visit(std::shared_ptr<FuncExp> node) {
		// Create function type
		FunctionType * ft = funcType(node->args.size());

		std::vector<llvm::Type *> captureContent = {
			int32Type, //Refcount
			int16Type, //Type
			int16Type, //Number Of arguments
			pointerType(dtorType), //Dtor
			pointerType(ft), //Function ptr
		};
		for (auto cap: node->captures)
			captureContent.push_back(llvmType(cap->type));
		
		StructType * captureType = StructType::create(getGlobalContext(), captureContent);
		assert(captureType->isSized());
		
		//Cache currentFunction
		auto old_ip = builder.saveIP();

		Function * dtor=nullptr;
		
		// Create dtor 
		{
			std::stringstream ss;
			ss << "f_dtor" << uid;
			dtor = Function::Create(dtorType, Function::InternalLinkage, ss.str(), module);
			BasicBlock * block = BasicBlock::Create(getGlobalContext(), "entry", dtor);
			builder.SetInsertPoint(block);
			auto & al = dtor->getArgumentList();
			auto self = al.begin();
			self->setName("self");
			Value * selfv = builder.CreatePointerCast(self, pointerType(captureType));
			

			//go through the capture list and abandon all
			for (size_t i=0; i < node->captures.size(); ++i) {
				OwnedLLVMVal val(stealOwnership(loadValue(selfv, {0, 5+(int)i}, node->captures[i]->type)));
				disown(val, node->captures[i]->type);
			}

			builder.CreateRetVoid();

			finishFunction(dtor);
		}
		
		Function * function=nullptr;
		
		// Create function inner
		{
			std::stringstream ss;
			ss << "f" << uid++;
			function = Function::Create(ft, Function::InternalLinkage, ss.str(), module);
			BasicBlock * block = BasicBlock::Create(getGlobalContext(), "entry", function);
			builder.SetInsertPoint(block);
			// Setup args		
			auto & al = function->getArgumentList();   
			auto arg = al.begin();
			auto self = &(*arg);
			++arg;
			auto ret = &(*arg);
			++arg;
			
			self->setName("self");
			ret->setName("ret");
			Value * selfv = builder.CreatePointerCast(self, pointerType(captureType));
			for (auto a: node->args) {
				Value * v = &(*arg);
				++arg;
				Value * t = &(*arg);
				++arg;
				a->llvmVal = stealOwnership(cast(BorrowedLLVMVal(v, t), TAny, a->type, a));
			}

			for (size_t i=0; i < node->captures.size(); ++i)
				node->captures[i]->llvmVal = stealOwnership(loadValue(selfv, {0, 5+(int)i}, node->captures[i]->type));
        
			// Build function code
			OwnedLLVMVal x = cast(
				takeOwnership(visitNode(node->body), node->body->type), node->body->type, TAny, node->body);
			
			for (auto a: node->args)
				forgetOwnership(a->llvmVal);
			
			for (auto c: node->captures)
				forgetOwnership(c->llvmVal);

			// HATKAT

			builder.CreateStore(x.value, builder.CreateConstGEP2_32(ret, 0, 0));
			builder.CreateStore(x.type, builder.CreateConstGEP2_32(ret, 0, 1));
			builder.CreateRetVoid();
			forgetOwnership(x);
			finishFunction(function);
		}

		// Revert state
		builder.restoreIP(old_ip);
		Constant* AllocSize = ConstantExpr::getSizeOf(captureType);
		AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, int32Type);
		auto pp = builder.CreateCall(getStdlibFunc("rm_createFunction"), AllocSize, "func");
		auto p = builder.CreatePointerCast(pp, pointerType(captureType), "func_c");
		builder.CreateStore(int32(1), builder.CreateConstGEP2_32(p, 0, 0, "refcnt")); //RefCount
		builder.CreateStore(int16(node->args.size()), builder.CreateConstGEP2_32(p, 0, 2, "argc")); //Argc
		builder.CreateStore(dtor, builder.CreateConstGEP2_32(p, 0, 3, "dtor")); //Dtor
		builder.CreateStore(function, builder.CreateConstGEP2_32(p, 0, 4, "fptr")); //Fptr
		
        // Store captures
		for (size_t i=0; i < node->captures.size(); ++i) {
			std::shared_ptr<FuncCaptureValue> fcv = node->captures[i];
			LLVMVal v=readNamedVariable(fcv->nameToken, fcv->store, fcv);
			saveValue(p, {0, 5+(int)i}, borrow(v), fcv->type);
			disown(v, fcv->type);
		}

		return OwnedLLVMVal(builder.CreatePointerCast(p, voidPtrType, "func"));
	}


	/** \brief Codegen for tuple creation */	
	LLVMVal visit(std::shared_ptr<TupExp> exp) {
		std::vector<LLVMVal> values;
		Value * entries = builder.CreateAlloca(tupleEntryType, int32(exp->items.size()) );
		for (size_t i=0; i < exp->items.size(); ++i) {
			auto item = exp->items[i];
			values.push_back( visitNode(item->exp) );
			BorrowedLLVMVal v = cast(borrow(values.back()), item->exp->type, TAny, item->exp);
			
			builder.CreateStore(globalString(item->nameToken), builder.CreateConstGEP2_32(entries, i, 0)); //name
			builder.CreateStore(v.value, builder.CreateConstGEP2_32(entries, i, 1)); //value
			builder.CreateStore(v.type, builder.CreateConstGEP2_32(entries, i, 2)); //type
		}

		OwnedLLVMVal r(builder.CreateCall3(getStdlibFunc("rm_createTup"), int32(exp->items.size()), 
										   entries, packCharRange(exp)));
		
		for (size_t i=0; i < exp->items.size(); ++i)
			disown(values[i], exp->items[i]->exp->type);
		return LLVMVal(std::move(r));
	}

	/** \brief Codegen for a block */
	LLVMVal visit(std::shared_ptr<BlockExp> node) {
		for (auto v: node->vals)
			v->exp->llvmVal = takeOwnership(visitNode(v->exp), v->exp->type);

		// This could be a reference to one of the vals, so we take ownership
		OwnedLLVMVal r = takeOwnership(visitNode(node->inExp), node->inExp->type); 
		
		for (auto v: node->vals) 
			disown(v->exp->llvmVal, v->exp->type);
		
		return LLVMVal(std::move(r));
	}

	LLVMVal genIsExpression(std::shared_ptr<BuiltInExp> & node, ::Type wantedType){

		// this is a special case which is only supported
		// for the atomic operators; we expect the first 
		// argument to be of type TRel or TTup, otherwise we 
		// emit an error. If it's a TAny, though, we need to
		// perform the check on runtime
		if(node->args.size() > 1){
			if(node->args[0]->type == TRel || node->args[0]->type == TTup){
				// in this case we just delegate to the standard library
				return opImp(
					{
						dOp([this, node, wantedType](BorrowedLLVMVal v) -> OwnedLLVMVal {
								return builder.CreateCall4(
									getStdlibFunc("rm_tupEntryType"), 
									v.value, 
									globalString(std::static_pointer_cast<VariableExp>(
													 node->args[1])->nameToken.getText(code)),
									typeRepr(wantedType),
									packCharRange(node));
							}, TBool, TTup),
							dOp([this, node, wantedType](BorrowedLLVMVal v) -> OwnedLLVMVal {
									return builder.CreateCall4(
										getStdlibFunc("rm_relEntryType"), 
										v.value, 
										globalString(std::static_pointer_cast<VariableExp>(
														 node->args[1])->nameToken.getText(code)),
										typeRepr(wantedType),
										packCharRange(node));
								}, TBool, TRel)
							}, node, node->args[0]);
					
			}
			else if(node->args[0]->type == TAny){
				// in this case we must check the type at runtime

				LLVMVal v = castVisit(node->args[0], TAny);
				BorrowedLLVMVal r(builder.CreateIntToPtr(v.value, voidPtrType, "rel"));
				BorrowedLLVMVal t(builder.CreateIntToPtr(v.value, voidPtrType, "tup"));

				BasicBlock * b1 = newBlock();		
				BasicBlock * b2 = newBlock();
				BasicBlock * b3 = newBlock();		
				BasicBlock * b4 = newBlock();
				BasicBlock * bend = newBlock();
				BasicBlock * c1 = newBlock();
				BasicBlock * c2 = newBlock();
				builder.CreateCondBr(builder.CreateICmpEQ(v.type, typeRepr(TRel)), b1, b2);
							
				builder.SetInsertPoint(b1);
				// we get here if the type is TRel
				Value * v1 = builder.CreateCall4(
					getStdlibFunc("rm_relEntryType"), 
					r.value,
					globalString(std::static_pointer_cast<VariableExp>(
									 node->args[1])->nameToken.getText(code)),
					typeRepr(wantedType),
					packCharRange(node));

				builder.CreateBr(bend);
							
				builder.SetInsertPoint(b2);

				builder.CreateCondBr(builder.CreateICmpEQ(v.type, typeRepr(TTup)), b3, b4);

				builder.SetInsertPoint(b3);
				// we get here if the type is TTup
				Value * v2 = builder.CreateCall4(
					getStdlibFunc("rm_tupEntryType"), 
					t.value,
					globalString(std::static_pointer_cast<VariableExp>(
									 node->args[1])->nameToken.getText(code)),
					typeRepr(wantedType),
					packCharRange(node));

				builder.CreateBr(bend);

				builder.SetInsertPoint(b4);

				// this is a hack. The value -1 tells us later that
				// the type isn't TRel/TTup, so we can emit an error
				// and abort. But we cannot abort directly inside blocks with
				// a following phi node; each block is required by llvm
				// to return a value to the phinode
				// Any suggestions for a better way? This will break if we
				// ever use -1 as internal value for booleans
				Value * v3 = int8(-1);

				builder.CreateBr(bend);

				builder.SetInsertPoint(bend);
							
				PHINode * phi = builder.CreatePHI(int8Type, 3, "istype");
				phi->addIncoming(v1, b1);
				phi->addIncoming(v2, b3);
				phi->addIncoming(v3, b4);

				// finally check if type was bad
				builder.CreateCondBr(builder.CreateICmpEQ(phi, int8(-1)), c1, c2);
				builder.SetInsertPoint(c1);
				builder.CreateCall3(getStdlibFunc("rm_emitIsTypeError"),
									int32(node->charRange.lo), int32(node->charRange.hi),
									v.type);
				builder.CreateUnreachable();				
				builder.SetInsertPoint(c2);

				disown(v, TAny);

				return OwnedLLVMVal(phi);


			}
			else{
				// not a TAny nor a TRel/TTup,
				// emit an error at compiletime
				rm_emitIsTypeError(node->charRange.lo, node->charRange.hi,
					node->args[0]->type);
				__builtin_unreachable();
			}
		}
		else if(node->args[0]->type == TAny){
			LLVMVal v = castVisit(node->args[0], TAny);
			LLVMVal r = cast(OwnedLLVMVal(builder.CreateSelect(
											  builder.CreateICmpEQ(v.type, typeRepr(wantedType)),
											  trueBool,
											  falseBool)), TBool, node->type, node);
			disown(v, TAny);
			return r;
		}
		else if(node->args[0]->type == wantedType)
			return OwnedLLVMVal(trueBool);
		else
			return OwnedLLVMVal(falseBool);
	}

	/** \brief Codegen for buildins */
	LLVMVal visit(std::shared_ptr<BuiltInExp> node) {
		switch (node->nameToken.id) {
		case TokenType::TK_PRINT:
		{
			LLVMVal v=castVisit(node->args[0], TAny);
			builder.CreateCall2(getStdlibFunc("rm_print"), v.type, v.value);
			disown(v, TAny);
			return OwnedLLVMVal(trueBool);
		}
		case TokenType::TK_HAS:			
			return opImp(
				{
					dOp([this, node](BorrowedLLVMVal v) -> OwnedLLVMVal {
							return builder.CreateCall2(
								getStdlibFunc("rm_tupHasEntry"), 
								v.value, 
								globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)));
						}, TBool, TTup),
					dOp([this, node](BorrowedLLVMVal v) -> OwnedLLVMVal {
							return builder.CreateCall2(
								getStdlibFunc("rm_relHasEntry"), 
								v.value, 
								globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)));
						}, TBool, TRel)
				}, 
				node,
				node->args[0]);
			break;
		case TokenType::TK_MAX:
		{	
			LLVMVal v=castVisit(node->args[0], TRel);
			OwnedLLVMVal r(builder.CreateCall3(
								   getStdlibFunc("rm_maxRel"),
								   v.value, 
								   globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)),
								   packCharRange(node)));
			disown(v, TRel);
			return LLVMVal(std::move(r));
		}
		case TokenType::TK_MIN:
		{	
			LLVMVal v=castVisit(node->args[0], TRel);
			OwnedLLVMVal r(builder.CreateCall3(
								   getStdlibFunc("rm_minRel"), 
								   v.value, 
								   globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)),
								   packCharRange(node)));
			disown(v, TRel);
			return LLVMVal(std::move(r));
		}
		case TokenType::TK_ADD:
		{	
			LLVMVal v=castVisit(node->args[0], TRel);
			OwnedLLVMVal r(builder.CreateCall3(
								   getStdlibFunc("rm_addRel"), 
								   v.value, 
								   globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)),
								   packCharRange(node)));
			disown(v, TRel);
			return LLVMVal(std::move(r));
		}
		case TokenType::TK_MULT:
		{	
			LLVMVal v=castVisit(node->args[0], TRel);
			OwnedLLVMVal r(builder.CreateCall3(
								   getStdlibFunc("rm_multRel"), 
								   v.value, 
								   globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)),
								   packCharRange(node)));
			disown(v, TRel);
			return LLVMVal(std::move(r));
		}
		case TokenType::TK_COUNT:
		{	
			LLVMVal v=castVisit(node->args[0], TRel);
			OwnedLLVMVal r(builder.CreateCall3(
								   getStdlibFunc("rm_countRel"), 
								   v.value, 
								   globalString(std::static_pointer_cast<VariableExp>(node->args[1])->nameToken.getText(code)),
								   packCharRange(node)));
			disown(v, TRel);
			return LLVMVal(std::move(r));
		}
		case TokenType::TK_ISATOM:
		{
			if(node->args[0]->type == TAny){
				LLVMVal v = castVisit(node->args[0], TAny);
				LLVMVal r = cast(OwnedLLVMVal(builder.CreateSelect(
												  builder.CreateICmpEQ(v.type, typeRepr(TInt)),
												  trueBool,
												  builder.CreateSelect(
													  builder.CreateICmpEQ(v.type, typeRepr(TBool)),
													  trueBool,
													  builder.CreateSelect(
														  builder.CreateICmpEQ(v.type, typeRepr(TText)),
														  trueBool,
														  falseBool
														  )))), TBool, node->type, node);
				disown(v, TAny);
				return r;
			}
			else if(node->args[0]->type == TBool || 
					node->args[0]->type == TInt || 
					node->args[0]->type == TText)
				return OwnedLLVMVal(trueBool);
			else
				return OwnedLLVMVal(falseBool);
			
		}
		case TokenType::TK_ISBOOL:
			return genIsExpression(node, TBool);
		case TokenType::TK_ISINT:
			return genIsExpression(node, TInt);
		case TokenType::TK_ISTEXT:
			return genIsExpression(node, TText);
		case TokenType::TK_ISTUP:
			return genIsExpression(node, TTup);
		case TokenType::TK_ISREL:
			return genIsExpression(node, TRel);
		case TokenType::TK_ISFUNC:
			return genIsExpression(node, TFunc);
		default:
			ICE("TODO BuildIn not implemented", node->nameToken.id, node->nameToken, node);
		}
	}

	/** \brief Codegen for constants */
	LLVMVal visit(std::shared_ptr<ConstantExp> node) {
		switch (node->valueToken.id) {
		case TokenType::TK_FALSE:
			return OwnedLLVMVal(falseBool);
        case TokenType::TK_TRUE:
			return OwnedLLVMVal(trueBool);
        case TokenType::TK_STDBOOL:
			return getUndef(TBool);
		case TokenType::TK_INT:
		case TokenType::TK_BADINT:
			return OwnedLLVMVal(int64(atol(node->valueToken.getText(code).c_str())));
		case TokenType::TK_TEXT: {
			std::string text=node->valueToken.getText(code);
			std::string ans;
			for (size_t i=1; i+1 < text.size(); ++i) {
				if (text[i] == '\\') {
					++i;
					switch (text[i]) {
					case 'r': ans.push_back('\r'); break;
					case 'n': ans.push_back('\n'); break;
					case 't': ans.push_back('\t'); break;
					default: ans.push_back(text[i]);
					}
				} else
					ans.push_back(text[i]);
			}
			return OwnedLLVMVal( builder.CreateCall(getStdlibFunc("rm_getConstText"), globalString(ans)));
		}
		case TokenType::TK_STDTEXT:
			return getUndef(TText);
        case TokenType::TK_STDINT:
			return getUndef(TInt);
        case TokenType::TK_ZERO:
			return extGlobalObject("zero_rel");
		case TokenType::TK_ONE:
			return extGlobalObject("one_rel");
		default:
			ICE("No codegen for", node->valueToken.id, node);
			break;		
		}
	}

	/** \brief Codegen for unary operations */
	LLVMVal visit(std::shared_ptr<UnsetExp> node) {
		if (node->isGlobal)
			builder.CreateCall(getStdlibFunc("rm_deleteGlobalAny"), globalString(node->nameToken));

		return OwnedLLVMVal(trueBool);
	}

	LLVMVal visit(std::shared_ptr<UnaryOpExp> node) {
		switch (node->opToken.id) {
		case TokenType::TK_NOT: {
			LLVMVal v=castVisit(node->exp, TBool);
			LLVMVal r=cast(OwnedLLVMVal(
							   builder.CreateSelect(
								   builder.CreateICmpEQ(v.value, undefBool),
								   undefBool,
								   builder.CreateSub(trueBool, v.value))), TBool, node->type, node);
			disown(v, TBool);
			return r;
		}
		case TokenType::TK_MINUS:
		{
			LLVMVal v=castVisit(node->exp, TInt);
			LLVMVal r=cast(OwnedLLVMVal(builder.CreateNeg(v.value)), TInt, node->type, node);
			disown(v, TInt);
			return r;
		}
		default:
			ICE("Unhandled operator", node->opToken.id, node);
		}
	}

	/** \brief Codegen for relation creators */
	LLVMVal visit(std::shared_ptr<RelExp> exp) {
		LLVMVal v = castVisit(exp->exp, TTup);
		LLVMVal v2 = OwnedLLVMVal(builder.CreateCall(getStdlibFunc("rm_createRel"), v.value));
		disown(v, TTup);
		return v2;
	}

	/** \brief Codegen for length computation */
	LLVMVal visit(std::shared_ptr<LenExp> exp) {
		LLVMVal v=castVisit(exp->exp, TAny);
		LLVMVal v2=OwnedLLVMVal(builder.CreateCall(
									getStdlibFunc("rm_length"), 
									builder.CreateIntToPtr(v.value, voidPtrType)));
		
		disown(v, TAny);
		return v2;
	}

	/** \brief Codegen for function invocation */
    LLVMVal visit(std::shared_ptr<FuncInvocationExp> node) {
    
		builder.CreateCall(getStdlibFunc("rm_checkAbort")); 
		builder.CreateCall(getStdlibFunc("rm_enterFunction")); 
	
		FunctionType * ft = funcType(node->args.size());
		
		LLVMVal cap=castVisit(node->funcExp, TFunc);
		Value * capture = builder.CreatePointerCast(cap.value, pointerType(funcBase));
		Value * rv = builder.CreateAlloca(anyRetType);
		
		std::stringstream ss1;
		ss1 << "check_fail_" << uid++;
		BasicBlock * fblock = BasicBlock::Create(getGlobalContext(), ss1.str(), getFunction());
		
		std::stringstream ss2;
		ss2 << "check_succ_" << uid++;
		BasicBlock * nblock = BasicBlock::Create(getGlobalContext(), ss2.str(), getFunction());
		
		Value * argc = builder.CreateLoad(builder.CreateConstGEP2_32(capture, 0, 2));
		Value * margc = int16(node->args.size());
		builder.CreateCondBr(builder.CreateICmpEQ(argc, margc), nblock, fblock);
		
		builder.SetInsertPoint(fblock);

		builder.CreateCall(getStdlibFunc("rm_exitFunction")); 
		builder.CreateCall4(getStdlibFunc("rm_emitArgCntError"), 
							int32(node->charRange.lo), 
							int32(node->charRange.hi),
							margc, 
							argc);
		builder.CreateUnreachable();

		builder.SetInsertPoint(nblock);

		std::vector<LLVMVal> ac;
		std::vector<Value *> args={capture, rv};
		for (auto arg: node->args) {
			ac.push_back(castVisit(arg, TAny));
			args.push_back(ac.back().value);
			args.push_back(ac.back().type);
		}

		Value * voidfp = builder.CreateLoad(builder.CreateConstGEP2_32(capture, 0, 4));
		Value * fp = builder.CreatePointerCast(voidfp, pointerType(ft));
		builder.CreateCall(fp, args);
		
		for (auto & a: ac)
			disown(a, TAny);
		disown(cap, TFunc);

		builder.CreateCall(getStdlibFunc("rm_exitFunction")); 

		return OwnedLLVMVal(builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 0)), 
							builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 1)));
	}

	/** \brief Codegen for substring expressions */
	LLVMVal visit(std::shared_ptr<SubstringExp> node) {
		LLVMVal s = castVisit(node->stringExp, TText);
		LLVMVal f = castVisit(node->fromExp, TInt);
		LLVMVal t = castVisit(node->toExp, TInt);
		LLVMVal r = cast(OwnedLLVMVal(builder.CreateCall3(getStdlibFunc("rm_substrText"), s.value, f.value, t.value)), TText, node->type, node);
		disown(s, TText);
		disown(f, TInt);
		disown(t, TInt);
		return r;;
	}

	/** \brief Codegen for relation rename */
	LLVMVal visit(std::shared_ptr<RenameExp> exp) {
		Value * names = builder.CreateAlloca(voidPtrType, int32(2*exp->renames.size()) );
		for (size_t i=0; i < exp->renames.size(); ++i) {
			builder.CreateStore(globalString(exp->renames[i]->fromNameToken), builder.CreateConstGEP1_32(names, 2*i));
			builder.CreateStore(globalString(exp->renames[i]->toNameToken), builder.CreateConstGEP1_32(names, 2*i+1));
		}
		
		LLVMVal rel=castVisit(exp->lhs, TRel);
		LLVMVal ret=OwnedLLVMVal(builder.CreateCall4(getStdlibFunc("rm_renameRel"), rel.value, int32(exp->renames.size()), names, packCharRange(exp)));
		disown(rel, TRel);
		return ret;

	}

	/** \brief Codegen for tuple lookup */
	LLVMVal visit(std::shared_ptr<DotExp> node) {
		LLVMVal tup=castVisit(node->lhs, TTup);
		Value * rv = builder.CreateAlloca(anyRetType);
		builder.CreateCall4(getStdlibFunc("rm_tupEntry"), tup.value, globalString(node->nameToken), rv, packCharRange(node));
		
		OwnedLLVMVal r=cast(OwnedLLVMVal(builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 0)), 
										 builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 1))),
							TAny, node->type, node);
		disown(tup, TTup);
		return LLVMVal(std::move(r));
	}

	/** \brief Codegen for tuple removal */
	LLVMVal visit(std::shared_ptr<TupMinus> node) {
		LLVMVal tup=castVisit(node->lhs, TTup);

		OwnedLLVMVal ans(builder.CreateCall3(getStdlibFunc("rm_tupRemove"), tup.value, globalString(node->nameToken),
											 packCharRange(node)));
		disown(tup, TTup);
		return LLVMVal(std::move(ans));
	}

	/** \brief Codegen for ?? */
	LLVMVal visit(std::shared_ptr<AtExp> node) {
		ICE("TODO At", node);
	}

	/** \brief Codegen projections */
	LLVMVal visit(std::shared_ptr<ProjectExp> exp) {
		Value * names = builder.CreateAlloca(voidPtrType, int32(exp->names.size()) );
		for (size_t i=0; i < exp->names.size(); ++i)
			builder.CreateStore(globalString(exp->names[i]), builder.CreateConstGEP1_32(names, i));
		
		LLVMVal rel=castVisit(exp->lhs, TRel);
		LLVMVal ret;
		switch (exp->projectionToken.id) {
		case TokenType::TK_PROJECT_MINUS:
			ret=OwnedLLVMVal(builder.CreateCall4(getStdlibFunc("rm_projectMinusRel"), rel.value, int32(exp->names.size()), names, packCharRange(exp)));
			break;
		case TokenType::TK_PROJECT_PLUS:
			ret=OwnedLLVMVal(builder.CreateCall4(getStdlibFunc("rm_projectPlusRel"), rel.value, int32(exp->names.size()), names, packCharRange(exp)));
			break;
		default:
			ICE("Bad project", exp->projectionToken.id, exp->projectionToken, exp);
		}
		disown(rel, TRel);
		return ret;
	}
	
	/**
	 * \brief Codegen a typed binary operation
	 * \param ops The possible operations
	 */
	LLVMVal binopImpl(std::shared_ptr<BinaryOpExp> node, 
					  std::initializer_list<OpImpl<::Type, ::Type> > ops) {
		return opImp(ops, node, node->lhs, node->rhs);
	}
	
	/** \brief Codegen a binary operation*/
	LLVMVal visit(std::shared_ptr<BinaryOpExp> node) {
		switch (node->opToken.id) {
		case TokenType::TK_CONCAT:
			return binopImpl(node, { 
					dCall("rm_concatText", TText, TText, TText)
						});
		case TokenType::TK_PLUS:
			return binopImpl(node, { 
					dOpU([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateAdd(lhs.value, rhs.value);
						}, TInt, TInt, TInt),
						dCallR("rm_unionRel", node, TRel, TRel, TRel)	
						});
		case TokenType::TK_MUL:
			return binopImpl(node, {
					dOpU([this, node](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateMul(lhs.value, rhs.value);
						}, TInt, TInt, TInt),
						dCallR("rm_joinRel", node, TRel, TRel, TRel)
						});
		case TokenType::TK_MINUS:
			return binopImpl(node, {
					dOpU([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateSub(lhs.value, rhs.value);
						}, TInt, TInt, TInt),
						dCallR("rm_diffRel", node, TRel, TRel, TRel)
				});
		case TokenType::TK_DIV:
			return binopImpl(node, {
					dOpU([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							BasicBlock * b1 = newBlock();		
							BasicBlock * b2 = newBlock();
							BasicBlock * bend = newBlock();
							builder.CreateCondBr(builder.CreateICmpEQ(rhs.value, int64(0)), b1, b2);
							
							builder.SetInsertPoint(b1);
							Value * v1 = undefInt;
							builder.CreateBr(bend);
							
							builder.SetInsertPoint(b2);
							Value * v2 = builder.CreateSDiv(lhs.value, rhs.value);
							builder.CreateBr(bend);
							
							builder.SetInsertPoint(bend);
							
							PHINode * phi = builder.CreatePHI(int64Type, 2, "div");
							phi->addIncoming(v1, b1);
							phi->addIncoming(v2, b2);
							return phi;
						}, TInt, TInt, TInt)
						});
		case TokenType::TK_MOD:
			return binopImpl(node, {
					dOpU([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							BasicBlock * b1 = newBlock();		
							BasicBlock * b2 = newBlock();
							BasicBlock * bend = newBlock();
							builder.CreateCondBr(builder.CreateICmpEQ(rhs.value, int64(0)), b1, b2);
							
							builder.SetInsertPoint(b1);
							Value * v1 = undefInt;
							builder.CreateBr(bend);
							
							builder.SetInsertPoint(b2);
							Value * v2 = builder.CreateSRem(lhs.value, rhs.value);
							builder.CreateBr(bend);
							
							builder.SetInsertPoint(bend);
							
							PHINode * phi = builder.CreatePHI(int64Type, 2, "mod");
							phi->addIncoming(v1, b1);
							phi->addIncoming(v2, b2);
							return phi;
						}, TInt, TInt, TInt)
						});
		case TokenType::TK_LESS:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_SLT),
					dComp(TBool, llvm::CmpInst::ICMP_SLT)
					});
		case TokenType::TK_LESSEQUAL:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_SLE),
					dComp(TBool, llvm::CmpInst::ICMP_SLE)
						});
		case TokenType::TK_GREATER:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_SGT),
					dComp(TBool, llvm::CmpInst::ICMP_SGT)
						});
		case TokenType::TK_GREATEREQUAL:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_SGE),
					dComp(TBool, llvm::CmpInst::ICMP_SGE)
						});
		case TokenType::TK_EQUAL:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_EQ),
					dComp(TBool, llvm::CmpInst::ICMP_EQ),
					dCall("rm_equalRel", TBool, TRel, TRel),
					dCall("rm_equalTup", TBool, TTup, TTup),
					dCall("rm_equalText", TBool, TText, TText),
						});
		case TokenType::TK_DIFFERENT:
			return binopImpl(node, { 
					dComp(TInt, llvm::CmpInst::ICMP_NE),
					dComp(TBool, llvm::CmpInst::ICMP_NE),
					dBInv(dCall("rm_equalRel", TBool, TRel, TRel)),
					dBInv(dCall("rm_equalTup", TBool, TTup, TTup)),
					dBInv(dCall("rm_equalText", TBool, TText, TText)),
						});
		case TokenType::TK_AND:
			return binopImpl(node, {
					dOp([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateAnd(lhs.value, rhs.value);
						}, TBool, TBool, TBool)
						});
		case TokenType::TK_OR:
			return binopImpl(node, {
					dOp([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateOr(lhs.value, rhs.value);
						}, TBool, TBool, TBool)
						});
		case TokenType::TK_QUESTION:
			return binopImpl(node, {
					dOp([this](BorrowedLLVMVal lhs, BorrowedLLVMVal rhs)->OwnedLLVMVal {
							return builder.CreateCall2(getStdlibFunc("rm_selectRel"), lhs.value, 
													   builder.CreatePointerCast(rhs.value, voidPtrType));
						}, TRel, TRel, TFunc)
						});
		case TokenType::TK_OPEXTEND:
			return binopImpl(node, { dCall("rm_extendTup", TTup, TTup, TTup) });
		default: 
			ICE("Binop not implemented", node->opToken.id, node->opToken, node);
		}
	}

	/** \brief Codegen a sequence */
	LLVMVal visit(std::shared_ptr<SequenceExp> node) {
		::Type t=TInvalid;
		LLVMVal x;
		for(auto n: node->sequence) {
			disown(x, t);
			x = visitNode(n);
			t=n->type;
		}
		
		LLVMVal ret=takeOwnership(std::move(x), t);
		
		for(auto n: node->sequence) {
			if (n->nodeType != NodeType::AssignmentExp) continue;
			std::shared_ptr<AssignmentExp> a=std::static_pointer_cast<AssignmentExp>(n);
			if (a->globalId != NOT_GLOBAL) continue;
			disown(a->llvmVal, a->type);
		}

		return ret;
	}

	/**
	 * Create a function for a given ast
	 */
	llvm::Function * translate(NodePtr ast) override {
		size_t id=uid++;
		llvm::FunctionType * ft = FunctionType::get(llvm::Type::getVoidTy(getGlobalContext()), {}, false);
		std::stringstream ss;
		ss << "INNER_" << id;
		Function * function = Function::Create(ft, Function::ExternalLinkage, ss.str(), module);
		BasicBlock * block = BasicBlock::Create(getGlobalContext(), "entry", function);
		builder.SetInsertPoint(block);
		LLVMVal val=visitNode(ast);
		disown(val, ast->type);
		builder.CreateRetVoid();
		finishFunction(function);
		return function ;
	}

	LLVMVal visit(std::shared_ptr<Choice> node) {ICE("Choice", node);}
	LLVMVal visit(std::shared_ptr<FuncCaptureValue> node) {ICE("FuncCaptureValue", node);}
	LLVMVal visit(std::shared_ptr<FuncArg> node) {ICE("FuncArg", node);}
	LLVMVal visit(std::shared_ptr<TupItem> node) {ICE("TupItem", node);}
	LLVMVal visit(std::shared_ptr<InvalidExp> node) {ICE("InvalidExp", node);}
	LLVMVal visit(std::shared_ptr<Val> node) {ICE("Val", node);}
	LLVMVal visit(std::shared_ptr<RenameItem> node) {ICE("RenameItem", node);}
};

} //nameless namespace

namespace rasmus {
namespace frontend {

std::shared_ptr<LLVMCodeGen> makeLlvmCodeGen(
	std::shared_ptr<Error> error, std::shared_ptr<Code> code, llvm::Module * module,
	bool dumpRawFunctions, bool dumpOptFunctions) {
	return std::make_shared<CodeGen>(error, code, module, dumpRawFunctions, dumpOptFunctions);
} 

} //namespace frontend
} //namespace rasmus
