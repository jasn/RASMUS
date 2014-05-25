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
#include <unordered_map>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/PassManager.h>
#include <llvm/Assembly/PrintModulePass.h>
#include <llvm/IR/Instructions.h>
namespace {
using namespace llvm;

class ICEException: public std::runtime_error {
public:
	ICEException(const std::string & msg): std::runtime_error(msg) {}
};


llvm::Type * voidType = llvm::Type::getVoidTy(getGlobalContext());
llvm::Type * int8Type = llvm::Type::getInt8Ty(getGlobalContext());
llvm::Type * int16Type = llvm::Type::getInt16Ty(getGlobalContext());
llvm::Type * int32Type = llvm::Type::getInt32Ty(getGlobalContext());
llvm::Type * int64Type = llvm::Type::getInt64Ty(getGlobalContext());
llvm::Type * pointerType(llvm::Type * t) {return PointerType::getUnqual(t);}
llvm::Type * voidPtrType = pointerType(voidType);

llvm::StructType * structType(std::string name, 
						std::initializer_list<llvm::Type *> types) {
	return StructType::create(getGlobalContext(), ArrayRef<llvm::Type * >(types.begin(), types.end()), name);
}

llvm::FunctionType * functionType(llvm::Type * ret, std::initializer_list<llvm::Type *> args) {
	return FunctionType::get(ret, ArrayRef<llvm::Type * >(args.begin(), args.end()), false);
}

llvm::StructType * anyRetType = structType("AnyRet", {int64Type, int8Type});
llvm::StructType * funcBase = structType("FuncBase", {pointerType(voidType), int16Type});

llvm::Type * llvmType(::Type t) {
	switch (t) {
	case TBool: return int8Type;
	case TInt: return int64Type;
	case TFunc: return pointerType(funcBase);
	default:
		throw ICEException(std::string("llvmType - Unhandled type ") + typeName(t));
	}
}

llvm::Value * int8(uint8_t value) {	return llvm::ConstantInt::get(int8Type, value);}
llvm::Value * int16(uint16_t value) {return llvm::ConstantInt::get(int16Type, value);}
llvm::Value * int32(uint32_t value) {return llvm::ConstantInt::get(int32Type, value);}
llvm::Value * int64(uint8_t value) {return llvm::ConstantInt::get(int64Type, value);}

llvm::Value * typeRepr(::Type t) {
	return int8(t);
}
	
LLVMVal getUndef(::Type t) {
	switch(t) {
	case TInt:
		return int64(std::numeric_limits<int64_t>::max() ); 
	case TBool:
		return int8(2);
	case TAny:
		return LLVMVal(int64(std::numeric_limits<int64_t>::max() ), typeRepr(TInt) ) ;
	case TFunc:
		return llvm::Constant::getNullValue(pointerType(funcBase));
	default:
		throw ICEException("Unhandled undef");
	}
}

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

class CodeGen: public LLVMCodeGen, public VisitorCRTP<CodeGen, LLVMVal> {
public:
	IRBuilder<> builder;
	size_t uid;
	Module * module;
	std::shared_ptr<Error> error;
	std::shared_ptr<Code> code;

	Function * getFunction() {
		return builder.GetInsertBlock()->getParent();
	}

	BasicBlock * newBlock() {
		std::stringstream ss;
		ss << "b" << uid++;
		return BasicBlock::Create(getGlobalContext(), ss.str(), getFunction());
	}
	
	LLVMVal cast(LLVMVal value, ::Type tfrom, ::Type tto, NodePtr node) {
		if (tfrom == tto) return value;
		if (tto == TAny) {
			switch(tfrom) {
			case TInt:
				return LLVMVal(value.value, typeRepr(tfrom));
			case TBool:
				return LLVMVal(builder.CreateZExt(value.value, int64Type), typeRepr(tfrom));
			case TText:
			case TFunc:
			case TRel:
				return LLVMVal(builder.CreatePtrToInt(value.value, int64Type), typeRepr(tfrom));
            default:
                throw ICEException("Unhandled type1");
			};
		}

		if (tfrom == TAny) {
			BasicBlock * fblock = newBlock();
			BasicBlock * nblock = newBlock();
			builder.CreateCondBr(builder.CreateICmpEQ(value.type, typeRepr(tto)), nblock, fblock);
			builder.SetInsertPoint(fblock);
			builder.CreateCall4(stdlib["rm_emitTypeError"],
								int32(node->charRange.lo), int32(node->charRange.hi),
								value.type,
								typeRepr(tto));
			builder.CreateUnreachable();
			builder.SetInsertPoint(nblock);
			switch (tto) {
			case TInt:
				return value.value;
			case TBool:
				return builder.CreateTruncOrBitCast(value.value, llvmType(tto));
			case TText:
			case TFunc:
			case TRel:
				return builder.CreateIntToPtr(value.value, voidPtrType);
			default:
				throw ICEException("Unhandled type 2");
			}
		}

		if (tfrom == TText)
			throw ICEException("Unhandled type 3");
		throw ICEException("Unhandled type 4");
	}

	
	LLVMVal castVisit(NodePtr node, ::Type tto) {
		return cast(visitNode(node), node->type, tto, node);
	}

