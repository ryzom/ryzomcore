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
#include <nel/3d/u_scene.h>
#include <nel/3d/u_camera.h>

// Project includes
#include "modules.h"
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
#include "tune_mrm_dialog.h"
#include "tune_timer_dialog.h"
#include "camera_control.h"
#include "object_viewer_constants.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

using namespace std;
using namespace NLMISC;

namespace NLQT
{

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  _isGraphicsInitialized(false),
	  _isGraphicsEnabled(false),
	  _isSoundInitialized(false),
	  _isSoundEnabled(false), // MTR workaround for sheet id nonsense
	  _GraphicsViewport(NULL),
	  _lastDir("."),
	  _mouseMode(NL3D::U3dMouseListener::edit3d)
{
	menu = NULL;
	nldebug("CMainWindow::CMainWindow:");
	setObjectName("CMainWindow");

	// create NeL viewport
	_GraphicsViewport = new CGraphicsViewport(this);
	setCentralWidget(_GraphicsViewport);

	setDockNestingEnabled(true);

	_GraphicsViewport->init();
	_isGraphicsInitialized = true;

	if (_isSoundEnabled)
	{
		Modules::sound().init();
		_isSoundInitialized = true;
	}

	_undoStack = new QUndoStack(this);
	_SkeletonTreeModel = new CSkeletonTreeModel(this);

	createDialogs();
	createActions();
	createMenus();
	createToolBars();

	setWindowIcon(QIcon(":/images/nel.png"));

	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);

	restoreState(settings->value("QtWindowState").toByteArray());
	restoreGeometry(settings->value("QtWindowGeometry").toByteArray());

	// As a special case, a QTimer with a timeout of 0 will time out as soon as all the events in the window system's event queue have been processed.
	// This can be used to do heavy work while providing a snappy user interface.
	_mainTimer = new QTimer(this);
	connect(_mainTimer, SIGNAL(timeout()), this, SLOT(updateRender()));
	connect(_TuneTimerDialog, SIGNAL(changeInterval(int)), this, SLOT(setInterval(int)));
	_TuneTimerDialog->setInterval(settings->value("TimerInterval", 25).toInt());
	settings->endGroup();

	_statusBarTimer = new QTimer(this);
	connect(_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));

	_statusInfo = new QLabel(this);
	_statusInfo->hide();
	Core::ICore::instance()->mainWindow()->statusBar()->addPermanentWidget(_statusInfo);
}

CMainWindow::~CMainWindow()
{
	nldebug("CMainWindow::~CMainWindow:");

	// save state & geometry of window and widgets
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::OBJECT_VIEWER_SECTION);
	settings->setValue("QtWindowState", saveState());
	settings->setValue("QtWindowGeometry", saveGeometry());
	settings->setValue("TimerInterval", _mainTimer->interval());
	settings->endGroup();
	settings->sync();

	removeMenus();

	delete _AnimationDialog;
	delete _AnimationSetDialog;
	delete _SlotManagerDialog;
	delete _SetupFog;
	delete _TuneMRMDialog;
	delete _TuneTimerDialog;
	delete _ParticleControlDialog;
	delete _ParticleWorkspaceDialog;
	delete _cameraControl;

	if (_isSoundInitialized)
		Modules::sound().releaseGraphics();

	_GraphicsViewport->release();
	delete _GraphicsViewport;
}

void CMainWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	Modules::objView().getDriver()->activate();
	if (_isSoundInitialized)
		Modules::sound().initGraphics();
	_mainTimer->start();
	_statusBarTimer->start(1000);
	_statusInfo->show();
}

void CMainWindow::hideEvent(QHideEvent *hideEvent)
{
	_mainTimer->stop();
	_statusBarTimer->stop();
	_statusInfo->hide();
	if (_isSoundInitialized)
		Modules::sound().releaseGraphics();
	QMainWindow::hideEvent(hideEvent);
}

int CMainWindow::getFrameRate()
{
	return _AnimationDialog->_frameRate;
}

