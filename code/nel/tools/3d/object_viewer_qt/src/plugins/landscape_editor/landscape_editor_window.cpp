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
#include "landscape_editor_window.h"
#include "landscape_editor_constants.h"
#include "builder_zone.h"
#include "zone_region_editor.h"
#include "landscape_scene.h"
#include "project_settings_dialog.h"
#include "snapshot_dialog.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QStatusBar>

namespace LandscapeEditor
{

static const int LANDSCAPE_ID = 32;
int NewLandCounter = 0;
QString _lastDir;

LandscapeEditorWindow::LandscapeEditorWindow(QWidget *parent)
	: QMainWindow(parent),
	  m_currentItem(0),
	  m_landscapeScene(0),
	  m_zoneBuilder(0),
	  m_undoStack(0),
	  m_oglWidget(0)
{
	m_ui.setupUi(this);

	m_undoStack = new QUndoStack(this);
	m_landscapeScene = new LandscapeScene(160, this);

	m_zoneBuilder = new ZoneBuilder(m_landscapeScene, m_ui.zoneListWidget, m_undoStack);
	m_ui.zoneListWidget->setZoneBuilder(m_zoneBuilder);
	m_ui.zoneListWidget->updateUi();

	m_landscapeScene->setZoneBuilder(m_zoneBuilder);
	m_ui.graphicsView->setScene(m_landscapeScene);

	m_ui.newLandAction->setIcon(QIcon(Core::Constants::ICON_NEW));
	m_ui.saveAction->setIcon(QIcon(Core::Constants::ICON_SAVE));
	m_ui.saveLandAction->setIcon(QIcon(Core::Constants::ICON_SAVE));
	m_ui.saveAsLandAction->setIcon(QIcon(Core::Constants::ICON_SAVE_AS));
	m_ui.zonesDockWidget->toggleViewAction()->setIcon(QIcon(Constants::ICON_LANDSCAPE_ZONES));
	m_ui.landscapesDockWidget->toggleViewAction()->setIcon(QIcon(Constants::ICON_ZONE_ITEM));

	m_ui.deleteLandAction->setEnabled(false);

	createMenus();
	createToolBars();
	readSettings();

	connect(m_ui.saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(m_ui.projectSettingsAction, SIGNAL(triggered()), this, SLOT(openProjectSettings()));
	connect(m_ui.snapshotAction, SIGNAL(triggered()), this, SLOT(openSnapshotDialog()));
	connect(m_ui.enableGridAction, SIGNAL(toggled(bool)), m_ui.graphicsView, SLOT(setVisibleGrid(bool)));

	connect(m_ui.newLandAction, SIGNAL(triggered()), this, SLOT(newLand()));
	connect(m_ui.setActiveLandAction, SIGNAL(triggered()), this, SLOT(setActiveLand()));
	connect(m_ui.saveLandAction, SIGNAL(triggered()), this, SLOT(saveSelectedLand()));
	connect(m_ui.saveAsLandAction, SIGNAL(triggered()), this, SLOT(saveAsSelectedLand()));
	connect(m_ui.deleteLandAction, SIGNAL(triggered()), this, SLOT(deleteSelectedLand()));
	connect(m_ui.transitionModeAction, SIGNAL(toggled(bool)), m_landscapeScene, SLOT(setTransitionMode(bool)));

	connect(m_ui.landscapesListWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenu()));
	m_ui.landscapesListWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	m_statusBarTimer = new QTimer(this);
	connect(m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));

	m_statusInfo = new QLabel(this);
	m_statusInfo->hide();
	Core::ICore::instance()->mainWindow()->statusBar()->addPermanentWidget(m_statusInfo);
}

LandscapeEditorWindow::~LandscapeEditorWindow()
{
	writeSettings();
	delete m_zoneBuilder;
}

QUndoStack *LandscapeEditorWindow::undoStack() const
{
	return m_undoStack;
}

void LandscapeEditorWindow::open()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(this,
							tr("Open NeL Ligo land file"), _lastDir,
							tr("All NeL Ligo land files (*.land)"));

	setCursor(Qt::WaitCursor);
	if (!fileNames.isEmpty())
	{
		QStringList list = fileNames;
		_lastDir = QFileInfo(list.front()).absolutePath();
		Q_FOREACH(QString fileName, fileNames)
		{
			int row = createLandscape(fileName);
			if (row != -1)
				setActiveLandscape(row);
		}
	}
	setCursor(Qt::ArrowCursor);
}

void LandscapeEditorWindow::save()
{
	saveLandscape(m_ui.landscapesListWidget->row(m_currentItem), true);
}

void LandscapeEditorWindow::openProjectSettings()
{
	ProjectSettingsDialog *dialog = new ProjectSettingsDialog(m_zoneBuilder->dataPath(), this);
	dialog->show();
	int ok = dialog->exec();
	if (ok == QDialog::Accepted)
	{
		m_zoneBuilder->init(dialog->dataPath(), true);
		m_ui.zoneListWidget->updateUi();
	}
	delete dialog;
}

