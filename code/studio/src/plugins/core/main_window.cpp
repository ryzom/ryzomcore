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
#include "icontext.h"
#include "icore_listener.h"
#include "menu_manager.h"
#include "context_manager.h"
#include "core.h"
#include "core_constants.h"
#include "settings_dialog.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QCoreApplication>
#include <QtGui/QUndoView>
#include <QtGui/QtGui>

namespace Core
{

MainWindow::MainWindow(ExtensionSystem::IPluginManager *pluginManager, QWidget *parent)
	: QMainWindow(parent),
	  m_pluginManager(0),
	  m_menuManager(0),
	  m_contextManager(0),
	  m_coreImpl(0),
	  m_lastDir("."),
	  m_undoGroup(0),
	  m_settings(0)
{
	QCoreApplication::setApplicationName(QLatin1String("Studio"));
	QCoreApplication::setApplicationVersion(QLatin1String(Core::Constants::OVQT_VERSION_LONG));
	QCoreApplication::setOrganizationName(QLatin1String("RyzomCore"));

	setObjectName(Constants::MAIN_WINDOW);
	setWindowIcon(QIcon(Constants::ICON_PILL));
	setWindowTitle(tr("Ryzom Core Studio"));

	m_pluginManager = pluginManager;
	m_settings = m_pluginManager->settings();
	m_coreImpl = new CoreImpl(this);

#ifdef Q_WS_MAC
	m_menuBar = new QMenuBar(0);
#else
	m_menuBar = new QMenuBar(this);
	setMenuBar(m_menuBar);
#endif

	m_menuManager = new MenuManager(m_menuBar, this);

	m_tabWidget = new QTabWidget(this);
	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setMovable(false);
	m_tabWidget->setDocumentMode(true);
	setCentralWidget(m_tabWidget);

	m_contextManager = new ContextManager(this, m_tabWidget);

	setDockNestingEnabled(true);
	m_originalPalette = QApplication::palette();
	m_undoGroup = new QUndoGroup(this);

	createDialogs();
	createActions();
	createMenus();
	createStatusBar();
	resize(1024, 768);
}

MainWindow::~MainWindow()
{
	m_pluginManager->removeObject(m_coreImpl);
	m_pluginManager->removeObject(m_menuManager);

	delete m_coreImpl;
	m_coreImpl = 0;
}

bool MainWindow::initialize(QString *errorString)
{
	Q_UNUSED(errorString);
	m_pluginManager->addObject(m_coreImpl);
	m_pluginManager->addObject(m_menuManager);
	return true;
}

void MainWindow::extensionsInitialized()
{
	readSettings();
	connect(m_contextManager, SIGNAL(currentContextChanged(Core::IContext *)),
			this, SLOT(updateContext(Core::IContext *)));
	if (m_contextManager->currentContext() != NULL)
		updateContext(m_contextManager->currentContext());
	show();
}

MenuManager *MainWindow::menuManager() const
{
	return m_menuManager;
}

ContextManager *MainWindow::contextManager() const
{
	return m_contextManager;
}

QSettings *MainWindow::settings() const
{
	return m_settings;
}

QUndoGroup *MainWindow::undoGroup() const
{
	return m_undoGroup;
}

ExtensionSystem::IPluginManager *MainWindow::pluginManager() const
{
	return m_pluginManager;
}

void MainWindow::addContextObject(IContext *context)
{
	QUndoStack *stack = context->undoStack();
	if (stack)
		m_undoGroup->addStack(stack);
}

void MainWindow::removeContextObject(IContext *context)
{
	QUndoStack *stack = context->undoStack();
	if (stack)
		m_undoGroup->removeStack(stack);
}

void MainWindow::open()
{
	m_contextManager->currentContext()->open();
}

void MainWindow::newFile()
{
	m_contextManager->currentContext()->newDocument();
}

void MainWindow::save()
{
	m_contextManager->currentContext()->save();
}

void MainWindow::saveAs()
{
	m_contextManager->currentContext()->saveAs();
}

void MainWindow::saveAll()
{
}

void MainWindow::closeDocument()
{
	m_contextManager->currentContext()->close();
}


void MainWindow::cut()
{
}

void MainWindow::copy()
{
}

void MainWindow::paste()
{
}

void MainWindow::del()
{
}

void MainWindow::find()
{
}

void MainWindow::gotoPos()
{
}

void MainWindow::setFullScreen(bool enabled)
{
	if (bool(windowState() & Qt::WindowFullScreen) == enabled)
		return;
	if (enabled)
		setWindowState(windowState() | Qt::WindowFullScreen);
	else
		setWindowState(windowState() & ~Qt::WindowFullScreen);
}

bool MainWindow::showOptionsDialog(const QString &group,
								   const QString &page,
								   QWidget *parent)
{
	if (!parent)
		parent = this;
	SettingsDialog settingsDialog(m_pluginManager, group, page, parent);
	settingsDialog.show();
	bool ok = settingsDialog.execDialog();
	if (ok)
		Q_EMIT m_coreImpl->changeSettings();
	return ok;
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Ryzom Core Studio"),
					   tr("<h2>Ryzom Core Studio</h2>"
						  "<p> Ryzom Core team <p>Compiled on %1 %2").arg(__DATE__).arg(__TIME__));
}

