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
#include "filtered_chat_summary.h"

//===================================================================================
void CFilteredChatSummary::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver= f.serialVersion(2);
	f.serialCheck(NELID("USHC"));
	f.serial(SrcGuild);
	f.serial(SrcTeam);
	f.serial(SrcAroundMe);
	f.serial(SrcTell);
	f.serial(SrcSystemInfo);
	f.serialEnum(Target);

	if(ver>=1)
		f.serial(SrcUniverse);

	if(ver>=2)
		f.serial(SrcRegion);
}

//===================================================================================
void CFilteredDynChatSummary::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(0);
	f.serialCheck(NELID("USHC"));
	if (ver >= 0)
	{
		for (uint8 i = 0; i < CChatGroup::MaxDynChanPerPlayer; i++)
		{
			f.serial(SrcDynChat[i]);
		}
	}

}