void LandscapeEditorWindow::openSnapshotDialog()
{
	SnapshotDialog *dialog = new SnapshotDialog(this);
	dialog->show();
	int ok = dialog->exec();
	if (ok == QDialog::Accepted)
	{
		QString fileName = QFileDialog::getSaveFileName(this,
						   tr("Save screenshot landscape"), _lastDir,
						   tr("Image file (*.png)"));

		setCursor(Qt::WaitCursor);

		NLLIGO::CZoneRegion &zoneRegion = m_zoneBuilder->currentZoneRegion()->ligoZoneRegion();
		sint32 regionMinX = zoneRegion.getMinX();
		sint32 regionMaxX = zoneRegion.getMaxX();
		sint32 regionMinY = zoneRegion.getMinY();
		sint32 regionMaxY = zoneRegion.getMaxY();

		int regionWidth = (regionMaxX - regionMinX + 1);
		int regionHeight = (regionMaxY - regionMinY + 1);

		int cellSize = m_landscapeScene->cellSize();
		QRectF rect(regionMinX * cellSize, abs(regionMaxY) * cellSize, regionWidth * cellSize, regionHeight * cellSize);

		if (dialog->isCustomSize())
		{
			int widthSnapshot = dialog->widthSnapshot();
			int heightSnapshot = dialog->heightSnapshot();
			if (dialog->isKeepRatio())
				heightSnapshot = (widthSnapshot / regionWidth) * regionHeight;

			m_landscapeScene->snapshot(fileName, widthSnapshot, heightSnapshot, rect);
		}
		else
		{
			m_landscapeScene->snapshot(fileName, regionWidth * dialog->resolutionZone(),
									   regionHeight * dialog->resolutionZone(), rect);
		}
		setCursor(Qt::ArrowCursor);
	}
	delete dialog;
}

void LandscapeEditorWindow::customContextMenu()
{
	if (m_ui.landscapesListWidget->currentRow() == -1)
		return;
	QMenu *popurMenu = new QMenu(this);
	popurMenu->addAction(m_ui.setActiveLandAction);
	popurMenu->addAction(m_ui.saveLandAction);
	popurMenu->addAction(m_ui.saveAsLandAction);
	popurMenu->addAction(m_ui.deleteLandAction);
	popurMenu->exec(QCursor::pos());
	delete popurMenu;
}

void LandscapeEditorWindow::newLand()
{
	int row = createLandscape(QString());
	if (row != -1)
		setActiveLandscape(row);
}

void LandscapeEditorWindow::setActiveLand()
{
	setActiveLandscape(m_ui.landscapesListWidget->currentRow());
}

void LandscapeEditorWindow::saveSelectedLand()
{
	saveLandscape(m_ui.landscapesListWidget->currentRow(), true);
}

void LandscapeEditorWindow::saveAsSelectedLand()
{
	saveLandscape(m_ui.landscapesListWidget->currentRow(), false);
}

void LandscapeEditorWindow::deleteSelectedLand()
{
	int row = m_ui.landscapesListWidget->currentRow();
	int current_row = m_ui.landscapesListWidget->row(m_currentItem);
	QListWidgetItem *item = m_ui.landscapesListWidget->item(row);
	if (row == current_row)
	{
		if (row == 0)
			++row;
		else
			--row;
		setActiveLandscape(row);
	}
	m_zoneBuilder->deleteZoneRegion(item->data(LANDSCAPE_ID).toInt());
	m_ui.landscapesListWidget->removeItemWidget(item);
	delete item;

	if (m_ui.landscapesListWidget->count() == 1)
		m_ui.deleteLandAction->setEnabled(false);

	m_undoStack->clear();
}

int LandscapeEditorWindow::createLandscape(const QString &fileName)
{
	int id;
	if (fileName.isEmpty())
		id = m_zoneBuilder->createZoneRegion();
	else
		id = m_zoneBuilder->createZoneRegion(fileName);

	if (id == -1)
	{
		QMessageBox::critical(this, "Landscape Editor", tr("Cannot add this zone because it overlaps existing ones"));
		return -1;
	}
	ZoneRegionObject *zoneRegion = m_zoneBuilder->zoneRegion(id);
	m_ui.graphicsView->setCenter(QPointF(zoneRegion->ligoZoneRegion().getMinX() * m_landscapeScene->cellSize(),
										 abs(zoneRegion->ligoZoneRegion().getMinY()) * m_landscapeScene->cellSize()));

	QListWidgetItem *item;
	if (fileName.isEmpty())
		item = new QListWidgetItem(QString("NewLandscape%1").arg(NewLandCounter++), m_ui.landscapesListWidget);
	else
		item = new QListWidgetItem(fileName, m_ui.landscapesListWidget);

	item->setData(LANDSCAPE_ID, id);
	item->setFont(QFont("SansSerif", 9, QFont::Normal));

	if (m_ui.landscapesListWidget->count() > 1)
		m_ui.deleteLandAction->setEnabled(true);

	return m_ui.landscapesListWidget->count() - 1;
}

