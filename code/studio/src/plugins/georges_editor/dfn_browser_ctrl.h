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

class DFNBrowserCtrl : public QObject
{
	Q_OBJECT
public:
	DFNBrowserCtrl( QObject *parent = NULL );
	~DFNBrowserCtrl();

	void setBrowser( QtTreePropertyBrowser *browser ){ m_browser = browser; }
	void setDFN( NLGEORGES::CFormDfn *dfn ){ m_dfn = dfn; }

	void onElementSelected( int idx );

private:
	QtTreePropertyBrowser *m_browser;
	NLGEORGES::CFormDfn *m_dfn;

	QtVariantPropertyManager *m_manager;
	QtVariantEditorFactory *m_factory;
};

#endif

