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
#include "table.hh"
#include "settings.hh"

namespace f=rasmus::frontend;

struct RIssue {
	size_t start;
	size_t end;
	std::string message;
	IssueType type;
};


class MyErr: public f::Error {
public:
	std::vector<RIssue> & rIssues;
	MyErr(std::vector<RIssue> & rIssues): rIssues(rIssues) {}
	size_t cnt;

	void report(std::string message, f::Token mainToken, std::initializer_list<f::CharRange> ranges, 
				IssueType type) {
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
		RIssue i;
		i.start= lo;
		i.end = hi;
		i.type = type;
		i.message = message;
		rIssues.push_back(i);
	}
	
	void reportWarning(std::string message,
					   f::Token mainToken,
					   std::initializer_list<f::CharRange> ranges) override {
		report(message, mainToken, ranges, IssueType::WARNING);
	}
	
	void reportError(std::string message,
					 f::Token mainToken,
					 std::initializer_list<f::CharRange> ranges={}) override {
		report(message, mainToken, ranges, IssueType::ERROR);
		++cnt;
	}

	size_t count() const override {return cnt;}
};

class MyCallback: public f::Callback {
public:
	void report(f::MsgType,
				std::shared_ptr<f::Code>,
				std::string,
				f::Token,
				std::vector<f::CharRange>) {}
	
	void report(f::MsgType, std::string) {}
	
	void print(Type, std::string) {}
	
	void saveRelation(rm_object *, const char *) {}
	rm_object * loadRelation(const char *) {return nullptr;}
	bool hasRelation(const char *) {return false;}

	void deleteRelation(const char *) {}

};

void Intellisense::process(std::vector<std::string> * blocks) {
	std::vector< std::pair<int, int> > locations;
	std::string str;
	for (size_t i=0; i < blocks->size(); ++i) {
		size_t j=0;
		std::string s = (*blocks)[i]+'\n';
		for (size_t k=0; k < s.size(); ++k) {
			str.push_back(s[k]);
			locations.push_back(std::make_pair(i, j));
			if ((s[k] & 0xc0) != 0x80) ++j; 
		}
	}
	std::vector<Issue> * issues = new std::vector<Issue>();
	if (str.empty()) {
		delete blocks;
		emit this->issues(issues);
		return;
	}
	std::vector<RIssue> rIssues;
	std::shared_ptr<f::Code> code=std::make_shared<f::Code>(str, "");
	std::shared_ptr<f::Error> error=std::make_shared<MyErr>(rIssues);
	std::shared_ptr<f::Callback> callback=std::make_shared<MyCallback>();
	std::shared_ptr<f::Lexer> lexer=std::make_shared<f::Lexer>(code);
	std::shared_ptr<f::Parser> parser=f::makeParser(lexer, error, false);
	std::shared_ptr<f::CharRanges > charRanges=f::makeCharRanges();
	std::shared_ptr<f::FirstParse> firstParse=f::makeFirstParse(error, code, callback, TAny);
	f::NodePtr n=parser->parse();
	if (n) charRanges->run(n);
	if (n) firstParse->run(n);

	
	

	for (auto r: rIssues) {
		r.start = std::min<size_t>(str.size()-1, r.start);
		r.end = std::min<size_t>(str.size(), r.end);
		if (r.start >= r.end) continue;
		Issue i;
		std::tie(i.block, i.start) = locations[r.start];
		i.message = r.message;
		i.type = r.type;
		while (i.block != locations[r.end-1].first) {
			i.end = blocks[i.block].size(); //TODO utf fix
			issues->push_back(i);
			i.start = 0;
		}
		i.end = locations[r.end-1].second+1;
		issues->push_back(i);
	}
	delete blocks;
	emit this->issues(issues);
}


