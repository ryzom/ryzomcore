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

#ifndef CL_SKY_SHEET_H
#define CL_SKY_SHEET_H

#include <string>
#include <vector>


#include "entity_sheet.h"
#include "sky_object_sheet.h"

class CSkySheet : public CEntitySheet
{
public:
	std::string InstanceGroupName;
	std::string	AnimationName;
	double		AnimLengthInSeconds;
	std::vector<CSkyObjectSheet> Objects;
	// bitmaps that gives the sky lighting depending
	std::string AmbientSunLightBitmap;
	std::string DiffuseSunLightBitmap;
	// fog color
	std::string	FogColorBitmap;
	// Water env map (computed from sky scene)
	float		WaterEnvMapCameraHeight;
	uint8		WaterEnvMapAlpha;
public:
	// ctor
	CSkySheet();
	// Build from an external script
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	/// From CEntitySheet
	virtual void build(const NLGEORGES::UFormElm &item);
	/// From CEntitySheet  : serialize sheet into binary data file.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};




#endif
