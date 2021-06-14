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

#ifndef RY_OUTPOST_H
#define RY_OUTPOST_H

#include "nel/misc/types_nl.h"
#include "mission_manager/ai_alias_translator.h"
#include "pvp_manager/pvp_zone.h"
#include "game_share/continent.h"
#include "game_share/outpost.h"
#include "outpost_squad.h"
#include "outpost_building.h"
#include "outpost_guild_db_updater.h"
#include "common_pd.h"

#include "cdb_group.h"
#include "database_outpost.h"

/**
 * All classes in this file : 
 * \author Nicolas Brigand
 * \author Olivier Cado
 * \author Sebastien Guignot
 * \author Jerome Vuarand
 * \author Nevrax France
 * \date 2005
 */


/// forward declarations
class CGuild;
class COutpost;
class CStaticOutpost;
class CGuildCharProxy;

namespace NLLIGO
{
	class IPrimitive;
}

/**
 * An outpost in the EGS.
 * Contains data read from primitive ( static data ) and persistent dynamic data
 */
class COutpost
: public CPVPOutpostZone
{
	friend class COutpostGuildDBUpdater;
	NLMISC_COMMAND_FRIEND(outpostSetFightData);

public:
	
	/// outposts are persistent and stored through PDR
	DECLARE_PERSISTENCE_METHODS
	
public:
	enum TChallengeOutpostErrors {
		NoError = 0,
		InvalidUser,
		InvalidOutpost,
		NoGuildModule,
		BadGuildGrade,
		BadGuildMemberLevel,
		NotEnoughMoney,
		AlreadyAttacked,
		AlreadyOwned,
		TimePeriodEstimationChanged,
		TooManyGuildOutposts,
		UnknownError,
	};
	
	enum TEditingAccessType
	{
		EditSquads,
		EditExpenseLimit,
		EditDefenseHour,

		NbEditingAccessType
	};
	
	enum TBroadcastAudience
	{
		OwnerGuild,
		AttackerGuild,
		OwnerFighters,
		AttackerFighters,
		
		NbBroadcastAudience
	};
	enum TBroadcastMessage
	{
		RoundNearEnd,
		RoundLost,
		RoundWon,
		LastRoundLost,
		LastRoundWon,
		AttackFailed,
		AttackSucceeded,
		DefenseFailed,
		DefenseSucceeded,
		AttackRounds,
		DefenseRounds,
		WarDeclared,
		
		NbBroadcastMessage
	};
	
public:
	COutpost();
	
	/// build an outpost from a primitive
	bool build(const NLLIGO::IPrimitive* prim,const std::string &filename,const std::string &dynSystem, CONTINENT::TContinent continent);
	
	/// return true if this building was awaited to be built
	bool onBuildingSpawned(CCreature *pBuilding);

	/// check if the building can be constructed on this outpost on the slot provided
	bool canConstructBuilding(const NLMISC::CSheetId &sid, const COutpostBuilding *slot) const;

	/// called only by the command : outpostAccelerateConstruction
	void setConstructionTime(uint32 nNbSeconds, uint32 nCurrentTime);

	/// accessors
	//@{
	TAIAlias getAlias() const { return _Alias; }
	const std::string& getName() const { return _Name; }
	const NLMISC::CSheetId& getSheet() const { return _Sheet; }
	/// returns the state of the outpost
	OUTPOSTENUMS::TOutpostState getState() const { return _State; }
	std::string getStateName() const;
	uint32 getCurrentLevel() const { return _CurrentOutpostLevel; }
	void setOutpostCurrentLevel(uint32 level) { _CurrentOutpostLevel = level; }

	/// returns the sheet of the outpost
	const CStaticOutpost* getStaticForm() const { return _Form; }

	/// return true if the owner of the outpost is a guild (not a tribe)
	bool isBelongingToAGuild() const;

	/// get challenge cost (price to attack this outpost)
	uint32 getChallengeCost() const;
	
	/// get a random spawn zone of the outpost
	TAIAlias getRandomSpawnZone() const;
	//@}

	/// \name outpost commands
	//@{
	/// assign the outpost to a guild (defender). Set 0 for tribe
	void setOwnerGuild(EGSPD::TGuildId ownerGuild);
	/// set the attacker guild (opponent) of the outpost. Set 0 for none
	void setAttackerGuild(EGSPD::TGuildId attackerGuild);
	/// return the id of the guild owning the outpost (defender), or 0 if a tribe owns the outpost
	EGSPD::TGuildId getOwnerGuild() const { return _OwnerGuildId; }
	/// return the id of the guild attacking the outpost (opponent), or 0 in peace time
	EGSPD::TGuildId getAttackerGuild() const { return _AttackerGuildId; }
	
	/// challenges the outpost (war declaration). attackerGuild must be non-null.
	TChallengeOutpostErrors challengeOutpost(CGuild* attackerGuild, bool simulate = false);
	
	/// if the attacking guild want to stop the attack (or counter attack)
	void giveupAttack();
	/// if the attacking guild want to stop the attack (or counter attack)
	void giveupOwnership();
	
	void ownerGuildVanished();
	void attackerGuildVanished();
	
	/// set a squad in the given slot
	bool setSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, uint32 shopSquadIndex);
	/// set the spawn zone of a squad
	bool setSquadSpawnZone(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, uint32 spawnZoneIndex);
	/// insert a default squad before the given slot
	bool insertDefaultSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot);
	/// remove the given squad slot
	bool removeSquad(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot);

	/// set owner expense limit
	void setOwnerExpenseLimit(uint32 expenseLimit);
	/// set attacker expense limit
	void setAttackerExpenseLimit(uint32 expenseLimit);

	/// dump outpost
	void dumpOutpost(NLMISC::CLog & log) const;
	/// get a one line description string (for debug)
	std::string toString() const;
	//@}
	
	/// PVP zone management (IPVPZone implementation)
	//@{
	virtual bool leavePVP(CCharacter * user, IPVP::TEndType type);
	virtual void addPlayer(CCharacter * user);
	virtual bool isCharacterInConflict(CCharacter *user) const;
	virtual PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * user, CEntityBase * target ) const;
	//@}
	
	/// get PVP mode to send to the client (IPVP implementation)
	//@{
	virtual PVP_MODE::TPVPMode getPVPMode() const { return PVP_MODE::PvpZoneOutpost; }
	//@}
	
	/// Init
	//@{
	/// called on outposts that have never been saved before
	void initNewOutpost();
	/// register outpost in its owner/attacker guilds if any
	void registerOutpostInGuilds();
	//@}
	
	/// @name Manager used methods
	//@{
	/// return the AI instance number of the continent of the outpost
	uint32 getAIInstanceNumber() const;
	
	/// send dynamic data to the corresponding AIS, when restarting EGS or
	/// AIS. aisUp() and aisDown() must be called alternatively (not two
	/// aisUp() between two aisDown()).
	void aisUp();
	/// process dynamic data when the corresponding AIS is down. See aisUp().
	void aisDown();

	bool isAISUp() const { return _AISUp; }
	
	/// update the outpost state depending on the elapsing time
	/// \param currentTime : seconds since 1970
	void updateOutpost(uint32 currentTime);
	
	/// simulate a timer end
	void simulateTimer0End(uint32 endTime = 1);
	/// simulate a timer end
	void simulateTimer1End(uint32 endTime = 1);
	/// simulate a timer end
	void simulateTimer2End(uint32 endTime = 1);
	//@}
	
	/// @name AIS events
	//@{
	/// called when a squad was created
	void aieventSquadCreated(uint32 createOrder, uint32 groupId);
	/// called when a squad spawned
	void aieventSquadSpawned(uint32 groupId);
	/// called when a squad despawned
	void aieventSquadDespawned(uint32 groupId);
	/// called when a squad died
	void aieventSquadDied(uint32 groupId);
	/// called when a squad leader died
	void aieventSquadLeaderDied(uint32 groupId);
	//@}
	
	/// @name Squad used methods
	//@{
	/// return the AIS Id corresponding to the instance of the continent of the outpost, or 0 (with a nlwarning) if not found (AIS not online...)
	NLNET::TServiceId getAISId() const;
	
	/// send a message to the AIS hosting the outpost
	template <class T>
		void sendOutpostMessage(const std::string& msgName, T& paramStruct)
	{
		NLNET::TServiceId aisId = getAISId();
		if ( aisId.get() != 0 )
		{
			NLNET::CMessage msgout( msgName );
			msgout.serial( paramStruct );
			sendMessageViaMirror( aisId, msgout );
		}
	}
	//@}
	
	/// @name AIS interface
	//@{
	/// initialize default squads
	void resetDefaultAttackSquads();
	/// initialize default squads
	void resetDefaultDefenseSquads();
	//@}


	/// Banishment management	
	bool isPlayerBanishedForAttack( NLMISC::CEntityId& id ) const;
	bool isPlayerBanishedForDefense( NLMISC::CEntityId& id ) const;
	bool isGuildBanishedForAttack( uint32 guildId ) const;
	bool isGuildBanishedForDefense( uint32 guildId ) const;
	void banishPlayerForAttack( NLMISC::CEntityId& id );
	void banishPlayerForDefense( NLMISC::CEntityId& id );
	void banishGuildForAttack( uint32 guildId );
	void banishGuildForDefense( uint32 guildId );
	void unBanishPlayerForAttack( NLMISC::CEntityId& id );
	void unBanishPlayerForDefense( NLMISC::CEntityId& id );
	void unBanishGuildForAttack( uint32 guildId );
	void unBanishGuildForDefense( uint32 guildId );
	void clearBanishment();
	
	/// Time
	void timeSetAttackHour(uint32 val);
	void timeSetDefenseHour(uint32 val);

	/// return false if the given editing access has been rejected (concurrency issue)
	/// NOTE: this method sends a message to the player if the access is rejected
	bool submitEditingAccess(OUTPOSTENUMS::TPVPSide side, NLMISC::CEntityId playerId, TEditingAccessType accessType);

	static std::string getErrorString(TChallengeOutpostErrors error);

	