void MainWindow::updateContext(Core::IContext *context)
{
	m_undoGroup->setActiveStack(context->undoStack());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	QList<ICoreListener *> listeners = m_pluginManager->getObjects<ICoreListener>();
	Q_FOREACH(ICoreListener *listener, listeners)
	{
		if (!listener->closeMainWindow())
		{
			event->ignore();
			return;
		}
	}
	Q_EMIT m_coreImpl->closeMainWindow();

	writeSettings();
	event->accept();
}

void MainWindow::createActions()
{
	m_newAction = new QAction(tr("&New"), this);
	m_newAction->setIcon(QIcon(Constants::ICON_NEW));
	m_newAction->setShortcut(QKeySequence::New);
	menuManager()->registerAction(m_newAction, Constants::NEW);
	connect(m_newAction, SIGNAL(triggered()), this, SLOT(newFile()));
	m_newAction->setEnabled(false);

	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(Constants::ICON_OPEN));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing file"));
	menuManager()->registerAction(m_openAction, Constants::OPEN);
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_saveAction = new QAction(tr("&Save"), this);
	m_saveAction->setIcon(QIcon(Constants::ICON_SAVE));
	m_saveAction->setShortcut(QKeySequence::Save);
	menuManager()->registerAction(m_saveAction, Constants::SAVE);
	connect(m_saveAction, SIGNAL(triggered()), this, SLOT(save()));
	m_saveAction->setEnabled(false);

	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setIcon(QIcon(Constants::ICON_SAVE_AS));
	m_saveAsAction->setShortcut(QKeySequence::SaveAs);
	menuManager()->registerAction(m_saveAsAction, Constants::SAVE_AS);
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));
	m_saveAsAction->setEnabled(false);

	m_saveAllAction = new QAction(tr("&Save A&ll"), this);
	m_saveAllAction->setShortcut(QKeySequence::SelectAll);
	menuManager()->registerAction(m_saveAllAction, Constants::SAVE_ALL);
	connect(m_saveAllAction, SIGNAL(triggered()), this, SLOT(saveAll()));
	m_saveAllAction->setEnabled(false);

	m_closeAction = new QAction(tr("Close"), this);
	m_closeAction->setShortcut(QKeySequence::Close);
	menuManager()->registerAction(m_closeAction, Constants::CLOSE);
	connect(m_closeAction, SIGNAL(triggered()), this, SLOT(closeDocument()));
	m_closeAction->setEnabled(false);

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
	m_exitAction->setStatusTip(tr("Exit the application"));
	menuManager()->registerAction(m_exitAction, Constants::EXIT);
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	m_cutAction = new QAction(tr("Cu&t"), this);
	m_cutAction->setShortcut(QKeySequence::Cut);
	menuManager()->registerAction(m_cutAction, Constants::CUT);
	connect(m_cutAction, SIGNAL(triggered()), this, SLOT(cut()));
	m_cutAction->setEnabled(false);

	m_copyAction = new QAction(tr("&Copy"), this);
	m_copyAction->setShortcut(QKeySequence::Copy);
	menuManager()->registerAction(m_copyAction, Constants::COPY);
	connect(m_copyAction, SIGNAL(triggered()), this, SLOT(copy()));
	m_copyAction->setEnabled(false);

	m_pasteAction = new QAction(tr("&Paste"), this);
	m_pasteAction->setShortcut(QKeySequence::Paste);
	menuManager()->registerAction(m_pasteAction, Constants::PASTE);
	connect(m_pasteAction, SIGNAL(triggered()), this, SLOT(paste()));
	m_pasteAction->setEnabled(false);

	m_delAction = new QAction(tr("&Delete"), this);
	m_delAction->setShortcut(QKeySequence::Delete);
	menuManager()->registerAction(m_delAction, Constants::DEL);
	connect(m_delAction, SIGNAL(triggered()), this, SLOT(del()));
	m_delAction->setEnabled(false);

	m_selectAllAction = new QAction(tr("Select &All"), this);
	m_selectAllAction->setShortcut(QKeySequence::SelectAll);
	menuManager()->registerAction(m_selectAllAction, Constants::SELECT_ALL);
	connect(m_selectAllAction, SIGNAL(triggered()), this, SLOT(selectAll()));
	m_selectAllAction->setEnabled(false);

	m_findAction = new QAction(tr("&Find"), this);
	m_findAction->setShortcut(QKeySequence::Find);
	menuManager()->registerAction(m_findAction, Constants::FIND);
	connect(m_findAction, SIGNAL(triggered()), this, SLOT(find()));
	m_findAction->setEnabled(false);

	m_gotoAction = new QAction(tr("&Go To.."), this);
	m_gotoAction->setShortcut(QKeySequence(tr("Ctrl+G")));
	menuManager()->registerAction(m_gotoAction, Constants::GOTO_POS);
	connect(m_gotoAction, SIGNAL(triggered()), this, SLOT(gotoPos()));
	m_gotoAction->setEnabled(false);

	m_fullscreenAction = new QAction(tr("Fullscreen"), this);
	m_fullscreenAction->setCheckable(true);
	m_fullscreenAction->setShortcut(QKeySequence(tr("Ctrl+Shift+F11")));
	menuManager()->registerAction(m_fullscreenAction, Constants::TOGGLE_FULLSCREEN);
	connect(m_fullscreenAction, SIGNAL(triggered(bool)), this, SLOT(setFullScreen(bool)));

	m_settingsAction = new QAction(tr("&Settings"), this);
	m_settingsAction->setIcon(QIcon(":/images/preferences.png"));
	m_settingsAction->setShortcut(QKeySequence::Preferences);
	m_settingsAction->setStatusTip(tr("Open the settings dialog"));
	menuManager()->registerAction(m_settingsAction, Constants::SETTINGS);
	connect(m_settingsAction, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));

	m_aboutAction = new QAction(tr("&About"), this);
	m_aboutAction->setStatusTip(tr("Show the application's About box"));
	menuManager()->registerAction(m_aboutAction, Constants::ABOUT);
	connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

	m_aboutQtAction = new QAction(tr("About &Qt"), this);
	m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
	menuManager()->registerAction(m_aboutQtAction, Constants::ABOUT_QT);
	connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	m_pluginViewAction = new QAction(tr("About &Plugins"), this);
	m_pluginViewAction->setStatusTip(tr("Show the plugin view dialog"));
	menuManager()->registerAction(m_pluginViewAction, Constants::ABOUT_PLUGINS);
	connect(m_pluginViewAction, SIGNAL(triggered()), m_pluginView, SLOT(show()));

