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
	nlassert(numVP * 2 >= Targets.size()); // not enough room to stores visual properties!
	CTarget  invalidTarget;
	uint index = 0;
	for(uint k = 0; k < numVP; ++k)
	{
		destVP[k] = index < Targets.size() ? Targets[index].getPacked() : invalidTarget.getPacked();
		++index;

		destVP[k] |= uint64(index < Targets.size() ? Targets[index].getPacked() : invalidTarget.getPacked()) << 32;
		++index;
	}
}

// ***********************************************************************
void CMultiTarget::unpack(const uint64 *srcVP, uint numVP)
{
	Targets.clear();
	for(uint k = 0; k < numVP; ++k)
	{
		CTarget t;
		t.setPacked(uint32(srcVP[k]));
		if(t.TargetSlot == CLFECOMMON::INVALID_SLOT)
			return;
		Targets.push_back(t);

		t.setPacked(uint32(srcVP[k] >> 32));
		if(t.TargetSlot == CLFECOMMON::INVALID_SLOT)
			return;
		Targets.push_back(t);
	}
}