private:
	/// an event affecting outpost state occured
	void eventTriggered(OUTPOSTENUMS::TOutpostEvent event, void* eventParams = NULL);
	/// an event affecting outpost state occured and has not been handled
	/// WARNING: it should only be called from eventTriggered
	void eventException(OUTPOSTENUMS::TOutpostEvent event, void* eventParams);
	
	// State machine parameters
	uint32 computeRoundCount() const;
	uint32 computeRoundTime() const;
	uint32 computeFightTime() const;
	uint32 computeLevelDecrementTime() const;
	uint32 computeSpawnDelay(uint32 roundLevel) const;
	uint32 computeSquadCountA(uint32 roundLevel) const;
	uint32 computeSquadCountB(uint32 roundLevel) const;
	uint32 computeChallengeTime() const;
	static uint32 s_computeAttackHour(uint32 challengeHour, uint32 attackHour);
	uint32 computeDefenseHour() const;
	static uint32 s_computeTimeBeforeAttack(uint32 challengeHour, uint32 attackHour);
	uint32 computeTimeBeforeAttack() const;
	uint32 computeTimeAfterAttack() const;
	uint32 computeTimeBeforeDefense() const;
	uint32 computeTimeAfterDefense() const;
	uint32 computeMinimumTimeToNextFight() const;
	uint8 computeStatusForClient() const;
	uint32 computeStateEndDateTickForClient() const;
	uint32 computeRoundEndDateTickForClient() const;
	uint32 computeTimeRangeAttForClient() const;
	uint32 computeTimeRangeDefForClient() const;
	uint32 computeTimeRangeLengthForClient() const;
	uint32 computeTribeOutpostLevel() const;
	uint32 computeGuildMinimumOutpostLevel() const;
	
