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


#include "stdpch.h"

#include "patat_grid.h"

#include "nel/misc/aabbox.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/path.h"
#include "nel/ligo/ligo_config.h"

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

extern NLLIGO::CLigoConfig LigoConfig;

/*
 * Constructor
 */
CPatatGrid::CPatatGrid()
{
}

/*
 * Destructor
 */
CPatatGrid::~CPatatGrid()
{
}


/*
 * Init patat grid
 */
void	CPatatGrid::init()
{
	// allocate 1 free entry (for no zone entry)
	_PrimZones.resize(1);
	_EntryTable.resize(1);
	_SelectGrid.clear();
	_ZoneMap.clear();
	_FlagTable.clear();
	_EntryMap.clear();
	_FreeEntries.clear();

	nlinfo("Initialized CPatatGrid, move grid is 1024x1024 uint16 (%.2f Mb)", (float)(sizeof(TEntryIndex)*1024*1024)/(1024.0f*1024.0f));
}


void	CPatatGrid::readPrimitive(IPrimitive *primitive, std::vector<uint32> &inFile)
{
	if (dynamic_cast<CPrimZone*>(primitive) != NULL)
	{
		// check good class
		string	primName;
		string	className;
		if (primitive->getPropertyByName("name", primName) &&
			primitive->getPropertyByName("class", className) &&
			_PrimZoneFilters.find(className) != _PrimZoneFilters.end())
		{
			_ZoneMap.insert(TZoneMap::value_type(primName, (sint32)_PrimZones.size()));
			inFile.push_back((uint32)_PrimZones.size());
			_PrimZones.push_back(static_cast<CPrimZone&>(*primitive));
//			_PrimZones.back().Name = primName;
		}
	}

	// parse children
	uint	i;
	for (i=0; i<primitive->getNumChildren(); ++i)
	{
		IPrimitive	*child;

		if (!primitive->getChild(child, i))
			continue;

		readPrimitive(child, inFile);
	}
}


/*
 * Use patat prim file
 */
