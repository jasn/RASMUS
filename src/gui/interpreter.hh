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
#ifndef __INTERPRETER_GUI_HH_GUARD
#define __INTERPRETER_GUI_HH_GUARD

#include <QObject>
#include <QString>
#include "relation_model.hh"

class rm_object;

namespace rasmus {
namespace stdlib {
class Relation;
}
}

class Settings;

class InterpreterPrivate;

class Interpreter : public QObject {
	Q_OBJECT
  
public:
	Interpreter(QObject *parent, Settings * settings);
	~Interpreter();
	void environmentChanged(const char *name);
	void enterRelationToEnvironment(rm_object * rel, const char * name);
	void doDisplayRelation(rasmus::stdlib::Relation *);
	bool relationExists(QString relationName);
	void doDisplay(QString string);
	void getCurrentEnvironment();


public slots:
  
	void run(QString line);
	void cancel();
	void runContent(QString name, QString content);
	void unset(QString name);
	void savePermutation(RelationModel * model);
	
signals:
	void incomplete();
	void complete();
	void display(QString string);
	void updateEnvironment(const char * name);
	void bussy(bool);

	void displayRelation(rasmus::stdlib::Relation *);

private:
	InterpreterPrivate *d_ptr;

};

#endif //__INTERPRETER_GUI_HH_GUARD
