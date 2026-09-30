// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QToolBar>
#include "world_editor_constants.h"
#include "primitives_model.h"
#include "world_editor_scene.h"
#include "world_editor_misc.h"
#include "world_editor_actions.h"
#include "world_editor_scene_item.h"
#include "pacs_overlay.h"
#include "primitive_item.h"
#include "project_settings_dialog.h"

// Core
#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// Lanscape Editor plugin
#include "../landscape_editor/builder_zone_base.h"
#include "../landscape_editor/zone_region_editor.h"
#include "../landscape_editor/landscape_view.h"
#include "../landscape_editor/landscape_editor_constants.h"

// NeL includes

// Qt includes
#include <QtCore/QSettings>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QMessageBox>
#include <QPersistentModelIndex>

namespace WorldEditor
{

WorldEditorWindow::WorldEditorWindow(QWidget *parent)
	: QMainWindow(parent),
	  m_primitivesModel(0),
	  m_undoStack(0),
	  m_oglWidget(0),
	  m_panelsChecked(false),
	  m_history(0),
	  m_historyView(0),
	  m_historyDock(0),
	  m_lastUndoIndex(0),
	  m_lastUndoCount(0),
	  m_visiblePacsAction(0),
	  m_pacsLegendAction(0),
	  m_pacsOverlay(0)
{

	m_ui.setupUi(this);
	m_undoStack = new QUndoStack(this);

	m_primitivesModel = new PrimitivesTreeModel(this);

	m_worldEditorScene = new WorldEditorScene(Utils::ligoConfig()->CellSize, m_primitivesModel, m_undoStack, this);
	m_zoneBuilderBase = new LandscapeEditor::ZoneBuilderBase(m_worldEditorScene);

	m_worldEditorScene->setZoneBuilder(m_zoneBuilderBase);

	// Hidden until switched on; the borders are loaded only then.
	m_pacsOverlay = new PacsOverlay(Utils::ligoConfig()->CellSize);
	m_pacsOverlay->setVisible(false);
	m_worldEditorScene->addItem(m_pacsOverlay);
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
	createHistoryPanel();
	readSettings();

	// Preset the zone builder with the configured landscape directory. Without it the
	// builder stays empty until a .worldedit project supplies its DATA_DIRECTORY or the
	// "Project Settings" dialog is used - a .land opened on its own then reports nothing
	// but "Pixmap and LIGO files not found". A project overrides the value later on.
	{
		QSettings *settings = Core::ICore::instance()->settings();
		settings->beginGroup(LandscapeEditor::Constants::LANDSCAPE_EDITOR_SECTION);
		const QString defaultDataDir =
			settings->value(LandscapeEditor::Constants::LANDSCAPE_DATA_DIRECTORY).toString();
		settings->endGroup();
		if (!defaultDataDir.isEmpty())
			m_zoneBuilderBase->init(defaultDataDir, false);
	}

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
	// Every one of these switches was in the tool bar but wired to nothing, so they sat
	// there greyed out. The .ui marks them disabled, so enable them here as well.
	m_ui.visibleLandAction->setEnabled(true);
	m_ui.visibleZonePrimitivesAction->setEnabled(true);
	m_ui.visiblePathPrimitivesAction->setEnabled(true);
	m_ui.vidiblePointPrimitives->setEnabled(true);
	m_ui.visibleDetailsAction->setEnabled(true);
	m_ui.visibleGridPointsAction->setEnabled(true);

	connect(m_ui.visibleGridAction, SIGNAL(toggled(bool)), m_ui.graphicsView, SLOT(setVisibleGrid(bool)));
	connect(m_ui.visibleLandAction, SIGNAL(toggled(bool)), this, SLOT(setVisibleLand(bool)));
	connect(m_ui.visibleZonePrimitivesAction, SIGNAL(toggled(bool)), this, SLOT(setVisibleZonePrimitives(bool)));
	connect(m_ui.visiblePathPrimitivesAction, SIGNAL(toggled(bool)), this, SLOT(setVisiblePathPrimitives(bool)));
	connect(m_ui.vidiblePointPrimitives, SIGNAL(toggled(bool)), this, SLOT(setVisiblePointPrimitives(bool)));
	connect(m_ui.visibleDetailsAction, SIGNAL(toggled(bool)), this, SLOT(setVisibleDetails(bool)));
	connect(m_ui.visibleGridPointsAction, SIGNAL(toggled(bool)), this, SLOT(setVisibleGridPoints(bool)));

	connect(m_ui.treePrimitivesView, SIGNAL(landscapeLoaded()), this, SLOT(updatePacs()));

	connect(m_ui.treePrimitivesView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
			this, SLOT(updateSelection(QItemSelection, QItemSelection)));

	connect(m_worldEditorScene, SIGNAL(updateSelectedItems(QList<QGraphicsItem *>)),
			this, SLOT(selectedItemsInScene(QList<QGraphicsItem *>)));

	connect(m_worldEditorScene, SIGNAL(contextMenuRequested(QGraphicsItem *, QPoint)),
			this, SLOT(showSceneContextMenu(QGraphicsItem *, QPoint)));

	connect(m_ui.treePrimitivesView, SIGNAL(zoomToRectRequested(QRectF)),
			this, SLOT(zoomToRect(QRectF)));

	connect(m_undoStack, SIGNAL(indexChanged(int)), this, SLOT(recordUndoChange(int)));

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
					   tr("Open NeL World Edit file"),
					   Utils::lastDirectory(Constants::LAST_WORLD_EDIT_DIR),
					   tr("All NeL World Editor file (*.worldedit)"));

