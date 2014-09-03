#include "typ_browser_ctrl.h"

#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "3rdparty/qtpropertybrowser/qtpropertymanager.h"
#include "3rdparty/qtpropertybrowser/qteditorfactory.h"

#include "nel/georges/type.h"

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

	vp = m_variantMgr->addProperty( QVariant::String, "Max" );
	vp->setValue( m_typ->Max.c_str() );
	m_browser->addProperty( vp );

	vp = m_variantMgr->addProperty( QVariant::String, "increment" );
	vp->setValue( m_typ->Increment.c_str() );
	m_browser->addProperty( vp );

	enableMgrConnections();
}

void TypBrowserCtrl::onVariantValueChanged( QtProperty *p, const QVariant &v )
{
}

void TypBrowserCtrl::onEnumValueChanged( QtProperty *p, int v )
{
}

void TypBrowserCtrl::enableMgrConnections()
{
	connect( m_variantMgr, SIGNAL( valueChanged( QtProperty*, const QVariant& ) ), this, SLOT( onVariantValueChanged( QtProperty*, const QVariant& ) ) );
	connect( m_enumMgr, SIGNAL( valueChanged( QtProperty*, int ) ), this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );
}


