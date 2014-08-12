#include "browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include <QModelIndex>

#include "nel/georges/form.h"

#include "formitem.h"

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

class BrowserCtrlPvt
{
public:
	BrowserCtrlPvt()
	{
		mgr = new QtVariantPropertyManager();
		factory = new QtVariantEditorFactory();
	}

	~BrowserCtrlPvt()
	{
		delete mgr;
		mgr = NULL;
		delete factory;
		factory = NULL;
		m_browser = NULL;
	}

	void setupElement( NLGEORGES::CFormElmStruct::CFormElmStructElm &elm )
	{
		QString key = elm.Name.c_str();
		QString value = "";
		QVariant::Type t = QVariant::String;

		if( elm.Element != NULL )
		{
			t = getValueType( elm.Element->getType() );			

			NLGEORGES::CFormElmAtom *atom = static_cast< NLGEORGES::CFormElmAtom* >( elm.Element );
			std::string v;
			atom->getValue( v, NLGEORGES::UFormElm::NoEval );
			value = v.c_str();
		}

		QtVariantProperty *p = mgr->addProperty( t, key );
		p->setValue( value );
		m_browser->addProperty( p );
	}

	void setupForm( NLGEORGES::CFormElmStruct *st )
	{
		for( int i = 0; i < st->Elements.size(); i++ )
		{
			NLGEORGES::CFormElmStruct::CFormElmStructElm &elm = st->Elements[ i ];
			setupElement( elm );
		}

		m_browser->setFactoryForManager( mgr, factory );
	}

	void clear()
	{
		m_browser->clear();
	}

	void setBrowser( QtTreePropertyBrowser *browser )
	{
		m_browser = browser;
	}

private:
	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;
};

BrowserCtrl::BrowserCtrl( QtTreePropertyBrowser *browser ) :
QObject( browser )
{
	m_pvt = new BrowserCtrlPvt();
	m_pvt->setBrowser( browser );
}

BrowserCtrl::~BrowserCtrl()
{
	delete m_pvt;
	m_pvt = NULL;
}

void BrowserCtrl::clicked( const QModelIndex &idx )
{
	m_pvt->clear();

	GeorgesQt::CFormItem *item = static_cast< GeorgesQt::CFormItem* >( idx.internalPointer() );
	NLGEORGES::UFormElm &root = m_form->getRootNode();
	NLGEORGES::UFormElm *node = NULL;
	bool b = false;
	
	b = m_form->getRootNode().getNodeByName( &node, item->formName().c_str() );
	
	if( !b || ( node == NULL ) )
		return;
	
	if( !node->isStruct() )
		return;
	
	NLGEORGES::CFormElmStruct *st = static_cast< NLGEORGES::CFormElmStruct* >( node );
	m_pvt->setupForm( st );

}

