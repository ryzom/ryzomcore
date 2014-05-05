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

#include "new_property_widget.h"
#include "widget_info_tree_node.h"
#include <qmessagebox.h>


namespace GUIEditor
{
	NewPropertyWidget::NewPropertyWidget( QWidget *parent ) :
	QWidget( parent )
	{
		widgetInfoTreeNode = NULL;
		setupUi( this );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
	}

	NewPropertyWidget::~NewPropertyWidget()
	{
	}

	void NewPropertyWidget::onAddClicked()
	{
		if( nameEdit->text().isEmpty() )
		{
			QMessageBox::warning( this,
								tr( "Adding a new property" ),
								tr( "You need to specify a name for the new property!" ),
								QMessageBox::Ok
								);
			return;
		}

		if( !checkName() )
		{
			QMessageBox::warning( this,
								tr( "Adding a new property" ),
								tr( "A property with that name already exists in the widget!" ),
								QMessageBox::Ok
								);
			return;
		}

		if( !checkDefaultValue() )
		{
			QMessageBox::warning( this,
								tr( "Adding a new property" ),
								tr( "You need to specify a valid default value." ),
								QMessageBox::Ok
								);
			return;
		}

		SPropEntry prop;
		prop.propName    = nameEdit->text().toUtf8().constData();
		prop.propType    = typeCB->currentText().toUtf8().constData();
		prop.propDefault = defvalEdit->text().toUtf8().constData();
		widgetInfoTreeNode->addPropertyToAll( prop );

		widgetInfoTreeNode = NULL;
		hide();

		Q_EMIT propertyAdded();
	}


	bool NewPropertyWidget::checkName() const
	{
		if( widgetInfoTreeNode == NULL )
			return false;

		if( widgetInfoTreeNode->hasProperty( nameEdit->text().toUtf8().constData() ) )
			return false;

		return true;
	}

	bool NewPropertyWidget::checkDefaultValue() const
	{
		if( widgetInfoTreeNode == NULL )
			return false;

		if( ( typeCB->currentText() != "string" ) && ( defvalEdit->text().isEmpty() ) )
			return false;

		if( typeCB->currentText() == "bool" )
		{
			if( defvalEdit->text().toUpper() == "TRUE" )
				return true;
			if( defvalEdit->text().toUpper() == "FALSE" )
				return true;
			return false;
		}
		else
		if( typeCB->currentText() == "int" )
		{
			bool ok = false;
			defvalEdit->text().toInt( &ok );
			if( ok )
				return true;
			return false;
		}

		return true;
	}

}

