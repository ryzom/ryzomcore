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


#ifndef GENERAL_SETTINGS_PAGE_H
#define GENERAL_SETTINGS_PAGE_H

#include <QtCore/QObject>

#include "ioptions_page.h"

#include "ui_general_settings_page.h"

class QWidget;

namespace Core
{
/**
@class GeneralSettingsPage
*/
class GeneralSettingsPage : public Core::IOptionsPage
{
	Q_OBJECT

public:
	explicit GeneralSettingsPage(QObject *parent = 0);
	~GeneralSettingsPage();

	QString id() const;
	QString trName() const;
	QString category() const;
	QString trCategory() const;
	QIcon categoryIcon() const;
	QWidget *createPage(QWidget *parent);

	void apply();
	void finish();

	void applyGeneralSettings();

private Q_SLOTS:
	void changeLanguage(const QString &lang);
	void setPluginsPath();
	void setLevelDesignPath();
	void setAssetsPath();
	void setPrimitivesPath();
	void setLigoConfigFile();

private:
	void readSettings();
	void writeSettings();

	QPalette m_originalPalette;
	QWidget *m_page;
	Ui::GeneralSettingsPage m_ui;
};

} // namespace Core

#endif // GENERAL_SETTINGS_H
