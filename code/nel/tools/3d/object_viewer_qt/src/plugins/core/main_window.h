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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

// STL includes

// Qt includes
#include <QtGui/QMainWindow>
#include <QtCore/QSettings>

// Project includes
#include "qnel_widget.h"
#include "../../extension_system/iplugin.h"
#include "plugin_view_dialog.h"

namespace Core
{
class CSettingsDialog;

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent = 0);
	~CMainWindow();

	inline QSettings *settings() const
	{
		return _settings;
	}

private Q_SLOTS:
	bool showOptionsDialog(const QString &group = QString(),
						   const QString &page = QString(),
						   QWidget *parent = 0);
	void about();

private:
	void createActions();
	void createMenus();
	void createStatusBar();
	void createDialogs();

	ExtensionSystem::IPluginManager *_pluginManager;
	ExtensionSystem::CPluginView *_pluginView;

	QPalette _originalPalette;
	QString _lastDir;

	QSettings *_settings;

	QTimer *_mainTimer;
	QTimer *_statusBarTimer;

	QTabWidget *_tabWidget;

	QMenu *_fileMenu;
	QMenu *_editMenu;
	QMenu *_viewMenu;
	QMenu *_toolsMenu;
	QMenu *_helpMenu;

	QAction *_openAction;
	QAction *_exitAction;
	QAction *_settingsAction;
	QAction *_pluginViewAction;
	QAction *_aboutAction;
	QAction *_aboutQtAction;

};/* class CMainWindow */

} /* namespace Core */

#endif // MAIN_WINDOW_H
