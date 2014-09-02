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

	void deleteRelation(const char * name) override {
		relations.erase(name);
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
		log_error() << "\"" << txt << "\": gave " << cb->printText << " expected '" << exp << "'" << std::endl;
		return false;
	}
	if ((cb->errors > 0) != error)
		return false;
	interperter->freeGlobals();
	size_t oc=interperter->objectCount();
	if (oc != 0 && (cb->errors == 0)) {
		log_error() << oc << " objects were not freed" << std::endl;
		return false;
	}
	return true;
}

void builtins(rasmus::teststream & ts) {
	ts << "is-Bool1" << result(it("is-Bool(1)", "false"));
	ts << "is-Bool2" << result(it("is-Bool(\"foo\")", "false"));
	ts << "is-Bool3" << result(it("is-Bool(true)", "true"));
	ts << "is-Bool4" << result(it("is-Bool(false)", "true"));
	ts << "is-Bool5" << result(it("is-Bool(?-Bool)", "true"));
	ts << "is-Bool6" << result(it("is-Bool(rel(tup(a:1)))", "false"));
	ts << "is-Bool7" << result(it("is-Bool((func()->(Bool) true end)())", "true"));
	ts << "is-Bool8" << result(it("is-Bool((func()->(Any) true end)())", "true"));
	ts << "is-Bool9" << result(it("is-Bool((func()->(Any) 42 end)())", "false"));
	ts << "is-Bool10" << result(it("is-Bool((func()->(Any) \"foo\" end)())", "false"));
	ts << "is-Bool11" << result(it("is-Bool((func()->(Any) rel(tup(a:1)) end)())", "false"));
	ts << "is-Int1" << result(it("is-Int(1234)", "true"));
	ts << "is-Int2" << result(it("is-Int(0)", "true"));
	ts << "is-Int3" << result(it("is-Int(-1234565432)", "true"));
	ts << "is-Int4" << result(it("is-Int(?-Int)", "true"));
	ts << "is-Int5" << result(it("is-Int(\"foo\")", "false"));
	ts << "is-Int6" << result(it("is-Int(true)", "false"));
	ts << "is-Int7" << result(it("is-Int(rel(tup(a:1)))", "false"));
	ts << "is-Int8" << result(it("is-Int((func()->(Any) true end)())", "false"));
	ts << "is-Int9" << result(it("is-Int((func()->(Any) 42 end)())", "true"));
	ts << "is-Text1" << result(it("is-Text(true)", "false"));
	ts << "is-Text2" << result(it("is-Text(\"foo\")", "true"));
	ts << "is-Text3" << result(it("is-Text(?-Text)", "true"));
	ts << "is-Text4" << result(it("is-Text(42)", "false"));
	ts << "is-Text5" << result(it("is-Text((func()->(Any) true end)())", "false"));
	ts << "is-Text6" << result(it("is-Text((func()->(Any) \"foo\" end)())", "true"));
	ts << "is-Tup1" << result(it("is-Tup(true)", "false"));
	ts << "is-Tup2" << result(it("is-Tup(tup(a:1))", "true"));
	ts << "is-Tup3" << result(it("is-Tup(42)", "false"));
	ts << "is-Tup4" << result(it("is-Tup((func()->(Any) true end)())", "false"));
	ts << "is-Tup5" << result(it("is-Tup((func()->(Any) tup(a:1) end)())", "true"));
	ts << "is-Rel1" << result(it("is-Rel(true)", "false"));
	ts << "is-Rel2" << result(it("is-Rel(\"foo\")", "false"));
	ts << "is-Rel3" << result(it("is-Rel(rel(tup(a:1)))", "true"));
	ts << "is-Rel4" << result(it("is-Rel(42)", "false"));
	ts << "is-Rel5" << result(it("is-Rel((func()->(Any) true end)())", "false"));
	ts << "is-Rel6" << result(it("is-Rel((func()->(Any) rel(tup(a:\"\")) end)())", "true"));
	ts << "is-Func1" << result(it("is-Func(true)", "false"));
	ts << "is-Func2" << result(it("is-Func(\"foo\")", "false"));
	ts << "is-Func4" << result(it("is-Func(42)", "false"));
	ts << "is-Func5" << result(it("is-Func((func()->(Any) true end))", "true"));
	ts << "is-Func6" << result(it("is-Func((func()->(Any) \"foo\" end))", "true"));
	ts << "is-Func7" << result(it("is-Func((func()->(Any) (func()->(Any) true end) end)())", "true"));
	ts << "is-Atom1" << result(it("is-Atom(true)", "true"));
	ts << "is-Atom2" << result(it("is-Atom(\"foo\")", "true"));
	ts << "is-Atom3" << result(it("is-Atom(rel(tup(a:1)))", "false"));
	ts << "is-Atom4" << result(it("is-Atom(tup(a:1))", "false"));
	ts << "is-Atom5" << result(it("is-Atom(42)", "true"));
	ts << "is-Atom6" << result(it("is-Atom((func()->(Any) true end)())", "true"));
	ts << "is-Atom7" << result(it("is-Atom((func()->(Any) 42 end)())", "true"));
	ts << "is-Atom8" << result(it("is-Atom((func()->(Any) \"foo\" end)())", "true"));
	ts << "is-Atom9" << result(it("is-Atom((func()->(Any) rel(tup(a:1)) end)())", "false"));
	ts << "is-Atom10" << result(it("is-Atom((func()->(Any) (func()->(Any) true end) end)())", "false"));
	

}

/*
		case TokenType::TK_ISATOM:
			ICE("IMPLEMENT ME");
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
*/

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