	setCursor(Qt::WaitCursor);
	if (!fileName.isEmpty())
	{
		Utils::setLastDirectory(Constants::LAST_WORLD_EDIT_DIR, fileName);
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

	if (!checkCurrentWorld())
		return;

	m_history->setProjectFile(fileName);

	m_undoStack->beginMacro(tr("Load %1").arg(QFileInfo(fileName).fileName()));

	m_undoStack->push(new CreateWorldCommand(fileName, m_primitivesModel));
	bool haveDataDir = false;
	for (size_t i = 0; i < worldEditList.size(); ++i)
	{
		switch (worldEditList[i].first)
		{
		case Utils::DataDirectoryType:
		{
			const QString dataDir = QString::fromUtf8(worldEditList[i].second.c_str());
			if (dataDir.isEmpty())
				break;
			m_zoneBuilderBase->init(dataDir, true);
			m_dataDir = dataDir;
			haveDataDir = true;
			break;
		}
		case Utils::ContextType:
			m_context = worldEditList[i].second.c_str();
			break;
		case Utils::LandscapeType:
		{
			const QString landFile = QString::fromUtf8(worldEditList[i].second.c_str());

			// Many project files carry an empty <DATA_DIRECTORY/>. Without a data path
			// the zone builder keeps the zone bank it loaded last, and then not a single
			// tile finds its image. By convention the zone data sits next to the .land
			// file (zoneligos/ and zonebitmaps/), so take the directory from there.
			if (!haveDataDir)
			{
				const QString landDir = QFileInfo(landFile).absolutePath();
				if (m_zoneBuilderBase->init(landDir, true))
				{
					m_dataDir = landDir;
					haveDataDir = true;
					nlinfo("World editor: no data directory in project, using '%s'",
					       landDir.toUtf8().constData());
				}
			}

			m_undoStack->push(new LoadLandscapeCommand(landFile, m_primitivesModel, m_zoneBuilderBase));
			break;
		}
		case Utils::PrimitiveType:
			m_undoStack->push(new LoadRootPrimitiveCommand(QString(worldEditList[i].second.c_str()),
							  m_worldEditorScene, m_primitivesModel, m_ui.treePrimitivesView));
			break;
		};
	}
	m_undoStack->endMacro();

	focusOnLandscape();
	updatePacs();
}

bool WorldEditorWindow::checkCurrentWorld()
{
	// This function used to be an empty body. With a world already loaded, every "New"
	// and every "Open" therefore ran into the nlerror in
	// PrimitivesTreeModel::createWorldEditNode() ("World edit node already is created.")
	// - which terminates the program on the spot.
	if (!m_primitivesModel->isWorldEditNodeLoaded())
		return true;

	const QMessageBox::StandardButton answer = QMessageBox::question(this,
			tr("Close current world"),
			tr("A world is already open. Close it and lose unsaved changes?"),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (answer != QMessageBox::Yes)
		return false;

	// Release the loaded zone regions, otherwise they stay behind in the view. By id:
	// deleting "region 0" until none are left never ended once id 0 was gone.
	Q_FOREACH (int id, m_zoneBuilderBase->zoneRegionIds())
		m_zoneBuilderBase->deleteZoneRegion(id);

	m_undoStack->clear();
	m_primitivesModel->deleteWorldEditNode();
	m_dataDir.clear();
	m_context.clear();
	m_pacsOverlay->clear();
	return true;
}

void WorldEditorWindow::newWorldEditFile()
{
	if (!checkCurrentWorld())
		return;

	m_undoStack->push(new CreateWorldCommand("NewWorldEdit", m_primitivesModel));
}

void WorldEditorWindow::saveWorldEditFileAs()
{
	QString fileName = QFileDialog::getSaveFileName(this,
						   tr("Save NeL World Edit file"),
						   Utils::lastDirectory(Constants::LAST_WORLD_EDIT_DIR),
						   tr("All NeL World Edit files (*.worldedit)"));

	if (fileName.isEmpty())
		return;

	if (!fileName.endsWith(QLatin1String(".worldedit"), Qt::CaseInsensitive))
		fileName += QLatin1String(".worldedit");

	// The name of the root node doubles as the target path WorldSaver uses.
	m_primitivesModel->setWorldEditFileName(fileName);
	m_history->setProjectFile(fileName);
	Utils::setLastDirectory(Constants::LAST_WORLD_EDIT_DIR, fileName);

	saveWorldEditFile();
}

void WorldEditorWindow::saveWorldEditFile()
{
	// A new world is called "NewWorldEdit" and has no target path yet. Ask for a file
	// name first in that case, rather than writing a file without a path or suffix.
	const QModelIndex rootIndex = m_primitivesModel->index(0, 0);
	const WorldEditNode *rootNode = rootIndex.isValid()
		? dynamic_cast<WorldEditNode *>(static_cast<Node *>(rootIndex.internalPointer()))
		: 0;
	const QString currentName = (rootNode != 0) ? rootNode->fileName() : QString();

	if (!currentName.endsWith(QLatin1String(".worldedit"), Qt::CaseInsensitive))
	{
		saveWorldEditFileAs();
		return;
	}

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

	// The whole selection, not just what changed in this step: with several rows picked,
	// "selected" holds only the ones that came last, and the property editor would show
	// that one alone.
	NodeList nodesSelected;
	Q_FOREACH(QModelIndex modelIndex, m_ui.treePrimitivesView->selectionModel()->selectedRows())
	{
		Node *node = static_cast<Node *>(modelIndex.internalPointer());
		if (node != 0)
			nodesSelected.push_back(node);
	}

	// The scene only needs what changed, to repaint those items.
	NodeList nodesJustSelected;
	Q_FOREACH(QModelIndex modelIndex, selected.indexes())
	{
		Node *node = static_cast<Node *>(modelIndex.internalPointer());
		nodesJustSelected.push_back(node);
	}

	NodeList nodesDeselected;
	Q_FOREACH(QModelIndex modelIndex, deselected.indexes())
	{
		Node *node = static_cast<Node *>(modelIndex.internalPointer());
		nodesDeselected.push_back(node);
	}

	// The property editor shows all of them at once now, not just the first.
	if (nodesSelected.size() > 0)
		m_ui.propertyEditWidget->updateSelection(nodesSelected);
	else
		m_ui.propertyEditWidget->clearProperties();

	QList<QGraphicsItem *> itemSelected;
	Q_FOREACH(Node *node, nodesJustSelected)
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
	QModelIndex firstIndex;
	Q_FOREACH(QGraphicsItem *item, selected)
	{
		QPersistentModelIndex *index = qvariant_cast<QPersistentModelIndex *>(item->data(Constants::NODE_PERISTENT_INDEX));
		if ((index != 0) && index->isValid())
		{
			QModelIndex modelIndex = index->operator const QModelIndex &();
			QItemSelection mergeItemSelection(modelIndex, modelIndex);
			itemSelection.merge(mergeItemSelection, QItemSelectionModel::Select);
			if (!firstIndex.isValid())
				firstIndex = modelIndex;
		}
		QApplication::processEvents();
	}

	selectionModel->select(itemSelection, QItemSelectionModel::Select);

	// Selecting the row is not enough while the tree around it is folded shut.
	m_ui.treePrimitivesView->revealIndex(firstIndex);

	// Same for a selection made on the map.
	NodeList nodesInScene;
	Q_FOREACH (QGraphicsItem *item, selected)
	{
		Node *node = qvariant_cast<Node *>(item->data(Constants::WORLD_EDITOR_NODE));
		if (node != 0)
			nodesInScene.push_back(node);
	}

	if (!nodesInScene.isEmpty())
		m_ui.propertyEditWidget->updateSelection(nodesInScene);
	else
		m_ui.propertyEditWidget->clearProperties();

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

	// restoreState() ran in the constructor, while this window was not visible yet, so
	// Qt only applies the restored arrangement now. A check made before that would have
	// looked at panels Qt is about to hide again, so repeat it here.
	if (!m_panelsChecked)
	{
		m_panelsChecked = true;
		ensurePanelsUsable();
	}
}

void WorldEditorWindow::hideEvent(QHideEvent *hideEvent)
{
	// Only hidden along with its parent - another tab, or the main window closing - the
	// arrangement is still the one the user left. Keep it: by the time the destructor
	// runs, it is gone (see writeSettings()).
	if (!isHidden())
	{
		m_savedWindowState = saveState();
		m_savedWindowGeometry = saveGeometry();
	}
	QMainWindow::hideEvent(hideEvent);
	m_statusInfo->hide();
	m_statusBarTimer->stop();
}

void WorldEditorWindow::createMenus()
{
	// The docks and tool bars of this window carry a close box, but there was no way
	// back: once closed they stayed closed, because the state is saved on exit.
	// QMainWindow::createPopupMenu() supplies exactly the right toggles - put here as a
	// submenu under "View", so it stays reachable even when every tool bar of this
	// window is hidden.
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
	QMenu *viewMenu = menuManager->menu(Core::Constants::M_VIEW);
	if (viewMenu == 0)
		return;

	m_panelsMenu = viewMenu->addMenu(tr("World Editor Panels"));
	connect(m_panelsMenu, SIGNAL(aboutToShow()), this, SLOT(updatePanelsMenu()));
}

void WorldEditorWindow::updatePanelsMenu()
{
	// The contents are rebuilt every time the menu opens, so they show the current
	// state. createPopupMenu() returns a fresh menu whose actions we take over; the
	// empty shell menu is discarded afterwards.
	m_panelsMenu->clear();
	QMenu *generated = createPopupMenu();
	if (generated == 0)
		return;

	Q_FOREACH (QAction *action, generated->actions())
		m_panelsMenu->addAction(action);

	generated->deleteLater();
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

	// There was no "Save As". Without it a newly created world cannot be stored at all:
	// WorldSaver takes the name of the root node as the file path, and for a new world
	// that name is simply "NewWorldEdit" - neither a path nor a suffix.
	m_saveAsAction = new QAction(tr("Save &As..."), this);
	m_saveAsAction->setIcon(QIcon(Core::Constants::ICON_SAVE_AS));
	m_saveAsAction->setShortcut(QKeySequence(tr("Ctrl+Shift+S")));
	connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(saveWorldEditFileAs()));
	m_ui.fileToolBar->addAction(m_saveAsAction);

	// The collision footprints are useful when placing things and in the way when reading
	// the map, so they get a switch of their own next to the other show/hide toggles.
	m_visibleCollisionsAction = new QAction(tr("S/H Collisions"), this);
	m_visibleCollisionsAction->setToolTip(tr("Show or hide the collision shape each sheet declares"));
	m_visibleCollisionsAction->setCheckable(true);
	connect(m_visibleCollisionsAction, SIGNAL(toggled(bool)), this, SLOT(setVisibleCollisions(bool)));
	m_ui.shToolBar->addAction(m_visibleCollisionsAction);

	// ID_VIEW_PACS of the MFC editor: the borders the server moves entities along.
	m_visiblePacsAction = new QAction(tr("S/H PACS"), this);
	m_visiblePacsAction->setToolTip(tr("Show or hide the PACS borders of the continent,\n"
									   "the ground the server lets entities walk on"));
	m_visiblePacsAction->setCheckable(true);
	connect(m_visiblePacsAction, SIGNAL(toggled(bool)), this, SLOT(setVisiblePacs(bool)));
	m_ui.shToolBar->addAction(m_visiblePacsAction);
	m_pacsLegendAction = m_ui.shToolBar->addWidget(createPacsLegend());
	m_pacsLegendAction->setVisible(false);

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

	// Remember the arrangement world_editor_window.ui just set up. It is the only way
	// back if the stored state turns out to be unusable, and it has to be read before
	// restoreState() replaces the layout.
	m_dockDefaults.clear();
	m_toolBarDefaults.clear();
	Q_FOREACH (QDockWidget *dock, findChildren<QDockWidget *>(QString(), Qt::FindDirectChildrenOnly))
		m_dockDefaults.append(qMakePair(dock, dockWidgetArea(dock)));
	Q_FOREACH (QToolBar *toolBar, findChildren<QToolBar *>(QString(), Qt::FindDirectChildrenOnly))
		m_toolBarDefaults.append(qMakePair(toolBar, toolBarArea(toolBar)));

	restoreState(settings->value(Constants::WORLD_WINDOW_STATE).toByteArray());
	ensurePanelsUsable();
	restoreGeometry(settings->value(Constants::WORLD_WINDOW_GEOMETRY).toByteArray());

	// setChecked() emits toggled() only on a change, so apply the value either way.
	const bool visibleCollisions =
			settings->value(Constants::VISIBLE_COLLISIONS, true).toBool();
	WorldEditor::setVisibleCollisions(visibleCollisions);
	m_visibleCollisionsAction->setChecked(visibleCollisions);

	restoreSwitch(m_ui.visibleLandAction, Constants::VISIBLE_LAND, true);
	restoreSwitch(m_ui.visibleZonePrimitivesAction, Constants::VISIBLE_ZONE_PRIMITIVES, true);
	restoreSwitch(m_ui.visiblePathPrimitivesAction, Constants::VISIBLE_PATH_PRIMITIVES, true);
	restoreSwitch(m_ui.vidiblePointPrimitives, Constants::VISIBLE_POINT_PRIMITIVES, true);
	restoreSwitch(m_ui.visibleDetailsAction, Constants::VISIBLE_DETAILS, false);
	restoreSwitch(m_ui.visibleGridAction, Constants::VISIBLE_GRID, true);
	restoreSwitch(m_ui.visibleGridPointsAction, Constants::VISIBLE_GRID_POINTS, false);
	restoreSwitch(m_visiblePacsAction, Constants::VISIBLE_PACS, false);

	// Use OpenGL graphics system instead raster graphics system
	if (settings->value(Constants::WORLD_EDITOR_USE_OPENGL, true).toBool())
	{
		m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
		//m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::SampleBuffers));
		m_ui.graphicsView->setViewport(m_oglWidget);
	}

	settings->endGroup();
}

void WorldEditorWindow::setVisibleCollisions(bool visible)
{
	WorldEditor::setVisibleCollisions(visible);
	m_worldEditorScene->update();
}

QWidget *WorldEditorWindow::createPacsLegend()
{
	// What the colours mean, next to the switch. The meanings follow
	// NLPACS::CGlobalRetriever::getBorders() and how build_rbank cuts the ground into
	// surfaces: a new surface starts at every 2 m of height, at the water's edge and at
	// every zone border, and ground too steep to walk on is left out.
	struct Entry
	{
		PacsOverlay::EdgeType type;
		const char *label;
		const char *explanation;
	};
	static const Entry entries[] =
	{
		{ PacsOverlay::Block, QT_TR_NOOP("Blocked"),
		  QT_TR_NOOP("Edge of the walkable ground. Nothing moves across it: behind it the slope\n"
					 "is too steep, a building or rock stands there, or the map simply ends.") },
		{ PacsOverlay::Surmountable, QT_TR_NOOP("Height step"),
		  QT_TR_NOOP("Border between two walkable surfaces, walkable in both directions.\n"
					 "PACS starts a new surface every 2 m of height, so these lines are\n"
					 "effectively contour lines.") },
		{ PacsOverlay::Link, QT_TR_NOOP("Zone link"),
		  QT_TR_NOOP("Border of a 160 m zone, where its ground joins the neighbouring zone.\n"
					 "Walkable wherever the other side has walkable ground too.") },
		{ PacsOverlay::Waterline, QT_TR_NOOP("Waterline"),
		  QT_TR_NOOP("Where the ground goes under water: dry surface on one side,\n"
					 "submerged surface on the other.") },
		{ PacsOverlay::Other, QT_TR_NOOP("Interior"),
		  QT_TR_NOOP("Outline of an interior (building, cave) placed on the continent,\n"
					 "including its openings to the outside.") }
	};

	QWidget *legend = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(legend);
	layout->setContentsMargins(6, 0, 6, 0);
	layout->setSpacing(10);
	for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); ++i)
	{
		const QString color = PacsOverlay::edgeColor(entries[i].type).name();
		QLabel *label = new QLabel(QString("<span style=\"color:%1; font-size:large;\">&#9632;</span> %2")
								   .arg(color, tr(entries[i].label)), legend);
		label->setToolTip(tr(entries[i].explanation));
		layout->addWidget(label);
	}
	legend->setToolTip(tr("PACS borders - hover over an entry for what it means"));
	return legend;
}

