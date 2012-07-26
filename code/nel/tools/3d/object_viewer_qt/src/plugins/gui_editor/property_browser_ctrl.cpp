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


#include "property_browser_ctrl.h"
#include "../../3rdparty/qtpropertybrowser/QtVariantPropertyManager"
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include <typeinfo>

namespace GUIEditor
{
	CPropBrowserCtrl::CPropBrowserCtrl()
	{
		browser = NULL;
		propertyMgr = new QtVariantPropertyManager;
	}

	CPropBrowserCtrl::~CPropBrowserCtrl()
	{
		delete propertyMgr;
		propertyMgr = NULL;
		browser = NULL;
	}

	void CPropBrowserCtrl::setBrowser( QtTreePropertyBrowser *b )
	{
		browser = b;
	}

	void CPropBrowserCtrl::setupWidgetInfo( const std::map< std::string, SWidgetInfo > &info )
	{
		widgetInfo.clear();

		std::map< std::string, SWidgetInfo >::const_iterator itr;
		for( itr = info.begin(); itr != info.end(); ++itr )
		{
			const SWidgetInfo &w = itr->second;
			widgetInfo[ w.GUIName ] = w;
		}
	}

	void CPropBrowserCtrl::onSelectionChanged( std::string &id )
	{
		if( browser == NULL )
			return;
		browser->clear();

		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( id );
		if( e == NULL )
			return;

		std::string n;
		n = typeid( *e ).name();
		std::string::size_type i = n.find_last_of( ':' );
		if( i != std::string::npos )
			n = n.substr( i + 1, n.size() - 1 );

		setupProperties( n, e );
	}

	void CPropBrowserCtrl::setupProperties( const std::string &type, const CInterfaceElement *element )
	{
		std::map< std::string, SWidgetInfo >::iterator itr = widgetInfo.find( type );
		if( itr == widgetInfo.end() )
			return;
		SWidgetInfo &w = itr->second;

		std::vector< SPropEntry >::const_iterator pItr;
		for( pItr = w.props.begin(); pItr != w.props.end(); ++pItr )
		{
			const SPropEntry &prop = *pItr;
			setupProperty( prop, element );
		}
	}

	void CPropBrowserCtrl::setupProperty( const SPropEntry &prop, const CInterfaceElement *element )
	{
		QtVariantProperty *p = NULL;
		
		if( prop.propType == "string" )
		{
			p = propertyMgr->addProperty( QVariant::String, prop.propName.c_str() );
			p->setValue( element->getProperty( prop.propName ).c_str() );
		}
		else
		if( prop.propType == "bool" )
		{
			p = propertyMgr->addProperty( QVariant::Bool, prop.propName.c_str() );
			bool value = false;
			if( element->getProperty( prop.propName ) == "true" )
				value = true;

			p->setValue( value );
		}
		if( p == NULL )
			return;

		browser->addProperty( p );
	}
}