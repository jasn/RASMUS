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

Highlighter::Highlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent) {
	{
		HighlightingRule rule;
		QTextCharFormat keywordFormat;
		keywordFormat.setForeground(Qt::darkBlue);
		keywordFormat.setFontWeight(QFont::Bold);
		QStringList keywordPatterns;
		keywordPatterns << "\\bfunc\\b" << "\\bInt\\b" << "\\bAny\\b"
						<< "\\bFunc\\b" << "\\bBool\\b" << "\\bText\\b"
						<< "\\b?-Int\\b" << "\\b?-Bool\\b" << "\\b?-Text\\b"
						<< "\\bRel\\b" << "\\bTup\\b" << "\\b#\\b"
						<< "\\bif\\b" << "\\bfi\\b" << "\\bend\\b"
						<< "\\brel\\b" << "\\btup\\b";

		foreach (const QString &pattern, keywordPatterns) {
			rule.pattern = QRegExp(pattern);
			rule.format = keywordFormat;
			highlightingRules.append(rule);
		}
	}
	{
		HighlightingRule rule;
		QTextCharFormat quotationFormat;
		quotationFormat.setForeground(Qt::darkGreen);
		rule.pattern = QRegExp("\".*\"");
		rule.format = quotationFormat;
		highlightingRules.append(rule);
	}
	{
		HighlightingRule rule;
		QTextCharFormat singleLineCommentFormat;
		singleLineCommentFormat.setForeground(Qt::red);
		rule.pattern = QRegExp("//[^\n]*");
		rule.format = singleLineCommentFormat;
		highlightingRules.append(rule);
	}
}

void Highlighter::highlightBlock(const QString &text) {
	foreach (const HighlightingRule &rule, highlightingRules) {
		QRegExp expression(rule.pattern);
		int index = expression.indexIn(text);
		while (index >= 0) {
			int length = expression.matchedLength();
			setFormat(index, length, rule.format);
			index = expression.indexIn(text, index + length);
		}
	}
	setCurrentBlockState(0);
}
