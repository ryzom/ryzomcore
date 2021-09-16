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



#ifndef NL_ACTION_BLOCK_H
#define NL_ACTION_BLOCK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/bit_mem_stream.h"

#include "entity_types.h"

#include <vector>

namespace CLFECOMMON {


class CAction;

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CActionBlock
{
public:
	NLMISC::TGameCycle			Cycle;
	TPacketNumber				FirstPacket;
	std::vector<CAction*>		Actions;
	bool						Success;

	/// Constructor
	CActionBlock() : Cycle(0), FirstPacket(0), Success(true) {}

	/// Destructor
	~CActionBlock();

	/// serialisation method
	void	serial(NLMISC::CBitMemStream &msg);

	///
	static uint32	getHeaderSizeInBits() { return (sizeof(NLMISC::TGameCycle)+sizeof(uint8))*8; }
};

} // CLFECOMMON

#endif // NL_ACTION_BLOCK_H

/* End of action_block.h */
