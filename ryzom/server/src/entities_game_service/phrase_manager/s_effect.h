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

#include "nel/misc/class_registry.h"
#include "game_share/effect_families.h"
#include "game_share/damage_types.h"
#include "game_share/base_types.h"
#include "game_share/timer.h" 
#include "game_share/persistent_data.h"

class CSEffect;
typedef NLMISC::CSmartPtr<CSEffect> CSEffectPtr;

/**
 * Class representing an event triggered by an effect, used to update effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2004
 */
class CUpdateEffectTimerEvent: public CTimerEvent
{
public:
	CUpdateEffectTimerEvent(CSEffect* parent, bool firstUpdate = false)
	{
		nlassert(parent);
		_FirstUpdate = firstUpdate;
		_Parent = parent;
	}
	
	void timerCallback(CTimer* owner);

private:
	/// Use smart pointer because an effect can remove itself (if it kills a creature)
	CSEffectPtr	_Parent;
	bool		_FirstUpdate;
};

/**
 * Class representing an event triggered by an effect, used to end effects
 * \author David Fleury
 * \author Nevrax France
 * \date 2004
 */
class CEndEffectTimerEvent: public CTimerEvent
{
	NL_INSTANCE_COUNTER_DECL(CEndEffectTimerEvent);
public:

	CEndEffectTimerEvent(CSEffect* parent)
	{
		nlassert(parent);
		_Parent = parent;
	}
	
	void timerCallback(CTimer* owner);

private:
	/// Use smart pointer because an effect can remove itself (if it kills a creature)
	CSEffectPtr _Parent;
};

/**
 * Class representing an effect managed by the EGS
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2003
 */
class CSEffect : public NLMISC::CRefCount
{
private:
	friend class CUpdateEffectTimerEvent;
	friend class CEndEffectTimerEvent;

	NL_INSTANCE_COUNTER_DECL(CSEffect);
public:
	DECLARE_VIRTUAL_PERSISTENCE_METHODS

	///\ctor
	CSEffect()
		:_IsRemoved(false),_Value(0),_Power(0),_IsFromConsumable(false)
	{
		_UpdateTimer.setRemaining(1, new CUpdateEffectTimerEvent(this, true));
		_Skill = SKILLS::unknown;
		_IsStackable = false;
		_EffectIndexInDB = -1;
		++NbAllocatedEffects;
	}
	
	inline CSEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family, bool stackable, sint32 effectValue, uint32 power)
		:_CreatorRowId(creatorRowId),_TargetRowId(targetRowId),_Family(family),_Value(effectValue),_Power(power),_IsStackable(stackable),_IsRemoved(false),_IsFromConsumable(false)
	{
		++NbAllocatedEffects;
		_EffectChatName = EFFECT_FAMILIES::getAssociatedChatId(family); // txt msg
		_UpdateTimer.setRemaining(1, new CUpdateEffectTimerEvent(this, true));
		_Skill = SKILLS::unknown;
		_EffectIndexInDB = -1;
	}

	virtual ~CSEffect()
	{
		++NbDesallocatedEffects;
		// reset timer to cancel any timer still running
		_UpdateTimer.reset();
		_EndTimer.reset();
	}

	/**
	 * apply the effects of the... effect
	 * \return true if effects ends
	 */
	virtual bool update(CTimerEvent * event, bool applyEffect) {return false;};

	/**
	 * stop the effect, may leads to effect destruction
	 */
	void stopEffect()
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
		_UpdateTimer.reset();
	}

	/**
	 * reactivate effect, force effect update
	 */
	void forceUpdate(bool forceApply = true)
	{
		_UpdateTimer.reset();
		NLMISC::CSmartPtr<CTimerEvent> eventPtr = new CUpdateEffectTimerEvent(this);
		update( eventPtr, forceApply );
	}

	// callback called when the effect is actually removed. Does nothing by default
	virtual void removed(){}

	///\name read accessors
	//@{
	inline uint32							getEffectId()		const{ return	_EffectId;}
	inline EFFECT_FAMILIES::TEffectFamily	getFamily()			const{ return	_Family;}
	inline const TDataSetRow &				getCreatorRowId()	const{ return	_CreatorRowId;}
	inline const TDataSetRow &				getTargetRowId()	const{ return	_TargetRowId;}
	inline sint32							getParamValue()		const{ return	_Value;}
	inline uint32							getPower()			const{ return	_Power;}
	inline SKILLS::ESkills					getSkill()			const{ return	_Skill; }
	virtual NLMISC::CSheetId				getAssociatedSheetId() const
	{
		if (_IsFromConsumable)
			return NLMISC::CSheetId("hatred.sbrick");
		else
			return EFFECT_FAMILIES::getAssociatedSheetId(_Family);
	}
	//@}

	///\name write accessors
	//@{
	inline void	setFamily(EFFECT_FAMILIES::TEffectFamily family)
	{ 
		_Family = family; 
		_EffectChatName = EFFECT_FAMILIES::getAssociatedChatId(family); 
	}

	inline void	setCreatorRowId(const TDataSetRow &id)			{ _CreatorRowId = id; }
	inline void	setTargetRowId(const TDataSetRow &id)			{ _TargetRowId = id; }
	inline void setParamValue(sint32 value)						{ _Value = value; }
	inline void setPower(uint32 pow)							{ _Power = pow; }
	inline void setSkill(SKILLS::ESkills skill)					{ _Skill = skill; }
	//@}
	
	///\set the id of the effect. Should be set by the effect manager
	inline void setEffectId(uint32 id)	{ _EffectId = id; }
	
	/// set effect index in DB
	inline void setEffectIndexInDB(sint8 index) { _EffectIndexInDB =index; }
	/// get effect index in DB
	inline sint8 getEffectIndexInDB() const { return _EffectIndexInDB; }

	/// returns true if the effect is stackable
	inline bool isStackable() const { return _IsStackable; }
	/// set if the effect is stackable
	inline void isStackable(bool b) { _IsStackable = b; }

	/// true if newer effect automatically prevails when launching several of the same family
	virtual bool automaticallyReplaceFamily() const { return false; }
	
	/// true if the effect can be inactive
	virtual bool canBeInactive() const { return true; }
	
	/// returns true if the effect has been removed from affected entity
	inline bool isRemoved() const { return _IsRemoved; }

	/// set if the effect has been removed from affected entity
	inline void isRemoved(bool removed) { _IsRemoved = removed; }

	inline void setIsFromConsumable(bool fromConsumable) { _IsFromConsumable = fromConsumable; }
	inline bool getIsFromConsumable() { return _IsFromConsumable; }


