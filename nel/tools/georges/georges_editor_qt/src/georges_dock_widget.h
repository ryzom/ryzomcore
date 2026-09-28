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

#ifndef GEORGES_DOCK_WIDGET_H
#define GEORGES_DOCK_WIDGET_H

#include <QWidget>
#include <QTreeWidget>

class GeorgesEditorDoc;
class CGeorgesEditDocSub;

/**
 * Left tree view widget showing document structure, ported from CLeftView.
 * Placed as the left pane of a QSplitter in each MDI child.
 */
class GeorgesDockWidget : public QWidget
{
	Q_OBJECT

public:
	GeorgesDockWidget(QWidget *parent = nullptr);
	~GeorgesDockWidget();

	void setDocument(GeorgesEditorDoc *doc);
	void refreshTree();
	void expandAll();
	void collapseAll();

	QTreeWidget *treeWidget() const { return _treeWidget; }

signals:
	void itemSelected(CGeorgesEditDocSub *sub);

private slots:
	void onItemSelectionChanged();
	void onCustomContextMenu(const QPoint &pos);
	void onCopy();
	void onPaste();
	void onCut();
	void onInsert();
	void onDelete();
	void onRename();

private:
	void populateTree();
	void populateTreeItem(QTreeWidgetItem *parentItem, CGeorgesEditDocSub *sub);
	QIcon getIconForSub(CGeorgesEditDocSub *sub) const;

	QTreeWidget *_treeWidget;
	GeorgesEditorDoc *_doc;
};

#endif // GEORGES_DOCK_WIDGET_H
