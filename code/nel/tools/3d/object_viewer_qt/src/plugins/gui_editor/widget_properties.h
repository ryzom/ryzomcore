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


#ifndef WIDGETPROPS_H
#define WIDGETPROPS_H

#include "ui_widget_properties.h"
#include <map>
#include <vector>
#include <string>

namespace GUIEditor
{
	struct SPropEntry
	{
		std::string propName;
		std::string propType;
		std::string propDefault;
		
		static SPropEntry create( const char *propname, const char *proptype, const char *propdefault )
		{
			SPropEntry entry;
			entry.propName = propname;
			entry.propType = proptype;
			entry.propDefault = propdefault;
			return entry;
		}
	};

	struct SWidgetInfo
	{
		std::string name;
		std::string GUIName;
		std::string description;
		bool isAbstract;
		std::string icon;

		std::vector< SPropEntry > props;
	};


	class CWidgetProperties : public QWidget, public Ui::WidgetProperties
	{
		Q_OBJECT

	public:
		CWidgetProperties( QWidget *parent = NULL );
		~CWidgetProperties();
		void setupWidgetInfo( std::map< std::string, SWidgetInfo > *info );

	private Q_SLOTS:
		void onListSelectionChanged( int i );

	private:
		void setPropsOf( const char *name );
		std::map< std::string, SWidgetInfo > *widgetInfo;
	};
}

#endif
