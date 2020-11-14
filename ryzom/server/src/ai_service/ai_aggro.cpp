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


#include "stdpch.h"
#include "ai_aggro.h"
#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"
#include "ai_player.h"

//////////////////////////////////////////////////////////////////////////////
// CBotAggroEntry                                                           //
//////////////////////////////////////////////////////////////////////////////

CBotAggroEntry::CBotAggroEntry(TDataSetRow const& bot, float aggro, CBotAggroOwner const& owner, NLMISC::CSmartPtr<CAIPlace const> place)
: _Bot(bot)
, _Owner(owner)
, _LastHitPlace(place)
{
#ifdef NL_DEBUG
	nlassert(aggro>=0.f);
#endif
	_Aggro = aggro;
	_Owner.aggroGain(_Bot);
}

CBotAggroEntry::~CBotAggroEntry()
{
	if(_Owner.getSendAggroLostToEGS())
		_Owner.aggroLost(_Bot);
}

void CBotAggroEntry::operator =(CBotAggroEntry const& other)
{
#ifdef NL_DEBUG
	nlassert(&_Owner==&other._Owner); // same owner ?
#endif
	_Bot = other._Bot;
	_Aggro = other._Aggro;
}

void CBotAggroEntry::addAggro(float aggro, NLMISC::CSmartPtr<CAIPlace const> place)
{
	// prevent to add aggro on a bot more than one time per tick.
	_Aggro += (1.f-_Aggro)*(aggro);
	if (place!=NULL)
		_LastHitPlace = place;
}

bool CBotAggroEntry::updateTime(uint32 const& ticks) const
{
	const_cast<CBotAggroEntry*>(this)->decrementAggros(ticks);
	return _Aggro==0.f;
}

void CBotAggroEntry::setMinimum(float aggro, NLMISC::CSmartPtr<CAIPlace const> place)
{
	if (aggro>=0.f && aggro<=1.f && _Aggro<aggro)
	{
		_Aggro = aggro;
		if (place)
			_LastHitPlace = place;
	}
}

void CBotAggroEntry::setMaximum(float aggro)
{
	if (aggro>=0.f && aggro<=1.f && _Aggro>aggro)
	{
		_Aggro = aggro;
	}
}

void CBotAggroEntry::decrementAggros(uint32 ticks)
{
	static float const TOTAL_DECAY_TIME_SEC = 120.f; // temporary.
	static float const AGGRO_DEC = (1.f/(TOTAL_DECAY_TIME_SEC*10.f));
	float const decay = (float)(ticks*AGGRO_DEC);
	
	if	(_Aggro>decay)
	{
		_Aggro -= decay;
	}
	else
	{
		_Aggro = 0.f;
	}
}

//////////////////////////////////////////////////////////////////////////////
// CBotAggroOwner                                                           //
//////////////////////////////////////////////////////////////////////////////
/*
double CBotAggroOwner::defaultReturnDistCheck = 1.5;
uint32 CBotAggroOwner::defaultD1Radius = 100 * 1000; // en mm
uint32 CBotAggroOwner::defaultD2Radius = 50 * 1000; // en mm
float CBotAggroOwner::defaultPrimaryGroupAggroCoef = 0.f; // %age, entre 0 et 1
float CBotAggroOwner::defaultSecondaryGroupAggroCoef = 0.f; // %age, entre 0 et 1
uint32 CBotAggroOwner::defaultAggroPropagationRadius = 60; // en m
*/
float CBotAggroOwner::getReturnDistCheck() const
{
	return AggroReturnDistCheck;
}

float CBotAggroOwner::getD1Radius() const
{
	return AggroD1Radius;
}

float CBotAggroOwner::getD2Radius() const
{
	return AggroD2Radius;
}

float CBotAggroOwner::getPrimaryGroupAggroDist() const
{
	return AggroPrimaryGroupDist;
}

float CBotAggroOwner::getPrimaryGroupAggroCoef() const
{
	return AggroPrimaryGroupCoef;
}

float CBotAggroOwner::getSecondaryGroupAggroDist() const
{
	return AggroSecondaryGroupDist;
}

float CBotAggroOwner::getSecondaryGroupAggroCoef() const
{
	return AggroSecondaryGroupCoef;
}

