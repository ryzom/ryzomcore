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

#ifndef NLQT_UNDO_REDO_BINDERS_H
#define NLQT_UNDO_REDO_BINDERS_H
#include <nel/misc/types_nl.h>

// STL includes

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QUndoCommand>

// NeL includes

// Project includes

class QUndoStack;
class QAbstractButton;
class QLineEdit;
class QComboBox;
class QtColorPicker;

namespace NLQT {

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoBinderButton
 * \brief CUndoRedoBinderButton
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoBinderButton : public QObject
{
	Q_OBJECT

public:
	CUndoRedoBinderButton(QAbstractButton *abstractButton, QUndoStack *undoStack);
	virtual ~CUndoRedoBinderButton();

	inline void enable(bool enabled) { m_Enabled = enabled; }
	
private slots:
	void abstractButtonToggled(bool checked);
	
private:
	bool m_Enabled; // binder enabled
	bool m_Checked;
	QAbstractButton *m_AbstractButton;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoBinderButton(const CUndoRedoBinderButton &);
	CUndoRedoBinderButton &operator=(const CUndoRedoBinderButton &);
	
}; /* class CUndoRedoBinderButton */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoCommandButton
 * \brief CUndoRedoCommandButton
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoCommandButton : public QUndoCommand
{
public:
	CUndoRedoCommandButton(CUndoRedoBinderButton *binder, QAbstractButton *abstractButton, bool undo/*, bool redo*/);
	virtual ~CUndoRedoCommandButton();

    virtual void undo();
    virtual void redo();
	
private:
	bool m_Undo;
	/*bool m_Redo;*/
	CUndoRedoBinderButton *m_Binder;
	QAbstractButton *m_AbstractButton;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoCommandButton(const CUndoRedoCommandButton &);
	CUndoRedoCommandButton &operator=(const CUndoRedoCommandButton &);
	
}; /* class CUndoRedoCommandButton */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoBinderLineEdit
 * \brief CUndoRedoBinderLineEdit
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoBinderLineEdit : public QObject
{
	Q_OBJECT

public:
	CUndoRedoBinderLineEdit(QLineEdit *lineEdit, QUndoStack *undoStack);
	virtual ~CUndoRedoBinderLineEdit();

	inline void enable(bool enabled) { m_Enabled = enabled; }
	
private slots:
	void lineEditTextChanged(const QString &text);
	
private:
	bool m_Enabled; // binder enabled
	QString m_LastValue;
	QLineEdit *m_LineEdit;
	QUndoStack *m_UndoStack;
	int m_Id;

private:
	CUndoRedoBinderLineEdit(const CUndoRedoBinderLineEdit &);
	CUndoRedoBinderLineEdit &operator=(const CUndoRedoBinderLineEdit &);
	
}; /* class CUndoRedoBinderLineEdit */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoCommandLineEdit
 * \brief CUndoRedoCommandLineEdit
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoCommandLineEdit : public QUndoCommand
{
public:
	CUndoRedoCommandLineEdit(CUndoRedoBinderLineEdit *binder, QLineEdit *abtractLineEdit, const QString &undo, const QString &redo, int id);
	virtual ~CUndoRedoCommandLineEdit();

    virtual void undo();
    virtual void redo();
	
	virtual int id() const { return m_Id; };
    virtual bool mergeWith(const QUndoCommand *other);

private:
	QString m_Undo;
	QString m_Redo;
	CUndoRedoBinderLineEdit *m_Binder;
	QLineEdit *m_LineEdit;
	QUndoStack *m_UndoStack;
	int m_Id;

private:
	CUndoRedoCommandLineEdit(const CUndoRedoCommandLineEdit &);
	CUndoRedoCommandLineEdit &operator=(const CUndoRedoCommandLineEdit &);
	
}; /* class CUndoRedoCommandLineEdit */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoBinderComboBox
 * \brief CUndoRedoBinderComboBox
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoBinderComboBox : public QObject
{
	Q_OBJECT

public:
	CUndoRedoBinderComboBox(QComboBox *comboBox, QUndoStack *undoStack);
	virtual ~CUndoRedoBinderComboBox();

	inline void enable(bool enabled) { m_Enabled = enabled; }
	
private slots:
	void comboBoxCurrentIndexChanged(int index);
	
private:
	bool m_Enabled; // binder enabled
	int m_LastValue;
	QComboBox *m_ComboBox;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoBinderComboBox(const CUndoRedoBinderComboBox &);
	CUndoRedoBinderComboBox &operator=(const CUndoRedoBinderComboBox &);
	
}; /* class CUndoRedoBinderComboBox */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoCommandComboBox
 * \brief CUndoRedoCommandComboBox
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoCommandComboBox : public QUndoCommand
{
public:
	CUndoRedoCommandComboBox(CUndoRedoBinderComboBox *binder, QComboBox *abtractComboBox, int undo, int redo);
	virtual ~CUndoRedoCommandComboBox();

    virtual void undo();
    virtual void redo();

private:
	int m_Undo;
	int m_Redo;
	CUndoRedoBinderComboBox *m_Binder;
	QComboBox *m_ComboBox;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoCommandComboBox(const CUndoRedoCommandComboBox &);
	CUndoRedoCommandComboBox &operator=(const CUndoRedoCommandComboBox &);
	
}; /* class CUndoRedoCommandComboBox */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoBinderColorPicker
 * \brief CUndoRedoBinderColorPicker
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoBinderColorPicker : public QObject
{
	Q_OBJECT

public:
	CUndoRedoBinderColorPicker(QtColorPicker *colorPicker, QUndoStack *undoStack);
	virtual ~CUndoRedoBinderColorPicker();

	inline void enable(bool enabled) { m_Enabled = enabled; }
	
private slots:
	void colorPickerColorChanged(const QColor &col);
	
private:
	bool m_Enabled; // binder enabled
	QColor m_LastValue;
	QtColorPicker *m_ColorPicker;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoBinderColorPicker(const CUndoRedoBinderColorPicker &);
	CUndoRedoBinderColorPicker &operator=(const CUndoRedoBinderColorPicker &);
	
}; /* class CUndoRedoBinderColorPicker */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

/**
 * CUndoRedoCommandColorPicker
 * \brief CUndoRedoCommandColorPicker
 * \date 2010-02-13 14:02GMT
 * \author Jan Boon (Kaetemi)
 */
class CUndoRedoCommandColorPicker : public QUndoCommand
{
public:
	CUndoRedoCommandColorPicker(CUndoRedoBinderColorPicker *binder, QtColorPicker *abtractColorPicker, const QColor &undo, const QColor &redo);
	virtual ~CUndoRedoCommandColorPicker();

    virtual void undo();
    virtual void redo();

private:
	QColor m_Undo;
	QColor m_Redo;
	CUndoRedoBinderColorPicker *m_Binder;
	QtColorPicker *m_ColorPicker;
	QUndoStack *m_UndoStack;

private:
	CUndoRedoCommandColorPicker(const CUndoRedoCommandColorPicker &);
	CUndoRedoCommandColorPicker &operator=(const CUndoRedoCommandColorPicker &);
	
}; /* class CUndoRedoCommandColorPicker */

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

} /* namespace NLQT */

#endif /* #ifndef NLQT_UNDO_REDO_BINDERS_H */

/* end of file */
