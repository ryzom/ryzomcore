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


#ifndef WIDGET_PROP_PARSER
#define WIDGET_PROP_PARSER

#include "widget_info.h"
#include <map>
#include <QString>
#include <QFile>
#include <QXmlStreamReader>

namespace GUIEditor
{
	class CWidgetInfoTree;

	/// Parser for the widget properties XML files
	class CWidgetPropParser
	{
	public:
		CWidgetPropParser();
		~CWidgetPropParser();
		void setWidgetPropMap( std::map< std::string, SWidgetInfo > *info ){ widgetInfo = info; }
		void setWidgetInfoTree( CWidgetInfoTree *tree ){ widgetInfoTree = tree; }

		/// Parse the GUI widget template definitions
		void parseGUIWidgets();

	private:
		void parseGUIWidget( const QString &file );
		void parseGUIWidgetXML( QFile &file );
		QString parseGUIWidgetHeader( QXmlStreamReader &reader );
		void parseGUIWidgetProperties( QXmlStreamReader &reader, const QString &widgetName );

		/// Build the widget info tree from the parsed data
		void buildWidgetInfoTree();
		
		std::map< std::string, SWidgetInfo > *widgetInfo;		
		CWidgetInfoTree *widgetInfoTree;
	};
}

#endif