void WorldEditorWindow::setVisiblePacs(bool visible)
{
	m_pacsOverlay->setVisible(visible);
	m_pacsLegendAction->setVisible(visible);
	if (visible)
		updatePacs();
}

void WorldEditorWindow::updatePacs()
{
	const QString dir = pacsDirectory();
	if (dir == m_pacsOverlay->directory())
		return;

	// Loading a whole continent takes a few seconds. Only do it while the borders are
	// shown; switching them on later catches up.
	if (!m_pacsOverlay->isVisible())
	{
		m_pacsOverlay->clear();
		return;
	}

	if (dir.isEmpty())
	{
		m_pacsOverlay->clear();
		statusBar()->showMessage(tr("No PACS directory next to the landscape or in the data directory"), 5000);
		return;
	}

	statusBar()->showMessage(tr("Loading PACS from %1 ...").arg(dir));
	QApplication::setOverrideCursor(Qt::WaitCursor);
	const int loaded = m_pacsOverlay->load(dir);
	QApplication::restoreOverrideCursor();

	if (loaded == 0)
		statusBar()->showMessage(tr("No PACS could be loaded from %1").arg(dir), 5000);
	else
		statusBar()->showMessage(tr("PACS: %1 border segments from %2")
								 .arg(m_pacsOverlay->edgeCount()).arg(dir), 5000);
}

