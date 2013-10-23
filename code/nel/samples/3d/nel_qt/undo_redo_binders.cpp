// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include <nel/misc/types_nl.h>
#include "undo_redo_binders.h"

// STL includes

// Qt includes
#include <QtGui/QUndoStack>
#include <QtGui/QAbstractButton>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>

// NeL includes
#include <nel/misc/debug.h>

// Project includes
#include "qtcolorpicker.h"

// using namespace std;
// using namespace NLMISC;

namespace NLQT {

namespace {

int a_UndoCommandId = 9000;

} /* anonymous namespace */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoBinderButton::CUndoRedoBinderButton(QAbstractButton *abstractButton, QUndoStack *undoStack)
	: QObject(abstractButton), m_AbstractButton(abstractButton), m_UndoStack(undoStack)
{
	nlassert(m_AbstractButton);
	bool c;
	m_Checked = m_AbstractButton->isChecked();
	c = connect(m_AbstractButton, SIGNAL(toggled(bool)), this, SLOT(abstractButtonToggled(bool)));
	nlassertex(c, ("connect toggled(bool)"));
	m_Enabled = true;
}

CUndoRedoBinderButton::~CUndoRedoBinderButton()
{
	m_UndoStack->clear(); // may contain commands on a deleted button
}

void CUndoRedoBinderButton::abstractButtonToggled(bool checked)
{
	if (m_Enabled)
	{
		if (checked != m_Checked)
		{
			bool undo = m_Checked;
			/* bool redo = checked; */
			m_Checked = checked; /* redo; */
			m_UndoStack->push(new CUndoRedoCommandButton(this, m_AbstractButton, undo));
		}
	}
	else
	{
		m_Checked = checked;
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoCommandButton::CUndoRedoCommandButton(CUndoRedoBinderButton *binder, QAbstractButton *abstractButton, bool undo/*, bool redo*/)
	: QUndoCommand(), m_Binder(binder), m_AbstractButton(abstractButton), m_Undo(undo)/*, m_Redo(redo)*/
{
	// nldebug("CUndoRedoCommandButton::CUndoRedoCommandButton()");
	nlassert(m_AbstractButton);	
}

CUndoRedoCommandButton::~CUndoRedoCommandButton()
{
	
}

void CUndoRedoCommandButton::undo()
{
	// nldebug("CUndoRedoCommandButton::undo()");
	// nlassert(m_AbstractButton);

	m_Binder->enable(false);
	if (m_AbstractButton->isChecked() != m_Undo)
		m_AbstractButton->setChecked(m_Undo);
	m_Binder->enable(true);
}

void CUndoRedoCommandButton::redo()
{
	// nldebug("CUndoRedoCommandButton::redo()");
	// nlassert(m_AbstractButton);

	m_Binder->enable(false);
	if (m_AbstractButton->isChecked() == m_Undo) /* != m_Redo) */
		m_AbstractButton->setChecked(!m_Undo); /* (m_Redo); */
	m_Binder->enable(true);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoBinderLineEdit::CUndoRedoBinderLineEdit(QLineEdit *lineEdit, QUndoStack *undoStack)
	: QObject(lineEdit), m_LineEdit(lineEdit), m_UndoStack(undoStack)
{
	nlassert(m_LineEdit);
	m_Id = ++a_UndoCommandId;
	bool c;
	m_LastValue = m_LineEdit->text();
	c = connect(m_LineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditTextChanged(const QString &)));
	nlassertex(c, ("connect textChanged(const QString &)"));
	m_Enabled = true;
}

CUndoRedoBinderLineEdit::~CUndoRedoBinderLineEdit()
{
	m_UndoStack->clear(); // may contain commands on a deleted LineEdit
}

void CUndoRedoBinderLineEdit::lineEditTextChanged(const QString &text)
{
	if (m_LineEdit->isRedoAvailable())
	{
		// workaround internal undo redo of lineedit
		m_LineEdit->redo();
		m_UndoStack->undo();
		return;
	}

	if (m_Enabled)
	{
		if (text != m_LastValue)
		{
			QString undo = m_LastValue;
			const QString &redo = text;
			m_LastValue = redo;
			m_UndoStack->push(new CUndoRedoCommandLineEdit(this, m_LineEdit, undo, redo, m_Id));
		}
	}
	else
	{
		m_LastValue = text;
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoCommandLineEdit::CUndoRedoCommandLineEdit(CUndoRedoBinderLineEdit *binder, QLineEdit *lineEdit, const QString &undo, const QString &redo, int id)
	: QUndoCommand(), m_Binder(binder), m_LineEdit(lineEdit), m_Undo(undo), m_Redo(redo), m_Id(id)
{
	// nldebug("CUndoRedoCommandLineEdit::CUndoRedoCommandLineEdit()");
	nlassert(m_LineEdit);
}

CUndoRedoCommandLineEdit::~CUndoRedoCommandLineEdit()
{
	
}

bool CUndoRedoCommandLineEdit::mergeWith(const QUndoCommand *other)
{
	if (m_Id != other->id()) return false;
	m_Redo = static_cast<const CUndoRedoCommandLineEdit *>(other)->m_Redo;
	return true;
}

void CUndoRedoCommandLineEdit::undo()
{
	// nldebug("CUndoRedoCommandLineEdit::undo()");
	// nlassert(m_LineEdit);
	
	m_Binder->enable(false);
	if (m_LineEdit->text() != m_Undo)
		m_LineEdit->setText(m_Undo);
	m_Binder->enable(true);
}

void CUndoRedoCommandLineEdit::redo()
{
	// nldebug("CUndoRedoCommandLineEdit::redo()");
	// nlassert(m_LineEdit);

	m_Binder->enable(false);
	if (m_LineEdit->text() != m_Redo)
		m_LineEdit->setText(m_Redo);
	m_Binder->enable(true);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoBinderComboBox::CUndoRedoBinderComboBox(QComboBox *comboBox, QUndoStack *undoStack)
	: QObject(comboBox), m_ComboBox(comboBox), m_UndoStack(undoStack)
{
	nlassert(m_ComboBox);
	bool c;
	m_LastValue = m_ComboBox->currentIndex();
	c = connect(m_ComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxCurrentIndexChanged(int)));
	nlassertex(c, ("connect currentIndexChanged(int)"));
	m_Enabled = true;
}

CUndoRedoBinderComboBox::~CUndoRedoBinderComboBox()
{
	m_UndoStack->clear(); // may contain commands on a deleted ComboBox
}

void CUndoRedoBinderComboBox::comboBoxCurrentIndexChanged(int index)
{
	if (m_Enabled)
	{
		if (index != m_LastValue)
		{
			int undo = m_LastValue;
			int redo = index;
			m_LastValue = redo;
			m_UndoStack->push(new CUndoRedoCommandComboBox(this, m_ComboBox, undo, redo));
		}
	}
	else
	{
		m_LastValue = index;
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoCommandComboBox::CUndoRedoCommandComboBox(CUndoRedoBinderComboBox *binder, QComboBox *comboBox, int undo, int redo)
	: QUndoCommand(), m_Binder(binder), m_ComboBox(comboBox), m_Undo(undo), m_Redo(redo)
{
	// nldebug("CUndoRedoCommandComboBox::CUndoRedoCommandComboBox()");
	nlassert(m_ComboBox);
}

CUndoRedoCommandComboBox::~CUndoRedoCommandComboBox()
{
	
}

void CUndoRedoCommandComboBox::undo()
{
	// nldebug("CUndoRedoCommandComboBox::undo()");
	// nlassert(m_ComboBox);
	
	m_Binder->enable(false);
	if (m_ComboBox->currentIndex() != m_Undo)
		m_ComboBox->setCurrentIndex(m_Undo);
	m_Binder->enable(true);
}

void CUndoRedoCommandComboBox::redo()
{
	// nldebug("CUndoRedoCommandComboBox::redo()");
	// nlassert(m_ComboBox);

	m_Binder->enable(false);
	if (m_ComboBox->currentIndex() != m_Redo)
		m_ComboBox->setCurrentIndex(m_Redo);
	m_Binder->enable(true);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoBinderColorPicker::CUndoRedoBinderColorPicker(QtColorPicker *colorPicker, QUndoStack *undoStack)
	: QObject(colorPicker), m_ColorPicker(colorPicker), m_UndoStack(undoStack)
{
	nlassert(m_ColorPicker);
	bool c;
	m_LastValue = m_ColorPicker->currentColor();
	c = connect(m_ColorPicker, SIGNAL(colorChanged(const QColor &)), this, SLOT(colorPickerColorChanged(const QColor &)));
	nlassertex(c, ("connect colorChanged(const QColor &)"));
	m_Enabled = true;
}

CUndoRedoBinderColorPicker::~CUndoRedoBinderColorPicker()
{
	m_UndoStack->clear(); // may contain commands on a deleted ColorPicker
}

void CUndoRedoBinderColorPicker::colorPickerColorChanged(const QColor &col)
{
	if (m_Enabled)
	{
		if (col != m_LastValue)
		{
			QColor undo = m_LastValue;
			const QColor &redo = col;
			m_LastValue = redo;
			m_UndoStack->push(new CUndoRedoCommandColorPicker(this, m_ColorPicker, undo, redo));
		}
	}
	else
	{
		m_LastValue = col;
	}
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

CUndoRedoCommandColorPicker::CUndoRedoCommandColorPicker(CUndoRedoBinderColorPicker *binder, QtColorPicker *colorPicker, const QColor &undo, const QColor &redo)
	: QUndoCommand(), m_Binder(binder), m_ColorPicker(colorPicker), m_Undo(undo), m_Redo(redo)
{
	// nldebug("CUndoRedoCommandColorPicker::CUndoRedoCommandColorPicker()");
	nlassert(m_ColorPicker);
}

CUndoRedoCommandColorPicker::~CUndoRedoCommandColorPicker()
{
	
}

void CUndoRedoCommandColorPicker::undo()
{
	// nldebug("CUndoRedoCommandColorPicker::undo()");
	// nlassert(m_ColorPicker);
	
	m_Binder->enable(false);
	if (m_ColorPicker->currentColor() != m_Undo)
		m_ColorPicker->setCurrentColor(m_Undo);
	m_Binder->enable(true);
}

void CUndoRedoCommandColorPicker::redo()
{
	// nldebug("CUndoRedoCommandColorPicker::redo()");
	// nlassert(m_ColorPicker);

	m_Binder->enable(false);
	if (m_ColorPicker->currentColor() != m_Redo)
		m_ColorPicker->setCurrentColor(m_Redo);
	m_Binder->enable(true);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

} /* namespace NLQT */

/* end of file */