void crash(rasmus::teststream & ts) {
	ts << "crash1" << result(it("X := one; (X ? (X = one)) = one", "true"));
	ts << "crash2" << result(it("a:=1\nb:=0\na/b","?-Int"));
	ts << "crash3" << result(it("foo := func (x:Int) -> (Int) x end; bar := func (y:Int) -> (Int) foo(y) end; bar(42)", "42"));
	ts << "crash4" << result(it("x := 0; 123 mod x", "?-Int"));
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
	ts << "construct_err1" << result(it("tup(a:1, a:2)", "", true));
	ts << "construct_err2" << result(it("tup(ABE:1, ABE:1)", "", true));
	ts << "construct_err3" << result(it("tup(ABE:\"foo\", ABE:1)", "", true));
	ts << "construct_err4" << result(it("tup(a:1, b:true, c: \"foo\", a:true)", "", true));
	ts << "construct_err5" << result(it("tup(ABE:?-Text, ABE:?-Text)", "", true));
	ts << "construct_err6" << result(it("tup(foo:?-Int, foo:?-Text)", "", true));
	ts << "construct_err7" << result(it("tup(foo:?-Int, foo:?-Int)", "", true));
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
	ts << "equality14" << result(it("tup(hat:1, bar:123) = tup(hat:1, bar:\"123\")", "false"));
	ts << "equality_error1" << result(it("tup(hat:1) = true", "", true));
	ts << "equality_error2" << result(it("tup() = rel(tup())", "", true));
	ts << "inequality1" << result(it("tup() <> tup()", "false"));
	ts << "inequality2" << result(it("tup() <> tup(abe: 4)", "true"));
	ts << "inequality3" << result(it("tup(foo: true) <> tup(foo: true)", "false"));
	ts << "inequality4" << result(it("tup(foo: true) <> tup(foo: false)", "true"));
	ts << "inequality5" << result(it("tup(bar: \"foo\") <> tup(bar: \"foo\")", "false"));
	ts << "inequality6" << result(it("tup(bar: \"baz\") <> tup(bar: \"foo\")", "true"));
	ts << "inequality7" << result(it("tup(a:1) <> tup(a:1)", "false"));
	ts << "inequality8" << result(it("tup(a:1,b:3,c:3) <> tup(a:1,b:true,c:3)", "true"));
	ts << "inequality9" << result(it("tup(a:1,b:true,c:\"foo\") <> tup(a:1,b:true,c:\"foo\")", "false"));
	ts << "inequality10" << result(it("tup(a:1,b:true,c:\"foo\") <> tup(a:1,b:true,c:\"foo\",d:1)", "true"));
	ts << "inequality11" << result(it("tup(a:1,b:true,c:\"foo\",d:1) <> tup(a:1,b:true,c:\"foo\")", "true"));
	ts << "inequality_error1" << result(it("tup() <> rel(tup())", "", true));
	ts << "inequality_error2" << result(it("tup() <> true", "", true));
	ts << "union1" << result(it("(tup(abe: 4, kat: 5) << tup(kat: ?-Text, baz: 2)) = tup(abe:4, kat: ?-Text, baz: 2)", "true"));
	ts << "union2" << result(it("(tup() << tup(a:1)) = tup(a:1)", "true"));
	ts << "union3" << result(it("(tup(a:1) << tup()) = tup(a:1)", "true"));
	ts << "union4" << result(it("(tup() << tup() << tup() << tup()) = tup()", "true"));
	ts << "union5" << result(it("(tup(a:1) << tup(b:2) << tup(c:3)) = tup(a:1,b:2,c:3)", "true"));
	ts << "union6" << result(it("(tup(a:1) << tup(a:true)) = tup(a:true)", "true"));
	ts << "union7" << result(it("(tup(a:1) << tup(b:true) << tup(c:\"\")) = tup(a:1,b:true,c:\"\")", "true"));
	ts << "union_error1" << result(it("tup() << rel(tup())", "", true));
	ts << "union_error2" << result(it("tup() << true", "", true));
	ts << "union_error3" << result(it("(tup(a:\"foo\") << (tup(a:1).a))", "", true));
	ts << "union_error4" << result(it("(tup(a:\"foo\") << 1)", "", true));
	ts << "union_error5" << result(it("(tup(a:\"foo\") << rel(tup()))", "", true));
	ts << "union_error6" << result(it("(tup(a:\"foo\") << false)", "", true));
	ts << "rem1" << result(it("tup(abe: 4, kat: ?-Text)\\abe", "(kat: ?-Text)"));
	ts << "rem2" << result(it("tup(a:1,b:true,c:\"fo\")\\a", "(b: true, c: \"fo\")"));
	ts << "rem3" << result(it("tup(a:1,b:true,c:\"fo\")\\b", "(a: 1, c: \"fo\")"));
	ts << "rem4" << result(it("tup(a:1,b:true,c:\"fo\")\\c", "(a: 1, b: true)"));
	ts << "rem5" << result(it("(((tup(a:1,b:2,c:3)\\a)\\b)\\c) = tup()", "true"));
	ts << "rem_error1" << result(it("tup(abe: 4, kat: ?-Text)\\baz", "", true));
	ts << "rem_error2" << result(it("tup()\\a", "", true));
	ts << "rem_error3" << result(it("tup(a:1,b:2,c:3)\\abc", "", true));
	ts << "rem_error4" << result(it("rel(tup(a:1))\\a", "", true));
	ts << "rem_error5" << result(it("true\\true", "", true));
	ts << "rem_error6" << result(it("tup(a:1)\\true", "", true));
	ts << "rem_error7" << result(it("tup(a:1)\\\"a\"", "", true));
	ts << "has1" << result(it("has(tup(abe: 4, kat: ?-Text), kat)", "true"));
	ts << "has2" << result(it("has(tup(abe: 4, kat: ?-Text), baz)", "false"));
	ts << "has3" << result(it("has(tup(abe: 4, kat: ?-Text), abe)", "true"));
	ts << "has4" << result(it("has(tup(b:1,c:2,a:1,d:23), a)", "true"));
	ts << "has5" << result(it("has(tup(a:1,c:2,b:1,d:23), a)", "true"));
	ts << "has6" << result(it("has(tup(b:1,c:2,a:\"foo\",d:23), a)", "true"));
	ts << "has7" << result(it("has(tup(b:1,c:2,a:true,d:23), a)", "true"));
	ts << "has8" << result(it("has(tup(b:1,c:2,a:1,d:23), qqq)", "false"));
	ts << "has9" << result(it("has(tup(b:1,c:2,a:1,d:23), cc)", "false"));
	ts << "has_error1" << result(it("has(tup(), \"foooo\")", "", true));
	ts << "has_error2" << result(it("has(true, true)", "", true));
	ts << "has_error3" << result(it("has(\"foo\", foo)", "", true));
	ts << "has_error4" << result(it("has(tup(), 123)", "", true));
	ts << "has_error5" << result(it("has(tup(), true)", "", true));
	ts << "dot1" << result(it("tup(abe: 4, kat: ?-Text).abe", "4"));
	ts << "dot2" << result(it("tup(foo: true, kat: ?-Text).foo", "true"));
	ts << "dot3" << result(it("tup(foo: true, bar: \"baz\", kat: ?-Text).bar", "baz"));
	ts << "dot4" << result(it("(tup(a:1) << tup(a:\"foo\")).a", "foo"));
	ts << "dot5" << result(it("(tup() << tup(a:1)).a", "1"));
	ts << "dot6" << result(it("tup(a:1,b:true,c:\"foo\",d:999999999).a", "1"));
	ts << "dot7" << result(it("tup(a:1,b:true,c:\"foo\",d:999999999).b", "true"));
	ts << "dot8" << result(it("tup(a:1,b:true,c:\"foo\",d:999999999).c", "foo"));
	ts << "dot9" << result(it("tup(a:1,b:true,c:\"foo\",d:999999999).d", "999999999"));
	ts << "dot_error1" << result(it("(tup()).true", "", true));
	ts << "dot_error2" << result(it("tup(a:1,b:true,c:\"foo\",d:999999999).e", "", true));
	ts << "dot_error3" << result(it("(tup()).123", "", true));
	ts << "dot_error4" << result(it("(tup()).b", "", true));
	ts << "dot_error5" << result(it("tup(abe: 4, kat: ?-Text).baz", "", true));
	ts << "len1" << result(it("|tup(abe: 4, kat: ?-Text)|", "2"));
	ts << "len2" << result(it("|tup()|", "0"));
	ts << "len3" << result(it("|tup(abe: 4, kat: ?-Text) << tup(abe: 3, kat: \"foo\")|", "2"));
	ts << "len4" << result(it("|tup() << tup() << tup()|", "0"));
	ts << "len5" << result(it("|tup(a:1) << tup(a:2) << tup(a:3)|", "1"));
	ts << "len6" << result(it("|tup(a:1,b:true,c:\"foo\",d:1,e:9999999999,f:?-Int,g:?-Text,h:?-Bool)|", "8"));
	// ts << "?union" << result(it("tup(abe: 4, kat: 5) << ?-Tup", "?-Tup"));
	// ts << "?dot" << result(it("?-Tup.abe", "", true));
	// ts << "?rem" << result(it("?-Tup\abe", "", true));
	// ts << "+len" << result(it("|?-Tup|", "?-Int"));
	// ts << "has" << result(it("has(?-Tup, kat)", "?-Bool"));
}

