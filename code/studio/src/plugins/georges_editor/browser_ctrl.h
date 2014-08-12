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

private Q_SLOTS:
	void onValueChanged( QtProperty *p, const QVariant &value );

private:
	void enableMgrConnections();
	void disableMgrConnections();


	BrowserCtrlPvt *m_pvt;
	NLGEORGES::UForm *m_form;
};

#endif
