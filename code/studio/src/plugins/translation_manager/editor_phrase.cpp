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

// Project includes
#include "editor_phrase.h"
#include "translation_manager_constants.h"

// Nel includes
#include "nel/misc/path.h"
#include "nel/misc/diff_tool.h"

// Qt includes
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtGui/QTextCursor>
#include <QtGui/QErrorMessage>
#include <QtGui/QMessageBox>
#include <QtGui/QCloseEvent>

using namespace std;

namespace TranslationManager
{

void CEditorPhrase::open(QString filename)
{
	std::vector<STRING_MANAGER::TPhrase> phrases;
	if(readPhraseFile(filename.toUtf8().constData(), phrases, false))
	{
		text_edit = new CTextEdit(this);
		text_edit->setUndoStack(current_stack);
		SyntaxHighlighter *highlighter = new SyntaxHighlighter(text_edit);
		text_edit->setUndoRedoEnabled(true);
		text_edit->document()->setUndoRedoEnabled(true);
		setWidget(text_edit);
		// read the file content
		QFile file(filename);
		file.open(QIODevice::ReadOnly | QIODevice::Text);
		QTextStream in(&file);
		// set the file content to the text edit
		QString data = in.readAll();
		text_edit->append(data);
		// window settings
		setCurrentFile(filename);
		setAttribute(Qt::WA_DeleteOnClose);
		editor_type = Constants::ED_PHRASE;
		current_file = filename;
		connect(text_edit->document(), SIGNAL(contentsChanged()), this, SLOT(docContentsChanged()));
		connect(text_edit->document(), SIGNAL(undoCommandAdded()), this, SLOT(newUndoCommandAdded()));
	}
	else
	{
		QErrorMessage error;
		error.showMessage("This file is not a phrase file.");
		error.exec();
	}
}

void CEditorPhrase::newUndoCommandAdded()
{
	current_stack->push(new CUndoPhraseNewCommand(text_edit));
}

void CEditorPhrase::docContentsChanged()
{
	setWindowModified(true);
}

void CEditorPhrase::activateWindow()
{
	showMaximized();
}

void CEditorPhrase::save()
{
	saveAs(current_file);
}

void CEditorPhrase::saveAs(QString filename)
{
	QFile file(filename);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out.setGenerateByteOrderMark(true);
	out << text_edit->toPlainText();
	current_file = filename;
	setCurrentFile(current_file);
}

void CEditorPhrase::closeEvent(QCloseEvent *event)
{
	if(isWindowModified())
	{
		QMessageBox msgBox;
		msgBox.setIcon(QMessageBox::Question);
		msgBox.setText(tr("The document has been modified."));
		msgBox.setInformativeText(tr("Do you want to save your changes?"));
		msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Save);

		int ret = msgBox.exec();
		switch (ret)
		{
		case QMessageBox::Save:
			save();
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			event->ignore();
			return;
		}
	}
	event->accept();
	close();
}

}