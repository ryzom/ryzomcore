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

#ifndef TYP_BROWSER_CTRL
#define TYP_BROWSER_CTRL

#include <QObject>

class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtTreePropertyBrowser;
class QtEnumPropertyManager;
class QtEnumEditorFactory;
class QVariant;
class QtProperty;

namespace NLGEORGES
{
	class CType;
}

class TypBrowserCtrl : public QObject
{
	Q_OBJECT
public:
	TypBrowserCtrl( QObject *parent = NULL );
	~TypBrowserCtrl();

	void load();

	void setTyp( NLGEORGES::CType *typ ){ m_typ = typ; }
	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }

Q_SIGNALS:
	void modified( const QString &k, const QString &v );

private Q_SLOTS:
	void onVariantValueChanged( QtProperty *p, const QVariant &v );
	void onEnumValueChanged( QtProperty *p, int v );

private:
	void enableMgrConnections();

	NLGEORGES::CType *m_typ;
	QtTreePropertyBrowser *m_browser;

	QtVariantPropertyManager *m_variantMgr;
	QtVariantEditorFactory *m_variantFactory;
	QtEnumPropertyManager *m_enumMgr;
	QtEnumEditorFactory *m_enumFactory;

};

#endif
