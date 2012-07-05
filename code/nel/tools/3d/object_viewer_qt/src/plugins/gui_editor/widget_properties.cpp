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

#include "widget_properties.h"

namespace GUIEditor{
	CWidgetProperties::CWidgetProperties( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		connect( closeButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );
	}

	CWidgetProperties::~CWidgetProperties()
	{
	}

	void CWidgetProperties::setupWidgetInfo( std::map< std::string, SWidgetInfo > *info )
	{
		widgetInfo = info;
		for( std::map< std::string, SWidgetInfo >::iterator itr = info->begin(); itr != info->end(); ++itr ){
			widgetList->addItem( itr->first.c_str() );
		}

		onListSelectionChanged( 0 );
		connect( widgetList, SIGNAL( currentRowChanged( int ) ), this, SLOT( onListSelectionChanged( int ) ) );
	}

	void CWidgetProperties::onListSelectionChanged( int i )
	{
		if( i >= widgetList->count() )
			return;

		QListWidgetItem *item = widgetList->item( i );
		setPropsOf( item->text().toStdString().c_str() );
	}

	void CWidgetProperties::setPropsOf( const char *name )
	{
		std::map< std::string, SWidgetInfo >::iterator itr =
			widgetInfo->find( name );

		if( itr == widgetInfo->end() )
			return;

		widgetPropTree->clear();

		std::vector< SPropEntry > &v = itr->second.props;
		for( std::vector< SPropEntry >::iterator itr2 = v.begin(); itr2 != v.end(); ++itr2 )
		{
			SPropEntry e = *itr2;
			QTreeWidgetItem *item = new QTreeWidgetItem;
			item->setText( 0, e.propName.c_str() );
			item->setText( 1, e.propType.c_str() );
			item->setText( 2, e.propDefault.c_str() );
			widgetPropTree->addTopLevelItem( item );
		}
	}
}


