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

	// Get the data from a string, and pack it into a QVariant properly
	// Needed for some special values, like color
	// which are represented differently in Nel and Qt
	QVariant stringToVariant( const QString &value, QVariant::Type type )
	{
		QVariant v;

		if( type == QVariant::Color )
		{
			QStringList l = value.split( ',' );
			if( l.size() != 3 )
				v = "";
			else
			{
				QColor c;
				c.setRed( l[ 0 ].toInt() );
				c.setGreen( l[ 1 ].toInt() );
				c.setBlue( l[ 2 ].toInt() );
				v = c;
			}

		}
		else
			v = value;

		return v;
	}

	// The inverse function of stringToVariant
	// Unpacks the data from a QVariant properly
	QString variantToString( const QVariant &value )
	{
		QString v;

		if( value.type() == QVariant::Color )
		{
			QColor c = value.value< QColor >();
			v += QString::number( c.red() );
			v += ',';
			v += QString::number( c.green() );
			v += ',';
			v += QString::number( c.blue() );
		}
		else
			v = value.toString();

		return v;
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

NLGEORGES::UFormElm* BrowserCtrlPvt::getNode( const QString &name )
{
	NLGEORGES::UFormElm *node = NULL;
	m_rootNode->getNodeByName( &node, name.toUtf8().constData(), NULL, true );

	return node;
}

NLGEORGES::UFormElm* BrowserCtrlPvt::getCurrentNode()
{
	return getNode( m_currentNode.name );
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

		if( entry.getType() == NLGEORGES::UFormDfn::EntryVirtualDfn )
			return;
	}	

	QString key = elm.Name.c_str();
	QString value = "";
	QVariant::Type t = QVariant::String;

	// If the atom exists, get the value from it
	// Otherwise just get the type so we can set up the proper editor
	if( elm.Element != NULL )
	{
		// Check if there's a type, if not get it from the Dfn
		const NLGEORGES::CType *type = elm.Element->getType();
		if( type != NULL )
			t = getValueType( elm.Element->getType() );			
		else
			t = getValueTypeFromDfn( st, idx );

		std::string formName;
		elm.Element->getFormName( formName, NULL );

		std::string v;
		m_rootNode->getValueByName( v, formName.c_str(), NLGEORGES::UFormElm::NoEval, NULL, 0 );
		value = stringToVariant( v.c_str(), t ).toString();
	}
	else
	{
		t = getValueTypeFromDfn( st, idx );
	}

	QtVariantProperty *p = addVariantProperty( t, key, value );
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

	setupStruct( n );
}

void BrowserCtrlPvt::setupVStruct( GeorgesQt::CFormItem *node )
{
	NLGEORGES::UFormElm *n = getGeorgesNode( node );

	QtVariantProperty *p;
	p = addVariantProperty( QVariant::String, "Dfn filename", QVariant() );

	if( n != NULL )
	{
		NLGEORGES::CFormElmVirtualStruct *vs = static_cast< NLGEORGES::CFormElmVirtualStruct* >( n );
		mgr->setValue( p, vs->DfnFilename.c_str() );
		setupStruct( n );
	}
}

void BrowserCtrlPvt::setupArray( GeorgesQt::CFormItem *node )
{
	NLGEORGES::UFormElm *n = getGeorgesNode( node );
	uint size = 0;

	if( n != NULL )
	{
		NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( n );
		arr->getArraySize( size );
	}

	QString key = QObject::tr( "Array size" );
	QtVariantProperty *p = addVariantProperty( QVariant::Int, key, size );
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

	QtVariantProperty *p = addVariantProperty( tt, "value", v.c_str() );
}

void BrowserCtrlPvt::setupNode( GeorgesQt::CFormItem *node )
{
	m_currentNode.clear();
	m_currentNode.name = node->formName().c_str();
	m_currentNode.type = node->type();

	m_rootNode = dynamic_cast< NLGEORGES::CFormElm* >( &(node->form()->getRootNode()) );

	if( node->isArray() )
		setupArray( node );
	else
	if( node->isStruct() )
		setupStruct( node );
	else
	if( node->isVStruct() )
		setupVStruct( node );
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
	std::string v;
	v = variantToString( value ).toUtf8().constData();

	NLGEORGES::UFormElm *node = getCurrentNode();

	bool created = false;
	node->setValueByName( v.c_str(), k.c_str(), &created );

	QString key = m_currentNode.name + "." + p->propertyName();

	Q_EMIT modified();
	Q_EMIT valueChanged( key, value.toString() );
}

void BrowserCtrlPvt::onVStructValueChanged( QtProperty *p, const QVariant &value )
{
	if( p->propertyName() != "Dfn filename" )
	{
		onStructValueChanged( p, value );
		return;
	}

	NLGEORGES::CFormElmVirtualStruct *vs = static_cast< NLGEORGES::CFormElmVirtualStruct* >( getCurrentNode() );
	if( vs == NULL )
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

		vs = static_cast< NLGEORGES::CFormElmVirtualStruct* >( node );
	}

	vs->DfnFilename = value.toString().toUtf8().constData();

	QString key = m_currentNode.name + "." + p->propertyName();
	
	Q_EMIT modified();
	Q_EMIT valueChanged( key, value.toString() );
	Q_EMIT vstructChanged( m_currentNode.name );
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

	
	NLGEORGES::UFormElm *node = getCurrentNode();

	if( node == NULL )
	{
		if( newSize != 1 )
			return;
		createArray();
		return;
	}

	NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( node );
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
}

void BrowserCtrlPvt::onAtomValueChanged( QtProperty *p, const QVariant &value )
{
	NLGEORGES::CFormElmAtom *atom = static_cast< NLGEORGES::CFormElmAtom* >( getCurrentNode() );
	atom->setValue( value.toString().toUtf8() );

	Q_EMIT modified();
	Q_EMIT valueChanged( m_currentNode.name, value.toString() );
}

void BrowserCtrlPvt::onValueChanged( QtProperty *p, const QVariant &value )
{
	if( m_currentNode.type == GeorgesQt::CFormItem::TYPE_VSTRUCT )
		onVStructValueChanged( p, value );
	else
	if( m_currentNode.type == GeorgesQt::CFormItem::TYPE_STRUCT )
		onStructValueChanged( p, value );
	else
	if( m_currentNode.type == GeorgesQt::CFormItem::TYPE_ARRAY )
		onArrayValueChanged( p, value );
	else
	if( m_currentNode.type == GeorgesQt::CFormItem::TYPE_ATOM )
		onAtomValueChanged( p, value );
}

QtVariantProperty* BrowserCtrlPvt::addVariantProperty( QVariant::Type type, const QString &key, const QVariant &value )
{
	QtVariantProperty *p = mgr->addProperty( type, key );
	
	// Remove the color sub-properties, so they don't get triggered on value change
	if( type == QVariant::Color )
	{
		QList< QtProperty* > sp = p->subProperties();
		QListIterator< QtProperty* > itr( sp );
		while( itr.hasNext() )
		{
			QtProperty *prop = itr.next();
			p->removeSubProperty( prop );
			delete prop;
		}
		sp.clear();
	}

	p->setValue( value );
	m_browser->addProperty( p );

	return p;
}

