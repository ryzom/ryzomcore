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

class ZoneRegionEditor
{
public:
	ZoneRegionEditor();
	~ZoneRegionEditor();

	// Load landscape data from file
	bool load(const std::string &fileName);

	// Save landscape data to file
	bool save();

	// Set file name
	void setFileName(const std::string &fileName);

	NLLIGO::CZoneRegion &zoneRegion();

private:

	bool m_modified;
	bool m_editable;
	std::string m_fileName;
	NLLIGO::CZoneRegion m_zoneRegion;
};

} /* namespace LandscapeEditor */

#endif // LANDSCAPE_EDITOR_H