#ifdef Q_WS_MAC
	m_exitAction->setMenuRole(QAction::QuitRole);
	m_settingsAction->setMenuRole(QAction::PreferencesRole);
	m_aboutAction->setMenuRole(QAction::AboutRole);
	m_aboutQtAction->setMenuRole(QAction::AboutQtRole);
	m_pluginViewAction->setMenuRole(QAction::ApplicationSpecificRole);
#endif
}

void MainWindow::createMenus()
{
	m_fileMenu = m_menuBar->addMenu(tr("&File"));
	menuManager()->registerMenu(m_fileMenu, Constants::M_FILE);
	m_fileMenu->addAction(m_newAction);
	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_saveAction);
	m_fileMenu->addAction(m_saveAsAction);
	m_fileMenu->addAction(m_saveAllAction);
	m_fileMenu->addAction(m_closeAction);
	m_fileMenu->addSeparator();

	m_recentFilesMenu = m_fileMenu->addMenu(tr("Recent &Files"));
	m_recentFilesMenu->setEnabled(false);
	menuManager()->registerMenu(m_recentFilesMenu, Constants::M_FILE_RECENTFILES);

	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	m_editMenu = m_menuBar->addMenu(tr("&Edit"));
	QAction *undoAction = m_undoGroup->createUndoAction(this);
	menuManager()->registerAction(undoAction, Constants::UNDO);
	undoAction->setIcon(QIcon(Constants::ICON_UNDO));
	undoAction->setShortcut(QKeySequence::Undo);
	QAction *redoAction = m_undoGroup->createRedoAction(this);
	menuManager()->registerAction(redoAction, Constants::REDO);
	redoAction->setIcon(QIcon(Constants::ICON_REDO));
	redoAction->setShortcut(QKeySequence::Redo);
	m_editMenu->addAction(undoAction);
	m_editMenu->addAction(redoAction);

	m_editMenu->addSeparator();
	m_editMenu->addAction(m_cutAction);
	m_editMenu->addAction(m_copyAction);
	m_editMenu->addAction(m_pasteAction);
	m_editMenu->addAction(m_delAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_selectAllAction);
	m_editMenu->addSeparator();
	m_editMenu->addAction(m_findAction);
	m_editMenu->addAction(m_gotoAction);
	menuManager()->registerMenu(m_editMenu, Constants::M_EDIT);

	m_viewMenu = m_menuBar->addMenu(tr("&View"));
	m_viewMenu->addAction(m_fullscreenAction);
	m_viewMenu->addAction(m_dockWidget->toggleViewAction());
	menuManager()->registerMenu(m_viewMenu, Constants::M_VIEW);

	m_toolsMenu = m_menuBar->addMenu(tr("&Tools"));
	menuManager()->registerMenu(m_toolsMenu, Constants::M_TOOLS);

	m_sheetMenu = m_toolsMenu->addMenu(tr("&Sheet"));
	menuManager()->registerMenu(m_sheetMenu, Constants::M_SHEET);

