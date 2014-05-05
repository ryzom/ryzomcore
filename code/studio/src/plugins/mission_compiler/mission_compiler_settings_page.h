// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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


#ifndef MISSION_COMPILER_SETTINGS_PAGE_H
#define MISSION_COMPILER_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "../core/ioptions_page.h"

#include "ui_mission_compiler_settings_page.h"

class QWidget;

namespace MissionCompiler
{
/**
@class MissionCompilerSettingsPage
*/
class MissionCompilerSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT

public:
	MissionCompilerSettingsPage(QObject *parent = 0);
	~MissionCompilerSettingsPage();

	QString id() const;
	QString trName() const;
	QString category() const;
	QString trCategory() const;
	QIcon categoryIcon() const;
	QWidget *createPage(QWidget *parent);

	void apply();
	void finish();

private Q_SLOTS:
	void addServer();
	void delServer();
	void editServer(int row, int column);

private:
	void readSettings();
	void writeSettings();

	QWidget *m_page;
	Ui::MissionCompilerSettingsPage m_ui;
};

} // namespace MissionCompiler

#endif // MISSION_COMPILER_SETTINGS_PAGE_H
