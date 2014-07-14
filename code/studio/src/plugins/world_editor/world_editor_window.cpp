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
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// Lanscape Editor plugin
#include "../landscape_editor/builder_zone_base.h"

// NeL includes

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QStatusBar>
#include <QtGui/QMessageBox>
#include <QPersistentModelIndex>

namespace WorldEditor
{

WorldEditorWindow::WorldEditorWindow(QWidget *parent)
	: QMainWindow(parent),
	  m_primitivesModel(0),
	  m_undoStack(0),
	  m_oglWidget(0)
{
	m_ui.setupUi(this);
	m_undoStack = new QUndoStack(this);

	m_primitivesModel = new PrimitivesTreeModel(this);

	m_worldEditorScene = new WorldEditorScene(Utils::ligoConfig()->CellSize, m_primitivesModel, m_undoStack, this);
	m_zoneBuilderBase = new LandscapeEditor::ZoneBuilderBase(m_worldEditorScene);

	m_worldEditorScene->setZoneBuilder(m_zoneBuilderBase);
	m_ui.graphicsView->setScene(m_worldEditorScene);
	m_ui.graphicsView->setVisibleText(false);

	m_ui.treePrimitivesView->setModel(m_primitivesModel);
	m_ui.treePrimitivesView->setUndoStack(m_undoStack);
	m_ui.treePrimitivesView->setZoneBuilder(m_zoneBuilderBase);
	m_ui.treePrimitivesView->setWorldScene(m_worldEditorScene);

	QActionGroup *sceneModeGroup = new QActionGroup(this);
	sceneModeGroup->addAction(m_ui.selectAction);
	sceneModeGroup->addAction(m_ui.moveAction);
	sceneModeGroup->addAction(m_ui.rotateAction);
	sceneModeGroup->addAction(m_ui.scaleAction);
	sceneModeGroup->addAction(m_ui.turnAction);
	m_ui.selectAction->setChecked(true);

	m_ui.newWorldEditAction->setIcon(QIcon(Core::Constants::ICON_NEW));
	m_ui.saveWorldEditAction->setIcon(QIcon(Core::Constants::ICON_SAVE));

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

	connect(m_modeMapper, SIGNAL(mapped(int)), this, SLOT(setMode(int)));
	connect(m_ui.pointsAction, SIGNAL(triggered(bool)), m_worldEditorScene, SLOT(setEnabledEditPoints(bool)));

	connect(m_ui.settingsAction, SIGNAL(triggered()), this, SLOT(openProjectSettings()));
	connect(m_ui.newWorldEditAction, SIGNAL(triggered()), this, SLOT(newWorldEditFile()));
	connect(m_ui.saveWorldEditAction, SIGNAL(triggered()), this, SLOT(saveWorldEditFile()));
	connect(m_ui.visibleGridAction, SIGNAL(toggled(bool)), m_ui.graphicsView, SLOT(setVisibleGrid(bool)));

	connect(m_ui.treePrimitivesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(updateSelection(QItemSelection, QItemSelection)));

	connect(m_worldEditorScene, SIGNAL(updateSelectedItems(QList<QGraphicsItem *>)),
			this, SLOT(selectedItemsInScene(QList<QGraphicsItem *>)));

	m_statusBarTimer = new QTimer(this);
	connect(m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));

	m_statusInfo = new QLabel(this);
	m_statusInfo->hide();
	Core::ICore::instance()->mainWindow()->statusBar()->addPermanentWidget(m_statusInfo);
}

WorldEditorWindow::~WorldEditorWindow()
{
	writeSettings();

	delete m_zoneBuilderBase;

	Core::ICore::instance()->mainWindow()->statusBar()->removeWidget( m_statusInfo );
	delete m_statusInfo;
	m_statusInfo = NULL;
}

QUndoStack *WorldEditorWindow::undoStack() const
{
	return m_undoStack;
}

