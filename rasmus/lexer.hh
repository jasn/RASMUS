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
#ifndef __lexer_hh__
#define __lexer_hh__

#include <functional>
#include <memory>
#include "code.hh"

enum TokenId {
  TK_ADD,
  TK_AFTER,
  TK_AND,
  TK_ASSIGN,
  TK_AT,
  TK_BANG,
  TK_BANGGT,
  TK_BANGLT,
  TK_BEFORE,
  TK_BLOCKEND,
  TK_BLOCKSTART,
  TK_CHOICE,
  TK_CLOSE,
  TK_COLON,
  TK_COMMA,
  TK_CONCAT,
  TK_COUNT,
  TK_DATE,
  TK_DAYS,
  TK_DIFFERENT,
  TK_DIV,
  TK_END,
  TK_EOF,
  TK_EQUAL,
  TK_ERR,
  TK_FALSE,
  TK_FI,
  TK_FUNC,
  TK_GREATER,
  TK_GREATEREQUAL,
  TK_HAS,
  TK_IF,
  TK_IN,
  TK_INT,
  TK_ISANY,
  TK_ISATOM,
  TK_ISBOOL,
  TK_ISFUNC,
  TK_ISINT,
  TK_ISREL,
  TK_ISTEXT,
  TK_ISTUP,
  TK_LBRACKET,
  TK_LEFT_ARROW,
  TK_LESS,
  TK_LESSEQUAL,
  TK_LPAREN,
  TK_MAX,
  TK_MIN,
  TK_MINUS,
  TK_MOD,
  TK_MUL,
  TK_MULT,
  TK_NAME,
  TK_NOT,
  TK_ONE,
  TK_ONE_DOT,
  TK_OPEN,
  TK_OPEXTEND,
  TK_OR,
  TK_PIPE,
  TK_PRINT,
  TK_PLUS,
  TK_PROJECT_MINUS,
  TK_PROJECT_PLUS,
  TK_QUESTION,
  TK_RBRACKET,
  TK_REL,
  TK_RIGHTARROW,
  TK_RPAREN,
  TK_SEMICOLON,
  TK_SET_MINUS,
  TK_SHARP,
  TK_STDBOOL,
  TK_STDINT,
  TK_STDTEXT,
  TK_SYSTEM,
  TK_TEXT,
  TK_TILDE,
  TK_TODAY,
  TK_TRUE,
  TK_TUP,
  TK_TWO_DOTS,
  TK_TYPE_ANY,
  TK_TYPE_ATOM,
  TK_TYPE_BOOL,
  TK_TYPE_FUNC,
  TK_TYPE_INT,
  TK_TYPE_REL,
  TK_TYPE_TEXT,
  TK_TYPE_TUP,
  TK_VAL,
  TK_WRITE,
  TK_ZERO,
  TK_INVALID
};

class Token {
public:
	Token(TokenId id, uint32_t start, uint32_t length): id(id), start(start), length(length) {}
	Token(): id(TK_INVALID), start(0), length(0) {}
	
	operator bool() {return id != TK_INVALID;}
	
	
	TokenId id;
	uint32_t start;
	uint32_t length;
};

namespace std {
  template <>
  struct hash<TokenId>: public unary_function<TokenId, size_t> {
    size_t operator()(TokenId c) const {return size_t(c);}
  };
}

const std::string getTokenName(TokenId id);

class Lexer {
public:
	std::shared_ptr<Code> code;
	size_t index;
	Lexer(std::shared_ptr<Code> code, size_t start=0): code(code), index(start) {}
	Token getNext();
};

#endif //__lexer_hh__
