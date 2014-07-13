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
#include <frontend/interpreter.hh>
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
				std::vector<CharRange>) override {
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

	bool hasRelation(const char * name) override {
		return relations.count(name);
	}

};

bool it(std::string txt, const char * exp, bool error=false) {
	std::shared_ptr<TestCallback> cb = std::make_shared<TestCallback>();
	std::shared_ptr<rasmus::frontend::Interperter> interperter=rasmus::frontend::makeInterperter(cb);
	interperter->setup();
	size_t start=0;
	while (start < txt.size()) {
		size_t end=txt.find_first_of("\n", start);
		if (end == std::string::npos) end=txt.size();
		if (!interperter->runLine(txt.substr(start, end-start)) && !error) return false;
		start = end+1;
	}
	if (cb->printText != exp) {
		log_error() << "\"" << txt << "\": gave " << cb->printText << " expected " << exp << std::endl;
		return false;
	}
	if ((cb->errors > 0) != error)
		return false;
	interperter->freeGlobals();
	size_t oc=interperter->objectCount();
	if (oc != 0 && (cb->errors == 0)) {
		log_error() << oc << " objects where not freed" << std::endl;
		return false;
	}
	return true;
}

void base(rasmus::teststream & ts) {
    ts << "block" << result(it("(+val a=4 in a +)", "4"));
    ts << "function" << result(it("(func()->(Int)5 end)()", "5"));
    ts << "capture" << result(it("(+val a=\"abe\" in func()->(Text)a end+)()", "abe"));
	ts << "globalFunc" << result(it("y:=func()->(Int) 4 end\ny()", "4"));
	ts << "argCntErr" << result(it("(+ val x = func ()->(Int)1 end in x(2) +)", "", true));
	ts << "argTypeErr" << result(it("(+ val x = func (t:Text)->(Int)t end in x(2) +)", "", true));
	ts << "Crash1" << result(it("(+ val x := 2 in x + 5 +)", "", true));
	ts << "fac"<< result(it("(+ val f = func(f:Func, i:Int)->(Int) if i = 1 -> 1 & true -> f(f, i-1)*i fi end in f(f, 5) +)", "120"));
	ts << "fib"<< result(it("(+ val f = func(f:Func, i:Int)->(Int) if i = 1 -> 1 & i = 2 -> 1 &  true -> f(f, i-1) + f(f,i-2) fi end in f(f, 5) +)", "5"));
}

void text(rasmus::teststream & ts) {
    ts << "string" << result(it("\"abe\"", "abe"));
	ts << "equal1" << result(it("\"monkey\" = \"monkey\"", "true"));
	ts << "equal2" << result(it("\"monkey\" = \"mankey\"", "false"));
	ts << "equal3" << result(it("\"monkey\" = ?-Text", "false"));
	ts << "diferent1" << result(it("\"monkey\" <> \"monkey\"", "false"));
	ts << "diferent2" << result(it("\"monkey\" <> \"mankey\"", "true"));
	ts << "diferent3" << result(it("\"monkey\" <> ?-Text", "true"));
	ts << "global" << result(it("y:=\"abe\"\ny", "abe"));
    ts << "concat" << result(it("\"abe\"++\"bea\"", "abebea"));
	ts << "substr" << result(it("\"minime\"(1..4)", "ini"));
	ts << "len" << result(it("|\"abe\"|", "3"));
	ts << "?" << result(it("?-Text", "?-Text"));
    ts << "?concat" << result(it("?-Text++\"bea\"", "?-Text"));
	ts << "?substr" << result(it("\"minime\"(?-Int..4)", "?-Text"));
	ts << "?len" << result(it("|?-Text|", "?-Int"));

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

	ts << "equal1" << result(it("1 = 1", "true"));
	ts << "equal2" << result(it("1 = 2", "false"));
	ts << "equal3" << result(it("1 = ?-Int", "?-Bool"));
	ts << "diferent1" << result(it("1 <> 1", "false"));
	ts << "diferent2" << result(it("1 <> 2", "true"));
	ts << "diferent3" << result(it("1 <> ?-Int", "?-Bool"));

	ts << "less1" << result(it("1 < 1", "false"));
	ts << "less2" << result(it("1 < 2", "true"));
	ts << "less3" << result(it("1 < ?-Int", "?-Bool"));
	ts << "greater1" << result(it("1 > 1", "false"));
	ts << "greater2" << result(it("2 > 1", "true"));
	ts << "greater3" << result(it("1 > ?-Int", "?-Bool"));

	ts << "less equal 1" << result(it("1 <= 0", "false"));
	ts << "less equal 2" << result(it("1 <= 1", "true"));
	ts << "less equal 3" << result(it("1 <= ?-Int", "?-Bool"));
	ts << "greater equal 1" << result(it("0 >= 1", "false"));
	ts << "greater equal 2" << result(it("1 >= 1", "true"));
	ts << "greater equal 3" << result(it("1 >= ?-Int", "?-Bool"));
}

