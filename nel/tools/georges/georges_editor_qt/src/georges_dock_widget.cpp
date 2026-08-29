// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "georges_dock_widget.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QApplication>
#include <QClipboard>

#include "georges_editor_doc.h"

GeorgesDockWidget::GeorgesDockWidget(QWidget *parent)
	: QWidget(parent), _doc(nullptr)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	_treeWidget = new QTreeWidget(this);
	_treeWidget->setHeaderLabel(tr("Document Structure"));
	_treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	_treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	layout->addWidget(_treeWidget);

	connect(_treeWidget, &QTreeWidget::itemSelectionChanged, this, &GeorgesDockWidget::onItemSelectionChanged);
	connect(_treeWidget, &QTreeWidget::customContextMenuRequested, this, &GeorgesDockWidget::onCustomContextMenu);
}

GeorgesDockWidget::~GeorgesDockWidget()
{
}

void GeorgesDockWidget::setDocument(GeorgesEditorDoc *doc)
{
	_doc = doc;
	refreshTree();
}

void GeorgesDockWidget::refreshTree()
{
	_treeWidget->clear();
	if (_doc)
		populateTree();
}

void GeorgesDockWidget::expandAll()
{
	_treeWidget->expandAll();
}

void GeorgesDockWidget::collapseAll()
{
	_treeWidget->collapseAll();
}

void GeorgesDockWidget::populateTree()
{
	CGeorgesEditDocSub &root = _doc->getDocTree();

	QTreeWidgetItem *rootItem = new QTreeWidgetItem(_treeWidget);
	rootItem->setText(0, QString::fromUtf8(root.getName().c_str()));
	rootItem->setIcon(0, QIcon(":/icons/root.ico"));
	rootItem->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<void *>(&root)));

	for (uint i = 0; i < root.getChildrenCount(); i++)
	{
		CGeorgesEditDocSub *child = root.getChild(i);
		if (child)
			populateTreeItem(rootItem, child);
	}

	_treeWidget->expandAll();
}

void GeorgesDockWidget::populateTreeItem(QTreeWidgetItem *parentItem, CGeorgesEditDocSub *sub)
{
	QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
	item->setText(0, QString::fromUtf8(sub->getName().c_str()));
	item->setIcon(0, getIconForSub(sub));
	item->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<void *>(sub)));

	for (uint i = 0; i < sub->getChildrenCount(); i++)
	{
		CGeorgesEditDocSub *child = sub->getChild(i);
		if (child)
			populateTreeItem(item, child);
	}
}

QIcon GeorgesDockWidget::getIconForSub(CGeorgesEditDocSub *sub) const
{
	switch (sub->getType())
	{
	case CGeorgesEditDocSub::Header:
		return QIcon(":/icons/header.ico");
	case CGeorgesEditDocSub::Type:
		return QIcon(":/icons/struct.ico");
	case CGeorgesEditDocSub::Dfn:
		return QIcon(":/icons/vstruct.ico");
	case CGeorgesEditDocSub::Form:
		return QIcon(":/icons/struct.ico");
	default:
		return QIcon(":/icons/root.ico");
	}
}

void GeorgesDockWidget::onItemSelectionChanged()
{
	QList<QTreeWidgetItem *> items = _treeWidget->selectedItems();
	if (items.isEmpty())
		return;

	QTreeWidgetItem *item = items.first();
	CGeorgesEditDocSub *sub = static_cast<CGeorgesEditDocSub *>(item->data(0, Qt::UserRole).value<void *>());
	emit itemSelected(sub);
}

void GeorgesDockWidget::onCustomContextMenu(const QPoint &pos)
{
	QTreeWidgetItem *item = _treeWidget->itemAt(pos);
	if (!item)
		return;

	QMenu menu(this);
	menu.addAction(tr("Copy"), this, &GeorgesDockWidget::onCopy);
	menu.addAction(tr("Paste"), this, &GeorgesDockWidget::onPaste);
	menu.addAction(tr("Cut"), this, &GeorgesDockWidget::onCut);
	menu.addSeparator();
	menu.addAction(tr("Insert"), this, &GeorgesDockWidget::onInsert);
	menu.addAction(tr("Delete"), this, &GeorgesDockWidget::onDelete);
	menu.addAction(tr("Rename"), this, &GeorgesDockWidget::onRename);
	menu.exec(_treeWidget->mapToGlobal(pos));
}

void GeorgesDockWidget::onCopy()
{
	// Stub
}

void GeorgesDockWidget::onPaste()
{
	// Stub
}

void GeorgesDockWidget::onCut()
{
	// Stub
}

void GeorgesDockWidget::onInsert()
{
	// Stub
}

void GeorgesDockWidget::onDelete()
{
	// Stub
}

void GeorgesDockWidget::onRename()
{
	QList<QTreeWidgetItem *> items = _treeWidget->selectedItems();
	if (items.isEmpty())
		return;

	QTreeWidgetItem *item = items.first();
	bool ok;
	QString newName = QInputDialog::getText(this, tr("Rename"), tr("New name:"),
		QLineEdit::Normal, item->text(0), &ok);
	if (ok && !newName.isEmpty())
	{
		item->setText(0, newName);
	}
}
