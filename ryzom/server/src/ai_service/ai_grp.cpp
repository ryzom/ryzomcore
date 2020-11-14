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
#include "ai_grp.h"
#include "visual_properties_interface.h"
#include "ai_profile_npc.h"

using namespace MULTI_LINE_FORMATER;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------------
// METHODS for debugging stuff
//--------------------------------------------------------------------------

bool	GrpHistoryRecordLog	=	true;
CAIVector	lastTriedPos;

CGroup::CGroup	(CManager *owner, RYAI_MAP_CRUNCH::TAStarFlag	denyFlags, CAIAliasDescriptionNode *aliasTree) :
		CAliasChild<CManager>(owner,aliasTree),
		CPersistent<CSpawnGroup>(),
		_EscortTeamId(CTEAM::InvalidTeamId),
		_EscortRange(30),
		_AutoSpawn(true),
		_DenyFlags(denyFlags),
		_AutoDestroy(false),
		_AggroRange(0),
		_UpdateNbTicks(30)
{
	owner->getAIInstance()->addGroupInfo(this);
}

CGroup::CGroup	(CManager *owner, RYAI_MAP_CRUNCH::TAStarFlag	denyFlags, uint32 alias, std::string const& name) :
		CAliasChild<CManager>(owner, alias, name),
		CPersistent<CSpawnGroup>(),
		_EscortTeamId(CTEAM::InvalidTeamId),
		_EscortRange(30),
		_AutoSpawn(true),
		_DenyFlags(denyFlags),
		_AutoDestroy(false),
		_AggroRange(0),
		_UpdateNbTicks(30)
{
	owner->getAIInstance()->addGroupInfo(this);
}

CGroup::~CGroup	()
{
	getAIInstance()->removeGroupInfo(this, this);
	getOwner()->removeFromSpawnList	(this);
	if (isSpawned())
	{
		despawnGrp	();
	}

}

CBot* CGroup::getLeader()
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CSpawnBot* const bot = itBot->getSpawnObj();
		if (bot && bot->isAlive())
			return *itBot;
	}
	// no bots alive, no leader !
	return NULL;
}

CBot* CGroup::getSquadLeader(bool checkAliveStatus)
{
	CCont<CBot>::iterator itBot = bots().begin();
	if (itBot!=bots().end())
	{
		CSpawnBot* const bot = itBot->getSpawnObj();
		if (bot && (!checkAliveStatus || bot->isAlive()))
			return *itBot;
	}
	// first bot not alive, no squad leader !
	return NULL;
}

void	CGroup::serviceEvent	(const	CServiceEvent	&info)
{
	CCont<CBot>::iterator	it=bots().begin(), itEnd=bots().end();
	while (it!=itEnd)
	{
		it->serviceEvent	(info);
		++it;
	}

}

void	CGroup::despawnBots	()
{
	for	(CCont<CBot>::iterator	it=_Bots.begin(), itEnd=_Bots.end(); it!=itEnd;++it)
	{
		if (it->isSpawned())
			it->despawnBot();
	}

}

std::string CGroup::getIndexString() const
{
	return getOwner()->getIndexString()+NLMISC::toString(":g%u", getChildIndex());
}

std::string CGroup::getOneLineInfoString() const
{
	return std::string("Group '") + getName() + "'";
}

std::vector<std::string> CGroup::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	pushTitle(container, "CGroup");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " alias=" + getAliasString();
	container.back() += " name=" + getName();
	pushEntry(container, "fullname=" + getFullName());
	pushEntry(container, "autoSpawn=" + NLMISC::toString(_AutoSpawn));
	pushEntry(container, "aggroRange=" + NLMISC::toString(_AggroRange));
	container.back() += " updateNbTicks=" + NLMISC::toString(_UpdateNbTicks);
	if (isSpawned())
	{
		std::vector<std::string> strings = getSpawnObj()->getMultiLineInfoString();
		FOREACHC(it, std::vector<std::string>, strings)
			pushEntry(container, *it);
	}
	else
		pushEntry(container, "<not spawned>");
	pushFooter(container);
	
	return container;
}

