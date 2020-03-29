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



#ifndef RY_MAGIC_ACTION_AI_TOXIC_CLOUD_H
#define RY_MAGIC_ACTION_AI_TOXIC_CLOUD_H


#include "magic_action.h"
#include "game_share/damage_types.h"
#include "game_share/scores.h"

class CMagicPhrase;

class CMagicAiActionToxicCloud : public IMagicAction
{
public:
	CMagicAiActionToxicCloud()
	{
		_EffectDuration = 0;
		_Damage = 0;
	}

	/// build from an ai action
	virtual bool initFromAiAction( const CStaticAiAction *aiAction, CMagicPhrase *phrase );

protected:
	virtual bool addBrick( const CStaticBrick & brick, CMagicPhrase * phrase, bool &effectEnd, CBuildParameters &buildParams )
	{ return false; }
	
	virtual bool validate(CMagicPhrase * phrase, std::string &errorCode) { return true; }

	virtual void launch( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						 const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						 const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport );

	virtual void apply( CMagicPhrase * phrase, sint deltaLevel, sint skillLevel, float successFactor, MBEHAV::CBehaviour & behav,
						const std::vector<float> &powerFactors, NLMISC::CBitSet & affectedTargets, const NLMISC::CBitSet & invulnerabilityOffensive,
						const NLMISC::CBitSet & invulnerabilityAll, bool isMad, NLMISC::CBitSet & resists, const TReportAction & actionReport,
						sint32 vamp, float vampRatio, bool reportXp );

protected:
	/// effect duration
	NLMISC::TGameCycle		_EffectDuration;
	/// UpdateFrequency
	NLMISC::TGameCycle		_UpdateFrequency;
	/// damage type
	DMGTYPE::EDamageType	_DamageType;
	/// affected score
	SCORES::TScores			_AffectedScore;
	/// amount of damage dealt each update
	sint32					_Damage;
	/// cloud radius in meters
	float					_Radius;
};


#endif // RY_MAGIC_ACTION_AI_TOXIC_CLOUD_H

/* End of magic_action_ai_toxic_cloud.h */
