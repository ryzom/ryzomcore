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
#include "action_list.h"
#include <QMessageBox>
#include <QInputDialog>
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
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditButtonClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddButtonClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveButtonClicked() ) );
		connect( upButton, SIGNAL( clicked( bool ) ), this, SLOT( onUpButtonClicked() ) );
		connect( downButton, SIGNAL( clicked( bool ) ), this, SLOT( onDownButtonClicked() ) );

		alist = new ActionList();
	}

	ProcEditor::~ProcEditor()
	{
		delete alist;
		alist = NULL;

		delete actionEditor;
		actionEditor = NULL;
	}

	void ProcEditor::setCurrentProc( const QString &name )
	{
		actionList->clear();

		currentProc = name;
		IParser *parser = CWidgetManager::getInstance()->getParser();
		CProcedure *proc = parser->getProc( name.toUtf8().constData() );
		
		std::vector< CProcAction >::const_iterator itr;
		for( itr = proc->Actions.begin(); itr != proc->Actions.end(); ++itr )
		{
			actionList->addItem( itr->Action.c_str() );
		}

		setWindowTitle( QString( "Procedure Editor - %1" ).arg( currentProc ) );
	}

	void ProcEditor::clear()
	{
		actionList->clear();
		actionEditor->clear();
	}

	void ProcEditor::onEditButtonClicked()
	{
		int row = actionList->currentRow();
		QListWidgetItem *item = actionList->item( row );
		if( item == NULL )
			return;

		CProcedure *proc =
			CWidgetManager::getInstance()->getParser()->getProc( currentProc.toUtf8().constData() );
		if( proc == NULL )
			return;

		actionEditor->setCurrentAction( &( proc->Actions[ row ] )  );
		actionEditor->show();
	}

	void ProcEditor::onAddButtonClicked()
	{
		alist->load();
		int result = alist->exec();
		
		if( result == QDialog::Accepted )
		{
			QString name = alist->getSelectedAction();

			CProcedure *proc =
				CWidgetManager::getInstance()->getParser()->getProc( currentProc.toUtf8().constData() );
			if( proc != NULL )
			{
				proc->addAction( name.toUtf8().constData() );
				actionList->addItem( name );
			}
			else
				nlinfo( "Cannot find procedure %s", currentProc.toUtf8().constData() );
		}
	}

	void ProcEditor::onRemoveButtonClicked()
	{
		QListWidgetItem *item = actionList->item( actionList->currentRow() );
		if( item == NULL )
			return;

		QMessageBox::StandardButton button =
			QMessageBox::question( this, 
								tr( "Removing an Action" ),
								tr( "Are you sure you want to remove '%1'" ).arg( item->text() ),
								QMessageBox::Yes | QMessageBox::Cancel );

		if( button != QMessageBox::Yes )
			return;

		CProcedure *proc = 
			CWidgetManager::getInstance()->getParser()->getProc( currentProc.toUtf8().constData() );

		if( proc != NULL )
		{
			if( !proc->removeAction( static_cast< uint32 >( actionList->currentRow() ) ) )
				nlinfo( "Action #%d not found in procedure %s.", actionList->currentRow(), currentProc.toUtf8().constData() );
			item = actionList->takeItem( actionList->currentRow() );
			delete item;
		}
		else
			nlinfo( "Cannot find procedure %s", currentProc.toUtf8().constData() );
	}

	void ProcEditor::onUpButtonClicked()
	{
		int row = actionList->currentRow();
		if( row == 0 )
			return;
		QListWidgetItem *item = actionList->item( row );
		if( item == NULL )
			return;

		QListWidgetItem *prevItem = actionList->item( row - 1 );
		if( prevItem == NULL )
			return;

		CProcedure *proc = CWidgetManager::getInstance()->getParser()->getProc( currentProc.toUtf8().constData() );
		if( proc == NULL )
			return;

		if( !proc->swap( row - 1, row ) )
			return;

		swapListItems( row - 1, row );
		actionList->setCurrentRow( row - 1 );
	}

	void ProcEditor::onDownButtonClicked()
	{
		int row = actionList->currentRow();
		if( row == ( actionList->count() - 1 ) )
			return;
		QListWidgetItem *item = actionList->item( row );
		if( item == NULL )
			return;
		QListWidgetItem *nextItem = actionList->item( row + 1 );
		if( nextItem == NULL )
			return;

		CProcedure *proc = CWidgetManager::getInstance()->getParser()->getProc( currentProc.toUtf8().constData() );
		if( proc == NULL )
			return;

		if( !proc->swap( row, row + 1 ) )
			return;

		swapListItems( row, row + 1 );
		actionList->setCurrentRow( row + 1 );
	}

	void ProcEditor::swapListItems( int row1, int row2 )
	{
		if( row1 == row2 )
			return;
		if( row1 >= actionList->count() )
			return;
		if( row2 >= actionList->count() )
			return;

		if( row1 > row2 ){
			std::swap( row1, row2 );
		}

		QListWidgetItem *item1 = actionList->item( row1 );
		if( item1 == NULL )
			return;
		QListWidgetItem *item2 = actionList->item( row2 );
		if( item2 == NULL )
			return;

		actionList->takeItem( row1 );
		actionList->takeItem( row2 - 1 );
		actionList->insertItem( row1, item2 );
		actionList->insertItem( row2, item1 );
	}
}