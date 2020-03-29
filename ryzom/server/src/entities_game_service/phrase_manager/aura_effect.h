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



#ifndef RY_POWER_AURA_EFFECT_H
#define RY_POWER_AURA_EFFECT_H

//////////////
//	INCLUDE	//
//////////////
#include "nel/misc/variable.h"
//
#include "phrase_manager/s_effect.h"
#include "entity_manager/entity_base.h"
#include "entity_manager/entity_manager.h"
#include "phrase_manager/phrase_utilities_functions.h"
//
#include "game_share/power_types.h"


/**
 * Root Effect class for auras
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CAuraRootEffect : public CSTimedEffect
{
public:
	NLMISC_DECLARE_CLASS(CAuraRootEffect)

	///\ctor
	CAuraRootEffect( const TDataSetRow & creatorRowId, uint32 endDate, EFFECT_FAMILIES::TEffectFamily rootEffectFamily, EFFECT_FAMILIES::TEffectFamily effectFamily, POWERS::TPowerType powerType, uint32 paramValue, bool affectGuild)
		:CSTimedEffect(creatorRowId, creatorRowId, rootEffectFamily, false, paramValue, 0/*power*/, endDate), 
		_PowerType(powerType), _CreatedEffectFamily(effectFamily)
	{
		_AffectGuild = affectGuild;
		_IsFromConsumable = false;
#ifdef NL_DEBUG
		_LastUpdateDate = CTickEventHandler::getGameCycle();
#endif
	}

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect);

	/// callback called when the effect is actually removed
	virtual void removed();

	inline POWERS::TPowerType powerType() const { return _PowerType; }
	inline EFFECT_FAMILIES::TEffectFamily createdEffectFamily() const { return _CreatedEffectFamily; }

	// set radius
	inline void setRadius(float radius) { _AuraRadius = radius; }	
	// set disable time for targets
	inline void setTargetDisableTime(NLMISC::TGameCycle time) { _TargetDisableTime = time; }
	inline void setIsFromConsumable(bool fromConsumable) { _IsFromConsumable = fromConsumable; }

protected:
	/// create or update the real effect on target entity
	virtual void createEffectOnEntity(CEntityBase *entity, CEntityBase *creator);

	/// check if entity is a valid target for this effect
	virtual bool isEntityValidTarget(CEntityBase *entity, CEntityBase *creator);

private:
	// disableTime for targets
	NLMISC::TGameCycle		_TargetDisableTime;

	/// power type
	POWERS::TPowerType		_PowerType;

	/// aura radius in meters
	float					_AuraRadius;

	/// created effect family
	EFFECT_FAMILIES::TEffectFamily _CreatedEffectFamily;
	
	// is aura affecting guild members ? (teammates are always affected)
	bool					_AffectGuild;

	bool					_IsFromConsumable;

#ifdef NL_DEBUG
	NLMISC::TGameCycle		_LastUpdateDate;
#endif

	// private ctor for use in NLMISC class registry
	CAuraRootEffect() {}
};


/**
 * Base Effect class for auras
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CAuraBaseEffect : public CSTimedEffect
{
	NL_INSTANCE_COUNTER_DECL(CAuraBaseEffect);
public:
	NLMISC_DECLARE_CLASS(CAuraBaseEffect)

	///\ctor
	CAuraBaseEffect( const CAuraRootEffect &rootEffect, TDataSetRow targetRowId);

	/**
	 * apply the effects of the... effect
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect) { return false; }

	/// callback called when the effect is actually removed
	virtual void removed();

	/// add lifetime to this aura (add enough to keep it till next RootAuraEffect update)
	void addLifeTime();

	/// set effect sheetId
	inline void setEffectDisabledEndDate(NLMISC::TGameCycle date) { _DisabledEndDate = date; }	

protected:
	/// power type
	POWERS::TPowerType		_PowerType;

	// effect disable end date
	NLMISC::TGameCycle		_DisabledEndDate;

	// private ctor for use in NLMISC class registry
	CAuraBaseEffect() {}
};


/**
 * class factory for aura Effects, built from power type
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class IAuraEffectFactory
{
	NL_INSTANCE_COUNTER_DECL(IAuraEffectFactory);
public:

	/// clear the class factory
	static void clear();

	/**
	 * Build the effect class from a power type
	 * \param prim : the primitive node used to build the step
	 * \return a pointer on the built effect class (NULL if failure)
	 */
	inline static CAuraBaseEffect * buildEffectFromPowerType(POWERS::TPowerType powerType, const CAuraRootEffect &rootEffect, TDataSetRow targetRowId)
	{
		//get appropriate factory
		for ( uint i = 0; i < Factories->size(); i++ )
		{
			if ( (*Factories)[i].first == powerType )
			{
				INFOLOG(" Power type %s is managed by the system. Building action...",POWERS::toString(powerType).c_str());
				return (*Factories)[i].second->buildEffect(rootEffect, targetRowId);
			}
		}
		nlwarning( "<IAuraEffectFactory buildEffectFromPowerType> the powerType %s has no corresponding effect class", POWERS::toString(powerType).c_str() );
		return NULL;
	}
protected:
	///\init the factories
	inline static void init()
	{	
		if( !Factories )
			Factories = new std::vector< std::pair< POWERS::TPowerType , IAuraEffectFactory* > >;
	}
	/**
	 * Create a step from parameters
	 * \param rootEffect pointer on the root effect form which params are taken
	 * \return a pointer on the built effect (NULL if failure)
	 */
	virtual CAuraBaseEffect * buildEffect( const CAuraRootEffect &rootEffect, TDataSetRow targetRowId ) = 0;
	
	///the phrase factories. We use a pointer here because we cant control the order inwhich ctor of static members are called
	static std::vector< std::pair< POWERS::TPowerType , IAuraEffectFactory* > >* Factories;
};

/**
 * Aura effect template factory
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
template <class T> class CAuraEffectTFactory : public IAuraEffectFactory
{
public:
	explicit CAuraEffectTFactory(POWERS::TPowerType powerType)
	{
		IAuraEffectFactory::init();
		
#ifdef NL_DEBUG
		// check this type isn't used yet
		for (uint i = 0; i < Factories->size(); i++ )
		{
			if ( (*Factories)[i].first == powerType )
			{
				nlstop;
			}
		}
#endif
		// add factory
		Factories->push_back(std::make_pair( powerType ,this));
	};

	/// buildEffect method
	CAuraBaseEffect * buildEffect( const CAuraRootEffect &rootEffect, TDataSetRow targetRowId )
	{
		T *instance = new T(rootEffect, targetRowId);
		if (!instance)
		{
			nlwarning("<CAuraEffectTFactory> failed to allocate element of template type");
			return 0;
		}
		return instance;
	}
};

#endif // RY_POWER_AURA_EFFECT_H

/* End of aura_effect.h */
