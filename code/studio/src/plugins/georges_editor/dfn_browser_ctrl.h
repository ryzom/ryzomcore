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

#ifndef DFN_BROWSER_CTRL
#define DFN_BROWSER_CTRL

#include <QObject>

namespace NLGEORGES
{
	class CFormDfn;
}

class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtTreePropertyBrowser;
class QVariant;
class QtProperty;

class QtEnumPropertyManager;
class QtEnumEditorFactory;
class FileManager;
class FileEditFactory;

class DFNBrowserCtrl : public QObject
{
	Q_OBJECT
public:
	DFNBrowserCtrl( QObject *parent = NULL );
	~DFNBrowserCtrl();

	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }
	void setDFN( NLGEORGES::CFormDfn *dfn ){ m_dfn = dfn; }

	void onElementSelected( int idx );

Q_SIGNALS:
	void valueChanged( const QString &key, const QString &value );

private Q_SLOTS:
	void onFileValueChanged( QtProperty *p, const QString &v );
	void onVariantValueChanged( QtProperty *p, const QVariant &v );
	void onEnumValueChanged( QtProperty *p, int v );

private:
	void connectManagers();
	void disconnectManagers();

	QtTreePropertyBrowser *m_browser;
	NLGEORGES::CFormDfn *m_dfn;

	QtVariantPropertyManager *m_manager;
	QtVariantEditorFactory *m_factory;

	QtEnumPropertyManager *m_enumMgr;
	QtEnumEditorFactory *m_enumFactory;

	FileManager *m_fileMgr;
	FileEditFactory *m_fileFactory;

	int m_idx;
};

#endif