Highlighter::Highlighter(QTextDocument *parent, Settings * settings)
	: QSyntaxHighlighter(parent), intellisense(nullptr), intellinensing(false), upToDate(true), noIntelli(false), settings(settings) {
	intellisenseThread.start();
	intellisense = new Intellisense();
	intellisense->moveToThread(&intellisenseThread);

	QObject::connect(this, SIGNAL(runIntellisense(std::vector<std::string>*)), 
					 intellisense, SLOT(process(std::vector<std::string>*)));

	QObject::connect(intellisense, SIGNAL(issues(std::vector<Issue>*)),
					 this, SLOT(registerIssues(std::vector<Issue>*)));
	
}

Highlighter::~Highlighter() {
	intellisenseThread.quit();
	intellisenseThread.wait();
}

enum class StyleType {
	NORMAL, TEXT, KEYWORD, ERROR, WARNING, COMMENT, INVALID
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


void Highlighter::highlightBlock(const QString &text) {
	int state=0;
	/*int state =previousBlockState();
	  if (state == -1) state = lexer::initialState;*/

	std::vector<Style> styles;

	std::string s=text.toUtf8().constData();
	s += '\n';
	
	size_t j=0;
	size_t start=j;
	for (size_t i=0; i < s.size(); ++i)  {
		state=lexer::table[state][(uint8_t)s[i]];
		if (state >= (int)lexer::TableTokenType::INVALID) {
			Style s;
			s.type = StyleType::NORMAL;
			switch ((lexer::TableTokenType)state) {
			case lexer::TableTokenType::TK_TEXT: 
				s.type=StyleType::TEXT; 
				break;
			case lexer::TableTokenType::TK_ISANY:
			case lexer::TableTokenType::TK_ISATOM:
			case lexer::TableTokenType::TK_ISBOOL:
			case lexer::TableTokenType::TK_ISFUNC:
			case lexer::TableTokenType::TK_ISINT:
			case lexer::TableTokenType::TK_ISREL:
			case lexer::TableTokenType::TK_ISTEXT:
			case lexer::TableTokenType::TK_ISTUP:
			case lexer::TableTokenType::TK_STDBOOL:
			case lexer::TableTokenType::TK_STDINT:
			case lexer::TableTokenType::TK_STDTEXT:
			case lexer::TableTokenType::TK_ADD:
			case lexer::TableTokenType::TK_AFTER:
			case lexer::TableTokenType::TK_AND:
			case lexer::TableTokenType::TK_BEFORE:
			case lexer::TableTokenType::TK_CLOSE:
			case lexer::TableTokenType::TK_COUNT:
			case lexer::TableTokenType::TK_DATE:
			case lexer::TableTokenType::TK_DAYS:
			case lexer::TableTokenType::TK_END:
			case lexer::TableTokenType::TK_FALSE:
			case lexer::TableTokenType::TK_FI:
			case lexer::TableTokenType::TK_FUNC:
			case lexer::TableTokenType::TK_HAS:
			case lexer::TableTokenType::TK_IF:
			case lexer::TableTokenType::TK_IN:
			case lexer::TableTokenType::TK_MAX:
			case lexer::TableTokenType::TK_MIN:
			case lexer::TableTokenType::TK_MOD:
			case lexer::TableTokenType::TK_MULT:
			case lexer::TableTokenType::TK_NOT:
			case lexer::TableTokenType::TK_ONE:
			case lexer::TableTokenType::TK_OPEN:
			case lexer::TableTokenType::TK_OR:
			case lexer::TableTokenType::TK_REL:
			case lexer::TableTokenType::TK_SYSTEM:
			case lexer::TableTokenType::TK_TODAY:
			case lexer::TableTokenType::TK_TRUE:
			case lexer::TableTokenType::TK_TUP:
			case lexer::TableTokenType::TK_TYPE_ANY:
			case lexer::TableTokenType::TK_TYPE_ATOM:
			case lexer::TableTokenType::TK_TYPE_BOOL:
			case lexer::TableTokenType::TK_TYPE_FUNC:
			case lexer::TableTokenType::TK_TYPE_INT:
			case lexer::TableTokenType::TK_TYPE_REL:
			case lexer::TableTokenType::TK_TYPE_TEXT:
			case lexer::TableTokenType::TK_TYPE_TUP:
			case lexer::TableTokenType::TK_VAL:
			case lexer::TableTokenType::TK_WRITE:
			case lexer::TableTokenType::TK_ZERO:
			case lexer::TableTokenType::TK_PRINT:
				s.type = StyleType::KEYWORD;
				break;
			case lexer::TableTokenType::INVALID:
				s.type = StyleType::INVALID;
				break;
			case lexer::TableTokenType::_TK_COMMENT:
				s.type = StyleType::COMMENT;
				break;
			default:
				break;
			}
			if (s.type != StyleType::NORMAL) {
				s.start=start;
				s.end=j;
				styles.push_back(s);
			}
			
			state = lexer::initialState;
			// We need to reprocess the current char
			if (j != start) { 
				--i;
				start=j;
				continue;
			}
			// A new token starts here
			start=j;
		}
		//If this is not a utf8 continuation char, increase the logical char count
		if ((s[i] & 0xc0) != 0x80) ++j; 
	}

	setCurrentBlockState(state);

	auto x=issues.find(currentBlock().blockNumber());
	if (x != issues.end()) 
		for(auto i: x->second) {
			Style s;
			s.start = i.start;
			s.end = i.end;
			s.message =i.message;
			switch(i.type) {
			case IssueType::ERROR: s.type=StyleType::ERROR; break;
			case IssueType::WARNING: s.type=StyleType::WARNING; break;
			}
			styles.push_back(s);
		}

	std::sort(styles.begin(), styles.end(), 
			  [](const Style & a, const Style & b) {
				  return a.start > b.start;
			  });
	
	std::set<Style> currentStyles;
	size_t prev=0;
	QTextCharFormat format;
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
		for(const Style & s: currentStyles) {
			switch(s.type) {
			case StyleType::INVALID:
				format.setForeground(settings->color(Colors::editorError));
			case StyleType::COMMENT:
				format.setForeground(settings->color(Colors::editorComment));
				break;
			case StyleType::TEXT:
				format.setForeground(settings->color(Colors::editorText));
				break;
			case StyleType::KEYWORD:
				format.setForeground(settings->color(Colors::editorKeyword));
				break;
			case StyleType::ERROR:
				format.setToolTip(QString::fromUtf8(s.message.c_str()));
				format.setForeground(settings->color(Colors::editorError));
				format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
				break;
			case StyleType::WARNING:
				format.setToolTip(QString::fromUtf8(s.message.c_str()));
				format.setForeground(settings->color(Colors::editorWarning));
				format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
				break;
			case StyleType::NORMAL:
				break;
			}
		}
	}
	setFormat(prev, currentBlock().length(), format);

	doIntellisense();
}

void Highlighter::doIntellisense() {
	if (noIntelli) return;

	if (intellinensing) {
		upToDate=false;
		return;
	}
	upToDate=true;
	intellinensing=true;

	std::vector<std::string> * blocks = new std::vector<std::string>();
	for(auto block=document()->begin(); block != document()->end(); block=block.next())
		blocks->push_back(block.text().toUtf8().constData());
	
	emit runIntellisense(blocks);
}

void Highlighter::registerIssues(std::vector<Issue> * issues) {
	intellinensing=false;
	
	this->issues.clear();
	for (Issue i: *issues)
		this->issues[i.block].push_back(i);
	delete issues;

	//Trigger redraw
	noIntelli=true;
	rehighlight();
	noIntelli=false;
	
	if (!upToDate) doIntellisense();
}

std::string Highlighter::getIssue(size_t block, size_t index) {
	auto itr=issues.find(block);
	if (itr == issues.end()) return "";
	for (auto issue: itr->second) {
		if (issue.start <= index &&
			issue.end > index) return issue.message;
	}
	return "";
}

void Highlighter::updateSettings(Settings * s) {
	settings = s;
	noIntelli=true;
	rehighlight();
	noIntelli=false;
}