void relation(rasmus::teststream & ts) {
	ts << "zero1" << result(it("|zero|", "0"));
	ts << "zero2" << result(it("(zero + zero) = zero", "true"));
	ts << "zero3" << result(it("(zero * one) = zero", "true"));
	ts << "zero4" << result(it("(zero * zero) = zero", "true"));
	ts << "one1" << result(it("|one|", "1"));
	ts << "one2" << result(it("(one + one) = one", "true"));
	ts << "one3" << result(it("A:=rel(tup(a:1)); one * A = A", "true"));
	ts << "equality1" << result(it("zero = zero", "true"));
	ts << "equality2" << result(it("zero = one", "false"));
	ts << "equality3" << result(it("rel(tup(a:1, b:2)) = rel(tup(a:1, b:2))", "true"));
	ts << "equality4" << result(it("rel(tup(a:1, b:2)) = rel(tup(a:1, b:3))", "false"));
	ts << "equality5" << result(it("rel(tup(a:1, b:2)) = rel(tup(a:1, c:2))", "false"));
	ts << "equality6" << result(it(""
"T := rel(tup(a:1,b:1,c:4)) + rel(tup(a:2,b:7,c:9)) + rel(tup(a:3,b:0,c:9)) + rel(tup(a:7,b:2,c:0)); "
"S := rel(tup(b:0,c:9,a:3)) + rel(tup(b:1,c:4,a:1)) + rel(tup(b:2,c:0,a:7)) + rel(tup(b:7,c:9,a:2)); "
"S = T", "true"));
	ts << "equality7" << result(it("rel(tup(a:1,b:true,c:\"foo\")) = rel(tup(a:1,b:true,c:\"foo\"))", "true"));
	ts << "equality8" << result(it("rel(tup(a:2,b:true,c:\"foo\")) = rel(tup(a:1,b:true,c:\"foo\"))", "false"));
	ts << "equality9" << result(it("rel(tup(a:1,b:false,c:\"foo\")) = rel(tup(a:1,b:true,c:\"foo\"))", "false"));
	ts << "equality10" << result(it("rel(tup(a:1,b:true,c:\"fooz\")) = rel(tup(a:1,b:true,c:\"foo\"))", "false"));
	ts << "equality11" << result(it("rel(tup(a:1,b:true,c:\"foo\")) = rel(tup(a:1,b:false,c:\"foo\"))", "false"));
	ts << "equality12" << result(it("rel(tup(a:1,b:true,c:\"foo\")) = rel(tup(b:false,c:\"foo\",a:1))", "false"));
	ts << "equality13" << result(it("rel(tup(a:3,b:true,c:\"foo\")) = rel(tup(b:false,c:\"foo\",a:true))", "false"));

	ts << "equality14" << result(it("rel(tup(a:1)) = rel(tup(a:true))", "false"));
	ts << "equality15" << result(it("rel(tup(a:\"foo\")) = rel(tup(a:true))", "false"));
	ts << "equality16" << result(it("rel(tup(a:1,b:true)) = rel(tup(a:1,b:2))", "false"));
	ts << "equality17" << result(it("rel(tup(a:1)) = rel(tup(a:1,b:2))", "false"));
	ts << "equality18" << result(it("rel(tup(a:1,c:true)) = rel(tup(a:1))", "false"));
	ts << "equality19" << result(it("rel(tup(a:1,b:2,c:3,d:4)) = rel(tup())", "false"));
	ts << "equality20" << result(it("rel(tup()) = rel(tup(a:1,b:2,c:3,d:4))", "false"));
	ts << "equality21" << result(it("(rel(tup(a:1)) + rel(tup(a:2))) = (rel(tup(a:2)) + rel(tup(a:1)))", "true"));
	ts << "equality22" << result(it("(rel(tup(a:1, b:2)) + rel(tup(a:2, b:2))) = (rel(tup(b:2, a:2)) + rel(tup(b:2, a:1)))", "true"));
	ts << "equality23" << result(it("(rel(tup(a:2, b:2))) = (rel(tup(b:2, a:2)) + rel(tup(b:2, a:1)))", "false"));
	ts << "equality24" << result(it("(rel(tup(a:1, b:2)) + rel(tup(a:2, b:2))) = (rel(tup(b:2, a:1)))", "false"));
	ts << "equality25" << result(it("(rel(tup(a:1, b:2)) + rel(tup(a:2, b:2))) = rel(tup())", "false"));
	ts << "equality26" << result(it("rel(tup(a:?-Int)) = rel(tup(a:?-Int))", "true"));
	ts << "equality27" << result(it("rel(tup(a:?-Bool)) = rel(tup(a:?-Int))", "false"));
	ts << "equality28" << result(it("rel(tup(a:\"foo\")) = rel(tup(a:?-Int))", "false"));
	ts << "equality29" << result(it("rel(tup(a:1)) = rel(tup(a:?-Int))", "false"));
	ts << "equality30" << result(it("rel(tup(a:true)) = rel(tup(a:?-Bool))", "false"));
	ts << "equality_error1" << result(it("rel(tup()) = tup()", "", true));
	ts << "equality_error2" << result(it("rel(tup()) = false", "", true));
	ts << "equality_error3" << result(it("rel(tup()) = 123", "", true));
	ts << "equality_error4" << result(it("rel(tup()) = \"foo\"", "", true));
	ts << "equality_error5" << result(it("rel(tup(a:1,b:2)) = tup(a:1,b:2)", "", true));
	ts << "inequality1" << result(it("zero <> zero", "false"));
	ts << "inequality2" << result(it("zero <> one", "true"));
	ts << "inequality3" << result(it("rel(tup(a:1, b:2)) <> rel(tup(a:1, b:2))", "false"));
	ts << "inequality4" << result(it("rel(tup(a:1, b:2)) <> rel(tup(a:1, c:2))", "true"));
	ts << "inequality5" << result(it("rel(tup(a:1, b:2)) <> rel(tup(a:1))", "true"));
	ts << "inequality6" << result(it("rel(tup(a:1, b:2)) <> rel(tup(a:1, b:2, c:3))", "true"));
	ts << "inequality7" << result(it("(rel(tup(a:1)) + rel(tup(a:2))) <> rel(tup(a:1))", "true"));
	ts << "inequality8" << result(it("(rel(tup(a:1)) + rel(tup(a:2))) <> (rel(tup(a:2)) + rel(tup(a:1)))", "false"));
	ts << "inequality9" << result(it("rel(tup(a:\"foo\", b:2)) <> rel(tup(a:\"foo\", b:2))", "false"));
	ts << "inequality10" << result(it("rel(tup(a:\"foo\", b:2)) <> rel(tup(a:\"zfoo\", b:2))", "true"));
	ts << "inequality11" << result(it("rel(tup(a:1, b:true)) <> rel(tup(a:1, b:true))", "false"));
	ts << "inequality12" << result(it("rel(tup(a:1, b:true)) <> rel(tup(a:1, b:false))", "true"));
	ts << "inequality13" << result(it("rel(tup(a:1, b:2)) <> rel(tup(a:true, b:2))", "true"));
	ts << "construct1" << result(it("|rel(tup(abe: 4, kat: ?-Text))|", "1"));
	ts << "construct2" << result(it("|rel(tup())|", "1"));
	ts << "construct3" << result(it("rel(tup()) = one", "true"));
	ts << "has1" << result(it("has(rel(tup(abe: 4, kat: ?-Text)), kat)", "true"));
	ts << "has2" << result(it("has(rel(tup(abe: 4, kat: ?-Text)), baz)", "false"));
	ts << "has3" << result(it("has(rel(tup()), baz)", "false"));
	ts << "has4" << result(it("has(rel(tup(a:1,b:2,c:3)), c)", "true"));
	ts << "has5" << result(it("has(rel(tup(a:1,b:2,c:3)), q)", "false"));
	ts << "has6" << result(it("has(rel(tup(a:2, b:2))*rel(tup(z:\"foo\")), a)", "true"));
	ts << "has7" << result(it("has(rel(tup(a:2, b:2))*rel(tup(z:\"foo\")), b)", "true"));
	ts << "has8" << result(it("has(rel(tup(a:2, b:2))*rel(tup(z:\"foo\")), z)", "true"));
	ts << "has9" << result(it("has(rel(tup(a:2, b:2))*rel(tup(z:\"foo\")), k)", "false"));
	ts << "has10" << result(it("has(rel(tup()), whatALongNameaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)", "false"));
	ts << "has11" << result(it("has(rel(tup(whatALongNameaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa:true)), whatALongNameaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa)", "true"));
	ts << "union" << result(it("|rel(tup(abe: 4, kat: ?-Text))+rel(tup(abe: 5, kat: ?-Text))|", "2"));
	ts << "union2" << result(it("rel(tup(abe: 4, kat: \"bar\"))+rel(tup(abe: 4, kat: \"bar\")) = rel(tup(abe: 4, kat: \"bar\"))", "true"));
	ts << "union3" << result(it("|rel(tup(a:1, b:2)) + rel(tup(a:9, b:2)) + rel(tup(a:1, b:2))|", "2"));
	ts << "union4" << result(it("(rel(tup(a:1, b:2)) + rel(tup(a:1, b:2)) + rel(tup(a:1, b:2))) = rel(tup(a:1, b:2))", "true"));
	ts << "union5" << result(it("rel(tup(a:1,b:2)) + rel(tup(a:1,b:2)) = rel(tup(b:2,a:1))", "true"));
	ts << "union6" << result(it("|rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))|", "3"));
	ts << "union7" << result(it("|rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:2)) + rel(tup(a:1))|", "2"));
	ts << "union8" << result(it("rel(tup(a:true)) + rel(tup(a:false)) = rel(tup(a:false)) + rel(tup(a:true))", "true"));
	ts << "union9" << result(it("rel(tup(a:1,b:true,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "true"));
	ts << "union10" << result(it("rel(tup(a:1,b:true,c:\"foo\")) + rel(tup(b:false,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "false"));
	ts << "union11" << result(it("rel(tup(a:1,b:true,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:1)) = rel(tup(b:true,a:1,c:\"foo\"))", "true"));
	ts << "union12" << result(it("rel(tup(a:true,b:true,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:false)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "false"));
	ts << "union13" << result(it("rel(tup(a:1,b:true,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:0,c:\"foo\"))", "false"));
	ts << "union14" << result(it("rel(tup()) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:0,c:\"foo\"))", "false"));
	ts << "union15" << result(it("(rel(tup(a:1)) ? false) + rel(tup(a:2)) = rel(tup(a:2))", "true"));
	ts << "union16" << result(it("(rel(tup(a:1)) ? false) + rel(tup(a:3)) = rel(tup(a:2))", "false"));
	ts << "union17" << result(it("(rel(tup(a:1,b:true)) ? false) + rel(tup(a:3,b:false)) = rel(tup(a:3,b:false))", "true"));
	ts << "union18" << result(it("|rel(tup(a:?-Int)) + rel(tup(a:1))|", "2"));
	ts << "union19" << result(it("|rel(tup(a:?-Bool)) + rel(tup(a:true))|", "2"));
	ts << "union20" << result(it("|rel(tup(a:?-Text)) + rel(tup(a:\"foo\"))|", "2"));
	ts << "union_error1" << result(it("rel(tup(a:1, b:2)) + rel(tup(a:1, c:2))", "", true));
	ts << "union_error2" << result(it("rel(tup(a:true,b:true,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "", true));
	ts << "union_error3" << result(it("rel(tup(a:1,b:1,c:\"foo\")) + rel(tup(b:true,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "", true));
	ts << "union_error4" << result(it("rel(tup(a:1,b:true,c:true)) + rel(tup(b:true,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "", true));
	ts << "union_error5" << result(it("rel(tup(a:1,b:true,c:\"foo\")) + rel(tup(b:3,c:\"foo\",a:2)) = rel(tup(c:\"foo\",b:true,a:2)) + rel(tup(b:true,a:1,c:\"foo\"))", "", true));
	ts << "union_error6" << result(it("rel(tup(a:1)) + rel(tup(a:false))", "", true));
	ts << "union_error7" << result(it("rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3, b:1))", "", true));
	ts << "union_error8" << result(it("rel(tup(a:1)) + rel(tup(a:1,b:true))", "", true));
	ts << "union_error9" << result(it("(rel(tup(a:1)) ? false) + rel(tup(b:2))", "", true));
	ts << "union_error10" << result(it("(rel(tup(a:true)) ? false) + rel(tup(a:2))", "", true));
	ts << "union_error11" << result(it("(rel(tup(a:\"foo\")) ? false) + rel(tup(a:2))", "", true));
	ts << "union_error12" << result(it("(rel(tup(a:1)) ? false) + rel(tup(a:2, b:1))", "", true));
	ts << "union_error13" << result(it("(rel(tup(a:1)) ? false) + rel(tup(a:\"foo\"))", "", true));
	ts << "union_error14" << result(it("(rel(tup(a:1)) ? false) + rel(tup(a:true))", "", true));
	ts << "union_error15" << result(it("(rel(tup(a:1)) ? false) + rel(tup(b:1))", "", true));
	ts << "union_error16" << result(it("rel(tup(a:?-Int)) + rel(tup(a:?-Bool))", "", true));
	ts << "diff1" << result(it("|rel(tup(abe: 4, kat: ?-Text))-rel(tup(abe: 4, kat: ?-Text))|", "0"));
	ts << "diff2" << result(it("rel(tup(a:1)) - rel(tup(a:2)) = rel(tup(a:1))", "true"));
	ts << "diff3" << result(it("(rel(tup(a:1, b:\"foo\", c:true)) + rel(tup(a:1, b:\"bar\", c:true))) - rel(tup(a:1, b:\"foo\", c:true)) = rel(tup(a:1, b:\"bar\", c:true))", "true"));
	ts << "diff4" << result(it("one - one = zero", "true"));
	ts << "diff5" << result(it("rel(tup(a:1)) + rel(tup(a:2)) - rel(tup(a:1)) = rel(tup(a:2))", "true"));
	ts << "diff6" << result(it("rel(tup(a:1,b:true,c:\"foo\")) - rel(tup(a:1,b:false,c:\"foo\")) = rel(tup(a:1,b:true,c:\"foo\"))", "true"));
	ts << "diff7" << result(it("rel(tup(a:1,b:true,c:\"foo\")) - (rel(tup(a:1,b:true,c:\"foo\")) ? false) = rel(tup(a:1,b:true,c:\"foo\"))", "true"));
	ts << "diff8" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:2)) = (rel(tup(a:1)) + rel(tup(a:3)))", "true"));
	ts << "diff9" << result(it("(rel(tup(a:444)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:2)) = (rel(tup(a:1)) + rel(tup(a:3)))", "false"));
	ts << "diff10" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:3)) = (rel(tup(a:1)) + rel(tup(a:2)))", "true"));
	ts << "diff11" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:3)) = (rel(tup(a:1)) + rel(tup(a:3)))", "false"));
	ts << "diff12" << result(it("(rel(tup(z:\"b\")) + rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) - (rel(tup(z:\"q\")) + rel(tup(z:\"()()()\"))) = rel(tup(z:\"a\")) + rel(tup(z:\"b\"))", "true"));
	ts << "diff13" << result(it("(rel(tup(z:\"b\")) + rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) - (rel(tup(z:\"b\")) + rel(tup(z:\"()()()\"))) = rel(tup(z:\"a\")) + rel(tup(z:\"q\"))", "true"));
	ts << "diff14" << result(it("(rel(tup(z:\"b\")) + rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) - (rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) = rel(tup(z:\"a\"))", "false"));
	ts << "diff15" << result(it("(rel(tup(z:\"b\")) + rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) - (rel(tup(z:\"q\")) + rel(tup(z:\"a\")) + rel(tup(z:\"()()()\"))) = rel(tup(z:\"b\"))", "true"));
	ts << "diff16" << result(it("|rel(tup(a:1,b:true)) - rel(tup(b:true,a:1))|", "0"));
	ts << "diff17" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:2,b:true))|", "3"));
	ts << "diff18" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:1234,b:true))|", "4"));
	ts << "diff19" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:0,b:false))|", "3"));
	ts << "diff20" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(b:false,a:0))|", "3"));
	ts << "diff21" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:99,b:true))|", "3"));
	ts << "diff22" << result(it("|(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:99,b:false))|", "4"));
	ts << "diff_error1" << result(it("rel(tup(a:1)) - rel(tup(b:1))", "", true));
	ts << "diff_error2" << result(it("rel(tup(a:1)) - rel(tup())", "", true));
	ts << "diff_error3" << result(it("rel(tup(a:1)) - rel(tup(a:1,b:2))", "", true));
	ts << "diff_error4" << result(it("rel(tup(a:1)) - rel(tup(a:true))", "", true));
	ts << "diff_error5" << result(it("rel(tup(a:1,b:true,c:1)) - rel(tup(a:1,b:2,c:3))", "", true));
	ts << "diff_error6" << result(it("(rel(tup(a:1,b:true,c:1)) ? false) - rel(tup(a:1,b:2,c:3))", "", true));
	ts << "diff_error7" << result(it("(rel(tup(b:1)) + rel(tup(b:2)) + rel(tup(b:3))) - rel(tup(a:3))", "", true));
	ts << "diff_error8" << result(it("(rel(tup(a:true)) + rel(tup(a:false))) - rel(tup(a:3))", "", true));
	ts << "diff_error9" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:false))", "", true));
	ts << "diff_error10" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(b:3))", "", true));
	ts << "diff_error11" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:\"fo\"))", "", true));
	ts << "diff_error12" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(b:\"fo\"))", "", true));
	ts << "diff_error13" << result(it("(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3))) - rel(tup(a:1,b:2))", "", true));
	ts << "diff_error14" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(c:1,b:true))", "", true));
	ts << "diff_error15" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:1))", "", true));
	ts << "diff_error16" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(b:true))", "", true));
	ts << "diff_error17" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:1,b:true,c:0))", "", true));
	ts << "diff_error18" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup())", "", true));
	ts << "diff_error19" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:true,b:true))", "", true));
	ts << "diff_error20" << result(it("(rel(tup(a:1,b:true)) + rel(tup(a:2,b:true)) + rel(tup(a:0,b:false)) + rel(tup(a:99,b:true))) - rel(tup(a:1,b:\"foo\"))", "", true));
	ts << "join1" << result(it("rel(tup(abe: 4, kat: 5)) * rel(tup(kat: 5, bar: 6)) = rel(tup(abe: 4, kat: 5, bar: 6))", "true"));
	ts << "join2" << result(it("rel(tup(a:1)) * rel(tup(a:1)) = rel(tup(a:1))", "true"));
	ts << "join3" << result(it("|rel(tup(b:1)) * rel(tup(a:1))| = 1", "true"));
	ts << "join4" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)); |a*a|=2", "true"));
	ts << "join5" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)); |a * a[a<-c,b<-d]|=4", "true"));
	ts << "join6" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)); |a * a[a<-c,b<-d] * a[a<-e,b<-f]|=8", "true"));
	ts << "join7" << result(it("|rel(tup(a:1, b:2)) * (rel(tup(c:true, d:\"foo\")) + rel(tup(c:false, d:\"bar\")))|=2", "true"));
	ts << "join8" << result(it("|zero * rel(tup(a:true))| = 0", "true"));
	ts << "join9" << result(it("|one * rel(tup(a:true))| = 1", "true"));
	ts << "join10" << result(it("|rel(tup(a:1, b:2)) * rel(tup(a:3, b:4))| = 0", "true"));
	ts << "join11" << result(it("|(rel(tup(a:1, b:2)) + rel(tup(a:3, b:4))) * rel(tup(c:3, d:4))| = 2", "true"));
	ts << "join12" << result(it("a:=(rel(tup(a:1,b:true,c:\"d\")) * rel(tup(d:1,e:\"1\",f:?-Int))); a |+ a,b,c,d,e,f = a", "true"));
	ts << "join13" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)); b := rel(tup(b:2, c:1)) + rel(tup(b:2, c:3)); a * b = rel(tup(a:1,b:2,c:1)) + rel(tup(a:1,b:2,c:3))", "true"));
	ts << "join14" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)); b := rel(tup(b:2, c:1)) + rel(tup(b:2, c:3)); (a*b) = (b*a)", "true"));
	ts << "join15" << result(it("a := rel(tup(a:1,b:2)) + rel(tup(a:3,b:4)) + rel(tup(a:5,b:999)) + rel(tup(a:6,b:12121)); b := rel(tup(b:2, c:1)) + rel(tup(b:2, c:3)); a * b = rel(tup(a:1,b:2,c:1)) + rel(tup(a:1,b:2,c:3))", "true"));
	ts << "join16" << result(it("a := rel(tup(b:2,a:1)) + rel(tup(b:4,a:3)); b := rel(tup(b:2, c:1)) + rel(tup(b:2, c:3)); a * b = rel(tup(a:1,b:2,c:1)) + rel(tup(a:1,b:2,c:3))", "true"));
	ts << "join17" << result(it("a := rel(tup(b:2,a:1)) + rel(tup(b:4,a:3)); b := rel(tup(c:1, b:2)) + rel(tup(c:3, b:2)); a * b = rel(tup(b:2,a:1,c:3)) + rel(tup(c:1,b:2,a:1))", "true"));
	ts << "join18" << result(it(""
"X := rel(tup(d:5,e:1,b:1,a:1,c:5)) + rel(tup(d:0,e:0,b:0,a:99,c:0)) + rel(tup(d:2,e:42,b:42,a:42,c:8)) + rel(tup(d:9,e:42,b:0,a:0,c:0)) + rel(tup(d:99,e:99,b:99,a:78,c:99));"
"Y := rel(tup(d:2,f:1,c:8)) + rel(tup(d:9,f:783,c:0)) + rel(tup(d:2,f:0,c:8)) + rel(tup(d:5,f:17,c:5)) + rel(tup(d:99,f:99,c:78));"
"R := rel(tup(a:0,b:0,c:0,d:9,e:42,f:783)) + rel(tup(a:1,b:1,c:5,d:5,e:1,f:17)) + rel(tup(a:42,b:42,c:8,d:2,e:42,f:0)) + rel(tup(a:42,b:42,c:8,d:2,e:42,f:1));"
"X * Y = R"
"", "true"));
	ts << "join_error1" << result(it("rel(tup(a:1, b:2)) * rel(tup(a:true, d:\"foo\"))", "", true));
	ts << "join_error2" << result(it("rel(tup(a:true)) * rel(tup(a:\"foo\"))", "", true));
	ts << "join_error3" << result(it(""
"X := rel(tup(c:1, a:true, b:\"foo\")) + rel(tup(c:2, a:false, b:\"bar\"));"
"Y := rel(tup(c:2, a:true, b:true));"
"X * Y", "", true));
	ts << "join_error4" << result(it(""
"X := rel(tup(c:1, a:true, b:\"foo\")) + rel(tup(c:2, a:false, b:\"bar\"));"
"Y := rel(tup(c:2, a:1, b:\"foo\"));"
"X * Y", "", true));
	ts << "join_error5" << result(it(""
"X := rel(tup(c:1, a:true, b:\"foo\")) + rel(tup(c:2, a:false, b:\"bar\"));"
"Y := rel(tup(c:true, a:true, b:\"foo\"));"
"X * Y"
"", "", true));
	ts << "join_error6" << result(it(""
"X := rel(tup(c:1, a:true, b:\"foo\")) + rel(tup(c:2, a:false, b:\"bar\"));"
"Y := (rel(tup(c:true, a:true, b:\"foo\")) ? false);"
"X * Y", "", true));
	ts << "join_error7" << result(it(""
"X := rel(tup(c:1, a:1, b:\"foo\")) + rel(tup(c:2, a:2, b:\"bar\"));"
"Y := rel(tup(c:2, a:true, b:\"foo\"));"
"X * Y", "", true));
	ts << "join_error8" << result(it(""
"X := rel(tup(c:true, a:true, b:\"foo\")) + rel(tup(c:false, a:false, b:\"bar\"));"
"Y := rel(tup(c:2, a:true, b:\"foo\"));"
"X * Y", "", true));
	ts << "join_error9" << result(it(""
"X := rel(tup(q:1,c:1, a:true, b:\"foo\")) + rel(tup(c:2, a:false,q:2, b:\"bar\"));"
"Y := rel(tup(c:2, q:\"text\",a:true, b:\"foo\"));"
"X * Y", "", true));
	ts << "join_error10" << result(it(""
"X := rel(tup(d:\"foo\", c:1, a:true, b:\"foo\")) + rel(tup(d:\"bar\", c:2, a:false, b:\"bar\"));"
"Y := rel(tup(c:2, a:true, b:\"foo\", d:99));"
"X * Y", "", true));
	ts << "join_error11" << result(it(""
"X := rel(tup(c:1, a:true, b:\"foo\", d:111)) + rel(tup(c:2, a:false, b:\"bar\", d:234));"
"Y := rel(tup(d:true, c:2, a:true, b:\"foo\"));"
"X * Y", "", true));
	ts << "select1" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.kat=5)|", "1"));
	ts << "select2" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.kat=4)|", "0"));
	ts << "select3" << result(it("(one ? true) = one", "true"));
	ts << "select4" << result(it("(one ? false) = zero", "true"));
	ts << "select5" << result(it("|rel(tup(a:1, b:2)) ? (#.a < #.b)|", "1"));
	ts << "select6" << result(it("|rel(tup(a:1, b:2)) ? (#.a > #.b)|", "0"));
	ts << "select7" << result(it("|rel(tup(a:\"foo\", b:\"foo\")) ? (#.a = #.b)|", "1"));
	ts << "select8" << result(it("|rel(tup(a:\"foo\", b:\"bar\")) ? (#.a = #.b)|", "0"));
	ts << "select9" << result(it("|rel(tup(a:true, b:true)) ? (#.a = #.b)|", "1"));
	ts << "select10" << result(it("|rel(tup(a:true, b:false)) ? (#.a = #.b)|", "0"));
	ts << "select11" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
