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
#include "code.hh"
#include "error.hh"
#include "parser.hh"
#include "lexer.hh"
#include "visitor.hh"
#include "llvmCodeGen.hh"
#include <readline/history.h>
#include <readline/readline.h>
#include <iostream>

#include <dlfcn.h>
#include <unistd.h>

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/JIT.h>

std::shared_ptr<Visitor<void> > charRanges(); //Declared in charRanges.cc
std::shared_ptr<Visitor<void> > firstParse(std::shared_ptr<Error> error, std::shared_ptr<Code> code); 

int main(int argc, char ** argv) {
	llvm::InitializeNativeTarget();

	std::shared_ptr<Code> c = std::make_shared<Code>("", "Interpreted");
	std::shared_ptr<Error> e = terminalError(c);
	std::shared_ptr<Lexer> l = std::make_shared<Lexer>(c);
	std::shared_ptr<Parser> p = parser(l, e, true);
	std::shared_ptr<Visitor<void> > cr = charRanges();
	std::shared_ptr<Visitor<void> > fp = firstParse(e, c);
	llvm::Module * module = new llvm::Module("my cool jit", llvm::getGlobalContext());
	std::shared_ptr<LLVMCodeGen> cg=llvmCodeGen(e, c, module);

	std::string incomplete;
	std::string theCode;

	std::string ErrStr;
	llvm::ExecutionEngine * engine = llvm::EngineBuilder(module).setErrorStr(&ErrStr).create();
	if (!engine) {
		std::cerr << "Could not create engine: " << ErrStr << std::endl;
	}
	
	void * dll = dlopen((std::string(get_current_dir_name())+"/libstdlib.so").c_str(), RTLD_NOW | RTLD_LOCAL);
	for (auto & p: cg->stdlib) {
		void * func = dlsym(dll, p.first.c_str());
		if (!func) 
			std::cerr << "Unable to load symbol " << p.first << " from stdlib" << std::endl;
		engine->addGlobalMapping(p.second, func);
	}

	while (true) {
		std::string line;
		char * rl = readline(incomplete.empty()?">>>> ":".... ");
		if (!rl) break;
		if (!rl[0]) {
			free(rl); 
			continue;
		}
		add_history(rl);
		line = rl;
		free(rl); 
		
		l->index = theCode.size();

		c->set(theCode + incomplete + line);
		try {
			size_t errorsPrior = e->count();
			NodePtr r=p->parse();
			if (r->nodeType == NodeType::InvalidExp) continue;
			
			Token t1(TK_PRINT, 0, 0);
			std::shared_ptr<BuiltInExp> t = std::make_shared<BuiltInExp>(t1, Token(TK_RPAREN, 0, 0)); 
			t->args.push_back(r);
			cr->run(t);
			fp->run(t);
			
			if (e->count() != errorsPrior) continue;

			theCode = c->code+"\n";
			incomplete = "";

			llvm::Function * f = cg->translate(t);
			f->dump();
			void * fp = engine->getPointerToFunction(f);
			void (*FP)() = (void (*)())fp;
			FP();
		} catch (IncompleteInputException) {
			incomplete = line + "\n";
		}
	}
	printf("\n");
}
