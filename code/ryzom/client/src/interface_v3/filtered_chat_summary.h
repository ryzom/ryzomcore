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



#ifndef CL_FILTERED_CHAT_SUMMARY_H
#define CL_FILTERED_CHAT_SUMMARY_H

#include "game_share/chat_group.h"

// class for serialisation of user chat (filtered chat) infos
class CFilteredChatSummary
{
public:
	// Inputs
	bool SrcGuild;
	bool SrcTeam;
	bool SrcAroundMe;
	bool SrcTell;
	bool SrcSystemInfo;
	bool SrcRegion;
	bool SrcUniverse;
	// output
	CChatGroup::TGroupType Target;
public:
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};

// class for serialisation of user dyn chat (filtered chat) info
class CFilteredDynChatSummary
{
public:
	// Inputs
	bool SrcDynChat[CChatGroup::MaxDynChanPerPlayer];
	// output
	CChatGroup::TGroupType Target;
public:
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};







#endif
