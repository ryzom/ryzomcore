/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes

#include <QtGui/QMainWindow>
#include <QPlainTextEdit>

// NeL includes
#include <nel/misc/config_file.h>

// Project includes

namespace NLMISC
{
	class CConfigFile;
}

namespace NLQT 
{

	class CGeorgesLogDialog;
	class CObjectViewerDialog;
	class CGeorgesDirTreeDialog;
	class CGeorgesTreeViewDialog;

	class CMainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		CMainWindow(QWidget *parent = 0);
		~CMainWindow();

		CGeorgesTreeViewDialog* getEmptyView() { return _emptyView;}
		QList<CGeorgesTreeViewDialog*>& getTreeViewList() { return _treeViewList; }
		QTabBar* getTabBar();
		void createEmptyView(QDockWidget* w = 0);
		void closeEvent(QCloseEvent *event);

		QString leveldesignPath() { return _leveldesignPath; }

	private Q_SLOTS:
		void open();
		void create();
		void save();
		void settings();
		void about();
		void updateStatusBar();
		CGeorgesTreeViewDialog * createTreeView(QString);
		void tabChanged(int);
		void openRecentFile();
		void loadFile(QString fileName);

	private:
		void createActions();
		void createMenus();
		void createToolBars();
		void createStatusBar();
		void createDialogs();
		
		void updateRecentFileActions();
		void setCurrentFile(const QString &fileName);

		void cfcbQtStyle(NLMISC::CConfigFile::CVar &var);
		void cfcbQtPalette(NLMISC::CConfigFile::CVar &var);

		CGeorgesLogDialog *_GeorgesLogDialog;
		CObjectViewerDialog *_ObjectViewerDialog;
		CGeorgesDirTreeDialog *_GeorgesDirTreeDialog;
		QList<CGeorgesTreeViewDialog*> _treeViewList;
		CGeorgesTreeViewDialog *_emptyView;
		CGeorgesTreeViewDialog *_currentView;

		QPalette _originalPalette;

		QTimer *_statusBarTimer;

		QMenu *_fileMenu;
		QMenu *_viewMenu;
		QMenu *_toolsMenu;
		QMenu *_helpMenu;
		QToolBar *_fileToolBar;
		QToolBar *_editToolBar;
		QToolBar *_toolsBar;
		QAction *_openAction;
		QAction *_createAction;
		QAction *_saveAction;
		QAction *_exitAction;
		QAction *_setBackColorAction;
		QAction *_settingsAction;
		QAction *_aboutAction;
		QAction *_aboutQtAction;
		QAction *_separatorAction;

		QString _leveldesignPath;

		enum { MaxRecentFiles = 5 };
		QAction *recentFileActs[MaxRecentFiles];

	};/* class CMainWindow */

} /* namespace NLQT */

#endif // MAIN_WINDOW_H
