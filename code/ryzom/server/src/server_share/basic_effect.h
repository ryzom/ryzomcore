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



#ifndef RY_BASIC_EFFECT_H
#define RY_BASIC_EFFECT_H

#include "nel/misc/types_nl.h"
#include "game_share/base_types.h"
#include "game_share/effect_families.h"


/**
 * <Class description>
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CBasicEffect
{
public:
	/// Constructor
	CBasicEffect(EFFECT_FAMILIES::TEffectFamily family, const TDataSetRow & creatorId, const TDataSetRow & targetRowId) 
	: _CreatorRowId(creatorId), _TargetRowId(targetRowId), _Family(family), _EffectId(0)
	{
		_EffectId = ++_EffectCounter;
	}

	CBasicEffect( EFFECT_FAMILIES::TEffectFamily family, const TDataSetRow & creatorId, const TDataSetRow & targetRowId, uint32 effectId ) 
		: _CreatorRowId(creatorId), _TargetRowId(targetRowId), _Family(family), _EffectId(effectId)
	{
	}



	/// Destructor
	virtual ~CBasicEffect() {}

	/// get the effect creator Id
	inline const TDataSetRow &creatorRowId() const { return _CreatorRowId; }
	
	/// get the effect target rowId
	inline const TDataSetRow &targetRowId() const { return _TargetRowId; }

	/// get the effect family
	inline EFFECT_FAMILIES::TEffectFamily family() const { return _Family; }

	/// get the effect Id
	inline uint32 effectId() const { return _EffectId; }

	/// get the effect counter
	inline static uint32 effectCounter() { return _EffectCounter; }

protected:
	/// effect creator Id
	TDataSetRow						_CreatorRowId;

	/// effect target Id
	TDataSetRow						_TargetRowId;

	/// effect family
	EFFECT_FAMILIES::TEffectFamily	_Family;

	/// effect ID
	uint32							_EffectId;

	/// counter, increased by one with every effect creation, give a unique Id per effect
	static uint32					_EffectCounter;
};


#endif // RY_BASIC_EFFECT_H

/* End of basic_effect.h */
