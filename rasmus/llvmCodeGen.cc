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
#include "AST.hh"
#include "code.hh"
#include "error.hh"
#include "visitor.hh"
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

llvm::Type * structType(std::string name, 
						std::initializer_list<llvm::Type *> types) {
	return StructType::create(getGlobalContext(), ArrayRef<llvm::Type * >(types.begin(), types.end()), name);
}

llvm::FunctionType * functionType(llvm::Type * ret, std::initializer_list<llvm::Type *> args) {
	return FunctionType::get(ret, ArrayRef<llvm::Type * >(args.begin(), args.end()), false);
}


llvm::Type * anyRetType = structType("AnyRet", {int8Type, int64Type});
llvm::Type * funcBase = structType("FuncBase", {pointerType(voidType), int16Type});


// def llvmConstant(t, v):
//     if t == TBool: return Constant.int(llvmType(t), int(v))
//     elif t == TInt: return Constant.int(llvmType(t), v)
//     elif t == TText: return Constant.stringz(v)
//     raise ICEException("Unhandled type %s"%str(t))

llvm::Type * llvmType(::Type t) {
	switch (t) {
	case TBool: return int8Type;
	case TInt: return int64Type;
	case TFunc: return pointerType(funcBase);
	default:
		throw ICEException(std::string("llvmType - Unhandled type ") + typeName(t));
	}
}

llvm::Value * typeRepr(::Type t) {
	switch (t) {
	case TBool:
		return llvm::ConstantInt::get(int8Type, 0);
	case TInt:
		return llvm::ConstantInt::get(int8Type, 1);
	case TFunc:
		return llvm::ConstantInt::get(int8Type, 2);
	case TText:
		return llvm::ConstantInt::get(int8Type, 3);
	default:
		throw ICEException(std::string("typeRepr - Unhandled type ") + typeName(t));
	}
}
	
llvm::Value * getUndef(::Type t) {
// def genUndef(t):
//     if t == TBool: return Constant.int(Type.int(8), 255)
//     elif t == TInt: return Constant.int(Type.int(64), 2**63-1)
//     elif t == TAny: return Constant.int(Type.int(64), 2**63-1)
//     elif t == TFunc: return Constant.null(Type.pointer(funcBase))
//     raise ICEException("Unhandled type %s"%str(t))
	//TODO
	return NULL;
}


llvm::Value * intp(uint32_t value) {
	return llvm::ConstantInt::get(int32Type, value);
}

llvm::Type * funcTyp(uint16_t argc) {
	std::vector<llvm::Type *> t;
	t.push_back(pointerType(funcBase));
	t.push_back(pointerType(anyRetType));
	for (size_t i=0; i < argc; ++i) {
		t.push_back(int8Type);
		t.push_back(int64Type);
	}
	return llvm::FunctionType::get(voidType, t, false);
}


class CodeGen: public LLVMCodeGen, public VisitorCRTP<CodeGen, LLVMVal> {
public:
	IRBuilder<> builder;
	size_t uid;
	Module * module;
	Function * function;
	std::shared_ptr<Error> error;
	std::shared_ptr<Code> code;


	BasicBlock * newBlock() {
		std::stringstream ss;
		ss << "b" << uid++;
		return BasicBlock::Create(getGlobalContext(), ss.str(), function);
	}
	
