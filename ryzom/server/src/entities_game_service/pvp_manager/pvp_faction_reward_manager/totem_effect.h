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

#ifndef TOTEM_EFFECT_H
#define TOTEM_EFFECT_H

#include "phrase_manager/s_effect.h"
#include "game_share/characteristics.h"

/**
 * Class representing an effect produced 
 * by a totem in faction PVP
 * \author Gregorie Diaconu
 * \author Nevrax France
 * \date 2005
 */
class CTotemEffect : public CSEffect
{
public :
	///\ctor
	CTotemEffect(	const TDataSetRow & creatorRowId,
					const TDataSetRow & targetRowId,
					EFFECT_FAMILIES::TEffectFamily family,
					sint32 effectValue
				) : CSEffect( creatorRowId, targetRowId, family, false, effectValue, 0)
	{}

	/**
	 * apply the effects of the... effect
	 * \return true if effects ends
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);
	
	/// callback called when the effect is actually removed
	virtual void removed();

	/// change the parameter value
	inline void setParamValue( sint32 value ) { _Value = value; } 
};


/**
 * Class representing an effect produced 
 * by a totem in faction PVP which affects physical 
 * characteristics
 * \author Gregorie Diaconu
 * \author Nevrax France
 * \date 2005
 */
class CTotemCharacEffect : public CTotemEffect
{
private :
	CHARACTERISTICS::TCharacteristics	_AffectedCharac;
	
public:
	///\ctor
	CTotemCharacEffect(	const TDataSetRow & creatorRowId,
						const TDataSetRow & targetRowId,
						EFFECT_FAMILIES::TEffectFamily family,
						sint32 effectValue );

	/**
	 * apply the effects of the... effect
	 * \return true if effects ends
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);
	
	/// callback called when the effect is actually removed
	virtual void removed();
};

#endif // TOTEM_EFFECT_H