	FunctionPassManager fpm;

	CodeGen(std::shared_ptr<Error> error, std::shared_ptr<Code> code,
			llvm::Module * module): error(error), code(code), module(module),
									builder(getGlobalContext()), fpm(module), uid(0) {

		//fpm.add(new DataLayout(module));
		//fpm.add(createBasicAliasAnalysisPass());
		// Do simple "peephole" optimizations and bit-twiddling optzns.
		//fpm.add(createInstructionCombiningPass());
		// Reassociate expressions.
		//fpm.add(createReassociatePass());
		// Eliminate Common SubExpressions.
		//fpm.add(createGVNPass());
		// Simplify the control flow graph (deleting unreachable blocks, etc).
		//fpm.add(createCFGSimplificationPass());
		fpm.doInitialization();

		std::vector< std::pair<std::string, FunctionType *> > fs =
		{
			{"rm_print", functionType(voidType, {int8Type, int64Type})},
			{"rm_concatText", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_getConstText", functionType(voidPtrType, {pointerType(int8Type)})},
			{"rm_emitTypeError", functionType(voidType, {int32Type, int32Type, int8Type, int8Type})},
			{"rm_emitArgCntError", functionType(voidType, {int32Type, int32Type, int16Type, int16Type})},
			{"rm_unionRel", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_joinRel", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_loadRel", functionType(voidPtrType, {pointerType(int8Type)})},
			{"rm_saveRel", functionType(voidType, {voidPtrType, pointerType(int8Type)})},
			{"rm_substrText", functionType(voidPtrType, {voidPtrType, int64Type, int64Type})}
		};

		for(auto p: fs)
			stdlib[p.first] = Function::Create(p.second, Function::ExternalLinkage, p.first, module);
	}

	std::string tokenToIdentifier(Token token) const {
		return code->code.substr(token.start, token.length);
	}
	
	LLVMVal visit(std::shared_ptr<VariableExp> node) {
		if (NodePtr store = node->store.lock()) {
			if (store->nodeType != NodeType::AssignmentExp) return store->llvmVal;
			std::shared_ptr<AssignmentExp> st=std::static_pointer_cast<AssignmentExp>(store);
			if (!st->global) return store->llvmVal;

			// TODO once we stop using any in assigment I also need to change
			LLVMVal v(
				builder.CreateLoad(builder.CreateConstGEP2_32(st->llvmGlobal, 0, 0)),
				builder.CreateLoad(builder.CreateConstGEP2_32(st->llvmGlobal, 0, 1)));
			
			return cast(v, TAny, node->type, node);
		} else {
			Constant * c = ConstantDataArray::getString(getGlobalContext(), tokenToIdentifier(node->nameToken) );
			GlobalVariable * gv = new GlobalVariable(*module,
													 c->getType(),
													 true,
													 llvm::GlobalValue::PrivateLinkage,
													 c);
			return builder.CreateCall(
				stdlib["rm_loadRel"],
				builder.CreateConstGEP2_32(gv, 0, 0));
		}
	}
	
	LLVMVal visit(std::shared_ptr<AssignmentExp> node) {
		LLVMVal val=visitNode(node->valueExp);
		if (!node->global)
			return node->llvmVal=val;

		//TODO there is noo need to store every global variable as a any type
		LLVMVal v=cast(val, node->type, TAny, node);

		GlobalVariable * gv = new GlobalVariable(
			*module,
			anyRetType,
			false,
			llvm::GlobalValue::PrivateLinkage, 
			llvm::ConstantStruct::get(anyRetType, int64(0), int8(0), NULL));
		node->llvmGlobal = gv;
		builder.CreateStore(v.value,  builder.CreateConstGEP2_32(gv, 0, 0));
		builder.CreateStore(v.type,  builder.CreateConstGEP2_32(gv, 0, 1));

		
		std::stringstream ss1;
		ss1 << "store_rel_" << uid++;
		BasicBlock * sblock = BasicBlock::Create(getGlobalContext(), ss1.str(), getFunction());

		std::stringstream ss2;
		ss2 << "continue_" << uid++;
		BasicBlock * cblock = BasicBlock::Create(getGlobalContext(), ss2.str(), getFunction());


		builder.CreateCondBr(builder.CreateICmpEQ(v.type, typeRepr(TRel)), sblock, cblock);
		builder.SetInsertPoint(sblock);
		Constant * c = ConstantDataArray::getString(getGlobalContext(), tokenToIdentifier(node->nameToken) );
		GlobalVariable * ng = new GlobalVariable(*module,
												 c->getType(),
												 true,
												 llvm::GlobalValue::PrivateLinkage,
												 c);
		builder.CreateCall2(stdlib["rm_saveRel"], 
							builder.CreateIntToPtr(v.value, voidPtrType),
							builder.CreateConstGEP2_32(ng, 0, 0));
		builder.CreateBr(cblock);
		builder.SetInsertPoint(cblock);
		return val;
	}

	LLVMVal visit(std::shared_ptr<IfExp> node) {
		if (node->type == TAny) 
			throw ICEException("If not implemented for anytype");
		bool done=false;
		std::vector<std::pair<LLVMVal, LLVMVal> > hats;
		LLVMVal val;
		for (auto choice: node->choices) {
			// Evaluate condition and cast value to bool
			LLVMVal cond = castVisit(choice->condition, TBool);

			llvm::ConstantInt * ci = dyn_cast<llvm::ConstantInt>(cond.value);
			if (done || (ci && ci->isZeroValue())) {
				error->reportWarning("Branch never taken", choice->arrowToken);
				continue;
			}
			
			if (ci && !ci->isZeroValue()) 
				done = true;

			// Evaluate value and cast to result type
			LLVMVal value = castVisit(choice->value, node->type);
			hats.push_back({cond, value});
		}
		if (!done) 
			val = getUndef(node->type);
		else {
			val = hats.back().second;
			hats.pop_back();
		}
		
		while (!hats.empty()) {
			val.value = builder.CreateSelect(hats.back().first.value, hats.back().second.value , val.value);
			hats.pop_back();
		}
		return val;
	}

	LLVMVal visit(std::shared_ptr<ForallExp> node) {
		throw ICEException("ForallExp");
	}

	LLVMVal visit(std::shared_ptr<FuncExp> node) {
		// Create function type
		FunctionType * ft = funcType(node->args.size());

		std::vector<llvm::Type *> captureContent = {
			pointerType(ft), //Function ptr
			int16Type, //Number Of arguments
		};
		// for (auto cap: node->capture)
		// 	captureContent.push_back(llvmType(cap->type));
		StructType * captureType = StructType::create(getGlobalContext(), captureContent);
		
		//Cache currentFunction
		auto old_ip = builder.saveIP();
		
		std::stringstream ss;
		ss << "f" << uid++;

		Function * function = Function::Create(ft, Function::InternalLinkage, ss.str(), module);
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
		Value * selfv = builder.CreateBitCast(self, pointerType(captureType));
		for (auto a: node->args) {
			Value * v = &(*arg);
			++arg;
			Value * t = &(*arg);
			++arg;
			t->setName("abe");
			v->setName("kat");
			// t->setName(std::string("t_")+a->name
			// v->setName(std::string("v_")+a->name
			a->llvmVal = cast(LLVMVal(v, t), TAny, a->type, a);
		}

//         for i in range(len(node.captures)):
//             cap = node.captures[i]
//             cap.value = self.builder.load(self.builder.gep(selfv, [intp(0), intp(2+i)]))
        
		// Build function code
		LLVMVal x = cast(visitNode(node->body), node->body->type, TAny, node->body);
		builder.CreateStore(x.value, builder.CreateConstGEP2_32(ret, 0, 0));
		builder.CreateStore(x.type, builder.CreateConstGEP2_32(ret, 0, 1));
		builder.CreateRetVoid();
		
		llvm::verifyFunction(*function);
		function->dump();
		fpm.run(*function);

		// Revert state
		builder.restoreIP(old_ip);

		/*Type * ty = llvm::Type::getInt32Ty(getGlobalContext());*/
		Constant* AllocSize = ConstantExpr::getSizeOf(captureType);
		AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, int32Type);
		Instruction * m = CallInst::CreateMalloc(builder.GetInsertBlock(), int32Type, captureType, AllocSize);
		auto p = builder.Insert(m);
		
		builder.CreateStore(function, builder.CreateConstGEP2_32(p, 0, 0));
		builder.CreateStore(int16(node->args.size()), builder.CreateConstGEP2_32(p, 0, 1));

//         # Store captures
//         for i in range(len(node.captures)):
//             cap = node.captures[i]
//             self.builder.store(cap.store.value, self.builder.gep(p, [intp(0), intp(2+i)]))
		return p;
	}
	
