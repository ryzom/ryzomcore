// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "action_editor.h"
#include "nel/gui/proc.h"

namespace GUIEditor
{
	ActionEditor::ActionEditor( QWidget *parent )
	{
		setupUi( this );
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOkButtonClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
	}

	ActionEditor::~ActionEditor()
	{
	}

	void ActionEditor::setCurrentAction( NLGUI::CProcAction *action )
	{
		currentAction = action;
		handlerEdit->setText( currentAction->Action.c_str() );
		handlerEdit->setEnabled( false );
		paramsEdit->setText( currentAction->Parameters.c_str() );
		condEdit->setText( currentAction->Conditions.c_str() );
	}

	void ActionEditor::clear()
	{
		currentAction = NULL;
		handlerEdit->clear();
		paramsEdit->clear();
		condEdit->clear();
	}

	void ActionEditor::onOkButtonClicked()
	{
		currentAction->Parameters = paramsEdit->text().toUtf8().constData();
		currentAction->Conditions = condEdit->text().toUtf8().constData();
		hide();
	}
}