void LandscapeEditorWindow::setActiveLandscape(int row)
{
	if ((0 <= row) && (row < m_ui.landscapesListWidget->count()))
	{
		if (m_currentItem != 0)
			m_currentItem->setFont(QFont("SansSerif", 9, QFont::Normal));

		QListWidgetItem *item = m_ui.landscapesListWidget->item(row);
		item->setFont(QFont("SansSerif", 9, QFont::Bold));
		m_zoneBuilder->setCurrentZoneRegion(item->data(LANDSCAPE_ID).toInt());
		m_currentItem = item;
	}
}

void LandscapeEditorWindow::saveLandscape(int row, bool force)
{
	if ((0 <= row) && (row < m_ui.landscapesListWidget->count()))
	{
		QListWidgetItem *item = m_ui.landscapesListWidget->item(row);
		ZoneRegionObject *regionObject = m_zoneBuilder->zoneRegion(item->data(LANDSCAPE_ID).toInt());
		if ((!force) || (regionObject->fileName().empty()))
		{
			QString fileName = QFileDialog::getSaveFileName(this,
							   tr("Save NeL Ligo land file"), _lastDir,
							   tr("NeL Ligo land file (*.land)"));
			if (!fileName.isEmpty())
			{
				regionObject->setFileName(fileName.toStdString());
				regionObject->save();
				regionObject->setModified(false);
				item->setText(fileName);
			}
		}
		else
		{
			regionObject->save();
			regionObject->setModified(false);
		}
	}
}

void LandscapeEditorWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	if (m_oglWidget != 0)
		m_oglWidget->makeCurrent();
	m_statusInfo->show();
	m_statusBarTimer->start(100);
}

void LandscapeEditorWindow::hideEvent(QHideEvent *hideEvent)
{
	QMainWindow::hideEvent(hideEvent);
	m_statusInfo->hide();
	m_statusBarTimer->stop();
}

void LandscapeEditorWindow::updateStatusBar()
{
	m_statusInfo->setText(m_landscapeScene->zoneNameFromMousePos());
}

void LandscapeEditorWindow::createMenus()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void LandscapeEditorWindow::createToolBars()
{
	Core::MenuManager *menuManager = Core::ICore::instance()->menuManager();
	//QAction *action = menuManager->action(Core::Constants::NEW);
	//m_ui.fileToolBar->addAction(action);
	//action = menuManager->action(Core::Constants::SAVE);
	//m_ui.fileToolBar->addAction(action);
	//action = menuManager->action(Core::Constants::SAVE_AS);
	//m_ui.fileToolBar->addAction(action);

	QAction *action = menuManager->action(Core::Constants::OPEN);
	m_ui.fileToolBar->addAction(m_ui.newLandAction);
	m_ui.fileToolBar->addAction(action);
	m_ui.fileToolBar->addAction(m_ui.saveAction);
	m_ui.fileToolBar->addSeparator();

	action = menuManager->action(Core::Constants::UNDO);
	if (action != 0)
		m_ui.fileToolBar->addAction(action);

	action = menuManager->action(Core::Constants::REDO);
	if (action != 0)
		m_ui.fileToolBar->addAction(action);

	m_ui.zoneToolBar->insertAction(m_ui.enableGridAction, m_ui.landscapesDockWidget->toggleViewAction());
	m_ui.zoneToolBar->insertAction(m_ui.enableGridAction, m_ui.zonesDockWidget->toggleViewAction());
}

void LandscapeEditorWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	restoreState(settings->value(Constants::LANDSCAPE_WINDOW_STATE).toByteArray());
	restoreGeometry(settings->value(Constants::LANDSCAPE_WINDOW_GEOMETRY).toByteArray());

	// Read landscape data directory (contains sub-paths: zone logos, zone bitmaps)
	m_zoneBuilder->init(settings->value(Constants::LANDSCAPE_DATA_DIRECTORY).toString());
	m_ui.zoneListWidget->updateUi();

	// Use OpenGL graphics system instead raster graphics system
	if (settings->value(Constants::LANDSCAPE_USE_OPENGL, false).toBool())
	{
		m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
		m_ui.graphicsView->setViewport(m_oglWidget);
	}

	settings->endGroup();
}

void LandscapeEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	settings->setValue(Constants::LANDSCAPE_WINDOW_STATE, saveState());
	settings->setValue(Constants::LANDSCAPE_WINDOW_GEOMETRY, saveGeometry());
	settings->setValue(Constants::LANDSCAPE_DATA_DIRECTORY, m_zoneBuilder->dataPath());
	settings->endGroup();
	settings->sync();
}

} /* namespace LandscapeEditor */
