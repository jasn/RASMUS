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
#include "tokenizer.hh"

#include <unordered_map>
#include <cctype>

namespace {
using namespace rasmus::frontend;

const std::vector<std::pair<lexer::TokenType, std::string> > operators = {
    {lexer::TokenType::TK_ASSIGN, ":="},
    {lexer::TokenType::TK_BANG, "!"},
    {lexer::TokenType::TK_BANGGT, "!>"},
    {lexer::TokenType::TK_BANGLT, "!<"},
    {lexer::TokenType::TK_BLOCKEND, "+)"}, 
    {lexer::TokenType::TK_BLOCKSTART, "(+"},
    {lexer::TokenType::TK_CHOICE, "&"},
    {lexer::TokenType::TK_COLON, ":"},
    {lexer::TokenType::TK_COMMA, ","},
    {lexer::TokenType::_TK_COMMENT, "//"},
    {lexer::TokenType::TK_CONCAT, "++"},
    {lexer::TokenType::TK_DIFFERENT, "<>"},
    {lexer::TokenType::TK_DIV, "/"},
    {lexer::TokenType::TK_EQUAL, "="},
    {lexer::TokenType::TK_GREATER, ">"},
    {lexer::TokenType::TK_GREATEREQUAL, ">="},
    {lexer::TokenType::TK_ISANY, "is-Any"},
    {lexer::TokenType::TK_ISATOM, "is-Atom"},
    {lexer::TokenType::TK_ISBOOL, "is-Bool"},
    {lexer::TokenType::TK_ISFUNC, "is-Func"},
    {lexer::TokenType::TK_ISINT, "is-Int"},
    {lexer::TokenType::TK_ISREL, "is-Rel"},
    {lexer::TokenType::TK_ISTEXT, "is-Text"},
    {lexer::TokenType::TK_ISTUP, "is-Tup"},
    {lexer::TokenType::TK_LBRACKET, "["},
    {lexer::TokenType::TK_LEFT_ARROW, "<-"},
    {lexer::TokenType::TK_LESS, "<"},
    {lexer::TokenType::TK_LESSEQUAL, "<="},
    {lexer::TokenType::TK_LPAREN, "("}, 
    {lexer::TokenType::TK_MINUS, "-"},
    {lexer::TokenType::TK_MUL, "*"},
    {lexer::TokenType::TK_ONE_DOT, "."},
    {lexer::TokenType::TK_OPEXTEND, "<<"},
    {lexer::TokenType::TK_PIPE, "|"},
    {lexer::TokenType::TK_PLUS, "+"},
    {lexer::TokenType::TK_PROJECT_MINUS, "|-"},
    {lexer::TokenType::TK_PROJECT_PLUS, "|+"},
    {lexer::TokenType::TK_SELECT, "?("},
    {lexer::TokenType::TK_RBRACKET, "]"},
    {lexer::TokenType::TK_RIGHTARROW, "->"},
    {lexer::TokenType::TK_RPAREN, ")"}, 
    {lexer::TokenType::TK_SEMICOLON, ";"},
    {lexer::TokenType::TK_SET_MINUS, "\\"},
    {lexer::TokenType::TK_SHARP, "#"},
    {lexer::TokenType::TK_STDBOOL, "?-Bool"},
    {lexer::TokenType::TK_STDINT, "?-Int"},
    {lexer::TokenType::TK_STDTEXT, "?-Text"},
    {lexer::TokenType::TK_TILDE, "~"},
    {lexer::TokenType::TK_TWO_DOTS, ".."}
};

const std::vector<std::pair<lexer::TokenType, std::string> > keywords = {
    {lexer::TokenType::TK_ADD, "add"},
    {lexer::TokenType::TK_AFTER, "after"},
    {lexer::TokenType::TK_AND, "and"},
    {lexer::TokenType::TK_BEFORE, "before"},
    {lexer::TokenType::TK_CLOSE, "close"},
    {lexer::TokenType::TK_COUNT, "count"},
    {lexer::TokenType::TK_DATE, "date"},
    {lexer::TokenType::TK_DAYS, "days"},
    {lexer::TokenType::TK_END, "end"},
    {lexer::TokenType::TK_FALSE, "false"}, 
    {lexer::TokenType::TK_FI, "fi"},
    {lexer::TokenType::TK_FUNC, "func"},
    {lexer::TokenType::TK_HAS, "has"},
    {lexer::TokenType::TK_IF, "if"},
    {lexer::TokenType::TK_IN, "in"},
    {lexer::TokenType::TK_MAX, "max"},
    {lexer::TokenType::TK_MIN, "min"},
    {lexer::TokenType::TK_MOD, "mod"},
    {lexer::TokenType::TK_MULT, "mult"},
    {lexer::TokenType::TK_NOT, "not"},
    {lexer::TokenType::TK_ONE, "one"},
    {lexer::TokenType::TK_OPEN, "open"},
    {lexer::TokenType::TK_OR, "or"},
    {lexer::TokenType::TK_REL, "rel"},
    {lexer::TokenType::TK_SYSTEM, "system"},
    {lexer::TokenType::TK_TODAY, "today"},
    {lexer::TokenType::TK_TRUE, "true"},
    {lexer::TokenType::TK_TUP, "tup"}, 
    {lexer::TokenType::TK_TYPE_ANY, "Any"},
    {lexer::TokenType::TK_TYPE_ATOM, "Atom"},
    {lexer::TokenType::TK_TYPE_BOOL, "Bool"},
    {lexer::TokenType::TK_TYPE_FUNC, "Func"},
    {lexer::TokenType::TK_TYPE_INT, "Int"},
    {lexer::TokenType::TK_TYPE_REL, "Rel"},
    {lexer::TokenType::TK_TYPE_TEXT, "Text"},
    {lexer::TokenType::TK_TYPE_TUP, "Tup"},
    {lexer::TokenType::TK_VAL, "val"},
    {lexer::TokenType::TK_WRITE, "write"},
    {lexer::TokenType::TK_ZERO, "zero"},
    {lexer::TokenType::TK_PRINT, "print"}
};


std::unordered_map<lexer::TokenType, std::string> tokenNames {
  {lexer::TokenType::END_OF_FILE, "End of file"},
    //{lexer::TokenType::TK_ERR, "Bad token"},
  {lexer::TokenType::TK_NAME, "Name"},
  {lexer::TokenType::TK_INT,  "Int"},
  {lexer::TokenType::TK_TEXT, "Text"}};
  
std::vector<std::unordered_map<std::string, lexer::TokenType> > operatorMap;
std::unordered_map<std::string, lexer::TokenType> keywordMap;

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

const std::string getTokenName(lexer::TokenType id) {
	return tokenNames[id];
}

Token Lexer::getNext() {
  const char *prev = this->tknizer.str;
  size_t prev_index = this->index;

  lexer::Token tk = this->tknizer.getNextToken();

  size_t diff = tk.curr - prev;
  size_t curr_index = prev_index + diff;

  size_t token_length = tk.curr - tk.start;

  size_t start_index = curr_index - token_length;

  this->index = curr_index;

  return Token(tk.tkn, start_index, token_length);

}

} //namespace frontend
} //namespace rasmus




