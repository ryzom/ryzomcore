// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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

#ifndef LANDSCAPE_EDITOR_H
#define LANDSCAPE_EDITOR_H

// Project includes

// NeL includes
#include <nel/ligo/zone_bank.h>
#include <nel/ligo/zone_region.h>

// STL includes
#include <string>

// Qt includes

namespace LandscapeEditor
{

struct LigoData
{
	uint8			posX;
	uint8			posY;
	uint8			rot;
	uint8			flip;
	std::string		zoneName;
	std::string		sharingMatNames[4];
	uint8			sharingCutEdges[4];

	LigoData();

	bool operator!= (const LigoData& other) const
	{
		return (posX != other.posX) ||
			   (posY != other.posY) ||
			   (rot != other.rot) ||
			   (flip != other.flip) ||
			   (zoneName != other.zoneName) ||
			   (sharingMatNames[0] != other.sharingMatNames[0]) ||
			   (sharingMatNames[1] != other.sharingMatNames[1]) ||
			   (sharingMatNames[2] != other.sharingMatNames[2]) ||
			   (sharingMatNames[3] != other.sharingMatNames[3]) ||
			   (sharingCutEdges[0] != other.sharingCutEdges[0]) ||
			   (sharingCutEdges[1] != other.sharingCutEdges[1]) ||
			   (sharingCutEdges[2] != other.sharingCutEdges[2]) ||
			   (sharingCutEdges[3] != other.sharingCutEdges[3]);
	}
};

class ZoneRegionEditor
{
public:
	ZoneRegionEditor();
	~ZoneRegionEditor();

	// Load landscape data from file
	bool load(const std::string &fileName);

	// Save landscape data to file
	bool save();

	void ligoData(LigoData &data, const sint32 x, const sint32 y);

	void setLigoData(const LigoData &data, const sint32 x, const sint32 y);

	// Set file name
	void setFileName(const std::string &fileName);

	NLLIGO::CZoneRegion &zoneRegion();

	void setZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);

private:

	bool m_modified;
	bool m_editable;
	std::string m_fileName;
	NLLIGO::CZoneRegion m_zoneRegion;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_EDITOR_H
