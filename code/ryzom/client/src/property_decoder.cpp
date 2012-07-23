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



#include "property_decoder.h"
#include "game_share/continuous_action.h"
#include "game_share/action_position.h"

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;

/*
 * Constructor
 */
CPropertyDecoder::CPropertyDecoder()
{
}


void	CPropertyDecoder::receive(TPacketNumber /* packetNumber */, CAction *action)
{
	if (!action->isContinuous())
		return;

	if (action->Code == ACTION_POSITION_CODE)
	{
		CActionPosition			*act = (CActionPosition *)(action);
/*
		CPropertyEntry			&px = _Entities[act->Slot].Properties[PROPERTY_POSX];
		CPropertyEntry			&py = _Entities[act->Slot].Properties[PROPERTY_POSY];
		CPropertyEntry			&pz = _Entities[act->Slot].Properties[PROPERTY_POSZ];
*/
#ifdef TEST_POSITION_CORRECTNESS
//#pragma message ("TEST_POSITION_CORRECTNESS")
		TCoord posx = (TCoord)(act->Position[0]);
		TCoord posy = (TCoord)(act->Position[1]);
#endif

		if ( act->IsRelative )
		{
			// Relative position (entity in a ferry...)
			act->Position[0] = (sint32)act->Position16[0];
			act->Position[1] = (sint32)act->Position16[1];
			act->Position[2] = (sint32)act->Position16[2];
			_Entities[act->Slot].PosIsRelative = true;
			_Entities[act->Slot].PosIsInterior = false;
			nlinfo( "Relative Pos: %d %d %d, Pos16: %hd %hd %hd, Date %u", (sint32)act->Position[0], (sint32)act->Position[1], (sint32)act->Position[2], act->Position16[0], act->Position16[1], act->Position16[2], act->GameCycle );
		}
		else
		{
			// Absolute position
			//nlinfo( "RefPos: %d %d %d RefBits: %hd %hd %hd", _RefPosX, _RefPosY, _RefPosZ, _RefBitsX, _RefBitsY, _RefBitsZ );
			decodeAbsPos2D( act->Position[0], act->Position[1], act->Position16[0], act->Position16[1] );
			act->Position[2] = ((sint32)((sint16)act->Position16[2])) << 4;
			if (act->Interior)
				act->Position[2] += 2;
			//act->Position[2] = _RefPosZ + (((sint32)((sint16)(act->Position16[2] - _RefBitsZ))) << 4);
			//nlinfo( "Pos16: %hd %hd %hd => Pos: %d %d %d", act->Position16[0], act->Position16[1], act->Position16[2], (sint32)px.LastReceived, (sint32)py.LastReceived, (sint32)pz.LastReceived );
			_Entities[act->Slot].PosIsRelative = false;
			_Entities[act->Slot].PosIsInterior = act->Interior;
			//nlinfo( "Slot %hu: Absolute, Pos: %d %d %d, Pos16: %hu %hu %hu, Date %u", (uint16)act->CLEntityId, (sint32)act->Position[0], (sint32)act->Position[1], (sint32)act->Position[2], act->Position16[0], act->Position16[1], act->Position16[2], act->GameCycle );
			//nlinfo( "            RefPos: %d %d %d, RefBits: %hu %hu %hu", _RefPosX, _RefPosY, _RefPosZ, _RefBitsX, _RefBitsY, _RefBitsZ );
		}
/*
		px.LastReceived = act->Position[0];
		py.LastReceived = act->Position[1];
		pz.LastReceived = act->Position[2];
		//nldebug("CLPROPD: Received position (Id=%d) %d,%d (Pck=%d)", act->CLEntityId, act->Position[0], act->Position[1], packetNumber);
*/
#ifdef TEST_POSITION_CORRECTNESS
//#pragma message ("TEST_POSITION_CORRECTNESS")
		if ( ! (act->IsRelative) )
		{
			// Check if the compression algo for positions worked
			nlassertex( abs(posx - (TCoord)act->Position[0]) < 2000, ("Computed posX: %d Real posX: %d PosX16: %hu RefPosX: %d RefBitsX: %hu", (TCoord)(act->Position[0]), posx, act->Position16[0], _RefPosX, _RefBitsX) );
			nlassertex( abs(posy - (TCoord)act->Position[1]) < 2000, ("Computed posY: %d Real posY: %d PosY16: %hu RefPosY: %d RefBitsY: %hu", (TCoord)(act->Position[1]), posy, act->Position16[1], _RefPosY, _RefBitsY) );
		}
		nlinfo( "Receiving position %d %d %d m", posx/1000, posy/1000, (TCoord)act->Position[2]/1000 );
#endif

	}
/*
	else
	{
		CContinuousAction *act = (CContinuousAction *)(action);

		CPropertyEntry			&property = _Entities[act->Slot].Properties[act->Code];

		property.LastReceived = act->getValue();

		nldebug("CLPROPD: Received (Id=%d,Act=%d)=(%"NL_I64"d)(Pck=%d)", act->Slot, act->Code, property.LastReceived, packetNumber);
	}
*/
}


