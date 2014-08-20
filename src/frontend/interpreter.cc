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

#include <frontend/interpreter.hh>

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
#include <llvm/ExecutionEngine/MCJIT.h>
#ifndef _WIN32
#include <unistd.h>
#include <dlfcn.h>
#endif //_WIN32
#include <stdlib/callback.hh>
#include <stdlib/text.hh>
#include <stdlib/relation.hh>
#include <frontend/charRanges.hh>
#include <frontend/firstParse.hh>
#include <frontend/astPrinter.hh>
#include <frontend/tokenizer.hh>
#include <sstream>
#include <stdlib/lib.h>

using lexer::TokenType;

namespace {
using namespace rasmus::frontend;

struct ErrException: public std::exception {};


class StdlibCallback: public rasmus::stdlib::Callback {
public:
	std::shared_ptr<rasmus::frontend::Callback> cb;
	std::shared_ptr<Code> code;

	void environmentChanged(const char * name) override {
		cb->environmentChanged(name);
	}

	StdlibCallback(std::shared_ptr<rasmus::frontend::Callback> cb,
				   std::shared_ptr<Code> code): cb(cb), code(code) {}

	void printInt(int64_t v) override {
		std::stringstream ss;
		rasmus::stdlib::printIntToStream(v, ss);
		cb->print(TInt, ss.str());
	}

	void printBool(int8_t v) override {
		std::stringstream ss;
		rasmus::stdlib::printBoolToStream(v, ss);
		cb->print(TBool, ss.str());
	}
	
	void printText(rm_object * o) override {
		std::stringstream ss;
		rasmus::stdlib::printTextToStream(static_cast<rasmus::stdlib::TextBase *>(o), ss);
		cb->print(TText, ss.str());
	}

	void printFunc(rm_object * o) override {
		std::stringstream ss;
		ss << "func(" << o << ")";
		cb->print(TFunc, ss.str());
	}

	void printTup(rm_object * o) override {
		std::stringstream ss;
		rasmus::stdlib::printTupleToStream(o, ss);
		cb->print(TTup, ss.str());
	}

	void printRel(rm_object * o) override {
		std::stringstream ss;
		rasmus::stdlib::printRelationToStream(o, ss);
		cb->print(TRel, ss.str());
	}
	
	void reportError(size_t start, size_t end, std::string text) override {
	
		if (start < end)
			cb->report(MsgType::error, code,
					   text,
					   Token(),
					   {{start, end}});
		else
			cb->report(MsgType::error, text);
		throw ErrException();
	}
	
	void reportAbort() override {
		cb->report(MsgType::error, "Aborted");
		throw ErrException();
	}
	

	void reportMessage(std::string text) override {
		cb->report(MsgType::info, text);
	}

	void saveRelation(rm_object * o, const char * name) override {
		cb->saveRelation(o, name);
	}

	rm_object * loadRelation(const char * name) override {
		return cb->loadRelation(name);
	}
};


class InterperterImpl: public Interperter {
public:
	std::shared_ptr<Code> code;
	std::shared_ptr<Error> error;
	std::shared_ptr<Lexer> lexer;
	std::shared_ptr<Parser> parser;
	std::shared_ptr<CharRanges> charRanges;
	std::shared_ptr<FirstParse> firstParse;
	std::shared_ptr<AstPrinter> astPrinter;
	llvm::Module * module;
	std::shared_ptr<LLVMCodeGen> codeGen;
	llvm::ExecutionEngine * engine;
	std::string incomplete;
	std::string theCode;
	std::shared_ptr<rasmus::frontend::Callback> callback;
	int options;
	InterperterImpl(std::shared_ptr<rasmus::frontend::Callback> callback): callback(callback) {}

	void setup(int options, std::string name) override {
		rasmus::stdlib::objectCount = 0;
		rasmus::stdlib::debugAllocations = options & DumpAllocations;
		llvm::InitializeNativeTarget();
		llvm::InitializeNativeTargetAsmPrinter();
		llvm::InitializeNativeTargetAsmParser();

		code = std::make_shared<Code>("", name);
		rasmus::stdlib::callback = std::make_shared<StdlibCallback>(callback, code);
		error = makeCallbackError(code, callback);
		lexer = std::make_shared<Lexer>(code);
		parser = makeParser(lexer, error, true);
		charRanges = makeCharRanges();
		firstParse = makeFirstParse(error, code, this->callback);
		astPrinter = makeAstPrinter(code);
		#ifndef _WIN32
		// TODO: if this line is removed ./rm does not link, WTF??
		if (options & 12345) dlopen("monkey.so", 0);
		#endif //_WIN32

		this->options = options;
	}

	void cancel() {
		if (incomplete.size() > 0) {
			incomplete = "";
		}
	}

	void runContent(const std::string & name, const std::string & content) {

	}

	bool runLine(const std::string & line) override {
		lexer->index = theCode.size();
		code->set(theCode + incomplete + line);
		lexer->tknizer = lexer::Tokenizer(code->code.data()+lexer->index);
		try {
			size_t errorsPrior = error->count();
			NodePtr r=parser->parse();
			if (r->nodeType == NodeType::InvalidExp) return false;
			
			std::shared_ptr<BuiltInExp> t = std::make_shared<BuiltInExp>(Token(TokenType::TK_PRINT, "print"), Token(TokenType::TK_RPAREN, ")")); 
			t->args.push_back(r);
			charRanges->run(t);
			firstParse->run(t);
			if (options & DumpAST) astPrinter->run(t, std::cout);

			if (error->count() != errorsPrior) return false;
			
			theCode = code->code+"\n";
			incomplete = "";

			module = new llvm::Module("my cool jit", llvm::getGlobalContext());
			#ifdef _WIN32
			//ase x86:     return "i386";
		  //ase x86_64:  return "x86_64";
			module->setTargetTriple("i386-pc-win32-elf");
			#endif //_WIN32

			codeGen = makeLlvmCodeGen(error, code, module,
									  options & DumpRawFunction,
									  options & DumpOptFunction);

			llvm::Function * f = codeGen->translate(t);
			
			std::string ErrStr;

			engine = llvm::EngineBuilder(module).setErrorStr(&ErrStr).setUseMCJIT(true).create();
			if (!engine) {
				callback->report(MsgType::error, std::string("Could not create engine: ")+ ErrStr);
				return false;
			}
		
			engine->finalizeObject();
			void * fp = engine->getPointerToFunction(f);
			void (*FP)() = (void (*)())fp;
			FP();
		} catch (ErrException) {
			return false; //Error in code execution
		} catch (ICEException e) {
			if (e.mainToken || e.ranges.size())
				callback->report(MsgType::error, code, e.what(), e.mainToken, e.ranges);
			else
				callback->report(MsgType::error, e.what());
			return false;
		} catch (IncompleteInputException) {
			incomplete = incomplete+line + "\n";
		}
		return true;
	}

	bool complete() const override {
		return incomplete.empty();
	}

	void destroy() override {

	}

	size_t objectCount() const override {
		return rasmus::stdlib::objectCount;
	}

	virtual void freeGlobals() {
		rm_clearGlobals();
	}
};

} //nameless namespace

namespace rasmus {
namespace frontend {

std::shared_ptr<Interperter> makeInterperter(std::shared_ptr<Callback> callback) {
	return std::make_shared<InterperterImpl>(callback);
}

} //namespace frontend
} //namespace rasmus
