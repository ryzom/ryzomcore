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

#include <nel/misc/types_nl.h>
#include "main_window.h"

// STL includes

// Qt includes
#include <QtGui/QtGui>
#include <QtGui/QTreeView>
#include <QtGui/QDirModel>
#include <QtGui/QUndoStack>
#include <QtGui/QScrollArea>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/i18n.h>
#include <nel/3d/u_driver.h>

// Project includes
#include "command_log.h"
#include "graphics_viewport.h"
#include "graphics_config.h"

using namespace std;
using namespace NLMISC;

namespace NLQT {

namespace {

QString nli18n(const char *label)
{
	return QString::fromUtf16(CI18N::get(label).c_str());
}

} /* anonymous namespace */

CMainWindow::CMainWindow(const QMap<QString, QSize> &customSizeHints, QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL), 
	m_IsGraphicsInitialized(false), m_IsGraphicsEnabled(false), 
	m_IsSoundInitialized(false), m_IsSoundEnabled(false), 
	m_GraphicsViewport(NULL), 
	m_CommandLog(NULL), m_CommandLogDock(NULL), 
	m_GraphicsConfig(NULL), m_GraphicsConfigScroll(NULL), m_GraphicsConfigDock(NULL), 
	m_FileMenu(NULL), m_EditMenu(NULL), m_ViewportMenu(NULL), m_WidgetsMenu(NULL), m_HelpMenu(NULL), 
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_AboutAct(NULL), m_QuitAct(NULL), m_PrintDebugAct(NULL), 
	m_UndoAct(NULL), m_RedoAct(NULL), m_SaveScreenshotAct(NULL)
{
	setObjectName("CMainWindow");
	
	m_UndoStack = new QUndoStack(this);

	m_Configuration.init();
	
	m_OriginalPalette = QApplication::palette();
	m_Configuration.setAndCallback("QtStyle", CConfigCallback(this, &CMainWindow::cfcbQtStyle));
	m_Configuration.setAndCallback("QtPalette", CConfigCallback(this, &CMainWindow::cfcbQtPalette));
	
	m_Internationalization.init(&m_Configuration);
	m_Internationalization.enableCallback(CEmptyCallback(this, &CMainWindow::incbLanguageCode));

	m_GraphicsViewport = new CGraphicsViewport(this);
    setCentralWidget(m_GraphicsViewport);
	
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();
	
	incbLanguageCode();

	recalculateMinimumWidth();
	
	// As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the window system's event queue have been processed. This can be used to do heavy work while providing a snappy user interface.
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(updateRender()));
	// timer->start(); // <- timeout 0
	// it's heavy on cpu, though, when no 3d driver initialized :)
	timer->start(40); // 25fps
	
	m_IsGraphicsEnabled = m_GraphicsConfig->getGraphicsEnabled();
	connect(m_GraphicsConfig, SIGNAL(applyGraphicsConfig()), this, SLOT(applyGraphicsConfig()));
	m_Configuration.setAndCallback("SoundEnabled", CConfigCallback(this, &CMainWindow::cfcbSoundEnabled));
}

CMainWindow::~CMainWindow()
{
	delete m_GraphicsConfig; m_GraphicsConfig = NULL;

	m_Configuration.dropCallback("SoundEnabled");
	updateInitialization(false);
	m_Internationalization.disableCallback(CEmptyCallback(this, &CMainWindow::incbLanguageCode));
	m_Internationalization.release();
	m_Configuration.dropCallback("QtPalette");
	m_Configuration.dropCallback("QtStyle");
	m_Configuration.release();
}

void CMainWindow::setVisible(bool visible)
{
	// called by show()
	// code assuming visible window needed to init the 3d driver
	if (visible != isVisible())
	{
		if (visible)
		{
			QMainWindow::setVisible(true);
			updateInitialization(true);
		}
		else
		{
			updateInitialization(false);
			QMainWindow::setVisible(false);
		}
	}
}

