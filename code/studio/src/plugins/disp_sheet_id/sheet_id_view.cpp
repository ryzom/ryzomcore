// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "sheet_id_view.h"

#include "nel/misc/path.h"

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>

class	CPred
{
public:
	bool	operator()(const NLMISC::CSheetId &a, const NLMISC::CSheetId &b)
	{
		return a.toString()<b.toString();
	}
};

SheetIdView::SheetIdView(QWidget *parent)
	: QDialog(parent)
{
	m_ui.setupUi(this);
	connect(m_ui.reloadButton, SIGNAL(clicked()), this, SLOT(pushToTable()));
}

SheetIdView::~SheetIdView()
{
}

void SheetIdView::pushToTable()
{
	// Check sheet_id.bin
	try
	{
		NLMISC::CPath::lookup("sheet_id.bin");
	}
	catch (NLMISC::Exception &e)
	{
		QMessageBox::critical(this, e.what(), tr("Path not found for sheet_id.bin. Add in the settings search path to the file"), QMessageBox::Ok);
		return;
	}
	// Load sheet_id.bin file
	NLMISC::CSheetId::init(false);
	NLMISC::CSheetId::buildIdVector(m_sheetList);
	CPred	Pred;
	std::sort(m_sheetList.begin(), m_sheetList.end(), Pred);

	// Fill table
	m_ui.table->clear();
	m_ui.table->setRowCount(m_sheetList.size());
	m_ui.table->setColumnCount(2);
	for (size_t i = 0; i < m_sheetList.size(); i++)
	{
		QTableWidgetItem *item1 = new QTableWidgetItem(QString(m_sheetList[i].toString().c_str()));
		QTableWidgetItem *item2 = new QTableWidgetItem(QString("%1").arg(m_sheetList[i].asInt()));
		m_ui.table->setItem(i,1,item1);
		m_ui.table->setItem(i,2,item2);
	}
}