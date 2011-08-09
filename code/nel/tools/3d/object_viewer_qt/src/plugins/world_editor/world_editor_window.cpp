// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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
#include "world_editor_window.h"
#include "world_editor_constants.h"
#include "primitives_model.h"
#include "world_editor_scene.h"
#include "world_editor_misc.h"
#include "world_editor_actions.h"
#include "world_editor_scene_item.h"
#include "project_settings_dialog.h"

// Core
#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

// Lanscape Editor plugin
#include "../landscape_editor/builder_zone_base.h"

// NeL includes

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>

namespace WorldEditor
{

WorldEditorWindow::WorldEditorWindow(QWidget *parent)
	: QMainWindow(parent),
	  m_primitivesModel(0),
	  m_undoStack(0)
{
	m_ui.setupUi(this);
	m_undoStack = new QUndoStack(this);

	m_worldEditorScene = new WorldEditorScene(NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->CellSize, this);
	m_zoneBuilderBase = new LandscapeEditor::ZoneBuilderBase(m_worldEditorScene);

	m_worldEditorScene->setZoneBuilder(m_zoneBuilderBase);
	m_ui.graphicsView->setScene(m_worldEditorScene);
	m_ui.graphicsView->setVisibleText(false);

	QActionGroup *sceneModeGroup = new QActionGroup(this);
	sceneModeGroup->addAction(m_ui.selectAction);
	sceneModeGroup->addAction(m_ui.moveAction);
	sceneModeGroup->addAction(m_ui.rotateAction);
	sceneModeGroup->addAction(m_ui.scaleAction);
	sceneModeGroup->addAction(m_ui.turnAction);
	sceneModeGroup->addAction(m_ui.radiusAction);
	m_ui.selectAction->setChecked(true);

	m_ui.newWorldEditAction->setIcon(QIcon(Core::Constants::ICON_NEW));
	m_ui.saveWorldEditAction->setIcon(QIcon(Core::Constants::ICON_SAVE));

	m_primitivesModel = new PrimitivesTreeModel(this);
	m_ui.treePrimitivesView->setModel(m_primitivesModel);

	// TODO: ?
	m_ui.treePrimitivesView->setUndoStack(m_undoStack);
	m_ui.treePrimitivesView->setZoneBuilder(m_zoneBuilderBase);
	m_ui.treePrimitivesView->setWorldScene(m_worldEditorScene);

	createMenus();
	createToolBars();
	readSettings();

	QSignalMapper *m_modeMapper = new QSignalMapper(this);
	connect(m_ui.selectAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.selectAction, 0);
	connect(m_ui.moveAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.moveAction, 1);
	connect(m_ui.rotateAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.rotateAction, 2);
	connect(m_ui.scaleAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.scaleAction, 3);
	connect(m_ui.turnAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.turnAction, 4);
	connect(m_ui.radiusAction, SIGNAL(triggered()), m_modeMapper, SLOT(map()));
	m_modeMapper->setMapping(m_ui.radiusAction, 5);

	connect(m_modeMapper, SIGNAL(mapped(int)), this, SLOT(setMode(int)));
	connect(m_ui.pointsAction, SIGNAL(triggered(bool)), m_worldEditorScene, SLOT(setEnabledEditPoint(bool)));

	connect(m_ui.settingsAction, SIGNAL(triggered()), this, SLOT(openProjectSettings()));
	connect(m_ui.newWorldEditAction, SIGNAL(triggered()), this, SLOT(newWorldEditFile()));
	connect(m_ui.saveWorldEditAction, SIGNAL(triggered()), this, SLOT(saveWorldEditFile()));
}

WorldEditorWindow::~WorldEditorWindow()
{
	writeSettings();

	delete m_zoneBuilderBase;
}

QUndoStack *WorldEditorWindow::undoStack() const
{
	return m_undoStack;
}

void WorldEditorWindow::open()
{
	QString fileName = QFileDialog::getOpenFileName(this,
					   tr("Open NeL World Edit file"), m_lastDir,
					   tr("All NeL World Editor file (*.worldedit)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		m_lastDir = QFileInfo(fileName).absolutePath();
		loadWorldEditFile(fileName);
	}
	setCursor(Qt::ArrowCursor);
}

void WorldEditorWindow::loadWorldEditFile(const QString &fileName)
{
	Utils::WorldEditList worldEditList;
	if (!Utils::loadWorldEditFile(fileName.toStdString(), worldEditList))
	{
		// TODO: add the message box
		return;
	}

	m_undoStack->beginMacro(QString("Load %1").arg(fileName));

	checkCurrentWorld();

	m_undoStack->push(new CreateWorldCommand(fileName, m_primitivesModel));
	for (size_t i = 0; i < worldEditList.size(); ++i)
	{
		switch (worldEditList[i].first)
		{
		case Utils::DataDirectoryType:
			m_zoneBuilderBase->init(QString(worldEditList[i].second.c_str()), true);
			break;
		case Utils::ContextType:
			break;
		case Utils::LandscapeType:
			m_undoStack->push(new LoadLandscapeCommand(QString(worldEditList[i].second.c_str()), m_primitivesModel, m_zoneBuilderBase));
			break;
		case Utils::PrimitiveType:
			m_undoStack->push(new LoadRootPrimitiveCommand(QString(worldEditList[i].second.c_str()), m_worldEditorScene, m_primitivesModel));
			break;
		};
	}
	m_undoStack->endMacro();
}

void WorldEditorWindow::checkCurrentWorld()
{
}

void WorldEditorWindow::newWorldEditFile()
{
	checkCurrentWorld();

	m_undoStack->push(new CreateWorldCommand("NewWorldEdit", m_primitivesModel));
}

void WorldEditorWindow::saveWorldEditFile()
{
}

void WorldEditorWindow::openProjectSettings()
{
	ProjectSettingsDialog *dialog = new ProjectSettingsDialog(m_zoneBuilderBase->dataPath(), this);
	dialog->show();
	int ok = dialog->exec();
	if (ok == QDialog::Accepted)
	{
		m_zoneBuilderBase->init(dialog->dataPath(), true);
	}
	delete dialog;
}

void WorldEditorWindow::setMode(int value)
{
	switch (value)
	{
	case 0:
		m_worldEditorScene->setModeEdit(WorldEditorScene::SelectMode);
		break;
	case 1:
		m_worldEditorScene->setModeEdit(WorldEditorScene::MoveMode);
		break;
	case 2:
		m_worldEditorScene->setModeEdit(WorldEditorScene::RotateMode);
		break;
	case 3:
		m_worldEditorScene->setModeEdit(WorldEditorScene::ScaleMode);
		break;
	case 4:
		m_worldEditorScene->setModeEdit(WorldEditorScene::TurnMode);
		break;
	case 5:
		m_worldEditorScene->setModeEdit(WorldEditorScene::RadiusMode);
		break;
	}
}

void WorldEditorWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	if (m_oglWidget != 0)
		m_oglWidget->makeCurrent();
	//m_statusInfo->show();
	//m_statusBarTimer->start(100);
}

void WorldEditorWindow::hideEvent(QHideEvent *hideEvent)
{
	QMainWindow::hideEvent(hideEvent);
	//m_statusInfo->hide();
	//m_statusBarTimer->stop();
}

void WorldEditorWindow::createMenus()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void WorldEditorWindow::createToolBars()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
	//QAction *action = menuManager->action(Core::Constants::NEW);
	//m_ui.fileToolBar->addAction(action);

