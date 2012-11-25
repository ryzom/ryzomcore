// Object Viewer Qt - BNP Manager Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Roland Winklmeier <roland.m.winklmeier@googlemail.com>
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

// Project includes
#include "bnp_filelist_dialog.h"
#include "bnp_file.h"

// Qt includes
#include <QtGui/QWidget>

// NeL includes
#include <nel/misc/debug.h>

using namespace std;

namespace BNPManager
{

BnpFileListDialog::BnpFileListDialog(QString bnpPath, QWidget *parent)
	:	QDockWidget(parent),
		m_DataPath(bnpPath)
{
	m_ui.setupUi(this);
}
// ***************************************************************************
BnpFileListDialog::~BnpFileListDialog()
{

}
// ***************************************************************************
void BnpFileListDialog::setupTable(int nbrows)
{
	// delete all old entries
	m_ui.tableWidget->clear();

	// set 2 colums: filename and size
	m_ui.tableWidget->setColumnCount(2);

	// set number of rows according to the number of files in the bnp file
	m_ui.tableWidget->setRowCount(nbrows);

	// only entire rows can be selected
	m_ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	
	// set the horizontal headers
	QStringList labels;
	labels << tr("Filename") << tr("Size");
	m_ui.tableWidget->setHorizontalHeaderLabels(labels);

	m_ui.tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
	m_ui.tableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch );
	m_ui.tableWidget->verticalHeader()->hide();

	// set vertical size a little bit smaller
	m_ui.tableWidget->verticalHeader()->setDefaultSectionSize(15);
	m_ui.tableWidget->setShowGrid(false);
    m_ui.tableWidget->setObjectName("tablewidget");
}
// ***************************************************************************
bool BnpFileListDialog::loadTable(const QString filePath)
{
	// reference to the BNPFileHandle singletone instance
	BNPFileHandle& myBNPFileHandle = BNPFileHandle::getInstance();
	// string vector of all packed files inside a bnp
	TPackedFilesList filelist;
	int row = 0;

	// read the header from the bnp file
	if (!myBNPFileHandle.readHeader( filePath.toUtf8().constData()) )
	{
		return false;
	}
	myBNPFileHandle.list( filelist );

	// create table with number of rows
	setupTable(filelist.size());

	// fill the table items
	TPackedFilesList::iterator it = filelist.begin();
	while (it != filelist.end() )
	{
		QTableWidgetItem *nameItem = new QTableWidgetItem (it->m_name.c_str() );
		QTableWidgetItem *sizeItem = new QTableWidgetItem (tr("%1 KB").arg(it->m_size));
		m_ui.tableWidget->setItem(row, 0, nameItem);
		m_ui.tableWidget->setItem(row, 1, sizeItem);
		it++;
		row++;
	}

	// Set the file path as the widgets title
	setWindowTitle(filePath);

	return true;
}
// ***************************************************************************
void BnpFileListDialog::clearTable()
{
	// create emtpy table
	setupTable(0);

	setWindowTitle("BNP File List");
}
// ***************************************************************************
void BnpFileListDialog::getSelections(TSelectionList& SelectionList)
{
	QModelIndex index;
	QAbstractItemModel *model = m_ui.tableWidget->model();
	QItemSelectionModel *selection = m_ui.tableWidget->selectionModel();
	QModelIndexList indexes = selection->selectedRows();

	Q_FOREACH(index, indexes)
	{
		QVariant data = model->data(index);
		QString filename = data.toString();
		SelectionList.push_back( filename.toUtf8().constData() );
	}
}

} // namespace BNPManager