"|X ? (#.a = 1)| = 2", "true"));
	ts << "select12" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
"|X ? (#.a > #.b)| = 2", "true"));
	ts << "select13" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
"|X ? (rel(#) = rel(tup(a:8,b:1,c:3)))| = 1", "true"));
	ts << "select14" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
"|X ? (#.b + #.a*2 = 4)| = 2", "true"));
	ts << "select15" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
								 "|X ? (tup(z: #.a) = tup(z:1))| = 2", "true"));
	ts << "select16" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
"|X ? (min(rel(#), a) = 1)| = 2", "true"));
	ts << "select17" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
								  "|X ? (# << # = #)| = 5", "true"));
	ts << "select18" << result(it("X := one; ( X ? (X = one) ) = one", "true"));
	ts << "select_err1" << result(it("rel(tup(a:1)) ? (#.b = 1)", "", true));
	ts << "select_err2" << result(it("rel(tup(a:1, b:\"foo\")) ? (#.b = #.a)", "", true));
	ts << "select_err3" << result(it("rel(tup(a:true, b:2)) ? (#.a > #.b)", "", true));
	ts << "select_err4" << result(it("rel(tup(a:true, b:2)) ? (#.c = 1)", "", true));
	ts << "select_err5" << result(it("rel(tup(a:true, b:2)) ? (#.a = #.no)", "", true));
	ts << "select_err6" << result(it("rel(tup(a:true, b:2)) ? (rel(tup()))", "", true));
	ts << "select_err7" << result(it("rel(tup(a:true, b:2)) ? \"foo\"", "", true));
	ts << "select_err8" << result(it("rel(tup(a:true, b:2)) ? (tup(#) = tup())", "", true));
	ts << "select_err9" << result(it("rel(tup(a:true, b:2)) ? (# = true)", "", true));
	ts << "select_err10" << result(it("rel(tup(a:true, b:2)) ? (# + # = true)", "", true));
	ts << "select_err11" << result(it("rel(tup(a:true, b:2)) ? (rel(#) + rel(tup(q:1,z:3)) = rel(tup()) )", "", true));
	ts << "select_err12" << result(it("rel(tup(a:true, b:2)) ? (rel(#) + rel(tup()) = rel(tup()))", "", true));
	ts << "select_err13" << result(it("rel(tup(a:true, b:2)) ? (rel(#) + rel(tup(a:true, b:2, c:1)) = rel(tup()) )", "", true));
	ts << "select_err14" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.kat=true)|", "", true));
	ts << "select_err15" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.kat=\"5\")|", "", true));
	ts << "select_err16" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.kat='5')|", "", true));
	ts << "select_err17" << result(it("|rel(tup(abe: 4, kat:5)) ? (#.hat)|", "", true));
	ts << "select_err18" << result(it("|rel(tup(abe: 4, kat:5)) ? #|", "", true));
	ts << "select_err19" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
									 "|X ? (#.b = true)| = 5", "", true));
	ts << "select_err20" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
									 "|X ? (# + tup(q:1) = tup())| = 5", "", true));
	ts << "select_err21" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
									 "|X ? (X ? (X ? (1)))| = 5", "", true));
	ts << "select_err22" << result(it("X := rel(tup(a:1,b:2,c:3)) + rel(tup(a:1,b:2,c:4)) + rel(tup(a:0,b:2,c:3)) + rel(tup(a:8,b:1,c:3)) + rel(tup(a:5,b:3,c:2));"
									 "|X ? (# # #)| = 5", "", true));
	ts << "pos_project1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |+ abe,baz = rel(tup(abe: 4, baz:2))", "true"));
	ts << "pos_project2" << result(it("(rel(tup(abe: 4, kat:5, baz:2)) + rel(tup(abe:4, kat:5, baz:3))) |+ abe,kat = rel(tup(abe: 4, kat:5))", "true"));
	ts << "pos_project3" << result(it("rel(tup(a:1,b:2,c:3,d:true,e:\"\",f:1)) |+ a = rel(tup(a:1))", "true"));
	ts << "pos_project4" << result(it("rel(tup(a:1,b:2,c:3,d:true,e:\"\",f:1)) |+ b,a = rel(tup(a:1,b:2))", "true"));
	ts << "pos_project5" << result(it("rel(tup(a:1,b:2,c:3,d:true,e:\"\",f:1)) |+ e,a,f = rel(tup(a:1,e:\"\",f:1))", "true"));
	ts << "pos_project6" << result(it("rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)) |+ b,c,a = rel(tup(a:true,b:1,c:\"\"))", "true"));
	ts << "pos_project7" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ a,b,c,d,e,f) = X", "true"));
	ts << "pos_project8" << result(it(""
"X := rel(tup(b:16, a:true, c:\"\", d:42)) + rel(tup(b:16, a:true, c:\"\", d:43));"
"|X |+ a| = 1", "true"));
	ts << "pos_project9" << result(it(""
"X := rel(tup(b:16, a:true, c:\"\", d:42)) + rel(tup(b:16, a:true, c:\"\", d:43));"
"|X |+ b,a| = 1", "true"));
	ts << "pos_project10" << result(it(""
"X := rel(tup(b:16, a:true, c:\"\", d:42)) + rel(tup(b:16, a:true, c:\"\", d:43));"
"(X |+ a,b) = (X |+ b,a)", "true"));
	ts << "pos_project11" << result(it(""
"X := rel(tup(b:16, a:true, c:\"\", d:42)) + rel(tup(b:16, a:true, c:\"\", d:43));"
"|X |+ a,b,c| = 1", "true"));
	ts << "pos_project12" << result(it(""
"X := rel(tup(b:16, a:true, c:\"\", d:42)) + rel(tup(b:16, a:true, c:\"\", d:43));"
"|X |+ d,a| = 2", "true"));
	ts << "pos_project_error13" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ a,b,c,d,d,e) = X |+ a,b,c,d,e", "true"));
	ts << "pos_project_error14" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ f,e,d,c,b,b) = X |+ b,c,d,e,f", "true"));

	ts << "pos_project_error1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |+ boo", "", true));
	ts << "pos_project_error2" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |+ abe, kat, baz, boo", "", true));
	ts << "pos_project_error3" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ a,b,z,c,d) = X", "", true));
	ts << "pos_project_error4" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ f,g) = X", "", true));
	ts << "pos_project_error5" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ true) = X", "", true));
	ts << "pos_project_error6" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ \"a\") = X", "", true));
	ts << "pos_project_error7" << result(it("X := rel(tup()); (X |+ a) = X", "", true));
	ts << "pos_project_error8" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ rel(tup())) = X", "", true));
	ts << "pos_project_error9" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ tup()) = X", "", true));
	ts << "pos_project_error10" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ 1) = X", "", true));
	ts << "pos_project_error11" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1)); (X |+ \"a\") = X", "", true));
	ts << "neg_project1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |- abe,baz = rel(tup(kat: 5))", "true"));
	ts << "neg_project2" << result(it("(rel(tup(a:1, b:1)) + rel(tup(a:1, b:2))) |- b = rel(tup(a: 1))", "true"));
	ts << "neg_project3" << result(it("|(rel(tup(a:1, b:1)) + rel(tup(a:1, b:2))) |- a| = 2", "true"));
	ts << "neg_project4" << result(it("rel(tup(abe: 4, kat:5, baz:2)) |- abe,baz,kat = one", "true"));
	ts << "neg_project5" << result(it("one |- a,b,c,d,e = one", "true"));
	ts << "neg_project6" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1));"
