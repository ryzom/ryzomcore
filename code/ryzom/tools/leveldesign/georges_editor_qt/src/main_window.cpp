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

#include "main_window.h"

// STL includes

// Qt includes
#include <QtGui/QtGui>

// NeL includes

// Project includes
#include "modules.h"
#include "settings_dialog.h"
#include "log_dialog.h"
#include "objectviewer_dialog.h"
#include "georges_dirtree_dialog.h"
#include "georges_treeview_dialog.h"

using namespace std;
using namespace NLMISC;

namespace NLQT
{

	CMainWindow::CMainWindow(QWidget *parent)
		: QMainWindow(parent), _GeorgesLogDialog(0), _ObjectViewerDialog(0), 
		_GeorgesDirTreeDialog(0)
	{
		setWindowTitle("Qt Georges Editor");

		setDockNestingEnabled(true);
		setCentralWidget(0);
		setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
		setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

		// create log dock widget
		_GeorgesLogDialog = new CGeorgesLogDialog(this);
		addDockWidget(Qt::RightDockWidgetArea, _GeorgesLogDialog);

		// empty form view as placeholder
		createEmptyView();
		_currentView = 0;

		// load and set leveldesign path from config
		QString ldPath = Modules::config().configLeveldesignPath().c_str();
		QFileInfo info(ldPath);
		if (!info.isDir()) 
			ldPath = "";

		// create georges dir dock widget
		_GeorgesDirTreeDialog = new CGeorgesDirTreeDialog(ldPath, this);
		addDockWidget(Qt::LeftDockWidgetArea, _GeorgesDirTreeDialog);
		if (ldPath == "") 
		{
			if (QMessageBox::information(this, tr("Missing leveldesign path"), 
				tr("Your leveldesign path seems to be empty or incorrect.\nDo you want to set it now?"),
				QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) 
			{
				settings();
			}
		}

		// create NeL viewport dock widget
		_ObjectViewerDialog = new CObjectViewerDialog(this);
		addDockWidget(Qt::LeftDockWidgetArea, _ObjectViewerDialog);

		createActions();
		createMenus();
		createToolBars();
		createStatusBar();

		//_ObjectViewerDialog->toggleViewAction()->trigger();
		QSettings settings("georges_editor_qt.ini", QSettings::IniFormat);
		settings.beginGroup("WindowSettings");
		restoreState(settings.value("QtWindowState").toByteArray());
		restoreGeometry(settings.value("QtWindowGeometry").toByteArray());
		settings.endGroup();

		setWindowIcon(QIcon(":/images/khead.png"));

		_statusBarTimer = new QTimer(this);
		connect(_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
		_statusBarTimer->start(5000);

		connect(_GeorgesDirTreeDialog, SIGNAL(selectedForm(QString)), 
			this, SLOT(openTreeView(QString)));
	}

	CMainWindow::~CMainWindow()
	{
		// save state & geometry of window and widgets
		QSettings settings("georges_editor_qt.ini", QSettings::IniFormat);
		settings.beginGroup("WindowSettings");
		settings.setValue("QtWindowState", saveState());
		settings.setValue("QtWindowGeometry", saveGeometry());
		settings.endGroup();

		_statusBarTimer->stop();

		delete _ObjectViewerDialog;
		delete _GeorgesDirTreeDialog;
		delete _GeorgesLogDialog;
	}

	void CMainWindow::openTreeView(QString file) 
	{
		// create or/and raise tree view dock widget for current file

		setCurrentFile(file);

		CGeorgesTreeViewDialog *newView = 0;

		Q_FOREACH(CGeorgesTreeViewDialog* dlg, _treeViewList)
		{
			if (dlg->loadedForm == file)
				newView = dlg;
		}
		if (!newView) 
		{
			newView	= new CGeorgesTreeViewDialog(this);
			//newView->setAllowedAreas(Qt::TopDockWidgetArea | Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
			newView->setWindowTitle(file);

			if (_treeViewList.isEmpty()) 
			{
				_emptyView->deleteLater();
				tabifyDockWidget(_emptyView, newView);
			} 
			else 
			{
				tabifyDockWidget(_currentView,newView);
				QTabBar* tb = Modules::mainWin().getTabBar();
				if (tb)
				{
					disconnect(tb, SIGNAL(currentChanged ( int ) ),
						this,SLOT(tabChanged(int)));
					connect(tb, SIGNAL(currentChanged ( int ) ),
						this,SLOT(tabChanged(int)));
				}
			}

			_treeViewList.append(newView);
			newView->selectedForm(file);
			_currentView = newView;

			connect(newView, SIGNAL(changeFile(QString)), 
				_GeorgesDirTreeDialog, SLOT(changeFile(QString)));
			connect(newView, SIGNAL(modified(bool)), 
				_saveAction, SLOT(setEnabled(bool)));
		}
		QApplication::processEvents();
		newView->raise();
	}

	void CMainWindow::settings()
	{
		CSettingsDialog _settingsDialog(this);

		connect(&_settingsDialog,SIGNAL(ldPathChanged(QString)),
			_GeorgesDirTreeDialog,SLOT(ldPathChanged(QString)));

		//_settingsDialog.show();
		_settingsDialog.exec();
	}

	void CMainWindow::about()
	{
		QMessageBox::about(this, tr("About Georges Viewer Qt"),
			tr("<h2>Georges Viewer Qt</h2>"
			"Author: aquiles<br>Credits:Thx to dnk-88 for parts of his code"));
	}

	void CMainWindow::updateStatusBar()
	{
		//if (_isGraphicsInitialized) 
		//  statusBar()->showMessage(QString(Modules::objView().getDriver()->getVideocardInformation()));
	}

	void CMainWindow::open()
	{
		/*QStringList fileNames = QFileDialog::getOpenFileNames(this,
		tr("Open NeL data file"), _lastDir,
		tr("All NeL files (*.shape *.ps);;"
		"NeL shape files (*.shape);;"
		"NeL particle system files (*.ps)"));

		setCursor(Qt::WaitCursor);
		if (!fileNames.isEmpty()) 
		{
		QStringList list = fileNames;
		QStringList::Iterator it = list.begin();
		_lastDir = QFileInfo(*it).absolutePath();

		QString skelFileName = QFileDialog::getOpenFileName(this,
		tr("Open skeleton file"), _lastDir,
		tr("NeL skeleton file (*.skel)"));

		while(it != list.end()) 
		{
		loadFile(*it, skelFileName);
		++it;
		}  
		_AnimationSetDialog->updateListObject();
		_AnimationSetDialog->updateListAnim();
		_SlotManagerDialog->updateUiSlots();
		}
		setCursor(Qt::ArrowCursor);*/
	}

	void CMainWindow::save()
	{
		if(!_currentView)
			return;

		setCursor(Qt::WaitCursor);

		_currentView->write();
		setWindowTitle(windowTitle().remove("*"));
		_saveAction->setEnabled(false);

		setCursor(Qt::ArrowCursor);
	}

	void CMainWindow::create()
	{
	}

	void CMainWindow::createEmptyView(QDockWidget* w)
	{
		_emptyView = new CGeorgesTreeViewDialog(this, true);
		if(w)
		{
			tabifyDockWidget(w, _emptyView);
		}
		else
		{
			addDockWidget(Qt::TopDockWidgetArea, _emptyView);
		}
	}

	void CMainWindow::createActions()
	{	    
		_openAction = new QAction(tr("&Open..."), this);
		_openAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
		_openAction->setShortcut(QKeySequence::Open);
		_openAction->setStatusTip(tr("Open an existing file"));
		connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

		_createAction = new QAction(tr("&New..."), this);
		_createAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_FileDialogNewFolder));
		_createAction->setShortcut(QKeySequence::New);
		_createAction->setStatusTip(tr("Create a new file"));
		connect(_createAction, SIGNAL(triggered()), this, SLOT(create()));

		_saveAction = new QAction(tr("&Save..."), this);
		_saveAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton));
		_saveAction->setShortcut(QKeySequence::Save);
		_saveAction->setStatusTip(tr("Saves the current file"));
		_saveAction->setDisabled(true);
		connect(_saveAction, SIGNAL(triggered()), this, SLOT(save()));

		_exitAction = new QAction(tr("E&xit"), this);
		_exitAction->setShortcut(QKeySequence::Close);
		_exitAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCloseButton));
		_exitAction->setStatusTip(tr("Exit the application"));
		connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

		_setBackColorAction = _ObjectViewerDialog->createSetBackgroundColor(this);
		_setBackColorAction->setText(tr("Set &background color"));
		_setBackColorAction->setStatusTip(tr("Set background color"));

		_settingsAction = new QAction(tr("&Settings"), this);
		_settingsAction->setIcon(QIcon(":/images/preferences.png"));
		_settingsAction->setStatusTip(tr("Settings"));
		connect(_settingsAction, SIGNAL(triggered()), this, SLOT(settings()));

		_aboutAction = new QAction(tr("&About"), this);
		_aboutAction->setStatusTip(tr("Show the application's About box"));
		connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

		_aboutQtAction = new QAction(tr("About &Qt"), this);
		_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
		connect(_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

		for (int i = 0; i < MaxRecentFiles; ++i) {
			recentFileActs[i] = new QAction(this);
			recentFileActs[i]->setVisible(false);
			connect(recentFileActs[i], SIGNAL(triggered()),
				this, SLOT(openRecentFile()));
		}
	}

	void CMainWindow::createMenus()
	{
		_fileMenu = menuBar()->addMenu(tr("&File"));
		_fileMenu->addAction(_createAction);
		_fileMenu->addAction(_openAction);
		_fileMenu->addAction(_saveAction);
		_separatorAction = _fileMenu->addSeparator();
		for (int i = 0; i < MaxRecentFiles; ++i)
			_fileMenu->addAction(recentFileActs[i]);
		_fileMenu->addSeparator();
		_fileMenu->addAction(_exitAction);
		updateRecentFileActions();

		_viewMenu = menuBar()->addMenu(tr("&View"));
		_viewMenu->addAction(_setBackColorAction);
		if (_GeorgesDirTreeDialog)
			_viewMenu->addAction(_GeorgesDirTreeDialog->toggleViewAction());

		_toolsMenu = menuBar()->addMenu(tr("&Tools"));
		if (_ObjectViewerDialog) 
		{
			_toolsMenu->addAction(_ObjectViewerDialog->toggleViewAction());
			_ObjectViewerDialog->toggleViewAction()->setIcon(QIcon(":/images/pqrticles.png"));
		}
		_toolsMenu->addSeparator();
		_toolsMenu->addAction(_settingsAction);

		menuBar()->addSeparator();

		_helpMenu = menuBar()->addMenu(tr("&Help"));
		_helpMenu->addAction(_aboutAction);
		_helpMenu->addAction(_aboutQtAction);
	}

	void CMainWindow::createToolBars()
	{
		_fileToolBar = addToolBar(tr("&File"));
		_fileToolBar->addAction(_createAction);
		_fileToolBar->addAction(_openAction);
		_fileToolBar->addAction(_saveAction);

		_toolsBar = addToolBar(tr("&Tools"));
		if (_ObjectViewerDialog)
			_toolsBar->addAction(_ObjectViewerDialog->toggleViewAction());
	}

	void CMainWindow::createStatusBar()
	{
		statusBar()->showMessage(tr("StatusReady"));
	}

	void CMainWindow::cfcbQtStyle(NLMISC::CConfigFile::CVar &var)
	{
		QApplication::setStyle(QStyleFactory::create(var.asString().c_str()));
	}

	void CMainWindow::cfcbQtPalette(NLMISC::CConfigFile::CVar &var)
	{
		if (var.asBool()) 
			QApplication::setPalette(QApplication::style()->standardPalette());
		else 
			QApplication::setPalette(_originalPalette);
	}

	QTabBar* CMainWindow::getTabBar()
	{
		// get the QTabBar
		QList<QTabBar *> tabList = findChildren<QTabBar *>();
		//tabList = _mainWindow->findChildren<QTabBar *>();
		//nlinfo(QString("%1 %2").arg(QString::number((int)this,16)).
		//		arg(QString::number((int)_mainWindow,16)).
		//		toStdString().c_str());
		QTabBar *tb = 0;
		Q_FOREACH(QTabBar *tabBar, tabList)
		{
			if (tabBar->parent() != this)
				continue;
			//nlinfo(QString("%1 %2 %3 %4").arg(tabBar->objectName()).
			//	arg(QString::number((int)tabBar,16)).
			//	arg(QString::number((int)tabBar->parentWidget(),16)).
			//	arg(QString::number((int)tabBar->parent(),16)).
			//	toStdString().c_str());
			for (int i = 0; i < tabBar->count(); i++)
			{
				QString currentTab = tabBar->tabText(i);
				//nlinfo(currentTab.toStdString().c_str());
			}
			tb = tabBar;
		}
		return tb;
	}

	void CMainWindow::tabChanged(int index)
	{
		if (index == -1) 
		{
			setWindowTitle("Qt Georges Editor");
			return;
		}

		QTabBar *tb = getTabBar();
		//nlinfo(QString("%1").arg(index).toStdString().c_str());

		Q_FOREACH(CGeorgesTreeViewDialog* dlg, _treeViewList) 
		{
			if (dlg->windowTitle() == tb->tabText(index))
			{
				//nlinfo(QString("%1 modified %2").arg(tb->tabText(index)).
				//	arg(dlg->modified()).
				//	toStdString().c_str());
				_currentView = dlg;
				setWindowTitle("Qt Georges Editor - " + tb->tabText(index));
				_saveAction->setEnabled(dlg->modified());
			}
		}
	}

	void CMainWindow::openRecentFile()
	{
		QAction *action = qobject_cast<QAction *>(sender());
		if (action)
			loadFile(action->data().toString());
	}

	void CMainWindow::setCurrentFile(const QString &fileName)
	{
		//curFile = fileName;
		//setWindowFilePath(curFile);

		QSettings settings("georges_editor_qt.ini", QSettings::IniFormat);
		settings.beginGroup("RecentFileList");
		QStringList files = settings.value("List").toStringList();
		settings.endGroup();

		files.removeAll(fileName);
		files.prepend(fileName);
		while (files.size() > MaxRecentFiles)
			files.removeLast();

		settings.beginGroup("RecentFileList");
		settings.setValue("List",files);
		settings.endGroup();

		Q_FOREACH (QWidget *widget, QApplication::topLevelWidgets())
		{
			CMainWindow *mainWin = qobject_cast<CMainWindow *>(widget);
			if (mainWin)
				mainWin->updateRecentFileActions();
		}
	}

	void CMainWindow::updateRecentFileActions()
	{
		QSettings settings("georges_editor_qt.ini", QSettings::IniFormat);
		settings.beginGroup("RecentFileList");
		QStringList files = settings.value("List").toStringList();
		settings.endGroup();
		int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

		for (int i = 0; i < numRecentFiles; ++i) 
		{
			QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
			recentFileActs[i]->setText(text);
			recentFileActs[i]->setData(files[i]);
			recentFileActs[i]->setVisible(true);
		}
		for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
			recentFileActs[j]->setVisible(false);

		_separatorAction->setVisible(numRecentFiles > 0);
	}

	void CMainWindow::loadFile(const QString &fileName)
	{
		/*QFile file(fileName);
		if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Recent Files"),
		tr("Cannot read file %1:\n%2.")
		.arg(fileName)
		.arg(file.errorString()));
		return;
		}

		QTextStream in(&file);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		textEdit->setPlainText(in.readAll());
		QApplication::restoreOverrideCursor();*/

		openTreeView(fileName);
		setCurrentFile(fileName);
		//statusBar()->showMessage(tr("File loaded"), 2000);
	}

} /* namespace NLQT */

/* end of file */
