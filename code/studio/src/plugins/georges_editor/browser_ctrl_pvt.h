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

class BrowserCtrlPvt : public QObject
{
	Q_OBJECT
public:
	BrowserCtrlPvt( QObject *parent = NULL );
	~BrowserCtrlPvt();

	void clear();
	void setupNode( GeorgesQt::CFormItem *node );
	void onValueChanged( QtProperty *p, const QVariant &value );

	QtVariantPropertyManager* manager() const{ return mgr; }
	void setRootNode( NLGEORGES::CFormElm *root ){ m_rootNode = root; }	
	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }

Q_SIGNALS:
	void arrayResized( const QString &name, int size );
	void modified();
	void valueChanged( const QString &key, const QString &value );

private:
	void setupStruct( NLGEORGES::UFormElm *node );
	void setupAtom( NLGEORGES::CFormElmStruct::CFormElmStructElm &elm );

	void setupStruct( GeorgesQt::CFormItem *node );
	void setupArray( GeorgesQt::CFormItem *node );

	void onStructValueChanged( QtProperty *p, const QVariant &value );
	void onArrayValueChanged( QtProperty *p, const QVariant &value );
	void createArray();
	
	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;

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
			p = NULL;
			name = "";
		}

		QString name;
		NLGEORGES::UFormElm *p;
	};

	CurrentNode m_currentNode;
};

#endif