"(X |- a,b,c,d,e,f) = one", "true"));
	ts << "neg_project7" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1));"
"(X |- a,b,e,f) = (X |+ c,d)", "true"));
	ts << "neg_project8" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1));"
"(X |- a,b,c) = (X |+ f,e,d)", "true"));
	ts << "neg_project9" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1));"
"(X |- f,a,c,e) = (X |- c,a,f,e)", "true"));
	ts << "neg_project10" << result(it("X := rel(tup(e:1,d:2,f:3,a:true,c:\"\",b:1));"
"(X |- f,a,c,e,q,w) = (X |- c,a,f,e)", "true"));
	ts << "neg_project11" << result(it("X := rel(tup(a:1,b:1,c:2,d:3,f:4));"
"X |- a,b,a = rel(tup(c:2,d:3,f:4))", "true"));
	ts << "neg_project12" << result(it("X := rel(tup(a:1,b:1,c:2,d:3,f:4));"
"X |- a,b,a,c,a,d = rel(tup(f:4))", "true"));
	ts << "neg_project13" << result(it("X := rel(tup(a:1,b:1,c:2,d:3,f:4));"
"X |- a,a,a,a,a = rel(tup(b:1,c:2,d:3,f:4))", "true"));
	ts << "neg_project14" << result(it("X := rel(tup(a:1,b:1,c:2,d:3,f:4));"
"X |- b,b,b,b,a,b = rel(tup(c:2,d:3,f:4))", "true"));
	ts << "neg_project15" << result(it("X := rel(tup(a:3,b:\"\",c:true)) + rel(tup(a:1,b:\"\",c:true)) + rel(tup(a:2, b:\"\", c:true));"
"| X |- b,c | = 3", "true"));
	ts << "neg_project16" << result(it("X := rel(tup(a:3,b:\"\",c:true)) + rel(tup(a:1,b:\"\",c:true)) + rel(tup(a:2, b:\"\", c:true));"
"| X |- a | = 1", "true"));
	ts << "neg_project17" << result(it("X := rel(tup(a:3,b:\"\",c:true)) + rel(tup(a:1,b:\"\",c:true)) + rel(tup(a:2, b:\"\", c:true));"
"| X |- b,c,a,b,a,c,q | = 1", "true"));
	ts << "neg_project18" << result(it("X := rel(tup(a:3,b:\"\",c:true)) + rel(tup(a:1,b:\"\",c:true)) + rel(tup(a:2, b:\"\", c:true));"
"| X |- c,a | = 1", "true"));
	ts << "rename1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-foo, kat<-taz] = rel(tup(foo: 4, taz:5, baz:2))", "true"));
	ts << "rename2" << result(it("rel(tup(abe: 4, kat:5)) [kat<-foo, abe<-bar] = rel(tup(foo: 5, bar:4))", "true"));
	ts << "rename3" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"Y := rel(tup(e:2,foo:1,bar:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,foo:2,bar:true,c:\"\",d:1,aa:true));"