void CMainWindow::open()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL data file"), _lastDir,
							tr("All NeL files (*.shape *.ps *.ig);;"
							   "NeL shape files (*.shape);;"
							   "NeL particle system files (*.ps)"
							   "NeL Instance Group files (*.ig)"));

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

void CMainWindow::reloadTextures()
{
	Modules::objView().reloadTextures();
}

void CMainWindow::setInterval(int value)
{
	_mainTimer->setInterval(value);
}

void CMainWindow::updateStatusBar()
{
	if (_isGraphicsInitialized)
	{
		_statusInfo->setText(QString("%1, Nb tri: %2 , Texture used (Mb): %3 , fps: %4  ").arg(
								 Modules::objView().getDriver()->getVideocardInformation()).arg(
								 _numTri).arg(
								 _texMem, 0,'f',4).arg(
								 _fps, 0,'f',2));
	}
}

void CMainWindow::createActions()
{
	_setBackColorAction = _GraphicsViewport->createSetBackgroundColor(this);
	_setBackColorAction->setText(tr("Set &background color"));
	_setBackColorAction->setIcon(QIcon(Constants::ICON_BGCOLOR));
	_setBackColorAction->setStatusTip(tr("Set background color"));

	_resetSceneAction = new QAction(tr("&Reset scene"), this);
	_resetSceneAction->setStatusTip(tr("Reset current scene"));
	connect(_resetSceneAction, SIGNAL(triggered()), this, SLOT(resetScene()));

	_reloadTexturesAction = new QAction(tr("Reload textures"), this);
	_reloadTexturesAction->setStatusTip(tr("Reload textures"));
	connect(_reloadTexturesAction, SIGNAL(triggered()), this, SLOT(reloadTextures()));

	_saveScreenshotAction = _GraphicsViewport->createSaveScreenshotAction(this);
	_saveScreenshotAction->setText(tr("Save Screenshot"));
	_saveScreenshotAction->setStatusTip(tr("Make a screenshot of the current viewport and save"));
}

void CMainWindow::createMenus()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();

	_openAction = menuManager->action(Core::Constants::OPEN);

	// register actions for view menu
	menuManager->registerAction(_setBackColorAction, "ObjectViewer.View.SetBackgroundColor");
	menuManager->registerAction(_SetupFog->toggleViewAction(), "ObjectViewer.View.SetupFog");
	menuManager->registerAction(_resetSceneAction, "ObjectViewer.View.ResetScene");
	menuManager->registerAction(_reloadTexturesAction, "ObjectViewer.View.ReloadTextures");
	menuManager->registerAction(_saveScreenshotAction, "ObjectViewer.View.SaveScreenshot");

	QMenu *ovMenu = menuManager->menuBar()->addMenu( "Object Viewer" );
	menuManager->registerMenu(ovMenu, "ObjectViewerQt.ObjectViewer");

	ovMenu->addAction(_setBackColorAction);
	ovMenu->addAction(_SetupFog->toggleViewAction());
	ovMenu->addAction(_resetSceneAction);
	ovMenu->addAction(_reloadTexturesAction);
	ovMenu->addAction(_saveScreenshotAction);

	ovMenu->addSeparator();

	ovMenu->addAction(_AnimationDialog->toggleViewAction() );
	_AnimationDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_ANIM));

	ovMenu->addAction(_AnimationSetDialog->toggleViewAction());
	_AnimationSetDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_ANIMSET));

	ovMenu->addAction(_SlotManagerDialog->toggleViewAction());
	_SlotManagerDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_MIXER));

	ovMenu->addAction(_ParticleControlDialog->toggleViewAction());
	_ParticleControlDialog->toggleViewAction()->setIcon(QIcon(":/images/pqrticles.png"));

	ovMenu->addAction(_DayNightDialog->toggleViewAction());
	_DayNightDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_DAYNIGHT));

	ovMenu->addAction(_WaterPoolDialog->toggleViewAction());
	_WaterPoolDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_WATER));
	_WaterPoolDialog->toggleViewAction()->setEnabled(false);

	ovMenu->addAction(_VegetableDialog->toggleViewAction());
	_VegetableDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_VEGET));

	ovMenu->addAction(_GlobalWindDialog->toggleViewAction());
	_GlobalWindDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_WIND));

	ovMenu->addAction(_SkeletonScaleDialog->toggleViewAction());
	_SkeletonScaleDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_SKELSCALE));

	ovMenu->addAction(_TuneTimerDialog->toggleViewAction());
	_TuneTimerDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_FRAMEDELAY));

	ovMenu->addAction(_SunColorDialog->toggleViewAction());

	ovMenu->addAction(_TuneMRMDialog->toggleViewAction());
	_TuneMRMDialog->toggleViewAction()->setIcon(QIcon(Constants::ICON_MRM_MESH));

	connect(_ParticleControlDialog->toggleViewAction(), SIGNAL(triggered(bool)),
			_ParticleWorkspaceDialog, SLOT(setVisible(bool)));

	connect(_ParticleControlDialog->toggleViewAction(), SIGNAL(triggered(bool)),
			_ParticleWorkspaceDialog->_PropertyDialog, SLOT(setVisible(bool)));

	menu = ovMenu;

}

