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



//////////////
//	INCLUDE	//
//////////////
#include "stdpch.h"
//
#include "resists.h"
#include "player_manager/character.h"
#include "game_item_manager/game_item.h"

//////////////
//	USING	//
//////////////
using namespace std;
using namespace NLMISC;

uint16 CCreatureResists::ImmuneScore = 0xffff;

//--------------------------------------------------------------
//					serial() 
//--------------------------------------------------------------
void CCreatureResists::serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serial(Fear);
	f.serial(Sleep);
	f.serial(Stun);
	f.serial(Root);
	f.serial(Slow);
	f.serial(Snare);
	f.serial(Blind);
	f.serial(Madness);
	
	f.serial(Acid);
	f.serial(Cold);
	f.serial(Electricity);
	f.serial(Fire);
	f.serial(Poison);
	f.serial(Rot);
	f.serial(Shockwave);	
} // serial //