std::string CGroup::getFullName() const
{
	return std::string(getOwner()->getFullName()+":"+getName());
}

void CGroup::lastBotDespawned()
{
	// send message
}

void CGroup::firstBotSpawned()
{
	// send message
}

//////////////////////////////////////////////////////////////////////////////
// CSpawnGroup                                                              //
//////////////////////////////////////////////////////////////////////////////

CSpawnGroup::~CSpawnGroup()
{
	FOREACH(it, CCont<CBot>,bots())
	{
		if ((*it)->isSpawned())
			(*it)->despawnBot();
	}
	
	getPersistent().despawnBots();
	
	// clear profiles
	_MovingProfile = CProfilePtr();
	_FightProfile = CProfilePtr();
	_ActivityProfile = CProfilePtr();
	_PunctualHoldActivityProfile = CProfilePtr();
	_PunctualHoldMovingProfile = CProfilePtr();
}

bool CSpawnGroup::calcCenterPos(CAIVector& grp_pos, bool allowDeadBot)
{
	if (bots().size()<=0)
		return	false;
	
	double x(0), y(0);
	uint count(0);
	
	FOREACH(it, CCont<CBot>, bots())
	{
		CSpawnBot const* const spawnBot = it->getSpawnObj();
		if (!spawnBot || (!allowDeadBot && !spawnBot->isAlive()))
			continue;
		
		x += spawnBot->pos().x().asDouble();
		y += spawnBot->pos().y().asDouble();
		++count;
	}
	if (count==0)
		return false;
	
	grp_pos.setX(x/count);
	grp_pos.setY(y/count);
	return	true;
}

void CSpawnGroup::spawnBotOfGroup()
{
	CCont<CBot>::iterator it = bots().begin();
	CCont<CBot>::iterator itEnd = bots().end();
	while (it!=itEnd)
	{
		CBot* bot = *it;
		if (!bot->isSpawned())
		{
			bool ok= bot->spawn();
// code removed by Sadge because it didn't fix the problem it was added for
//			if (ok)
//			{
//				// the spawn succeeded
//				// make sure the bot isn't in the despawn list
//				for (uint32 i= _BotsToDespawn.size(); i--;)
//				{
//					if (_BotsToDespawn[i].getBotIndex()== bot->getChildIndex())
//					{
//						nldebug("Removing bot from the despawn list because they just respawned: %s",bot->getFullName().c_str());
//						_BotsToDespawn[i]= _BotsToDespawn.back();
//						_BotsToDespawn.pop_back();
//					}
//				}
//			}
			if (!ok)
			{
				// the spawn failed
				std::string	name;
				
				if (!bot->getFullName().empty())
					name = bot->getFullName();
				else
				{
					if (!bot->getOwner()->getFullName().empty())
						name = std::string("from group ") + bot->getOwner()->getFullName();
					else
						name = std::string("Unknown");
				}
				
				if (bot->getSheet()->SheetId()==NLMISC::CSheetId::Unknown)
					nlwarning("***> Spawn failed position(%s), UNKNOWN SHEET! Bot %s ",	lastTriedPos.toString().c_str(), name.c_str());
				else
					nlwarning("***> Spawn failed position(%s), sheetId(%s) Bot %s ", lastTriedPos.toString().c_str(), bot->getSheet()->SheetId().toString().c_str(), name.c_str());
			}
		}
		++it;
	}
}

void CSpawnGroup::addBotToDespawnAndRespawnTime(CBot* bot, uint32 despawnTime, uint32 respawnTime)
{
	nlassert(bot->isSpawned());
	nlassert(bot->getOwner()==&getPersistent());
	
	uint32 const botIndex = bot->getChildIndex();
	
	FOREACH(it, std::vector<CBotToSpawn>, _BotsToDespawn)
	{
		if (it->getBotIndex()==botIndex)
		{
			*it = CBotToSpawn(botIndex, despawnTime, respawnTime);
			return;
		}
	}
	_BotsToDespawn.push_back(CBotToSpawn(botIndex, despawnTime, respawnTime));
}