public:
	static uint32 s_computeEstimatedAttackTimeForClient(uint32 hour);
private:
	// Actions
	void actionPostNextState(OUTPOSTENUMS::TOutpostState state);
	void actionSetTimer0(uint32 seconds);
	void actionSetTimer0End(uint32 seconds);
	void actionSetTimer1(uint32 seconds);
	void actionSetTimer2(uint32 seconds);
	void actionStopTimer0();
	void actionStopTimer1();
	void actionStopTimer2();
	void actionSetPVPActive(bool active);
	void actionSpawnSquadsA(uint32 squadCount, OUTPOSTENUMS::TPVPSide side);
	void actionSpawnSquadsB(uint32 squadIndex, OUTPOSTENUMS::TPVPSide side);
	void actionBuySquadsA(uint32 squadCount, OUTPOSTENUMS::TPVPSide side);
	void actionBuySquadsB(uint32 squadIndex, OUTPOSTENUMS::TPVPSide side);
	void actionPayBackAliveSquads(OUTPOSTENUMS::TPVPSide side);
	void actionDespawnAllSquads();
	void actionChangeOwner();
	void actionCancelOutpostChallenge();
	void actionResetOwnerExpenseLimit();
	void actionResetAttackerExpenseLimit();
	void actionResetMoneySpent();
	void actionPayBackMoneySpent();

	// State data
	struct SFightData
	{
		SFightData()
		: _SpawnedSquadsA(0)
		, _SpawnedSquadsB(0)
		, _KilledSquads(0)
		, _CurrentCombatLevel(0)
		, _CurrentCombatRound(0)
		, _MaxAttackLevel(0)
		, _MaxDefenseLevel(0)
		{ }
		uint32 _SpawnedSquadsA;
		uint32 _SpawnedSquadsB;
		uint32 _KilledSquads;
		uint32 _CurrentCombatLevel;
		uint32 _CurrentCombatRound;
		uint32 _MaxAttackLevel;
		uint32 _MaxDefenseLevel;
	} _FightData;
	
