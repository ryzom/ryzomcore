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



#ifndef RY_VISUAL_FX_H
#define RY_VISUAL_FX_H

#include "magic_fx.h"

/** Class to unpack / pack visual fx infos from a visual property
  */
class CVisualFX
{
public:
	bool			 AuraReceipt;
	MAGICFX::TAuraFX Aura;
	uint			 Link; // match the MAGICFX::TMagicFx enum, but with 1 added (0 means no link)
public:
	// build from a visual property
	void unpack(sint64 src);
	// build a visual property from that object
	void pack(sint64 &dest);
};

/////////////
// INLINES //
/////////////

// **********************************************************************************************
inline void	CVisualFX::unpack(sint64 vp)
{
	Aura = (MAGICFX::TAuraFX) (vp & 31);
	Link = (uint) ((vp >> 5) & 31);
	AuraReceipt = (vp & (1 << 10)) != 0;
}

// **********************************************************************************************
inline void CVisualFX::pack(sint64 &dest)
{
	dest = ((sint64) AuraReceipt << 10) | ((sint64) (Aura & 31)) | ((sint64) (Link & 31) << 5);
}

#endif
