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
#include "fx_stick_mode.h"

#include "nel/georges/u_form_elm.h"

using namespace NLGEORGES;

// ***************************************************************************
// CFXStickMode
// ***************************************************************************

//-----------------------------------------------
// build
//-----------------------------------------------
bool CFXStickMode::build(const NLGEORGES::UFormElm &item, const std::string &prefix /* = ""*/)
{
	bool ok = true;
	uint32 stickMode;
	ok &= item.getValueByName(stickMode, (prefix + "StickMode").c_str());
	if (ok) Mode = (CFXStickMode::TStickMode) stickMode;
	std::string userBoneName;
	ok &= item.getValueByName(userBoneName, (prefix + "UserBone").c_str());
	UserBoneName = NLMISC::CStringMapper::map(userBoneName);
	return ok;
}// build //

//-----------------------------------------------
// serial
//-----------------------------------------------
void CFXStickMode::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialEnum(Mode);
	NLMISC::CStringMapper::serialString(f, UserBoneName);
}// serial //