void CSpawnGroup::checkDespawn()
{
	if (_BotsToDespawn.empty())
		return;
	
	//FOREACH_NOINC(it, std::vector<CBotToSpawn>, _BotsToDespawn)
	for(uint32 i = 0; i < _BotsToDespawn.size();)
	{
		CBotToSpawn& botToDespawn = _BotsToDespawn[i];
		if (botToDespawn.waitingDespawnTimeOver())
		{
			if (botToDespawn.getBotIndex()>=getPersistent().bots().size())
			{
				STOP("Array overflow in despawn code!");
			}
			else if (getPersistent().bots()[botToDespawn.getBotIndex()]==NULL)
			{
				STOP("Trying to despawn a bot who doesn't exist!!");
			}
			else
			{
				getPersistent().bots()[botToDespawn.getBotIndex()]->despawnBot();
				if	(getPersistent().isAutoSpawn())
					_BotsToRespawn.push_back(botToDespawn);
			}

			// pop this entry out of the bots to despawn vector
			// ace: we don't use iterator because when pop_back(), all iterators are invalidated
			_BotsToDespawn[i] = _BotsToDespawn.back();
			_BotsToDespawn.pop_back();
			continue;
		}
		i++;
	}

	if (_NbSpawnedBot==0 && _BotsToRespawn.size()==0)
	{
		// Warn the parent manager that this group is now dead.
		getPersistent().getOwner()->getOwner()->groupDead(&getPersistent());
		if	(getPersistent()._AutoDestroy)
			getPersistent().getOwner()->groups().removeChildByIndex(getPersistent().getChildIndex());
	}
}


void CSpawnGroup::incSpawnedBot(CBot& spawnBot)
{
#if !FINAL_VERSION
	uint32	 botIndex = spawnBot.getChildIndex();
	for	(uint32 i=(uint32)_BotsToRespawn.size(); i--; )
	{
		if (_BotsToRespawn[i].getBotIndex()==botIndex)
		{
			nldebug("Removing bot from _BotsToRespawn because they just respawned: %s",spawnBot.getFullName().c_str());
//			nlwarning("WARNING!!! Old assert \"_BotsToRespawn[i].getBotIndex()!=botIndex\" would have failed");
			_BotsToRespawn[i]=_BotsToRespawn.back();
			_BotsToRespawn.pop_back();
		}
	}
	for	(uint32 i=(uint32)_BotsToDespawn.size(); i--; )
	{
		if (_BotsToDespawn[i].getBotIndex()==botIndex)
		{
			nldebug("Removing bot from _BotsToDespawn because they just respawned: %s",spawnBot.getFullName().c_str());
//			nlwarning("WARNING!!! Old assert \"_BotsToDespawn[i].getBotIndex()!=botIndex\" would have failed");
			_BotsToDespawn[i]=_BotsToDespawn.back();
			_BotsToDespawn.pop_back();
		}
	}
#endif
	if (_NbSpawnedBot==0)
	{
		getPersistent().firstBotSpawned();
	}
	_NbSpawnedBot++;
}

void CSpawnGroup::decSpawnedBot()
{
	--_NbSpawnedBot;
	if (_NbSpawnedBot==0)
	{
		getPersistent().lastBotDespawned();
	}
}