private:
	/// called before storing a pdr save record
	void preStore() const;
	/// called before applying a pdr save record
	void preLoad();
	/// called after applying a pdr save record for all generic processing work
	void postLoad();
	void postLoadSquad(COutpostSquadData & squadData);

	void setNextAttackSquadA(uint32 index, COutpostSquadData const& squadData);
	void setNextAttackSquadB(uint32 index, COutpostSquadData const& squadData);
	void setNextDefenseSquadA(uint32 index, COutpostSquadData const& squadData);
	void setNextDefenseSquadB(uint32 index, COutpostSquadData const& squadData);
	
	bool createSquad(COutpostSquadPtr& squad, COutpostSquadData const& squadData, CGuildCharProxy* leader, CGuild* originGuild, OUTPOSTENUMS::TPVPSide side);
	
public: //for commands
	/// changes the state of the outpost
	void setState(OUTPOSTENUMS::TOutpostState state);
private:

	/// get an attack/defense squad from slot
	bool getSquadFromSlot(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot, std::vector<COutpostSquadData> * &squads, uint32 & squadIndex);
	COutpostSquadData * getSquadFromSlot(OUTPOSTENUMS::TPVPSide side, uint32 squadSlot);

	/// get the index of the spawn zone (for client database)
	bool getSpawnZoneIndex(TAIAlias spawnZoneAlias, uint32 & spawnZoneIndex) const;

	/// convert shop squad index to squad alias
	/// \param shopSquadIndex : index of the squad in the squad shop
	/// \param squadDesc : return the squad descriptor
	/// \return false if index cannot be converted
	bool convertShopSquadIndex(uint32 shopSquadIndex, COutpostSquadDescriptor & squadDesc) const;
	/// convert spawn zone index to spawn zone alias
	/// \param spawnZoneIndex : index of the spawn zone
	/// \param spawnZoneAlias : return the alias of the spawn zone
	/// \return false if index cannot be converted
	bool convertSpawnZoneIndex(uint32 spawnZoneIndex, TAIAlias & spawnZoneAlias) const;

	/// ask an update in the guild database for this outpost
	void askGuildDBUpdate(COutpostGuildDBUpdater::TDBPropSet dbPropSet) const;

	/// ask an update of the outpost database
	void askOutpostDBUpdate();

	/// update timers for client
	/// the updated values are accessible with the following methods:
	/// 	computeStateEndDateTickForClient()
	///		computeRoundEndDateTickForClient()
	void updateTimersForClient();

	std::string getBroadcastString(TBroadcastMessage message) const;
	void broadcastMessageMsg(std::vector<TDataSetRow> const& audience, std::string const& message, TVectorParamCheck const& params=TVectorParamCheck()) const;
	void broadcastMessagePopup(std::vector<TDataSetRow> const& audience, std::string const& message, TVectorParamCheck const& params=TVectorParamCheck(), TVectorParamCheck const& paramsTitle=TVectorParamCheck()) const;
	void broadcastMessage(TBroadcastAudience audience, TBroadcastMessage message) const;
	
