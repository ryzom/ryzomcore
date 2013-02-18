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

#ifndef DECAL_ANIM_H
#define DECAL_ANIM_H


namespace NLMISC
{
	class CVector2f;
}

namespace NLGUI
{
	class CLuaObject;
}

using namespace NLGUI;

class CDecal;

// TODO nico : this would fit nicely in the particle system animation system (would be more flexible)
class CDecalAnim
{
public:
	std::string		Texture;
	uint			DurationInMs;
	//
	float			EndScaleFactor;
	float			EndAngleInDegrees;
	NLMISC::CRGBA	StartDiffuse;
	NLMISC::CRGBA	EndDiffuse;
	NLMISC::CRGBA	StartEmissive;
	NLMISC::CRGBA	EndEmissive;
public:
	CDecalAnim();
	void updateDecal(const NLMISC::CVector2f &pos, float animRatio, CDecal &dest, float refScale) const;
	void buildFromLuaTable(CLuaObject &table);
};

#endif
