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

#ifndef WORLD_EDITOR_WINDOW_H
#define WORLD_EDITOR_WINDOW_H

// Project includes
#include "ui_world_editor_window.h"

// Qt includes
#include <QtGui/QUndoStack>
#include <QtGui/QLabel>
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

class WorldEditorWindow: public QMainWindow
{
	Q_OBJECT

public:
	explicit WorldEditorWindow(QWidget *parent = 0);
	~WorldEditorWindow();

	QUndoStack *undoStack() const;
	void maybeSave();

Q_SIGNALS:
public Q_SLOTS:
	void open();

private Q_SLOTS:
	void newWorldEditFile();
	void saveWorldEditFile();
	void openProjectSettings();

	void setMode(int value);
	void updateStatusBar();

	void updateSelection(const QItemSelection &selected, const QItemSelection &deselected);
	void selectedItemsInScene(const QList<QGraphicsItem *> &selected);

protected:
	virtual void showEvent(QShowEvent *showEvent);
	virtual void hideEvent(QHideEvent *hideEvent);

private:
	void createMenus();
	void createToolBars();
	void readSettings();
	void writeSettings();

	void loadWorldEditFile(const QString &fileName);
	void checkCurrentWorld();

	QString m_context;
	QString m_dataDir;

	QString m_lastDir;

	QLabel *m_statusInfo;
	QTimer *m_statusBarTimer;

	PrimitivesTreeModel *m_primitivesModel;
	QUndoStack *m_undoStack;
	WorldEditorScene *m_worldEditorScene;
	LandscapeEditor::ZoneBuilderBase *m_zoneBuilderBase;
	QSignalMapper m_modeMapper;
	QGLWidget *m_oglWidget;
	Ui::WorldEditorWindow m_ui;
}; /* class WorldEditorWindow */

} /* namespace WorldEditor */

#endif // WORLD_EDITOR_WINDOW_H
