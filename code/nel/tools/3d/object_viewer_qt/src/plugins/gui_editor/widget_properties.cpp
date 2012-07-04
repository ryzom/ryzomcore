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
#include <map>
#include <vector>

namespace
{
	struct SPropEntry
	{
		std::string propName;
		std::string propType;
		std::string propDefault;

		static SPropEntry create( const char *propname, const char *proptype, const char *propdefault )
		{
			SPropEntry entry;
			entry.propName = propname;
			entry.propType = proptype;
			entry.propDefault = propdefault;
			return entry;
		}
	};

	std::map< std::string, std::vector< SPropEntry > >  props;
}

namespace GUIEditor{
	
	CWidgetProperties::CWidgetProperties( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );

		widgetList->addItem( QString( "InterfaceElement" ) );
		widgetList->addItem( QString( "CtrlBase" ) );

		props[ "InterfaceElement" ] = std::vector< SPropEntry >();
		props[ "CtrlBase" ] = std::vector< SPropEntry >();

		std::map< std::string, std::vector< SPropEntry > >::iterator itr =
			props.find( "InterfaceElement" );
		if( itr != props.end() )
		{
			itr->second.push_back( SPropEntry::create( "id", "string", "ie" ) );
			itr->second.push_back( SPropEntry::create( "active", "bool", "false" ) );
		}

		itr = props.find( "CtrlBase" );
		if( itr != props.end() )
		{
			itr->second.push_back( SPropEntry::create( "on_tooltip", "string", "tooltip" ) );
			itr->second.push_back( SPropEntry::create( "on_tooltip_params", "string", "params" ) );
		}

		connect( closeButton, SIGNAL( clicked(bool) ), this, SLOT( hide() ) );
		connect( widgetList, SIGNAL( currentRowChanged( int ) ), this, SLOT( onListSelectionChanged( int ) ) );

	}

	CWidgetProperties::~CWidgetProperties()
	{
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
		std::map< std::string, std::vector< SPropEntry > >::iterator itr =
			props.find( name );

		if( itr == props.end() )
			return;

		widgetPropTree->clear();

		std::vector< SPropEntry > &v = itr->second;
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


