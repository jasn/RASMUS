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

#include <QToolTip>
#include "codeTextEdit.hh"
#include "highlighter.hh"
#include <iostream>

bool CodeTextEdit::event(QEvent* event) {
	if (event->type() != QEvent::ToolTip ||
		!highlighter) return QPlainTextEdit::event(event);
	
	QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
	QTextCursor cursor = cursorForPosition(helpEvent->pos());
	std::string msg=highlighter->getIssue(cursor.blockNumber(), cursor.positionInBlock());

	if (msg.empty()) {
		QToolTip::hideText();
		return true;
	}
	
	QToolTip::showText(helpEvent->globalPos(), QString::fromUtf8(msg.c_str()));
	return true;
}
