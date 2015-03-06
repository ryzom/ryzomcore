// Translation Manager Plugin - OVQT Plugin <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2011  Emanuel Costea <cemycc@gmail.com>
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

#ifndef TRANSLATION_MANAGER_EDITOR_H
#define TRANSLATION_MANAGER_EDITOR_H

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QMdiArea>
#include <QtGui/QMdiSubWindow>
#include <QtGui/QUndoStack>
#include <QtCore/QFileInfo>

namespace TranslationManager
{

class CEditor : public QMdiSubWindow
{
	Q_OBJECT

public:
	CEditor(QMdiArea *parent) : QMdiSubWindow(parent) { current_stack = NULL; }
	CEditor() : QMdiSubWindow() { current_stack = NULL; }
	virtual void open(QString filename) =0;
	virtual void save() =0;
	virtual void saveAs(QString filename) =0;
	virtual void activateWindow() =0;

	int eType()
	{
		return editor_type;
	}
	QString subWindowFilePath()
	{
		return current_file;
	}
	void setUndoStack(QUndoStack *stack)
	{
		current_stack = stack;
	}
	void setCurrentFile(QString filename)
	{
		QFileInfo *file = new QFileInfo(filename);
		current_file = file->canonicalFilePath();
		setWindowModified(false);
		setWindowTitle(file->fileName() + "[*]");
		setWindowFilePath(current_file);
	}

protected:
	QUndoStack *current_stack;
	QString current_file;
	int editor_type;
};

}

#endif	/* TRANSLATION_MANAGER_EDITOR_H */