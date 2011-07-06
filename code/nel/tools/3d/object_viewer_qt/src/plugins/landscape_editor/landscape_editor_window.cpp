// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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
#include "landscape_editor_window.h"
#include "landscape_editor_constants.h"
#include "builder_zone.h"
#include "zone_region_editor.h"
#include "landscape_scene.h"
#include "project_settings_dialog.h"
#include "snapshot_dialog.h"

#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

// NeL includes
#include <nel/misc/debug.h>

// Qt includes
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace LandscapeEditor
{
QString _lastDir;

LandscapeEditorWindow::LandscapeEditorWindow(QWidget *parent)
	: QMainWindow(parent),
	  m_landscapeScene(0),
	  m_zoneBuilder(0),
	  m_undoStack(0),
	  m_oglWidget(0)
{
	m_ui.setupUi(this);

	m_undoStack = new QUndoStack(this);
	m_landscapeScene = new LandscapeScene(this);

	m_zoneBuilder = new ZoneBuilder(m_landscapeScene, m_ui.zoneListWidget, m_undoStack);
	m_zoneBuilder->init("e:/-nel-/install/continents/newbieland", true);
	m_ui.zoneListWidget->setZoneBuilder(m_zoneBuilder);
	m_ui.zoneListWidget->updateUi();

	m_landscapeScene->setZoneBuilder(m_zoneBuilder);
	m_ui.graphicsView->setScene(m_landscapeScene);
	//m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer));
	m_oglWidget = new QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::SampleBuffers));
	m_ui.graphicsView->setViewport(m_oglWidget);

	createMenus();
	createToolBars();
	readSettings();

	connect(m_ui.saveAction, SIGNAL(triggered()), this, SLOT(save()));
	connect(m_ui.projectSettingsAction, SIGNAL(triggered()), this, SLOT(openProjectSettings()));
	connect(m_ui.snapshotAction, SIGNAL(triggered()), this, SLOT(openSnapshotDialog()));
	connect(m_ui.enableGridAction, SIGNAL(toggled(bool)), m_ui.graphicsView, SLOT(setVisibleGrid(bool)));
}

LandscapeEditorWindow::~LandscapeEditorWindow()
{
	delete m_zoneBuilder;
	writeSettings();
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
			int id = m_zoneBuilder->createZoneRegion(fileName);
			if (id == -1)
			{
				QMessageBox::critical(this, "Landscape Editor", "Cannot add this zone because it overlaps existing ones");
				continue;
			}
			ZoneRegionObject *zoneRegion = m_zoneBuilder->zoneRegion(id);
			m_ui.graphicsView->centerOn(zoneRegion->ligoZoneRegion().getMinX() * m_landscapeScene->cellSize(),
										abs(zoneRegion->ligoZoneRegion().getMinY()) * m_landscapeScene->cellSize());

			m_zoneBuilder->setCurrentZoneRegion(id);
		}
	}
	setCursor(Qt::ArrowCursor);
}

void LandscapeEditorWindow::save()
{
	ZoneRegionObject *zoneRegion = m_zoneBuilder->currentZoneRegion();
	if (zoneRegion->fileName().empty())
	{
		QString fileName = QFileDialog::getSaveFileName(this,
						   tr("Save NeL Ligo land file"), _lastDir,
						   tr("NeL Ligo land file (*.land)"));
		if (!fileName.isEmpty())
			zoneRegion->setFileName(fileName.toStdString());
	}
	zoneRegion->save();
}

void LandscapeEditorWindow::openProjectSettings()
{
	ProjectSettingsDialog *dialog = new ProjectSettingsDialog(m_zoneBuilder->dataPath(), this);
	dialog->show();
	int ok = dialog->exec();
	if (ok == QDialog::Accepted)
	{
		m_zoneBuilder->init(dialog->dataPath(), false);
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

void LandscapeEditorWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	if (m_oglWidget != 0)
		m_oglWidget->makeCurrent();
}

void LandscapeEditorWindow::createMenus()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
}

void LandscapeEditorWindow::createToolBars()
{
	Core::IMenuManager *menuManager = Core::ICore::instance()->menuManager();
	//QAction *action = menuManager->action(Core::Constants::NEW);
	//m_ui.fileToolBar->addAction(action);
	QAction *action = menuManager->action(Core::Constants::OPEN);
	m_ui.fileToolBar->addAction(action);
	//action = menuManager->action(Core::Constants::SAVE);
	//m_ui.fileToolBar->addAction(action);
	//action = menuManager->action(Core::Constants::SAVE_AS);
	//m_ui.fileToolBar->addAction(action);
}

void LandscapeEditorWindow::readSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	restoreState(settings->value(Constants::LANDSCAPE_WINDOW_STATE).toByteArray());
	restoreGeometry(settings->value(Constants::LANDSCAPE_WINDOW_GEOMETRY).toByteArray());
	settings->endGroup();
}

void LandscapeEditorWindow::writeSettings()
{
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Constants::LANDSCAPE_EDITOR_SECTION);
	settings->setValue(Constants::LANDSCAPE_WINDOW_STATE, saveState());
	settings->setValue(Constants::LANDSCAPE_WINDOW_GEOMETRY, saveGeometry());
	settings->endGroup();
	settings->sync();
}

} /* namespace LandscapeEditor */
