#include "browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include <QModelIndex>

#include "nel/georges/form.h"

#include "formitem.h"

#include "browser_ctrl_pvt.h"

BrowserCtrl::BrowserCtrl( QtTreePropertyBrowser *browser ) :
QObject( browser )
{
	m_pvt = new BrowserCtrlPvt();
	m_pvt->setBrowser( browser );

	connect( m_pvt, SIGNAL( arrayResized( const QString&, int ) ), this, SLOT( onArrayResized( const QString&, int ) ) );
	connect( m_pvt, SIGNAL( modified() ), this, SLOT( onModified() ) );
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

	m_pvt->setupNode( item );

	enableMgrConnections();

}

void BrowserCtrl::onValueChanged( QtProperty *p, const QVariant &value )
{
	m_pvt->onValueChanged( p, value );
}

void BrowserCtrl::onArrayResized( const QString &name, int size )
{
	Q_EMIT arrayResized( name, size );
}

void BrowserCtrl::onModified()
{
	Q_EMIT modified();
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