float CBotAggroOwner::getAggroPropagationRadius() const
{
	return AggroPropagationRadius;
}


CBotAggroOwner::CBotAggroOwner()
: _LastHitTime(CTimeInterface::gameCycle())
, _AggroBlocked(false)
, _ReturnAggroIgnored(false)
, _DontAggro(false)
, _SendAggroLostToEGS(true)
, _ReturnPos()
, _FirstHitPlace(NULL)
//, _State(NoAggro)
{
}

CBotAggroOwner::~CBotAggroOwner()
{
#ifdef NL_DEBUG
	nlassert(_BotAggroList.size()==0);	//	have to clear this vector before call destructor (coz of call back).
#endif
}

void CBotAggroOwner::mergeAggroList(CBotAggroOwner const& otherList, float scale)
{
	H_AUTO(AGGRO_mergeAggroList);
	NLMISC::clamp(scale, 0.0f, 1.0f);
	
	FOREACHC(it, TBotAggroList, otherList._BotAggroList)
	{
		CBotAggroEntry* bae = it->second;
		setAggroMinimumFor(bae->getBot(), bae->finalAggro()*scale, true, bae->getLastHitPlace());
	}
}


void CBotAggroOwner::updateListAndMarkBot(std::vector<CAIEntityPhysical*>& botList, float coef)
{
	H_AUTO(AGGRO_ULAMB);
	std::list<TDataSetRow> botsToRemove;
	{
		H_AUTO(AGGRO_ULAMB_buildList);
		FOREACH(it, TBotAggroList, _BotAggroList)
		{
			if (!isAggroValid(it->first))
			{
				botsToRemove.push_back(it->first);
				continue;
			}
			CAIEntityPhysical* const entity = CAIS::instance().getEntityPhysical(it->second->getBot());
			nlassert(entity); // if this entry is no more valid, then isAggroValid is bugged
			if (entity->_ChooseLastTime!=CTimeInterface::gameCycle())
			{
				entity->_ChooseLastTime = CTimeInterface::gameCycle();
				entity->_AggroScore = it->second->finalAggro()*coef;
				botList.push_back(entity);
			}
			else
			{
				entity->_AggroScore += it->second->finalAggro()*coef;
			}
		}
	}
	{
		H_AUTO(AGGRO_ULAMB_removeInvalids);
		FOREACH(it, std::list<TDataSetRow>, botsToRemove)
		{
			_BotAggroList.erase(*it);
		}
	}
}

void CBotAggroOwner::blockAggro(sint32 blockTime)
{
	if (!_AggroBlocked || _AggroBlockTimer.timeRemaining()<blockTime)
		_AggroBlockTimer.set(blockTime);
	_AggroBlocked = true;
}

void CBotAggroOwner::ignoreReturnAggro(bool ignored)
{
	_ReturnAggroIgnored = ignored;
}

void CBotAggroOwner::setCanAggro(bool canAggro)
{
	_DontAggro = !canAggro;
}

void CBotAggroOwner::update(uint32 ticks)
{
	H_AUTO(AGGRO_update);
	if (_AggroBlocked)
	{
		if (!_AggroBlockTimer.test())
			return;
		_AggroBlocked = false;
	}
	std::deque<TDataSetRow> botsToRemove;
	FOREACH(it, TBotAggroList, _BotAggroList)
	{
		// :TODO: Check aggro validity (target position), but it's too expensive I think
		if (it->second->updateTime(ticks))
		{
			botsToRemove.push_back(it->first);
		}
		else
		{
			CAIEntityPhysical* const entity = CAIS::instance().getEntityPhysical(it->second->getBot());
			if (!entity)
				botsToRemove.push_back(it->first);
		}
	}
	FOREACH(it, std::deque<TDataSetRow>, botsToRemove)
	{
		_BotAggroList.erase(*it);
	}
	if (_BotAggroList.empty() && _FirstHitPlace)
	{
		if (_ReturnPos.toAIVector().quickDistTo(getAggroPos().toAIVector()) < getReturnDistCheck())
			_FirstHitPlace = NULL;
	}
}

