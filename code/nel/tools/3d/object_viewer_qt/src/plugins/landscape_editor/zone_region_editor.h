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

// Data
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

	bool operator!= (const LigoData& other) const;
};

/**
@class ZoneRegionObject
@brief
@details
*/
class ZoneRegionObject
{
public:
	ZoneRegionObject();
	~ZoneRegionObject();

	/// Load landscape data from file
	bool load(const std::string &fileName);

	/// Save landscape data to file (before save, should set file name).
	bool save();

	/// Get ligo data
	void ligoData(LigoData &data, const sint32 x, const sint32 y);

	/// Set ligo data
	void setLigoData(const LigoData &data, const sint32 x, const sint32 y);

	/// Get file name
	std::string fileName() const;

	/// Set file name, use for saving data in file
	void setFileName(const std::string &fileName);

	/// Accessor to LIGO CZoneRegion
	NLLIGO::CZoneRegion &ligoZoneRegion();

	void setLigoZoneRegion(const NLLIGO::CZoneRegion &zoneRegion);

	/// Check position, it belongs to the landscape
	bool checkPos(const sint32 x, const sint32 y);

	/// Helper flag to know if a ps has been modified
	/// @{
	bool isModified() const;

	void setModified(bool modified);
	/// @}

private:

	bool m_modified;
	bool m_editable;
	std::string m_fileName;
	NLLIGO::CZoneRegion m_zoneRegion;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_EDITOR_H
