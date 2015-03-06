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


#ifndef NEW_PROPERTY_WIDGET_H
#define NEW_PROPERTY_WIDGET_H

#include "ui_new_property_widget.h"

namespace GUIEditor
{
	class CWidgetInfoTreeNode;

	/// Widget for adding a new property
	class NewPropertyWidget : public QWidget, public Ui::NewPropertyWidget
	{
		Q_OBJECT
	public:
		NewPropertyWidget( QWidget *parent = NULL );
		~NewPropertyWidget();

		/// Sets the widget info, so that we can check for duplicates
		void setWidgetInfo( CWidgetInfoTreeNode *node ){ widgetInfoTreeNode = node; }
	
	private Q_SLOTS:
		void onAddClicked();

		/// Checks the validity of the property name field
		bool checkName() const;

		/// Checks the validity of the default value field
		bool checkDefaultValue() const;

	private:
		CWidgetInfoTreeNode *widgetInfoTreeNode;


	Q_SIGNALS:
		void propertyAdded();
	};
}

#endif

