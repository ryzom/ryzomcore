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

#ifndef BROWSER_CTRL_H
#define BROWSER_CTRL_H

#include <QObject>

namespace NLGEORGES
{
	class UForm;
}

class QtTreePropertyBrowser;
class QModelIndex;
class QVariant;
class QtProperty;

class BrowserCtrlPvt;

class BrowserCtrl : public QObject
{
	Q_OBJECT
public:
	BrowserCtrl( QtTreePropertyBrowser *browser );
	~BrowserCtrl();
	void setForm( NLGEORGES::UForm *form ){ m_form = form; }

public Q_SLOTS:
	void clicked( const QModelIndex &idx );

Q_SIGNALS:
	void arrayResized( const QString &name, int size );
	void modified();
	void valueChanged( const QString &key, const QString &value );
	void vstructChanged( const QString &name );

private Q_SLOTS:
	void onValueChanged( QtProperty *p, const QVariant &value );
	void onValueChanged( const QString &key, const QString &value );
	void onArrayResized( const QString &name, int size );
	void onModified();
	void onVStructChanged( const QString &name );

private:
	void enableMgrConnections();
	void disableMgrConnections();


	BrowserCtrlPvt *m_pvt;
	NLGEORGES::UForm *m_form;
};

#endif