void	CPatatGrid::usePrim(const string &primFile, std::vector<uint32> &inFile)
{
/*
	CIFile	f(CPath::lookup(primFile));
	CPrimRegion		region;
	CIXml			xml;
	xml.init(f);
	region.serial(xml);

	nlinfo("Loaded prim file '%s'", primFile.c_str());

	uint	i;
	uint	firstPrimZone = _PrimZones.size();
	inFile.clear();

	for (i=0; i<region.VZones.size(); ++i)
	{
		if (_ZoneMap.find(region.VZones[i].Name) != _ZoneMap.end())
		{
			nlwarning("Prim zone %s (in file %s) already added, discarded", region.VZones[i].Name.c_str(), primFile.c_str());
		}
		else
		{
			_ZoneMap.insert(TZoneMap::value_type(region.VZones[i].Name, _PrimZones.size()));
			inFile.push_back(_PrimZones.size());
			_PrimZones.push_back(region.VZones[i]);
		}
	}
*/

	// lookup for primitive file
	CIFile		f(CPath::lookup(primFile));
	CIXml		xml;

	CPrimitives	prims;

	// load xml file
	xml.init(f);
	nlinfo("Loaded prim file '%s'", primFile.c_str());

	// read nodes
	if (!prims.read(xml.getRootNode(), primFile.c_str(), LigoConfig))
	{
		nlwarning("Can't use primitive file '%s', xml parse error",  primFile.c_str());
		return;
	}

	uint	i;
	uint	firstPrimZone = (uint)_PrimZones.size();
	inFile.clear();

	// get CPrimNode
	CPrimNode	*primRootNode = prims.RootNode;

	// read recursive node
	readPrimitive(primRootNode, inFile);
	
	const uint32	maxGridEntryIndex = (1 << (sizeof(TEntryIndex)*8)) - 1;

	_FlagTable.resize(_PrimZones.size(), false);

	// for each patat, sample points (check they belong to the patat) and find the matching entry
	for (i=firstPrimZone; i<_PrimZones.size(); ++i)
	{
		CPrimZone	&zone = _PrimZones[i];

		if (zone.VPoints.empty())
			continue;

		// build bounding box
		double	xmin = 1.0e10f, xmax = -xmin, ymin = xmin, ymax = xmax;
		double	x, y;

		uint	j;
		for (j=0; j<zone.VPoints.size(); ++j)
		{
			if (zone.VPoints[j].x < xmin)	xmin = zone.VPoints[j].x;
			if (zone.VPoints[j].y < ymin)	ymin = zone.VPoints[j].y;
			if (zone.VPoints[j].x > xmax)	xmax = zone.VPoints[j].x;
			if (zone.VPoints[j].y > ymax)	ymax = zone.VPoints[j].y;
		}

		xmin = _SelectGrid.round(xmin);
		ymin = _SelectGrid.round(ymin);
		xmax = _SelectGrid.round(xmax);
		ymax = _SelectGrid.round(ymax);

		nlinfo("Sampling %d/%d (%d samples)", i-firstPrimZone+1, _PrimZones.size()-firstPrimZone, (sint)((xmax-xmin)/PatatGridResolution * (ymax-ymin)/PatatGridResolution));

		// sample zone
		for (y=ymin; y<=ymax; y+=PatatGridResolution)
		{
			for (x=xmin; x<=xmax; x+=PatatGridResolution)
			{
				CVector	v((float)x, (float)y, 0.0f);

				// check sampled point belongs to the patat
				if (!zone.contains(v))
					continue;

				// get the entry index for this point
				TEntryIndex	index = (TEntryIndex)getEntryIndex(v);
				CEntry		&entry = _EntryTable[index];

				// compute the new hash code for this point
				// basically, hash is previous hash code times zone number
				//uint32	hash = (index == 0) ? i : entry.HashCode * i;

				// other hash function, assuming index0 entry has 0 hashcode.
				uint32	hash = (entry.HashCode<<4) + (entry.HashCode>>28) + i;

				pair<TEntryMap::iterator, TEntryMap::iterator>	range;
				TEntryMap::iterator	it;

				// get all entries with this hash code
				range = _EntryMap.equal_range(hash);

				// check if selected entries match the new point
				for (it=range.first; it!=range.second; ++it)
				{
					CEntry	&selected = _EntryTable[(*it).second];

					// selected must have one zone more than entry
					if (selected.Zones.size() != entry.Zones.size()+1)
						continue;

					uint	k;
					for (k=0; k<entry.Zones.size(); ++k)
						if (entry.Zones[k] != selected.Zones[k])
							break;

					// if first zones of selected are different from entry, give up
					if (k != entry.Zones.size())
						continue;

					// if all zones in selected and extended entry matches, found good entry
					if (selected.Zones.back() == i)
						break;
				}

				// if found an entry matching for the point, set up the grid to point on the selected entry
				if (it != _EntryMap.end())
				{
					CEntry	&selected = _EntryTable[(*it).second];
					setEntryIndex(v, selected.EntryIndex);

					--entry.NumSamples;
					++selected.NumSamples;

					// if no more samples point to this entry, free it and leave it _FreeEntries stack
					if (entry.EntryIndex != 0 && entry.NumSamples == 0)
					{
						entry.HashCode = 0xffffffff;
						entry.Zones.clear();
						_FreeEntries.push_back(entry.EntryIndex);
					}
				}
				else
				{
					// else create a new entry
					uint32		newIndex = 0;

					// use a freed entry if possible
					if (!_FreeEntries.empty())
					{
						newIndex = _FreeEntries.front();
						_FreeEntries.pop_front();
					}
					else
					{
						newIndex = (uint32)_EntryTable.size();
						_EntryTable.resize(newIndex+1);
					}

					// reloads entry, as it might have be reallocated
					CEntry		&entry = _EntryTable[index];

					// checks the index fits in table
					if (newIndex >= maxGridEntryIndex)
					{
						nlerror("Error in PatatGrid build, type 'TEntryIndex' too short (%d bytes), use wider type !", sizeof(TEntryIndex));
					}

					CEntry	&newEntry = _EntryTable[newIndex];

					// inits new entry
					newEntry.EntryIndex = (TEntryIndex)newIndex;
					newEntry.HashCode = hash;
					newEntry.NumSamples = 1;
					newEntry.Zones = entry.Zones;
					newEntry.Zones.push_back(i);

					// add entry to map
					_EntryMap.insert(TEntryMap::value_type(hash, (TEntryIndex)newIndex));

					// points point to new entry
					setEntryIndex(v, newEntry.EntryIndex);
				}
			}
		}
	}
}

/*
 * build patat grid
 */
bool	CPatatGrid::diff(TEntryIndex previous, TEntryIndex next, vector<uint32> &in, vector<uint32> &out)
{
	if (previous >= _EntryTable.size())
	{
		nlwarning("Entry index %d out of bounds, assume empty zone", previous);
		previous = 0;
	}
	if (next >= _EntryTable.size())
	{
		nlwarning("Entry index %d out of bounds, assume empty zone", next);
		next = 0;
	}

	in.clear();
	out.clear();

	if (previous == next)
		return false;

	CEntry	&p = _EntryTable[previous];
	CEntry	&n = _EntryTable[next];

	uint	i;

	// flag all of previous
	for (i=0; i<p.Zones.size(); ++i)
		_FlagTable[p.Zones[i]] = true;

	// check new
	for (i=0; i<n.Zones.size(); ++i)
	{
		// if flag clear, a new zone
		if (!_FlagTable[n.Zones[i]])
			in.push_back(n.Zones[i]);
		_FlagTable[n.Zones[i]] = false;
	}

	// check previous
	for (i=0; i<p.Zones.size(); ++i)
	{
		// if flag set (didn't change), a left zone
		if (_FlagTable[p.Zones[i]])
			out.push_back(p.Zones[i]);
		_FlagTable[p.Zones[i]] = false;
	}

	return (!in.empty() || !out.empty());
}
