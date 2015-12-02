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

#ifndef __SRC_GUI_CODETEXTEDIT_HH__
#define __SRC_GUI_CODETEXTEDIT_HH__

#include <QPlainTextEdit>

class Highlighter;

class CodeTextEdit: public QPlainTextEdit {
public:
	explicit CodeTextEdit(QWidget * parent): QPlainTextEdit(parent), highlighter(nullptr) {}

	bool event(QEvent* event) override;

	Highlighter * highlighter;
};

#endif //__SRC_GUI_CODETEXTEDIT_HH__