private:
	/// @name Static data read from primitive
	//@{
	/// alias of the outpost
	TAIAlias								_Alias;
	/// name of the outpost
	std::string								_Name;
	/// Georges sheet id of the outpost, read from an outpost sheet
	NLMISC::CSheetId						_Sheet;
	/// continent of the outpost
	CONTINENT::TContinent					_Continent;
	
	/// current buildings
	std::vector< COutpostBuilding >			_Buildings;

	/// squads to give to a tribe owner (initial squads)
	std::vector<COutpostSquadDescriptor>	_TribeSquadsA;
	/// squads to give to a tribe owner (following squads)
	std::vector<COutpostSquadDescriptor>	_TribeSquadsB;
	/// default squads to give to outpost owners (ie these squads are free), the first one is the default one
	std::vector<COutpostSquadDescriptor>	_DefaultSquads;
	/// squads that can be bought
	std::vector<COutpostSquadDescriptor>	_BuyableSquads;
	/// spawn zones of the outpost
	std::vector<COutpostSpawnZone>			_SpawnZones;
	
	/// PVP type allowed in the outpost
	OUTPOSTENUMS::TPVPType					_PVPType;
	
	/// outpost sheet
	const CStaticOutpost*					_Form;
	//@}
	
	/// @name Dynamic persistent data
	//@{
	/// current state of the outpost
	OUTPOSTENUMS::TOutpostState				_State;
	/// the id of the guild owning the outpost (defender), or 0 if a tribe owns the outpost
	EGSPD::TGuildId							_OwnerGuildId;
	/// the id of the guild attacking the outpost (opponent), or 0 in peace time
	EGSPD::TGuildId							_AttackerGuildId;
	/// squads to use in next attack round (initial squads), they belong to outpost owner
	std::vector<COutpostSquadData>			_NextAttackSquadsA;
	/// squads to use in next attack round (following squads), they belong to outpost owner
	std::vector<COutpostSquadData>			_NextAttackSquadsB;
	/// squads to use in next defense round (initial squads), they belong to outpost attacker
	std::vector<COutpostSquadData>			_NextDefenseSquadsA;
	/// squads to use in next defense round (following squads), they belong to outpost attacker
	std::vector<COutpostSquadData>			_NextDefenseSquadsB;
	/// date of the end of timer 0
	uint32									_Timer0EndTime;
	/// date of the end of timer 1
	uint32									_Timer1EndTime;
	/// date of the end of timer 2
	uint32									_Timer2EndTime;
	/// status of the AIS
	bool									_AISUp;
	/// level to reach in next outpost attack
	uint32									_CurrentOutpostLevel;
	/// squads to use in current round (initial squads, to spawn)
	std::vector<COutpostSquadData>			_CurrentSquadsAQueue;
	/// squads to use in current round (following squads, spawned)
	std::vector<COutpostSquadData>			_CurrentSquadsBQueue;

	/// expense limit allowed by owner
	uint32									_OwnerExpenseLimit;
	/// expense limit allowed by attacker
	uint32									_AttackerExpenseLimit;
	/// money spent by owner for the current battle
	uint32									_MoneySpentByOwner;
	/// money spent by attacker for the current battle
	uint32									_MoneySpentByAttacker;

	/// tells if last outpost challenge ended in a server crash
	bool									_CrashHappened;
	
	/// Timings
	///
	/// Challenge starts at _ChallengeHour on day D (or _ChallengeTime since 1970)
	/// Attack starts at _ChallengeHour+24*hours
	/// Attack fight starts at (_ChallengeTime-_ChallengeHour)+24*hours+_AttackHour + (_AttackHour<_ChallengeHour?24*hours:0)
	/// Defense starts at _ChallengeHour+24*hours
	/// Attack fight starts at (_ChallengeTime-_ChallengeHour)+48*hours+_DefenseHour + (_DefenseHour<_ChallengeHour?24*hours:0)
	//@{
	uint32	_RealChallengeTime;	///< Absolute time of challenge start (when challenge is declared), in seconds since 1970 (UTC)
	uint32	_ChallengeTime;	///< Absolute time of challenge start (aligned on hour boundary), in seconds since 1970 (UTC)
	uint32	_ChallengeHour;	///< Hour in the day, starting at UTC midnight, from 0 to 23
	uint32	_AttackHour;	///< Hour in the day, starting at UTC midnight, from 0 to 23
	uint32	_DefenseHour;	///< Hour in the day, starting at UTC midnight, from 0 to 23
	//@}
	//@}
	
	/// \name Temporary data
	//@{
	/// next state of the outpost
	OUTPOSTENUMS::TOutpostState				_NextState;
	/// squads used in current round (initial squads)
	std::vector<COutpostSquadPtr>			_CurrentSquadsA;
	/// squads used in current round (following squads, spawned)
	std::vector<COutpostSquadPtr>			_CurrentSquadsB;

	/// outpost editing access (used for managing concurrency in outpost editing)
	struct CEditingAccess
	{
		CEditingAccess() : PlayerId(NLMISC::CEntityId::Unknown), Tick(0) {}

		NLMISC::CEntityId	PlayerId;
		NLMISC::TGameCycle	Tick;
	};
	/// last editing accesses
	CEditingAccess	_LastOwnerEditingAccess[NbEditingAccessType];
	CEditingAccess	_LastAttackerEditingAccess[NbEditingAccessType];
	//@}

	/// guilds banished for this outpost
	std::set<uint32>		_DefenseBanishedGuilds;
	std::set<uint32>		_AttackBanishedGuilds;

	/// players banished for this outpost
	std::set<NLMISC::CEntityId>		_DefenseBanishedPlayers;
	std::set<NLMISC::CEntityId>		_AttackBanishedPlayers;

	
	/// @name CDB stuff
	//@{
