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



#ifndef RY_S_LINK_EFFECT_H
#define RY_S_LINK_EFFECT_H

#include "nel/misc/variable.h"
#include "phrase_manager/s_effect.h"
#include "magic_focus_item.h"
#include "progression/progression_pve.h"
#include "game_share/scores.h"
#include "game_share/skills.h"
#include "game_share/magic_fx.h"

class CSLinkEffect : public CSEffect
{
public:
	inline CSLinkEffect( 
		const TDataSetRow & creatorRowId, 
		const TDataSetRow & targetRowId,
		EFFECT_FAMILIES::TEffectFamily family, 
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill,
		uint32 maxDistance,
		sint32 value,
		uint32 power,
		TReportAction& report )
		:CSEffect(creatorRowId,targetRowId,family,false,value,power),
		_CostPerUpdate(cost),
		_EnergyCost(energyCost),
		_MaxDistance(maxDistance),
		_Report(report),
		_NoLinkSurvivalTime(0),
		_LinkExists(true),
		_SpellPower(0),
		_PhraseBookIndex(0)
	{
		_MagicFxType = MAGICFX::toMagicFx(family);
		_EndTimer.reset();
		setSkill(skill);
	}

	virtual bool update(CTimerEvent * event, bool applyEffect);
	
	virtual void removed();
	
	/// method called when the link is broken
	inline void breakLink(float factorOnSurvivalTime = 1.0f) 
	{ 
		if (factorOnSurvivalTime < 0.0f)
			factorOnSurvivalTime = 0.0f;
		
		_LinkExists = false; 
		const NLMISC::TGameCycle duration = NLMISC::TGameCycle( factorOnSurvivalTime * (_NoLinkSurvivalTime + CSLinkEffect::getNoLinkDurationTime(_Family)) );
		_EndTimer.setRemaining( duration , new CEndEffectTimerEvent(this));
	}

	/// set _NoLinkSurvivalTime
	inline void setNoLinkSurvivalTime(NLMISC::TGameCycle time) { _NoLinkSurvivalTime = time; }

	/// get _MagicFxType
	inline MAGICFX::TMagicFx getMagicFxType() const { return _MagicFxType; }

	/// set spell power
	inline void setSpellPower(uint16 spellPower) { _SpellPower = spellPower; }

	/// set phrase ID
	inline void setPhraseBookIndex(uint16 id) { _PhraseBookIndex = id; }
	/// get phrase ID
	inline uint16 getPhraseBookIndex() const { return _PhraseBookIndex; }

	/// get update period for given effect family
	static NLMISC::TGameCycle getUpdatePeriod( EFFECT_FAMILIES::TEffectFamily family);

	/// get cost per update
	inline sint32 costPerUpdate() const { return _CostPerUpdate; }

private:
	static uint32 getNoLinkDurationTime( EFFECT_FAMILIES::TEffectFamily family);
	
protected:
	sint32					_CostPerUpdate;
	SCORES::TScores			_EnergyCost;
	TReportAction			_Report;
	// max link distance in mm
	uint32					_MaxDistance;
	/// duration of the effect once the link is broken
	NLMISC::TGameCycle		_NoLinkSurvivalTime;
	///
	bool					_LinkExists;
	/// associated magic fx type
	MAGICFX::TMagicFx		_MagicFxType;
	/// used magic focus if any
	CMagicFocusItemFactor	_Focus;
	/// original link spell power
	uint16					_SpellPower;
	///	index in client phrase book (0 = not in the phrase book)
	uint16					_PhraseBookIndex;
};


typedef NLMISC::CSmartPtr<CSLinkEffect> CSLinkEffectPtr;

class CSLinkEffectOffensive : public CSLinkEffect
{
public:
	inline CSLinkEffectOffensive( const TDataSetRow & creatorRowId, 
		const TDataSetRow & targetRowId,
		EFFECT_FAMILIES::TEffectFamily family, 
		sint32 cost,
		SCORES::TScores energyCost,
		SKILLS::ESkills skill,
		uint32 maxDistance,
		sint32 value,
		uint8 power,
		TReportAction& report )
		:CSLinkEffect(creatorRowId,targetRowId,family,cost,energyCost,skill,maxDistance,value,power,report),_ResistFactor(0.0f)
	{
		_FirstResist = true;
		_Report.ActionNature = ACTNATURE::OFFENSIVE_MAGIC;
	}
	
	virtual bool update(CTimerEvent * event, bool)
	{
		return updateOffensive(event, true);
	}
	
	bool updateOffensive(CTimerEvent * event, bool sendReportForXP);
	
protected:
	float	_ResistFactor;
	/// first resist (the first time, do not test resist)
	bool	_FirstResist;
};

#endif // RY_S_LINK_EFFECT_H

/* End of s_link_effect.h */
