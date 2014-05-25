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

bool lt(const char * txt, std::initializer_list<TokenId> exp) {
	std::shared_ptr<Code> code = std::make_shared<Code>(txt, "monkey");
	Lexer l(code, 0);
	
	std::vector<TokenId> lst;
	while (true) {
		Token t = l.getNext();
		if (t.id == TK_EOF) break;;
		lst.push_back(t.id);
	}
	return lst==std::vector<TokenId>(exp);
}


void base(rasmus::teststream & ts) {
    ts << "tup" << result(lt("tup", {TK_TUP}));
    ts << "rel" << result(lt("rel", {TK_REL}));
    ts << "func" << result(lt("func", {TK_FUNC}));
    ts << "end" << result(lt("end", {TK_END}));
    ts << "at" << result(lt("@", {TK_AT}));
    ts << "sharp" << result(lt("#", {TK_SHARP}));
    ts << "not" << result(lt("not nott", {TK_NOT, TK_NAME}));
    ts << "minus" << result(lt("-", {TK_MINUS}));
    ts << "and" << result(lt("and", {TK_AND}));
    ts << "or" << result(lt("or", {TK_OR}));
    ts << "truefalse" << result(lt("false-true truee", {TK_FALSE, TK_MINUS, TK_TRUE, TK_NAME}));
    ts << "paren" << result(lt("((++ )+)", {TK_LPAREN, TK_BLOCKSTART, TK_PLUS, TK_RPAREN, TK_BLOCKEND}));
    ts << "val" << result(lt("val", {TK_VAL}));
    ts << "in" << result(lt("in", {TK_IN}));
    ts << "name" << result(lt("namee", {TK_NAME}));
    ts << "colon" << result(lt(":", {TK_COLON}));
    ts << "int" << result(lt("1234", {TK_INT}));
    ts << "text" << result(lt("\"hello\"", {TK_TEXT}));
    ts << "zero" << result(lt("zero", {TK_ZERO}));
    ts << "one" << result(lt("one", {TK_ONE}));
    ts << "stdbool" << result(lt("?-Bool", {TK_STDBOOL}));
    ts << "stdint" << result(lt("?-Int", {TK_STDINT}));
    ts << "stdtext" << result(lt("?-Text", {TK_STDTEXT}));
    ts << "comma" << result(lt(",", {TK_COMMA}));
    ts << "arrow" << result(lt("--> <-", {TK_MINUS, TK_RIGHTARROW, TK_LEFT_ARROW}));
    ts << "type_bool" << result(lt("Bool", {TK_TYPE_BOOL}));
    ts << "type_int" << result(lt("Int", {TK_TYPE_INT}));
    ts << "type_text" << result(lt("Text", {TK_TYPE_TEXT}));
    ts << "type_atom" << result(lt("Atom", {TK_TYPE_ATOM}));
    ts << "type_tup" << result(lt("Tup", {TK_TYPE_TUP}));
    ts << "type_rel" << result(lt("Rel", {TK_TYPE_REL}));
    ts << "type_func" << result(lt("Func", {TK_TYPE_FUNC}));
    ts << "type_any" << result(lt("Any", {TK_TYPE_ANY}));
    ts << "operators" << result(lt("+*/++-\\", {TK_PLUS, TK_MUL, TK_DIV, TK_CONCAT, TK_MINUS, TK_SET_MINUS}));
    ts << "mod" << result(lt("mod", {TK_MOD}));
    ts << "comp" << result(lt("< <> <= >= = >", {TK_LESS, TK_DIFFERENT, TK_LESSEQUAL, TK_GREATEREQUAL, TK_EQUAL, TK_GREATER}));
    ts << "semicolon" << result(lt(";", {TK_SEMICOLON}));
    ts << "pipe" << result(lt("|", {TK_PIPE}));
    ts << "dot" << result(lt("...", {TK_TWO_DOTS, TK_ONE_DOT}));
    ts << "question" << result(lt("?", {TK_QUESTION}));
    ts << "project" << result(lt("||+|--", {TK_PIPE, TK_PROJECT_PLUS, TK_PROJECT_MINUS, TK_MINUS}));
    ts << "bracket" << result(lt("[]", {TK_LBRACKET, TK_RBRACKET}));
    ts << "buildin" << result(lt("max min count add mult days before after today date open close write system has", {TK_MAX, TK_MIN, TK_COUNT, TK_ADD, TK_MULT, TK_DAYS, TK_BEFORE, TK_AFTER, TK_TODAY, TK_DATE, TK_OPEN, TK_CLOSE, TK_WRITE, TK_SYSTEM, TK_HAS}));
    ts << "if" << result(lt("if iff fi", {TK_IF, TK_NAME, TK_FI}));
    ts << "choice" << result(lt("&", {TK_CHOICE}));
    ts << "bang" << result(lt("!!<<!>", {TK_BANG, TK_BANGLT, TK_LESS, TK_BANGGT}));
    ts << "tilde" << result(lt("~", {TK_TILDE}));
    ts << "is" << result(lt("is-Booll is-Int is-Text is-Atom is-Tup is-Rel is-Func is-Any",
							{TK_ISBOOL, TK_NAME, TK_ISINT, TK_ISTEXT, TK_ISATOM, TK_ISTUP, TK_ISREL, TK_ISFUNC, TK_ISANY}));
    ts << "error" << result(lt("hat %%Kat hat", {TK_NAME, TK_ERR, TK_NAME}));
}


int main(int argc, char **argv) {
	return rasmus::tests(argc, argv)
		.multi_test(base, "base");
}
