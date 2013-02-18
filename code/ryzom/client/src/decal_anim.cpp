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
//
#include "decal_anim.h"
#include "decal.h"
#include "nel/gui/lua_ihm.h"
#include "nel/gui/lua_object.h"
//
#include "nel/misc/vector_2f.h"
#include "nel/misc/algo.h"


using namespace NLMISC;

// *****************************************************************************
CDecalAnim::CDecalAnim()
{
	DurationInMs = 1000;
	EndScaleFactor = 1.f;
	EndAngleInDegrees = 0.f;
	StartDiffuse = CRGBA::White;
	EndDiffuse = CRGBA::Black;
	StartEmissive = CRGBA::Black;
	EndEmissive = CRGBA::Black;
}

// *****************************************************************************
void CDecalAnim::updateDecal(const NLMISC::CVector2f &pos, float animRatio, CDecal &dest, float refScale) const
{
	dest.setTexture(Texture);
	dest.setWorldMatrixForSpot(pos, refScale * blend(1.f, EndScaleFactor, animRatio), blend(0.f, EndAngleInDegrees, animRatio));
	dest.setDiffuse(blend(StartDiffuse, EndDiffuse, animRatio));
	dest.setEmissive(blend(StartEmissive, EndEmissive, animRatio));
}




// *****************************************************************************
void CDecalAnim::buildFromLuaTable(CLuaObject &table)
{
	// retrieve a value from a lua table or affect a default value if not found
	#define GET_LUA_VALUE(dest, Type, CastType, Default) \
	(dest) = (table[#dest].is##Type()) ? (CastType) table[#dest].to##Type() : (Default);
	GET_LUA_VALUE(DurationInMs,      Number, uint, 1000)
	GET_LUA_VALUE(EndScaleFactor,    Number, float, 1.f)
	GET_LUA_VALUE(EndAngleInDegrees, Number, float, 1.f)
	GET_LUA_VALUE(Texture, String, std::string, "")
	GET_LUA_VALUE(StartDiffuse, RGBA, CRGBA, CRGBA::White)
	GET_LUA_VALUE(EndDiffuse, RGBA, CRGBA, CRGBA::White)
	GET_LUA_VALUE(StartEmissive, RGBA, CRGBA, CRGBA::Black)
	GET_LUA_VALUE(EndEmissive, RGBA, CRGBA, CRGBA::Black)
}
