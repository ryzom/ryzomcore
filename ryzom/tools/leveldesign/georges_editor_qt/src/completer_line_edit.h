#ifndef COMPLETELINEEDIT_H
#define COMPLETELINEEDIT_H


// code from
// http://www.cppblog.com/biao/archive/2009/10/31/99873.html
// there was no license attached

#include <QtGui/QLineEdit>
#include <QStringList>

class QListView;
class QStringListModel;
class QModelIndex;

class CompleteLineEdit : public QLineEdit 
{
	Q_OBJECT

public:
	CompleteLineEdit(QWidget *parent = 0, QStringList words = QStringList());

	void setStringList(QStringList list);

public Q_SLOTS:
	void setCompleter(const QString &text);
	void completeText(const QModelIndex &index);

protected:
	virtual void keyPressEvent(QKeyEvent *e);
	//virtual void focusOutEvent(QFocusEvent *e);

private:
	QStringList _words;
	QListView *listView;
	QStringListModel *model;
};

#endif // COMPLETELINEEDIT_H