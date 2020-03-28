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

#ifndef _PACKED_WORLD_BUILDER_H
#define _PACKED_WORLD_BUILDER_H


#include "nel/misc/vector_2f.h"

namespace NL3D
{
	class CPackedWorld;
}

class CPackedWorldBuilder
{
public:
	// merge a set of zones into a packed world
	void build(const std::vector<std::string> &zoneNames, const std::string &cachePath, bool addLandscapeIG, NL3D::CPackedWorld &dest, float refineTheshold);
	// create a window & fly through a list of packed worlds
	class CIslandInfo
	{
	public:
		NL3D::CPackedWorld *PW;
		NLMISC::CVector		StartPosition;
		std::string			TexName; // projected texture
		NLMISC::CVector		CornerMin, CornerMax;
		float				UScale, VScale;
		CIslandInfo() : PW(NULL), UScale(1.f), VScale(1.f) {}
	};
	//
	void fly(std::vector<CIslandInfo>  &islands, float camSpeed);
};


#endif
