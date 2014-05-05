// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef LINK_LIST_H
#define LINK_LIST_H

#include "ui_link_list.h"

namespace GUIEditor
{
	class LinkEditor;

	class LinkList : public QWidget, public Ui::LinkList
	{
		Q_OBJECT
	public:
		LinkList( QWidget *parent = NULL );
		~LinkList();
		void clear();

	public Q_SLOTS:
		void onGUILoaded();

	private Q_SLOTS:
		void onAddButtonClicked();
		void onRemoveButtonClicked();
		void onEditButtonClicked();
		void onItemDblClicked( QTreeWidgetItem *item );
		void onEditorFinished();

	private:
		LinkEditor *linkEditor;
	};
}

#endif