	m_ui.fileToolBar->addAction(m_ui.newWorldEditAction);
	QAction *action = menuManager->action(Core::Constants::OPEN);
	m_ui.fileToolBar->addAction(action);
	m_ui.fileToolBar->addAction(m_ui.saveWorldEditAction);
	m_ui.fileToolBar->addSeparator();

	action = menuManager->action(Core::Constants::UNDO);
	if (action != 0)
		m_ui.fileToolBar->addAction(action);

	action = menuManager->action(Core::Constants::REDO);
	if (action != 0)
		m_ui.fileToolBar->addAction(action);

	//action = menuManager->action(Core::Constants::SAVE);
	//m_ui.fileToolBar->addAction(action);
	//action = menuManager->action(Core::Constants::SAVE_AS);
	//m_ui.fileToolBar->addAction(action);
}

void WorldEditorWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
	restoreState(settings->value(Constants::WORLD_WINDOW_STATE).toByteArray());
	restoreGeometry(settings->value(Constants::WORLD_WINDOW_GEOMETRY).toByteArray());

	// Use OpenGL graphics system instead raster graphics system
	if (settings->value(Constants::WORLD_EDITOR_USE_OPENGL, true).toBool())
	{
		m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
		m_ui.graphicsView->setViewport(m_oglWidget);
	}

	settings->endGroup();
}

void WorldEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
	settings->setValue(Constants::WORLD_WINDOW_STATE, saveState());
	settings->setValue(Constants::WORLD_WINDOW_GEOMETRY, saveGeometry());
	settings->endGroup();
	settings->sync();
}

} /* namespace LandscapeEditor */
