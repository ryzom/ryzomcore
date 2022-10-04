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



#ifndef NL_PATAT_GRID_H
#define NL_PATAT_GRID_H

// Nel Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"

// Nel Ligo
#include "nel/ligo/primitive.h"

//
#include "move_grid.h"

// stl
#include <deque>
#include <vector>
#include <map>
#include <set>
#include <string>

// resolution in meters
#define	PatatGridResolution 1.0f
#define	mmPatatGridResolution 1000

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CPatatGrid
{
public:
	/// \name public Typedefs
	//@{

	/// The grid entry index
	typedef uint16							TEntryIndex;

	//@}

protected:
	/// \name typedefs
	//@{

	/// The container of prim zone
	typedef std::vector<NLLIGO::CPrimZone>	TZoneVector;

	/// The entry type
	class CEntry
	{
	public:
		CEntry() : EntryIndex(0), HashCode(0), NumSamples(0) {}
		/// The entry index for this entry
		TEntryIndex							EntryIndex;
		/// The entry hash code
		uint32								HashCode;
		/// The number of point in grid that point to this entry
		uint32								NumSamples;
		/// The zones for this
		std::vector<uint32>					Zones;

		/// serial
		void	serial(NLMISC::IStream &f)
		{
			sint	version = f.serialVersion(0);
			f.serial(EntryIndex, HashCode, NumSamples);
			f.serialCont(Zones);
		}
	};

	/// The entry table type
	typedef std::vector<CEntry>				TEntryTable;

	/// The entry map type to find entry quickly
	typedef std::multimap<uint32, TEntryIndex>	TEntryMap;

	/// The map of zone name (to zone id)
	typedef std::map<std::string, sint32>	TZoneMap;

	/// The move grid, used as grid
	typedef CMoveGrid<TEntryIndex, 1024, mmPatatGridResolution>	TGrid;

	//@}

	/// \name Attributes
	//@{

	/// Patats
	TZoneVector			_PrimZones;

	/// Patat map
	TZoneMap			_ZoneMap;

	/// The move grid
	TGrid				_SelectGrid;

	/// The entry table
	TEntryTable			_EntryTable;

	/// The flag table
	std::vector<bool>	_FlagTable;

	/// The fast entry map
	TEntryMap			_EntryMap;

	/// The free table entries
	std::deque<TEntryIndex>	_FreeEntries;

	//@}

	/// \name Class filtering
	//@{

	std::set<std::string>	_PrimZoneFilters;

	//@}

public:

	/// Constructor
	CPatatGrid();

	/// Destructor
	~CPatatGrid();

	/// Init grid;
	void	init();

	/// Use a prim file
	void	usePrim(const std::string &primFile, std::vector<uint32> &inFile);

	/// Check if patat exists
	bool	exist(const std::string &name) const	{ return (_ZoneMap.find(name) != _ZoneMap.end()); }

	/// Get entry index
	sint32	getEntryIndex(const NLMISC::CVector &v)
	{
		_SelectGrid.select(v);
		TGrid::CIterator	it = _SelectGrid.begin();
		sint32	index = (it != _SelectGrid.end()) ? (*it) : 0;
		_SelectGrid.clearSelection();
		return index;
	}

	/// Set entry index
	void	setEntryIndex(const NLMISC::CVector &v, sint32 entry)
	{
		_SelectGrid.select(v);
		TGrid::CIterator	it = _SelectGrid.begin();
		if (it != _SelectGrid.end())
		{
			(*it) = (TEntryIndex)entry;
		}
		else
		{
			TEntryIndex	idx = (TEntryIndex)entry;
			_SelectGrid.insert(idx, v);
		}
		_SelectGrid.clearSelection();
	}

	/// Get zone id from its name
	sint32	getZoneId(const std::string &name) const
	{
		TZoneMap::const_iterator	it = _ZoneMap.find(name);
		if (it == _ZoneMap.end())
		{
			nlwarning("Can't find Prim zone %s", name.c_str());
			return 0;
		}
		return (*it).second;
	}

	/// Get zone name from its id
	const std::string	&getZoneName(uint32 id) const
	{
		std::string *ret;
		if (_PrimZones[id].getPropertyByName("name", ret))
			return *ret;
		else
		{
			static std::string noName;
			return noName;
		}
	}

	/// Get zone differences between 2 entry
	bool	diff(TEntryIndex previous, TEntryIndex next, std::vector<uint32> &in, std::vector<uint32> &out);

	/// Serial
	void	serial(NLMISC::IStream &f)
	{
		sint	version = f.serialVersion(1);

		f.serialCont(_PrimZones);
		f.serialCont(_ZoneMap);
		f.serial(_SelectGrid);
		f.serialCont(_EntryTable);
		f.serialCont(_FlagTable);
		f.serialCont(_EntryMap);
		f.serialCont(_FreeEntries);

		if (version >= 1)
			f.serialCont(_PrimZoneFilters);
	}

	/// Display
	void	displayInfo(NLMISC::CLog *log = NLMISC::InfoLog)
	{
		log->displayNL("Display PatatGrid PrimZones: %d zones", _PrimZones.size());
		uint	i, j;
		for (i=0; i<_PrimZones.size(); ++i)
		{
			std::string name;
			_PrimZones[i].getPropertyByName("name", name);
			log->displayNL(" + %d: Name:%s Points:%d", i, name.c_str(), _PrimZones[i].VPoints.size());
		}

		log->displayNL("Display PatatGrid entries: %d entries", _EntryTable.size());
		for (i=0; i<_EntryTable.size(); ++i)
		{
			log->displayNL(" + %d: EntryIndex:%d HashCode:%d NumSamples:%d", i, _EntryTable[i].EntryIndex, _EntryTable[i].HashCode, _EntryTable[i].NumSamples);
			for (j=0; j<_EntryTable[i].Zones.size(); ++j)
				log->displayNL("   + Zone %d", _EntryTable[i].Zones[j]);
		}

		log->displayNL("Display quick EntryMap: %d in map", _EntryMap.size());
		TEntryMap::iterator	ite;
		for (ite=_EntryMap.begin(); ite!=_EntryMap.end(); ++ite)
			log->displayNL(" + %d->%d", (*ite).first, (*ite).second);

		log->displayNL("Display free Entries: %d free entries", _FreeEntries.size());
		for (i=0; i<_FreeEntries.size(); ++i)
			log->displayNL(" + %d", _FreeEntries[i]);

		log->displayNL("End of PatatGrid info");
	}

	/// Add CPrimZone class filter
	void	addPrimZoneFilter(const std::string &filter)
	{
		_PrimZoneFilters.insert(filter);
	}

	/// Remove CPrimZone class filter
	void	removePrimZoneFilter(const std::string &filter)
	{
		_PrimZoneFilters.erase(filter);
	}

	/// Reset CPrimZone class filter
	void	resetPrimZoneFilter()
	{
		_PrimZoneFilters.clear();
	}

protected:

	/// read recursive primitive
	void	readPrimitive(NLLIGO::IPrimitive *primitive, std::vector<uint32> &inFile);

};


#endif // NL_PATAT_GRID_H

/* End of patat_grid.h */
