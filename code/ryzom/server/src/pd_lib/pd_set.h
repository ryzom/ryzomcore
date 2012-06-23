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

#ifndef NL_PD_SET_H
#define NL_PD_SET_H

#include "nel/misc/types_nl.h"
#include <set>
#include <map>

namespace RY_PDS
{


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
template<typename Key, typename T>
class CPDSet
{
public:

	/// Constructor
	CPDSet()
	{
	}

public:

	class iterator
	{
	public:
	};

	class const_iterator
	{
	public:
	};

private:

	/// Container Type
	typedef std::map<Key, T>		TMap;

	/// Internal STL Container
	TMap							_Map;

};

}; // RY_PDS

#endif // NL_PD_SET_H

/* End of pd_set.h */
