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

#ifndef TYPE_DIALOG_H
#define TYPE_DIALOG_H

#include <QWidget>
#include <QComboBox>
#include <QTableWidget>
#include <QLineEdit>

class GeorgesEditorDoc;

/**
 * Right panel widget for editing a Type (.typ) node, ported from CTypeDialog.
 */
class TypeDialog : public QWidget
{
	Q_OBJECT

public:
	TypeDialog(QWidget *parent = nullptr);
	~TypeDialog();

	void setDocument(GeorgesEditorDoc *doc);
	void updateFromDocument();

private slots:
	void onTypeChanged(int index);
	void onUITypeChanged(int index);
	void onDefaultChanged(const QString &text);
	void onMinChanged(const QString &text);
	void onMaxChanged(const QString &text);
	void onIncrementChanged(const QString &text);
	void onPredefValueChanged(int row, int column);
	void onAddPredefValue();
	void onRemovePredefValue();

private:
	void setupUi();

	GeorgesEditorDoc *_doc;

	QComboBox *_typeCombo;
	QComboBox *_uiTypeCombo;
	QComboBox *_defaultCombo;
	QComboBox *_minCombo;
	QComboBox *_maxCombo;
	QComboBox *_incrementCombo;
	QTableWidget *_predefTable;
};

#endif // TYPE_DIALOG_H