void CSpawnGroup::checkRespawn()
{
	// respawn if there is not too much dead .. (no more than one at each tick).
	if (_BotsToRespawn.size()<=0)
		return;
	
	//FOREACH_NOINC(it, std::vector<CBotToSpawn>, _BotsToRespawn)
	for(uint32 i = 0; i < _BotsToRespawn.size();)
	{
		CBotToSpawn const& botToSpawn = _BotsToRespawn[i];
		if (botToSpawn.waitingRespawnTimeOver())
		{
			CBot* botPt = getPersistent().bots()[botToSpawn.getBotIndex()];

			CBotToSpawn const botToSpawn = _BotsToRespawn.back();

			//	remove the entry
			_BotsToRespawn[i] = _BotsToRespawn.back();
			_BotsToRespawn.pop_back();

			if (botPt->isSpawned())
			{
				nlwarning("CSpawnGroup::checkRespawn : trying to respawn a spawned bot");
			}
			if (botPt->isSpawned() || botPt->reSpawn(false))
			{
				continue;					//	directly test the same it (the next in fact).
			}
			else
			{
				_BotsToRespawn.insert(_BotsToRespawn.begin(), botToSpawn);	//	push_front so the end doesn't change.
			}
		}
		++i;
	}
	
}

CBot* CSpawnGroup::findLeader()
{
	FOREACH(itBot, CCont<CBot>, bots())
	{
		CBot* bot = *itBot;
		if (bot->isSpawned())
		{
			if (bot->getSpawnObj()->isAlive())
				return bot;
		}
	}
	return NULL;
}

std::vector<std::string> CSpawnGroup::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	pushTitle(container, "CSpawnGroup");
	pushEntry(container, "move profile:     " + _MovingProfile.getOneLineInfoString());
	pushEntry(container, "activity profile: " + _ActivityProfile.getOneLineInfoString());
	pushEntry(container, "fight profile:    " + _FightProfile.getOneLineInfoString());
	pushFooter(container);
	
	return container;
}

NLMISC::CSmartPtr<CAIPlace const> CSpawnGroup::buildFirstHitPlace(TDataSetRow const& aggroBot) const
{
	if (_ActivityProfile.getAIProfileType()==AITYPES::ACTIVITY_SQUAD)
		return static_cast<CGrpProfileSquad*>(_ActivityProfile.getAIProfile())->buildFirstHitPlace(aggroBot);
	return NULL;
}

void CSpawnGroup::addAggroFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place)
{
	CGroup& grp = getPersistent();
	FOREACH(itBot, CCont<CBot>, grp.bots())
	{
		CBot* pBot = *itBot;
		if (pBot)
		{
			CSpawnBot* spBot = pBot->getSpawnObj();
			if (spBot)
			{
				spBot->addAggroFor(bot, aggro, forceReturnAggro, place, false);
			}
		}
	}
}
void CSpawnGroup::setAggroMinimumFor(TDataSetRow const& bot, float aggro, bool forceReturnAggro, NLMISC::CSmartPtr<CAIPlace const> place)
{
	CGroup& grp = getPersistent();
	FOREACH(itBot, CCont<CBot>, grp.bots())
	{
		CBot* pBot = *itBot;
		if (pBot)
		{
			CSpawnBot* spBot = pBot->getSpawnObj();
			if (spBot)
			{
				spBot->setAggroMinimumFor(bot, aggro, forceReturnAggro, place, false);
			}
		}
	}
}

bool CSpawnGroup::haveAggro() const
{
	CGroup const& group = getPersistent();
	FOREACHC(itBot, CCont<CBot>, group.bots())
	{
		CBot const* pBot = *itBot;
		if (pBot)
		{
			CSpawnBot const* spBot = pBot->getSpawnObj();
			if (spBot && spBot->haveAggro())
				return true;
		}
	}
	return false;
}

bool CSpawnGroup::haveAggroOrReturnPlace() const
{
	CGroup const& group = getPersistent();
	FOREACHC(itBot, CCont<CBot>, group.bots())
	{
		CBot const* pBot = *itBot;
		if (pBot)
		{
			CSpawnBot const* spBot = pBot->getSpawnObj();
			if (spBot && spBot->haveAggroOrReturnPlace())
				return true;
		}
	}
	return false;
}
