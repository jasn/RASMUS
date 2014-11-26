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
#include <frontend/error.hh>

namespace {
using namespace rasmus::frontend;

class CallbackError: public Error {
public:
	size_t numberOfErrors;
	std::shared_ptr<Code> code;
	std::shared_ptr<Callback> callback;

	CallbackError(std::shared_ptr<Code> code,
				  std::shared_ptr<Callback> callback): 
		numberOfErrors(0), code(code), callback(callback) {}
	
	virtual void reportWarning(std::string message,
							   Token mainToken,
							   std::initializer_list<CharRange> ranges) override {
		callback->report(MsgType::warning, code, message, mainToken, ranges, "");
	}
	
	virtual void reportError(std::string message,
							 Token mainToken,
							 std::initializer_list<CharRange> ranges, 
							 std::string additional) override {
		++numberOfErrors;
		callback->report(MsgType::error, code, message, mainToken, ranges, additional);
	}

	virtual size_t count() const {return numberOfErrors;}
};

class CountError: public Error {
public:
	size_t numberOfErrors;

	CountError(): numberOfErrors(0) {}
	
	virtual void reportWarning(std::string,
							   Token,
							   std::initializer_list<CharRange>) override {
	}
	
	virtual void reportError(std::string,
							 Token,
							 std::initializer_list<CharRange>, std::string) override {
		++numberOfErrors;
	}

	virtual size_t count() const {return numberOfErrors;}
};

	
} //anonymous namespace

namespace rasmus {
namespace frontend {

std::shared_ptr<Error> makeCallbackError(std::shared_ptr<Code> code,
									 std::shared_ptr<Callback> callback) {
	return std::make_shared<CallbackError>(code, callback);
}

std::shared_ptr<Error> makeCountError() {
	return std::make_shared<CountError>();
}

} //namespace frontend
} //namespace rasmus
    
