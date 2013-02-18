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


#ifndef NEW_WIDGET_WIDGET_H
#define NEW_WIDGET_WIDGET_H

#include "ui_new_widget_widget.h"
#include <vector>
#include <string>

namespace GUIEditor
{
	class CWidgetInfoTree;

	class NewWidgetWidget : public QWidget, public Ui::NewWidgetWidget
	{
		Q_OBJECT
	public:
		NewWidgetWidget( QWidget *parent = NULL );
		~NewWidgetWidget();

		/// Fills the widget list with the widget names
		void fillWidgetList( std::vector< std::string > &widgets );

		/// Sets the widget info tree so we can add new widgets
		void setWidgetInfoTree( CWidgetInfoTree *tree ){ widgetInfoTree = tree; }

	private Q_SLOTS:
		void onAddClicked();

	private:
		/// Checks if the name is valid
		bool checkNameField();

		/// Checks if the name is not a duplicate
		bool checkNameDuplicate();

		/// Adds the new widget
		void addNewWidget();

		CWidgetInfoTree *widgetInfoTree;

	Q_SIGNALS:
		void widgetAdded();

	};
}


#endif

