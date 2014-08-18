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

#include "highlighter.hh"
#include <iostream>
#include <frontend/lexer.hh>
#include <frontend/code.hh>
#include <frontend/error.hh>
#include <frontend/parser.hh>
#include <frontend/charRanges.hh>
#include <frontend/firstParse.hh>
#include <frontend/callback.hh>
#include <algorithm>
#include <set>

Highlighter::Highlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent) {
}

enum class StyleType {
	NORMAL, TEXT, KEYWORD, ERROR
};

struct Style {
	size_t start;
	size_t end;
	std::string message;
	StyleType type;
	friend bool operator <(const Style & l, const Style & r) {
		return l.end < r.end;
	}
};

namespace f=rasmus::frontend;

class MyErr: public f::Error {
public:
	std::vector<Style> & styles;
	MyErr(std::vector<Style> & styles): styles(styles) {}


	void reportWarning(std::string message,
					   f::Token mainToken,
					   std::initializer_list<f::CharRange> ranges) override {

	}
	
	void reportError(std::string message,
					 f::Token mainToken,
					 std::initializer_list<f::CharRange> ranges={}) override {
		int lo = std::numeric_limits<int>::max();
		int hi = std::numeric_limits<int>::min();
		if (mainToken) {
			lo = std::min<int>(lo, mainToken.start);
			hi = std::max<int>(hi, mainToken.length + mainToken.start);
		}
		for (auto r: ranges) {
			lo = std::min<int>(lo, r.lo);
			hi = std::max<int>(hi, r.hi);
		}
		Style s;
		s.start= lo;
		s.end = hi;
		s.type = StyleType::ERROR;
		s.message = message;
		styles.push_back(s);
	}

	size_t count() const override {
		return 0;
	}
};



class MyCallback: public f::Callback {
public:
	void report(f::MsgType type, 
				std::shared_ptr<f::Code> code,
				std::string message,
				f::Token mainToken,
				std::vector<f::CharRange> ranges) {}
	
	void report(f::MsgType type, std::string message) {}
	
	void print(Type type, std::string repr) {}
	
	void saveRelation(rm_object * o, const char * name) {}
	rm_object * loadRelation(const char * name) {}
	bool hasRelation(const char * name) {return false;}
};

