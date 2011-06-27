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

#ifndef LANDSCAPE_EDITOR_WINDOW_H
#define LANDSCAPE_EDITOR_WINDOW_H

// Project includes
#include "ui_landscape_editor_window.h"

// Qt includes
#include <QtGui/QUndoStack>

namespace LandscapeEditor
{

class LandscapeScene;
class ZoneBuilder;

class LandscapeEditorWindow: public QMainWindow
{
	Q_OBJECT

public:
	LandscapeEditorWindow(QWidget *parent = 0);
	~LandscapeEditorWindow();

	QUndoStack *undoStack() const;

Q_SIGNALS:
public Q_SLOTS:
	void open();

private Q_SLOTS:
	void openProjectSettings();
	void openSnapshotDialog();

private:
	void createMenus();
	void createToolBars();
	void readSettings();
	void writeSettings();

	LandscapeScene *m_landscapeScene;
	ZoneBuilder *m_zoneBuilder;
	QUndoStack *m_undoStack;
	Ui::LandscapeEditorWindow m_ui;
}; /* class LandscapeEditorWindow */

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_EDITOR_WINDOW_H
