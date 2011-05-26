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
	  m_settings(0)
{
	QCoreApplication::setApplicationName(QLatin1String("ObjectViewerQt"));
	QCoreApplication::setApplicationVersion(QLatin1String(Core::Constants::OVQT_VERSION_LONG));
	QCoreApplication::setOrganizationName(QLatin1String("RyzomCore"));

	setObjectName(Constants::MAIN_WINDOW);
	setWindowIcon(QIcon(Constants::ICON_PILL));
	setWindowTitle(tr("Object Viewer Qt"));

	m_pluginManager = pluginManager;
	m_settings = m_pluginManager->settings();
	m_coreImpl = new CoreImpl(this);

	m_menuManager = new MenuManager(this);
	m_menuManager->setMenuBar(menuBar());

	m_tabWidget = new QTabWidget(this);
	m_tabWidget->setTabPosition(QTabWidget::South);
	m_tabWidget->setMovable(false);
	m_tabWidget->setDocumentMode(true);
	setCentralWidget(m_tabWidget);

	m_contextManager = new ContextManager(m_tabWidget);

	setDockNestingEnabled(true);
	m_originalPalette = QApplication::palette();

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
	QList<IContext *> listContexts = m_pluginManager->getObjects<IContext>();

	Q_FOREACH(IContext *context, listContexts)
	{
		addContextObject(context);
	}

	connect(m_pluginManager, SIGNAL(objectAdded(QObject *)), this, SLOT(checkObject(QObject *)));
	readSettings();
	show();
}

IMenuManager *MainWindow::menuManager() const
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

ExtensionSystem::IPluginManager *MainWindow::pluginManager() const
{
	return m_pluginManager;
}

void MainWindow::open()
{
}

void MainWindow::checkObject(QObject *obj)
{
	IContext *context = qobject_cast<IContext *>(obj);
	if (context)
		addContextObject(context);
}

bool MainWindow::showOptionsDialog(const QString &group,
								   const QString &page,
								   QWidget *parent)
{
	if (!parent)
		parent = this;
	CSettingsDialog settingsDialog(m_pluginManager, group, page, parent);
	settingsDialog.show();
	bool ok = settingsDialog.execDialog();
	if (ok)
		Q_EMIT m_coreImpl->changeSettings();
	return ok;
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Object Viewer Qt"),
					   tr("<h2>Object Viewer Qt</h2>"
						  "<p> Ryzom Core team <p>Compiled on %1 %2").arg(__DATE__).arg(__TIME__));
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

void MainWindow::addContextObject(IContext *context)
{
	QWidget *tabWidget = new QWidget(m_tabWidget);
	m_tabWidget->addTab(tabWidget, context->icon(), context->trName());
	QGridLayout *gridLayout = new QGridLayout(tabWidget);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout_") + context->id());
	gridLayout->setContentsMargins(0, 0, 0, 0);
	gridLayout->addWidget(context->widget(), 0, 0, 1, 1);
}

void MainWindow::createActions()
{
	m_openAction = new QAction(tr("&Open..."), this);
	m_openAction->setIcon(QIcon(Constants::ICON_OPEN));
	m_openAction->setShortcut(QKeySequence::Open);
	m_openAction->setStatusTip(tr("Open an existing file"));
	menuManager()->registerAction(m_openAction, Constants::OPEN);
	connect(m_openAction, SIGNAL(triggered()), this, SLOT(open()));

	m_exitAction = new QAction(tr("E&xit"), this);
	m_exitAction->setShortcut(QKeySequence(tr("Ctrl+Q")));
	m_exitAction->setStatusTip(tr("Exit the application"));
	menuManager()->registerAction(m_exitAction, Constants::EXIT);
	connect(m_exitAction, SIGNAL(triggered()), this, SLOT(close()));

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
	m_fileMenu = menuBar()->addMenu(tr("&File"));
	menuManager()->registerMenu(m_fileMenu, Constants::M_FILE);
//	m_fileMenu->addAction(m_openAction);
	m_fileMenu->addSeparator();
	m_fileMenu->addAction(m_exitAction);

	m_editMenu = menuBar()->addMenu(tr("&Edit"));
	menuManager()->registerMenu(m_editMenu, Constants::M_EDIT);

	m_viewMenu = menuBar()->addMenu(tr("&View"));
	menuManager()->registerMenu(m_viewMenu, Constants::M_VIEW);

	m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
	menuManager()->registerMenu(m_toolsMenu, Constants::M_TOOLS);

	m_sheetMenu = m_toolsMenu->addMenu(tr("&Sheet"));
	menuManager()->registerMenu(m_sheetMenu, Constants::M_SHEET);

//	m_toolsMenu->addSeparator();

	m_toolsMenu->addAction(m_settingsAction);

	menuBar()->addSeparator();

	m_helpMenu = menuBar()->addMenu(tr("&Help"));
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
	m_pluginView = new ExtensionSystem::CPluginView(m_pluginManager, this);
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
