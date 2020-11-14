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



#ifndef RY_CHG_CHARAC_EFFECT_H
#define RY_CHG_CHARAC_EFFECT_H

#include "phrase_manager/s_effect.h"

#include "game_share/persistent_data.h"

/**
 * Effect class affecting characteristics
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2005
 */
class CChgCharacEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CChgCharacEffect)
		
	DECLARE_PERSISTENCE_METHODS

	/// default ctor
	CChgCharacEffect() : CSTimedEffect() {}

	///\ctor
	CChgCharacEffect( const TDataSetRow & creatorRowId, uint32 endDate, EFFECT_FAMILIES::TEffectFamily effectFamily, std::string& affectedCharac, float modifier1, float modifier2)
		:CSTimedEffect(creatorRowId, creatorRowId, effectFamily, true, 0/*paramvalue*/, 0/*power*/, endDate)
	{
#ifdef NL_DEBUG
		_LastUpdateDate = CTickEventHandler::getGameCycle();
#endif
		_Modifier1 = modifier1;
		_Modifier2 = modifier2;
		_AffectedCharac = affectedCharac;
	}
	
	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

	// set disable time for targets
	inline void setTargetDisableTime(NLMISC::TGameCycle time) { _TargetDisableTime = time; }

	// re-activate effect after loaded with a character
	void activate();

private:
	// disableTime for targets
	NLMISC::TGameCycle	_TargetDisableTime;
	
	/// affected characteristics
	std::string			_AffectedCharac;

	/// characteristics modifiers
	float				_Modifier1;
	float				_Modifier2;

	NLMISC::CEntityId	_CreatorEntityId;

#ifdef NL_DEBUG
	NLMISC::TGameCycle	_LastUpdateDate;
#endif
};


#endif // RY_CHG_CHARAC_EFFECT_H

/* End of chg_charac_effect.h */
