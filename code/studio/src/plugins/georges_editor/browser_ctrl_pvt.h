#ifndef BROWSER_CTRL_PVT_H
#define BROWSER_CTRL_PVT_H

#include <QObject>

namespace NLGEORGES
{
	class CFormElm;
	class UFormElm;
	class CFormElmStruct;
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
	void setupNode( NLGEORGES::UFormElm *node );
	void onValueChanged( QtProperty *p, const QVariant &value );

	QtVariantPropertyManager* manager() const{ return mgr; }
	void setRootNode( NLGEORGES::CFormElm *root ){ m_rootNode = root; }	
	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }

Q_SIGNALS:
	void arrayResized( const QString &name, int size );
	void modified();

private:
	void setupStruct( NLGEORGES::UFormElm *node );
	void setupArray( NLGEORGES::UFormElm *node );
	void setupAtom( NLGEORGES::CFormElmStruct::CFormElmStructElm &elm );

	void onStructValueChanged( QtProperty *p, const QVariant &value );
	void onArrayValueChanged( QtProperty *p, const QVariant &value );
	
	QtVariantPropertyManager *mgr;
	QtVariantEditorFactory *factory;
	QtTreePropertyBrowser *m_browser;

	NLGEORGES::UFormElm *m_currentNode;
	NLGEORGES::CFormElm *m_rootNode;
};

#endif