void ifs(rasmus::teststream & ts) {
	ts << "int" << result(it("if false -> 2 & true -> 1 fi", "1"));
	ts << "bool" << result(it("if false -> true & true -> ?-Bool fi", "?-Bool"));
	ts << "text" << result(it("if false -> \"abe\" & true -> \"kat\" fi", "kat"));
	ts << "any" << result(it("(+ val ident=func(x:Any)->(Any) x end in if false -> ident(2) & ?-Bool -> ident(3) & true -> ident(4) fi +)", "4"));
	ts << "first" << result(it("if false -> 1 & true -> 2 & true -> 3 & false -> 4 fi", "2"));
	ts << "none-int" << result(it("if false -> 0 fi", "?-Int"));
	ts << "none-bool" << result(it("if false -> true fi", "?-Bool"));
	ts << "none-text" << result(it("if false -> \"abe\" fi", "?-Text"));
	ts << "none-any" << result(it("(+ val ident=func()->(Any) 0 end in if false -> x() fi +)", "", true));
	ts << "err" << result(it("if 42 -> true fi","",true));
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
	ts << "equal 1" << result(it("false = false", "true"));
	ts << "equal 2" << result(it("false = true", "false"));
	ts << "equal 3" << result(it("false = ?-Bool", "?-Bool"));
	ts << "equal 4" << result(it("true = ?-Bool", "?-Bool"));
	ts << "equal 5" << result(it("?-Bool = ?-Bool", "?-Bool"));
	ts << "defferent 1" << result(it("false <> false", "false"));
	ts << "defferent 2" << result(it("false <> true", "true"));
	ts << "defferent 3" << result(it("false <> ?-Bool", "?-Bool"));
	ts << "defferent 4" << result(it("true <> ?-Bool", "?-Bool"));
	ts << "defferent 5" << result(it("?-Bool <> ?-Bool", "?-Bool"));
	ts << "not 1" << result(it("not ?-Bool", "?-Bool"));
	ts << "not 2" << result(it("not true", "false"));
	ts << "not 2" << result(it("not false", "true"));
	ts << "var" << result(it("x:=?-Bool\nx", "?-Bool"));

	ts << "less0" << result(it("true < true", "false"));
	ts << "less2" << result(it("false < true", "true"));
	ts << "less3" << result(it("true < ?-Bool", "?-Bool"));
	ts << "greater0" << result(it("true > true", "false"));
	ts << "greater2" << result(it("true > false", "true"));
	ts << "greater3" << result(it("true > ?-Bool", "?-Bool"));

	ts << "less equal 0" << result(it("true <= false", "false"));
	ts << "less equal 2" << result(it("true <= true", "true"));
	ts << "less equal 3" << result(it("true <= ?-Bool", "?-Bool"));
	ts << "greater equal 0" << result(it("false >= true", "false"));
	ts << "greater equal 2" << result(it("true >= true", "true"));
	ts << "greater equal 3" << result(it("true >= ?-Bool", "?-Bool"));

}

