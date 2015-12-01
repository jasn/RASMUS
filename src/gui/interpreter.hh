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

/**
 * This class is responsible for the communication between
 * rasmus::frontend classes and the GUI.
 */
class Interpreter : public QObject {
	Q_OBJECT
  
public:

	/**
	 * Sets up a rasmus::frontend:interperter.
	 * @param parent is the parent in the Qt hierarchy.
	 * @param settings is necessary since it is responsible for e.g. path variable.
	 */
	Interpreter(QObject *parent, Settings * settings);
	~Interpreter();

	/**
	 * Whenever a global RASMUS variable is created/deleted/changed this 
	 * function needs to be called with the name of the variable.
	 * This function is thus called via a CallBack mechanism.
	 * See GuiCallBack for more information.
	 * @param name is the name of the variable.
	 */
	void environmentChanged(const char *name);

	/**
	 * Since it is possible to load relations in the GUI, there needs to be
	 * a way of telling the rasmus::frontend:Interperter to load that relation.
	 * That is the purpose of this function.
	 */
	void enterRelationToEnvironment(rm_object * rel, const char * name);

	/**
	 * Whenever the result of a RASMUS expression yields a table, it is up to
	 * the GUI to draw it. This is called when the rasmus::frontends needs to
	 * print a relation.
	 * @param r is the relation to be displayed.
	 */
	void doDisplayRelation(rasmus::stdlib::Relation *r);

	/**
	 * Query rasmus::frontend::Interperter whether there exists a relation
	 * with the name relationName.
	 * @param name is the name of the relation we wish to know whether exists.
	 * @return true if relationName exists as a relation and false otherwise.
	 */
	bool relationExists(QString relationName);

	/**
	 * Called whenever the rasmus::frontend needs to display a message
	 * or computation result.
	 * @param string is the message to be displayed.
	 */
	void doDisplay(QString string);


public slots:

	/**
	 * When the user enters some code to be executed, this slot has to
	 * receive a signal with the code.
	 * Console::run(QString) is the signal that this slot receives.
	 * @param line is the code that needs to be executed.
	 */
	void run(QString line);

	/**
	 * When a computation needs to be cancelled, this slot should receive a signal.
	 * This slot communicates to rasmus::frontend::Interperter that the user wishes
	 * to stop the current computation.
	 * This slot receives the Console::cancel() signal.
	 */
	void cancel();

	/**
	 * It is sometimes necessary to execute RASMUS expressions
	 */
	void runContent(QString name, QString content);

	/**
	 * This slot is used by the GUI to change the environment.
	 * Whenever a variable needs to be deleted, it needs to be unset
	 * via a RASMUS expression, since the rasmus::frontend
	 * is responsible for maintaining the environment.
	 *
	 * @param name is the variable that needs to be unset.
	 */
	void unset(QString name);
	
signals:
	/**
	 * This signal is emitted if the RASMUS code was incomplete,
	 * i.e. it ended with a unexpected EOF.
	 */
	void incomplete();
	/**
	 * This signal is emitted when the RASMUS code is complete.
	 * i.e. it gave no parse errors.
	 */
	void complete();

	/**
	 * This signal is emitted when RASMUS code is done executing,
	 * and it needs to display a string.
	 * The string could be warning/error/result of expression.
	 */
	void display(QString string);

	/**
	 * This signal is emitted by Interpreter::environmentChanged(const char*)
	 * @param name is the variable that has changed or been introduced.
	 */
	void updateEnvironment(QString name);

	/**
	 * This signal is emitted by Interpreter::run(QString)
	 * This is how the interpreter tells that it is currently
	 * evaluating a RASMUS expression or that it is done.
	 * @param b is true if the interpreter is done and false if it
	 * is starting an evalutation.
	 */
	void bussy(bool b);

	/**
	 * This signal is similar to Interpreter::display(QString), but instead
	 * of displaying a simple message, the interpreter needs to be display a relation
	 * which might be handled differently from display a regualr message.
	 */
	void displayRelation(rasmus::stdlib::Relation *);

private:

	InterpreterPrivate *d_ptr; /**< Qt pattern to ensure binary compatibility
								  and faster compilation times */

};

#endif //__INTERPRETER_GUI_HH_GUARD
