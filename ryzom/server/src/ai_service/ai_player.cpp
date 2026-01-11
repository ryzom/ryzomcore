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
#include "ai_player.h"
#include "ai_bot_fauna.h"
#include "ai_bot_npc.h"

using namespace MULTI_LINE_FORMATER;


//////////////////////////////////////////////////////////////////////////////
// CBotPlayer                                                               //
//////////////////////////////////////////////////////////////////////////////

CBotPlayer::CBotPlayer(CManagerPlayer* owner, TDataSetRow const& DataSetRow, NLMISC::CEntityId const& id, uint32 level)
: CChild<CManagerPlayer>(owner)
, CAIEntityPhysical(static_cast<CPersistentOfPhysical&>(*this), DataSetRow, id, 0.5f, level, RYAI_MAP_CRUNCH::Nothing)
, _CurrentTeamId(CTEAM::InvalidTeamId)
, _FollowMode(false)
, _Aggroable(true)
{
#ifdef NL_DEBUG
	nlassert(owner->playerList().find(dataSetRow())==owner->playerList().end());
#endif
	owner->playerList().insert(std::make_pair(dataSetRow(), this));
	NLMISC::CSheetId sheetId = CMirrors::sheet(DataSetRow);
	_Sheet = AISHEETS::CSheets::getInstance()->lookupRaceStats(sheetId);
}

CBotPlayer::~CBotPlayer()
{
	getOwner()->playerList().erase(dataSetRow());
	if (isSpawned())
	{
		despawnBot();
	}
}

std::string	CBotPlayer::getIndexString()	const
{
	return	getOwner()->getIndexString()+NLMISC::toString(":p%u", getChildIndex());
}

std::string CBotPlayer::getEntityIdString() const
{
	return getEntityId().toString() ;
}

std::string	CBotPlayer::getOneLineInfoString() const
{
	return std::string("Player '") + getEntityId().toString() + "'";
}

std::vector<std::string> CBotPlayer::getMultiLineInfoString() const
{
	std::vector<std::string> container;
	
	
	pushTitle(container, "CBotPlayer");
	pushEntry(container, "id=" + getIndexString());
	container.back() += " eid=" + getEntityIdString();
	container.back() += " teamid=" + NLMISC::toString("%u", _CurrentTeamId);
	pushFooter(container);
	
	
	return container;
}

void CBotPlayer::processEvent(CCombatInterface::CEvent const& event)
{
	//	if heal happends, dispatch aggro on targetters.	
	if (event._nature==ACTNATURE::CURATIVE_MAGIC && event._weight>=0)
	{
		float aggro = -event._weight;
		if (aggro>-0.5f)
			aggro = -0.5f;
		else if (aggro<-1.f)
			aggro = -1.f;
		
		CAIEntityPhysical const* const targetBot = CAIS::instance().getEntityPhysical(event._targetRow);
		
		if (targetBot)
		{
			CAIEntityPhysical* targeter = targetBot->firstTargeter();
			while (targeter)
			{
				if (targeter->dataSetRow()!=event._originatorRow)
				{
					switch (targeter->getRyzomType())
					{
					case RYZOMID::creature:
						{
							CSpawnBotFauna* const fauna = NLMISC::safe_cast<CSpawnBotFauna*>(targeter);
							fauna->addAggroFor(event._originatorRow, aggro, true);
						}
						break;
					case RYZOMID::npc:
						{
							CSpawnBotNpc* const npc = NLMISC::safe_cast<CSpawnBotNpc*>(targeter);
							npc->addAggroFor(event._originatorRow, aggro, true);
						}
						break;
					default:
						break;
					}
				}
				targeter = targeter->nextTargeter();
			}
		}
	}
}

void CBotPlayer::updatePos()
{
	RYAI_MAP_CRUNCH::CWorldPosition	wpos;
	if (!CWorldContainer::getWorldMap().setWorldPosition(pos().h(), wpos,CAIVector(pos())))
	{
		_PlayerPosIsInvalid = true;
		return;
	}
	_PlayerPosIsInvalid = false;
	setWPos(wpos);
	linkEntityToMatrix(this->pos(),getOwner()->getOwner()->playerMatrix());
	
	if (wpos.getFlags()&RYAI_MAP_CRUNCH::Water)
		setActionFlags(RYZOMACTIONFLAGS::InWater);
	else
		removeActionFlags(RYZOMACTIONFLAGS::InWater);
}

CAIPos CBotPlayer::aipos() const
{
	if (_PlayerPosIsInvalid)
		return CAIPos(wpos().toAIVector(), wpos().h(), 0); // This is last valid position on AI collision map
	else
		return CAIPos(pos());
}

void CBotPlayer::update()
{
	updatePos();
}

CAIInstance* CBotPlayer::getAIInstance() const
{
	return getOwner()->getAIInstance();
}

bool CBotPlayer::spawn()
{
	setSpawn(this);
	return true;
}

void CBotPlayer::despawnBot()
{
	setSpawn(NULL);
}

