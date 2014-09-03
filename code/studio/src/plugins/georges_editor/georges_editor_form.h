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

class GeorgesDockWidget;

namespace GeorgesQt
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

	static QUndoStack *UndoStack;

public Q_SLOTS:
	void open();
	void loadFile(const QString &fileName);
    void loadFile(const QString &fileName, bool loadFromDfn);
	
	void newTyp();
	void newDfn();
	void newForm();

	void save();
	void settingsChanged();
	void closingTreeView();
	void setModified();

	void focusChanged(QWidget *old, QWidget *now);

private:
	void readSettings();
	void writeSettings();

	GeorgesDockWidget* loadTypDialog(const QString &fileName);
	GeorgesDockWidget* loadDfnDialog(const QString &fileName);
	GeorgesDockWidget* loadFormDialog(const QString &fileName, bool loadFromDFN );

	Ui::GeorgesEditorForm m_ui;

	CGeorgesDirTreeDialog *m_georgesDirTreeDialog;
	QToolBar *m_fileToolBar;
	QAction *m_openAction;
	QAction *m_newTypAction;
	QAction *m_newDfnAction;
	QAction *m_newFormAction;
	QAction *m_saveAction;

	QString m_leveldesignPath;

	QMainWindow *m_mainDock;
		
    /// Contains a list of all of the open forms.
	QList<GeorgesDockWidget*> m_dockedWidgets;

    /// Contains a pointer to the last known focal change for active documents.
	GeorgesDockWidget *m_lastActiveDock;

    /// Contains a record of the last directory a sheet file dialog was opened for.
    QString m_lastSheetDir;

}; /* class GeorgesEditorForm */

} /* namespace GeorgesQt */

#endif // GEORGES_EDITOR_FORM_H
