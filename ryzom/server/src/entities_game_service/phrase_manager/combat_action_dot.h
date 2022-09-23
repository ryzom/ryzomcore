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



#ifndef RY_COMBAT_ACTION_DOT_H
#define RY_COMBAT_ACTION_DOT_H


#include "combat_action.h"
#include "game_share/effect_families.h"
#include "game_share/damage_types.h"
#include "game_share/scores.h"

class CCombatPhrase;

class CCombatActionDoT : public CCombatAction
{
public:
	CCombatActionDoT()
	{
		_EffectDuration = 0;
	}

	/// build from an ai action
	virtual bool initFromAiAction( const CStaticAiAction *aiAction, CCombatPhrase *phrase );

	// validate
	virtual bool validate(CCombatPhrase * phrase, std::string &errorCode) 
	{ return true; }

	// apply
	virtual void apply(CCombatPhrase *phrase);

	/// apply on entity
	virtual void applyOnEntity( CEntityBase *entity, float successFactor );
protected:	
	/// apply combat action effects on target
	void applyOnTarget( uint8 targetIndex, CCombatPhrase *phrase);

protected:
	/// effect family
	EFFECT_FAMILIES::TEffectFamily _EffectFamily;
	/// effect duration
	NLMISC::TGameCycle		_EffectDuration;
	/// UpdateFrequency
	NLMISC::TGameCycle		_UpdateFrequency;
	/// Affected score
	SCORES::TScores			_AffectedScore;
	/// effect param value
	sint32					_TotalDamageValue;
	/// damage type
	DMGTYPE::EDamageType	_DamageType;
};


#endif // RY_COMBAT_ACTION_DOT_H

/* End of combat_action_dot.h */





















