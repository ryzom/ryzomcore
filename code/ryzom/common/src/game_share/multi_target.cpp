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
#include "multi_target.h"

// ***********************************************************************
void CMultiTarget::pack(uint64 *destVP, uint numVP)
{
	nlassert(numVP * 4 >= Targets.size()); // not enough room to stores visual properties!
	CTarget  invalidTarget;
	uint index = 0;
	for(uint k = 0; k < numVP; ++k)
	{
		uint16 parts[4];
		for(uint l = 0; l < 4; ++l)
		{
			parts[l] = index < Targets.size() ? Targets[index].getPacked() : invalidTarget.getPacked();
			++ index;
		}
		destVP[k] = (uint64) parts[0] |  ((uint64) parts[1] << 16) | ((uint64) parts[2] << 32) | ((uint64) parts[3] << 48);
	}
}

// ***********************************************************************
void CMultiTarget::unpack(const uint64 *srcVP, uint numVP)
{
	Targets.clear();
	for(uint k = 0; k < numVP; ++k)
	{
		for(uint l = 0; l < 4; ++l)
		{
			CTarget t;
			t.setPacked((uint16) ((srcVP[k] >> (16 * l)) & 0xffff));
			if (t.TargetSlot != CLFECOMMON::INVALID_SLOT)
			{
				Targets.push_back(t);
			}
			else
			{
				return;
			}
		}
	}
}
