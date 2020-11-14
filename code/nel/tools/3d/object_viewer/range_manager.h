// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef RANGE_MANAGER_H
#define RANGE_MANAGER_H

#include <map>
#include <string>

#include "nel/misc/stream.h"

/** This class records the ranges used for edition (for a given slider).
 *  It enables the user to set the range that match best what he wants
 *  full ASCII names are used as identifier
 */

template <class T> class CRangeManager
{
public:
	/** retrieve the range used for a given ID.
	 *  If the ID wasn't present, a new entry is created, and the given values are returned
	 */
	static std::pair<T, T> GetRange(const std::string &id, const T &minRange, const T &maxRange)
	{
		nlassert(id != "");
		if (_RangeMap.count(id) == 0) // not present yet ?
		{
			_RangeMap[id] = std::pair<T, T>(minRange, maxRange);			
		}
		return _RangeMap[id];
	}

	/** the same but no default values are provided. An assertion  occurs if not present.
	 *  In release build, the entrie is created, but with uninitialized values ...
	 */
	
	static std::pair<T, T> GetRange(const std::string &id)
	{
		nlassert(id != "");
		return _RangeMap[id];
	}

	/// set a new value for the given range

	static void SetRange(const std::string &id, const T &minRange, const T &maxRange)
	{
		nlassert(id != "");
		_RangeMap[id] = std::pair<T, T>(minRange, maxRange);	
	}

	/// serialization
	static void serial(NLMISC::IStream &f)
	{
		uint32 size;		
		if (!f.isReading())
		{
			size = _RangeMap.size();
			f.serial(size);
			for (TRangeMap::const_iterator it = _RangeMap.begin(); it != _RangeMap.end(); ++it)
			{	
				std::string s = it->first;
				f.serial(s);
				std::pair<T , T> value = it->second;
				f.serial(value.first);
				f.serial(value.second);
			}
		}
		else
		{
			_RangeMap.clear();
			f.serial(size);
			while (size --)
			{
				std::string id;
				std::pair<T , T> value;
				f.serial(id);
				f.serial(value.first);
				f.serial(value.second);
				_RangeMap[id] = value;
			}
		}
	}

	typedef std::map< std::string, std::pair<T, T> > TRangeMap;

protected:
	
	// the map that contains the ID, and their range values
	static TRangeMap _RangeMap;
	
};




#endif
