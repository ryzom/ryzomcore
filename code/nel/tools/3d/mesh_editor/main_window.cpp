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
#include <QtGui>
#include <QTreeView>
#include <QDirModel>
#include <QUndoStack>
#include <QScrollArea>
#include <QAction>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QMessageBox>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/3d/u_driver.h>
#include <nel/misc/path.h>
#include <nel/pipeline/project_config.h>

// Project includes
#include "../shared_widgets/command_log.h"
#include "../shared_widgets/error_list.h"
#include "graphics_viewport.h"
#include "graphics_config.h"
#include "texture_browser.h"

using namespace std;
using namespace NLMISC;
using namespace NLQT;

CMainWindow::CMainWindow(QWidget *parent, Qt::WindowFlags flags)
	: QMainWindow(parent, flags),
	m_UndoStack(NULL), 
	m_IsGraphicsInitialized(false), m_IsGraphicsEnabled(false), 
	m_IsSoundInitialized(false), m_IsSoundEnabled(false), 
	m_Timer(NULL), m_GraphicsViewport(NULL),
	m_CommandLog(NULL), m_CommandLogDock(NULL), 
	m_ErrorList(NULL), m_ErrorListDock(NULL), 
	m_GraphicsConfig(NULL), m_GraphicsConfigScroll(NULL), m_GraphicsConfigDock(NULL), 
	m_FileMenu(NULL), m_EditMenu(NULL), m_ViewportMenu(NULL), m_WidgetsMenu(NULL), m_HelpMenu(NULL), 
	m_FileToolBar(NULL), m_EditToolBar(NULL),
	m_AboutAct(NULL), m_QuitAct(NULL), m_PrintDebugAct(NULL), 
	m_UndoAct(NULL), m_RedoAct(NULL), m_SaveScreenshotAct(NULL),
	m_IsExiting(false)
{
	setObjectName("CMainWindow");
	setWindowTitle(tr("NeL Mesh Editor"));
	
	m_UndoStack = new QUndoStack(this);

	m_Configuration.init();

	m_GraphicsViewport = new CGraphicsViewport(this);
    setCentralWidget(m_GraphicsViewport);
	
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();

	m_Configuration.setAndCallback("LanguageCode", CConfigCallback(this, &CMainWindow::cfcbLanguageCode));

	recalculateMinimumWidth();
	
	m_Timer = new QTimer(this);
	connect(m_Timer, SIGNAL(timeout()), this, SLOT(updateRender()));
	m_Timer->start(40); // 25fps
	
	m_IsGraphicsEnabled = m_GraphicsConfig->getGraphicsEnabled();
	connect(m_GraphicsConfig, SIGNAL(applyGraphicsConfig()), this, SLOT(applyGraphicsConfig()));
	m_Configuration.setAndCallback("SoundEnabled", CConfigCallback(this, &CMainWindow::cfcbSoundEnabled));

	NLMISC::CConfigFile::CVar *lastFiles = m_Configuration.getConfigFile().getVarPtr("LastFiles");
	if (lastFiles)
	{
		for (uint i = 0; i < lastFiles->size(); ++i)
		{
			if (NLMISC::CFile::isExists(lastFiles->asString()))
			{
				initProjectConfig(lastFiles->asString());
				break;
			}
		}
	}

	QDockWidget *dock = new QDockWidget(this);
	dock->setFloating(true);
	CTextureBrowser *browser = new CTextureBrowser(dock);
	dock->setWidget(browser);
	dock->resize(800, 800);
}

CMainWindow::~CMainWindow()
{
	m_Timer->stop();
	updateInitialization(false);

	delete m_GraphicsConfig; m_GraphicsConfig = NULL;

	m_Configuration.dropCallback("SoundEnabled");
	m_Configuration.dropCallback("LanguageCode");

	m_Configuration.release();
}

void CMainWindow::initProjectConfig(const std::string &asset)
{
	NLPIPELINE::CProjectConfig::init(asset,
		NLPIPELINE::CProjectConfig::DatabaseTextureSearchPaths,
		true);

	std::string databaseRoot = NLPIPELINE::CProjectConfig::assetRoot();
	m_AssetTreeView->setRootIndex(m_AssetTreeModel->index(QString::fromUtf8(databaseRoot.c_str())));
}