void CMainWindow::removeMenus()
{
	delete menu;
	menu = NULL;
}

void CMainWindow::createToolBars()
{
	_fileToolBar = addToolBar(tr("&File"));
	_fileToolBar->addAction(_openAction);

	//_viewToolBar = addToolBar(tr("&Edit"));

	_toolsBar = addToolBar(tr("&Tools"));

	_toolsBar->addAction(_setBackColorAction);
	_toolsBar->addSeparator();

	_toolsBar->addAction(_AnimationDialog->toggleViewAction());
	_toolsBar->addAction(_AnimationSetDialog->toggleViewAction());
	_toolsBar->addAction(_SlotManagerDialog->toggleViewAction());
	_toolsBar->addAction(_ParticleControlDialog->toggleViewAction());
	_toolsBar->addAction(_DayNightDialog->toggleViewAction());
	_toolsBar->addAction(_WaterPoolDialog->toggleViewAction());
	_toolsBar->addAction(_VegetableDialog->toggleViewAction());
	_toolsBar->addAction(_GlobalWindDialog->toggleViewAction());
	_toolsBar->addAction(_TuneTimerDialog->toggleViewAction());
	_toolsBar->addAction(_SkeletonScaleDialog->toggleViewAction());
	_toolsBar->addAction(_TuneMRMDialog->toggleViewAction());

	_cameraControl = new CCameraControl(this);
	this->addToolBar(_cameraControl->getToolBar());
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
	tabifyDockWidget(_AnimationSetDialog, _SlotManagerDialog);
	_SlotManagerDialog->setVisible(false);

	// create particle control dialog
	_ParticleControlDialog = new CParticleControlDialog(_SkeletonTreeModel ,this);
	tabifyDockWidget(_AnimationDialog, _ParticleControlDialog);
	_ParticleControlDialog->setVisible(false);

	// create particle workspace dialog
	_ParticleWorkspaceDialog = new  CParticleWorkspaceDialog(this);
	tabifyDockWidget(_SlotManagerDialog, _ParticleWorkspaceDialog);
	_ParticleWorkspaceDialog->setVisible(false);

	// create day night (water) dialog
	_DayNightDialog = new CDayNightDialog(this);
	addDockWidget(Qt::TopDockWidgetArea, _DayNightDialog);
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
	tabifyDockWidget(_DayNightDialog, _GlobalWindDialog);
	_GlobalWindDialog->setVisible(false);

	// create sun color dialog
	_SunColorDialog = new CSunColorDialog(this);
	tabifyDockWidget(_SlotManagerDialog, _SunColorDialog);
	_SunColorDialog->setVisible(false);

	// add property editor in widget area
	addDockWidget(Qt::RightDockWidgetArea, _ParticleWorkspaceDialog->_PropertyDialog);
	tabifyDockWidget(_VegetableDialog, _ParticleWorkspaceDialog->_PropertyDialog);
	_ParticleWorkspaceDialog->_PropertyDialog->setVisible(false);

	// create skeleton scale dialog
	_SkeletonScaleDialog = new CSkeletonScaleDialog(_SkeletonTreeModel, this);
	tabifyDockWidget(_VegetableDialog, _SkeletonScaleDialog);
	_SkeletonScaleDialog->setVisible(false);

	// create setup fog dialog
	_SetupFog = new CSetupFog(this);
	tabifyDockWidget(_VegetableDialog,  _SetupFog);
	_SetupFog->setVisible(false);

	// create tune mrm dialog
	_TuneMRMDialog = new CTuneMRMDialog(this);
	tabifyDockWidget(_ParticleControlDialog, _TuneMRMDialog);
	_TuneMRMDialog->setVisible(false);

	_TuneTimerDialog = new CTuneTimerDialog(this);
	tabifyDockWidget(_GlobalWindDialog, _TuneTimerDialog);
	_TuneTimerDialog->setVisible(false);

	connect(_ParticleControlDialog, SIGNAL(changeState()), _ParticleWorkspaceDialog, SLOT(setNewState()));
	connect(_ParticleWorkspaceDialog, SIGNAL(changeActiveNode()), _ParticleControlDialog, SLOT(updateActiveNode()));
	connect(_AnimationSetDialog->ui.setLengthToolButton, SIGNAL(clicked()), _AnimationDialog, SLOT(changeAnimLength()));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SkeletonTreeModel, SLOT(rebuildModel()));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SkeletonScaleDialog, SLOT(setCurrentShape(QString)));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _AnimationDialog, SLOT(setCurrentShape(QString)));
	connect(_AnimationSetDialog, SIGNAL(changeCurrentShape(QString)), _SlotManagerDialog, SLOT(updateUiSlots()));
	connect(_ParticleControlDialog, SIGNAL(changeAutoCount(bool)), _ParticleWorkspaceDialog->getPropertyDialog()->getLocatedPage(), SLOT(setDisabledCountPS(bool)));
}

