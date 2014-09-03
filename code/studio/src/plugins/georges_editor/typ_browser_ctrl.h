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