	LLVMVal cast(LLVMVal value, ::Type tfrom, ::Type tto, NodePtr node) {
		if (tfrom == tto) return value;
		if (tto == TAny) {
			std::cout << "I WAS HERE" << std::endl;
			switch(tfrom) {
			case TInt:
				return LLVMVal(value.value, typeRepr(tfrom));
			case TBool:
				return LLVMVal(builder.CreateZExt(value.value, int64Type), typeRepr(tfrom));
			case TText:
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
								intp(node->charRange.lo), intp(node->charRange.hi),
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

		std::vector< std::pair<std::string, FunctionType *> > fs =
		{
			{"rm_print", functionType(voidType, {int8Type, int64Type})},
			{"rm_concatText", functionType(voidPtrType, {voidPtrType, voidPtrType})},
			{"rm_getConstText", functionType(voidPtrType, {pointerType(int8Type)})}
		};

		for(auto p: fs) {
			stdlib[p.first] = Function::Create(p.second, Function::ExternalLinkage, p.first, module);
		}

		fpm.doInitialization();
	}
            
//     def __init__(self, err, code):
//         self.code = code
//         self.err = err
//         self.module = Module.new("Monkey")
//         #self.module.link_in("stdlib")
//         self.uid = 0
//         self.passMgr = FunctionPassManager.new(self.module)
        

//         substringSearchType = Type.function(Type.int(8), [Type.pointer(Type.void()), Type.pointer(Type.void())])

//         typeErrType = Type.function(Type.void(),
//                                     [Type.int(32), Type.int(32), Type.int(8), Type.int(8)])
//         argCntErrType = Type.function(Type.void(), 
//                                         [Type.int(32), Type.int(32), Type.int(16), Type.int(16)])

//         self.innerType = Type.function(Type.void(), [])
//         interactiveWrapperType = Type.function(Type.int(8), 
//                                                [Type.pointer(Type.int(8)), Type.pointer(self.innerType)])

//         fs = [
//             ('rm_substringSearch', substringSearchType),
//             ('rm_print',printType), 
//             ('rm_emitTypeError', typeErrType), 
//             ('rm_emitArgCntError', argCntErrType),
//             ('rm_interactiveWrapper', interactiveWrapperType)
//         ]
             
//         # Do simple "peephole" optimizations and bit-twiddling optzns.
//         self.passMgr.add(PASS_INSTCOMBINE)
//         # Reassociate expressions.
//         self.passMgr.add(PASS_REASSOCIATE)
//         # Eliminate Common SubExpressions.
//         self.passMgr.add(PASS_GVN)

//         self.passMgr.add(PASS_DCE)
//         # Simplify the control flow graph (deleting unreachable blocks, etc).
//         #self.passMgr.add(PASS_CFG_SIMPLIFICATION)
//         self.passMgr.initialize()

	LLVMVal visit(std::shared_ptr<VariableExp> node) {
		NodePtr store(node->store);
		return store->llvmVal;
	}
	
	LLVMVal visit(std::shared_ptr<AssignmentExp> node) {
		return node->llvmVal = visitNode(node->valueExp);
	}

	LLVMVal visit(std::shared_ptr<IfExp> node) {
		// TODO I DO NOT WORK FOR ANY TYPE do to value beeing a pair
		bool done=false;
		std::vector<std::pair<LLVMVal, LLVMVal> > hats;
		LLVMVal val;
		for (auto choice: node->choices) {
			// Evaluate condition and cast value to bool
			LLVMVal cond = castVisit(choice->condition, TBool);
			//if cond == llvmConstant(TBool, False) or done:
			//self.err.reportWarning("Branch never taken", choice.arrowToken)
			//			   continue
            
			// if cond == llvmConstant(TBool, True):
// done = True

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
//         # Create function type
//         funct_type = funcType(len(node.args))

//         # Create function object type
//         captureType = Type.struct( [
//             Type.pointer(funct_type), #Function ptr
//             Type.int(16), # Number of arguments
//         ] + [llvmType(cap.type) for cap in node.captures])

//         # Cache current state
//         f = self.function
//         b = self.block
//         bb = self.builder.basic_block
        
//         # Name new function and block
//         name = "f%d"%self.uid
//         self.uid += 1
//         self.function = Function.new(self.module, funct_type, name)
//         self.block = self.function.append_basic_block('entry')
//         self.builder.position_at_end(self.block)

//         func = self.function

//         # Setup args
//         self.function.args[0].name = "self"
//         self.function.args[1].name = "ret"
//         selfv = self.builder.bitcast(self.function.args[0], Type.pointer(captureType))
//         ret = self.function.args[1]
//         for i in range(len(node.args)):
//             arg = node.args[i]
//             arg.value = self.cast(
//                 (self.function.args[i*2+2], self.function.args[i*2+3]),
//                 TAny, arg.type, arg)
//             self.function.args[i*2+2].name = "t_"+arg.name
//             self.function.args[i*2+3].name = "v_"+arg.name

//         for i in range(len(node.captures)):
//             cap = node.captures[i]
//             cap.value = self.builder.load(self.builder.gep(selfv, [intp(0), intp(2+i)]))
                    
//         # Build function code
//         x = self.visit(node.body)
//         t, v = self.cast(x, node.body.type, TAny, node.body)
//         print node.body.type ,x, t,v
//         self.builder.store(t, self.builder.gep(ret, [intp(0), intp(0)]))
//         self.builder.store(v, self.builder.gep(ret, [intp(0), intp(1)]))
//         self.builder.ret_void()
//         print self.function
//         self.function.verify()
//         self.passMgr.run(self.function)

//         # Revert state
//         self.function = f
//         self.block = b
//         self.builder.position_at_end(bb)
        
//         # Allocate function object
//         p = self.builder.malloc(captureType)

//         # Store function ptr
//         self.builder.store(func, 
//                            self.builder.gep(p, [intp(0), intp(0)]))
//         # Store number of arguments
//         self.builder.store(Constant.int(Type.int(16), len(node.args)), 
//                            self.builder.gep(p, [intp(0), intp(1)]))
//         # Store captures
//         for i in range(len(node.captures)):
//             cap = node.captures[i]
//             self.builder.store(cap.store.value, self.builder.gep(p, [intp(0), intp(2+i)]))
//         return p
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
			return ConstantInt::get(int8Type, 1);
		}
		break;
		default:
			throw ICEException("BuildIn not implemented");
		}
	}

     LLVMVal visit(std::shared_ptr<ConstantExp> node) {
		 switch (node->type) {
		 case TInt:
			 return LLVMVal(ConstantInt::get(int64Type, node->int_value));
		 case TBool:
			 return LLVMVal(ConstantInt::get(int8Type, node->bool_value?1:0));
		 case TText:
		 {
			 std::cout << "I WAS HERE" << std::endl;
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
//         if node.token.id == TK_NOT:
           
//             v = self.visit(node.exp)
//             v2 = llvmConstant(TBool, True)
		//             v3 = self.builder.xor(v, v2)
//             return self.cast(v3, node.type, TBool, node)

//         elif node.token.id == TK_MINUS:
//             return self.cast(self.builder.neg(self.visit(node.exp)), node.type, TInt, node)
//         raise ICEException("Unary")
	}

	LLVMVal visit(std::shared_ptr<RelExp> node) {
        throw ICEException("Rel");
	}

	LLVMVal visit(std::shared_ptr<LenExp> node) {
		throw ICEException("Len");
	}

    LLVMVal visit(std::shared_ptr<FuncInvocationExp> node) {
//         ft = funcType(len(node.args))
        
//         capture=self.builder.bitcast(self.cast(self.visit(node.funcExp), node.funcExp.type, 
//                                                TFunc, node.funcExp), 
//                                      Type.pointer(funcBase))
//         rv = self.builder.alloca(anyRetType)

//         fblock = self.function.append_basic_block('check_fail_%d'%self.uid)
//         nblock = self.function.append_basic_block('check_succ_%d'%(self.uid+1))
//         self.uid += 2
//         argc = self.builder.load(self.builder.gep(capture, [intp(0), intp(1)]))
//         margc = Constant.int(Type.int(16), len(node.args))
//         self.builder.cbranch(self.builder.icmp(ICMP_EQ, argc, margc),
//                              nblock, fblock)
        
//         self.builder.position_at_end(fblock)
//         self.builder.call(self.stdlib['rm_emitArgCntError'],
//                           [Constant.int(Type.int(32), node.charRange.lo),
//                            Constant.int(Type.int(32), node.charRange.hi),
//                            margc,
//                            argc])
//         self.builder.unreachable()
//         self.block = nblock
//         self.builder.position_at_end(self.block)

//         args = [capture, rv]

//         for arg in node.args:
//             argv = self.visit(arg)
//             if arg.type == TAny:
//                 args.append(argv[0])
//                 args.append(argv[1])
//             else:
//                 args.append(typeRepr(arg.type))
//                 args.append(self.builder.bitcast(argv, Type.int(64)))
//         voidfp = self.builder.load(self.builder.gep(capture, [intp(0), intp(0)]))
//         fp = self.builder.bitcast(voidfp, Type.pointer(ft))
//         rv = self.builder.call(fp, args)
        
//         return (self.builder.load(self.builder.gep(rv, [intp(0), intp(0)])),
//                 self.builder.load(self.builder.gep(rv, [intp(0), intp(1)])))
	}

	LLVMVal visit(std::shared_ptr<SubstringExp> node) {
		LLVMVal a = castVisit(node->fromExp, TText);
		LLVMVal b = castVisit(node->toExp, TText);

		//return cast(builder.createCall(self.stdlib['rm_substringSearch'], [a, b]),
		//				 TBool,
		//				 node.type,
		//				 node)
	}

	LLVMVal visitConcatExp(std::shared_ptr<BinaryOpExp> node) {
		LLVMVal a = castVisit(node->lhs, TText);
		LLVMVal b = castVisit(node->rhs, TText);
		return cast(builder.CreateCall2(stdlib["rm_concatText"], a.value, b.value),
						 TText, node->type, node);
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

	LLVMVal visit(std::shared_ptr<Exp> node) {
		return visitNode(node->exp);
	}

	LLVMVal visit(std::shared_ptr<Choice> node) {throw ICEException("Choice");}
	LLVMVal visit(std::shared_ptr<FuncCaptureValue> node) {throw ICEException("FuncCaptureValue");}
	LLVMVal visit(std::shared_ptr<FuncArg> node) {throw ICEException("FuncArg");}
	LLVMVal visit(std::shared_ptr<TupItem> node) {throw ICEException("TupItem");}
	LLVMVal visit(std::shared_ptr<InvalidExp> node) {throw ICEException("InvalidExp");}
	LLVMVal visit(std::shared_ptr<Val> node) {throw ICEException("Val");}
	LLVMVal visit(std::shared_ptr<RenameItem> node) {throw ICEException("RenameItem");}


	LLVMVal visit(std::shared_ptr<BinaryOpExp> node) {
		if (node->opToken.id == TK_CONCAT)
			return visitConcatExp(node);
		
//         elif node.token.id == TK_TILDE:
//             return self.visitSubstringExp(node)
		LLVMVal a = castVisit(node->lhs, TInt);
		LLVMVal b = castVisit(node->rhs, TInt);
		switch (node->opToken.id) {
		case TK_PLUS:
			return cast(builder.CreateAdd(a.value, b.value), TInt, node->type, node);
		}
//         if node.token.id == TK_PLUS:
//             return self.cast(self.builder.add(a, b), TInt, node.type, node)
//         elif node.token.id == TK_MUL:
//             return self.builder.mul(a,b)
//         elif node.token.id == TK_MINUS:
//             return self.cast(self.builder.sub(a, b), TInt, node.type, node)
//         elif node.token.id == TK_DIV:
//             return self.cast(self.builder.sdiv(a, b), TInt, node.type, node)
//         elif node.token.id == TK_MOD:
//             return self.cast(self.builder.srem(a, b), TInt, node.type, node)
//         elif node.token.id == TK_LESS:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_SLT, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_SLT, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_LESSEQUAL:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_ULE, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_ULE, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_GREATER:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_SGT, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_SGT, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_GREATEREQUAL:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_UGE, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_UGE, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_DIFFERENT:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_NE, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_NE, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_EQUAL:
//             if node.lhs.type == TInt and node.rhs.type == TInt:
//                 return self.builder.icmp(ICMP_EQ, a, b)
//             elif node.lhs.type == TBool and node.rhs.type == TBool:
//                 return self.builder.icmp(ICMP_EQ, a, b)
//             else:
//                 raise ICEException("Only Integer comparison implemented")
//         elif node.token.id == TK_TILDE:
//             # text comparison: true if a is a substring of b
            
//             raise ICEExpcetion("Substring test not implemented")
//         elif node.token.id == TK_AND:
//             return self.cast(self.builder.and_(a, b), TBool, node.type, node)
//         elif node.token.id == TK_OR:
//             return self.cast(self.builder.or_(a, b), TBool, node.type, node)
//         elif node.token.id == TK_CONCAT:
//             raise ICEException("Binary operation: Concatenation not implemented")
//         else:
//             raise ICEException("Binop not implemented")
	}

	LLVMVal visit(std::shared_ptr<SequenceExp> node) {
		LLVMVal x;
		for(auto n: node->sequence) 
			x=visitNode(n);
		return x;
	}

	/*BasicBlock * bb;*/
	
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
		
		llvm::verifyFunction(*function);
		function->dump();
		fpm.run(*function);
		return function;
	}
};



} //nameless namespace


std::shared_ptr<LLVMCodeGen> llvmCodeGen(
	std::shared_ptr<Error> error, std::shared_ptr<Code> code, llvm::Module * module) {
	return std::make_shared<CodeGen>(error, code, module);
}
