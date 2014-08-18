// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
// Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include "../core/ui_settings_dialog.h"

// Qt includes
#include <QtCore/QList>

// Project includes
#include "../../extension_system/iplugin_manager.h"

namespace Core
{
class IOptionsPage;

/**
@class CSettingsDialog
@brief Settings dialog
*/
class SettingsDialog: public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(ExtensionSystem::IPluginManager *pluginManager,
				   const QString &initialCategory = QString(),
				   const QString &initialPage = QString(),
				   QWidget *parent = 0);

	~SettingsDialog();

	/// Run the dialog and return true if 'Ok' was choosen or 'Apply' was invoked at least once
	bool execDialog();

public Q_SLOTS:
	void done(int);

private Q_SLOTS:
	void pageSelected();
	void accept();
	void reject();
	void apply();

private:
	QList<IOptionsPage *> m_pages;
	bool m_applied;
	QString m_currentCategory;
	QString m_currentPage;

	ExtensionSystem::IPluginManager *m_plugMan;

	Ui::SettingsDialog m_ui;
}; /* class CSettingsDialog */

} /* namespace Core */

#endif // SETTINGS_DIALOG_H