//	m_toolsMenu->addSeparator();

	m_toolsMenu->addAction(m_settingsAction);

	m_menuBar->addSeparator();

	m_helpMenu = m_menuBar->addMenu(tr("&Help"));
	menuManager()->registerMenu(m_helpMenu, Constants::M_HELP);
	m_helpMenu->addAction(m_aboutAction);
	m_helpMenu->addAction(m_aboutQtAction);
	m_helpMenu->addAction(m_pluginViewAction);
}

void MainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("StatusReady"));
}

void MainWindow::createDialogs()
{
	m_pluginView = new PluginView(m_pluginManager, this);

	// Create undo/redo command list
	m_dockWidget = new QDockWidget("Command List", this);
	m_dockWidget->setObjectName(QString::fromUtf8("UndoRedoCommandDockWidget"));
	QUndoView *undoView = new QUndoView(m_undoGroup, m_dockWidget);
	m_dockWidget->setWidget(undoView);
	addDockWidget(Qt::RightDockWidgetArea, m_dockWidget);
}

void MainWindow::readSettings()
{
	m_settings->beginGroup(Constants::MAIN_WINDOW_SECTION);
	restoreState(m_settings->value(Constants::MAIN_WINDOW_STATE).toByteArray());
	restoreGeometry(m_settings->value(Constants::MAIN_WINDOW_GEOMETRY).toByteArray());
	m_settings->endGroup();
}

void MainWindow::writeSettings()
{
	m_settings->beginGroup(Constants::MAIN_WINDOW_SECTION);
	m_settings->setValue(Constants::MAIN_WINDOW_STATE, saveState());
	m_settings->setValue(Constants::MAIN_WINDOW_GEOMETRY, saveGeometry());
	m_settings->endGroup();
}

} /* namespace Core */

/* end of file */