"X[a<-foo, b<-bar] = Y", "true"));
	ts << "rename4" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-a] = X", "true"));
	ts << "rename5" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"(X[a<-foo])[foo<-a] = X", "true"));
	ts << "rename6" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"((X[e<-foo, a<-bar])[foo<-e])[bar<-a] = X", "true"));
	ts << "rename_error1" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [foo<-bar]", "", true));
	ts << "rename_error2" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [kat<-abe]", "", true));
	ts << "rename_error3" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-foo, baz<-foo, kat<-taz]", "", true));
	ts << "rename_error4" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [nope<-no]", "", true));
	ts << "rename_error5" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [nope<-abe]", "", true));
	ts << "rename_error6" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-nope, nope<-kat]", "", true));
	ts << "rename_error7" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-true]", "", true));
	ts << "rename_error8" << result(it("rel(tup(abe: 4, kat:5, baz:2)) [abe<-kat]", "", true));
	ts << "rename_error9" << result(it("(rel(tup(abe: 4, kat:5, baz:2)) [abe<-foo])[foo<-kat]", "", true));

	ts << "rename_error10" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-e]", "", true));
	ts << "rename_error11" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[e<-a]", "", true));
	ts << "rename_error12" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[e<-1234]", "", true));
	ts << "rename_error13" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-true]", "", true));
	ts << "rename_error14" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-\"foo\"]", "", true));
	ts << "rename_error15" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-q,e<-q]", "", true));
	ts << "rename_error16" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[a<-foo,b<-foo]", "", true));
	ts << "rename_error17" << result(it(""
"X := rel(tup(e:2,a:1,b:true,c:\"\",d:1,aa:true)) + rel(tup(e:2,a:2,b:true,c:\"\",d:1,aa:true));"
"X[c<-bla,a<-good,b<-goodtoo,d<-yes,c<-blaz]", "", true));
	ts << "rename_error18" << result(it("rel(tup(a:1,b:2,c:3))[a<-aa,b<-bb,c<-cc,d<-dd]", "", true));
	ts << "rename_error19" << result(it("rel(tup(a:1,b:2,c:3))[b<-aa,a<-bb,c<-cc,d<-dd]", "", true));
	ts << "rename_error20" << result(it("rel(tup(a:1,b:2,c:3))[a<-aa,b<-bb,c<-cc,a<-ee]", "", true));
	ts << "min1" << result(it("min(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "4"));
	ts << "min2" << result(it("min(rel(tup(abe: 4)) + rel(tup(abe: 0)), abe)", "0"));
	ts << "min3" << result(it("min(rel(tup(abe:4))-rel(tup(abe:4)), abe)", "?-Int"));
	ts << "min4" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:0));"
