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



#ifndef NL_IMPULSE_DECODER_H
#define NL_IMPULSE_DECODER_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_mem_stream.h"

#include "game_share/action.h"
#include "game_share/entity_types.h"


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CImpulseDecoder
{
private:
	CLFECOMMON::TPacketNumber	_LastAck0[1];
	CLFECOMMON::TPacketNumber	_LastAck1[2];
	CLFECOMMON::TPacketNumber	_LastAck2[4];

public:

	/// Constructor
	CImpulseDecoder();


	///
	void		decode(NLMISC::CBitMemStream &inbox, CLFECOMMON::TPacketNumber receivedPacket, CLFECOMMON::TPacketNumber receivedAck, CLFECOMMON::TPacketNumber nextSentPacket, std::vector<CLFECOMMON::CAction *> &actions);

	///
	void		reset();
};


#endif // NL_IMPULSE_DECODER_H

/* End of impulse_decoder.h */
