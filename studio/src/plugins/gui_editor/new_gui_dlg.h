// Ryzom Core Studio - GUI Editor Plugin
//
// Copyright (C) 2010-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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
#include <QList>

class NewGUIDlg : public QDialog
{
	Q_OBJECT

public:
	NewGUIDlg( QWidget *parent = NULL );
	~NewGUIDlg();

	QString getProjectName() const;
	QString getWindowName() const;
	QString getProjectDirectory() const;
	void getMapList( QList< QString > &l );

private Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();
	void onProjectDirTBClicked();
	void onAddClicked();
	void onRemoveClicked();

private:
	Ui::NewGUIDialog m_ui;
};

#endif

