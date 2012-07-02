#ifndef ITEMS_EDITDLG_H
#define ITEMS_EDITDLG_H

#include <QtGui/QtGui>
#include <QtGui/QDialog>
#include "ui_items_edit_qt.h"

class CItems_edit_dlg : public QDialog
{
	Q_OBJECT

public:
    static QStringList getItems(QWidget *parent, const QString &title, const QString &label, const QStringList &availablelist, const QStringList &selectedlist, bool *ok = 0,Qt::WindowFlags f = 0);

	QStringList currentItems();

private slots:
	void on_addToItemsPushButton_clicked();
	void on_removeFromItemsPushButton_clicked();

private:	
	CItems_edit_dlg(QWidget *parent = 0, Qt::WindowFlags f = 0);
	void initDialog(const QString &, const QString &, const QStringList&, const QStringList&);
	Ui::ItemsEditDialog ui;
};

#endif

