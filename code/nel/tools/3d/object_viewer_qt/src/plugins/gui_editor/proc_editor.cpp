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


#include "proc_editor.h"
#include "action_editor.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	ProcEditor::ProcEditor( QWidget *parent )
	{
		setupUi( this );
		actionEditor = new ActionEditor;
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), actionEditor, SLOT( show() ) );
	}

	ProcEditor::~ProcEditor()
	{
		delete actionEditor;
		actionEditor = NULL;
	}

	void ProcEditor::setCurrentProc( const QString &name )
	{
		actionList->clear();

		currentProc = name;
		IParser *parser = CWidgetManager::getInstance()->getParser();
		CProcedure *proc = parser->getProc( name.toStdString() );
		
		std::vector< CProcAction >::const_iterator itr;
		for( itr = proc->Actions.begin(); itr != proc->Actions.end(); ++itr )
		{
			actionList->addItem( itr->Action.c_str() );
		}

		setWindowTitle( QString( "Procedure Editor - %1" ).arg( currentProc ) );
	}
}