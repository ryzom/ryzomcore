#include "browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include <QModelIndex>

#include "nel/georges/form.h"

#include "formitem.h"

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
	}

	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;
};

BrowserCtrl::BrowserCtrl( QtTreePropertyBrowser *browser ) :
QObject( browser )
{
	m_pvt = new BrowserCtrlPvt();
	m_pvt->m_browser = browser;
}

BrowserCtrl::~BrowserCtrl()
{
	delete m_pvt;
	m_pvt = NULL;
}

void BrowserCtrl::clicked( const QModelIndex &idx )
{
	m_pvt->m_browser->clear();

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
	for( int i = 0; i < st->Elements.size(); i++ )
	{
		NLGEORGES::CFormElmStruct::CFormElmStructElm &elm = st->Elements[ i ];

		QString key = elm.Name.c_str();
		QString value = "";

		if( elm.Element != NULL )
		{
			const NLGEORGES::UType *typ = elm.Element->getType();
			NLGEORGES::UType::TType ttyp = NLGEORGES::UType::String;
			if( typ != NULL )
				ttyp = typ->getType();

			NLGEORGES::CFormElmAtom *atom = static_cast< NLGEORGES::CFormElmAtom* >( elm.Element );
			std::string v;
			atom->getValue( v, NLGEORGES::UFormElm::NoEval );
			value = v.c_str();
		}

		QtVariantProperty *p = m_pvt->mgr->addProperty( QVariant::String, key );
		p->setValue( value );
		m_pvt->m_browser->addProperty( p );
	}

}