bool CBotPlayer::isUnReachable() const
{
	// _PlayerPosIsInvalid does not reflect the fact that the player is unreachable
	// _PlayerPosIsInvalid is true when player is in delta between PACS and WorldMap
	// collisions, and is reachable in those cases if the bot is near
	if (useOldUnreachable)
	{
		return	_PlayerPosIsInvalid;
	}
	else
	{
		return false;
	}
}

bool CBotPlayer::setPos(CAIPos const& pos)
{
#ifdef NL_DEBUG
	nlassert(1==0);
#endif
	return	true;
}

float CBotPlayer::walkSpeed() const
{
	nlerror("Non-virtual overriden function walkSpeed in CBotPlayer");
	return 3.f/10.f;
}

float CBotPlayer::runSpeed() const
{
	nlerror("Non-virtual overriden function runSpeed in CBotPlayer");
	return 6.f/10.f;
}

bool CBotPlayer::isAggressive() const
{
	MBEHAV::TMode const mode = getMode();
	return mode==MBEHAV::COMBAT_FLOAT || mode==MBEHAV::COMBAT;
}

void CBotPlayer::addAggroer(TDataSetRow const& row)
{
#if !FINAL_VERSION
	for	(sint32 i=(sint32)_AggroerList.size()-1;i>=0;i--)
		nlassert(_AggroerList[i]!=row);
#endif
	_AggroerList.push_back(row);
}

void CBotPlayer::removeAggroer(TDataSetRow const& row)
{
	for	(sint32 i=(sint32)_AggroerList.size()-1;i>=0;i--)
	{
		if (_AggroerList[i]==row)
		{
			_AggroerList.at(i)=_AggroerList.back();
			_AggroerList.pop_back();
			break;
		}
	}
}

void CBotPlayer::updateInsideTriggerZones(const std::set<uint32>& newInsideTriggerZone, std::vector<uint32>& onEnterZone, std::vector<uint32>& onLeaveZone)
{
	std::set<uint32>::const_iterator firstInside(_InsideTriggerZones.begin()), lastInside( _InsideTriggerZones.end());
	std::set<uint32>::const_iterator firstNewInside(newInsideTriggerZone.begin()), lastNewInside( newInsideTriggerZone.end());
	
	std::set_difference(firstInside, lastInside, firstNewInside, lastNewInside, std::back_inserter(onLeaveZone));
	std::set_difference(firstNewInside, lastNewInside, firstInside, lastInside, std::back_inserter(onEnterZone));
	
	_InsideTriggerZones = newInsideTriggerZone;
}

//////////////////////////////////////////////////////////////////////////////
// CManagerPlayer                                                           //
//////////////////////////////////////////////////////////////////////////////

CManagerPlayer::~CManagerPlayer()
{
	TPlayerMap::iterator it = _spawnedPlayers.begin();
	while (it != _spawnedPlayers.end())
	{
		CBotPlayer* player = (*it).second;

		// a CBotPlayer object removes itself from _spawnedPlayers at destruction
		// increment the iterator before it becomes invalid
		++it;
		player->despawnBot();
		removeChildByIndex(player->getChildIndex());
		// now the player object is destroyed
	}
}

void CManagerPlayer::update()
{
	FOREACH(it, TPlayerMap, _spawnedPlayers)
	{
		it->second->CBotPlayer::update();
	}
}

void CManagerPlayer::addSpawnedPlayer(TDataSetRow const& dataSetRow, NLMISC::CEntityId const& id)
{
	CBotPlayer*	player = new CBotPlayer(this,dataSetRow,id,1); // :TODO: default player level calculation (skill & hp ?).
	addChild(player);
	player->spawn();
	
	player->linkToWorldMap(player,	player->pos(), getOwner()->playerMatrix());
	player->updatePos();
	
	// update team id and composition
	CMirrorPropValueRO<TYPE_TEAM_ID> value( *CMirrors::DataSet, dataSetRow, DSPropertyTEAM_ID );
	player->setCurrentTeamId(value());
	if (value() != CTEAM::InvalidTeamId)
	{
		_teams[value()].insert(dataSetRow);
	}
}

void CManagerPlayer::removeDespawnedPlayer(TDataSetRow const& dataSetRow)
{
	//	Remove player from Manager.
	TPlayerMap::iterator it = _spawnedPlayers.find(dataSetRow);
	if (it==_spawnedPlayers.end())
	{
		// need to log some warning
		nlwarning("Player Despawn Error");
	#ifdef NL_DEBUG
		nlerror("Player Despawn Error");
	#endif
		return;
	}
	else
	{
		CBotPlayer* const player = (*it).second;
		
		// update team composition
		if (player->getCurrentTeamId() != CTEAM::InvalidTeamId)
		{
			CHashMap<int, std::set<TDataSetRow> >::iterator it(_teams.find(player->getCurrentTeamId()));
			if (it != _teams.end())
			{
				it->second.erase(dataSetRow);
				if (it->second.empty())
					_teams.erase(it);
			}
		}
		player->despawnBot();
		removeChildByIndex(player->getChildIndex());
	}
}

