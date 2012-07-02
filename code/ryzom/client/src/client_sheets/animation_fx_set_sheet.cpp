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
#include "animation_fx_set_sheet.h"
#include "nel/georges/u_form_elm.h"

// ****************************************************************************************************
CAnimationFXSetSheet::CAnimationFXSetSheet()
{
	Type = ANIMATION_FX_SET;
	for(uint k = 0; k < MaxNumFX; ++k)
	{
		CanReplaceStickMode[k] = (k == 0);
		CanReplaceStickBone[k] = (k == 0);
	}
}

// ****************************************************************************************************
void CAnimationFXSetSheet::build(const NLGEORGES::UFormElm &item)
{
	buildWithPrefix(item);
}

// ****************************************************************************************************
void CAnimationFXSetSheet::buildWithPrefix(const NLGEORGES::UFormElm &item, const std::string &prefix /*=""*/)
{
	FX.clear();
	// build standard fx
	for(uint k = 0; k < MaxNumFX; ++k)
	{
		CAnimationFXSheet fs;
		fs.build(item, NLMISC::toString((prefix + "FX%d.").c_str(), (int) k).c_str());
		if (!fs.PSName.empty())
		{
			FX.push_back(fs);
		}
		item.getValueByName(CanReplaceStickMode[k], NLMISC::toString((prefix + "CanReplaceStickMode%d").c_str(), (int) k).c_str());
		item.getValueByName(CanReplaceStickBone[k], NLMISC::toString((prefix + "CanReplaceStickBone%d").c_str(), (int) k).c_str());
	}
}

// ****************************************************************************************************
void CAnimationFXSetSheet::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialCont(FX);
	for(uint k = 0; k < MaxNumFX; ++k)
	{
		f.serial(CanReplaceStickMode[k]);
		f.serial(CanReplaceStickBone[k]);
	}
}

