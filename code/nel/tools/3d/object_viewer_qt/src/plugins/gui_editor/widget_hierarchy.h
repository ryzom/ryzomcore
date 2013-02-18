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

#ifndef WIDGET_HA_H
#define WIDGET_HA_H

#include "ui_widget_hierarchy.h"

namespace NLGUI
{
	class CInterfaceGroup;
}

class QTreeWidgetItem;

namespace GUIEditor
{
	class WidgetHierarchy : public QWidget, public Ui::WidgetHierarchyTree
	{
		Q_OBJECT
	public:
		WidgetHierarchy( QWidget *parent = NULL );
		~WidgetHierarchy();

		void setMasterGroup( const std::string &name ){ masterGroup = name; }

		void clearHierarchy();
		void buildHierarchy( std::string &masterGroup );

	private:
		void buildHierarchy( QTreeWidgetItem *parent, NLGUI::CInterfaceGroup *group );

	public Q_SLOTS:
		void onGUILoaded();

	private Q_SLOTS:
		void onItemDblClicked( QTreeWidgetItem *item );

	private:
		std::string currentSelection;
		std::string masterGroup;

	Q_SIGNALS:
		void selectionChanged( std::string &id );
	};
}

#endif
