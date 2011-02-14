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

// Project includes
#include "../../extension_system/iplugin.h"
#include "plugin_view_dialog.h"

// STL includes

// Qt includes
#include <QtGui/QMainWindow>
#include <QtCore/QSettings>

namespace Core
{
class CSettingsDialog;
class CorePlugin;
class IAppPage;

class CMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CMainWindow(CorePlugin *corePlugin, QWidget *parent = 0);
	~CMainWindow();

private Q_SLOTS:
	void checkObject(QObject *obj);
	bool showOptionsDialog(const QString &group = QString(),
						   const QString &page = QString(),
						   QWidget *parent = 0);
	void about();
protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void addAppPage(IAppPage *appPage);

	void createActions();
	void createMenus();
	void createStatusBar();
	void createDialogs();

	void readSettings();
	void writeSettings();

	ExtensionSystem::IPluginManager *_pluginManager;
	ExtensionSystem::CPluginView *_pluginView;
	CorePlugin *_corePlugin;

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
