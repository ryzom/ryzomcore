// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013-2014  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef PROC_EDITOR_H
#define PROC_EDITOR_H

#include "ui_proc_editor.h"

class ActionList;

namespace GUIEditor
{
	class ActionEditor;

	class ProcEditor : public QWidget, public Ui::ProcEditor
	{
		Q_OBJECT
	public:
		ProcEditor( QWidget *parent = NULL );
		~ProcEditor();

		void setCurrentProc( const QString &name );
		void clear();

	private Q_SLOTS:
		void onEditButtonClicked();
		void onAddButtonClicked();
		void onRemoveButtonClicked();
		void onUpButtonClicked();
		void onDownButtonClicked();

	private:
		void swapListItems( int row1, int row2 );

		ActionEditor *actionEditor;
		QString currentProc;

		ActionList *alist;
	};
}

#endif

