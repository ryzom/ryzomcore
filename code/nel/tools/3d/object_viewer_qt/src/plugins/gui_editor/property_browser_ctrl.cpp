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
#include "widget_info_tree.h"

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

	void CPropBrowserCtrl::setupWidgetInfo( CWidgetInfoTree *tree )
	{
		widgetInfo.clear();

		std::vector< std::string > names;
		tree->getNames( names );		

		std::vector< std::string >::const_iterator itr;
		for( itr = names.begin(); itr != names.end(); ++itr )
		{
			CWidgetInfoTreeNode *node = tree->findNodeByName( *itr );
			const SWidgetInfo &w = node->getInfo();
			widgetInfo[ w.GUIName ] = w;
		}
	}

	void CPropBrowserCtrl::clear()
	{
		browser->clear();
		disconnect( propertyMgr, SIGNAL( propertyChanged( QtProperty* ) ),
			this, SLOT( onPropertyChanged( QtProperty* ) ) );
	}

	void CPropBrowserCtrl::onSelectionChanged( std::string &id )
	{
		if( browser == NULL )
			return;

		disconnect( propertyMgr, SIGNAL( propertyChanged( QtProperty* ) ),
			this, SLOT( onPropertyChanged( QtProperty* ) ) );

		browser->clear();

		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( id );
		if( e == NULL )
			return;

		currentElement = id;

        std::string n = e->getClassName();

        setupProperties( n, e );
		connect( propertyMgr, SIGNAL( propertyChanged( QtProperty* ) ),
			this, SLOT( onPropertyChanged( QtProperty* ) ) );
	}

	void CPropBrowserCtrl::onPropertyChanged( QtProperty *prop )
	{
		QString propName = prop->propertyName();
		QString propValue = prop->valueText();

		// for some reason booleans cannot be extracted from a QtProperty :(
		if( propValue.isEmpty() )
		{
			QtVariantProperty *p = propertyMgr->variantProperty( prop );
			if( p != NULL )
				propValue = p->value().toString();
		}

		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( currentElement );
		if( e == NULL )
			return;
		e->setProperty( propName.toUtf8().constData(), propValue.toUtf8().constData() );
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

		QtVariantEditorFactory *factory = new QtVariantEditorFactory;
		browser->setFactoryForManager( propertyMgr, factory );
	}

	void CPropBrowserCtrl::setupProperty( const SPropEntry &prop, const CInterfaceElement *element )
	{
		QtVariantProperty *p = NULL;
		QVariant v;
		
		if( prop.propType == "string" )
		{
			p = propertyMgr->addProperty( QVariant::String, prop.propName.c_str() );
			v = element->getProperty( prop.propName ).c_str();
		}
		else
		if( prop.propType == "bool" )
		{
			p = propertyMgr->addProperty( QVariant::Bool, prop.propName.c_str() );
			bool value = false;
			NLMISC::fromString( element->getProperty( prop.propName ), value );
			v = value;
		}
		else
		if( prop.propType == "int" )
		{
			p = propertyMgr->addProperty( QVariant::Int, prop.propName.c_str() );
			sint32 value = 0;
			NLMISC::fromString( element->getProperty( prop.propName ), value );
			v = value;
		}

		if( p == NULL )
			return;

		p->setValue( v );
		browser->addProperty( p );
	}
}
