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

#include "ui_settings_dialog.h"

// Qt includes
#include <QtCore/QList>

// Project includes
#include "../../extension_system/iplugin.h"

namespace Core
{
class CorePlugin;
class IOptionsPage;

/**
@class CSettingsDialog
@brief Settings dialog
*/
class CSettingsDialog: public QDialog
{
	Q_OBJECT

public:
	CSettingsDialog(CorePlugin *corePlugin,
					const QString &initialCategory = QString(),
					const QString &initialPage = QString(),
					QWidget *parent = 0);

	~CSettingsDialog();

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
	QList<IOptionsPage *> _pages;
	bool _applied;
	QString _currentCategory;
	QString _currentPage;

	ExtensionSystem::IPluginManager *_plugMan;

	Ui::CSettingsDialog _ui;
}; /* class CSettingsDialog */

} /* namespace Core */

#endif // SETTINGS_DIALOG_H
