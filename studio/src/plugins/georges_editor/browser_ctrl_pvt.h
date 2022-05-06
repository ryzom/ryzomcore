// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2010-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef BROWSER_CTRL_PVT_H
#define BROWSER_CTRL_PVT_H

#include <QObject>

namespace NLGEORGES
{
	class CFormElm;
	class UFormElm;
	class CFormElmStruct;
}

namespace GeorgesQt
{
	class CFormItem;
}

class QtVariantPropertyManager;
class QtVariantEditorFactory;
class QtTreePropertyBrowser;
class QVariant;
class QtProperty;
class QtVariantProperty;

class FileManager;
class FileEditFactory;

class BrowserCtrlPvt : public QObject
{
	Q_OBJECT
public:
	BrowserCtrlPvt( QObject *parent = NULL );
	~BrowserCtrlPvt();

	void clear();
	void setupNode( GeorgesQt::CFormItem *node );
	void onValueChanged( QtProperty *p, const QVariant &value );
	void onFileValueChanged( QtProperty *p, const QString &value );

	QtVariantPropertyManager* manager() const{ return mgr; }
	FileManager* fileManager() const{ return m_fileMgr; }
	void setRootNode( NLGEORGES::CFormElm *root ){ m_rootNode = root; }	
	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }

Q_SIGNALS:
	void arrayResized( const QString &name, int size );
	void modified();
	void valueChanged( const QString &key, const QString &value );
	void vstructChanged( const QString &name );

private:
	NLGEORGES::UFormElm* getNode( const QString &name );
	NLGEORGES::UFormElm* getCurrentNode();

	void setupStruct( NLGEORGES::UFormElm *node );
	void setupAtom( NLGEORGES::CFormElmStruct *st, int idx );

	void setupStruct( GeorgesQt::CFormItem *node );
	void setupVStruct( GeorgesQt::CFormItem *node );
	void setupArray( GeorgesQt::CFormItem *node );
	void setupAtom( GeorgesQt::CFormItem *node );

	void onStructValueChanged( QtProperty *p, const QVariant &value );
	void onVStructValueChanged( QtProperty *p, const QVariant &value );
	void onArrayValueChanged( QtProperty *p, const QVariant &value );
	void onAtomValueChanged( QtProperty *p, const QVariant &value );
	void createArray();

	QtVariantProperty* addVariantProperty( QVariant::Type type, const QString &key, const QVariant &value );
	QtProperty *addFileProperty( const QString &key, const QString &value );
	
	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;

	FileManager *m_fileMgr;
	FileEditFactory *m_fileFactory;

	QString m_currentNodeName;
	NLGEORGES::CFormElm *m_rootNode;

	struct CurrentNode
	{
		CurrentNode()
		{
			clear();
		}

		void clear()
		{
			type = -1;
			name = "";
		}

		QString name;
		int type;
	};

	CurrentNode m_currentNode;
};

#endif
