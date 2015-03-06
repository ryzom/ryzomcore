// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef PROJECT_SETTINGS_DIALOG_H
#define PROJECT_SETTINGS_DIALOG_H

// Project includes
#include "ui_project_settings_dialog.h"

// Qt includes

namespace LandscapeEditor
{

class ProjectSettingsDialog: public QDialog
{
	Q_OBJECT

public:
	ProjectSettingsDialog(const QString &dataPath, QWidget *parent = 0);
	~ProjectSettingsDialog();

	QString dataPath() const;

private Q_SLOTS:
	void selectPath();

private:

	Ui::ProjectSettingsDialog m_ui;
}; /* class ProjectSettingsDialog */

} /* namespace LandscapeEditor */

#endif // PROJECT_SETTINGS_DIALOG_H