protected:
	/// send chat message for effect begin
	void sendEffectBeginMessages();
	/// send chat message for effect end
	void sendEffectEndMessages();
	/// called when the effect ends, remove the effect from affected entity
	virtual void endEffect();

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
	/// power of the effect
	uint32							_Power;
	/// related skill
	SKILLS::ESkills					_Skill;
	/// sheetid related to this effect (to display on client side)
	NLMISC::CSheetId				_EffectSheetId;
	// index of effect in DB
	sint8							_EffectIndexInDB;
	
	/// effect name (used to send chat messages to client)
	std::string						_EffectChatName;
	
	/// true if effect is stackable
	bool							_IsStackable;

	/// true if the effect has been removed from affected entity
	bool							_IsRemoved;

	/// timer used to update the effect
	CTimer							_UpdateTimer;
	/// timer used to end the effect
	CTimer							_EndTimer;

	bool							_IsFromConsumable;


public:
	static uint32					NbAllocatedEffects;
	static uint32					NbDesallocatedEffects;
};

class CSTimedEffect : public CSEffect , public NLMISC::IClassable
{
public:
//	NLMISC_DECLARE_CLASS(CSTimedEffect)

	DECLARE_VIRTUAL_PERSISTENCE_METHODS

	CSTimedEffect() : CSEffect()
	{
		_EndTimer.setRemaining(1, new CEndEffectTimerEvent(this));
	}

	CSTimedEffect( const TDataSetRow & creatorRowId, const TDataSetRow & targetRowId, EFFECT_FAMILIES::TEffectFamily family,bool stackable, sint32 effectValue, uint32 power, uint32 endDate)
		:CSEffect(creatorRowId,targetRowId,family,stackable,effectValue,power), _EndDate(endDate)
	{
		_EndTimer.set(_EndDate, new CEndEffectTimerEvent(this));
	}

	/// set endDate
	inline void setEndDate(NLMISC::TGameCycle date) 
	{ 
		_EndDate = date; 
		if (_EndTimer.getEvent() != NULL)
			_EndTimer.set(date, _EndTimer.getEvent()); 
		else
			_EndTimer.set(date, new CEndEffectTimerEvent(this));
	}

	// reset end timer (only for store/apply process, do not use in normal effect usage (use setEndDate() or stopEffect())
	void disableEvent() 
	{ 
		NLMISC::TGameCycle date = CTickEventHandler::getGameCycle() + 9999999;
		_EndTimer.reset(); 
		_UpdateTimer.reset(); 
		_EndTimer.set(date, new CEndEffectTimerEvent(this));
		_UpdateTimer.set(date, new CEndEffectTimerEvent(this));
	}

	// get endDate
	inline NLMISC::TGameCycle getEndDate() { return _EndDate; }

	// virtual method for re-activate an effect from character save file
	virtual void activate() {}

protected:
	/// effect end date
	NLMISC::TGameCycle			_EndDate;
};


#endif // RY_S_EFFECT_H


/* End of s_effect.h */