public:
	void addOutpostDBRecipient(NLMISC::CEntityId const& player);
	void removeOutpostDBRecipient(NLMISC::CEntityId const& player);
	void fillCharacterOutpostDB( CCharacter * user );

private:
//	void setClientDBProp(std::string const& prop, sint64 value );
//	void setClientDBPropString(std::string const& prop, const ucstring &value );
//	sint64 getClientDBProp(std::string const& prop);
	void fillOutpostDB();
	void sendOutpostDBDeltas();

private:
	/// CDB group for outpost state
//	CCDBGroup	_DbGroup;	
	CBankAccessor_OUTPOST	_DbGroup;

	bool		_NeedOutpostDBUpdate;

	/// last update of timers for client (cf. STATE_END_DATE and ROUND_END_DATE)
	uint32		_LastUpdateOfTimersForClient;
	uint32		_StateEndDateTickForClient;
	uint32		_RoundEndDateTickForClient;
	//@}

	/// @command accessors, must not be used elsewhere
	//@{
public:
	uint32	getRealChallengeTime() const { return _RealChallengeTime; }
	uint32	getChallengeTime() const { return _ChallengeTime; }
	uint32	getChallengeHour() const { return _ChallengeHour; }
	void	setRealChallengeTime(uint32 t) { _RealChallengeTime = t; }
	void	setChallengeTime(uint32 t) { _ChallengeTime = t; }
	void	setChallengeHour(uint32 t) { _ChallengeHour = t; }
};


#endif
