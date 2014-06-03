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
#include <stdlib/callback.hh>
#include <stdlib/relation.hh>
using namespace rasmus;
using namespace rasmus::frontend;

class TestCallback: public Callback {
public:
	TestCallback(): errors(0) {}

	void report(MsgType type, 
				std::shared_ptr<Code>,
				std::string message,
				Token,
				std::initializer_list<CharRange>) override {
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
	std::map<std::string, std::string> relations;
	
	void print(Type type, std::string repr) override {
		printType = type;
		printText = repr;
	}

	void saveRelation(rm_object * o, const char * name) override {
		std::stringstream ss;
		rasmus::stdlib::saveRelationToStream(o, ss);
		relations[name] = ss.str();
	}

	rm_object * loadRelation(const char * name) override {
		auto x=relations.find(name);
		if (x == relations.end())
			throw rasmus::stdlib::IOException("Does not exist");
		std::stringstream ss(x->second);
		return rasmus::stdlib::loadRelationFromStream(ss);
	}
};

bool it(std::string txt, const char * exp, size_t errors=0) {
	std::shared_ptr<TestCallback> cb = std::make_shared<TestCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interperter=rasmus::frontend::makeInterperter(cb);
	interperter->setup();
	size_t start=0;
	while (start < txt.size()) {
		size_t end=txt.find_first_of("\n", start);
		if (end == std::string::npos) end=txt.size();
		if (!interperter->runLine(txt.substr(start, end-start)) && (errors == 0)) return false;
		start = end+1;
	}
	if (cb->printText != exp) {
		log_error() << "\"" << txt << "\": gave " << cb->printText << " expected " << exp << std::endl;
		return false;
	}
	if (cb->errors != errors) 
		return false;
	interperter->freeGlobals();
	size_t oc=interperter->objectCount();
	if (oc != 0) {
		log_error() << oc << " objects where not freed" << std::endl;
		return false;
	}
	return true;
}

void base(rasmus::teststream & ts) {
    ts << "true" << result(it("true", "true"));
    ts << "false" << result(it("false", "false"));
    ts << "string" << result(it("\"abe\"", "abe"));
    ts << "concat" << result(it("\"abe\"++\"bea\"", "abebea"));
    ts << "block" << result(it("(+val a=4 in a +)", "4"));
    ts << "function" << result(it("(func()->(Int)5 end)()", "5"));
    ts << "capture" << result(it("(+val a=\"abe\" in func()->(Text)a end+)()", "abe"));
	ts << "globalText" << result(it("y:=\"abe\"\ny", "abe"));
	ts << "globalFunc" << result(it("y:=func()->(Int) 4 end\ny()", "4"));
	ts << "argCntErr" << result(it("(+ val x = func ()->(Int)1 end in x(2) +)", "", 1));
	ts << "argTypeErr" << result(it("(+ val x = func (t:Text)->(Int)t end in x(2) +)", "", 1));
	ts << "Crash1" << result(it("(+ val x := 2 in x + 5 +)", "", 1));
}

void integer(rasmus::teststream & ts) {
    ts << "int" << result(it("1", "1"));
	ts << "var" << result(it("x:=3\nx", "3"));
    ts << "add" << result(it("2+3", "5"));
    ts << "mult" << result(it("2*3", "6"));
    ts << "div" << result(it("5/2", "2"));
	ts << "mod" << result(it("5 mod 2", "1"));
	ts << "prec1" << result(it("2+3*4", "14"));
	ts << "prec2" << result(it("(2+3)*4", "20"));
	ts << "prec3" << result(it("2+3-4+1", "2"));
	ts << "prec4" << result(it("4*2+3", "11"));
    ts << "?" << result(it("?-Int", "?-Int"));
    ts << "?1" << result(it("1+?-Int", "?-Int"));
    ts << "?2" << result(it("?-Int*2", "?-Int"));
	ts << "?3" << result(it("5/0", "?-Int"));
}
	
void bools(rasmus::teststream & ts) {
    ts << "false" << result(it("false", "false"));
	ts << "true" << result(it("true", "true"));
	ts << "?-Bool" << result(it("?-Bool", "?-Bool"));
	ts << "and 1" << result(it("true and false", "false"));
	ts << "and 2" << result(it("true and true", "true"));
	ts << "and 3" << result(it("true and ?-Bool", "?-Bool"));
	ts << "and 4" << result(it("false and ?-Bool", "false"));
	ts << "or 1" << result(it("true or false", "true"));
	ts << "or 2" << result(it("false or false", "false"));
	ts << "or 3" << result(it("true or ?-Bool", "true"));
	ts << "or 4" << result(it("false or ?-Bool", "?-Bool"));
	ts << "equal 1" << result(it("false == false", "true"));
	ts << "equal 2" << result(it("false == true", "false"));
	ts << "equal 3" << result(it("false == ?-Bool", "?-Bool"));
	ts << "equal 4" << result(it("true == ?-Bool", "?-Bool"));
	ts << "equal 5" << result(it("?-Bool == ?-Bool", "?-Bool"));
	ts << "defferent 1" << result(it("false <> false", "false"));
	ts << "defferent 2" << result(it("false <> true", "true"));
	ts << "defferent 3" << result(it("false <> ?-Bool", "?-Bool"));
	ts << "defferent 4" << result(it("true <> ?-Bool", "?-Bool"));
	ts << "defferent 5" << result(it("?-Bool <> ?-Bool", "?-Bool"));
}


bool relation() {
	std::shared_ptr<TestCallback> cb = std::make_shared<TestCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interperter=rasmus::frontend::makeInterperter(cb);
	interperter->setup();
	std::string input = "2\n" 
		"T Name\n"
		"I Age\n" 
		"Bruce Jones\n" 
		"5\n" 
		"Mary Ross\n" 
		"12\n" 
		"Ann Bird\n" 
		"12\n" 
		"Kenneth Lewis\n" 
		"17\n";
	cb->relations["abe"] = input;
	if (!interperter->runLine("hat:=abe")) return false;
	ensure_eq(cb->relations["hat"], input);
	return true;
}


bool comments() {
	std::shared_ptr<TestCallback> cb = std::make_shared<TestCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interpreter=rasmus::frontend::makeInterperter(cb);
	interpreter->setup();

	std::string program = "// comment here \n"
		"a:=2+2;"
		"//another comment \n"
		"\n"
		"// hulla bulla \n"
		"// fdsa fdas \n"
		"b:=a+a;\n\n\n"
		"// asdf\n\n  \n\t"
		"// last comment\n\n\n\n"
		"b+b";
	
	if (!interpreter->runLine(program)) return false;
	ensure_eq(cb->printText, "16");
	return true;

}

int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
		.multi_test(base, "base")
		.multi_test(integer, "integer")
		.multi_test(bools, "bool")
		.test(comments, "comments")
		.test(relation, "relation");
}