void CManagerPlayer::updatePlayerTeam(TDataSetRow const& dataSetRow)
{
	TPlayerMap::iterator it(_spawnedPlayers.find(dataSetRow));
	if (it!=_spawnedPlayers.end())
	{
		uint16 const oldTeam = it->second->getCurrentTeamId();
		if (oldTeam!=CTEAM::InvalidTeamId)
		{
			CHashMap<int, std::set<TDataSetRow> >::iterator it(_teams.find(oldTeam));
			if (it != _teams.end())
			{
				it->second.erase(dataSetRow);
				if (it->second.empty())
					_teams.erase(it);
			}
		}
		// update team id and composition
		CMirrorPropValueRO<TYPE_TEAM_ID> value(*CMirrors::DataSet, dataSetRow, DSPropertyTEAM_ID);
		it->second->setCurrentTeamId(value());
		if (value() != CTEAM::InvalidTeamId)
		{
			_teams[value()].insert(dataSetRow);
		}
	}
	else
	{
		nlwarning("CManagerPlayer::updatePlayerTeam : dataSetRow %u, can't find spawned player !", dataSetRow.getIndex());
	}
}

// This static data is just to have a ref return type anytime, bad habit.
std::set<TDataSetRow> CManagerPlayer::emptySet;

std::set<TDataSetRow> const& CManagerPlayer::getPlayerTeam(TDataSetRow const& playerRow)
{
	TPlayerMap::iterator it(_spawnedPlayers.find(playerRow));
	
	if (it != _spawnedPlayers.end())
	{
		uint16 const teamId = it->second->getCurrentTeamId();
		return getPlayerTeam(teamId);
	}
	else
	{
		nlwarning("CManagerPlayer::getPlayerTeam can't find player from dataset %u", playerRow.getIndex());
		return emptySet;
	}
}

std::set<TDataSetRow> const& CManagerPlayer::getPlayerTeam(uint16 teamId)
{
	if (teamId == CTEAM::InvalidTeamId)
	{
		return emptySet;
	}
	else
	{
		TTeamMap::iterator itTeam = _teams.find(teamId);
		if (itTeam != _teams.end())
		{
			return itTeam->second;
		}
		else
		{
			nlwarning("CManagerPlayer::getPlayerTeam : no player in team %u", teamId);
			return emptySet;
		}
	}
}

void CManagerPlayer::getTeamIds(std::vector<uint16>& teamIds)
{
	FOREACH(itTeam, TTeamMap, _teams)
	{
		teamIds.push_back(itTeam->first);
	}
}

void CBotPlayer::forgotAggroForAggroer()
{

	for	(sint32 i=(sint32)_AggroerList.size()-1; i>=0; --i)
	{
		CAIEntityPhysical* const phys = CAIS::instance().getEntityPhysical(_AggroerList[i]);
		if (!phys)
			continue;
		CBotAggroOwner* aggroOwner = NULL;
		
		switch(phys->getRyzomType())
		{
		case RYZOMID::creature:
			aggroOwner = NLMISC::safe_cast<CBotAggroOwner*>(NLMISC::safe_cast<CSpawnBotFauna*>(phys));
			break;
		case RYZOMID::npc:
			aggroOwner = NLMISC::safe_cast<CBotAggroOwner*>(NLMISC::safe_cast<CSpawnBotNpc*>(phys));
			break;
		}
		if (!aggroOwner)
			continue;
		aggroOwner->forgetAggroFor(dataSetRow());
	}
}

bool CBotPlayer::useOldUnreachable = false;

NLMISC_COMMAND(playerUseOldUnreachable, "Old unreachable state computing is used","")
{
	if(args.size()>1) return false;
	if(args.size()==1) StrToBool(CBotPlayer::useOldUnreachable, args[0]);
	log.displayNL("playerUseOldUnreachable is %s", CBotPlayer::useOldUnreachable?"true":"false");
	return true;
}

sint32 CBotPlayer::getFame(std::string const& faction, bool modulated, bool returnUnknownValue) const
{
	sint32 fame = CAIEntityPhysical::getFame(faction, modulated, true);
	if (fame==NO_FAME)
	{
		fame = CStaticFames::getInstance().getStaticFame(_Sheet->Race(), faction);
	}
	if (!returnUnknownValue && fame==NO_FAME)
		fame = 0;
	return fame;
}

sint32 CBotPlayer::getFameIndexed(uint32 factionIndex, bool modulated, bool returnUnknownValue) const
{
	sint32 fame = CAIEntityPhysical::getFameIndexed(factionIndex, modulated, true);
	if (fame==NO_FAME)
	{
		uint32 playerFaction = CStaticFames::getInstance().getFactionIndex(_Sheet->Race());
		fame = CStaticFames::getInstance().getStaticFameIndexed(playerFaction, factionIndex);
	}
	if (!returnUnknownValue && fame==NO_FAME)
		fame = 0;
	return fame;
}
