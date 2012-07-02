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



#ifndef AI_PLAYER_H
#define AI_PLAYER_H

#include "ai_share/ai_coord.h"
#include "nel/misc/sheet_id.h"
#include "ai.h"
#include "ai_bot.h"
#include "owners.h"

class CFauna;



#ifdef NL_OS_WINDOWS
#pragma warning (disable : 4355)
#endif // NL_OS_WINDOWS

//////////////////////////////////////////////////////////////////////////////
// CBotPlayer                                                               //
//////////////////////////////////////////////////////////////////////////////

//	Player is considered as both a persistent and a spawned entity (philosophycally, the spawning process is part of the client).
class CBotPlayer
: public NLMISC::CDbgRefCount<CBotPlayer>
, public CChild<CManagerPlayer>
, public CPetOwner
, public CAIEntityPhysical
, public CPersistentOfPhysical
{
public:	
	CBotPlayer(CManagerPlayer* owner, TDataSetRow const& dataSetRow, NLMISC::CEntityId const& id, uint32 level);
	virtual ~CBotPlayer();
	
	/// @name CChild implementation
	//@{
	virtual std::string	getIndexString() const;
	virtual std::string getEntityIdString() const;
	virtual std::string	getOneLineInfoString() const;
	std::vector<std::string> getMultiLineInfoString() const;
	//@}
	
	CAIInstance* getAIInstance() const;
	
	void setAggroable(bool aggroable = true) { _Aggroable = aggroable; }
	bool isAggroable() const { return _Aggroable; }
	
	// player are always attackable (this is a IA point of view, it mean that IA can attack player)
	virtual bool isBotAttackable() const { return true; }
	
	virtual bool spawn();
	void despawnBot();
	
	void update();
	
	//	update the pos and link of player (if the position is valid, otherwise, no move are done)
	//	perhaps should we invalidate the player worldPosition. (!?).
	void updatePos();
	virtual CAIPos aipos() const;
	
	void setTarget(CAIEntityPhysical* target) { CTargetable<CAIEntityPhysical>::setTarget(target); }
	void setVisualTarget(CAIEntityPhysical* target) { CTargetable<CAIEntityPhysical>::setVisualTarget(target); }
	
	bool isUnReachable() const;
	
	bool setPos(CAIPos const& pos);
	
	float walkSpeed() const;
	float runSpeed() const;
	
	CAIEntityPhysical& getPhysical() { return *this; }
	
	virtual RYZOMID::TTypeId getRyzomType() const { return RYZOMID::player; }
	
	bool isAggressive() const;
	
	void processEvent(const	CCombatInterface::CEvent &);

	uint16	getCurrentTeamId()	const			{ return _CurrentTeamId;}
	void	setCurrentTeamId(uint16 teamId)		{ _CurrentTeamId = teamId;}
	
	bool	getFollowMode()	const				{ return _FollowMode; }
	void	setFollowMode(bool followMode)		{ _FollowMode = followMode; }
	
	void addAggroer(TDataSetRow const& row);
	
	void removeAggroer(TDataSetRow const& row);
	
	void forgotAggroForAggroer();
	/// Updates the reference to zone in which the player is that can trigger event (on enter, on leave)	
	void updateInsideTriggerZones(const std::set<uint32>& newInsideTriggerZone, std::vector<uint32>& onEnterZone, std::vector<uint32>& onLeaveZone);
	
	virtual sint32 getFame(std::string const& faction, bool modulated = false, bool returnUnknowValue = false) const;
	virtual sint32 getFameIndexed(uint32 factionIndex, bool modulated = false, bool returnUnknowValue = false) const;
	
public:
	static bool useOldUnreachable;

private:	
	uint16	_CurrentTeamId;
	bool	_FollowMode;
	bool	_PlayerPosIsInvalid;
	bool	_Aggroable;
	std::vector<TDataSetRow>	_AggroerList;
	std::set<uint32>	_InsideTriggerZones;
	AISHEETS::IRaceStatsCPtr	_Sheet;
};

//////////////////////////////////////////////////////////////////////////////
// CManagerPlayer                                                           //
//////////////////////////////////////////////////////////////////////////////

class CManagerPlayer
: public CCont<CBotPlayer>
, public CChild<CAIInstance>
{
public:
	typedef	CHashMap<TDataSetRow, NLMISC::CDbgPtr<CBotPlayer>, TDataSetRow::CHashCode>	TPlayerMap;
	
public:
	CManagerPlayer(CAIInstance* owner)
	: CChild<CAIInstance>(owner)
	{
	}
	
	virtual ~CManagerPlayer();
	
	CAIInstance* getAIInstance() const
	{
		return getOwner();
	}
	
	void addSpawnedPlayer(TDataSetRow const& dataSetRow, NLMISC::CEntityId const& id);
	// Strict mean that the player MUST be in this manager (otherwise, the function log a warning)
	void removeDespawnedPlayer(TDataSetRow const& dataSetRow);
	/// Called when the team id value from mirror change.
	void updatePlayerTeam(TDataSetRow const& dataSetRow);
	
	//	update the manager.
	void update();
	
	/// Return a set of player in the same team of the indicated player
	std::set<TDataSetRow> const& getPlayerTeam(TDataSetRow const& playerRow);
	/// Return a set of player in the specified team
	std::set<TDataSetRow> const& getPlayerTeam(uint16 teamId);
	
	/// Debug feature, build a set of currently active team.
	void getTeamIds(std::vector<uint16>& teamIds);
	
	std::string	getIndexString() const
	{
		return getOwner()->getIndexString()+NLMISC::toString(":players");
	}
	
	TPlayerMap& playerList()
	{
		return _spawnedPlayers;
	}
	
private:
	TPlayerMap _spawnedPlayers; // hum .. still useful ?
	/// Team composition.
	typedef CHashMap<int, std::set<TDataSetRow> > TTeamMap;
	TTeamMap _teams;
	
private:
	static std::set<TDataSetRow> emptySet;
};

#endif