void WorldEditorWindow::maybeSave()
{
	QMessageBox *messageBox = new QMessageBox(tr("World Editor"),
			tr("The data has been modified.\n"
			   "Do you want to save your changes?"),
			QMessageBox::Warning,
			QMessageBox::Yes | QMessageBox::Default,
			QMessageBox::No,
			QMessageBox::Cancel | QMessageBox::Escape,
			this, Qt::Sheet);

	messageBox->setButtonText(QMessageBox::Yes,
							  tr("Save"));

	messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

	messageBox->show();
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
	if (m_primitivesModel->isWorldEditNodeLoaded())
		return;

	Utils::WorldEditList worldEditList;
	if (!Utils::loadWorldEditFile(fileName.toUtf8().constData(), worldEditList))
	{
		std::string error = Utils::getLastError();

		QMessageBox::critical( this,
								tr( "Error opening world editor file" ),
								tr( error.c_str() ) );

		return;
	}

	m_undoStack->beginMacro(tr("Load %1").arg(fileName));

	checkCurrentWorld();

	m_undoStack->push(new CreateWorldCommand(fileName, m_primitivesModel));
	for (size_t i = 0; i < worldEditList.size(); ++i)
	{
		switch (worldEditList[i].first)
		{
		case Utils::DataDirectoryType:
			m_zoneBuilderBase->init(QString(worldEditList[i].second.c_str()), true);
			m_dataDir = worldEditList[i].second.c_str();
			break;
		case Utils::ContextType:
			m_context = worldEditList[i].second.c_str();
			break;
		case Utils::LandscapeType:
			m_undoStack->push(new LoadLandscapeCommand(QString(worldEditList[i].second.c_str()), m_primitivesModel, m_zoneBuilderBase));
			break;
		case Utils::PrimitiveType:
			m_undoStack->push(new LoadRootPrimitiveCommand(QString(worldEditList[i].second.c_str()),
							  m_worldEditorScene, m_primitivesModel, m_ui.treePrimitivesView));
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
	WorldSaver saver( m_primitivesModel, m_zoneBuilderBase, m_dataDir.toUtf8().constData(), m_context.toUtf8().constData() );
	bool ok = saver.save();
	QString error = saver.getLastError().c_str();

	if( !ok )
	{
		QMessageBox::critical( this,
								tr( "Failed to save world editor files" ),
								tr( "Failed to save world editor files.\nError:\n " ) + error  );
	}
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

void WorldEditorWindow::updateStatusBar()
{
	m_statusInfo->setText(m_worldEditorScene->zoneNameFromMousePos());
}

void WorldEditorWindow::updateSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
	m_ui.pointsAction->setChecked(false);
	m_worldEditorScene->setEnabledEditPoints(false);

	NodeList nodesSelected;
	Q_FOREACH(QModelIndex modelIndex, selected.indexes())
	{
		Node *node = static_cast<Node *>(modelIndex.internalPointer());
		nodesSelected.push_back(node);
	}

	NodeList nodesDeselected;
	Q_FOREACH(QModelIndex modelIndex, deselected.indexes())
	{
		Node *node = static_cast<Node *>(modelIndex.internalPointer());
		nodesDeselected.push_back(node);
	}

	// update property editor
	if (nodesSelected.size() > 0)
	{
		// only single selection
		m_ui.propertyEditWidget->updateSelection(nodesSelected.at(0));
	}
	else
	{
		m_ui.propertyEditWidget->clearProperties();
	}

	QList<QGraphicsItem *> itemSelected;
	Q_FOREACH(Node *node, nodesSelected)
	{
		QGraphicsItem *item = getGraphicsItem(node);
		if (item != 0)
			itemSelected.push_back(item);
	}

	QList<QGraphicsItem *> itemDeselected;
	Q_FOREACH(Node *node, nodesDeselected)
	{
		QGraphicsItem *item = getGraphicsItem(node);
		if (item != 0)
			itemDeselected.push_back(item);
	}

	// Update world editor scene
	m_worldEditorScene->updateSelection(itemSelected, itemDeselected);
}

void WorldEditorWindow::selectedItemsInScene(const QList<QGraphicsItem *> &selected)
{
	QItemSelectionModel *selectionModel = m_ui.treePrimitivesView->selectionModel();
	disconnect(m_ui.treePrimitivesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			   this, SLOT(updateSelection(QItemSelection, QItemSelection)));

	selectionModel->clear();
	QItemSelection itemSelection;
	Q_FOREACH(QGraphicsItem *item, selected)
	{
		QPersistentModelIndex *index = qvariant_cast<QPersistentModelIndex *>(item->data(Constants::NODE_PERISTENT_INDEX));
		if (index->isValid())
		{
			QModelIndex modelIndex = index->operator const QModelIndex &();
			QItemSelection mergeItemSelection(modelIndex, modelIndex);
			itemSelection.merge(mergeItemSelection, QItemSelectionModel::Select);
		}
		QApplication::processEvents();
	}

	selectionModel->select(itemSelection, QItemSelectionModel::Select);

	// update property editor
	if (!selected.isEmpty())
	{
		// only single selection
		Node *node = qvariant_cast<Node *>(selected.at(0)->data(Constants::WORLD_EDITOR_NODE));
		m_ui.propertyEditWidget->updateSelection(node);
	}
	else
	{
		m_ui.propertyEditWidget->clearProperties();
	}

	connect(m_ui.treePrimitivesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(updateSelection(QItemSelection, QItemSelection)));
}

void WorldEditorWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	if (m_oglWidget != 0)
		m_oglWidget->makeCurrent();
	m_statusInfo->show();
	m_statusBarTimer->start(100);
}

void WorldEditorWindow::hideEvent(QHideEvent *hideEvent)
{
	QMainWindow::hideEvent(hideEvent);
	m_statusInfo->hide();
	m_statusBarTimer->stop();
}

void WorldEditorWindow::createMenus()
{
	//Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void WorldEditorWindow::createToolBars()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
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
		//m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::SampleBuffers));
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
