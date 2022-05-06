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
#include "action_position.h"

//
// Using
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;


namespace CLFECOMMON {

//
// Classes
//

/*
 * Unpacks the positions from the bitmemstream into Position16[]
 * (Position[] must then be filled externally)
 */
void CActionPosition::unpack (NLMISC::CBitMemStream &message)
{
//	if (_ClientDateMode)
//	{
	/*	uint32	d;
		message.serial(d, _DateBitPack);
		TickDate = d;*/
//	}

	/*message.serial (IsDelta);
	message.serial (Garanty);
	if (IsDelta)
	{
		message.serial (PackedDelta[0]);
		message.serial (PackedDelta[1]);
		message.serial (PackedDelta[2]);
	}
	else
	{
		message.serial (Position[0]);
		message.serial (Position[1]);
		message.serial (Position[2]);
	}*/

	// Get Position16 and decode absolute/relative mode from LSbit of poszr
	message.serialAndLog1( Position16[0] );
	message.serialAndLog1( Position16[1] );
	message.serialAndLog1( Position16[2] );
	IsRelative	=	(Position16[2] & (uint16)0x1)!=0;
	Interior	=	(Position16[2] & (uint16)0x2)!=0;

#ifdef TEST_POSITION_CORRECTNESS
//#pragma message (NL_LOC_MSG "TEST_POSITION_CORRECTNESS")
	TCoord px, py;
	message.serialAndLog1( px );
	message.serialAndLog1( py );
	Position[0] = px;
	Position[1] = py;
#endif
	// Decode position
	// Here, we don't set the MSdword (absolute pos in case of relative mode)
	// because it is deduced using the parent (on the client)
	/*Position[0] = 0;
	Position[0] = (uint64)(uint32)(((sint32)posx) << 4);
	Position[1] = 0;
	Position[1] = (uint64)(uint32)(((sint32)posy) << 4);
	Position[2] = 0;
	Position[2] = (uint64)(uint32)(((sint32)poszr & 0xFFFE) << 4);*/
}


/*
 * Packs the positions in Position[] into the bitmemstream
 */
void CActionPosition::pack (NLMISC::CBitMemStream &message)
{
	/*
	 * Warning: When adding data here, don't forget to update size()
	 */

//	if (!_ClientDateMode)
//	{
	/*	uint32	d = (uint32)TickDate;
		message.serial(d, _DateBitPack);*/
//	}

	/*message.serial (IsDelta);
	message.serial (Garanty);
	if (IsDelta)
	{
		message.serial (PackedDelta[0]);
		message.serial (PackedDelta[1]);
		message.serial (PackedDelta[2]);
	}
	else
	{
		message.serial (Position[0]);
		message.serial (Position[1]);
		message.serial (Position[2]);
	}*/

	//H_BEFORE(PositionPack);

	// Get the right position, depending on the "relative" bit, and
	// scale precision from 1 mm to 16 mm and take only 16 lower bits (=> 1048 m range)
	uint32 pxy16;
	uint16 posx16, posy16, posz16;
	posx16 = (uint16)(Position[0] >> 4);
	posy16 = (uint16)(Position[1] >> 4);
	pxy16 = ((uint32)(posx16) << 16) | (uint32)posy16;
	posz16 = ((uint16)(Position[2] >> 4) + 2) & ((uint16)0xFFFC);
	if ( IsRelative )	posz16 |= (uint16)0x1;
	if ( Interior )		posz16 |= (uint16)0x2;
	//nlinfo( "Slot% hu: Pos: %d %d %d, Pos16: %hu %hu %hu, Date %u", (uint16)CLEntityId, (sint32)Position[0], (sint32)Position[1], (sint32)Position[2], posx, posy, poszr, GameCycle );

	message.serialAndLog1( pxy16 );
	message.serialAndLog1( posz16 );

#ifdef TEST_POSITION_CORRECTNESS
//#pragma message (NL_LOC_MSG "TEST_POSITION_CORRECTNESS")
		TCoord px = Position[0];
		TCoord py = Position[1];
		message.serialAndLog1( px );
		message.serialAndLog1( py );
#endif

	//H_AFTER(PositionPack);
}

void CActionPosition::serial (NLMISC::IStream &f)
{
	f.serial (Position[0], Position[1], Position[2]);
}

uint32 CActionPosition::priority ()
{
	return 1;
}

//
// Static variables
//

//static const uint			DefaultDateBitPack = 6;

//bool		CActionPosition::_ClientDateMode = false;
/*uint		CActionPosition::_DateBitPack = DefaultDateBitPack;
uint		CActionPosition::_MaxDateDelta = (1 << DefaultDateBitPack) - 1;*/

}