void CMainWindow::closeEvent(QCloseEvent *e)
{
	m_Timer->stop();
	updateInitialization(false);

	QMainWindow::closeEvent(e);
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
				
				m_SoundUtilities.init(&m_Configuration);
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
	m_QuitAct->setText(tr("&Exit"));
	m_QuitAct->setStatusTip(tr("Close the NeL Qt application."));
	m_AboutAct->setText(tr("&About"));
	m_AboutAct->setStatusTip(tr("Show the about box."));
	m_PrintDebugAct->setText(tr("Print Debug"));
	m_PrintDebugAct->setStatusTip(tr("Print something to debug."));
	m_UndoAct->setText(tr("&Undo"));
	m_UndoAct->setStatusTip(tr("Undo the last action."));
	m_RedoAct->setText(tr("&Redo"));
	m_RedoAct->setStatusTip(tr("Redo the last undone action."));
	m_SaveScreenshotAct->setText(tr("Save &Screenshot"));
	m_SaveScreenshotAct->setStatusTip(tr("Make a screenshot of the current viewport and save it to the default screenshot directory."));
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
	m_FileMenu->setTitle(tr("&File"));
	m_EditMenu->setTitle(tr("&Edit"));
	m_ViewportMenu->setTitle(tr("&Viewport"));
	m_WidgetsMenu->setTitle(tr("&Widgets"));
	m_HelpMenu->setTitle(tr("&Help"));
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
	m_FileToolBar->setWindowTitle(tr("File"));
	m_EditToolBar->setWindowTitle(tr("Edit"));
}

void CMainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
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
		m_CommandLog = new CCommandLogDisplayer(m_CommandLogDock);
		m_CommandLogDock->setWidget(m_CommandLog);
		addDockWidget(Qt::BottomDockWidgetArea, m_CommandLogDock);
		m_WidgetsMenu->addAction(m_CommandLogDock->toggleViewAction());
	}

	// ErrorList (Error List)
	{
		m_ErrorListDock = new QDockWidget(this);
		m_ErrorListDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		m_ErrorList = new CErrorList(m_ErrorListDock);
		m_ErrorListDock->setWidget(m_ErrorList);
		addDockWidget(Qt::BottomDockWidgetArea, m_ErrorListDock);
		m_WidgetsMenu->addAction(m_ErrorListDock->toggleViewAction());
		tabifyDockWidget(m_CommandLogDock, m_ErrorListDock);
	}

	// GraphicsConfig (Graphics Configuration)
	{
		m_GraphicsConfigDock = new QDockWidget(this);
		m_GraphicsConfigDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		m_GraphicsConfigScroll = new QScrollArea();
		m_GraphicsConfigScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		m_GraphicsConfigScroll->setWidgetResizable(true);
		m_GraphicsConfig = new CGraphicsConfig(m_GraphicsConfigDock, &m_Configuration, m_UndoStack);
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
		/*
		QStringList filters;
		filters.push_back("*.nelmat");
		filters.push_back("*.dae");
		filters.push_back("*.3ds");
		filters.push_back("*.fbx");
		filters.push_back("*.blend");
		m_AssetTreeModel->setNameFilters(filters);
		*/
		m_AssetTreeView->setModel(m_AssetTreeModel);
		m_AssetTreeView->setSortingEnabled(true);
		m_AssetTreeDock->setWidget(m_AssetTreeView);
		addDockWidget(Qt::LeftDockWidgetArea, m_AssetTreeDock);
		m_WidgetsMenu->addAction(m_AssetTreeDock->toggleViewAction());
	}
}

void CMainWindow::translateDockWindows()
{
	m_CommandLogDock->setWindowTitle(tr("Console"));
	m_ErrorListDock->setWindowTitle(tr("Error List"));
	m_GraphicsConfigDock->setWindowTitle(tr("Graphics Configuration"));
	m_AssetTreeDock->setWindowTitle(tr("Asset Database"));
}

void CMainWindow::recalculateMinimumWidth()
{
	if (m_GraphicsConfigScroll) 
		m_GraphicsConfigScroll->setMinimumWidth(m_GraphicsConfig->minimumSizeHint().width() + m_GraphicsConfigScroll->minimumSizeHint().width());
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

void CMainWindow::cfcbLanguageCode(NLMISC::CConfigFile::CVar &var)
{
	setWindowTitle(tr("NeL Mesh Editor"));
	translateActions();
	translateMenus();
	translateToolBars();
	translateDockWindows();
	recalculateMinimumWidth();
}

void CMainWindow::about()
{
	QMessageBox::about(this, tr("NeL Mesh Editor"), tr("NeL Mesh Editor"));
}

void CMainWindow::updateRender()
{
	bool visible = isVisible();
	updateInitialization(visible);
	
	if (visible)
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

/* end of file */
