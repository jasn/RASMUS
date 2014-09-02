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

#ifndef __SRC_GUI_HIGHLIGHTER_H__
#define __SRC_GUI_HIGHLIGHTER_H__

#include <QTextDocument>
#include <QThread>
#include <QSyntaxHighlighter>
#include <map>
#include <vector>

class QTextDocument;
class Settings;

enum class IssueType {WARNING,ERROR};

struct Issue {
	size_t block;
	size_t start;
	size_t end;
	std::string message;
	IssueType type;
};

class Intellisense: public QObject {
	Q_OBJECT
public slots:
	void process(std::vector<std::string> * blocks);
signals:
	void issues(std::vector<Issue> * issues);
};


class Highlighter : public QSyntaxHighlighter {
	Q_OBJECT

public:
	Highlighter(QTextDocument *parent = 0);
	~Highlighter();

	std::string getIssue(size_t block, size_t index);
 
protected:
	void highlightBlock(const QString &text);

signals:
	void runIntellisense(std::vector<std::string> * blocks);

public slots:
	void registerIssues(std::vector<Issue> * issues);

	void updateSettings(Settings *);
private:
	void doIntellisense();

	QThread intellisenseThread;
	Intellisense * intellisense;
	std::map<size_t, std::vector<Issue> > issues;
	bool intellinensing;
	bool upToDate;
	bool noIntelli;
	Settings * settings;
};


#endif //__SRC_GUI_HIGHLIGHTER_H__
