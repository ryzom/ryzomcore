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


#ifndef WIDGET_SERIALIZER_H
#define WIDGET_SERIALIZER_H

#include <fstream>
#include <string>

struct _xmlNode;

namespace GUIEditor
{

	class WidgetSerializer
	{
	public:
		WidgetSerializer()
		{
			level = 0;
		}

		~WidgetSerializer(){}

		void setFile( const std::string &name ){ fileName = name; }
		void setActiveGroup( const std::string &name ){ activeGroup = name; }
		bool serialize( const std::string &masterGroup );

	private:
		bool serializeTree( _xmlNode *node );

		std::string fileName;
		std::string activeGroup;
		std::ofstream out;
		long level;
	};

}

#endif

