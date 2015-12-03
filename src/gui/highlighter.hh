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

/**
 * This class is reponsible for highlighting code in the CodeTextEdit window.
 * The highlighter finds keywords and changes their color according to settings.
 * If intellisense is enable the highlighter also marks code that gives parse
 * errors and rudimentary type warnings.
 */
class Highlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	/**
	 * @param parent is a pointer to the CodeTextEdit window to highlight.
	 */
	Highlighter(QTextDocument *parent = 0, Settings * settings = 0);
	~Highlighter();

	/**
	 * Used for retrieving issues in the document this hlighter is applied to.
	 * @param block is the block number in the document.
	 * @param index is the offset inside the block.
	 * @return If there is an issue whose range covers the position
	 * (\p block, \p index) it is returned. Otherwise an empty string ("") is
	 * returned.
	 */
	std::string getIssue(size_t block, size_t index);
 
protected:
	/**
	 * Called by the Qt framework to figure out how to highlight a block of 
	 * text from the document.
	 * @param text is the text that needs to have highlighting applied.
	 */
	void highlightBlock(const QString &text);

signals:
	/**
	 * This signal is issued by the highlighter to tell the intellisense
	 * that it needs to run.
	 * @param blocks a pointer to a list of strings that needs to have
	 * intellisense applied to them (i.e. the entire document).
	 */
	void runIntellisense(std::vector<std::string> * blocks);

public slots:
	/**
	 * When intellisense is enabled, this slot receives a signal to update
	 * how the code is highlighted based on the issues the parser returned.
	 * @param issues is a pointer to issues that were produced by the RASMUS
	 * parser and typechecker. The pointer should be deleted by registerIssues
	 * when finished.
	 */
	void registerIssues(std::vector<Issue> * issues);

	/**
	 * Whenever settings change, this function needs to be called with
	 * the new settings.
	 * @param s is a pointer to the new settings.
	 */
	void updateSettings(Settings *s);
private:
	/**
	 * Helper method for finding the strings that needs to have intellisensing
	 * applied. After finding the strings it signals the intellisense to
	 * Intellisense::process(std::vector<std::string> *b) the text.
	 */
	void doIntellisense();

	QThread intellisenseThread; /**< seperate thread for intellisense */
	Intellisense * intellisense; /**< object that performs intellisense */
	std::map<size_t, std::vector<Issue> > issues; /**< Things that needs special highlighting*/

	/* intellinensing indicates whether the intellisense is currently running */
	bool intellinensing;

	/* Once intellisense is started a number of edits can occur at the same time.
	   If there were no updates during the time it took the intellisense to
	   finish it does not need to run immediately again. Otherwise it needs to.
	   This variable indicates which is the case. True meaning run immediately again.
	 */
	bool upToDate;

	/* A rehighlight triggers an 'edit' event, which in turn triggers the
	   intellisense. To avoid getting an infinite loop we need to not start
	   intellisensing while rehighlightning. Setting this to false prevents
	   the intellisense from being started.
	 */
	bool noIntelli;

	/* Settings. Used for finding the colors for fonts, etc. */
	Settings * settings;
};


#endif //__SRC_GUI_HIGHLIGHTER_H__
