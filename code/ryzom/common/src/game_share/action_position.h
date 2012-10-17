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



#ifndef NL_ACTION_POSITION_H
#define NL_ACTION_POSITION_H

//
// Includes
//

#include <nel/misc/types_nl.h>
#include <nel/misc/stream.h>
#include <nel/misc/vector.h>

#include "continuous_action.h"
//#include "../frontend_service/fe_types.h"


namespace CLFECOMMON {

//
// Classes
//

class CActionPosition : public CContinuousAction
{
private:
	/*static uint			_DateBitPack;
	static uint			_MaxDateDelta;*/

public:

	/** This function creates initializes its fields using the buffer.
	 * \param buffer pointer to the buffer where the data are
	 * \size size of the buffer
	 */
	void	unpack (NLMISC::CBitMemStream &message);

	/** This function transform the internal field and transform them into a buffer for the UDP connection.
	 * \param buffer pointer to the buffer where the data will be written
	 * \size size of the buffer
	 */
	void	pack (NLMISC::CBitMemStream &message);

	/// This functions is used when you want to transform an action into an IStream.
	void	serial (NLMISC::IStream &f);

	/** Returns the size of this action when it will be send to the UDP connection:
	 * the size is IN BITS, not in bytes (the actual size is this one plus the header size)
	 */
#ifdef TEST_POSITION_CORRECTNESS
#pragma message (NL_LOC_MSG "TEST_POSITION_CORRECTNESS")
	uint32	size () { return 3*16 + 2*32; }
#else
	uint32	size () { return 3*16; } // See also CActionFactory::sizeFast()
#endif

	/// Returns the maximum size of this action (INCLUDING the header size handled by CActionFactory!)
#ifdef TEST_POSITION_CORRECTNESS
#pragma message (NL_LOC_MSG "TEST_POSITION_CORRECTNESS")
	static uint32	getMaxSizeInBit() { return 3*16 + 2*32; }
#else
	static uint32	getMaxSizeInBit() { return 3*16; }
#endif


	/// Returns the priority of this action, it can changed dynamically if you want
	uint32	priority ();

	static CAction	*create () { return new CActionPosition; }

	///
	/*void	packDelta(CAction::TValue originx, CAction::TValue originy, CAction::TValue originz)
	{
		if (IsDelta)
			return;

		TValue	delta;

		delta = Position[0] - originx;
		if (delta>32767 || delta<-32768) return;
		PackedDelta[0] = (sint16)delta;

		delta = Position[1] - originy;
		if (delta>32767 || delta<-32768) return;
		PackedDelta[1] = (sint16)delta;

		delta = Position[2] - originz;
		if (delta>32767 || delta<-32768) return;
		PackedDelta[2] = (sint16)delta;

		IsDelta = true;
	}

	///
	void	unpackDelta(CAction::TValue originx, CAction::TValue originy, CAction::TValue originz)
	{
		if (!IsDelta)
			return;

		IsDelta = false;

		Position[0] = (sint32)(originx + PackedDelta[0]);
		Position[1] = (sint32)(originy + PackedDelta[1]);
		Position[2] = (sint32)(originz + PackedDelta[2]);
	}*/

	///
	void	getPosition(CAction::TValue originx, CAction::TValue originy, CAction::TValue originz,
						CAction::TValue &posx, CAction::TValue &posy, CAction::TValue &posz) const
	{
		/*if (IsDelta)
		{
			posx = originx + PackedDelta[0];
			posy = originy + PackedDelta[1];
			posz = originz + PackedDelta[2];
		}
		else
		{*/
			posx = Position[0];
			posy = Position[1];
			posz = Position[2];
		//}
	}

	///
	//void	setGaranty(bool g) { Garanty = g; }

	//bool	isDelta() const { return IsDelta; }

	//bool	hasGaranty() const { return Garanty; }


	/** Setup the number of bits to pack dates.
	 * Should be between 5 and 64 bits, and should be the same value on both client and server.
	 */
	/*static void	setDateBits(uint num)
	{
		_DateBitPack = num;
		_MaxDateDelta = (1 << num) - 1;
	}

	/// Setup the date within the position, that will be used by the client for lag compensation (used by the server.)
	void		setupTickDate(NLMISC::TGameCycle sync, NLMISC::TGameCycle originDate, uint32 sendPacket)
	{
		TickDate = sendPacket - (originDate-sync);
		if (TickDate > _MaxDateDelta)
			TickDate = _MaxDateDelta;
	}

	/// Decode the date for the lag compensation on the client.
	NLMISC::TGameCycle	getTickDate(NLMISC::TGameCycle sync, uint32 receivePacket)
	{
		return (TickDate = receivePacket - (TickDate-sync));
	}

	/// Returns the tick date in the action without any decoding process
	NLMISC::TGameCycle	getTickDate()
	{
		return TickDate;
	}*/

	//
	// The overloaded function that print warning messages. Theses must NOT be used
	//

	///
	virtual CAction::TValue	getValue() const
	{
		nlwarning("CActionPosition: forbidden call to getValue()");
		return (CAction::TValue)0;
	}

	///
	virtual void	setValue(const CAction::TValue &value)
	{
		nlwarning("CActionPosition: forbidden call to setValue()");
	}

public:

	sint32				Position[3];
	uint16				Position16[3];
	//NLMISC::TGameCycle	TickDate; // Now there is a timestamp in every action from the FE to the client
	bool				IsRelative;
	bool				Interior;

//protected:

	CActionPosition () {}

	void	reset() { /*IsDelta = false; Garanty = false;*/ /*TickDate = 0;*/ }
};

}

#endif // NL_ACTION_POSITION_H

/* End of action_position.h */