QString WorldEditorWindow::pacsDirectory() const
{
	// Continent directories to take the PACS from, in order of preference.
	QStringList continents;

	// The landscape that is on screen comes first. A .land opened on its own leaves the
	// data directory at its configured default, which may be another continent.
	const QModelIndex worldEdit = m_primitivesModel->index(0, 0);
	for (int row = 0; worldEdit.isValid() && row < m_primitivesModel->rowCount(worldEdit); ++row)
	{
		const QModelIndex index = m_primitivesModel->index(row, 0, worldEdit);
		Node *node = static_cast<Node *>(index.internalPointer());
		if ((node != 0) && (node->type() == Node::LandscapeNodeType))
		{
			const QString landFile = static_cast<LandscapeNode *>(node)->fileName();
			continents << QFileInfo(landFile).absolutePath();
			break;
		}
	}

	const QString dataDir = m_zoneBuilderBase->dataPath();
	if (!dataDir.isEmpty())
		continents << dataDir;

	// The pacs/ directories next to the landscapes are whatever was copied there once,
	// and several no longer match what the game ships (continents/nexus/pacs is neither
	// the live state nor the current test build). A PACS root holding <continent>_pacs
	// directories, laid out like the unpacked client data, therefore comes first.
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::WORLD_EDITOR_SECTION);
	const QString pacsRoot = settings->value(Constants::PACS_ROOT).toString();
	settings->endGroup();

	Q_FOREACH (const QString &continent, continents)
	{
		const QString name = QDir(continent).dirName();
		if (!pacsRoot.isEmpty())
		{
			const QString shipped = QDir(pacsRoot).filePath(name + "_pacs");
			if (QFileInfo(shipped).isDir())
				return QDir(shipped).absolutePath();
		}

		const QString local = QDir(continent).filePath("pacs");
		if (QFileInfo(local).isDir())
			return QDir(local).absolutePath();
	}
	return QString();
}

