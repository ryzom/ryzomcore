/*
    Object Viewer Qt
    Copyright (C) 2010 Dzmitry Kamiahin <dnk-88@tut.by>

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

#include "stdpch.h"
#include "main_window.h"

// STL includes

// Qt includes
#include <QtGui/QtGui>

// NeL includes
#include <nel/3d/u_driver.h>

// Project includes
#include "modules.h"
#include "settings_dialog.h"
#include "graphics_viewport.h"
#include "animation_dialog.h"
#include "animation_set_dialog.h"
#include "particle_control_dialog.h"
#include "particle_workspace_dialog.h"
#include "slot_manager_dialog.h"
#include "setup_fog_dialog.h"
#include "skeleton_scale_dialog.h"
#include "skeleton_tree_model.h"
#include "water_pool_dialog.h"
#include "vegetable_dialog.h"
#include "global_wind_dialog.h"
#include "day_night_dialog.h"
#include "sun_color_dialog.h"

#include "extension_system/plugin_view.h"

using namespace std;
using namespace NLMISC;

namespace NLQT
{

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  _isGraphicsInitialized(false), _isGraphicsEnabled(false),
	  _isSoundInitialized(false), _isSoundEnabled(false),
	  _isLandscapeInitialized(false), _isLandscapeEnabled(false),
	  _GraphicsViewport(NULL), _lastDir(".")
{
	nldebug("CMainWindow::CMainWindow:");
	setObjectName("CMainWindow");

	// create NeL viewport
	_GraphicsViewport = new CGraphicsViewport(this);
	setCentralWidget(_GraphicsViewport);

	setDockNestingEnabled(true);

	// setup Qt style and palette from config file
	_originalPalette = QApplication::palette();
	Modules::config().setAndCallback("QtStyle", CConfigCallback(this, &CMainWindow::cfcbQtStyle));
	Modules::config().setAndCallback("QtPalette", CConfigCallback(this, &CMainWindow::cfcbQtPalette));

	_SkeletonTreeModel = new CSkeletonTreeModel(this);

	createDialogs();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();

	setWindowIcon(QIcon(":/images/nel.png"));

	QSettings settings("object_viewer_qt.ini", QSettings::IniFormat);
	settings.beginGroup("WindowSettings");
	restoreState(settings.value("QtWindowState").toByteArray());
	restoreGeometry(settings.value("QtWindowGeometry").toByteArray());
	settings.endGroup();

	_isGraphicsEnabled = true;
	_isLandscapeEnabled = true;

	// As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the window system's event queue have been processed.
	// This can be used to do heavy work while providing a snappy user interface.
	_mainTimer = new QTimer(this);
	connect(_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
	// timer->start(); // <- timeout 0
	// it's heavy on cpu, though, when no 3d driver initialized :)
	_mainTimer->start(25); // 25fps

	_statusBarTimer = new QTimer(this);
	connect(_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
	_statusBarTimer->start(5000);

	Modules::config().setAndCallback("SoundEnabled", CConfigCallback(this, &CMainWindow::cfcbSoundEnabled));
}

CMainWindow::~CMainWindow()
{
	nldebug("CMainWindow::~CMainWindow:");

	// save state & geometry of window and widgets
	QSettings settings("object_viewer_qt.ini", QSettings::IniFormat);
	settings.beginGroup("WindowSettings");
	settings.setValue("QtWindowState", saveState());
	settings.setValue("QtWindowGeometry", saveGeometry());
	settings.endGroup();

	Modules::config().dropCallback("SoundEnabled");
	Modules::config().dropCallback("QtPalette");
	Modules::config().dropCallback("QtStyle");

	_mainTimer->stop();
	_statusBarTimer->stop();

	delete _AnimationDialog;
	delete _AnimationSetDialog;
	delete _SlotManagerDialog;
	delete _SetupFog;
	delete _ParticleControlDialog;
	delete _ParticleWorkspaceDialog;
	delete _GraphicsViewport;
	delete _PluginView;
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

int CMainWindow::getFrameRate()
{
	return _AnimationDialog->_frameRate;
}

void CMainWindow::open()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL data file"), _lastDir,
							tr("All NeL files (*.shape *.ps);;"
							   "NeL shape files (*.shape);;"
							   "NeL particle system files (*.ps)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		_lastDir = QFileInfo(list.front()).absolutePath();

		QString skelFileName = QFileDialog::getOpenFileName(this,
							   tr("Open skeleton file"), _lastDir,
							   tr("NeL skeleton file (*.skel)"));

		Q_FOREACH(QString fileName, list)
		loadFile(fileName, skelFileName);

		_AnimationSetDialog->updateListObject();
		_AnimationSetDialog->updateListAnim();
		_SlotManagerDialog->updateUiSlots();
	}
	setCursor(Qt::ArrowCursor);
}

void CMainWindow::resetScene()
{
	Modules::objView().resetScene();
	_AnimationSetDialog->updateListObject();
	_AnimationSetDialog->updateListAnim();
	_SlotManagerDialog->updateUiSlots();
	_SkeletonTreeModel->resetTreeModel();
}

void CMainWindow::settings()
{
	CSettingsDialog _settingsDialog(this);
	_settingsDialog.show();
	_settingsDialog.exec();
}

void CMainWindow::about()
{
	QMessageBox::about(this, tr("About Object Viewer Qt"),
					   tr("<h2>Object Viewer Qt  8-)</h2>"
						  "<p> Authors: dnk-88, sfb, Kaetemi, kervala <p>Compiled on %1 %2").arg(__DATE__).arg(__TIME__));
}

void CMainWindow::updateStatusBar()
{
	if (_isGraphicsInitialized)
		statusBar()->showMessage(QString(Modules::objView().getDriver()->getVideocardInformation()));
}

void CMainWindow::updateInitialization(bool visible)
{
	bool done;
	do
	{
		done = true; // set false whenever change
		bool wantSound = _isSoundEnabled && visible;
		bool wantGraphics = _isGraphicsEnabled && visible;
		// TODO WARNING:  made Landscape stuff
		bool wantLandscape = wantGraphics && _isGraphicsInitialized && _isLandscapeEnabled;

		// .. stuff that depends on other stuff goes on top to prioritize deinitialization

		// Landscape
		if (_isLandscapeInitialized)
		{
			if (!wantLandscape)
			{
				_isLandscapeInitialized = false;
//				if (_isGraphicsInitialized)
//					Modules::veget().releaseGraphics();
				Modules::veget().release();
				done = false;
			}
		}
		else
		{
			if (wantLandscape)
			{
				Modules::veget().init();
//				if (_isGraphicsInitialized)
//					Modules::veget().initGraphics();
				_isLandscapeInitialized = true;
				done = false;
			}
		}

		// Graphics (Driver)
		if (_isGraphicsInitialized)
		{
			if (!wantGraphics)
			{
				_isGraphicsInitialized = false;
				if (_isSoundInitialized)
					Modules::sound().releaseGraphics();
				_GraphicsViewport->release();
				done = false;
			}
		}
		else
		{
			if (wantGraphics)
			{
				_GraphicsViewport->init();
				if (_isSoundInitialized)
					Modules::sound().initGraphics();
				_isGraphicsInitialized = true;
				done = false;
			}
		}

		// Sound (AudioMixer)
		if (_isSoundInitialized)
		{
			if (!wantSound)
			{
				_isSoundInitialized = false;
				if (_isGraphicsInitialized)
					Modules::sound().releaseGraphics();
				Modules::sound().release();
				done = false;
			}
		}
		else
		{
			if (wantSound)
			{
				Modules::sound().init();
				if (_isGraphicsInitialized)
					Modules::sound().initGraphics();
				_isSoundInitialized = true;
				done = false;
			}
		}

	}
	while (!done);
}

void CMainWindow::createActions()
{
	_openAction = new QAction(tr("&Open..."), this);
	_openAction->setIcon(QIcon(":/images/open-file.png"));
	_openAction->setShortcut(QKeySequence::Open);
	_openAction->setStatusTip(tr("Open an existing file"));
	connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

	_exitAction = new QAction(tr("E&xit"), this);
	_exitAction->setShortcut(tr("Ctrl+Q"));
	_exitAction->setStatusTip(tr("Exit the application"));
	connect(_exitAction, SIGNAL(triggered()), this, SLOT(close()));

	_setBackColorAction = _GraphicsViewport->createSetBackgroundColor(this);
	_setBackColorAction->setText(tr("Set &background color"));
	_setBackColorAction->setIcon(QIcon(":/images/ico_bgcolor.png"));
	_setBackColorAction->setStatusTip(tr("Set background color"));

	_resetSceneAction = new QAction(tr("&Reset scene"), this);
	_resetSceneAction->setStatusTip(tr("Reset current scene"));
	connect(_resetSceneAction, SIGNAL(triggered()), this, SLOT(resetScene()));

	_saveScreenshotAction = _GraphicsViewport->createSaveScreenshotAction(this);
	_saveScreenshotAction->setText(tr("Save &Screenshot"));
	_saveScreenshotAction->setStatusTip(tr("Make a screenshot of the current viewport and save"));

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

	_pluginViewAction = new QAction(tr("About &Plugins"), this);
	_pluginViewAction->setStatusTip(tr("Show the plugin view dialog"));
	connect(_pluginViewAction, SIGNAL(triggered()), _PluginView, SLOT(show()));
}

void CMainWindow::createMenus()
{
	_fileMenu = menuBar()->addMenu(tr("&File"));
	_fileMenu->setObjectName("ovqt.Menu.File");
	_fileMenu->addAction(_openAction);
	_fileMenu->addSeparator();
	_fileMenu->addAction(_exitAction);

	_viewMenu = menuBar()->addMenu(tr("&View"));
	_viewMenu->setObjectName("ovqt.Menu.View");
	_viewMenu->addAction(_setBackColorAction);
	_viewMenu->addAction(_SetupFog->toggleViewAction());

	_sceneMenu = menuBar()->addMenu(tr("&Scene"));
	_sceneMenu->setObjectName("ovqt.Menu.Scene");
	_sceneMenu->addAction(_resetSceneAction);
	_sceneMenu->addAction(_saveScreenshotAction);

	_toolsMenu = menuBar()->addMenu(tr("&Tools"));
	_toolsMenu->setObjectName("ovqt.Menu.Tools");

	_toolsMenu->addAction(_AnimationDialog->toggleViewAction());
	_AnimationDialog->toggleViewAction()->setIcon(QIcon(":/images/anim.png"));

	_toolsMenu->addAction(_AnimationSetDialog->toggleViewAction());
	_AnimationSetDialog->toggleViewAction()->setIcon(QIcon(":/images/animset.png"));

	_toolsMenu->addAction(_SlotManagerDialog->toggleViewAction());
	_SlotManagerDialog->toggleViewAction()->setIcon(QIcon(":/images/mixer.png"));

	_toolsMenu->addAction(_ParticleControlDialog->toggleViewAction());
	_ParticleControlDialog->toggleViewAction()->setIcon(QIcon(":/images/pqrticles.png"));

	_toolsMenu->addAction(_DayNightDialog->toggleViewAction());
	_DayNightDialog->toggleViewAction()->setIcon(QIcon(":/images/dqynight.png"));

	_toolsMenu->addAction(_WaterPoolDialog->toggleViewAction());
	_WaterPoolDialog->toggleViewAction()->setIcon(QIcon(":/images/water.png"));
	_WaterPoolDialog->toggleViewAction()->setEnabled(false);

	_toolsMenu->addAction(_VegetableDialog->toggleViewAction());
	_VegetableDialog->toggleViewAction()->setIcon(QIcon(":/images/veget.png"));

	_toolsMenu->addAction(_GlobalWindDialog->toggleViewAction());
	_GlobalWindDialog->toggleViewAction()->setIcon(QIcon(":/images/wind.png"));

	_toolsMenu->addAction(_SkeletonScaleDialog->toggleViewAction());
	_SkeletonScaleDialog->toggleViewAction()->setIcon(QIcon(":/images/ico_skelscale.png"));

	_toolsMenu->addAction(_SunColorDialog->toggleViewAction());

	connect(_ParticleControlDialog->toggleViewAction(), SIGNAL(triggered(bool)),
			_ParticleWorkspaceDialog, SLOT(setVisible(bool)));

	connect(_ParticleControlDialog->toggleViewAction(), SIGNAL(triggered(bool)),
			_ParticleWorkspaceDialog->_PropertyDialog, SLOT(setVisible(bool)));

	_toolsMenu->addSeparator();

	_toolsMenu->addAction(_settingsAction);

	menuBar()->addSeparator();

	_helpMenu = menuBar()->addMenu(tr("&Help"));
	_helpMenu->setObjectName("ovqt.Menu.Help");
	_helpMenu->addAction(_aboutAction);
	_helpMenu->addAction(_aboutQtAction);
	_helpMenu->addAction(_pluginViewAction);

	Modules::plugMan().addObject(_fileMenu);
	Modules::plugMan().addObject(_viewMenu);
	Modules::plugMan().addObject(_sceneMenu);
	Modules::plugMan().addObject(_toolsMenu);
	Modules::plugMan().addObject(_helpMenu);
}

void CMainWindow::createToolBars()
{
	_fileToolBar = addToolBar(tr("&File"));
	_fileToolBar->addAction(_openAction);

	//_editToolBar = addToolBar(tr("&Edit"));
	//_editToolBar->addSeparator();
	_toolsBar = addToolBar(tr("&Tools"));
	_toolsBar->addAction(_AnimationDialog->toggleViewAction());
	_toolsBar->addAction(_AnimationSetDialog->toggleViewAction());
	_toolsBar->addAction(_SlotManagerDialog->toggleViewAction());
	_toolsBar->addAction(_ParticleControlDialog->toggleViewAction());
	_toolsBar->addAction(_DayNightDialog->toggleViewAction());
	_toolsBar->addAction(_WaterPoolDialog->toggleViewAction());
	_toolsBar->addAction(_VegetableDialog->toggleViewAction());
	_toolsBar->addAction(_GlobalWindDialog->toggleViewAction());
	_toolsBar->addAction(_SkeletonScaleDialog->toggleViewAction());
}

void CMainWindow::createStatusBar()
{
	statusBar()->showMessage(tr("StatusReady"));
}

void CMainWindow::createDialogs()
{
	// create animation dialog
	_AnimationDialog = new CAnimationDialog(this);
	addDockWidget(Qt::BottomDockWidgetArea, _AnimationDialog);
	_AnimationDialog->setVisible(false);

	// create animation set manager dialog
	_AnimationSetDialog = new CAnimationSetDialog(this);
	addDockWidget(Qt::LeftDockWidgetArea, _AnimationSetDialog);
	_AnimationSetDialog->setVisible(false);

	// create animation slot manager dialog
	_SlotManagerDialog = new CSlotManagerDialog(this);
	addDockWidget(Qt::RightDockWidgetArea, _SlotManagerDialog);
	_SlotManagerDialog->setVisible(false);

	// create particle control dialog
	_ParticleControlDialog = new CParticleControlDialog(_SkeletonTreeModel ,this);
	addDockWidget(Qt::BottomDockWidgetArea, _ParticleControlDialog);
	_ParticleControlDialog->setVisible(false);

	// create particle workspace dialog
	_ParticleWorkspaceDialog = new  CParticleWorkspaceDialog(this);
	addDockWidget(Qt::LeftDockWidgetArea, _ParticleWorkspaceDialog);
	_ParticleWorkspaceDialog->setVisible(false);

	// create day night (water) dialog
	_DayNightDialog = new CDayNightDialog(this);
	addDockWidget(Qt::BottomDockWidgetArea, _DayNightDialog);
	_DayNightDialog->setVisible(false);

	// create water pool editor dialog
	_WaterPoolDialog =  new CWaterPoolDialog(this);
	addDockWidget(Qt::RightDockWidgetArea, _WaterPoolDialog);
	_WaterPoolDialog->setVisible(false);

	// create vegetable editor dialog
	_VegetableDialog = new CVegetableDialog(this);
	addDockWidget(Qt::RightDockWidgetArea, _VegetableDialog);
	_VegetableDialog->setVisible(false);

	// create global wind power/direction dialog
	_GlobalWindDialog = new CGlobalWindDialog(this);
	addDockWidget(Qt::TopDockWidgetArea, _GlobalWindDialog);
	_GlobalWindDialog->setVisible(false);

	// create sun color dialog
	_SunColorDialog = new CSunColorDialog(this);
	addDockWidget(Qt::LeftDockWidgetArea, _SunColorDialog);
	_SunColorDialog->setVisible(false);

	// add property editor in widget area
	addDockWidget(Qt::RightDockWidgetArea, _ParticleWorkspaceDialog->_PropertyDialog);
	_ParticleWorkspaceDialog->_PropertyDialog->setVisible(false);

	// create skeleton scale dialog
	_SkeletonScaleDialog = new CSkeletonScaleDialog(_SkeletonTreeModel, this);
	addDockWidget(Qt::RightDockWidgetArea, _SkeletonScaleDialog);
	_SkeletonScaleDialog->setVisible(false);

	// create setup fog dialog
	_SetupFog = new CSetupFog(this);
	addDockWidget(Qt::RightDockWidgetArea, _SetupFog);
	_SetupFog->setVisible(false);

	_PluginView = new CPluginView(&Modules::plugMan(), this);

	connect(_ParticleControlDialog, SIGNAL(changeState()), _ParticleWorkspaceDialog, SLOT(setNewState()));
	connect(_ParticleWorkspaceDialog, SIGNAL(changeActiveNode()), _ParticleControlDialog, SLOT(updateActiveNode()));
	connect(_AnimationSetDialog->ui.setLengthPushButton, SIGNAL(clicked()), _AnimationDialog, SLOT(changeAnimLength()));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SkeletonTreeModel, SLOT(rebuildModel()));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SkeletonScaleDialog, SLOT(setCurrentShape(QString)));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _AnimationDialog, SLOT(setCurrentShape(QString)));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SlotManagerDialog, SLOT(updateUiSlots()));
	connect(_ParticleControlDialog, SIGNAL(changeAutoCount(bool)), _ParticleWorkspaceDialog->getPropertyDialog()->getLocatedPage(), SLOT(setDisabledCountPS(bool)));
}

bool CMainWindow::loadFile(const QString &fileName, const QString &skelName)
{
	if (!Modules::objView().loadMesh(fileName.toStdString(), skelName.toStdString()))
	{
		statusBar()->showMessage(tr("Loading canceled"),2000);
		return false;
	}
	statusBar()->showMessage(tr("File loaded"),2000);
	return true;
}

void CMainWindow::cfcbQtStyle(NLMISC::CConfigFile::CVar &var)
{
	QApplication::setStyle(QStyleFactory::create(var.asString().c_str()));
}

void CMainWindow::cfcbQtPalette(NLMISC::CConfigFile::CVar &var)
{
	if (var.asBool()) QApplication::setPalette(QApplication::style()->standardPalette());
	else QApplication::setPalette(_originalPalette);
}

void CMainWindow::cfcbSoundEnabled(NLMISC::CConfigFile::CVar &var)
{
	_isSoundEnabled = var.asBool(); // update loop inits
}

void CMainWindow::updateRender()
{
	updateInitialization(isVisible());

	if (isVisible())
	{

		// call all update functions
		// 01. Update Utilities (configuration etc)

		// 02. Update Time (deltas)
		// ...

		// 03. Update Receive (network, servertime, receive messages)
		// ...

		// 04. Update Input (keyboard controls, etc)
		if (_isGraphicsInitialized)
			Modules::objView().updateInput();

		// 05. Update Weather (sky, snow, wind, fog, sun)
		// ...

		// 06. Update Entities (movement, do after possible tp from incoming messages etc)
		//      - Move other entities
		//      - Update self entity
		//      - Move bullets
		// ...

		// 07. Update Landscape (async zone loading near entity)


		// 08. Update Collisions (entities)
		//      - Update entities
		//      - Update move container (swap with Update entities? todo: check code!)
		//      - Update bullets
		Modules::veget().update();

		// 09. Update Animations (playlists)
		//      - Needs to be either before or after entities, not sure,
		//        there was a problem with wrong order a while ago!!!
		Modules::objView().updateAnimatePS();

		Modules::objView().updateAnimation(_AnimationDialog->getTime());

		// 09.2 Update Particle system editor
		Modules::psEdit().update();

		// 10. Update Camera (depends on entities)
		// ...

		// 11. Update Interface (login, ui, etc)
		// ...

		// 12. Update Sound (sound driver)
		if (_isSoundInitialized)
		{
			Modules::sound().setListenerMatrix(Modules::objView().get3dMouseListener()->getViewMatrix());
			Modules::sound().update();
		}
		// 13. Update Send (network, send new position etc)
		// ...

		// 14. Update Debug (stuff for dev)
		// ...

		if (_isGraphicsInitialized && !Modules::objView().getDriver()->isLost())
		{
			// 01. Render Driver (background color)
			Modules::objView().renderDriver(); // clear all buffers

			// 02. Render Sky (sky scene)
			// ...

			// 04. Render Scene (entity scene)
			Modules::objView().renderScene();

			// 05. Render Effects (flare)
			// ...

			// 06. Render Interface 3D (player names)
			// ...

			// 07. Render Debug 3D
			// ...

			// 08. Render Interface 2D (chatboxes etc, optionally does have 3d)
			// ...

			// 09. Render Debug 2D (stuff for dev)
			Modules::objView().renderDebug2D();

			// swap 3d buffers
			Modules::objView().getDriver()->swapBuffers();
		}
	}
}


} /* namespace NLQT */

/* end of file */
