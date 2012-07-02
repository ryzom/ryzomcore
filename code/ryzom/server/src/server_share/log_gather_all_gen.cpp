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
#include "game_share/utils.h"



extern void forceLink_Player();

extern void forceLink_Character();

extern void forceLink_Item();

extern void forceLink_Command();

extern void forceLink_Outpost();

extern void forceLink_Ring();

extern void forceLink_Chat();


void forceLinkOfAllLogs()
{
	
	forceLink_Player();
	
	forceLink_Character();
	
	forceLink_Item();
	
	forceLink_Command();
	
	forceLink_Outpost();
	
	forceLink_Ring();
	
	forceLink_Chat();
	
};
	


