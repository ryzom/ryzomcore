// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef EDITOR_WORKSHEET_H
#define EDITOR_WORKSHEET_H

// Project includes
#include "translation_manager_editor.h"
#include "extract_new_sheet_names.h"

// Nel includes
#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"
#include "nel/ligo/ligo_config.h"

// Qt includes
#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QTableWidget>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoCommand>
#include <QtGui/QUndoStack>


namespace TranslationManager
{

struct CTableWidgetItemStore
{
public:
	CTableWidgetItemStore(QTableWidgetItem *item, int row, int column) :
		m_item(item),
		m_row(row),
		m_column(column) { }

	QTableWidgetItem *m_item;
	int m_row;
	int m_column;
};

class CEditorWorksheet : public CEditor
{
	Q_OBJECT

public:
	CEditorWorksheet(QMdiArea *parent) : CEditor(parent) {}
	CEditorWorksheet() : CEditor() {}
	QTableWidget *table_editor;
	void open(QString filename);
	void save();
	void saveAs(QString filename);
	void activateWindow();
	void mergeWorksheetFile(QString filename);
	bool compareWorksheetFile(QString filename);
	void extractBotNames(std::list<std::string> filters, std::string level_design_path, NLLIGO::CLigoConfig ligoConfig);
	void extractWords(QString filename, QString columnId, IWordListBuilder &wordListBuilder);
	void insertTableRecords(QList<QString> records, QList<CTableWidgetItemStore> new_items);
	bool isBotNamesTable();
	bool isSheetTable(QString type);
	void closeEvent(QCloseEvent *event);

private Q_SLOTS:
	void worksheetEditorCellEntered(QTableWidgetItem *item);
	void worksheetEditorChanged(QTableWidgetItem *item);
	void insertRow();
	void deleteRow();
	void contextMenuEvent(QContextMenuEvent *e);

private:
	QString temp_content;
};

class CUndoWorksheetCommand : public QUndoCommand
{
public:
	CUndoWorksheetCommand(QTableWidget *table, QTableWidgetItem *item, const QString &ocontent, QUndoCommand *parent = 0) :  QUndoCommand("Insert characters in cells", parent), m_table(table), m_item(item),  m_ocontent(ocontent)
	{
		m_ccontent = m_ocontent;
	}

	void redo()
	{
		if(m_item->text() == m_ocontent)
		{
			m_item->setText(m_ccontent);
		}
	}
	void undo()
	{
		if(m_item->text() != m_ocontent)
		{
			m_ccontent = m_item->text();
		}
		m_item->setText(m_ocontent);
	}
private:
	QTableWidget *m_table;
	QTableWidgetItem *m_item;
	QString m_ocontent;
	QString m_ccontent;
};

class CUndoWorksheetNewCommand : public QUndoCommand
{
public:
	CUndoWorksheetNewCommand(QTableWidget *table, int rowID, QUndoCommand *parent = 0) :  QUndoCommand("Insert a new row", parent), m_table(table), m_rowID(rowID)
	{ 	}

	void redo()
	{
		m_table->setRowCount(m_rowID + 1);
		for(int j = 0; j < m_table->columnCount(); j++)
		{
			QTableWidgetItem *item = new QTableWidgetItem();
			m_table->setItem(m_rowID, j, item);
			m_table->scrollToBottom();
		}
	}

	void undo()
	{
		m_table->removeRow(m_rowID);
	}

private:
	QTableWidget *m_table;
	int m_rowID;
};

class CUndoWorksheetExtraction : public QUndoCommand
{
public:
	CUndoWorksheetExtraction(QList<CTableWidgetItemStore> items, QTableWidget *table, QUndoCommand *parent = 0) : QUndoCommand("Word extraction", parent),
		m_items(items),
		m_table(table)
	{   }

	void redo()
	{
		Q_FOREACH(CTableWidgetItemStore is, m_items)
		{
			m_table->setItem(is.m_row, is.m_column, is.m_item);
		}
	}

	void undo()
	{
		Q_FOREACH(CTableWidgetItemStore is, m_items)
		{
			m_table->setItem(is.m_row, is.m_column, is.m_item);
			m_table->takeItem(is.m_row, is.m_column);
		}
	}

private:
	QList<CTableWidgetItemStore> m_items;
	QTableWidget *m_table;
};

class CUndoWorksheetDeleteCommand : public QUndoCommand
{
public:
	CUndoWorksheetDeleteCommand(QTableWidget *table, int rowID, QUndoCommand *parent = 0) :  QUndoCommand("Delete row", parent), m_table(table), m_rowID(rowID)
	{ }

	void redo()
	{
		for(int i = 0; i < m_table->columnCount(); i++)
		{
			QTableWidgetItem *item = new QTableWidgetItem();
			QTableWidgetItem *table_item = m_table->item(m_rowID, i);
			item->setText(table_item->text());
			m_deletedItems.push_back(item);
		}
		m_table->removeRow(m_rowID);
	}

	void undo()
	{
		int lastRow = m_table->rowCount();
		m_table->setRowCount(m_table->rowCount() + 1);
		int i = 0;
		Q_FOREACH(QTableWidgetItem* item, m_deletedItems)
		{
			m_table->setItem(lastRow, i, item);
			i++;
		}
		m_deletedItems.clear();
	}

private:
	QList<QTableWidgetItem *> m_deletedItems;
	QTableWidget *m_table;
	int m_rowID;
};

}
#endif	/* EDITOR_WORKSHEET_H */

