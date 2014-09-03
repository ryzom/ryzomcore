// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

#include "typ_browser_ctrl.h"

#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "3rdparty/qtpropertybrowser/qtpropertymanager.h"
#include "3rdparty/qtpropertybrowser/qteditorfactory.h"

#include "nel/georges/type.h"

namespace
{
	QString typeToString( int v )
	{
		QString s;

		switch( v )
		{
		case NLGEORGES::UType::UnsignedInt: s = "UnsignedInt"; break;
		case NLGEORGES::UType::SignedInt: s = "SignedInt"; break;
		case NLGEORGES::UType::Double: s = "Double"; break;
		case NLGEORGES::UType::String: s = "String"; break;
		case NLGEORGES::UType::Color: s = "Color"; break;
		}

		return s;
	}

	QString uitypeToString( int v )
	{
		QString s;

		switch( v )
		{
		case NLGEORGES::CType::Edit: s = "Edit"; break;
		case NLGEORGES::CType::EditSpin: s = "EditSpin"; break;
		case NLGEORGES::CType::NonEditableCombo: s = "NonEditableCombo"; break;
		case NLGEORGES::CType::FileBrowser: s = "FileBrowser"; break;
		case NLGEORGES::CType::BigEdit: s = "BigEdit"; break;
		case NLGEORGES::CType::ColorEdit: s = "ColorEdit"; break;
		case NLGEORGES::CType::IconWidget: s = "IconWidget"; break;
		}

		return s;
	}
}

TypBrowserCtrl::TypBrowserCtrl( QObject *parent ) :
QObject( parent )
{
	m_typ = NULL;

	m_variantMgr = new QtVariantPropertyManager( this );
	m_variantFactory = new QtVariantEditorFactory( this );
	m_enumMgr = new QtEnumPropertyManager( this );
	m_enumFactory = new QtEnumEditorFactory( this );
}

TypBrowserCtrl::~TypBrowserCtrl()
{
	m_typ = NULL;
	m_variantMgr = NULL;
	m_variantFactory = NULL;
	m_enumMgr = NULL;
	m_enumFactory = NULL;
}

void TypBrowserCtrl::load()
{
	m_browser->clear();
	m_browser->setFactoryForManager( m_variantMgr, m_variantFactory );
	m_browser->setFactoryForManager( m_enumMgr, m_enumFactory );

	m_typ->Type;
	m_typ->UIType;

	QtProperty *p = NULL;

	p = m_enumMgr->addProperty( "type" );
	QStringList l;
	l.push_back( "UnsignedInt" );
	l.push_back( "SignedInt" );
	l.push_back( "Double" );
	l.push_back( "String" );
	l.push_back( "Color" );
	m_enumMgr->setEnumNames( p, l );
	m_enumMgr->setValue( p, m_typ->Type );
	m_browser->addProperty( p );	

	p = m_enumMgr->addProperty( "uitype" );
	l.clear();
	l.push_back( "Edit" );
	l.push_back( "EditSpin" );
	l.push_back( "NonEditableCombo" );
	l.push_back( "FileBrowser" );
	l.push_back( "BigEdit" );
	l.push_back( "ColorEdit" );
	l.push_back( "IconWidget" );
	m_enumMgr->setEnumNames( p, l );
	m_enumMgr->setValue( p, m_typ->UIType );
	m_browser->addProperty( p );
	
	
	QtVariantProperty *vp = NULL;

	vp = m_variantMgr->addProperty( QVariant::String, "default" );
	vp->setValue( m_typ->Default.c_str() );
	m_browser->addProperty( vp );

	vp = m_variantMgr->addProperty( QVariant::String, "min" );
	vp->setValue( m_typ->Min.c_str() );
	m_browser->addProperty( vp );

	vp = m_variantMgr->addProperty( QVariant::String, "max" );
	vp->setValue( m_typ->Max.c_str() );
	m_browser->addProperty( vp );

	vp = m_variantMgr->addProperty( QVariant::String, "increment" );
	vp->setValue( m_typ->Increment.c_str() );
	m_browser->addProperty( vp );

	enableMgrConnections();
}

void TypBrowserCtrl::onVariantValueChanged( QtProperty *p, const QVariant &v )
{
	QString n = p->propertyName();
	if( n == "default" )
	{
		m_typ->Default = v.toString().toUtf8().constData();
	}
	else
	if( n == "min" )
	{
		m_typ->Min = v.toString().toUtf8().constData();
	}
	else
	if( n == "max" )
	{
		m_typ->Max = v.toString().toUtf8().constData();
	}
	else
	if( n == "increment" )
	{
		m_typ->Increment = v.toString().toUtf8().constData();
	}
	else
		return;

	Q_EMIT modified( n, v.toString().toUtf8().constData() );
}

void TypBrowserCtrl::onEnumValueChanged( QtProperty *p, int v )
{
	QString n = p->propertyName();
	QString value;

	if( n == "type" )
	{
		m_typ->Type = NLGEORGES::UType::TType( v );
		value = typeToString( v );
	}
	else
	if( n == "uitype" )
	{
		m_typ->UIType = NLGEORGES::CType::TUI( v );
		value = uitypeToString( v );
	}
	else
		return;

	Q_EMIT modified( n, value );
}

void TypBrowserCtrl::enableMgrConnections()
{
	connect( m_variantMgr, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ), this, SLOT( onVariantValueChanged( QtProperty*, const QVariant& ) ) );
	connect( m_enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ), this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );
}


