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


#include "proc_list.h"
#include "proc_editor.h"
#include <QMessageBox>
#include <QInputDialog>
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	ProcList::ProcList( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		procEditor = new ProcEditor;

		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOkButtonClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditButtonClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddButtonClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveButtonClicked() ) );
	}

	ProcList::~ProcList()
	{
		delete procEditor;
	}

	void ProcList::clear()
	{
		procList->clear();
		procEditor->clear();
	}

	void ProcList::onGUILoaded()
	{
		setupProcList();
	}

	void ProcList::onOkButtonClicked()
	{
		hide();
	}

	void ProcList::onEditButtonClicked()
	{
		IParser *parser = CWidgetManager::getInstance()->getParser();
		QListWidgetItem *item = procList->item( procList->currentRow() );
		if( item == NULL )
			return;

		CProcedure *proc = parser->getProc( item->text().toUtf8().constData() );
		if( proc == NULL )
			return;

		procEditor->setCurrentProc( item->text() );
		procEditor->show();
	}

	void ProcList::onAddButtonClicked()
	{
		bool ok;
		QString newProc =
			QInputDialog::getText( this,
								tr( "Adding a new procedure" ),
								tr( "Please specify the name of the new procedure" ),
								QLineEdit::Normal,
								QString(),
								&ok );
		if( ok )
		{
			IParser *parser = CWidgetManager::getInstance()->getParser();
			if( !parser->addProc( newProc.toUtf8().constData() ) )
			{
				QMessageBox::critical( this,
									tr( "Cannot add new procedure" ),
									tr( "There was an error adding the new procedure" ) );
			}
			procList->addItem( newProc );
			procList->sortItems();
		}
	}

	void ProcList::onRemoveButtonClicked()
	{
		IParser *parser = CWidgetManager::getInstance()->getParser();
		QListWidgetItem *item = procList->item( procList->currentRow() );
		if( item == NULL )
			return;

		QMessageBox::StandardButton button =
			QMessageBox::question( this, 
								tr( "Removing a procedure" ),
								tr( "Are you sure you want to remove '%1'" ).arg( item->text() ),
								QMessageBox::Yes | QMessageBox::Cancel );

		if( button != QMessageBox::Yes )
			return;
		
		if( !parser->removeProc( item->text().toUtf8().constData() ) )
			return;
		item = procList->takeItem( procList->currentRow() );
		delete item;
	}

	void ProcList::setupProcList()
	{
		procList->clear();

		const TProcedureMap& procMap =
			CWidgetManager::getInstance()->getParser()->getProcMap();

		TProcedureMap::const_iterator itr;
		for( itr = procMap.begin(); itr != procMap.end(); ++itr )
		{
			procList->addItem( itr->first.c_str() );
		}
	}
}