// :NOTE: If you modify this method modify isNewAggroValid accordingly.
bool CBotAggroOwner::isAggroValid(TDataSetRow const& bot)
{
	H_AUTO(AGGRO_isAggroValid);
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	// Bot has no aggro
	if (it==_BotAggroList.end())
		return false;
	// Other bot don't exist
	CAIEntityPhysical const* entity = CAIS::instance().getEntityPhysical(bot);
	if (!entity)
		return false;
	// Bot fled too far from last time he hit
	if (!it->second->atPlace(entity))
		return false;
	// He's outside of first hit place
	if (!this->atPlace(entity))
		return false;
	// He seems OK, let's aggro him
	return true;
}

// :NOTE: If you modify this method modify isAggroValid accordingly.
bool CBotAggroOwner::isNewAggroValid(TDataSetRow const& bot)
{
	H_AUTO(AGGRO_isNewAggroValid);
	// Other bot don't exist
	CAIEntityPhysical const* entity = CAIS::instance().getEntityPhysical(bot);
	if (!entity)
		return false;
	// He's outside of first hit place
	if (!this->atPlace(entity))
		return false;
	// He seems OK, let's aggro him
	return true;
}

bool CBotAggroOwner::haveAggroWithEntity(const TDataSetRow& rowId) const
{
	H_AUTO(AGGRO_haveAggroWithEntity);
	return _BotAggroList.find(rowId) != _BotAggroList.end();
}

void CBotAggroOwner::addAggroFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place, bool transferAggro)
{
	H_AUTO(AGGRO_AA_Total);
	if (!isAggroable(bot) || (_ReturnAggroIgnored && !forceReturnAggro))
		return;
	
	if (_DontAggro)
		return;
	
	// If group aggro build a bot list who aggro (self included), and return
	if (transferAggro && getPrimaryGroupAggroCoef()>0.f)
	{
		std::set<CBotAggroOwner*> primaryGroup, secondaryGroup;
		{
			H_AUTO(AGGRO_AA_BuildAggroTransferList);
			primaryGroup = getAggroGroup(true);
			primaryGroup.erase(this);
			FOREACH(it, std::set<CBotAggroOwner*>, primaryGroup)
			{
				if (*it)
				{
					std::set<CBotAggroOwner*> aggroGroup = (*it)->getAggroGroup(false);
					std::set_difference(aggroGroup.begin(), aggroGroup.end(), primaryGroup.begin(), primaryGroup.end(), std::inserter(secondaryGroup, secondaryGroup.end()));
				}
			}
			secondaryGroup.erase(this);
		}
		this->addAggroFor(bot, aggro, forceReturnAggro, place, false);
		FOREACH(it, std::set<CBotAggroOwner*>, primaryGroup)
			(*it)->addAggroFor(bot, aggro*getPrimaryGroupAggroCoef(), forceReturnAggro, place, false);
		FOREACH(it, std::set<CBotAggroOwner*>, secondaryGroup)
			(*it)->addAggroFor(bot, aggro*getSecondaryGroupAggroCoef(), forceReturnAggro, place, false);
		return;
	}
	
#ifdef NL_DEBUG
	nlassert(aggro<=0);
#endif
	
	// If we have no firstHitPlace start fight
	if (!_FirstHitPlace)
	{
		H_AUTO(AGGRO_AA_BuildFirstHitPlace);
		_ReturnPos = getAggroPos();
		_FirstHitPlace = buildFirstHitPlace(bot);
	}
	// Else verify this bot is still in range
	else
	{
		H_AUTO(AGGRO_AA_CheckFirstHitPlace);
		// If aggro is not valid for that bot (he's outside of range)
		if (!isNewAggroValid(bot))
		{
			// Forget him
			forgetAggroFor(bot, true);
			// And ignore him
			return;
		}
	}
	if (!place)
	{
		H_AUTO(AGGRO_AA_BuildLastHitPlace);
		CAIEntityPhysical const* entity = NULL;
		H_TIME(AGGRO_AA_BLHP_GetEntity, entity = CAIS::instance().getEntityPhysical(bot););
		if (entity)
		{
			NLMISC::CSmartPtr<CAIPlaceFastXYR> newPlace(NULL);
			H_TIME(AGGRO_AA_BLHP_AllocPlace, newPlace = NLMISC::CSmartPtr<CAIPlaceFastXYR>(new CAIPlaceFastXYR(NULL)););
			H_TIME(AGGRO_AA_BLHP_ParamPlace, newPlace->setPosAndRadius(AITYPES::vp_auto, entity->pos(), (uint32)(getD2Radius()*1000.f)););
			place = newPlace;
		}
	}
	
	_LastHitTime = CTimeInterface::gameCycle();
	
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	if (it!=_BotAggroList.end())
	{
		H_AUTO(AGGRO_AA_AddAggro);
		it->second->addAggro(-aggro, place);
	}
	else
	{
		H_AUTO(AGGRO_AA_CreateAggro);
		//	not found, so add it.
		//	as its the first time, majorate aggro (square its effect)..
		float firstAggro = 1+aggro;
		firstAggro = 1-firstAggro*firstAggro;
		
		_BotAggroList.insert(std::make_pair(bot, TAggroEntryPtr(new CBotAggroEntry(bot, firstAggro, *this, place))));
	}
}