	LLVMVal visit(std::shared_ptr<TupExp> node) {
		throw ICEException("Tub not implemented");
	}

	LLVMVal visit(std::shared_ptr<BlockExp> node) {
		for (auto v: node->vals) 
			v->exp->llvmVal = visitNode(v->exp);
		return visitNode(node->inExp);
	}

	LLVMVal visit(std::shared_ptr<BuiltInExp> node) {
		switch (node->nameToken.id) {
		case TK_PRINT:
		{
			LLVMVal v=castVisit(node->args[0], TAny);
			builder.CreateCall2(stdlib["rm_print"], v.type, v.value);
			return int8(1);
		}
		break;
		default:
			throw ICEException("BuildIn not implemented");
		}
	}

	LLVMVal visit(std::shared_ptr<ConstantExp> node) {
		 switch (node->type) {
		 case TInt:
			 return LLVMVal(int64(node->int_value));
		 case TBool:
			 return LLVMVal(int8(node->bool_value?1:0));
		 case TText:
		 {
			 Constant * c = ConstantDataArray::getString(getGlobalContext(), node->txt_value);
			 GlobalVariable * gv = new GlobalVariable(*module,
													  c->getType(),
													  true,
													  llvm::GlobalValue::PrivateLinkage,
													  c);
			 return builder.CreateCall(
				 stdlib["rm_getConstText"],
				 builder.CreateConstGEP2_32(gv, 0, 0));
		 }
		 default:
			 throw ICEException("Const");
		 }
	}
	
