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




#ifndef RY_S_EFFECT_H
#define RY_S_EFFECT_H


#include "nel/misc/types_nl.h"
#include "game_share/effect_families.h"
#include "game_share/damage_types.h"
#include "game_share/base_types.h"

/**
 * Class representing an effect managed by the EGS
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CSEffect
{
public:

	///\ctor
	inline CSEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family, sint32 effectValue, uint8 power)
		:_CreatorRowId(creatorRowId),_TargetRowId(targetRowId),_Family(family),_Value(effectValue),_Power(power)
	{
	}

	/**
	 *  return true if it is time to update the effect. It modifies the next update of the effect
	 */
	virtual bool isTimeToUpdate() = 0;

	/**
	 * apply the effects of the... effect
	 * \param updateFlag is a flag telling which effect type has been already processed for an entity. An effect shoud set to 1 the bit corresponding to its effect family
	 * \return true if the effect ends must be removed
	 */
	virtual bool update( uint32 & updateFlag ) = 0;

	/// callback called when the effect is actually removed. Does nothing by default
	virtual void removed(){}

	///\name accessors
	//@{
	inline uint32							getEffectId()		const{ return	_EffectId;}
	inline EFFECT_FAMILIES::TEffectFamily	getFamily()			const{ return	_Family;}
	inline const TDataSetRow &				getCreatorRowId()	const{ return	_CreatorRowId;}
	inline const TDataSetRow &				getTargetRowId()	const{ return	_TargetRowId;}
	inline sint32							getParamValue()		const{ return	_Value;}
	inline uint8							getPower()			const{ return	_Power;}
	//@}
	
	///\set the id of the effect. Should be set by the effect manager
	inline void setEffectId(uint32 id)	{ _EffectId = id; }

protected:
	/// effect creator Id
	TDataSetRow						_CreatorRowId;
	/// effect target Id
	TDataSetRow						_TargetRowId;
	/// effect family
	EFFECT_FAMILIES::TEffectFamily	_Family;
	/// effect ID
	uint32							_EffectId;
	/// effect Value
	sint32							_Value;
	/// power of the effect (to be counter with cures)
	uint8							_Power;
};




#endif // RY_S_EFFECT_H


/* End of s_effect.h */
