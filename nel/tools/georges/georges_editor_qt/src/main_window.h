// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>

#include <nel/misc/config_file.h>

#include "../../3d/shared_widgets/configuration.h"
#include "../../3d/shared_widgets/internationalization.h"

class FileBrowserDock;
class GeorgesEditorDoc;

namespace NLQT {
	class CCommandLogDisplayer;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void openDocument(const QString &path);
	void createNewType();
	void createNewDfn();
	void createNewForm(const QString &dfnName = QString());

	NLQT::CConfiguration &getConfiguration() { return m_Configuration; }
	const std::string &getProjectRoot() const { return m_ProjectRoot; }

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void onNewType();
	void onNewDfn();
	void onNewForm();
	void onOpen();
	void onSave();
	void onSaveAll();
	void onClose();
	void onCloseAll();
	void onUndo();
	void onRedo();
	void onCut();
	void onCopy();
	void onPaste();
	void onInsert();
	void onDelete();
	void onRename();
	void onExpandAll();
	void onCollapseAll();
	void onToggleFileBrowser();
	void onToggleOutputConsole();
	void onRefresh();
	void onAbout();
	void onSubWindowActivated(QMdiSubWindow *window);
	void onOpenRecentFile();
	void updateMenus();

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void createDockWidgets();
	QMdiSubWindow *findMdiChild(const QString &fileName) const;

	// Translation methods (called on language change)
	void translateActions();
	void translateMenus();
	void translateToolBars();
	void translateDockWindows();

	// Configuration callbacks
	void cfcbQtStyle(NLMISC::CConfigFile::CVar &var);
	void cfcbQtPalette(NLMISC::CConfigFile::CVar &var);
	void cfcbRootSearchDirectory(NLMISC::CConfigFile::CVar &var);

	// Internationalization callback
	void incbLanguageCode();

	// Recent files
	void addRecentFile(const QString &filePath);
	void updateRecentFileActions();

	// NeL configuration and i18n (following nel_qt pattern)
	NLQT::CConfiguration m_Configuration;
	NLQT::CInternationalization m_Internationalization;
	QPalette m_OriginalPalette;
	std::string m_ProjectRoot;

	QMdiArea *_mdiArea;
	FileBrowserDock *_fileBrowser;

	// Console dock using shared_widgets CCommandLogDisplayer
	NLQT::CCommandLogDisplayer *_commandLog;
	QDockWidget *_commandLogDock;

	// File menu actions
	QAction *_newTypeAction;
	QAction *_newDfnAction;
	QAction *_newFormAction;
	QAction *_openAction;
	QAction *_saveAction;
	QAction *_saveAllAction;
	QAction *_closeAction;
	QAction *_closeAllAction;
	QAction *_exitAction;

	// Edit menu actions
	QAction *_undoAction;
	QAction *_redoAction;
	QAction *_cutAction;
	QAction *_copyAction;
	QAction *_pasteAction;
	QAction *_insertAction;
	QAction *_deleteAction;
	QAction *_renameAction;
	QAction *_expandAllAction;
	QAction *_collapseAllAction;

	// Hold/Fetch actions
	QAction *_hold1Action;
	QAction *_hold2Action;
	QAction *_hold3Action;
	QAction *_hold4Action;
	QAction *_fetch1Action;
	QAction *_fetch2Action;
	QAction *_fetch3Action;
	QAction *_fetch4Action;

	// View menu actions
	QAction *_fileBrowserAction;
	QAction *_refreshAction;

	// Help menu actions
	QAction *_aboutAction;

	// Recent files
	enum { MaxRecentFiles = 10 };
	QAction *_recentFileActions[MaxRecentFiles];
	QMenu *_recentMenu;

	// Menus
	QMenu *_fileMenu;
	QMenu *_editMenu;
	QMenu *_viewMenu;
	QMenu *_helpMenu;

	// Toolbars
	QToolBar *_fileToolBar;
	QToolBar *_editToolBar;
};

#endif // MAIN_WINDOW_H