void WorldEditorWindow::setVisibleLand(bool visible)
{
	m_worldEditorScene->setVisibleZones(visible);
}

void WorldEditorWindow::setVisibleZonePrimitives(bool visible)
{
	m_worldEditorScene->setVisibleZonePrimitives(visible);
}

void WorldEditorWindow::setVisiblePathPrimitives(bool visible)
{
	m_worldEditorScene->setVisiblePathPrimitives(visible);
}

void WorldEditorWindow::setVisiblePointPrimitives(bool visible)
{
	m_worldEditorScene->setVisiblePointPrimitives(visible);
}

void WorldEditorWindow::setVisibleDetails(bool visible)
{
	// "Details" are the zone names the view writes across the landscape once it is
	// zoomed in far enough.
	m_ui.graphicsView->setVisibleText(visible);
}

void WorldEditorWindow::setVisibleGridPoints(bool visible)
{
	m_ui.graphicsView->setVisibleGridPoints(visible);
}

void WorldEditorWindow::createHistoryPanel()
{
	m_history = new EditHistory(this);

	m_historyDock = new QDockWidget(tr("History"), this);
	QDockWidget *dock = m_historyDock;
	dock->setObjectName(QString::fromUtf8("historyDockWidget"));

	m_historyView = new QPlainTextEdit(dock);
	m_historyView->setReadOnly(true);
	m_historyView->setLineWrapMode(QPlainTextEdit::NoWrap);
	m_historyView->setPlaceholderText(
			tr("What was done to the project is recorded here, and next to the project "
			   "file as <name>.worldedit.history."));
	dock->setWidget(m_historyView);
	addDockWidget(Qt::RightDockWidgetArea, dock);

	connect(m_history, SIGNAL(historyLoaded(QStringList, QString)),
			this, SLOT(showHistory(QStringList, QString)));
	connect(m_history, SIGNAL(entryAppended(QString)),
			this, SLOT(appendHistory(QString)));
}