void Highlighter::highlightBlock(const QString &text) {
	std::shared_ptr<f::Code> code = std::make_shared<f::Code>(
		text.toUtf8().constData(), "hat");
	
	std::vector<Style> styles;
	{
		f::Lexer l(code);
		bool done=false;
		while (!done) {
			f::Token t=l.getNext();
			Style s;
			s.start=t.start;
			s.end=t.start+t.length;
			s.type=StyleType::ERROR;
			switch (t.id) {
			case lexer::TokenType::TK_TEXT:
				s.type=StyleType::TEXT;
				break;
			case lexer::TokenType::TK_ISANY:
			case lexer::TokenType::TK_ISATOM:
			case lexer::TokenType::TK_ISBOOL:
			case lexer::TokenType::TK_ISFUNC:
			case lexer::TokenType::TK_ISINT:
			case lexer::TokenType::TK_ISREL:
			case lexer::TokenType::TK_ISTEXT:
			case lexer::TokenType::TK_ISTUP:
			case lexer::TokenType::TK_STDBOOL:
			case lexer::TokenType::TK_STDINT:
			case lexer::TokenType::TK_STDTEXT:
			case lexer::TokenType::TK_ADD:
			case lexer::TokenType::TK_AFTER:
			case lexer::TokenType::TK_AND:
			case lexer::TokenType::TK_BEFORE:
			case lexer::TokenType::TK_CLOSE:
			case lexer::TokenType::TK_COUNT:
			case lexer::TokenType::TK_DATE:
			case lexer::TokenType::TK_DAYS:
			case lexer::TokenType::TK_END:
			case lexer::TokenType::TK_FALSE:
			case lexer::TokenType::TK_FI:
			case lexer::TokenType::TK_FUNC:
			case lexer::TokenType::TK_HAS:
			case lexer::TokenType::TK_IF:
			case lexer::TokenType::TK_IN:
			case lexer::TokenType::TK_MAX:
			case lexer::TokenType::TK_MIN:
			case lexer::TokenType::TK_MOD:
			case lexer::TokenType::TK_MULT:
			case lexer::TokenType::TK_NOT:
			case lexer::TokenType::TK_ONE:
			case lexer::TokenType::TK_OPEN:
			case lexer::TokenType::TK_OR:
			case lexer::TokenType::TK_REL:
			case lexer::TokenType::TK_SYSTEM:
			case lexer::TokenType::TK_TODAY:
			case lexer::TokenType::TK_TRUE:
			case lexer::TokenType::TK_TUP:
			case lexer::TokenType::TK_TYPE_ANY:
			case lexer::TokenType::TK_TYPE_ATOM:
			case lexer::TokenType::TK_TYPE_BOOL:
			case lexer::TokenType::TK_TYPE_FUNC:
			case lexer::TokenType::TK_TYPE_INT:
			case lexer::TokenType::TK_TYPE_REL:
			case lexer::TokenType::TK_TYPE_TEXT:
			case lexer::TokenType::TK_TYPE_TUP:
			case lexer::TokenType::TK_VAL:
			case lexer::TokenType::TK_WRITE:
			case lexer::TokenType::TK_ZERO:
			case lexer::TokenType::TK_PRINT:
				s.type=StyleType::KEYWORD;
				break;
			case lexer::TokenType::END_OF_FILE:
				done=true;
				break;
			default:
				s.type=StyleType::NORMAL;
				break;
			}
			if (s.type != StyleType::ERROR)
				styles.push_back(std::move(s));
		}
	}

	{
		std::shared_ptr<f::Error> error=std::make_shared<MyErr>(styles);
		std::shared_ptr<f::Callback> callback=std::make_shared<MyCallback>();
		std::shared_ptr<f::Lexer> lexer=std::make_shared<f::Lexer>(code);
		std::shared_ptr<f::Parser> parser=f::makeParser(lexer, error, false);
		std::shared_ptr<f::CharRanges > charRanges=f::makeCharRanges();
		std::shared_ptr<f::FirstParse> firstParse=f::makeFirstParse(error, code, callback,
																	TAny);
		f::NodePtr n=parser->parse();
		if (n) charRanges->run(n);
		if (n) firstParse->run(n);
	}
	std::sort(styles.begin(), styles.end(), 
			  [](const Style & a, const Style & b) {
				  return a.start > b.start;
			  });

	std::set<Style> currentStyles;
	size_t prev=0;
	QTextCharFormat format;
	format.setForeground(Qt::red);
	format.setUnderlineColor(Qt::red);
	while (!currentStyles.empty() || !styles.empty()) {
		size_t cur=std::numeric_limits<size_t>::max();
		if (!currentStyles.empty())
			cur=std::min(cur, currentStyles.begin()->end);
		if (!styles.empty())
			cur=std::min(cur, styles.back().start);

		while (!styles.empty() && styles.back().start <= cur) {
			currentStyles.insert(styles.back());
			styles.pop_back();
		}

		while (!currentStyles.empty() && currentStyles.begin()->end <= cur)
			currentStyles.erase(currentStyles.begin());

		if (cur != prev) setFormat(prev, cur-prev, format);
		prev=cur;

		format = QTextCharFormat();
		format.setForeground(Qt::red);
		for(const Style & s: currentStyles) {
			switch(s.type) {
			case StyleType::NORMAL:
				format.setForeground(Qt::black);
				break;
			case StyleType::TEXT:
				format.setForeground(Qt::green);				
				break;
			case StyleType::KEYWORD:
				format.setForeground(Qt::blue);
				break;
			case StyleType::ERROR:
				format.setToolTip(QString::fromUtf8(s.message.c_str()));
				format.setUnderlineColor(Qt::red);
				format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
				break;
			}
		}
	}
	setFormat(prev, 123123123, format);
}


void Highlighter::highlightAll() {
}
