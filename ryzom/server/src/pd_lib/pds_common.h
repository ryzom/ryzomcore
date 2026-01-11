// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#ifndef RY_PDS_COMMON_H
#define RY_PDS_COMMON_H

#include <nel/misc/types_nl.h>
#include <nel/misc/i_xml.h>
#include <vector>

#include <nel/misc/log.h>

//
// pd_lib Includes
//
#include "pd_lib.h"



/**
 * Task Event Listener
 */
class ITaskEventListener
{
public:

	/// Called when RBS task was done successfully
	virtual void	taskSuccessful(void* arg) = 0;

	/// Called when RBS task has failed
	virtual void	taskFailed(void* arg) = 0;
};




class CLocatePath
{
public:

	/// Constructor
	CLocatePath() : Pos(0)	{}	

	/// End of path?
	bool				end() const
	{
		return Pos >= FullPath.size();
	}

	struct CLocateAttributeNode
	{
		/// The name of the attribute to locate
		std::string		Name;

		/// Is this node an array?
		bool			Array;

		/// Is this node a set?
		bool			Set;

		/// Key, for array and set
		std::string		Key;
	};

	/// Get current node
	CLocateAttributeNode&	node()
	{
		return FullPath[Pos];
	}

	/// Step to next
	bool				next()
	{
		++Pos;
		return !end();
	}

	typedef std::vector<CLocateAttributeNode>	TLocatePath;

	/// Full path to value
	TLocatePath			FullPath;

	/// Current pos in path
	uint				Pos;
};






/// 'for' loop through xml children
#define	FOREACH_CHILD(node, parent, type)	for (node=CIXml::getFirstChildNode(parent, #type);\
												 node!=NULL;\
												 node=CIXml::getNextChildNode(node, #type))




/*
 * Utility functions.
 */

/**
 * getProperty()
 * Store node property propName into result
 * Returns true iff the property appears in the node (but won't tell if property was of the matching type)
 */
template<typename T>
bool	getProperty(xmlNodePtr node, const char* propName, T &result)
{
	std::string	res;
	if (!NLMISC::CIXml::getPropertyString(res, node, propName))
	{
		nlwarning("Couldn't get property '%s' in xml node", propName);
		return false;
	}
	NLMISC::fromString(res, result);
	return true;
}

/**
 * getProperty()
 * Store node property propName into result, and if property not present in node use defaultValue instead
 * Returns true iff the property appears in the node (but won't tell if property was of the matching type)
 */
template<typename T>
bool	getProperty(xmlNodePtr node, const char* propName, T &result, const T &defaultValue, bool quiet = true)
{
	std::string	res;
	if (!NLMISC::CIXml::getPropertyString(res, node, propName))
	{
		if (!quiet)
			nlwarning("Couldn't get property '%s' in xml node, use '%s' value instead", propName, NLMISC::toString(defaultValue).c_str());
		result = defaultValue;
		return false;
	}
	NLMISC::fromString(res, result);
	return true;
}




#endif //RY_PDS_COMMON_H



