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
#include <frontend/lexer.hh>
#include <frontend/code.hh>
using namespace rasmus;
using namespace rasmus::frontend;

using namespace lexer;

template <typename T>
void print(std::ostream & o, T begin, T end) {
	o << "[";
	for (T i=begin; i != end; ++i) {
		if (i != end) o << ", ";
		o << *i;
	}
	o << "]";
}

bool lt(const char * txt, std::initializer_list<TokenType> exp) {
	std::shared_ptr<Code> code = std::make_shared<Code>(txt, "monkey");
	Lexer l(code, 0);
	
	std::vector<TokenType> lst;
	while (true) {
		rasmus::frontend::Token t = l.getNext();
		if (t.id == TokenType::END_OF_FILE) break;;
		lst.push_back(t.id);
	}
	if (lst!=std::vector<TokenType>(exp)) {
		print(log_error(), lst.begin(), lst.end());
		log_error() << " != ";
		print(log_error(), exp.begin(), exp.end());
		log_error() << std::endl;
	}

	return lst==std::vector<TokenType>(exp);
}


void base(rasmus::teststream & ts) {
    ts << "tup" << result(lt("tup", {TokenType::TK_TUP}));
    ts << "rel" << result(lt("rel", {TokenType::TK_REL}));
    ts << "func" << result(lt("func", {TokenType::TK_FUNC}));
    ts << "end" << result(lt("end", {TokenType::TK_END}));
    ts << "at" << result(lt("@(2)", {TokenType::TK_AT}));
    ts << "at2" << result(lt("@(31)", {TokenType::TK_AT}));
    ts << "sharp" << result(lt("#", {TokenType::TK_SHARP}));
    ts << "not" << result(lt("not nott", {TokenType::TK_NOT, TokenType::TK_NAME}));
    ts << "minus" << result(lt("-", {TokenType::TK_MINUS}));
    ts << "and" << result(lt("and", {TokenType::TK_AND}));
    ts << "or" << result(lt("or", {TokenType::TK_OR}));
    ts << "truefalse" << result(lt("false-true truee", {TokenType::TK_FALSE, TokenType::TK_MINUS, TokenType::TK_TRUE, TokenType::TK_NAME}));
    ts << "paren" << result(lt("((++ )+)", {TokenType::TK_LPAREN, TokenType::TK_BLOCKSTART, TokenType::TK_PLUS, TokenType::TK_RPAREN, TokenType::TK_BLOCKEND}));
    ts << "val" << result(lt("val", {TokenType::TK_VAL}));
    ts << "in" << result(lt("in", {TokenType::TK_IN}));
    ts << "name" << result(lt("namee", {TokenType::TK_NAME}));
    ts << "colon" << result(lt(":", {TokenType::TK_COLON}));
    ts << "int" << result(lt("1234", {TokenType::TK_INT}));
    ts << "text" << result(lt("\"hello\"", {TokenType::TK_TEXT}));
    ts << "text2" << result(lt("\"hel\\\"lo\"", {TokenType::TK_TEXT}));
    ts << "zero" << result(lt("zero", {TokenType::TK_ZERO}));
    ts << "one" << result(lt("one", {TokenType::TK_ONE}));
    ts << "stdbool" << result(lt("?-Bool", {TokenType::TK_STDBOOL}));
    ts << "stdint" << result(lt("?-Int", {TokenType::TK_STDINT}));
    ts << "stdtext" << result(lt("?-Text", {TokenType::TK_STDTEXT}));
    ts << "comma" << result(lt(",", {TokenType::TK_COMMA}));
    ts << "arrow" << result(lt("--> <-", {TokenType::TK_MINUS, TokenType::TK_RIGHTARROW, TokenType::TK_LEFT_ARROW}));
    ts << "type_bool" << result(lt("Bool", {TokenType::TK_TYPE_BOOL}));
    ts << "type_int" << result(lt("Int", {TokenType::TK_TYPE_INT}));
    ts << "type_text" << result(lt("Text", {TokenType::TK_TYPE_TEXT}));
    ts << "type_atom" << result(lt("Atom", {TokenType::TK_TYPE_ATOM}));
    ts << "type_tup" << result(lt("Tup", {TokenType::TK_TYPE_TUP}));
    ts << "type_rel" << result(lt("Rel", {TokenType::TK_TYPE_REL}));
    ts << "type_func" << result(lt("Func", {TokenType::TK_TYPE_FUNC}));
    ts << "type_any" << result(lt("Any", {TokenType::TK_TYPE_ANY}));
    ts << "operators" << result(lt("+*/++-\\", {TokenType::TK_PLUS, TokenType::TK_MUL, TokenType::TK_DIV, TokenType::TK_CONCAT, TokenType::TK_MINUS, TokenType::TK_SET_MINUS}));
    ts << "mod" << result(lt("mod", {TokenType::TK_MOD}));
    ts << "comp" << result(lt("< <> <= >= = >", {TokenType::TK_LESS, TokenType::TK_DIFFERENT, TokenType::TK_LESSEQUAL, TokenType::TK_GREATEREQUAL, TokenType::TK_EQUAL, TokenType::TK_GREATER}));
    ts << "semicolon" << result(lt(";", {TokenType::TK_SEMICOLON}));
    ts << "pipe" << result(lt("|", {TokenType::TK_PIPE}));
    ts << "dot" << result(lt("...", {TokenType::TK_TWO_DOTS, TokenType::TK_ONE_DOT}));
    ts << "select" << result(lt("?(", {TokenType::TK_SELECT}));
    ts << "project" << result(lt("||+|--", {TokenType::TK_PIPE, TokenType::TK_PROJECT_PLUS, TokenType::TK_PROJECT_MINUS, TokenType::TK_MINUS}));
    ts << "bracket" << result(lt("[]", {TokenType::TK_LBRACKET, TokenType::TK_RBRACKET}));
    ts << "buildin" << result(lt("max min count add mult days before after today date open close write system has", {TokenType::TK_MAX, TokenType::TK_MIN, TokenType::TK_COUNT, TokenType::TK_ADD, TokenType::TK_MULT, TokenType::TK_DAYS, TokenType::TK_BEFORE, TokenType::TK_AFTER, TokenType::TK_TODAY, TokenType::TK_DATE, TokenType::TK_OPEN, TokenType::TK_CLOSE, TokenType::TK_WRITE, TokenType::TK_SYSTEM, TokenType::TK_HAS}));
    ts << "if" << result(lt("if iff fi", {TokenType::TK_IF, TokenType::TK_NAME, TokenType::TK_FI}));
    ts << "choice" << result(lt("&", {TokenType::TK_CHOICE}));
    ts << "bang" << result(lt("!!<<!>", {TokenType::TK_BANG, TokenType::TK_BANGLT, TokenType::TK_LESS, TokenType::TK_BANGGT}));
    ts << "tilde" << result(lt("~", {TokenType::TK_TILDE}));
	ts << "remove" << result(lt("abe\\baz", {TokenType::TK_NAME, TokenType::TK_SET_MINUS, TokenType::TK_NAME}));
    ts << "is" << result(lt("is-Bool is-Int is-Text is-Atom is-Tup is-Rel is-Func is-Any",
							{TokenType::TK_ISBOOL, TokenType::TK_ISINT, TokenType::TK_ISTEXT, TokenType::TK_ISATOM, TokenType::TK_ISTUP, TokenType::TK_ISREL, TokenType::TK_ISFUNC, TokenType::TK_ISANY}));
    ts << "error" << result(lt("hat %%Kat hat", {TokenType::TK_NAME, TokenType::INVALID, TokenType::INVALID, TokenType::TK_NAME, TokenType::TK_NAME}));
}


int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
		.multi_test(base, "base");
}
