#include "browser_ctrl_pvt.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include <QVariant>

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

}


BrowserCtrlPvt::BrowserCtrlPvt( QObject *parent ) :
QObject( parent )
{
	mgr = new QtVariantPropertyManager();
	factory = new QtVariantEditorFactory();
	m_currentNode = NULL;
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

void BrowserCtrlPvt::setupAtom( NLGEORGES::CFormElmStruct::CFormElmStructElm &elm )
{
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

	QtVariantProperty *p = mgr->addProperty( t, key );
	p->setValue( value );
	m_browser->addProperty( p );
}

void BrowserCtrlPvt::setupArray( NLGEORGES::UFormElm *node )
{
	NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( node );
	uint size = 0;
	arr->getArraySize( size );

	QString key = QObject::tr( "Array size" );
	QtVariantProperty *p = mgr->addProperty( QVariant::Int, key );
	p->setValue( size );
	m_browser->addProperty( p );

}

void BrowserCtrlPvt::setupStruct( NLGEORGES::UFormElm *node )
{
	NLGEORGES::CFormElmStruct *st = static_cast< NLGEORGES::CFormElmStruct* >( node );

	for( int i = 0; i < st->Elements.size(); i++ )
	{
		NLGEORGES::CFormElmStruct::CFormElmStructElm &elm = st->Elements[ i ];
		if( ( elm.Element != NULL ) && !elm.Element->isAtom() )
			continue;

		if( elm.Element == NULL )
		{
			NLGEORGES::CFormDfn::CEntry &entry = st->FormDfn->getEntry( i );
			if( entry.getArrayFlag() )
				continue;				
		}

		setupAtom( elm );
	}
}

void BrowserCtrlPvt::setupNode( NLGEORGES::UFormElm *node )
{
	if( node->isStruct() )
		setupStruct( node );
	else
	if( node->isArray() )
		setupArray( node );
	else
		return;

	m_currentNode = node;
	m_browser->setFactoryForManager( mgr, factory );
}

void BrowserCtrlPvt::clear()
{
	m_browser->clear();
	m_currentNode = NULL;
}


void BrowserCtrlPvt::onStructValueChanged( QtProperty *p, const QVariant &value )
{
	std::string k = p->propertyName().toUtf8().constData();
	std::string v  = value.toString().toUtf8().constData();

	bool created = false;
	m_currentNode->setValueByName( v.c_str(), k.c_str(), &created );

	Q_EMIT modified();
}

void BrowserCtrlPvt::onArrayValueChanged( QtProperty *p, const QVariant &value )
{
	NLGEORGES::CFormElmArray *arr = static_cast< NLGEORGES::CFormElmArray* >( m_currentNode );
	std::string formName;
	arr->getFormName( formName, NULL );
	
	int newSize = value.toInt();
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

void BrowserCtrlPvt::onValueChanged( QtProperty *p, const QVariant &value )
{
	if( m_currentNode == NULL )
		return;

	if( m_currentNode->isStruct() )
		onStructValueChanged( p, value );
	else
	if( m_currentNode->isArray() )
		onArrayValueChanged( p, value );
}


