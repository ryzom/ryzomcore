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


#include "browser_ctrl.h"
#include "3rdparty/qtpropertybrowser/qttreepropertybrowser.h"
#include "3rdparty/qtpropertybrowser/qtvariantproperty.h"
#include "filepath_property_manager.h"
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
	connect( m_pvt, SIGNAL( valueChanged( const QString&, const QString& ) ), this, SLOT( onValueChanged( const QString&, const QString& ) ) );
	connect( m_pvt, SIGNAL( vstructChanged( const QString& ) ), this, SLOT( onVStructChanged( const QString& ) ) );
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

void BrowserCtrl::onValueChanged( const QString &key, const QString &value )
{
	Q_EMIT valueChanged( key, value );
}

void BrowserCtrl::onFileValueChanged( QtProperty *p, const QString &value )
{
	m_pvt->onFileValueChanged( p, value );
}

void BrowserCtrl::onArrayResized( const QString &name, int size )
{
	Q_EMIT arrayResized( name, size );
}

void BrowserCtrl::onModified()
{
	Q_EMIT modified();
}

void BrowserCtrl::onVStructChanged( const QString &name )
{
	Q_EMIT vstructChanged( name );
}

void BrowserCtrl::enableMgrConnections()
{
	QtVariantPropertyManager *mgr = m_pvt->manager();
	FileManager *fileMgr = m_pvt->fileManager();

	connect( mgr, SIGNAL( valueChanged( QtProperty*, const QVariant & ) ),
		this, SLOT( onValueChanged( QtProperty*, const QVariant & ) ) );
	connect( fileMgr, SIGNAL( valueChanged( QtProperty*, const QString & ) ),
		this, SLOT( onFileValueChanged( QtProperty*, const QString & ) ) );
}

void BrowserCtrl::disableMgrConnections()
{
	QtVariantPropertyManager *mgr = m_pvt->manager();
	FileManager *fileMgr = m_pvt->fileManager();

	disconnect( mgr, SIGNAL( valueChanged( QtProperty*, const QVariant & ) ),
		this, SLOT( onValueChanged( QtProperty*, const QVariant & ) ) );
	disconnect( fileMgr, SIGNAL( valueChanged( QtProperty*, const QString & ) ),
		this, SLOT( onFileValueChanged( QtProperty*, const QString & ) ) );
}

