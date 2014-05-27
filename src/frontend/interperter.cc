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

#include <frontend/interperter.hh>

#include <frontend/code.hh>
#include <frontend/error.hh>
#include <frontend/parser.hh>
#include <frontend/lexer.hh>
#include <frontend/visitor.hh>
#include <frontend/llvmCodeGen.hh>
#include <iostream>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/JIT.h>

#include <unistd.h>
#include <dlfcn.h>

std::shared_ptr<Visitor<void> > charRanges(); //Declared in charRanges.cc
std::shared_ptr<Visitor<void> > firstParse(std::shared_ptr<Error> error, std::shared_ptr<Code> code); 


namespace {

class InterperterImpl: public Interperter {
public:
	std::shared_ptr<Code> code;
	std::shared_ptr<Error> error;
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Parser> p;
	std::shared_ptr<Visitor<void> > cr;
	std::shared_ptr<Visitor<void> > fp;
	llvm::Module * module;
	std::shared_ptr<LLVMCodeGen> codeGen;
	llvm::ExecutionEngine * engine;

	std::string incomplete;
	std::string theCode;
	
	void setup() override {
		llvm::InitializeNativeTarget();
		code = std::make_shared<Code>("", "Interpreted");
		error = terminalError(code);
		lexer = std::make_shared<Lexer>(code);
		p = parser(lexer, error, true);
		cr = charRanges();
		fp = firstParse(error, code);
		module = new llvm::Module("my cool jit", llvm::getGlobalContext());
		codeGen = llvmCodeGen(error, code, module);
		std::string ErrStr;
		engine = llvm::EngineBuilder(module).setErrorStr(&ErrStr).create();
		if (!engine) {
			std::cerr << "Could not create engine: " << ErrStr << std::endl;
		}
	
		void * dll = dlopen((std::string(get_current_dir_name())+"/libstdlib.so").c_str(), RTLD_NOW | RTLD_LOCAL);
		for (auto & p: codeGen->stdlib) {
			void * func = dlsym(dll, p.first.c_str());
			if (!func) 
				std::cerr << "Unable to load symbol " << p.first << " from stdlib" << std::endl;
			engine->addGlobalMapping(p.second, func);
		}
	}

	bool runLine(const std::string & line) override {
		lexer->index = theCode.size();
		code->set(theCode + incomplete + line);
		try {
			size_t errorsPrior = error->count();
			NodePtr r=p->parse();
			if (r->nodeType == NodeType::InvalidExp) return false;
			
			Token t1(TK_PRINT, 0, 0);
			std::shared_ptr<BuiltInExp> t = std::make_shared<BuiltInExp>(t1, Token(TK_RPAREN, 0, 0)); 
			t->args.push_back(r);
			cr->run(t);
			fp->run(t);
			
			if (error->count() != errorsPrior) return false;
			
			theCode = code->code+"\n";
			incomplete = "";

			llvm::Function * f = codeGen->translate(t);
			f->dump();
			void * fp = engine->getPointerToFunction(f);
			void (*FP)() = (void (*)())fp;
			FP();
		} catch (IncompleteInputException) {
			incomplete = line + "\n";
		}
		return true;
	}

	bool complete() const override {
		return incomplete.empty();
	}

	void destroy() override {

	}

	size_t objectCount() const override {
		return 0;
	}
};

} //nameless namespace

std::shared_ptr<Interperter> makeInterperter(std::shared_ptr<Callback> callBack) {
	return std::make_shared<InterperterImpl>();
}

