// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry KAMIAHIN (dnk-88) <dnk-88@tut.by>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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

#ifndef WORLD_EDITOR_WINDOW_H
#define WORLD_EDITOR_WINDOW_H

// Project includes
#include "ui_world_editor_window.h"
#include "edit_history.h"

// Qt includes
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPlainTextEdit>
#include <QtCore/QTimer>
#include <QtCore/QSignalMapper>
#include <QtOpenGL/QGLWidget>

namespace LandscapeEditor
{
class ZoneBuilderBase;
}

namespace WorldEditor
{
class PrimitivesTreeModel;
class WorldEditorScene;
class PacsOverlay;

class WorldEditorWindow: public QMainWindow
{
	Q_OBJECT

public:
	explicit WorldEditorWindow(QWidget *parent = 0);
	~WorldEditorWindow();

	QUndoStack *undoStack() const;
    void onActivated();
	void maybeSave();

Q_SIGNALS:
public Q_SLOTS:
	void open();

private Q_SLOTS:
	/// Move the view onto the loaded landscape, so it is not left on empty grid.
	void focusOnLandscape();

	/// Show or hide the collision footprints the sheets declare.
	void setVisibleCollisions(bool visible);

	/// Show or hide the PACS borders of the continent, loading them on first use.
	void setVisiblePacs(bool visible);

	/// Load the PACS borders that belong to what is open now, if they are shown.
	void updatePacs();

	/// The show/hide switches in the tool bar. Every one of these was present in the
	/// user interface but connected to nothing.
	void setVisibleLand(bool visible);
	void setVisibleZonePrimitives(bool visible);
	void setVisiblePathPrimitives(bool visible);
	void setVisiblePointPrimitives(bool visible);
	void setVisibleDetails(bool visible);
	void setVisibleGridPoints(bool visible);

	/// Right click on the map: offer the same menu the tree offers for that primitive.
	void showSceneContextMenu(QGraphicsItem *item, const QPoint &globalPos);

	/// Move the view onto an area of the map, picked in the tree.
	void zoomToRect(const QRectF &sceneRect);

	/// Write what the undo stack just did into the project's journal.
	void recordUndoChange(int index);

	void showHistory(const QStringList &entries, const QString &projectFile);
	void appendHistory(const QString &line);

	void updatePanelsMenu();

	void newWorldEditFile();
	void saveWorldEditFile();
	void saveWorldEditFileAs();
	void openProjectSettings();

	void setMode(int value);
	void updateStatusBar();

	void updateSelection(const QItemSelection &selected, const QItemSelection &deselected);
	void selectedItemsInScene(const QList<QGraphicsItem *> &selected);

protected:
	virtual void showEvent(QShowEvent *showEvent);
	virtual void hideEvent(QHideEvent *hideEvent);

private:
	QMenu *m_panelsMenu;
	QAction *m_saveAsAction;

	void createMenus();
	void createToolBars();
	void readSettings();
	void writeSettings();

	/// The dock and tool bar arrangement world_editor_window.ui sets up, kept so an
	/// unusable stored state can be rolled back to it.
	typedef QList<QPair<QDockWidget *, Qt::DockWidgetArea> > DockDefaults;
	typedef QList<QPair<QToolBar *, Qt::ToolBarArea> > ToolBarDefaults;

	/// Undo a stored state that hides every dock and tool bar of this window.
	void ensurePanelsUsable();
	void createHistoryPanel();

	/// Restore one show/hide switch and apply it to the scene.
	void restoreSwitch(QAction *action, const char *settingsKey, bool defaultValue);

	QAction *m_visibleCollisionsAction;

	/// The arrangement as it was when the window last went out of view, see writeSettings().
	QByteArray m_savedWindowState;
	QByteArray m_savedWindowGeometry;
	QAction *m_visiblePacsAction;

	/// Colour key for the PACS borders, shown in the tool bar while they are.
	QWidget *createPacsLegend();
	QAction *m_pacsLegendAction;

	/// Where the PACS of the open landscape are: pacs/ next to the first loaded .land,
	/// else pacs/ in the data directory, as the MFC editor had it. Empty if neither exists.
	QString pacsDirectory() const;
	PacsOverlay *m_pacsOverlay;

	/// Journal of what was done to the project, kept across sessions.
	EditHistory *m_history;
	QPlainTextEdit *m_historyView;
	QDockWidget *m_historyDock;
	int m_lastUndoIndex;
	int m_lastUndoCount;

	/// Bounding rectangle of every loaded zone region, in scene coordinates.
	QRectF landscapeBounds() const;

	void loadWorldEditFile(const QString &fileName);
	bool checkCurrentWorld();

	QString m_context;
	QString m_dataDir;


	QLabel *m_statusInfo;
	QTimer *m_statusBarTimer;

	PrimitivesTreeModel *m_primitivesModel;
	QUndoStack *m_undoStack;
	DockDefaults m_dockDefaults;
	ToolBarDefaults m_toolBarDefaults;
	bool m_panelsChecked;

	WorldEditorScene *m_worldEditorScene;
	LandscapeEditor::ZoneBuilderBase *m_zoneBuilderBase;
	QSignalMapper m_modeMapper;
	QGLWidget *m_oglWidget;
	Ui::WorldEditorWindow m_ui;
}; /* class WorldEditorWindow */

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_WINDOW_H
