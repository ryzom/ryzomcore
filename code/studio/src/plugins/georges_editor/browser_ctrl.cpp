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
		m_currentNode = NULL;
		m_rootNode = NULL;
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

	void setupForm( NLGEORGES::CFormElmStruct *st )
	{
		for( int i = 0; i < st->Elements.size(); i++ )
		{
			NLGEORGES::CFormElmStruct::CFormElmStructElm &elm = st->Elements[ i ];
			setupElement( elm );
		}

		m_currentNode = st;
		m_browser->setFactoryForManager( mgr, factory );
	}

	void clear()
	{
		m_browser->clear();
		m_currentNode = NULL;
	}

	void setBrowser( QtTreePropertyBrowser *browser )
	{
		m_browser = browser;
	}

	void onValueChanged( QtProperty *p, const QVariant &value )
	{
		if( m_currentNode == NULL )
			return;

		std::string k = p->propertyName().toUtf8().constData();
		std::string v  = value.toString().toUtf8().constData();

		bool created = false;
		m_currentNode->setValueByName( v.c_str(), k.c_str(), &created );
	}

	QtVariantPropertyManager* manager() const{ return mgr; }

	void setRootNode( NLGEORGES::CFormElm *root ){ m_rootNode = root; }

private:
	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;

	NLGEORGES::UFormElm *m_currentNode;
	NLGEORGES::CFormElm *m_rootNode;
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
	disableMgrConnections();
	m_pvt->clear();

	GeorgesQt::CFormItem *item = static_cast< GeorgesQt::CFormItem* >( idx.internalPointer() );
	NLGEORGES::UFormElm &root = m_form->getRootNode();
	NLGEORGES::CFormElm *rootNode = dynamic_cast< NLGEORGES::CFormElm* >( &root );
	m_pvt->setRootNode( rootNode );
	NLGEORGES::UFormElm *node = NULL;
	bool b = false;
	
	b = m_form->getRootNode().getNodeByName( &node, item->formName().c_str() );
	
	if( !b || ( node == NULL ) )
		return;
	
	if( !node->isStruct() )
		return;
	
	NLGEORGES::CFormElmStruct *st = static_cast< NLGEORGES::CFormElmStruct* >( node );
	m_pvt->setupForm( st );

	enableMgrConnections();

}

void BrowserCtrl::onValueChanged( QtProperty *p, const QVariant &value )
{
	m_pvt->onValueChanged( p, value );
}

void BrowserCtrl::enableMgrConnections()
{
	QtVariantPropertyManager *mgr = m_pvt->manager();

	connect( mgr, SIGNAL( valueChanged( QtProperty*, const QVariant & ) ),
		this, SLOT( onValueChanged( QtProperty*, const QVariant & ) ) );
}

void BrowserCtrl::disableMgrConnections()
{
	QtVariantPropertyManager *mgr = m_pvt->manager();

	disconnect( mgr, SIGNAL( valueChanged( QtProperty*, const QVariant & ) ),
		this, SLOT( onValueChanged( QtProperty*, const QVariant & ) ) );
}

