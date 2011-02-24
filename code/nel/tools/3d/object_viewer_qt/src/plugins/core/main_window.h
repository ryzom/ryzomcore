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
#include "../../extension_system/iplugin_manager.h"
#include "plugin_view_dialog.h"

// STL includes

// Qt includes
#include <QtGui/QMainWindow>
#include <QtCore/QSettings>

namespace Core
{
class CSettingsDialog;
class CorePlugin;
class IContext;
class IMenuManager;
class MenuManager;
class CoreImpl;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent = 0);
	~MainWindow();

	bool initialize(QString *errorString);
	void extensionsInitialized();

	IMenuManager *menuManager() const;
	QSettings *settings() const;

	ExtensionSystem::IPluginManager *pluginManager() const;

public Q_SLOTS:
	bool showOptionsDialog(const QString &group = QString(),
						   const QString &page = QString(),
						   QWidget *parent = 0);

private Q_SLOTS:
	void checkObject(QObject *obj);
	void about();

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void addContextObject(IContext *appPage);

	void createActions();
	void createMenus();
	void createStatusBar();
	void createDialogs();

	void readSettings();
	void writeSettings();

	ExtensionSystem::IPluginManager *m_pluginManager;
	ExtensionSystem::CPluginView *m_pluginView;
	MenuManager *m_menuManager;
	CoreImpl *m_coreImpl;

	QPalette m_originalPalette;
	QString m_lastDir;

	QSettings *m_settings;

	QTimer *m_mainTimer;
	QTimer *m_statusBarTimer;

	QTabWidget *m_tabWidget;

	QMenu *m_fileMenu;
	QMenu *m_editMenu;
	QMenu *m_viewMenu;
	QMenu *m_toolsMenu;
	QMenu *m_helpMenu;

	QAction *m_openAction;
	QAction *m_exitAction;
	QAction *m_settingsAction;
	QAction *m_pluginViewAction;
	QAction *m_aboutAction;
	QAction *m_aboutQtAction;

};/* class MainWindow */

} /* namespace Core */

#endif // MAIN_WINDOW_H
