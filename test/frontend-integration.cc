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
#include "common.hh"
#include <frontend/interperter.hh>
using namespace rasmus;
using namespace rasmus::frontend;

class TestCallback: public Callback {
public:
	TestCallback(): errors(0) {}

	void report(MsgType type, 
				std::shared_ptr<Code> code,
				std::string message,
				Token mainToken,
				std::initializer_list<CharRange> ranges) override {
		report(type, message);
	}
	
	void report(MsgType type, std::string message) override {
		switch (type) {
		case MsgType::error: log_error() << message << std::endl; ++errors; break;
		case MsgType::warning: log_warning() << message << std::endl; break;
		case MsgType::info: log_info() << message << std::endl; break;
		}
	}
	Type printType;
	std::string printText;
	size_t errors;
	
	void print(Type type, std::string repr) override {
		printType = type;
		printText = repr;
	}
};

bool it(std::string txt, const char * exp, int errors=0) {
	std::shared_ptr<TestCallback> cb = std::make_shared<TestCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interperter=rasmus::frontend::makeInterperter(cb);
	interperter->setup();
	size_t start=0;
	while (start < txt.size()) {
		size_t end=txt.find_first_of("\n");
		if (end == std::string::npos) end=txt.size();
		if (!interperter->runLine(txt.substr(start, end-start))) return false;
		start = end+1;
	}
	if (cb->printText != exp) {
		log_error() << "Got " << cb->printText << " expected " << exp << std::endl;
		return false;
	}
	if (cb->errors != errors) 
		return false;
	return true;
}

void base(rasmus::teststream & ts) {
    ts << "true" << result(it("true", "true"));
    ts << "false" << result(it("false", "false"));
    ts << "int" << result(it("1", "1"));
    ts << "addition" << result(it("1+2", "3"));
    ts << "string" << result(it("\"abe\"", "abe"));
    ts << "concat" << result(it("\"abe\"++\"bea\"", "abebea"));
    ts << "block" << result(it("(+val a=4 in a +)", "4"));
    ts << "function" << result(it("(func()->(Int)5 end)()", "5"));
    ts << "capture" << result(it("(+val a=\"abe\" in func()->(Text)a end+)()", "abe"));
}


int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
		.multi_test(base, "base");
}