void CMainWindow::updateInitialization(bool visible)
{
	bool done;
	do
	{
		done = true; // set false whenever change
		bool wantSound = m_IsSoundEnabled && visible;
		bool wantGraphics = m_IsGraphicsEnabled && visible;
		// bool wantLandscape = wantGraphics && m_IsGraphicsInitialized && isLandscapeEnabled;

		// .. stuff that depends on other stuff goes on top to prioritize deinitialization

		// Landscape
		// ...

		// Graphics (Driver)
		if (m_IsGraphicsInitialized)
		{
			if (!wantGraphics)
			{
				m_IsGraphicsInitialized = false;
				if (m_IsSoundInitialized)
					m_SoundUtilities.releaseGraphics();
				m_GraphicsViewport->release();
				done = false;
			}
		}
		else
		{
			if (wantGraphics)
			{
				m_GraphicsViewport->init(m_GraphicsConfig);
				if (m_IsSoundInitialized)
					m_SoundUtilities.initGraphics(m_GraphicsViewport);
				m_IsGraphicsInitialized = true;
				done = false;
			}
		}
		
		// Sound (AudioMixer)
		if (m_IsSoundInitialized)
		{
			if (!wantSound)
			{
				m_IsSoundInitialized = false;
				if (m_IsGraphicsInitialized)
					m_SoundUtilities.releaseGraphics();
				m_SoundUtilities.release();
				done = false;
			}
		}
		else
		{
			if (wantSound)
			{
				
				m_SoundUtilities.init(&m_Configuration, &m_Internationalization);
				if (m_IsGraphicsInitialized)
					m_SoundUtilities.initGraphics(m_GraphicsViewport);
				m_IsSoundInitialized = true;
				done = false;
			}
		}

	} while (!done);
}

void CMainWindow::createActions()
{
	m_QuitAct = new QAction(this);
	m_QuitAct->setShortcuts(QKeySequence::Quit);	
	connect(m_QuitAct, SIGNAL(triggered()), this, SLOT(close()));
	
	m_AboutAct = new QAction(this);
	connect(m_AboutAct, SIGNAL(triggered()), this, SLOT(about()));

	m_PrintDebugAct = new QAction(this);
	connect(m_PrintDebugAct, SIGNAL(triggered()), this, SLOT(printDebug()));

	m_UndoAct = m_UndoStack->createUndoAction(this);
	m_UndoAct->setShortcuts(QKeySequence::Undo);
	m_RedoAct = m_UndoStack->createRedoAction(this);
	m_RedoAct->setShortcuts(QKeySequence::Redo);

	m_SaveScreenshotAct = m_GraphicsViewport->createSaveScreenshotAction(this);
}

void CMainWindow::translateActions()
{
	m_QuitAct->setText(nli18n("ActionQuit"));
	m_QuitAct->setStatusTip(nli18n("ActionQuitStatusTip"));
	m_AboutAct->setText(nli18n("ActionAbout"));
	m_AboutAct->setStatusTip(nli18n("ActionAboutStatusTip"));
	m_PrintDebugAct->setText(nli18n("ActionPrintDebug"));
	m_PrintDebugAct->setStatusTip(nli18n("ActionPrintDebugStatusTip"));
	m_UndoAct->setText(nli18n("ActionUndo"));
	m_UndoAct->setStatusTip(nli18n("ActionUndoStatusTip"));
	m_RedoAct->setText(nli18n("ActionRedo"));
	m_RedoAct->setStatusTip(nli18n("ActionRedoStatusTip"));
	m_SaveScreenshotAct->setText(nli18n("ActionSaveScreenshot"));
	m_SaveScreenshotAct->setStatusTip(nli18n("ActionSaveScreenshotStatusTip"));
}

void CMainWindow::createMenus()
{
	m_FileMenu = menuBar()->addMenu(QString::null);
	//m_FileMenu->addAction(saveAct);
	//m_FileMenu->addSeparator();
	m_FileMenu->addAction(m_QuitAct);

	m_EditMenu = menuBar()->addMenu(QString::null);
	m_EditMenu->addAction(m_UndoAct);
	m_EditMenu->addAction(m_RedoAct);

	m_ViewportMenu = menuBar()->addMenu(QString::null);
	m_ViewportMenu->addAction(m_SaveScreenshotAct);
	
	m_WidgetsMenu = menuBar()->addMenu(QString::null);
	
	menuBar()->addSeparator();
	
	m_HelpMenu = menuBar()->addMenu(QString::null);
	m_HelpMenu->addAction(m_AboutAct);
}

