// Ryzom Core Studio World Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef CONST_STR_ARR_EDIT_DLG
#define CONST_STR_ARR_EDIT_DLG

#include "ui_const_string_array_editor.h"
#include <QStringList>

class ConstStrArrEditDialog : public QDialog, public Ui::ConstStrArrEditorDialog
{
	Q_OBJECT
public:
	ConstStrArrEditDialog( QDialog *parent = NULL );
	~ConstStrArrEditDialog();

	void setStrings( const QStringList &strings );
	void setValue( const QString &value );
	QString getValue() const;

public Q_SLOTS:
	void accept();
	void reject();

private Q_SLOTS:
	void onAddClicked();
	void onRemoveClicked();

private:
	void setupConnections();
};


#endif