void WorldEditorWindow::showHistory(const QStringList &entries, const QString &projectFile)
{
	m_historyView->clear();
	if (!entries.isEmpty())
		m_historyView->appendPlainText(entries.join(QLatin1String("\n")));
	else
		m_historyView->appendPlainText(
				tr("No history recorded for %1 yet.").arg(QFileInfo(projectFile).fileName()));
}

void WorldEditorWindow::appendHistory(const QString &line)
{
	m_historyView->appendPlainText(line);
}

void WorldEditorWindow::recordUndoChange(int index)
{
	const int count = m_undoStack->count();

	if (index > m_lastUndoIndex)
	{
		// The stack only changes length when a command is pushed - a redo just moves the
		// index along. That is enough to tell a new action from a repeated one.
		const bool isNewAction = (count != m_lastUndoCount);
		for (int i = m_lastUndoIndex; i < index; ++i)
		{
			const QString text = m_undoStack->text(i);
			m_history->append(isNewAction ? text : tr("redo: %1").arg(text));
		}
	}
	else if (index < m_lastUndoIndex)
	{
		for (int i = m_lastUndoIndex - 1; i >= index; --i)
			m_history->append(tr("undo: %1").arg(m_undoStack->text(i)));
	}

	m_lastUndoIndex = index;
	m_lastUndoCount = count;
}

