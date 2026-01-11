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



#ifndef RY_DAMAGE_AURA_EFFECT_H
#define RY_DAMAGE_AURA_EFFECT_H

//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"



/**
 * class for damage auras effects (like Stench)
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CDamageAuraEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CDamageAuraEffect)

	///\ctor
	CDamageAuraEffect() : CSTimedEffect()
	{
		_AffectAttackableEntities = false;
		_AffectPlayers = true;
		_AuraRadius = 0.0f;
	}

	///\ctor
	CDamageAuraEffect( const TDataSetRow & creatorRowId, 
						const TDataSetRow & targetRowId, 
						EFFECT_FAMILIES::TEffectFamily family, 
						sint32 effectValue, 
						NLMISC::TGameCycle endDate,
						NLMISC::TGameCycle cycleLenght,
						uint16 cycleDamage,
						float radius = 5.0f
						)
		:	CSTimedEffect(creatorRowId, targetRowId, family, true, effectValue,(uint8)cycleDamage, endDate),
			_CycleLength(cycleLenght),
			_CycleDamage(cycleDamage)
	{
		_AffectAttackableEntities = false;
		_AffectPlayers = true;
		_AuraRadius = radius;
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();
	/// set radius
	inline void setRadius(float radius) { _AuraRadius = radius; }	

	/// accessors on _AffectAttackableEntities
	void affectAttackableEntities(bool b) { _AffectAttackableEntities = b; }
	bool affectAttackableEntities() const { return _AffectAttackableEntities; }

	void affectPlayers(bool b) { _AffectPlayers = b; }
	bool affectPlayers() const { return _AffectPlayers; }

private:
	/// check entity is a valid target
	bool isEntityValidTarget(CEntityBase *entity, CEntityBase *actor) const;

private:
	/// cycle lenght in ticks
	NLMISC::TGameCycle		_CycleLength;

	/// number of hp lost by surrounding entities each cycle
	uint16					_CycleDamage;

	/// aura radius in meters
	float					_AuraRadius;

	/// affect players ?
	bool					_AffectPlayers;
	/// affect attackable entities
	bool					_AffectAttackableEntities;

	/// affected entity
	mutable CEntityBaseRefPtr	_AffectedEntity;
};


#endif // RY_DAMAGE_AURA_EFFECT_H

/* End of damage_aura_effect.h */
