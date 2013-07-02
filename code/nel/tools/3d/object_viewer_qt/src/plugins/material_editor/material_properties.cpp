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

#include "material_properties.h"
#include "material_property_editor.h"
#include "nel3d_interface.h"

namespace MaterialEditor
{

	MatPropWidget::MatPropWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		matPropEditWidget = new MatPropEditWidget();
		setupConnections();
		edit = false;
		proxy = NULL;
	}

	MatPropWidget::~MatPropWidget()
	{
		clear();

		delete matPropEditWidget;
		matPropEditWidget = NULL;
	}

	void MatPropWidget::load( CRenderPassProxy *proxy )
	{
		clear();
		this->proxy = new CRenderPassProxy( *proxy );

		std::string n;
		proxy->getName( n );
		nameEdit->setText( n.c_str() );

		std::vector< SMatProp > v;
		proxy->getProperties( v );

		std::vector< SMatProp >::iterator itr = v.begin();
		while( itr != v.end() )
		{
			SMatProp &mp = *itr;
			QTreeWidgetItem *item = new QTreeWidgetItem();

			item->setData( 0, Qt::DisplayRole, QString( mp.id.c_str() ) );
			item->setData( 1, Qt::DisplayRole, QString( mp.label.c_str() ) );
			
			QString type = SMatProp::typeIdToString( mp.type ).c_str();
			item->setData( 2, Qt::DisplayRole, type );

			treeWidget->addTopLevelItem( item );

			++itr;
		}
	}

	void MatPropWidget::clear()
	{
		treeWidget->clear();
		nameEdit->clear();
		if( this->proxy != NULL )
		{
			delete this->proxy;
			this->proxy = NULL;
		}
	}

	void MatPropWidget::onOKClicked()
	{
		if( proxy != NULL )
		{
			std::vector< SMatProp > v;
			SMatProp p;
			QTreeWidgetItem *item = NULL;
			std::string s;

			for( int i = 0; i < treeWidget->topLevelItemCount(); i++ )
			{
				item = treeWidget->topLevelItem( i );
				p.id = item->text( 0 ).toUtf8().data();
				p.label = item->text( 1 ).toUtf8().data();
				
				s = item->text( 2 ).toUtf8().data();
				p.type = SMatProp::typeStringToId( s );

				v.push_back( p );
			}

			proxy->setProperties( v );
		}

		clear();
		close();
	}

	void MatPropWidget::onCancelClicked()
	{
		clear();
		close();
	}

	void MatPropWidget::onAddClicked()
	{
		edit = false;
		matPropEditWidget->clear();
		matPropEditWidget->show();
	}

	void MatPropWidget::onEditClicked()
	{
		QTreeWidgetItem *item = treeWidget->currentItem();
		if( item == NULL )
			return;

		MaterialProperty prop;
		prop.prop  = item->data( 0, Qt::DisplayRole ).toString();
		prop.label = item->data( 1, Qt::DisplayRole ).toString();
		prop.type  = item->data( 2, Qt::DisplayRole ).toString();
		
		edit = true;
		matPropEditWidget->setProperty( prop );
		matPropEditWidget->show();
	}

	void MatPropWidget::onRemoveClicked()
	{
		QTreeWidgetItem *item = treeWidget->currentItem();
		if( item == NULL )
			return;

		delete item;
	}

	void MatPropWidget::onEditorOKClicked()
	{
		MaterialProperty prop;
		matPropEditWidget->getProperty( prop );

		if( edit )
		{
			QTreeWidgetItem *item = treeWidget->currentItem();
			item->setData( 0, Qt::DisplayRole, prop.prop );
			item->setData( 1, Qt::DisplayRole, prop.label );
			item->setData( 2, Qt::DisplayRole, prop.type );
		}
		else
		{
			QTreeWidgetItem *item = new QTreeWidgetItem();
			item->setData( 0, Qt::DisplayRole, prop.prop );
			item->setData( 1, Qt::DisplayRole, prop.label );
			item->setData( 2, Qt::DisplayRole, prop.type );
			treeWidget->addTopLevelItem( item );
		}

	}

	void MatPropWidget::setupConnections()
	{
		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
		connect( editButton, SIGNAL( clicked( bool ) ), this, SLOT( onEditClicked() ) );
		connect( removeButton, SIGNAL( clicked( bool ) ), this, SLOT( onRemoveClicked() ) );
		connect( matPropEditWidget, SIGNAL( okClicked() ), this, SLOT( onEditorOKClicked() ) );
	}

}