void CMainWindow::translateMenus()
{
	m_FileMenu->setTitle(nli18n("MenuFile"));
	m_EditMenu->setTitle(nli18n("MenuEdit"));
	m_ViewportMenu->setTitle(nli18n("MenuViewport"));
	m_WidgetsMenu->setTitle(nli18n("MenuWidgets"));
	m_HelpMenu->setTitle(nli18n("MenuHelp"));
}

void CMainWindow::createToolBars()
{
	m_FileToolBar = addToolBar(QString::null);
	m_FileToolBar->addAction(m_QuitAct);
	m_FileToolBar->addAction(m_PrintDebugAct);

	m_EditToolBar = addToolBar(QString::null);
	m_EditToolBar->addAction(m_AboutAct);
}

void CMainWindow::translateToolBars()
{
	m_FileToolBar->setWindowTitle(nli18n("BarFile"));
	m_EditToolBar->setWindowTitle(nli18n("BarEdit"));
}

void CMainWindow::createStatusBar()
{
	statusBar()->showMessage(nli18n("StatusReady"));
}

void CMainWindow::createDockWindows()
{
	//QDockWidget *dock = new QDockWidget(tr("Test1"), this);
	//dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//customerList = new QListWidget(dock);
	//dock->setWidget(customerList);
	//addDockWidget(Qt::RightDockWidgetArea, dock);
	//m_WidgetsMenu->addAction(dock->toggleViewAction());

	//dock = new QDockWidget(tr("Test2"), this);
	//paragraphsList = new QListWidget(dock);
	//dock->setWidget(paragraphsList);
	//addDockWidget(Qt::RightDockWidgetArea, dock);
	//m_WidgetsMenu->addAction(dock->toggleViewAction());

	//connect(customerList, SIGNAL(currentTextChanged(QString)),
	//	this, SLOT(insertCustomer(QString)));
	//connect(paragraphsList, SIGNAL(currentTextChanged(QString)),
	//	this, SLOT(addParagraph(QString)));

	//dock = new QDockWidget(

	// CommandLog (Console)
	{
		m_CommandLogDock = new QDockWidget(this);
		m_CommandLogDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_CommandLog = new CCommandLog(m_CommandLogDock);
		m_CommandLogDock->setWidget(m_CommandLog);
		addDockWidget(Qt::BottomDockWidgetArea, m_CommandLogDock);
		m_WidgetsMenu->addAction(m_CommandLogDock->toggleViewAction());
	}

	// GraphicsConfig (Graphics Configuration)
	{
		m_GraphicsConfigDock = new QDockWidget(this);
		m_GraphicsConfigDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_GraphicsConfigScroll = new QScrollArea();
		m_GraphicsConfigScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_GraphicsConfigScroll->setWidgetResizable(true);
		m_GraphicsConfig = new CGraphicsConfig(m_GraphicsConfigDock, &m_Configuration, &m_Internationalization, m_UndoStack);
		m_GraphicsConfigScroll->setWidget(m_GraphicsConfig);
		m_GraphicsConfigDock->setWidget(m_GraphicsConfigScroll);
		addDockWidget(Qt::RightDockWidgetArea, m_GraphicsConfigDock);
		m_WidgetsMenu->addAction(m_GraphicsConfigDock->toggleViewAction());
	}

	// AssetTree (Assets)
	{
		m_AssetTreeDock = new QDockWidget(this);
		m_AssetTreeDock->setAllowedAreas(Qt::AllDockWidgetAreas);
		m_AssetTreeView = new QTreeView(m_AssetTreeDock);
		m_AssetTreeModel = new QDirModel();
		m_AssetTreeView->setModel(m_AssetTreeModel);
		m_AssetTreeView->setSortingEnabled(true);
		m_AssetTreeDock->setWidget(m_AssetTreeView);
		addDockWidget(Qt::LeftDockWidgetArea, m_AssetTreeDock);
		m_WidgetsMenu->addAction(m_AssetTreeDock->toggleViewAction());
	}
}

void CMainWindow::translateDockWindows()
{
	m_CommandLogDock->setWindowTitle(nli18n("WidgetCommandLog"));
	m_GraphicsConfigDock->setWindowTitle(nli18n("WidgetGraphicsConfig"));
	m_AssetTreeDock->setWindowTitle(nli18n("WidgetAssetTree"));
}

