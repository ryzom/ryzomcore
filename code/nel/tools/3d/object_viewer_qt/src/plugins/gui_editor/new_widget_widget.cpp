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

#include "new_widget_widget.h"
#include "widget_info_tree.h"
#include <qmessagebox.h>

namespace GUIEditor
{
	NewWidgetWidget::NewWidgetWidget( QWidget *parent ) :
	QWidget( parent )
	{
		widgetInfoTree = NULL;
		setupUi( this );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
	}

	NewWidgetWidget::~NewWidgetWidget()
	{
		widgetInfoTree = NULL;
	}

	void NewWidgetWidget::fillWidgetList( std::vector< std::string > &widgets )
	{
		ancestorCB->clear();

		std::vector< std::string >::const_iterator itr = widgets.begin();
		while( itr != widgets.end() )
		{
			ancestorCB->addItem( QString( itr->c_str() ) );
			++itr;
		}
	}


	void NewWidgetWidget::onAddClicked()
	{
		if( !checkNameField() )
		{
			QMessageBox::warning( this,
								tr( "Adding a new widget" ),
								tr( "The new widget's name is invalid!" ),
								QMessageBox::Ok 
								);
			return;
		}

		if( !checkNameDuplicate() )
		{
			QMessageBox::warning( this,
								tr( "Adding a new widget" ),
								tr( "That widget name already exists!" ),
								QMessageBox::Ok 
								);
			return;
		}

		addNewWidget();
		hide();
		Q_EMIT widgetAdded();
	}


	bool NewWidgetWidget::checkNameField()
	{
		if( nameEdit->text().isEmpty() )
			return false;

		return true;
	}

	bool NewWidgetWidget::checkNameDuplicate()
	{
		if( widgetInfoTree == NULL )
			return false;

		CWidgetInfoTreeNode *node = widgetInfoTree->findNodeByName( nameEdit->text().toUtf8().constData() );
		if( node != NULL )
			return false;

		return true;
	}


	void NewWidgetWidget::addNewWidget()
	{
		CWidgetInfoTreeNode *node = widgetInfoTree->findNodeByName( ancestorCB->currentText().toUtf8().constData() );
		if( node == NULL )
		{
			nlerror( "Ancestor %s doesn't exist! Aborting addition!", ancestorCB->currentText().toUtf8().constData() );
			return;
		}

		SWidgetInfo info;
		info.ancestor   = ancestorCB->currentText().toUtf8().constData();
		info.name       = nameEdit->text().toUtf8().constData();
		info.GUIName    = "C" + info.name;
		info.isAbstract = false;
		info.resolved   = true;
		node->addChild( info );
	}

}


