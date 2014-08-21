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
#include <interpreter.hh>
#include <memory>
#include <iostream>
#include <algorithm>

#include <stdlib/relation.hh>
#include "frontend/callback.hh"
#include "frontend/interpreter.hh"
#include <llvm/Support/FileSystem.h>
#include <relation_model.hh>
#include "settings.hh"
#include <QDir>

namespace rf = rasmus::frontend;
namespace rs = rasmus::stdlib;

struct escaped {
public:
	const char * str;
	explicit escaped(const char * str): str(str) {}
	explicit escaped(const std::string & str): str(str.c_str()) {}
  
	friend std::ostream & operator<<(std::ostream & o, const escaped & e) {
		// only write the first say 10 lines
		const char * c=e.str;
		while (true) {
			switch (*c) {
			case '<': o << "&lt;"; break;
			case '>': o << "&ge;"; break;
			case ' ': o << "&nbsp;"; break;
			case '"': o << "&quot;"; break;
			case '\'': o << "&apos;"; break;
			case '&': o << "&amp;" ; break;
			case '\n': o << "<br>" ; break;
			case '\0': return o;
			default: o << *c; break;
			}
			++c;
		}
		return o;
	}
};


class GuiCallBack : public rasmus::frontend::Callback {
public:
	GuiCallBack(Interpreter * interperter, Settings * settings)
		: interperter(interperter)
		, settings(settings) {}

	void environmentChanged(const char * name) override {
		interperter->environmentChanged(name);
	}

	void printRel(rm_object *o) {
		rs::Relation *r = static_cast<rs::Relation*>(o);
		size_t sz = r->tuples.size();
		if (sz < 8) {
			rf::Callback::printRel(o);
		} else {
			interperter->doDisplay("Relation too big, displaying in seperate window");

			interperter->doDisplayRelation(r);
		}
	}

	virtual void report(rf::MsgType type, 
						std::shared_ptr<rf::Code> code,
						std::string message,
						rf::Token mainToken,
						std::vector<rf::CharRange> ranges) override {
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
	
		auto it=std::upper_bound(code->lineStarts.begin(), code->lineStarts.end(), lo);
		int line=it - code->lineStarts.begin();
		std::stringstream ss;
		switch (type) {
		case rf::MsgType::error:
			ss << "<span style=\"color: red\">error</span>: ";
			break;
		case rf::MsgType::warning:
			ss << "<span style=\"color: yellow\">warning</span>: ";
		default:
			break;
		}
		ss << " " << escaped(message) << "<br>";
		int startOfLine = std::min<int>(
			std::max<int>(0, code->lineStarts[line-1]+1),
			code->code.size());
		int endOfLine = std::max<int>(0,
									  std::min<int>(code->lineStarts[line], code->code.size()));
		if (endOfLine < startOfLine) endOfLine=startOfLine;
		ss << "<span style=\"color: green\">" << escaped(code->code.substr(startOfLine,endOfLine-startOfLine))
		   << "</span><br>";
		if (ranges.size() == 0 && mainToken) {
			ss << escaped(std::string(mainToken.start-startOfLine, ' '))
			   << "<span style=\"color: blue\">"
			   << "^"
			   << escaped(std::string(std::max<size_t>(mainToken.length, 1)-1, '~'))
			   << "</span>";
		} else {
			std::string i(endOfLine - startOfLine, ' ');
			for (auto r: ranges)
				for (int x=std::max<int>(startOfLine, r.lo); x < std::min<int>(endOfLine, r.hi); ++x) 
					i[x-startOfLine] = '~';
			if (mainToken)
				i[std::max<int>(size_t(0), std::min<int>(mainToken.start + (mainToken.length-1) / 2 - startOfLine, endOfLine-startOfLine-1))] = '^';
			ss << "<span style=\"color: blue\">" << escaped(i) << "</span>";
		}
		interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
	}


	virtual void report(rf::MsgType type, std::string message) override {
		std::stringstream ss;
		switch (type) {
		case rf::MsgType::error:
			ss << "<span style=\"color: red\">error</span>: ";
			break;
		case rf::MsgType::warning:
			ss << "<span style=\"color: yellow\">warning</span>: ";
		default:
			break;
		}
		ss << " " << escaped(message);
		interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
	}

	virtual void print(Type type, std::string repr) override {
		std::stringstream ss;
		ss /*<< "<span style=\"color: green\">=&nbsp;</span>" */
			<< escaped(repr);
		interperter->doDisplay(QString::fromUtf8(ss.str().c_str()));
	}

	QString location(const char * name) {
		return settings->path + "/" + name + ".rdb";
	}

	virtual void saveRelation(rm_object * o, const char * name) override {
		QDir d;
		d.mkpath(settings->path);
		rasmus::stdlib::saveRelationToFile(o, location(name).toUtf8().data());
	}

	virtual rm_object * loadRelation(const char * name) override {
		rm_object * res = rasmus::stdlib::loadRelationFromFile(location(name).toUtf8().data());
		return res;
	}
	
	virtual bool hasRelation(const char * name) override { 
		
		return llvm::sys::fs::exists(location(name).toUtf8().data());
	}

private:
	Interpreter * interperter;
	Settings * settings;
};


class InterpreterPrivate {

public:
	std::shared_ptr<rasmus::frontend::Interperter> interpreter;
	std::shared_ptr<rasmus::frontend::Callback> callback;

	InterpreterPrivate(Interpreter * interperter, Settings * settings) {
		int options = 0; // dump no info on terminal.
		callback = std::make_shared<GuiCallBack>(interperter, settings);
		interpreter=rasmus::frontend::makeInterperter(callback);
		interpreter->setup(options, std::string("Interpreter"));    
	}
};

Interpreter::Interpreter(QObject *parent, Settings * settings) : QObject(parent) {
	d_ptr = new InterpreterPrivate(this, settings);
}

Interpreter::~Interpreter() {
	delete d_ptr;
}

void Interpreter::doDisplayRelation(rs::Relation * r) {
	r->incref();
	emit displayRelation(r);
}

void Interpreter::environmentChanged(const char * name) {
	emit updateEnvironment(name);
}

void Interpreter::enterRelationToEnvironment(rm_object * rel, const char * name) {
	d_ptr->callback->saveRelation(rel, name);
	d_ptr->callback->loadRelation(name);
}

void Interpreter::cancel() {
	d_ptr->interpreter->cancel();
	emit complete();
}

void Interpreter::run(QString line) {
	emit bussy(true);
	d_ptr->interpreter->runLine(line.toUtf8().data());
	emit bussy(false);
	if (d_ptr->interpreter->complete()) {
		emit complete();
	} else {
		emit incomplete();
	}
}

void Interpreter::runContent(QString name, QString content) {
	emit bussy(true);
	d_ptr->interpreter->runContent(name.toUtf8().data(), content.toUtf8().data());
	emit bussy(false);
	if (d_ptr->interpreter->complete()) {
		emit complete();
	} else {
		emit incomplete();
	}
}

void Interpreter::getCurrentEnvironment() {
	emit bussy(false);
}

void Interpreter::doDisplay(QString string) {
	emit display(string);
}