void CMainWindow::recalculateMinimumWidth()
{
	if (m_GraphicsConfigScroll) 
		m_GraphicsConfigScroll->setMinimumWidth(m_GraphicsConfig->minimumSizeHint().width() + m_GraphicsConfigScroll->minimumSizeHint().width());
}

void CMainWindow::cfcbQtStyle(NLMISC::CConfigFile::CVar &var)
{
	QApplication::setStyle(QStyleFactory::create(var.asString().c_str()));
	recalculateMinimumWidth();
}

void CMainWindow::cfcbQtPalette(NLMISC::CConfigFile::CVar &var)
{
	if (var.asBool()) QApplication::setPalette(QApplication::style()->standardPalette());
	else QApplication::setPalette(m_OriginalPalette);
}

void CMainWindow::applyGraphicsConfig()
{
	// reinitializes the graphics system completely
	// heavy lifting is done in updateInitialization
	m_IsGraphicsEnabled = false;
	updateInitialization(isVisible());
	m_IsGraphicsEnabled = m_GraphicsConfig->getGraphicsEnabled();
	updateInitialization(isVisible());
}

void CMainWindow::cfcbSoundEnabled(NLMISC::CConfigFile::CVar &var)
{
	// temp, todo as above
	m_IsSoundEnabled = var.asBool(); // update loop inits
}

void CMainWindow::incbLanguageCode()
{
	setWindowTitle(nli18n("WindowTitle"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
	recalculateMinimumWidth();
}

void CMainWindow::about()
{
	QMessageBox::about(this, nli18n("AboutTitle"), nli18n("AboutText"));
}

void CMainWindow::updateRender()
{
	updateInitialization(isVisible());
	
	if (isVisible())
	{

		// call all update functions
		// 01. Update Utilities (configuration etc)
		m_Configuration.updateUtilities();

		// 02. Update Time (deltas)
		// ...

		// 03. Update Receive (network, servertime, receive messages)
		// ...
		
		// 04. Update Input (keyboard controls, etc)
		if (m_IsGraphicsInitialized)
			m_GraphicsViewport->updateInput();
		
		// 05. Update Weather (sky, snow, wind, fog, sun)
		// ...

		// 06. Update Entities (movement, do after possible tp from incoming messages etc)
		//      - Move other entities
		//      - Update self entity
		//      - Move bullets
		// ...

		// 07. Update Landscape (async zone loading near entity)
		// ...
		
		// 08. Update Collisions (entities)
		//      - Update entities
		//      - Update move container (swap with Update entities? todo: check code!)
		//      - Update bullets
		// ...
		
		// 09. Update Animations (playlists)
		//      - Needs to be either before or after entities, not sure, 
		//        there was a problem with wrong order a while ago!!!
		// ...
		
		// 10. Update Camera (depends on entities)
		// ...
		
		// 11. Update Interface (login, ui, etc)
		// ...
		
		// 12. Update Sound (sound driver)
		if (m_IsSoundInitialized)
			m_SoundUtilities.updateSound();
		
		// 13. Update Send (network, send new position etc)
		// ...
		
		// 14. Update Debug (stuff for dev)
		// ...
		
		if (m_IsGraphicsInitialized && !m_GraphicsViewport->getDriver()->isLost())
		{
			// 01. Render Driver (background color)			
			m_GraphicsViewport->renderDriver(); // clear all buffers
				
			// 02. Render Sky (sky scene)
			// ...
			
			// 04. Render Scene (entity scene)
			// ...
			
			// 05. Render Effects (flare)
			// ...
			
			// 06. Render Interface 3D (player names)
			// ...

			// 07. Render Debug 3D
			// ...

			// 08. Render Interface 2D (chatboxes etc, optionally does have 3d)
			// ...
				
			// 09. Render Debug 2D (stuff for dev)
			m_GraphicsViewport->renderDebug2D();

			// swap 3d buffers
			m_GraphicsViewport->getDriver()->swapBuffers();
		}
	}
}

void CMainWindow::printDebug()
{
	nldebug("This is a debug message.");
}

} /* namespace NLQT */

/* end of file */
