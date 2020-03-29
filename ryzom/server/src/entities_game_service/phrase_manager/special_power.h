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



#ifndef RYZOM_SPECIAL_POWER_H
#define RYZOM_SPECIAL_POWER_H

//
#include "entity_manager/entity_base.h"
//
#include "game_share/power_types.h"

class CSpecialPowerPhrase;


/**
 * Base class for special powers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPower
{
	NL_INSTANCE_COUNTER_DECL(CSpecialPower);
public:
	/// Constructor
	CSpecialPower() : _Phrase(NULL), _ApplyOnTargets(true), _PowerType(POWERS::UnknownType), _DisablePowerTime(0), _ByPassDisablePowerTimer(false)
	{}

	/// validate the power utilisation
	virtual bool validate(std::string &errorCode);

	/// apply effects
	virtual void apply() {}

	/// apply on targets or on actor
	inline bool applyOnTargets() const { return _ApplyOnTargets; }
	inline void applyOnTargets(bool b) { _ApplyOnTargets = b; }

	/// set ByPass DisablePowerTimer
	inline void setByPass(bool flag) { _ByPassDisablePowerTimer = flag; }

protected:
	/// actor
	TDataSetRow			_ActorRowId;

	/// flag indicating if the action is done on the combat phrase targets or actor
	bool				_ApplyOnTargets;

	/// special power type
	POWERS::TPowerType	_PowerType;

	// disable power for x ticks
	NLMISC::TGameCycle	_DisablePowerTime;

	/// related phrase
	CSpecialPowerPhrase	*_Phrase;

	bool				_ByPassDisablePowerTimer;
};

/**
 * Base class for special powers
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerAuras : public CSpecialPower
{
public:
	/// Constructor
	CSpecialPowerAuras() : CSpecialPower()
	{
		_TargetsDisableAuraTime = 0;
		_AuraRadius = 0.0f;
		_AffectGuild = false;
		_ByPassTargetsDisableAuraTime = false;
	}

	/// validate the power utilisation
	virtual bool validate(std::string &errorCode);

	// getRadius
	inline float getRadius() const { return _AuraRadius; }

	// setRadius
	inline void setRadius(float radius) { _AuraRadius = radius; }

	/// set affect guild flag
	inline void affectGuild(bool flag) { _AffectGuild = flag; }

	/// set ByPass TargetsDisableAuraTime
	inline void setByPass(bool flag) { _ByPassTargetsDisableAuraTime = flag; }
	
protected:
	// disable this aura on targets for x ticks
	NLMISC::TGameCycle	_TargetsDisableAuraTime;
	// radius of the aura
	float				_AuraRadius;
	/// affect guild members ? (always affect teammates)
	bool				_AffectGuild;

	bool				_ByPassTargetsDisableAuraTime;

};

#endif // RYZOM_SPECIAL_POWER_H

/* End of special_power.h */