	LLVMVal visit(std::shared_ptr<UnaryOpExp> node) {
		switch (node->opToken.id) {
		case TK_NOT:
			return cast(builder.CreateNot(castVisit(node->exp, TBool).value), TBool, node->type, node);
		case TK_MINUS:
			return cast(builder.CreateNeg(castVisit(node->exp, TInt).value), TInt, node->type, node);
		default:
			throw ICEException("Unhandled unary operator");
		}
	}

	LLVMVal visit(std::shared_ptr<RelExp> node) {
        throw ICEException("Rel");
	}

	LLVMVal visit(std::shared_ptr<LenExp> node) {
		throw ICEException("Len");
	}

    LLVMVal visit(std::shared_ptr<FuncInvocationExp> node) {
        FunctionType * ft = funcType(node->args.size());
		
		Value * capture = builder.CreateBitCast(castVisit(node->funcExp, TFunc).value, pointerType(funcBase));
		Value * rv = builder.CreateAlloca(anyRetType);
		
		std::stringstream ss1;
		ss1 << "check_fail_" << uid++;
		BasicBlock * fblock = BasicBlock::Create(getGlobalContext(), ss1.str(), getFunction());
		
		std::stringstream ss2;
		ss2 << "check_succ_" << uid++;
		BasicBlock * nblock = BasicBlock::Create(getGlobalContext(), ss2.str(), getFunction());
		
		Value * argc = builder.CreateLoad(builder.CreateConstGEP2_32(capture, 0, 1));
		Value * margc = int16(node->args.size());
		builder.CreateCondBr(builder.CreateICmpEQ(argc, margc), nblock, fblock);
		
		builder.SetInsertPoint(fblock);

		builder.CreateCall4(stdlib["rm_emitArgCntError"], 
							int32(node->charRange.lo), 
							int32(node->charRange.hi),
							margc, 
							argc);
		builder.CreateUnreachable();

		builder.SetInsertPoint(nblock);

		std::vector<Value *> args={capture, rv};
		for (auto arg: node->args) {
			LLVMVal v=castVisit(arg, TAny);
			args.push_back(v.value);
			args.push_back(v.type);
		}

		Value * voidfp = builder.CreateLoad(builder.CreateConstGEP2_32(capture, 0, 0));
		Value * fp = builder.CreateBitCast(voidfp, pointerType(ft));
		builder.CreateCall(fp, args);
		
		return LLVMVal(
			builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 0)),
			builder.CreateLoad(builder.CreateConstGEP2_32(rv, 0, 1)));
	}

	LLVMVal visit(std::shared_ptr<SubstringExp> node) {
		LLVMVal s = castVisit(node->stringExp, TText);
		LLVMVal f = castVisit(node->fromExp, TInt);
		LLVMVal t = castVisit(node->toExp, TInt);
		return cast(builder.CreateCall3(stdlib["rm_substrText"], s.value, f.value, t.value), TText, node->type, node);
	}

	LLVMVal visit(std::shared_ptr<RenameExp> node) {
		throw ICEException("Rename");
	}
        
	LLVMVal visit(std::shared_ptr<DotExp> node) {
		throw ICEException("DotExp");
	}

	LLVMVal visit(std::shared_ptr<AtExp> node) {
		throw ICEException("At");
	}

	LLVMVal visit(std::shared_ptr<ProjectExp> node) {
		throw ICEException("Project");
	}


	LLVMVal visit(std::shared_ptr<Choice> node) {throw ICEException("Choice");}
	LLVMVal visit(std::shared_ptr<FuncCaptureValue> node) {throw ICEException("FuncCaptureValue");}
	LLVMVal visit(std::shared_ptr<FuncArg> node) {throw ICEException("FuncArg");}
	LLVMVal visit(std::shared_ptr<TupItem> node) {throw ICEException("TupItem");}
	LLVMVal visit(std::shared_ptr<InvalidExp> node) {throw ICEException("InvalidExp");}
	LLVMVal visit(std::shared_ptr<Val> node) {throw ICEException("Val");}
	LLVMVal visit(std::shared_ptr<RenameItem> node) {throw ICEException("RenameItem");}

	struct BinopHelp {
		::Type lhsType;
		::Type rhsType;
		::Type resType;
		std::function<LLVMVal(CodeGen&, LLVMVal, LLVMVal) > func;
	};
	
	LLVMVal binopImpl(std::shared_ptr<BinaryOpExp> node, std::initializer_list<BinopHelp> types) {
		std::vector<BinopHelp> matches;
		for(auto h: types) {
			if (h.lhsType != node->lhs->type && node->lhs->type != TAny) continue;
			if (h.rhsType != node->rhs->type && node->rhs->type != TAny) continue;
			matches.push_back(h);
		}
		if (matches.size() == 0)
			throw ICEException("Infeasible types in binopImpl, there is a bug in the typechecker!!");

		if (matches.size() == 1) {
			// Even though there might be any arguments
			// we have determined that this is the only possible alloweable call
			BinopHelp h = matches[0];
			LLVMVal a = castVisit(node->lhs, h.lhsType);
			LLVMVal b = castVisit(node->rhs, h.rhsType);
			LLVMVal v = h.func(*this, a, b);
			return cast(v, h.resType, node->type, node);
		} 

		// There was more then one possible operator we want to call
		// So we have to determine on runtime which is the right
		throw ICEException("Dynamic binops are not implemented");
	}

	LLVMVal binopAddInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateAdd(lhs.value, rhs.value);
	}

	LLVMVal binopMinusInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateSub(lhs.value, rhs.value);
	}

	LLVMVal binopMulInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateMul(lhs.value, rhs.value);
	}

	LLVMVal binopDivInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateSDiv(lhs.value, rhs.value);
	}

	LLVMVal binopModInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateSRem(lhs.value, rhs.value);
	}

	LLVMVal binopConcat(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateCall2(stdlib["rm_concatText"], lhs.value, rhs.value);
	}

	LLVMVal binopLessInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpSLT(lhs.value, rhs.value);
	}

	LLVMVal binopLessBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpULT(lhs.value, rhs.value);
	}

	LLVMVal binopLessEqualInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpSLE(lhs.value, rhs.value);
	}

	LLVMVal binopLessEqualBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpULE(lhs.value, rhs.value);
	}

	LLVMVal binopGreaterInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpSGT(lhs.value, rhs.value);
	}

	LLVMVal binopGreaterBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpUGT(lhs.value, rhs.value);
	}

	LLVMVal binopGreaterEqualInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpSGE(lhs.value, rhs.value);
	}

	LLVMVal binopGreaterEqualBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpUGE(lhs.value, rhs.value);
	}

	LLVMVal binopEqualInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpEQ(lhs.value, rhs.value);
	}

	LLVMVal binopEqualBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpEQ(lhs.value, rhs.value);
	}

	LLVMVal binopDifferentInt(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpNE(lhs.value, rhs.value);
	}

	LLVMVal binopDifferentBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateICmpNE(lhs.value, rhs.value);
	}

	LLVMVal binopAndBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateAnd(lhs.value, rhs.value);
	}

	LLVMVal binopOrBool(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateOr(lhs.value, rhs.value);
	}

	LLVMVal binopUnionRel(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateCall2(stdlib["rm_unionRel"], lhs.value, rhs.value);
	}

	LLVMVal binopJoinRel(LLVMVal lhs, LLVMVal rhs) {
		return builder.CreateCall2(stdlib["rm_joinRel"], lhs.value, rhs.value);
	}
	
	LLVMVal visit(std::shared_ptr<BinaryOpExp> node) {
		switch (node->opToken.id) {
		case TK_CONCAT:
			return binopImpl(node, { {TText, TText, TText, &CodeGen::binopConcat} });
		case TK_PLUS:
			return binopImpl(node, { 
					{TInt, TInt, TInt, &CodeGen::binopAddInt},
					{TRel, TRel, TRel, &CodeGen::binopUnionRel}
				});
		case TK_MUL:
			return binopImpl(node, {
					{TInt, TInt, TInt, &CodeGen::binopMulInt},
					{TRel, TRel, TRel, &CodeGen::binopJoinRel}
				});
		case TK_MINUS:
			return binopImpl(node, { {TInt, TInt, TInt, &CodeGen::binopMinusInt} });
		case TK_DIV:
			return binopImpl(node, { {TInt, TInt, TInt, &CodeGen::binopDivInt} });
		case TK_MOD:
			return binopImpl(node, { {TInt, TInt, TInt, &CodeGen::binopModInt} });
		case TK_LESS:
			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopLessInt} ,
					{TBool, TBool, TBool, &CodeGen::binopLessBool} ,
						});
		case TK_LESSEQUAL:
			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopLessEqualInt} ,
					{TBool, TBool, TBool, &CodeGen::binopLessEqualBool} ,
						});
		case TK_GREATER:
			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopGreaterInt} ,
					{TBool, TBool, TBool, &CodeGen::binopGreaterBool} ,
						});
		case TK_GREATEREQUAL:
			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopGreaterEqualInt} ,
					{TBool, TBool, TBool, &CodeGen::binopGreaterEqualBool} ,
						});
		case TK_EQUAL:
			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopEqualInt} ,
					{TBool, TBool, TBool, &CodeGen::binopEqualBool} ,
						});
		case TK_DIFFERENT:

			return binopImpl(node, { 
					{TInt, TInt, TBool, &CodeGen::binopDifferentInt} ,
					{TBool, TBool, TBool, &CodeGen::binopDifferentBool} ,
						});
		case TK_AND:
			return binopImpl(node, { {TBool, TBool, TBool, &CodeGen::binopAndBool} });
		case TK_OR:
			return binopImpl(node, { {TBool, TBool, TBool, &CodeGen::binopOrBool} });
		default: 
			throw ICEException("Binop not implemented");
		}
	}

	LLVMVal visit(std::shared_ptr<SequenceExp> node) {
		LLVMVal x;
		for(auto n: node->sequence) 
			x=visitNode(n);
		return x;
	}
	
	llvm::Function * translate(NodePtr ast) override {
		size_t id=uid++;
		llvm::FunctionType * ft = FunctionType::get(llvm::Type::getVoidTy(getGlobalContext()), {}, false);
		std::stringstream ss;
		ss << "INNER_" << id;
		Function * function = Function::Create(ft, Function::ExternalLinkage, ss.str(), module);
		BasicBlock * block = BasicBlock::Create(getGlobalContext(), "entry", function);
		builder.SetInsertPoint(block);
		visitNode(ast);
		builder.CreateRetVoid();
		function->dump();
		llvm::verifyFunction(*function);
		fpm.run(*function);
		return function;
	}
};

} //nameless namespace

std::shared_ptr<LLVMCodeGen> llvmCodeGen(
	std::shared_ptr<Error> error, std::shared_ptr<Code> code, llvm::Module * module) {
	return std::make_shared<CodeGen>(error, code, module);
}
