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

		CInterfaceElement *ag =
			CWidgetManager::getInstance()->getElementFromId( activeGroup );
		if( ag == NULL )
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

		if( mg->serializeGroup( root, "root" ) == NULL )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		if( !CWidgetManager::getInstance()->serializeOptions( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		if( !CWidgetManager::getInstance()->getParser()->serializeKeySettings( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}


		if( !CWidgetManager::getInstance()->getParser()->serializePointerSettings( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}


		if( !CWidgetManager::getInstance()->getParser()->serializeVariables( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}


		if( !CWidgetManager::getInstance()->getParser()->serializeProcs( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		ag->setActive( false );

		if( mg->serializeSubGroups( root ) == NULL )
		{
			ag->setActive( true );
			xmlFreeNode( root );
			out.close();
			return false;
		}

		ag->setActive( true );

		if( !mg->serializeLinks( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		if( !CWidgetManager::getInstance()->serializeTreeData( root ) )
		{
			xmlFreeNode( root );
			out.close();
			return false;
		}

		level = -1;
		serializeTree( root );


		xmlFreeNode( root );
		out.close();

		return true;
	}

	bool WidgetSerializer::serializeTree( _xmlNode *node )
	{
		level++;

		std::string tabs;
		for( int i = 0; i < level; i++ )
			tabs.push_back( '\t' );

		out << tabs << "<" << node->name;

		xmlAttrPtr prop = node->properties;
		while( prop != NULL )
		{
			std::string name  =
				std::string( reinterpret_cast< const char* >( prop->name ) );

			std::string value =
				std::string( reinterpret_cast< const char* >( xmlGetProp( node, BAD_CAST name.c_str() ) ) );

			out << " " << name << "=\"" << value << "\"" << std::endl << tabs;

			prop = prop->next;
		}

		if( node->children != NULL )
		{
			out << tabs << ">" << std::endl << std::endl;

			xmlNodePtr child = node->children;
			while( child != NULL )
			{
				serializeTree( child );
				child = child->next;
			}

			out << tabs << "</" << node->name << ">" << std::endl << std::endl;
		}
		else
		{
			out << tabs << "/>" << std::endl << std::endl;
		}

		level--;

		return true;
	}
}