void WorldEditorWindow::zoomToRect(const QRectF &sceneRect)
{
	if (sceneRect.isNull())
		return;

	// A single NPC has next to no extent, and fitting the view to that lands between two
	// pixels of terrain. Frame a fixed piece of ground around it instead. A group already
	// spans something, so the minimum never applies there.
	const qreal minimumExtent = 50.0;

	QRectF target = sceneRect;
	if (target.width() < minimumExtent)
		target.adjust(-(minimumExtent - target.width()) / 2, 0,
					  (minimumExtent - target.width()) / 2, 0);
	if (target.height() < minimumExtent)
		target.adjust(0, -(minimumExtent - target.height()) / 2,
					  0, (minimumExtent - target.height()) / 2);

	m_ui.graphicsView->showRect(target);
}

void WorldEditorWindow::showSceneContextMenu(QGraphicsItem *item, const QPoint &globalPos)
{
	QPersistentModelIndex *index = qvariant_cast<QPersistentModelIndex *>(
			item->data(Constants::NODE_PERISTENT_INDEX));
	if ((index == 0) || !index->isValid())
		return;

	m_ui.treePrimitivesView->showContextMenu(index->operator const QModelIndex &(),
											 globalPos, true);
}

QRectF WorldEditorWindow::landscapeBounds() const
{
	QRectF bounds;
	Q_FOREACH (int id, m_zoneBuilderBase->zoneRegionIds())
	{
		LandscapeEditor::ZoneRegionObject *region = m_zoneBuilderBase->zoneRegion(id);
		if (region == 0)
			continue;

		const QRectF regionRect = Utils::zoneRegionSceneRect(region->ligoZoneRegion());
		if (!regionRect.isNull())
			bounds = bounds.isNull() ? regionRect : bounds.united(regionRect);
	}
	return bounds;
}

void WorldEditorWindow::focusOnLandscape()
{
	const QRectF bounds = landscapeBounds();
	if (bounds.isEmpty())
		return;

	m_ui.graphicsView->showRect(bounds);
}

void WorldEditorWindow::restoreSwitch(QAction *action, const char *settingsKey,
									  bool defaultValue)
{
	QSettings *settings = Core::ICore::instance()->settings();
	const bool value = settings->value(settingsKey, defaultValue).toBool();

	// setChecked() only emits toggled() when the state changes, so the scene would keep
	// its own default when the two already agree. Apply it by hand instead.
	action->setChecked(value);
	Q_EMIT action->toggled(value);
}

