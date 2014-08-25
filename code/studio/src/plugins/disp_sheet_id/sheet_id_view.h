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

#ifndef SHEET_ID_VIEW_H
#define SHEET_ID_VIEW_H

#include "ui_sheet_id_view.h"

#include "nel/misc/types_nl.h"
#include "nel/misc/sheet_id.h"

#include <QtGui/QDialog>

class SheetIdView : public QDialog
{
	Q_OBJECT

public:
	explicit SheetIdView(QWidget *parent = 0);
	~SheetIdView();

public Q_SLOTS:
	void pushToTable();

private:
	std::vector<NLMISC::CSheetId>	m_sheetList;
	Ui::SheetIdView m_ui;
};

#endif // SHEET_ID_VIEW_H
