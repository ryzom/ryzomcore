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



#ifndef RY_SHOOT_AGAIN_EFFECT_H
#define RY_SHOOT_AGAIN_EFFECT_H

// game share
#include "game_share/scores.h"
//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/** Class for shoot again proc buff (the first spell cast when this buff is in
 * effect consumes the effect)
 * \author Jerome Vuarand
 * \author Nevrax France
 * \date 2005
 * 
 * This class is inspired from CEnchantWeaponEffect and thus may have residual
 * side effects from the copy'n'paste.
*/
class CShootAgainEffect
: public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CShootAgainEffect)

	CShootAgainEffect(
		TDataSetRow const& creatorRowId,
		TDataSetRow const& targetRowId,
		EFFECT_FAMILIES::TEffectFamily family,
		sint32 effectValue,
		uint32 power,
		NLMISC::TGameCycle endDate)
	: CSTimedEffect(creatorRowId, targetRowId, family, false, effectValue, power, endDate)
	{
	}
	
	/// apply the effect
	virtual bool update(CTimerEvent * event, bool applyEffect);
	
	/// callback called when the effect is actually removed
	virtual void removed();
	
	virtual NLMISC::CSheetId getAssociatedSheetId() const;

	virtual bool automaticallyReplaceFamily() const { return true; }
	
	virtual bool canBeInactive() const { return false; }

private:
	// private ctor for use in NLMISC class registry
	CShootAgainEffect() {}
};

#endif