bool CMainWindow::loadFile(const QString &fileName, const QString &skelName)
{
	QFileInfo fileInfo(fileName);
	bool loaded;
	if (fileInfo.suffix() == "ig")
		loaded = Modules::objView().loadInstanceGroup(fileName.toUtf8().constData());
	else
		loaded = Modules::objView().loadMesh(fileName.toUtf8().constData(), skelName.toUtf8().constData());

	if (!loaded)
	{
		//statusBar()->showMessage(tr("Loading canceled"),2000);
		return false;
	}
	//statusBar()->showMessage(tr("File loaded"),2000);
	return true;
}

void CMainWindow::updateRender()
{
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

		// 15. Calc FPS
		static sint64 lastTime = NLMISC::CTime::getPerformanceTime ();
		sint64 newTime = NLMISC::CTime::getPerformanceTime ();
		_fps = float(1.0 / NLMISC::CTime::ticksToSecond (newTime-lastTime));
		lastTime = newTime;

		if (_isGraphicsInitialized && !Modules::objView().getDriver()->isLost())
		{
			// 01. Render Driver (background color)
			//Modules::objView().getDriver()->activate();
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

			// 10. Get profile information
			NL3D::CPrimitiveProfile in, out;
			Modules::objView().getDriver()->profileRenderedPrimitives (in, out);

			_numTri = in.NLines+in.NPoints+in.NQuads*2+in.NTriangles+in.NTriangleStrips;
			_texMem = float(Modules::objView().getDriver()->getUsedTextureMemory() / float(1024*1024));

			// swap 3d buffers
			Modules::objView().getDriver()->swapBuffers();
		}
	}
}

} /* namespace NLQT */

/* end of file */
