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

#include "browser_ctrl_pvt.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include <QVariant>
#include "formitem.h"

#include "nel/georges/form.h"

namespace
{
	QVariant::Type getValueType( const NLGEORGES::UType *typ )
	{
		QVariant::Type t = QVariant::String;

		NLGEORGES::UType::TType ttyp = NLGEORGES::UType::String;
		if( typ != NULL )
			ttyp = typ->getType();

		switch( ttyp )
		{
		case NLGEORGES::UType::UnsignedInt: t = QVariant::UInt; break;
		case NLGEORGES::UType::SignedInt: t = QVariant::Int; break;
		case NLGEORGES::UType::Double: t = QVariant::Double; break;
		case NLGEORGES::UType::Color: t = QVariant::Color; break;
		case NLGEORGES::UType::String: t = QVariant::String; break;
		}

		return t;
	}

	QVariant::Type getValueTypeFromDfn( NLGEORGES::CFormElmStruct *st, int idx )
	{
		NLGEORGES::CFormDfn *cdfn = st->FormDfn;
		NLGEORGES::CFormDfn::CEntry entry = cdfn->getEntry( idx );
		return getValueType( entry.getTypePtr() );
	}


	QVariant::Type getValueTypeFromDfn( NLGEORGES::CFormElmAtom *atom )
	{
		QVariant::Type t = QVariant::String;

		NLGEORGES::CFormElm *cparent = static_cast< NLGEORGES::CFormElm* >( atom->getParent() );
		
		if( cparent->isArray() )
		{
			NLGEORGES::CFormElmStruct *aparent = static_cast< NLGEORGES::CFormElmStruct* >( cparent->getParent() );
			NLGEORGES::CFormDfn *cdfn = static_cast< NLGEORGES::CFormDfn* >( aparent->getStructDfn() );

			int idx = -1;
			for( idx = 0; idx < aparent->Elements.size(); idx++ )
			{
				if( aparent->Elements[ idx ].Element == cparent )
					break;
			}

			NLGEORGES::CFormDfn::CEntry entry = cdfn->getEntry( idx );
			return getValueType( entry.getTypePtr() );
		}
		else
		if( cparent->isStruct() )
		{
			NLGEORGES::CFormElmStruct *sparent = static_cast< NLGEORGES::CFormElmStruct* >( cparent );
			NLGEORGES::CFormDfn *cdfn = static_cast< NLGEORGES::CFormDfn* >( cparent->getStructDfn() );

			int idx = -1;
			for( idx = 0; idx < sparent->Elements.size(); idx++ )
			{
				if( sparent->Elements[ idx ].Element == atom )
					break;
			}

			NLGEORGES::CFormDfn::CEntry entry = cdfn->getEntry( idx );
			return getValueType( entry.getTypePtr() );
		}

		return t;
	}

	NLGEORGES::UFormElm* getGeorgesNode( GeorgesQt::CFormItem *item )
	{
		NLGEORGES::UFormElm *n = NULL;
		item->form()->getRootNode().getNodeByName( &n, item->formName().c_str() );
		return n;
	}
}


BrowserCtrlPvt::BrowserCtrlPvt( QObject *parent ) :
QObject( parent )
{
	mgr = new QtVariantPropertyManager();
	factory = new QtVariantEditorFactory();
	m_rootNode = NULL;
}

BrowserCtrlPvt::~BrowserCtrlPvt()
{
	delete mgr;
	mgr = NULL;
	delete factory;
	factory = NULL;
	m_browser = NULL;
}

void BrowserCtrlPvt::setupAtom( NLGEORGES::CFormElmStruct *st, int idx )
{
	NLGEORGES::CFormElmStruct::CFormElmStructElm &elm = st->Elements[ idx ];
	if( ( elm.Element != NULL ) && !elm.Element->isAtom() )
			return;
	
	if( elm.Element == NULL )
	{
		NLGEORGES::CFormDfn::CEntry &entry = st->FormDfn->getEntry( idx );
		if( entry.getArrayFlag() )
			return;
	}	

	QString key = elm.Name.c_str();
	QString value = "";
	QVariant::Type t = QVariant::String;

	if( elm.Element != NULL )
	{
		t = getValueType( elm.Element->getType() );			

		std::string formName;
		elm.Element->getFormName( formName, NULL );

		std::string v;
		m_rootNode->getValueByName( v, formName.c_str(), NLGEORGES::UFormElm::NoEval, NULL, 0 );
		value = v.c_str();
	}
	else
	{
		t = getValueTypeFromDfn( st, idx );
	}

	QtVariantProperty *p = mgr->addProperty( t, key );
	p->setValue( value );
	m_browser->addProperty( p );
}

void BrowserCtrlPvt::setupStruct( NLGEORGES::UFormElm *node )
{
	NLGEORGES::CFormElmStruct *st = static_cast< NLGEORGES::CFormElmStruct* >( node );

	for( int i = 0; i < st->Elements.size(); i++ )
	{
		setupAtom( st, i );
	}
}

void BrowserCtrlPvt::setupStruct( GeorgesQt::CFormItem *node )
{
	NLGEORGES::UFormElm *n = getGeorgesNode( node );
	if( n == NULL )
		return;

	m_currentNode.p = n;

	setupStruct( n );
}

void BrowserCtrlPvt::setupArray( GeorgesQt::CFormItem *node )
{
	NLGEORGES::UFormElm *n = getGeorgesNode( node );
	uint size = 0;

	if( n != NULL )
	{
		NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( n );
		arr->getArraySize( size );
		m_currentNode.p = n;
	}

	QString key = QObject::tr( "Array size" );
	QtVariantProperty *p = mgr->addProperty( QVariant::Int, key );
	p->setValue( size );
	m_browser->addProperty( p );
}