void tuple(rasmus::teststream & ts) {
	ts << "construct" << result(it("tup()", "()"));
	ts << "construct2" << result(it("tup(abe: 4, kat: ?-Text)", "(abe: 4, kat: ?-Text)"));
	ts << "construct3" << result(it("tup(kat: 1)", "(kat: 1)"));
	ts << "construct4" << result(it("tup(boo: false)", "(boo: false)"));
	ts << "construct5" << result(it("tup(boo: true)", "(boo: true)"));
	ts << "construct6" << result(it("tup(foo: \"bar\")", "(foo: \"bar\")"));
	ts << "construct7" << result(it("tup(foo: 1, bar: true, baz: \"bax\")", "(foo: 1, bar: true, baz: \"bax\")"));
	ts << "equality1" << result(it("tup() = tup()", "true"));
	ts << "equality2" << result(it("tup() = tup(abe: 4)", "false"));
	ts << "equality3" << result(it("tup(kat: 1) = tup(abe: 4)", "false"));
	ts << "equality4" << result(it("tup(kat: 1, bar: 2) = tup(kat: 1, bar: 2)", "true"));
	ts << "equality5" << result(it("tup(kat: 1, bar: 1) = tup(kat: 1, bar: 2)", "false"));
	ts << "equality6" << result(it("tup(kat: 1, bar: 2, baz: false) = tup(kat: 1, bar: 2, baz: true)", "false"));
	ts << "equality7" << result(it("tup(kat: 1, bar: 2, baz: false) = tup(kat: 1, bar: 2)", "false"));
	ts << "equality8" << result(it("tup(kat: 1, bar: 2, baz: false) = tup()", "false"));
	ts << "equality9" << result(it("tup(kat: \"foo\") = tup(kat: \"foo\")", "true"));
	ts << "equality10" << result(it("tup(kat: \"foo\") = tup(kat: \"bar\")", "false"));
	ts << "equality11" << result(it("tup(hat: \"foo\") = tup(kat: \"foo\")", "false"));
	ts << "equality12" << result(it("tup(kat: ?-Int) = tup(kat: 123)", "false"));
	ts << "equality13" << result(it("tup(hat: ?-Bool) = tup(hat: true)", "false"));
	ts << "inequality1" << result(it("tup() <> tup()", "false"));
	ts << "inequality2" << result(it("tup() <> tup(abe: 4)", "true"));
	ts << "inequality3" << result(it("tup(foo: true) <> tup(foo: true)", "false"));
	ts << "inequality4" << result(it("tup(foo: true) <> tup(foo: false)", "true"));
	ts << "inequality5" << result(it("tup(bar: \"foo\") <> tup(bar: \"foo\")", "false"));
	ts << "inequality6" << result(it("tup(bar: \"baz\") <> tup(bar: \"foo\")", "true"));
	ts << "union" << result(it("tup(abe: 4, kat: 5) << tup(kat: ?-Text, baz: 2)", "(abe: 4, kat: 5, baz: 2)"));
	ts << "rem" << result(it("tup(abe: 4, kat: ?-Text)\\abe", "(kat: ?-Text)"));
	ts << "rem2" << result(it("tup(abe: 4, kat: ?-Text)\\baz", "", true));
	ts << "has" << result(it("has(tup(abe: 4, kat: ?-Text), kat)", "true"));
	ts << "has2" << result(it("has(tup(abe: 4, kat: ?-Text), baz)", "false"));
	ts << "dot" << result(it("tup(abe: 4, kat: ?-Text).abe", "4"));
	ts << "dot2" << result(it("tup(foo: true, kat: ?-Text).foo", "true"));
	ts << "dot3" << result(it("tup(foo: true, bar: \"baz\", kat: ?-Text).bar", "baz"));
	ts << "dot_err" << result(it("tup(abe: 4, kat: ?-Text).baz", "", true));
	ts << "len" << result(it("|tup(abe: 4, kat: ?-Text)|", "2"));
	// ts << "?union" << result(it("tup(abe: 4, kat: 5) << ?-Tup", "?-Tup"));
	// ts << "?dot" << result(it("?-Tup.abe", "", true));
	// ts << "?rem" << result(it("?-Tup\abe", "", true));
	// ts << "+len" << result(it("|?-Tup|", "?-Int"));
	// ts << "has" << result(it("has(?-Tup, kat)", "?-Bool"));
}