void CBotAggroOwner::setAggroMinimumFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place, bool transferAggro)
{
	H_AUTO(AGGRO_SAM_Total);
	if (!isAggroable(bot) || (_ReturnAggroIgnored && !forceReturnAggro))
		return;

	if (_DontAggro)
		return;
	
	// If group aggro build a bot list who aggro (self included), and return
	if (transferAggro && getPrimaryGroupAggroCoef()>0.f)
	{
		std::set<CBotAggroOwner*> primaryGroup, secondaryGroup;
		{
			H_AUTO(AGGRO_SAM_BuildAggroTransferList);
			primaryGroup = getAggroGroup(true);
			primaryGroup.erase(this);
			FOREACH(it, std::set<CBotAggroOwner*>, primaryGroup)
			{
				if (*it)
				{
					std::set<CBotAggroOwner*> aggroGroup = (*it)->getAggroGroup(false);
					std::set_difference(aggroGroup.begin(), aggroGroup.end(), primaryGroup.begin(), primaryGroup.end(), std::inserter(secondaryGroup, secondaryGroup.end()));
				}
			}
			secondaryGroup.erase(this);
		}
		this->setAggroMinimumFor(bot, aggro, forceReturnAggro, place, false);
		FOREACH(it, std::set<CBotAggroOwner*>, primaryGroup)
			(*it)->setAggroMinimumFor(bot, aggro*getPrimaryGroupAggroCoef(), forceReturnAggro, place, false);
		FOREACH(it, std::set<CBotAggroOwner*>, secondaryGroup)
			(*it)->setAggroMinimumFor(bot, aggro*getSecondaryGroupAggroCoef(), forceReturnAggro, place, false);
		return;
	}
	
#ifdef NL_DEBUG
	nlassert(aggro>=0);
#endif
	if (!_FirstHitPlace)
	{
		H_AUTO(AGGRO_SAM_BuildFirstHitPlace);
		_ReturnPos = getAggroPos();
		_FirstHitPlace = buildFirstHitPlace(bot);
	}
	// Else verify this bot is still in range
	else
	{
		H_AUTO(AGGRO_SAM_CheckFirstHitPlace);
		// If aggro is not valid for that bot (he's outside of range)
		if (!isNewAggroValid(bot))
		{
			// Forget him
			forgetAggroFor(bot, true);
			// And ignore him
			return;
		}
	}
	if (!place)
	{
		H_AUTO(AGGRO_SAM_BuildLastHitPlace);
		CAIEntityPhysical const* entity = NULL;
		H_TIME(AGGRO_SAM_BLHP_GetEntity, entity = CAIS::instance().getEntityPhysical(bot););
		if (entity)
		{
			NLMISC::CSmartPtr<CAIPlaceFastXYR> newPlace(NULL);
			H_TIME(AGGRO_SAM_BLHP_AllocPlace, newPlace = NLMISC::CSmartPtr<CAIPlaceFastXYR>(new CAIPlaceFastXYR(NULL)););
			H_TIME(AGGRO_SAM_BLHP_ParamPlace, newPlace->setPosAndRadius(AITYPES::vp_auto, entity->pos(), (uint32)(getD2Radius()*1000.f)););
			place = newPlace;
		}
	}
	
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	if (it!=_BotAggroList.end())
	{
		H_AUTO(AGGRO_SAM_SetMinimum);
		it->second->setMinimum(aggro, place);
	}
	else
	{
		H_AUTO(AGGRO_SAM_CreateAggro);
		// not found, so add it.
		_BotAggroList.insert(std::make_pair(bot, TAggroEntryPtr(new CBotAggroEntry(bot, aggro, *this, place))));
	}
}

