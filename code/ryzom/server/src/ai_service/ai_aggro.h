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

#ifndef AI_AGGRO_H
#define AI_AGGRO_H

#include "timer.h"
#include "ai.h"
#include "ai_place.h"

class CBotAggroOwner;
class CAIEntityPhysical;

//////////////////////////////////////////////////////////////////////////////
// CBotAggroEntry                                                           //
//////////////////////////////////////////////////////////////////////////////

/** A pair of aggro/entity values.
 * 
 * Aggro values are between 0 and 1. 0 means no aggro, 1 means full aggro.
 */
class CBotAggroEntry
: public NLMISC::CRefCount
{
public:	
	CBotAggroEntry(TDataSetRow const& bot, float aggro, CBotAggroOwner const& owner, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL));
	virtual ~CBotAggroEntry();
	
	float finalAggro() const { return _Aggro; }
	
	void addAggro(float aggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL));
	void decrementAggros(uint32 ticks);
	
	bool updateTime(uint32 const& ticks) const;
	
	TDataSetRow const& getBot() const { return _Bot; }
	NLMISC::CSmartPtr<CAIPlace const> getLastHitPlace() const { return _LastHitPlace; }
	
	void setMinimum(float aggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL));
	void setMaximum(float aggro);
	
	void scaleBy(float const& scale) { _Aggro *= scale; }
	
	void operator=(CBotAggroEntry const& other);
	
	virtual bool atPlace(CAIVector const& pos) const { if (_LastHitPlace) return _LastHitPlace->atPlace(pos); else return true; }
	virtual bool atPlace(CAIVectorMirror const& pos) const { if (_LastHitPlace) return _LastHitPlace->atPlace(pos); else return true; }
	virtual bool atPlace(CAIEntityPhysical const* entity) const { if (_LastHitPlace) return _LastHitPlace->atPlace(entity); else return true; }
	
private:
	CBotAggroOwner const&	_Owner;
	TDataSetRow				_Bot;
	float					_Aggro;
	NLMISC::CSmartPtr<CAIPlace const>	_LastHitPlace;
};

//////////////////////////////////////////////////////////////////////////////
// CBotAggroOwner                                                           //
//////////////////////////////////////////////////////////////////////////////

class CBotAggroOwner
{
public:
	typedef NLMISC::CSmartPtr<CBotAggroEntry> TAggroEntryPtr;
	typedef std::map<TDataSetRow, TAggroEntryPtr> TBotAggroList;
	
public:
	CBotAggroOwner();
	virtual ~CBotAggroOwner();
	
	static bool isAggroable(TDataSetRow const& dataSetRow);
	
	void update(uint32 ticks);
	
	/// @name Management of an aggro entry
	//@{
	/// Command used by fight script only
	void blockAggro(sint32 blockTime);
	void ignoreReturnAggro(bool ignored);
	virtual void maximizeAggroFor(TDataSetRow const& botRow);
	/// Command used by fight script only
	void minimizeAggroFor(TDataSetRow const& botRow);
	void addAggroFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL), bool transferAggro = true);
	// Sets the minimum value the aggro can have in [0;1]
	void setAggroMinimumFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place = NLMISC::CSmartPtr<CAIPlace const>(NULL), bool transferAggro = true);
	void forgetAggroFor(TDataSetRow const& bot, bool forgetDamages = false);
	bool isAggroValid(TDataSetRow const& bot);
	bool isNewAggroValid(TDataSetRow const& bot);
	bool haveAggro() const { return !_BotAggroList.empty(); }
	bool haveAggroWithEntity(const TDataSetRow& rowId) const;
	bool haveAggroOrReturnPlace() const { return haveAggro() || _FirstHitPlace!=NULL; }
	virtual RYAI_MAP_CRUNCH::CWorldPosition getAggroPos() const;
	virtual NLMISC::CEntityId getAggroOwnerEid() const;
	virtual NLMISC::CSmartPtr<CAIPlace const> buildFirstHitPlace(TDataSetRow const& aggroBot) const;
	virtual std::set<CBotAggroOwner*> getAggroGroup(bool primary) const { return std::set<CBotAggroOwner*>(); }
	virtual void propagateAggro() const { }
	
	virtual float getReturnDistCheck() const;
	virtual float getD1Radius() const;
	virtual float getD2Radius() const;
	virtual float getPrimaryGroupAggroDist() const;
	virtual float getPrimaryGroupAggroCoef() const;
	virtual float getSecondaryGroupAggroDist() const;
	virtual float getSecondaryGroupAggroCoef() const;
	virtual float getAggroPropagationRadius() const;
	//@}
	
	float getAggroFor(TDataSetRow const& bot);
	
	/// Used by CSpawnBotFauna for assist only
	bool hasBeenHit(uint32 ticks) const;
	
	/// Used by group fight profile only
	/// merge all the aggro value from the other aggro list into this aggro list
	void mergeAggroList(CBotAggroOwner const& otherList, float scale);
	
	void updateListAndMarkBot(std::vector<CAIEntityPhysical*>& botList, float coef);

	void setCanAggro(bool canAggro);
	
	bool getSendAggroLostToEGS() const { return _SendAggroLostToEGS; }

	/// @name Accessors
	//@{
	TBotAggroList const& getBotAggroList() const { return _BotAggroList; }
	void clearAggroList(bool sendMessageToEGS = true);
	bool isReturning() const { return _BotAggroList.empty() && _FirstHitPlace; }
	RYAI_MAP_CRUNCH::CWorldPosition getReturnPos() const { return _ReturnPos; }
	//@}
	
	/// @name Event handlers
	//@{
	virtual void aggroLost(TDataSetRow const& aggroBot) const = 0;
	virtual void aggroGain(TDataSetRow const& aggroBot) const = 0;
	//@}
	
private:
	virtual bool atPlace(CAIVector const& pos) const { if (_FirstHitPlace) return _FirstHitPlace->atPlace(pos); else return true; }
	virtual bool atPlace(CAIVectorMirror const& pos) const { if (_FirstHitPlace) return _FirstHitPlace->atPlace(pos); else return true; }
	virtual bool atPlace(CAIEntityPhysical const* entity) const { if (_FirstHitPlace) return _FirstHitPlace->atPlace(entity); else return true; }
	
private:
	TBotAggroList	_BotAggroList;
	uint32			_LastHitTime;
	bool			_AggroBlocked;
	bool			_ReturnAggroIgnored;
	bool			_DontAggro;
	bool			_SendAggroLostToEGS;
	CAITimer		_AggroBlockTimer;
	RYAI_MAP_CRUNCH::CWorldPosition		_ReturnPos;
	NLMISC::CSmartPtr<CAIPlace const>	_FirstHitPlace;
};

#endif