void relation(rasmus::teststream & ts) {
	ts << "zero" << result(it("|zero|", "0"));
	ts << "one" << result(it("|one|", "1"));
	ts << "equality1" << result(it("zero = zero", "true"));
	ts << "equality2" << result(it("zero = one", "false"));
	ts << "inequality1" << result(it("zero <> zero", "false"));
	ts << "inequality2" << result(it("zero <> one", "true"));
	ts << "construct" << result(it("|rel(tup(abe: 4, kat: ?-Text))|", "1"));
	ts << "has" << result(it("has(rel(tup(abe: 4, kat: ?-Text)), kat)", "true"));
	ts << "has2" << result(it("has(rel(tup(abe: 4, kat: ?-Text)), baz)", "false"));
	ts << "union" << result(it("|rel(tup(abe: 4, kat: ?-Text))+rel(tup(abe: 5, kat: ?-Text))|", "2"));
	ts << "union2" << result(it("rel(tup(abe: 4, kat: \"bar\"))+rel(tup(abe: 4, kat: \"bar\")) = rel(tup(abe: 4, kat: \"bar\"))", "true"));
	ts << "union3" << result(it("|rel(tup(a:1, b:2)) + rel(tup(a:9, b:2)) + rel(tup(a:1, b:2))|", "2"));
	ts << "diff" << result(it("|rel(tup(abe: 4, kat: ?-Text))-rel(tup(abe: 4, kat: ?-Text))|", "0"));
	ts << "join" << result(it("rel(tup(abe: 4, kat: 5)) * rel(tup(kat: 5, bar: 6)) = rel(tup(abe: 4, kat: 5, bar: 6))", "true"));
	ts << "select" << result(it("|rel(tup(abe: 4, kat:5))?(#.kat=5)|", "1"));
	ts << "pos_project1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |+ abe,baz = rel(tup(abe: 4, baz:2))", "true"));
	ts << "pos_project2" << result(it("(rel(tup(abe: 4, kat:5, baz:2)) + rel(tup(abe:4, kat:5, baz:3))) |+ abe,kat = rel(tup(abe: 4, kat:5))", "true"));
	ts << "neg_project1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |- abe,baz = rel(tup(kat: 5))", "true"));
	ts << "rename" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-foo, kat<-taz] = rel(tup(foo: 4, taz:5, baz:2))", "true"));
	ts << "min" << result(it("min(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "4"));
	ts << "max" << result(it("max(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "5"));
	ts << "add" << result(it("add(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "9"));
	ts << "mult" << result(it("mult(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "20"));
	ts << "count" << result(it("count(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "2"));
	ts << "count2" << result(it("count(rel(tup(abe: ?-Int)) + rel(tup(abe: 5)), abe)", "1"));
	ts << "count3" << result(it("count(rel(tup(abe: ?-Text)) + rel(tup(abe: \"foo\")), abe)", "1"));
	ts << "count4" << result(it("count(rel(tup(abe: ?-Text)), abe)", "0"));
	ts << "count_err" << result(it("count(rel(tup()), abe)", "", true));
	//TODO factor
}


bool relation_external() {
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
		.multi_test(text, "text")
		.multi_test(bools, "bool")
		.multi_test(tuple, "tuple")
		.multi_test(relation, "relation")
		.multi_test(ifs, "if")
		.test(comments, "comments")
		.test(relation_external, "relation_external");
}
