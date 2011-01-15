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

// Project includes
#include "main_window.h"
#include "core_constants.h"
#include "settings_dialog.h"

// Qt includes
#include <QtGui/QtGui>

namespace Core
{

CMainWindow::CMainWindow(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent)
	: QMainWindow(parent),
	  _lastDir(".")
{
	_pluginManager = pluginManager;

	setObjectName(Constants::MAIN_WINDOW);

	_tabWidget = new QTabWidget(this);
	setCentralWidget(_tabWidget);

	QWidget *qwidg1 = new QWidget(_tabWidget);
	QWidget *qwidg2 = new QWidget(_tabWidget);

	_tabWidget->addTab(qwidg1, "tab1");
	_tabWidget->addTab(qwidg2, "tab2");


	QGridLayout *gridLayout = new QGridLayout(qwidg1);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	NLQT::QNLWidget *_nelWidget = new NLQT::QNLWidget(qwidg1);
	_nelWidget->setObjectName(QString::fromUtf8("NELWIdget1"));
	gridLayout->addWidget(_nelWidget, 0, 0, 1, 1);

	QGridLayout *gridLayout2 = new QGridLayout(qwidg2);
	NLQT::QNLWidget *_nelWidget2 = new NLQT::QNLWidget(qwidg2);
	_nelWidget2->setObjectName(QString::fromUtf8("NELWIdget2"));
	gridLayout2->addWidget(_nelWidget2, 0, 0, 1, 1);

	setDockNestingEnabled(true);

	_originalPalette = QApplication::palette();

	createDialogs();
	createActions();
	createMenus();
	createStatusBar();

	setWindowIcon(QIcon(Constants::ICON_NEL));
	setWindowTitle(tr("Object Viewer Qt"));
}

CMainWindow::~CMainWindow()
{
	delete _pluginView;
}

bool CMainWindow::showOptionsDialog(const QString &group,
									const QString &page,
									QWidget *parent)
{
	if (!parent)
		parent = this;
	CSettingsDialog _settingsDialog(_pluginManager, group, page, parent);
	_settingsDialog.show();
	return _settingsDialog.execDialog();
}

void CMainWindow::about()
{
	QMessageBox::about(this, tr("About Object Viewer Qt"),
					   tr("<h2>Object Viewer Qt NG</h2>"
						  "<p> Author: dnk-88 <p>Compiled on %1 %2").arg(__DATE__).arg(__TIME__));
}

void CMainWindow::createActions()
{
	_openAction = new QAction(tr("&Open..."), this);
	_openAction->setIcon(QIcon(":/images/open-file.png"));
	_openAction->setShortcut(QKeySequence::Open);
	_openAction->setStatusTip(tr("Open an existing file"));
//	connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

	_exitAction = new QAction(tr("E&xit"), this);
	_exitAction->setShortcut(tr("Ctrl+Q"));
	_exitAction->setStatusTip(tr("Exit the application"));
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	_settingsAction = new QAction(tr("&Settings"), this);
	_settingsAction->setIcon(QIcon(":/images/preferences.png"));
	_settingsAction->setStatusTip(tr("Open the settings dialog"));
	connect(_settingsAction, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

	_aboutAction = new QAction(tr("&About"), this);
	_aboutAction->setStatusTip(tr("Show the application's About box"));
	connect(_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	_aboutQtAction = new QAction(tr("About &Qt"), this);
	_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	connect(_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	_pluginViewAction = new QAction(tr("About &Plugins"), this);
	_pluginViewAction->setStatusTip(tr("Show the plugin view dialog"));
	connect(_pluginViewAction, SIGNAL(triggered()), _pluginView, SLOT(show()));
}

void CMainWindow::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("&File"));
	_fileMenu->setObjectName(Constants::M_FILE);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_editMenu = menuBar()->addMenu(tr("&Edit"));
	_editMenu->setObjectName(Constants::M_EDIT);

	_viewMenu = menuBar()->addMenu(tr("&View"));
	_viewMenu->setObjectName(Constants::M_VIEW);

	_toolsMenu = menuBar()->addMenu(tr("&Tools"));
	_toolsMenu->setObjectName(Constants::M_TOOLS);


	_toolsMenu->addSeparator();

	_toolsMenu->addAction(_settingsAction);

	menuBar()->addSeparator();

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->setObjectName(Constants::M_HELP);
	_helpMenu->addAction(_aboutAction);
	_helpMenu->addAction(_aboutQtAction);
	_helpMenu->addAction(_pluginViewAction);

	_pluginManager->addObject(_fileMenu);
	_pluginManager->addObject(_editMenu);
	_pluginManager->addObject(_viewMenu);
	_pluginManager->addObject(_toolsMenu);
	_pluginManager->addObject(_helpMenu);
}

void CMainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("StatusReady"));
}

void CMainWindow::createDialogs()
{
	_pluginView = new ExtensionSystem::CPluginView(_pluginManager, this);
}

} /* namespace Core */

/* end of file */
