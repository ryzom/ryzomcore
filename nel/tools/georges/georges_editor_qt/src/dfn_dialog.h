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

#ifndef DFN_DIALOG_H
#define DFN_DIALOG_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

class GeorgesEditorDoc;

/**
 * Right panel widget for editing a DFN (.dfn) node, ported from CDfnDialog.
 */
class DfnDialog : public QWidget
{
	Q_OBJECT

public:
	DfnDialog(QWidget *parent = nullptr);
	~DfnDialog();

	void setDocument(GeorgesEditorDoc *doc);
	void updateFromDocument();

private slots:
	void onParentsCellChanged(int row, int column);
	void onStructureCellChanged(int row, int column);
	void onAddParent();
	void onRemoveParent();
	void onAddStructEntry();
	void onRemoveStructEntry();
	void onBrowseParent();
	void onBrowseTypeName();

private:
	void setupUi();

	GeorgesEditorDoc *_doc;

	QTableWidget *_parentsTable;
	QPushButton *_addParentBtn;
	QPushButton *_removeParentBtn;

	QTableWidget *_structTable;
	QPushButton *_addStructBtn;
	QPushButton *_removeStructBtn;
};

#endif // DFN_DIALOG_H
