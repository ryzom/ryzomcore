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
#include "../../3rdparty/qtpropertybrowser/QtEnumPropertyManager"
#include "../../3rdparty/qtpropertybrowser/QtTreePropertyBrowser"
#include "../../3rdparty/qtpropertybrowser/QtEnumEditorFactory"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include <typeinfo>
#include "widget_info_tree.h"
#include <QList>

namespace
{

	class NelPosRef
	{
	public:

		enum NELPosRef
		{
			POSREF_BL   = 0,
			POSREF_BM   = 1,
			POSREF_BR   = 2,
			POSREF_ML   = 3,
			POSREF_MM   = 4,
			POSREF_MR   = 5,
			POSREF_TL   = 6,
			POSREF_TM   = 7,
			POSREF_TR   = 8
		};

		static int fromString( const std::string &s )
		{
			int r = -1;
		
			if( s == "BL" )
				r = POSREF_BL;
			else
			if( s == "BM" )
				r = POSREF_BM;
			else
			if( s == "BR" )
				r = POSREF_BR;
			else
			if( s == "ML" )
				r = POSREF_ML;
			else
			if( s == "MM" )
				r = POSREF_MM;
			else
			if( s == "MR" )
				r = POSREF_MR;
			else
			if( s == "TL" )
				r = POSREF_TL;
			else
			if( s == "TM" )
				r = POSREF_TM;
			else
			if( s == "TR" )
				r = POSREF_TR;

			return r;
		}

		static std::string toString( int value )
		{
			std::string v;
		
			switch( value )
			{
			case POSREF_BL: v = "BL"; break;
			case POSREF_BM: v = "BM"; break;
			case POSREF_BR: v = "BR"; break;
			case POSREF_ML: v = "ML"; break;
			case POSREF_MM: v = "MM"; break;
			case POSREF_MR: v = "MR"; break;
			case POSREF_TL: v = "TL"; break;
			case POSREF_TM: v = "TM"; break;
			case POSREF_TR: v = "TR"; break;
			}

			return v;
		}
	};


	class NelButtonType
	{
	public:
		
		enum NELButtonTypes
		{
			BUTTON_TYPE_PUSH = 0,
			BUTTON_TYPE_TOGGLE = 1,
			BUTTON_TYPE_RADIO = 2
		};

		static int fromString( const std::string &s )
		{
			int r = -1;

			if( s == "push_button" )
				r = BUTTON_TYPE_PUSH;
			else
			if( s == "toggle_button" )
				r = BUTTON_TYPE_TOGGLE;
			else
			if( s == "radio_button" )
				r = BUTTON_TYPE_RADIO;

			return r;
		}

		static std::string toString( int value )
		{
			std::string v;

			switch( value )
			{
			case BUTTON_TYPE_PUSH: v = "push_button"; break;
			case BUTTON_TYPE_TOGGLE: v = "toggle_button"; break;
			case BUTTON_TYPE_RADIO: v = "radio_button"; break;
			}

			return v;
		}

	};

	class NelTxtJustification
	{
	public:
		
		enum NELTxtJustification
		{
			TEXT_CLIPWORD,
			TEXT_DONTCLIPWORD,
			TEXT_JUSTIFIED
		};

		static int fromString( const std::string &s )
		{
			int r = -1;

			if( s == "clip_word" )
				r = TEXT_CLIPWORD;
			else
			if( s == "dont_clip_word" )
				r = TEXT_DONTCLIPWORD;
			else
			if( s == "justified" )
				r = TEXT_JUSTIFIED;

			return r;
		}

		static std::string toString( int value )
		{
			std::string v;

			switch( value )
			{
			case TEXT_CLIPWORD: v = "clip_word"; break;
			case TEXT_DONTCLIPWORD: v = "dont_clip_word"; break;
			case TEXT_JUSTIFIED: v = "justified"; break;
			}

			return v;
		}

	};

}

namespace GUIEditor
{

	CPropBrowserCtrl::CPropBrowserCtrl()
	{
		browser = NULL;
		propertyMgr = new QtVariantPropertyManager;
		enumMgr = new QtEnumPropertyManager;
	}

	CPropBrowserCtrl::~CPropBrowserCtrl()
	{
		delete enumMgr;
		enumMgr = NULL;
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
		disablePropertyWatchers();
		browser->clear();
	}

	void CPropBrowserCtrl::onSelectionChanged( std::string &id )
	{
		if( browser == NULL )
			return;

		disablePropertyWatchers();
		browser->clear();

		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( id );
		if( e == NULL )
		{
			enablePropertyWatchers();
			return;
		}

		currentElement = id;

        std::string n = e->getClassName();

        setupProperties( n, e );
		enablePropertyWatchers();
	}

	void CPropBrowserCtrl::onPropertyChanged( QtProperty *prop, const QVariant &v )
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
		
		if( v.type() == QVariant::Color )
		{
			QColor c = v.value< QColor >();
			QString val = "%1 %2 %3 %4";
			val = val.arg( c.red() ).arg( c.green() ).arg( c.blue() ).arg( c.alpha() );
			propValue = val;
		}

		CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( currentElement );
		if( e == NULL )
			return;
		e->setProperty( propName.toUtf8().constData(), propValue.toUtf8().constData() );
		
		
		// Make sure the changes are applied
		bool active = e->getActive();
		e->setActive( !active );
		e->setActive( active );
	}

	void CPropBrowserCtrl::onEnumPropertyChanged( QtProperty *prop, int value )
	{
		QString propName = prop->propertyName();

		if( propName == "button_type" )
		{
			CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( currentElement );
			if( e == NULL )
				return;

			std::string v;
			v = NelButtonType::toString( value );
			if( v.empty() )
				return;

			e->setProperty( propName.toUtf8().constData(), v );
		}
		else
		if( propName == "justification" )
		{
			CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( currentElement );
			if( e == NULL )
				return;

			std::string v;
			v = NelTxtJustification::toString( value );
			if( v.empty() )
				return;

			e->setProperty( propName.toUtf8().constData(), v );
		}
		else
		if( ( propName == "posref" ) ||
			( propName == "parentposref" ) ||
			( propName == "text_posref" ) ||
			( propName == "text_parent_posref" )
			)
		{
			CInterfaceElement *e = CWidgetManager::getInstance()->getElementFromId( currentElement );
			if( e == NULL )
				return;

			std::string v = NelPosRef::toString( value );
			if( v.empty() )
				return;

			e->setProperty( propName.toUtf8().constData(), v );
		}
	}

	void CPropBrowserCtrl::enablePropertyWatchers()
	{
		connect( propertyMgr, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ),
			this, SLOT( onPropertyChanged( QtProperty*, const QVariant& ) ) );
		connect( enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumPropertyChanged( QtProperty*, int ) ) );
	}

	void CPropBrowserCtrl::disablePropertyWatchers()
	{
		disconnect( propertyMgr, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ),
			this, SLOT( onPropertyChanged( QtProperty*, const QVariant& ) ) );
		disconnect( enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumPropertyChanged( QtProperty*, int ) ) );
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

		QtEnumEditorFactory *efactory = new QtEnumEditorFactory;
		browser->setFactoryForManager( enumMgr, efactory );
	}

	void CPropBrowserCtrl::setupProperty( const SPropEntry &prop, const CInterfaceElement *element )
	{
		QtVariantProperty *p = NULL;
		QVariant v;
		
		if( prop.propType == "button_type" )
		{
			std::string btype = element->getProperty( prop.propName );
			if( btype.empty() )
				return;

			int e = -1;
			e = NelButtonType::fromString( btype );
			if( e == -1 )
				return;

			QtProperty *pp = enumMgr->addProperty( prop.propName.c_str() );
			if( pp == NULL )
				return;

			QStringList enums;
			enums.push_back( "push_button" );
			enums.push_back( "toggle_button" );
			enums.push_back( "radio_button" );

			enumMgr->setEnumNames( pp, enums );
			enumMgr->setValue( pp, e );
			browser->addProperty( pp );
			return;
		}
		else
		if( prop.propType == "text_justification" )
		{
			std::string j = element->getProperty( prop.propName );
			if( j.empty() )
				return;

			int e = -1;
			e = NelTxtJustification::fromString( j );
			if( e == -1 )
				return;

			QtProperty *pp = enumMgr->addProperty( prop.propName.c_str() );
			if( pp == NULL )
				return;

			QStringList enums;
			enums.push_back( "clip_word" );
			enums.push_back( "dont_clip_word" );
			enums.push_back( "justified" );

			enumMgr->setEnumNames( pp, enums );
			enumMgr->setValue( pp, e );
			browser->addProperty( pp );

			return;
		}
		else
		if( prop.propType == "posref" )
		{
			std::string j = element->getProperty( prop.propName );
			if( j.empty() )
				return;

			int e = -1;
			e = NelPosRef::fromString( j );
			if( e == -1 )
				return;

			QtProperty *pp = enumMgr->addProperty( prop.propName.c_str() );
			if( pp == NULL )
				return;

			QStringList enums;
			enums.push_back( "BL" );
			enums.push_back( "BM" );
			enums.push_back( "BR" );
			enums.push_back( "ML" );
			enums.push_back( "MM" );
			enums.push_back( "MR" );
			enums.push_back( "TL" );
			enums.push_back( "TM" );
			enums.push_back( "TR" );

			enumMgr->setEnumNames( pp, enums );
			enumMgr->setValue( pp, e );
			browser->addProperty( pp );

			return;
		}
		else
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
		else
		if( prop.propType == "color" )
		{
			p = propertyMgr->addProperty( QVariant::Color, prop.propName.c_str() );

			std::string s = element->getProperty( prop.propName );
			// Parse string into a QColor
			QString qs = s.c_str();
			QStringList l = qs.split( " " );
			int r = l[ 0 ].toInt();
			int g = l[ 1 ].toInt();
			int b = l[ 2 ].toInt();
			int a = l[ 3 ].toInt();

			QColor value;
			value.setRed( r );
			value.setGreen( g );
			value.setBlue( b );
			value.setAlpha( a );
			v = value;
			

			// Remove the subproperties
			QList< QtProperty* > pl = p->subProperties();
			QListIterator< QtProperty* > itr( pl );
			while( itr.hasNext() )
			{
				delete itr.next();
			}
			pl.clear();

		}

		if( p == NULL )
			return;

		p->setValue( v );
		browser->addProperty( p );
	}
}
