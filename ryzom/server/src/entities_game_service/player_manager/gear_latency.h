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



#ifndef RY_GEAR_LATENCY_H
#define RY_GEAR_LATENCY_H

#include "nel/misc/bit_set.h"
#include "game_share/inventories.h"
#include "egs_sheets/egs_sheets.h"

class CCharacter;
/**
 * Class managing gear latency
 * When a player put an item on him, there is a time during which he can't use actions, use this item, dodge, shield, parry
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGearLatency
{
	NL_INSTANCE_COUNTER_DECL(CGearLatency);
public:

	/// update latencies
	void update(CCharacter * user);

	/// set a slot as latent
	void setSlot( INVENTORIES::TInventory inventory, uint32 slot, const CStaticItem * form, CCharacter * user );

	/// unset a slot as latent
	void unsetSlot( INVENTORIES::TInventory inventory, uint32 slot, CCharacter * user);

	/// return true if the slot is latent
	inline bool isSlotLatent( INVENTORIES::TInventory inventory, uint32 slot ) const
	{

		bool inHand = false;
		if (inventory == INVENTORIES::handling)
			inHand = true;
		else
			nlassert(inventory == INVENTORIES::equipment);
			
		std::list<CGearSlot>::const_iterator it = _GearLatencies.begin();
		for (; it != _GearLatencies.end(); ++it)
		{
			if ( (*it).InHand == inHand && (*it).Slot == slot )
				return true;
		}
		return false;
	}

	/// return true if there are latent gears
	inline bool isLatent() const
	{
		return !_GearLatencies.empty();
	}

private:
	struct CGearSlot
	{
		/// date of the end of the latency
		NLMISC::TGameCycle	LatencyEndDate;
		/// true if the item is in hand (otherwise, on the body)
		bool				InHand;
		/// slot (position) of the item in the inventory
		uint32				Slot;
	};

	/// list of affected slots, sorted by increasing latency end dates
	std::list<CGearSlot>	_GearLatencies;	
};


#endif // RY_GEAR_LATENCY_H

/* End of gear_latency.h */


