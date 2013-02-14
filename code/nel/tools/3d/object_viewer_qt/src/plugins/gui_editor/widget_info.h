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


#ifndef WIDGET_INFO_H
#define WIDGET_INFO_H

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

		bool operator==( const SPropEntry &other ) const
		{
			if( ( propName == other.propName ) &&
				( propType == other.propType ) &&
				( propDefault == other.propDefault ) )
				return true;
			else
				return false;
		}
	};

	struct SWidgetInfo
	{
		std::string name;
		std::string GUIName;
		std::string ancestor;
		std::string description;
		bool isAbstract;
		std::string icon;
		bool resolved;

		std::vector< SPropEntry > props;

		SWidgetInfo()
		{
			resolved = false;
			isAbstract = true;
		}

		~SWidgetInfo()
		{
			resolved = false;
			isAbstract = false;
		}

		/// Find a property by it's name
		std::vector< SPropEntry >::iterator findProp( const std::string &name )
		{
			std::vector< SPropEntry >::iterator itr = props.begin();
			while( itr != props.end() )
			{
				if( itr->propName == name )
					break;
				++itr;
			}
			return itr;
		}
	};
}

#endif
