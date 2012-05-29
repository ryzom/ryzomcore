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
#include "animation_fx_sheet.h"
#include "nel/georges/u_form_elm.h"


using namespace NLGEORGES;

// ***************************************************************************
// CAnimationFXSheet
// ***************************************************************************

//-----------------------------------------------
// CAnimationFXSheet
//-----------------------------------------------
CAnimationFXSheet::CAnimationFXSheet(const std::string &psName, const float *userParams)
{
	Type = ANIMATION_FX;
	PSName = psName;
	StickMode.Mode = CFXStickMode::Spawn;
	if (!userParams)
	{
		UserParam[0] = UserParam[1] = UserParam[2] = UserParam[3] = 0.f;
	}
	else
	{
		std::copy(userParams, userParams + 4, UserParam);
	}
	Color = NLMISC::CRGBA::White;
	ScaleFX = false;
	RepeatMode = Loop;
	RayRefLength = 1.f;
}// CAnimationFXSheet //

//-----------------------------------------------
// build with prefix
//-----------------------------------------------
void CAnimationFXSheet::build(const NLGEORGES::UFormElm &item, const std::string &prefix /* = ""*/)
{
	bool ok = true;
	ok &= item.getValueByName(PSName,			(prefix + "PSName").c_str());
	ok &= item.getValueByName(UserParam[0],		(prefix + "UserParam0").c_str());
	ok &= item.getValueByName(UserParam[1],		(prefix + "UserParam1").c_str());
	ok &= item.getValueByName(UserParam[2],		(prefix + "UserParam2").c_str());
	ok &= item.getValueByName(UserParam[3],		(prefix + "UserParam3").c_str());
	ok &= item.getValueByName(Color,			(prefix + "Color").c_str());
	ok &= item.getValueByName(TrajectoryAnim,	(prefix + "TrajectoryAnim").c_str());
	ok &= item.getValueByName(ScaleFX,			(prefix + "ScaleFX").c_str());
	ok &= item.getValueByName(RayRefLength,		(prefix + "RayRefLength").c_str());
	//
	ok &= StickMode.build(item, (prefix + "StickMode.").c_str());

	uint32 repeatMode = Loop;
	bool okRepeatMode = item.getValueByName(repeatMode, (prefix + "RepeatMode").c_str());
	if (okRepeatMode)
	{
		RepeatMode = (TRepeatMode) repeatMode;
	}
	ok &= okRepeatMode;
	if (!ok)
	{
		nlwarning("couldn't read all fields");
	}
}// build //

//-----------------------------------------------
// build
//-----------------------------------------------
void CAnimationFXSheet::build(const NLGEORGES::UFormElm &item)
{
	build(item, "");
}


//-----------------------------------------------
// serial
//-----------------------------------------------
void CAnimationFXSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(PSName);
	f.serial(StickMode);
	for(uint k = 0; k < 4; ++k)
	{
		f.serial(UserParam[k]);
	}
	f.serial(TrajectoryAnim);
	f.serial(Color);
	f.serial(ScaleFX);
	f.serialEnum(RepeatMode);
	f.serial(RayRefLength);
}// serial //