void WorldEditorWindow::ensurePanelsUsable()
{
	// A stored state in which every dock and every tool bar is hidden leaves a window
	// that cannot be worked with. The docks carry a close box, so reaching that state
	// takes no more than a few stray clicks, and it never recovers on its own: the
	// state is written back on exit, so the next start hides them again.
	typedef QPair<QDockWidget *, Qt::DockWidgetArea> DockDefault;
	typedef QPair<QToolBar *, Qt::ToolBarArea> ToolBarDefault;

	// The history panel is a read-only side note. A window showing nothing but the
	// journal is just as unusable as an empty one, so it does not count as a panel here.
	Q_FOREACH (const DockDefault &entry, m_dockDefaults)
	{
		if ((entry.first != m_historyDock) && !entry.first->isHidden())
			return;
	}
	Q_FOREACH (const ToolBarDefault &entry, m_toolBarDefaults)
	{
		if (!entry.first->isHidden())
			return;
	}

	if (m_dockDefaults.isEmpty() && m_toolBarDefaults.isEmpty())
		return;

	nlinfo("World Editor: the stored window state hides every panel and tool bar, "
		   "restoring the default arrangement.");

	// Adding them back is what matters. After restoring a state that listed them as
	// hidden, QMainWindow keeps them as place holders in its layout, and saveState()
	// writes the place holder's flag - not the widget's. Simply showing the widgets
	// would fix the current session and still store "hidden" again on exit.
	Q_FOREACH (const DockDefault &entry, m_dockDefaults)
	{
		addDockWidget(entry.second, entry.first);
		entry.first->setVisible(true);
	}
	Q_FOREACH (const ToolBarDefault &entry, m_toolBarDefaults)
	{
		addToolBar(entry.second, entry.first);
		entry.first->setVisible(true);
	}
}

void WorldEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::WORLD_EDITOR_SECTION);

	// On exit the context manager removes the tab and deletes its page. On the way this
	// window and every dock and tool bar in it get hidden explicitly, before this runs
	// from the destructor - so saveState() stored all panels as hidden, every time, and
	// ensurePanelsUsable() had to repair it on the next start. Measured with a log of
	// isHidden() and the bits saveState() writes at each step. Once explicitly hidden,
	// use the arrangement kept from the last time the window went out of view; if it
	// never was on screen this session, leave the stored one alone.
	if (!isHidden())
	{
		settings->setValue(Constants::WORLD_WINDOW_STATE, saveState());
		settings->setValue(Constants::WORLD_WINDOW_GEOMETRY, saveGeometry());
	}
	else if (!m_savedWindowState.isEmpty())
	{
		settings->setValue(Constants::WORLD_WINDOW_STATE, m_savedWindowState);
		settings->setValue(Constants::WORLD_WINDOW_GEOMETRY, m_savedWindowGeometry);
	}

	// Write the effective value back, not a bare read. Without the default, the first
	// run stored an invalid entry, and from the second run on the key existed but was
	// empty - so the default no longer applied and the OpenGL viewport stayed off.
	settings->setValue(Constants::WORLD_EDITOR_USE_OPENGL,
					   settings->value(Constants::WORLD_EDITOR_USE_OPENGL, true));
	settings->setValue(Constants::VISIBLE_COLLISIONS, WorldEditor::isVisibleCollisions());
	settings->setValue(Constants::VISIBLE_LAND, m_ui.visibleLandAction->isChecked());
	settings->setValue(Constants::VISIBLE_ZONE_PRIMITIVES, m_ui.visibleZonePrimitivesAction->isChecked());
	settings->setValue(Constants::VISIBLE_PATH_PRIMITIVES, m_ui.visiblePathPrimitivesAction->isChecked());
	settings->setValue(Constants::VISIBLE_POINT_PRIMITIVES, m_ui.vidiblePointPrimitives->isChecked());
	settings->setValue(Constants::VISIBLE_DETAILS, m_ui.visibleDetailsAction->isChecked());
	settings->setValue(Constants::VISIBLE_GRID, m_ui.visibleGridAction->isChecked());
	settings->setValue(Constants::VISIBLE_GRID_POINTS, m_ui.visibleGridPointsAction->isChecked());
	settings->setValue(Constants::VISIBLE_PACS, m_visiblePacsAction->isChecked());
	settings->endGroup();
	settings->sync();
}

} /* namespace WorldEditor */
