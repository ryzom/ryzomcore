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



#ifndef NL_PLAYER_VISION_DELTA_H
#define NL_PLAYER_VISION_DELTA_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/net/message.h"
#include "ryzom_entity_id.h"

#include "base_types.h"

/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CPlayerVisionDelta
{
public:
	class CIdSlot
	{
	public:
		//NLMISC::CEntityId	Id;
		TDataSetRow			Index;
		uint8				Slot;

		CIdSlot() {}
		//CIdSlot(const NLMISC::CEntityId &id, uint8 slot) : Id(id), Slot(slot) {}
		CIdSlot(TDataSetRow index, uint8 slot) : Index(index), Slot(slot) {}

		void		serial(NLMISC::IStream &s)
		{
			//s.serial(Id);
			s.serial(Index);
			s.serial(Slot);
		}

		std::string toString() const
		{
			return NLMISC::toString("[%u]=%s",Slot,Index.toString().c_str());
		}
	};

public:

	/// Constructor
	CPlayerVisionDelta() {}


public:
	/// The Id of the player that has vision changes
	//NLMISC::CEntityId			PlayerId;
	TDataSetRow					PlayerIndex;

	/// Entities in vision
	std::vector<CIdSlot>		EntitiesIn;

	/// Entities out vision
	std::vector<CIdSlot>		EntitiesOut;

	/// Entities replaced (not needed anymore)
	//std::vector<CIdSlot>		EntitiesReplace;

	/// clear fout the vision delta record without deallocating memory
	void reset(TDataSetRow playerIndex)
	{
		PlayerIndex= playerIndex;
		EntitiesIn.clear();
		EntitiesOut.clear();
	}

	// check whether the vision delta record is empty or not
	bool empty() const
	{
		return EntitiesIn.empty() && EntitiesOut.empty();
	}

	// check whether the vision delta record is empty or not
	std::string toString() const
	{
		std::string result;
		result+="In(";
		for (uint32 i=0;i<EntitiesIn.size();++i)
		{
			if (i>1) result+=',';
			result+= EntitiesIn[i].toString();
		}
		result+=") Out(";
		for (uint32 i=0;i<EntitiesOut.size();++i)
		{
			if (i>1) result+=',';
			result+= EntitiesOut[i].toString();
		}
		result+=")";
		return result;
	}

	/// Serialises the whole vision delta
	void		serial(NLMISC::IStream &s)
	{
		//s.serial(PlayerId);
		s.serial(PlayerIndex);
		s.serialCont(EntitiesIn);
		s.serialCont(EntitiesOut);
		//s.serialCont(EntitiesReplace);
	}

	/// Decodes a message containing a list of vision deltas sent by the gpms
	static void	decodeVisionDelta(NLNET::CMessage &msgin, std::list<CPlayerVisionDelta> &listVisionDelta)
	{
		while ((uint)msgin.getPos() < (uint)msgin.length())
		{
			listVisionDelta.push_back(CPlayerVisionDelta());
			msgin.serial(listVisionDelta.back());
		}
	}

	/// Decodes a message containing a list of vision deltas sent by the gpms
	static void	decodeVisionDelta(NLNET::CMessage &msgin, std::vector<CPlayerVisionDelta> &vectorVisionDelta)
	{
		while ((uint)msgin.getPos() < (uint)msgin.length())
		{
			vectorVisionDelta.push_back(CPlayerVisionDelta());
			msgin.serial(vectorVisionDelta.back());
		}
	}
};


#endif // NL_PLAYER_VISION_DELTA_H

/* End of player_vision_delta.h */