void BrowserCtrlPvt::setupAtom( GeorgesQt::CFormItem *node )
{
	NLGEORGES::UFormElm *n = getGeorgesNode( node );

	if( n == NULL )
		return;

	NLGEORGES::CFormElmAtom *atom = static_cast< NLGEORGES::CFormElmAtom* >( n );
	std::string v = atom->getValue();
	
	const NLGEORGES::CType *t = atom->getType();
	QVariant::Type tt = QVariant::String;
	if( t != NULL )
	{
		tt = getValueType( t );
	}
	else
	{
		tt = getValueTypeFromDfn( atom );
	}

	QtVariantProperty *p = mgr->addProperty( tt, "value" );
	p->setValue( v.c_str() );
	m_browser->addProperty( p );

	m_currentNode.p = n;
}

void BrowserCtrlPvt::setupNode( GeorgesQt::CFormItem *node )
{
	m_currentNode.clear();
	m_currentNode.name = node->formName().c_str();
	
	m_rootNode = dynamic_cast< NLGEORGES::CFormElm* >( &(node->form()->getRootNode()) );

	if( node->isArray() )
		setupArray( node );
	else
	if( node->isStruct() )
		setupStruct( node );
	else
	if( node->isAtom() )
		setupAtom( node );

	m_browser->setFactoryForManager( mgr, factory );
}

void BrowserCtrlPvt::clear()
{
	m_browser->clear();
	m_currentNode.clear();
}


void BrowserCtrlPvt::onStructValueChanged( QtProperty *p, const QVariant &value )
{
	std::string k = p->propertyName().toUtf8().constData();
	std::string v  = value.toString().toUtf8().constData();

	bool created = false;
	m_currentNode.p->setValueByName( v.c_str(), k.c_str(), &created );

	QString key = m_currentNode.name + "." + p->propertyName();

	Q_EMIT modified();
	Q_EMIT valueChanged( key, value.toString() );
}

void BrowserCtrlPvt::createArray()
{
	const NLGEORGES::CFormDfn *parentDfn;
	const NLGEORGES::CFormDfn *nodeDfn;
	uint indexDfn;
	const NLGEORGES::CType *type;
	NLGEORGES::UFormDfn::TEntryType entryType;
	NLGEORGES::CFormElm *node;
	bool created;
	bool isArray;

	m_rootNode->createNodeByName( m_currentNode.name.toUtf8().constData(), &parentDfn, indexDfn, &nodeDfn, &type, &node, entryType, isArray, created );

	if( !created )
		return;

	m_currentNode.p = node;

	NLGEORGES::CFormElmArray *arr = dynamic_cast< NLGEORGES::CFormElmArray* >( node );
	QString idx = "[0]";
	arr->createNodeByName( idx.toUtf8().constData(), &parentDfn, indexDfn, &nodeDfn, &type, &node, entryType, isArray, created );
	
	std::string formName;
	arr->getFormName( formName, NULL );

	Q_EMIT arrayResized( formName.c_str(), 1 );
	Q_EMIT modified();

}

void BrowserCtrlPvt::onArrayValueChanged( QtProperty *p, const QVariant &value )
{
	// Newsize checks hacked in, because setting unsigned value type in QVariant crashes the property browser!
	int newSize = value.toInt();
	if( newSize < 0 )
		return;

	if( m_currentNode.p == NULL )
	{
		if( newSize != 1 )
			return;
		createArray();
		return;
	}

	NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( m_currentNode.p );
	std::string formName;
	arr->getFormName( formName, NULL );
	
	int oldSize = arr->Elements.size();

	if( newSize == oldSize )
		return;

	if( newSize < oldSize )
	{
		for( int i = newSize; i < oldSize; i++ )
		{
			delete arr->Elements[ i ].Element;
		}

		arr->Elements.resize( newSize );
	}
	else
	{
		arr->Elements.resize( newSize );


		const NLGEORGES::CFormDfn *parentDfn;
		const NLGEORGES::CFormDfn *nodeDfn;
		uint indexDfn;
		const NLGEORGES::CType *type;
		NLGEORGES::UFormDfn::TEntryType entryType;
		NLGEORGES::CFormElm *node;
		bool created;
		bool isArray;

		QString idx;

		for( int i = oldSize; i < newSize; i++ )
		{
			idx.clear();
			idx += "[";
			idx += QString::number( i );
			idx += "]";

			bool b;
			b = arr->createNodeByName( idx.toUtf8().constData(), &parentDfn, indexDfn, &nodeDfn, &type, &node, entryType, isArray, created );
		}
	}

	QString name = formName.c_str();
	Q_EMIT arrayResized( name, newSize );
	Q_EMIT modified();

	if( newSize == 0 )
		m_currentNode.p = NULL;
}

void BrowserCtrlPvt::onAtomValueChanged( QtProperty *p, const QVariant &value )
{
	NLGEORGES::CFormElmAtom *atom = static_cast< NLGEORGES::CFormElmAtom* >( m_currentNode.p );
	atom->setValue( value.toString().toUtf8() );

	Q_EMIT modified();
	Q_EMIT valueChanged( m_currentNode.name, value.toString() );
}

void BrowserCtrlPvt::onValueChanged( QtProperty *p, const QVariant &value )
{
	if( m_currentNode.p == NULL )
	{
		if( m_currentNode.name.isEmpty() )
			return;

		onArrayValueChanged( p, value );
		return;
	}

	if( m_currentNode.p->isStruct() )
		onStructValueChanged( p, value );
	else
	if( m_currentNode.p->isArray() )
		onArrayValueChanged( p, value );
	else
	if( m_currentNode.p->isAtom() )
		onAtomValueChanged( p, value );
}