void CBotAggroOwner::maximizeAggroFor(TDataSetRow const& bot)
{
	H_AUTO(AGGRO_maximizeAggroFor);
	if (!isAggroable(bot))
		return;
	
	if (_DontAggro)
		return;
	
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	if (it!=_BotAggroList.end())
	{
		it->second->setMinimum(0.999f);
		FOREACH(it2, TBotAggroList, _BotAggroList)
		{
			if (it2!=it)
				it2->second->scaleBy(0.15f);
		}
	}
	else
	{
		_BotAggroList.insert(std::make_pair(bot, TAggroEntryPtr(new CBotAggroEntry(bot, 0.999f, *this))));
	}
}

void CBotAggroOwner::minimizeAggroFor(TDataSetRow const& bot)
{
	H_AUTO(AGGRO_minimizeAggroFor);
	if (!isAggroable(bot))
		return;
	
	if (_DontAggro)
		return;
	
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	if (it!=_BotAggroList.end())
	{
		it->second->setMaximum(0.1f);
		FOREACH(it2, TBotAggroList, _BotAggroList)
		{
			if (it2!=it)
				it2->second->setMinimum(0.15f);
		}
	}
	else
	{
		_BotAggroList.insert(std::make_pair(bot, TAggroEntryPtr(new CBotAggroEntry(bot, 0.1f, *this))));
	}
}

void CBotAggroOwner::forgetAggroFor(TDataSetRow const& bot, bool forgetDamages)
{
	H_AUTO(AGGRO_forgetAggroFor);
	_BotAggroList.erase(bot);
	if (forgetDamages && getAggroOwnerEid()!=NLMISC::CEntityId::Unknown)
	{
		CAIEntityPhysical* const entity = CAIS::instance().getEntityPhysical(bot);
		if (entity)
		{
			// Tell EGS that player is a cheater
			if (entity->getRyzomType()==RYZOMID::player)
			{
				NLMISC::CEntityId targetId = entity->getEntityId();
				NLMISC::CEntityId botId = getAggroOwnerEid();
				
				NLNET::CMessage	msgout("PLAYER_UNREACHABLE");
				msgout.serial(botId);
				msgout.serial(targetId);
				sendMessageViaMirror("EGS", msgout);
			}
		}
	}
}

bool CBotAggroOwner::hasBeenHit(uint32 ticks) const
{
	return (CTimeInterface::gameCycle()-_LastHitTime)<=ticks;
}

bool CBotAggroOwner::isAggroable(TDataSetRow const& dataSetRow)
{
	H_AUTO(AGGRO_isAggroable);
	if (CMirrors::getEntityId(dataSetRow).getType()!=RYZOMID::player)
		return true;
	
	CAIEntityPhysical* ep = CAIS::instance().getEntityPhysical(dataSetRow);
	if (!ep)
		return false;
	
	CBotPlayer const* const player = NLMISC::safe_cast<CBotPlayer const*>(ep);
	if (!player)
		return true;
	return player->isAggroable();
}

RYAI_MAP_CRUNCH::CWorldPosition CBotAggroOwner::getAggroPos() const
{
	return RYAI_MAP_CRUNCH::CWorldPosition();
}

NLMISC::CEntityId CBotAggroOwner::getAggroOwnerEid() const
{
	return NLMISC::CEntityId::Unknown;
}

NLMISC::CSmartPtr<CAIPlace const> CBotAggroOwner::buildFirstHitPlace(TDataSetRow const& aggroBot) const
{
	return NLMISC::CSmartPtr<CAIPlace const>(NULL);
}

void CBotAggroOwner::clearAggroList(bool sendMessageToEGS)
{
	_SendAggroLostToEGS = sendMessageToEGS;
	_BotAggroList.clear();
	_SendAggroLostToEGS = true;
}

float CBotAggroOwner::getAggroFor(TDataSetRow const& bot)
{
	TBotAggroList::iterator it = _BotAggroList.find(bot);
	if (it!=_BotAggroList.end())
		return it->second->finalAggro();
	else
		return 0.f;
}
