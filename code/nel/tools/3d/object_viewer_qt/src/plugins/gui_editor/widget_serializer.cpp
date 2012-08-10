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


#include "widget_serializer.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"

namespace GUIEditor
{
	bool WidgetSerializer::serialize( const std::string &masterGroup )
	{
		if( fileName.empty() )
			return false;

		CInterfaceGroup *mg =
			CWidgetManager::getInstance()->getMasterGroupFromId( masterGroup );

		if( mg == NULL )
			return false;

		out.open( fileName.c_str() );
		if( !out.is_open() )
			return false;

		xmlNodePtr root = xmlNewNode( NULL, BAD_CAST "interface_config" );
		if( root == NULL )
		{
			out.close();
			return false;
		}

		if( mg->serialize( root, "root" ) == NULL )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		serializeTree( root );


		xmlFreeNode( root );
		out.close();

		return true;
	}

	bool WidgetSerializer::serializeTree( _xmlNode *node )
	{
		out << "<" << node->name;

		xmlAttrPtr prop = node->properties;
		while( prop != NULL )
		{
			std::string name  =
				std::string( reinterpret_cast< const char* >( prop->name ) );

			std::string value =
				std::string( reinterpret_cast< const char* >( xmlGetProp( node, BAD_CAST name.c_str() ) ) );

			out << " " << name << "=\"" << value << "\"" << std::endl;

			prop = prop->next;
		}

		out << ">" << std::endl;

		xmlNodePtr child = node->children;
		while( child != NULL )
		{
			serializeTree( child );
			child = child->next;
		}

		out << "</" << node->name << ">" << std::endl;

		return true;
	}
}



