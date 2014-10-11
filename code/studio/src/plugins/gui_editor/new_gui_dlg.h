// Ryzom Core Studio - GUI Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

#ifndef NEW_GUI_DLG_H
#define NEW_GUI_DLG_H

#include "ui_new_gui_dlg.h"

class NewGUIDlg : public QDialog
{
	Q_OBJECT

public:
	NewGUIDlg( QWidget *parent = NULL );
	~NewGUIDlg();

	QString getProjectName() const;
	QString getWindowName() const;

private Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();

private:
	Ui::NewGUIDialog m_ui;
};

#endif

