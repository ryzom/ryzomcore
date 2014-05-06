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
#include <QtGui/QUndoGroup>
#include <QtCore/QSettings>

namespace Core
{
class CSettingsDialog;
class CorePlugin;
class IContext;
class MenuManager;
class ContextManager;
class CoreImpl;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent = 0);
	~MainWindow();

	bool initialize(QString *errorString);
	void extensionsInitialized();

	MenuManager *menuManager() const;
	ContextManager *contextManager() const;
	QSettings *settings() const;
	QUndoGroup *undoGroup() const;

	ExtensionSystem::IPluginManager *pluginManager() const;

	void addContextObject(IContext *context);
	void removeContextObject(IContext *context);

public Q_SLOTS:
	bool showOptionsDialog(const QString &group = QString(),
						   const QString &page = QString(),
						   QWidget *parent = 0);
	void updateContext(Core::IContext *context);

private Q_SLOTS:
	void open();
	void newFile();
	void save();
	void saveAs();
	void saveAll();
	void closeDocument();
	void cut();
	void copy();
	void paste();
	void del();
	void find();
	void gotoPos();
	void setFullScreen(bool enabled);
	void about();

protected:
	virtual void closeEvent(QCloseEvent *event);

private:
	void createActions();
	void createMenus();
	void createStatusBar();
	void createDialogs();

	void readSettings();
	void writeSettings();

	ExtensionSystem::IPluginManager *m_pluginManager;
	PluginView *m_pluginView;
	MenuManager *m_menuManager;
	ContextManager *m_contextManager;
	CoreImpl *m_coreImpl;

	QPalette m_originalPalette;
	QString m_lastDir;

	QDockWidget *m_dockWidget;
	QUndoGroup *m_undoGroup;
	QSettings *m_settings;

	QTimer *m_mainTimer;
	QTimer *m_statusBarTimer;

	QTabWidget *m_tabWidget;

	QMenu *m_fileMenu;
	QMenu *m_recentFilesMenu;
	QMenu *m_editMenu;
	QMenu *m_viewMenu;
	QMenu *m_toolsMenu;
	QMenu *m_helpMenu;
	QMenuBar *m_menuBar;
	QMenu *m_sheetMenu;

	QAction *m_newAction;
	QAction *m_openAction;
	QAction *m_saveAction;
	QAction *m_saveAsAction;
	QAction *m_saveAllAction;
	QAction *m_closeAction;
	QAction *m_exitAction;
	QAction *m_cutAction;
	QAction *m_copyAction;
	QAction *m_pasteAction;
	QAction *m_delAction;
	QAction *m_selectAllAction;
	QAction *m_findAction;
	QAction *m_gotoAction;
	QAction *m_fullscreenAction;
	QAction *m_settingsAction;
	QAction *m_pluginViewAction;
	QAction *m_aboutAction;
	QAction *m_aboutQtAction;

};/* class MainWindow */

} /* namespace Core */

#endif // MAIN_WINDOW_H
