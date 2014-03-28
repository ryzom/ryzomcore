// Object Viewer Qt Material Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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

#include "material_property_editor.h"

namespace MaterialEditor
{
	MatPropEditWidget::MatPropEditWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();
	}

	MatPropEditWidget::~MatPropEditWidget()
	{
	}

	void MatPropEditWidget::getProperty( MaterialProperty &prop )
	{
		prop.prop  = propertyEdit->text();
		prop.label = labelEdit->text();
		prop.type  = typeCB->currentText();
	}

	void MatPropEditWidget::setProperty( const MaterialProperty &prop )
	{
		propertyEdit->setText( prop.prop );
		labelEdit->setText( prop.label );
		int i = typeCB->findText( prop.type );
		if( i != -1 )
			typeCB->setCurrentIndex( i );

	}

	void MatPropEditWidget::clear()
	{
		propertyEdit->clear();
		labelEdit->clear();
		typeCB->setCurrentIndex( 0 );
	}

	void MatPropEditWidget::onOKClicked()
	{
		close();
		Q_EMIT okClicked();
	}

	void MatPropEditWidget::onCancelClicked()
	{
		close();
	}

	void MatPropEditWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	}
}

