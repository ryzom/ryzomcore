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



#ifndef CL_ANIMATION_FX_SHEET_H
#define CL_ANIMATION_FX_SHEET_H


#include "fx_stick_mode.h"
#include "entity_sheet.h"

/**
 * \author Matthieu 'Trap' Besson
 * \author Nevrax France
 * \date November 2003
 */
class CAnimationFXSheet : public CEntitySheet
{

public:

	enum TRepeatMode
	{
		Loop = 0,      // The fx is spawned at start of anim, and cut at end (emitters are shutdown). If the same anim is repeated, the fx isn't recreated.
		Respawn,       // The fx is spawned at start of anim, and is respawned if the same anim is playes again. The fx should shutdown by itself (must have a finite duration). In all case, it is removed after an arbitrary (long) timeout
		RespawnAndCut  // The fx is spawned at start of anim, and is respawned if the same anim is played again. It is shutdown at the end of the anim (its emitters are shutdown)
	};

public:

	std::string		PSName;
	CFXStickMode	StickMode;

	float			UserParam[4];		// user params of the particle system
	std::string		TrajectoryAnim;		// name of an anim that export position of the fx, or empty if no such anim is used
	NLMISC::CRGBA	Color;
	bool			ScaleFX;			// true if the fx must be scaled to match character size
	TRepeatMode		RepeatMode;
	float			RayRefLength;       // if object is to sticked in 'ray' mode, then give the reference on z  axis so that scale can apply

public:
	// Constructor
	CAnimationFXSheet(const std::string &psName = "", const float *userParams = NULL);
	/// from CEntitySheet
	virtual void build(const NLGEORGES::UFormElm &item);
	/// Build the fx from an external script.
	void build(const NLGEORGES::UFormElm &item, const std::string &prefix);
	/// Serialize a CAnimationFX.
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};


#endif
