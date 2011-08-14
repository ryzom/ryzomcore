// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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

#ifndef GEORGES_EDITOR_FORM_H
#define GEORGES_EDITOR_FORM_H

// Project includes
#include "ui_georges_editor_form.h"

// Qt includes
#include <QtGui/QUndoStack>

namespace Plugin
{

class CGeorgesDirTreeDialog;
class CGeorgesTreeViewDialog;
class GeorgesEditorForm: public QMainWindow
{
	Q_OBJECT

public:
	GeorgesEditorForm(QWidget *parent = 0);
	~GeorgesEditorForm();

	QUndoStack *undoStack() const;

public Q_SLOTS:
	void open();
	void newFile();
	void save();
	void settingsChanged();
	void loadFile(const QString fileName);

private:
	void readSettings();
	void writeSettings();

	QUndoStack *m_undoStack;
	Ui::GeorgesEditorForm m_ui;

	CGeorgesDirTreeDialog *m_georgesDirTreeDialog;
	QToolBar *_fileToolBar;
	QAction *_openAction;
	QAction *_newAction;
	QAction *_saveAction;

	QString m_leveldesignPath;

	QDockWidget *m_emptyDock;
	QMainWindow *m_mainDock;
		
	QList<CGeorgesTreeViewDialog*> m_dockedWidgets;
	QList<QTabBar*> m_tabBars;
}; /* class GeorgesEditorForm */

} /* namespace Plugin */

#endif // GEORGES_EDITOR_FORM_H
