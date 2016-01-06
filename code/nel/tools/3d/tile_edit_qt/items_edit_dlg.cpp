#include "common.h"
#include <QtCore/QString>
#include <QtCore/QStringList>
#include "items_edit_dlg.h"



CItems_edit_dlg::CItems_edit_dlg(QWidget *parent, Qt::WindowFlags flags)
     : QDialog(parent, flags)
 {
	 ui.setupUi(this);
 }



void CItems_edit_dlg::initDialog(const QString &title, const QString &label, const QStringList& availableItemsList, const QStringList& selectedItemsList)
{
	this->setWindowTitle(title);
	ui.itemNameLabel->setText(label);
	ui.availableItemsListWidget->addItems(availableItemsList);
	ui.selectedItemsListWidget->addItems(selectedItemsList);
}


void CItems_edit_dlg::on_addToItemsPushButton_clicked()
{
	int nindex = ui.availableItemsListWidget->currentRow();
	if (nindex != -1) 
	{
		QListWidgetItem* item = ui.availableItemsListWidget->takeItem(nindex);
		ui.selectedItemsListWidget->addItem(item);
	}
}
void CItems_edit_dlg::on_removeFromItemsPushButton_clicked()
{
	int nindex = ui.selectedItemsListWidget->currentRow();
	if (nindex != -1) 
	{
		QListWidgetItem* item = ui.selectedItemsListWidget->takeItem(nindex);
		ui.availableItemsListWidget->addItem(item);
	}
}

QStringList CItems_edit_dlg::currentItems()
{
	QStringList allSelected;
	for(int index = 0; index < ui.selectedItemsListWidget->count(); index++)
	{
		allSelected.append(ui.selectedItemsListWidget->item(index)->text());
	}
	return allSelected;
}

QStringList CItems_edit_dlg::getItems(QWidget *parent, const QString &title, const QString &label, const QStringList &availableList, const QStringList &selectedList, bool *ok, Qt::WindowFlags f)
{
    CItems_edit_dlg dlg(parent, f);
	dlg.initDialog(title, label, availableList, selectedList);
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return dlg.currentItems();
}