"min(X, a)", "0"));
	ts << "min5" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:13));"
"min(X, b)", "3"));
	ts << "min6" << result(it("min(rel(tup(abe: ?-Int)) + rel(tup(abe: 5)), abe)", "5"));
	ts << "min7" << result(it("min(rel(tup(abe: ?-Int)), abe)", "?-Int"));
	ts << "min_error1" << result(it("min(zero, kat)", "", true));
	ts << "min_error2" << result(it("min(rel(tup(foo:1)), kat)", "", true));
	ts << "min_error3" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:0));"
									"min(X, c)", "", true));
	ts << "min_error4" << result(it("min(rel(tup(a:\"fo\")), a)", "", true));
	ts << "max1" << result(it("max(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "5"));
	ts << "max2" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:0));"
"max(X, a)", "99"));
	ts << "max3" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:0));"
"max(X, b)", "99"));
	ts << "max4" << result(it("max(rel(tup(abe: ?-Int)) + rel(tup(abe: 5)), abe)", "5"));
	ts << "max5" << result(it("max(rel(tup(abe: ?-Int)), abe)", "?-Int"));
	ts << "max6" << result(it("max(rel(tup(abe: ?-Int)) + rel(tup(abe: 0)), abe)", "0"));
	ts << "max_error1" << result(it("max(zero, kat)", "", true));
	ts << "max_error2" << result(it("max(rel(tup(foo:1)), kat)", "", true));
	ts << "add1" << result(it("add(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "9"));
	ts << "add2" << result(it("X := rel(tup(a:1,b:3)) + rel(tup(a:1,b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:1, b:5)) + rel(tup(a:0,b:10)) + rel(tup(a:99,b:99)) + rel(tup(a:0,b:0));"
"add(X, a)", "102"));
	ts << "add_error1" << result(it("add(zero, kat)", "", true));
	ts << "add_error2" << result(it("add(rel(tup(foo:1)), kat)", "", true));
	ts << "add_error3" << result(it("add(rel(tup(foo:\"2\")), foo)", "", true));
	ts << "add_error4" << result(it("add(rel(tup(foo:true)), foo)", "", true));
	ts << "mult1" << result(it("mult(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "20"));
	ts << "mult2" << result(it("mult(rel(tup(abe: 0)) + rel(tup(abe: 5)), abe)", "0"));
	ts << "mult3" << result(it("X := rel(tup(a:1,b:1)) + rel(tup(a:1,b:2)) + rel(tup(a:0,b:3)) + rel(tup(a:1, b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:99,b:6)) + rel(tup(a:0,b:7));"
"mult(X, b)", "5040"));
	ts << "mult_error1" << result(it("mult(zero, kat)", "", true));
	ts << "mult_error2" << result(it("mult(rel(tup(foo:1)), kat)", "", true));
	ts << "mult_error3" << result(it("mult(rel(tup(foo:true)), foo)", "", true));
	ts << "mult_error4" << result(it("mult(rel(tup(foo:\"\")), foo)", "", true));
	ts << "count1" << result(it("count(rel(tup(abe: 4)) + rel(tup(abe: 5)), abe)", "2"));
	ts << "count2" << result(it("count(rel(tup(abe: ?-Int)) + rel(tup(abe: 5)), abe)", "1"));
	ts << "count3" << result(it("count(rel(tup(abe: ?-Text)) + rel(tup(abe: \"foo\")), abe)", "1"));
	ts << "count4" << result(it("count(rel(tup(abe: ?-Text)), abe)", "0"));
	ts << "count5" << result(it("X := rel(tup(a:1,b:1)) + rel(tup(a:1,b:2)) + rel(tup(a:0,b:3)) + rel(tup(a:1, b:4)) + rel(tup(a:0,b:5)) + rel(tup(a:99,b:6)) + rel(tup(a:0,b:7));"
"count(X, a)", "7"));
	ts << "count6" << result(it(""
"X := rel(tup(b:1,a:2,c:3)) + rel(tup(b:0,a:2,c:5)) + rel(tup(b:99,a:2,c:3)) + rel(tup(b:0,a:1,c:1));"
"count(X, a)", "4"));
	ts << "count7" << result(it(""
"X := rel(tup(b:1,a:2,c:3)) + rel(tup(b:0,a:2,c:5)) + rel(tup(b:99,a:2,c:3)) + rel(tup(b:0,a:1,c:1));"
"count(X, a)", "4"));
	ts << "count8" << result(it(""
"X := rel(tup(b:1,a:2,c:3)) + rel(tup(b:0,a:?-Int,c:5)) + rel(tup(b:99,a:?-Int,c:3)) + rel(tup(b:0,a:1,c:1));"
"count(X, a)", "2"));
	ts << "count_err1" << result(it("count(rel(tup()), abe)", "", true));
	ts << "count_err2" << result(it("count(rel(tup(c:1,a:2,b:3)), d)", "", true));
	ts << "factor1" << result(it(""
"X := rel(tup(a: 1, b:1)) + rel(tup(a: 2, b:3)) + rel(tup(a: 2, b:4)) + rel(tup(a: 7, b:2)); "
"Y := rel(tup(a: 1, c:4)) + rel(tup(a: 2, c:2)) + rel(tup(a: 2, c:7)) + rel(tup(a: 3, c:9));"
"Z := rel(tup(a: 1, b: 1, c: 4)) + rel(tup(a: 2, b: 7, c: 9)) + rel(tup(a: 3, b: 0, c: 9)) + rel(tup(a: 7, b: 2, c: 0));"
"(!(X, Y)|a : rel(# << tup(b: add(@(1), b), c: add(@(2), c)))) = Z"
"", "true"));
	ts << "factor2" << result(it(""
"X := rel(tup(a:1, b:2)) + rel(tup(a:2, b:3)) + rel(tup(a:2, b:1)) + rel(tup(a:3, b:4));"
"Y := rel(tup(a:1, b:2)) + rel(tup(a:1, b:3)) + rel(tup(a:2, b:3)) + rel(tup(a:7, b:3));"
"Z := rel(tup(a:7, c:11)) + rel(tup(a:9, c:2));"
"R := rel(tup(a:1, m:2, n:5, o: 0)) + rel(tup(a:2, m:4, n:3, o: 0)) + rel(tup(a:3, m:4, n:0, o: 0)) + rel(tup(a:7, m:0, n:3, o: 11)) + rel(tup(a:9, m:0, n:0, o: 2));"
"(!(X, Y, Z)|a : rel(# << tup(m: add(@(1), b), n: add(@(2), b), o: add(@(3), c)))) = R"
								 "", "true"));
	ts << "factor3" << result(it(""
"X :=  rel(tup(a:1,b:2,c:1)) + rel(tup(a:1,b:3,c:2));"
"Y :=  rel(tup(c:1,b:1,a:1)) + rel(tup(c:2,b:3,a:1));"
"Z :=  rel(tup(a:1,b:2,c:1)) ? false;"
"R :=  rel(tup(a:1,b:1,p:0,q:1)) + rel(tup(a:1,b:2,p:1,q:0)) + rel(tup(a:1,b:3,p:2,q:2));"
"(!(X,Y,Z)|b,a : rel(# << tup(p: add(@(1), c), q:add(@(2), c)))) = R"
								 "", "true"));
	ts << "factor4" << result(it("(!(rel(tup(a:1)))|a : one) = one", "true"));
	ts << "factor5" << result(it(""
								 "X := rel(tup(a: 1, b:1)) + rel(tup(a: 2, b:3)) + rel(tup(a: 2, b:4)) + rel(tup(a: 7, b:2));"
								 "(!(X)|a : one) = one", "true"));
	ts << "factor6" << result(it("boo := rel(tup(a:1)) ? false; (!(boo)|a : rel(tup(a:1))) = zero", "true"));
	ts << "factor7" << result(it("empty := rel(tup(a:0, b:0)) ? false; (!(empty, empty, rel(tup(a:1, b:2)), empty, rel(tup(a:1, c:4, b:2)), empty)|a,b : rel(tup(q:#.a))) = rel(tup(q:1))", "true"));
	ts << "factor8" << result(it("(!(rel(tup(a:2)))|a : @(1) ) = one", "true"));
	ts << "factor9" << result(it(""
"empty := rel(tup(a:1, b:2)) ? false;"
"(R := !(empty, empty, rel(tup(a:1, b:2)), empty, rel(tup(a:1, c:4, b:3)), empty)|a,b : @(1));"
"R = zero"
								"", "true"));
	ts << "factor10" << result(it(""
"empty := rel(tup(a:1, b:2)) ? false;"
"(R := !(empty, empty, rel(tup(a:1, b:2)), empty, rel(tup(a:1, c:4, b:3)), empty)|a : @(3));"
"R = rel(tup(b:2))"
								"", "true"));
	ts << "factor11" << result(it(""
"empty := rel(tup(a:1, b:2)) ? false;"
"(R := !(empty, empty, rel(tup(a:1, b:2)), empty, rel(tup(a:1, c:4, b:3)), empty)|a : rel(#));"
"R = rel(tup(a:1))"
								"", "true"));
	ts << "factor12" << result(it(""
"empty := rel(tup(a:1, b:2)) ? false;"
"(R := !(empty, empty, rel(tup(a:1, b:2)), empty, rel(tup(a:1, c:4, b:3)), empty)|a,b : rel(#));"
"R = rel(tup(a:1, b:2)) + rel(tup(a:1, b:3))"
								"", "true"));
	ts << "factor13" << result(it(""
"(X := rel(tup(a:\"foo\", b: true)) + rel(tup(a:\"bar\", b:true)));"
"(Y := rel(tup(a:\"bar\", b: true)) + rel(tup(a:\"bar\", b:false)));"
"(R := !(X, Y)|a,b : rel(#));"
"|R| = 3"
								"", "true"));
	ts << "factor_error1" << result(it("!(rel(tup(a:1)))|a,b : rel(tup())", "", true));
	ts << "factor_error2" << result(it("!(rel(tup(a:1, b:2)))|b,b : rel(tup())", "", true));
	ts << "factor_error3" << result(it("!(rel(tup(a:1, b:2)), rel(tup(a:2)))|a,b : rel(tup())", "", true));
	ts << "factor_error4" << result(it("!(rel(tup(a:1, b:2)), rel(tup(a:2)))|b : rel(tup())", "", true));
	ts << "factor_error5" << result(it("!(rel(tup(a:1, b:2)), rel(tup(a:2)))|b,a : rel(tup())", "", true));
	ts << "factor_error6" << result(it("!(rel(tup(a:1, b:2)), rel(tup(a:2)))|a,a : rel(tup())", "", true));
	ts << "factor_error7" << result(it("!(rel(tup(a:1,b:2,c:3)))|1 : rel(tup())", "", true));
	ts << "factor_error8" << result(it("!(rel(tup(a:1)))|c,b,d : rel(tup())", "", true));
	ts << "factor_error9" << result(it("!(rel(tup(b:2,c:3,a:1)))|b,c,a,o : rel(tup())", "", true));
	ts << "factor_error10" << result(it("!(rel(tup()))|a : rel(tup())", "", true));
	ts << "factor_error11" << result(it("!(rel(tup(a:1,b:2,c:3)))|a,b,c,b : rel(tup())", "", true));
	ts << "factor_error12" << result(it("!(rel(tup(a:1,b:2,c:3)))|b,a,b : rel(tup())", "", true));
	ts << "factor_error13" << result(it("!(rel(tup(a:1,b:2,c:3)))|true : rel(tup())", "", true));
	ts << "factor_error14" << result(it("!(rel(tup(a:1,b:2,c:3)))|\"a\" : rel(tup())", "", true));
	ts << "factor_error15" << result(it("!(rel(tup(a:1,b:2,c:3)))|rel(tup()) : rel(tup())", "", true));
	ts << "factor_error16" << result(it("X := rel(tup(a:1)); Y := rel(tup(a:true)); !(X,Y)|a : rel(tup())", "", true));
	ts << "forall1" << result(it("|!(rel(tup(a:1)) + rel(tup(a:2)) + rel(tup(a:3)) + rel(tup(a:4)) ) : rel(#)| = 4", "true"));
	ts << "forall2" << result(it("(!(rel(tup())) : rel(tup())) = one", "true"));
	ts << "forall3" << result(it(""
"X := rel(tup(a:1, b:2)) + rel(tup(a:2, b:2)) + rel(tup(a:0, b:3)); "
"( !(X) : rel(tup(p: 2*#.a + #.b)) ) = rel(tup(p:3)) + rel(tup(p:4)) + rel(tup(p:6))"
								 "", "true"));
	ts << "forall4" << result(it("X := rel(tup(a:1, b:2)) + rel(tup(a:2, b:2)) + rel(tup(a:0, b:3)); "
								 "( !(X) : rel(#) ) = X", "true"));
	ts << "forall5" << result(it("(!(rel(tup(b:1,c:0))) : rel(tup(a:#.b/#.c))) = rel(tup(a:?-Int))", "true"));
	ts << "forall_error1" << result(it("!(rel(tup(a:1)), rel(tup(a:2))) : rel(tup())", "", true));
	ts << "forall_error2" << result(it("!() : rel(tup())", "", true));
	ts << "forall_error3" << result(it("!(true) : rel(tup())", "", true));
	ts << "forall_error4" << result(it("!(1) : rel(tup())", "", true));
	ts << "forall_error5" << result(it("!(\"foo\") : rel(tup())", "", true));
	ts << "forall_error6" << result(it("!(rel(tup(a:1))) : rel(tup(b:#.b))", "", true));
	ts << "forall_error7" << result(it("!(rel(tup(a:1))) : true", "", true));
	ts << "forall_error8" << result(it("!(rel(tup(a:1))) : rel(a:1)", "", true));
	ts << "forall_error9" << result(it("!(rel(tup(a:1))) : \"foo\"", "", true));
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
		.multi_test(builtins, "builtins")
		.multi_test(base, "base")
		.multi_test(integer, "integer")
		.multi_test(text, "text")
		.multi_test(bools, "bool")
		.multi_test(tuple, "tuple")
		.multi_test(relation, "relation")
		.multi_test(ifs, "if")
		.multi_test(crash, "crash")
		.test(comments, "comments")
		.test(relation_external, "relation_external");
}
