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



#ifndef RY_MULTI_TARGET_H
#define RY_MULTI_TARGET_H

#include "entity_types.h"

const float MULTI_TARGET_DISTANCE_UNIT = 100.f / 127.f;

/** A list of targets, which can be packed/unpacked in visual properties (see PROPERTY_TARGET_LIST_x in entity_types.h)
  *
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2003
  */
class CMultiTarget
{
public:
	// a single target
	class CTarget
	{
		public:
			uint8 TargetSlot;	// the slot that is targetted
			uint8 Info;         // Damage shield for melee  5:3 format (damage shield io : power )
			                    // Distance for range attacks (that do not have damage shield) format is 7:1
		public:
			CTarget(uint8 slot = CLFECOMMON::INVALID_SLOT, bool resist = false, uint8 dist = 0) : TargetSlot(slot)
			{
				Info = (resist == 0 ? 0 : 0x80) | (dist & 0x7f);
			}
			uint16 getPacked() const;
			void   setPacked(uint16 value);
	};
	typedef std::vector<CTarget> TTargetVect;
	// the list of targets
	TTargetVect Targets;
public:
	/** create packed version of targets (to store in visual properties)
	  * each VP encodes 4 targets
	  * caller must provide enough room to store the result (assertion is raised otherwise)
	  */
	void pack(uint64 *destVP, uint numVP);
	// build the target from their packed version
	void unpack(const uint64 *srcVP, uint numVP);
};

////////////
// INLINE //
////////////

// *******************************************************************************************
inline uint16 CMultiTarget::CTarget::getPacked() const
{
	//return (uint16) TargetSlot | ((uint16) (Resist ? 1 : 0) << 15) | ((uint16) Distance << 8);
	return (uint16) TargetSlot | (uint16(Info) << 8);
}

// *******************************************************************************************
inline void   CMultiTarget::CTarget::setPacked(uint16 value)
{
	TargetSlot = uint8(value & 0xff);
	/*Distance = (uint8) ((value >> 8) & 0x7f);
	Resist = (value & 0x8000) != 0;*/
	Info = value >> 8;
}

#endif