void	CPropertyDecoder::receive(TPacketNumber packetNumber, TPacketNumber /* ack */, vector<CAction *> &actions)
{
	// 1- process ack

	//this->ack(packetNumber, ack);


	// 2- receive actions, decode them and store values

	uint	i;
	for (i=0; i<actions.size(); ++i)
	{
		receive(packetNumber, actions[i]);
	}
}


//

void	CPropertyDecoder::setReferencePosition(const NLMISC::CVectorD &position)
{
	_RefPosX = (sint32)(position.x * 1000.0) & ~0xf; // correction here by Sadge: clear low-order to prevent flickering depending on player's reference position
	_RefPosY = (sint32)(position.y * 1000.0) & ~0xf; // correction here by Sadge
	//_RefPosZ = (sint32)(position.z * 1000.0);

	_RefBitsX = (uint16)((_RefPosX >> 4)&0xffff);
	_RefBitsY = (uint16)((_RefPosY >> 4)&0xffff);
	//_RefBitsZ = (uint16)(_RefPosZ >> 4);
}


//
/*
const CAction::TValue	&CPropertyDecoder::getProperty(TCLEntityId entity, TProperty property) const
{
	nlassert(entity < _Entities.size() && _Entities[entity].EntryUsed);

	return _Entities[entity].Properties[property].LastReceived;
}

void					CPropertyDecoder::getPosition(TCLEntityId entity, CAction::TValue &posx, CAction::TValue &posy, CAction::TValue &posz) const
{
	nlassert(entity < _Entities.size() && _Entities[entity].EntryUsed);
	const CEntityEntry	&entry = _Entities[entity];
	posx = entry.Properties[PROPERTY_POSX].LastReceived;
	posy = entry.Properties[PROPERTY_POSY].LastReceived;
	posz = entry.Properties[PROPERTY_POSZ].LastReceived;
}
*/
//

void	CPropertyDecoder::clear()
{
	uint	sz = (uint)_Entities.size();
	_Entities.clear();
	_Entities.resize(sz);
}

void	CPropertyDecoder::setMaximumEntities(uint maximum)
{
	_Entities.resize(maximum);
}

void	CPropertyDecoder::addEntity(TCLEntityId entity, TSheetId sheet)
{
	nlassert(entity < _Entities.size() && !_Entities[entity].EntryUsed);

	_Entities[entity].EntryUsed = true;
	_Entities[entity].Sheet = sheet;
/*
	uint	i;
	for (i=0; i<=LAST_CONTINUOUS_PROPERTY; ++i)
		_Entities[entity].Properties[i].init();
*/
}

bool	CPropertyDecoder::removeEntity(TCLEntityId entity)
{
	nlassertex(entity < _Entities.size(), ("entity=%hu size=%u", (uint16)entity,_Entities.size()) );

	//Workaround: assert converted to test when failure in vision from the server
	if ( _Entities[entity].EntryUsed )
	{
		_Entities[entity].EntryUsed = false;
		_Entities[entity].Sheet = 0xffff;
	}

	return true;
}
