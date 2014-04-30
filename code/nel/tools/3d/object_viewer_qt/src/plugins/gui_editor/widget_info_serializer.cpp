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

#include "widget_info_serializer.h"
#include "widget_info_tree.h"
#include <fstream>

namespace GUIEditor
{
	void CWidgetInfoSerializer::serialize( CWidgetInfoTree *tree )
	{
		tree->accept( this );
	}

	void CWidgetInfoSerializer::visit( CWidgetInfoTreeNode *node )
	{
		std::fstream f;
		std::string filename = "widgets/" + node->getInfo().name + ".xml";
		SWidgetInfo &info = node->getInfo();

		// Make a copy of the properties, then remove all the parents' properties, since we only want to save the properties of this node
		std::vector< SPropEntry > props;
		props.resize( info.props.size() );
		std::copy( info.props.begin(), info.props.end(), props.begin() );

		CWidgetInfoTreeNode *parent = node->getParent();
		if( parent != NULL )
		{
			SWidgetInfo &parentInfo = parent->getInfo();
			std::vector< SPropEntry >::const_iterator itr = parentInfo.props.begin();
			while( itr != parentInfo.props.end() )
			{
				std::vector< SPropEntry >::iterator fItr;
				fItr = std::find( props.begin(), props.end(), *itr );
				if( fItr == props.end() )
					continue;
				props.erase( fItr );
				++itr;
			}
		}
		
		f.open( filename.c_str(), std::ios_base::out );
		if( !f.is_open() )
		{
			nlinfo( "Failed to open %s for writing!\n", filename.c_str() );
			return;
		}

		f << "<widget>" << std::endl;
		f << "\t<header>" << std::endl;
		f << "\t\t<name>" << info.name << "</name>" << std::endl;
		f << "\t\t<guiname>" << info.GUIName << "</guiname>" << std::endl;
		f << "\t\t<classname>" << info.className << "</classname>" << std::endl;
		f << "\t\t<ancestor>" << info.ancestor << "</ancestor>" << std::endl;
		f << "\t\t<description>" << info.description << "</description>" << std::endl;
		
		if( info.isAbstract )
			f << "\t\t<abstract>" << "true" << "</abstract>" << std::endl;
		else
			f << "\t\t<abstract>" << "false" << "</abstract>" << std::endl;

		f << "\t\t<icon>" << info.icon << "</icon>" << std::endl;

		f << "\t</header>" << std::endl;
		f << "\t<properties>" << std::endl;

		for( std::vector< SPropEntry >::const_iterator itr = props.begin(); itr != props.end(); ++itr )
		{
			f << "\t\t<property>" << std::endl;
			
			f << "\t\t\t<name>" << itr->propName << "</name>" << std::endl;
			f << "\t\t\t<type>" << itr->propType << "</type>" << std::endl;
			f << "\t\t\t<default>" << itr->propDefault << "</default>" << std::endl;

			f << "\t\t</property>" << std::endl;
		}

		f << "\t</properties>" << std::endl;
		f << "</widget>" << std::endl;
		f << std::endl;

		f.close();

	}
}

