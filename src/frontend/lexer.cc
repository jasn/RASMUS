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
#include "lexer.hh"
#include <unordered_map>
#include <cctype>

namespace {
using namespace rasmus::frontend;

const std::vector<std::pair<TokenId, std::string> > operators = {
    {TK_ASSIGN, ":="},
    {TK_AT, "@"},
    {TK_BANG, "!"},
    {TK_BANGGT, "!>"},
    {TK_BANGLT, "!<"},
    {TK_BLOCKEND, "+)"}, 
    {TK_BLOCKSTART, "(+"},
    {TK_CHOICE, "&"},
    {TK_COLON, ":"},
    {TK_COMMA, ","},
	{TK_COMMENT, "//"},
    {TK_CONCAT, "++"},
    {TK_DIFFERENT, "<>"},
    {TK_DIV, "/"},
    {TK_EQUAL, "="},
    {TK_GREATER, ">"},
    {TK_GREATEREQUAL, ">="},
    {TK_ISANY, "is-Any"},
    {TK_ISATOM, "is-Atom"},
    {TK_ISBOOL, "is-Bool"},
    {TK_ISFUNC, "is-Func"},
    {TK_ISINT, "is-Int"},
    {TK_ISREL, "is-Rel"},
    {TK_ISTEXT, "is-Text"},
    {TK_ISTUP, "is-Tup"},
    {TK_LBRACKET, "["},
    {TK_LEFT_ARROW, "<-"},
    {TK_LESS, "<"},
    {TK_LESSEQUAL, "<="},
    {TK_LPAREN, "("}, 
    {TK_MINUS, "-"},
    {TK_MUL, "*"},
    {TK_ONE_DOT, "."},
    {TK_OPEXTEND, "<<"},
    {TK_PIPE, "|"},
    {TK_PLUS, "+"},
    {TK_PROJECT_MINUS, "|-"},
    {TK_PROJECT_PLUS, "|+"},
    {TK_QUESTION, "?"},
    {TK_RBRACKET, "]"},
    {TK_RIGHTARROW, "->"},
    {TK_RPAREN, ")"}, 
    {TK_SEMICOLON, ";"},
    {TK_SET_MINUS, "\\"},
    {TK_SHARP, "#"},
    {TK_STDBOOL, "?-Bool"},
    {TK_STDINT, "?-Int"},
    {TK_STDTEXT, "?-Text"},
    {TK_TILDE, "~"},
    {TK_TWO_DOTS, ".."}
};

const std::vector<std::pair<TokenId, std::string> > keywords = {
    {TK_ADD, "add"},
    {TK_AFTER, "after"},
    {TK_AND, "and"},
    {TK_BEFORE, "before"},
    {TK_CLOSE, "close"},
    {TK_COUNT, "count"},
    {TK_DATE, "date"},
    {TK_DAYS, "days"},
    {TK_END, "end"},
    {TK_FALSE, "false"}, 
    {TK_FI, "fi"},
    {TK_FUNC, "func"},
    {TK_HAS, "has"},
    {TK_IF, "if"},
    {TK_IN, "in"},
    {TK_MAX, "max"},
    {TK_MIN, "min"},
    {TK_MOD, "mod"},
    {TK_MULT, "mult"},
    {TK_NOT, "not"},
    {TK_ONE, "one"},
    {TK_OPEN, "open"},
    {TK_OR, "or"},
    {TK_REL, "rel"},
    {TK_SYSTEM, "system"},
    {TK_TODAY, "today"},
    {TK_TRUE, "true"},
    {TK_TUP, "tup"}, 
    {TK_TYPE_ANY, "Any"},
    {TK_TYPE_ATOM, "Atom"},
    {TK_TYPE_BOOL, "Bool"},
    {TK_TYPE_FUNC, "Func"},
    {TK_TYPE_INT, "Int"},
    {TK_TYPE_REL, "Rel"},
    {TK_TYPE_TEXT, "Text"},
    {TK_TYPE_TUP, "Tup"},
    {TK_VAL, "val"},
    {TK_WRITE, "write"},
    {TK_ZERO, "zero"},
    {TK_PRINT, "print"}
};


std::unordered_map<TokenId, std::string> tokenNames {
    {TK_EOF, "End of file"},
	{TK_ERR, "Bad token"},
	{TK_NAME, "Name"},
	{TK_INT,  "Int"},
	{TK_TEXT, "Text"}};

std::vector<std::unordered_map<std::string, TokenId> > operatorMap;
std::unordered_map<std::string, TokenId> keywordMap;

struct InitLexer {
  InitLexer() {
    tokenNames.insert(operators.begin(), operators.end());
	for (auto & p: keywords)
		tokenNames[p.first] = std::string("\"") + p.second + "\"";
    size_t maxLength=0;
    for (auto & p: operators) maxLength=std::max(maxLength, p.second.length());
    operatorMap.resize(maxLength+1);
    for (auto & p: operators) operatorMap[p.second.length()][p.second] = p.first;
    for (auto & p: keywords) keywordMap[p.second] = p.first;
  }
};
InitLexer initLexer;

} //nameless namespace

namespace rasmus {
namespace frontend {

const std::string getTokenName(TokenId id) {
	return tokenNames[id];
}

Token Lexer::getNext() {
	const std::string & c=code->code;
	size_t i=index;

	bool moreComments = true;
	while (moreComments) {
		moreComments = false;
		while (i < c.length() && isspace(c[i]))
			++i;
		
		if (i + 1 < c.length() && c[i] == '/' && c[i+1] == '/') {
			while (i < c.length() && c[i] != '\n') {
				++i;
			}
			++i; // skip the '\n'
			moreComments = true;
		}
	}
	
	index=i;

	if (i == c.length())
		return Token(TK_EOF, i, 0);

	// check if is operator
	for (size_t l=operatorMap.size()-1; l != 0; --l) {
		auto p=operatorMap[l].find(c.substr(i, l));
		if (p == operatorMap[l].end()) continue;
		index += l;
		return Token(p->second, i, l);
	}

	// check if is Name or keyword
	if (isalpha(c[i])) {
		size_t j = 1;
		while (i+j < c.length() && isalnum(c[i+j]))
			++j;
		index += j;
		//check if is keyword
		auto p=keywordMap.find(c.substr(i, j));
		if (p != keywordMap.end())
			return Token(p->second, i, j);
		return Token(TK_NAME, i, j);
	}
       
	// check if it is an int
	if (isdigit(c[i])) {
		size_t j = 0;
		while (i+j < c.length() && isdigit(c[i+j]))
			++j;
		index += j;
		return Token(TK_INT, i, j);
	}

	// check if it is a text
	if (c[i] == '"') {
		size_t j = 1;
		while (i + j < c.length() && c[i+j] != '"') ++j;
		if (i+j == c.length()) {
			index += j;
			return Token(TK_ERR, i, j);
		}
		index += j+1;
		return Token(TK_TEXT, i, j+1);
	}
    
	// skip invalid token
	size_t j = 0;
	while (i+j < c.length() && !isspace(c[i+j])) 
		++j;
	index += j;
	return Token(TK_ERR, i, j);
}

} //namespace frontend
} //namespace rasmus




