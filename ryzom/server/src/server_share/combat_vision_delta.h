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



#ifndef NL_COMBAT_VISION_DELTA_H
#define NL_COMBAT_VISION_DELTA_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/net/message.h"
#include "game_share/ryzom_entity_id.h"

/**
 * Serialisation of vision deltas from gpms to cms
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2002
 */
class CCombatVisionDelta
{
public:
	/// Constructor
	CCombatVisionDelta() {}


public:
	/// The Id of the player that has vision changes
	NLMISC::CEntityId				EntityId;

	/// Entities in vision
	std::vector<NLMISC::CEntityId>	EntitiesIn;

	/// Entities out vision
	std::vector<NLMISC::CEntityId>	EntitiesOut;

	/// Serialises the whole vision delta
	void		serial(NLMISC::IStream &s)
	{
		// Serialises entity id
		s.serial(EntityId);
		// new entities in vision
		s.serialCont(EntitiesIn);
		// previous entities no longer in vision
		s.serialCont(EntitiesOut);
	}

	/// Decodes a message containing a list of vision deltas sent by the gpms
	static void	decodeVisionDelta(NLNET::CMessage &msgin, std::list<CCombatVisionDelta> &visionDelta)
	{
		while ((uint)msgin.getPos() < (uint)msgin.length())
		{
			visionDelta.push_back(CCombatVisionDelta());
			msgin.serial(visionDelta.back());
		}
	}

	/// Decodes a message containing a list of vision deltas sent by the gpms
	static void	decodeVisionDelta(NLNET::CMessage &msgin, std::vector<CCombatVisionDelta> &visionDelta)
	{
		while ((uint)msgin.getPos() < (uint)msgin.length())
		{
			visionDelta.push_back(CCombatVisionDelta());
			msgin.serial(visionDelta.back());
		}
	}
};


#endif // NL_COMBAT_VISION_DELTA_H

/* End of combat_vision_delta.h */
