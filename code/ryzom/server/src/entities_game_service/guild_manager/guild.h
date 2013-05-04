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

#ifndef RY_GUILD_H
#define RY_GUILD_H

#include "game_share/string_manager_sender.h"
#include "game_share/pvp_clan.h"

#include "egs_pd.h"
#include "cdb_group.h"
#include "mission_manager/mission_template.h"
#include "game_item_manager/guild_inv.h"
#include "outpost_manager/outpost_guild_db_updater.h"
#include "guild_interface.h"
#include "database_guild.h"
#include "mission_manager/mission_guild.h"

class CMissionGuild;
class CGuildMember;

/* Storage class for mission history data.
*/
struct TMissionHistory;


/**
 * A guild in ryzom
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuild :
	public IGuild,
	public EGSPD::CGuildPD,
	public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CGuild);
public:

	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	/// Test if this is a local or proxy guild
	bool isProxy()	const		{ return _Proxy; }
	void setProxy(bool proxy)	{ _Proxy = proxy; }

	/// Set the guild proxy flag
	void setIsProxy(bool isProxy);


	/// init members (called when creating or before loading a guild)
	void initNonPDMembers();
	/// called after creating (the first time)
	void postCreate();

	/// Constructor
	CGuild();
	/// Destructor
	~CGuild();

	///\name Accessors to the guild data
	//@{
	/// return the entity id of the guild
	const NLMISC::CEntityId & getEId() const { return _EId; }
	/// get the IOS id of the guild name
	uint32 getNameId()const;
	/// get the IOS id of the guild description
	uint32 getDescriptionId()const;
	/// get the guild name
	const ucstring & getName()const;
	/// set the guild name
	void setName(const ucstring & str);
	/// get the guild description
	const ucstring & getDescription()const;
	/// set the guild description
	void setDescription(const ucstring & str);
	/// get the guild current session
	uint8 getMembersSession()const;
	/// spend an amount of XP
//	void spendXP( uint32 xp );
	/// add an amount of XP
//	void addXP( uint32 xp );
	/// spend an amount of money
	void spendMoney(uint64 money);
	/// add an amount of money
	void addMoney(uint64 money);
	/// set money
	void setMoney(uint64 money);
	/// clear the guild charge points
//	void clearChargePoints();
	/// add an amount of charge point
//	void addChargePoints( uint32 points );
	/// return true if the specified role master was bought for the guild
//	bool hasRoleMaster( EGSPD::CSPType::TSPType type );
	/// add a role master of the specified type.
//	void addRoleMaster( EGSPD::CSPType::TSPType type );
	/// set a new icon
	void setIcon( uint64 icon );
	/// set the guild building
	void setBuilding(TAIAlias buildingAlias);
	/// set the message of the day
	void setMOTD( const std::string& motd, const NLMISC::CEntityId & eId);
	//@}

	/// dump guild infos
	void dumpGuildInfos( NLMISC::CLog & log );
	/// register the guild in Game subsystems ( Fame, chat groups )
	void registerGuild();
	/// Open the chat channel for the guild
	void openGuildChatGroup();
	/// member to be called when a guild string is updated
	void onGuildStringUpdated(ICDBStructNode *node);
	/// unregister the guild in Game subsystems ( Fame, chat groups )
	void unregisterGuild();
	/// rebuild the client database, use after a name unifier update
//	void rebuildCliendDB();


	///\name Member management
	//@{
	/// add a new member to the guild
	CGuildMember* newMember( const EGSPD::TCharacterId & id, NLMISC::TGameCycle enterTime = 0);
	/// Remove a member of the guild, taking appropriate action if the player is on or off line
	void removeMember(const EGSPD::TCharacterId &id);
	/// delete a member from the guild
	void deleteMember( CGuildMember* member );
	/// get a member of the guild from its index in the guild
	CGuildMember* getMemberByIndex(uint16 index) const;
	/// return the number members in the guild
	uint16 getMemberCount()const;
	/// return the number of members with the specified grade
	uint16 getGradeCount(EGSPD::CGuildGrade::TGuildGrade grade)const;
	/// return the maximum number of members that can have the specified grade
	uint16 getMaxGradeCount(EGSPD::CGuildGrade::TGuildGrade grade)const;
	/// increment a grade counter
	void incGradeCount( EGSPD::CGuildGrade::TGuildGrade grade );
	/// increment a grade counter
	void decGradeCount( EGSPD::CGuildGrade::TGuildGrade grade );
	/// send client datbase deltas
	void sendClientDBDeltas();
	/// flag a user offline ( unregister it in other system, e.g. : chat group )
	void setMemberOffline( CGuildMember * member );
	/// flag a user online ( register it in other system, e.g. : chat group )
	void setMemberOnline( CGuildMember * member, uint8 dynamicId );
	/// add the member into the guild chat
	void addMemberToGuildChat(CGuildMember *member);
	/// send a message to all members
	void sendMessageToGuildMembers( const std::string &  msg, const TVectorParamCheck & params =  TVectorParamCheck() )const;
	/// send a message to all members in guild chat
	void sendMessageToGuildChat( const std::string &  msg, const TVectorParamCheck & params =  TVectorParamCheck() )const;
	/// set information relative to a member in the guild client database
	void setMemberClientDB( CGuildMember* member );
	/// return the best online user
	const EGSPD::TCharacterId getHighestGradeOnlineUser() const;
	/// update members string ids in guild client database
	void updateMembersStringIds();
	/// set member grade (for admin commands only)
	bool setMemberGrade(CGuildMember * member, EGSPD::CGuildGrade::TGuildGrade grade, NLMISC::CLog * log = NULL, const NLMISC::CEntityId & csrEId = NLMISC::CEntityId::Unknown);
	/// get member from entity id
	CGuildMember * getMemberFromEId(NLMISC::CEntityId eId);
	/// get leader
	CGuildMember * getLeader();
	//@}

	///\name Mission management
	//@{
	void removeMission(CMissionGuild * mission, TMissionResult result)
	{
		for (uint i = 0; i < _Missions.size(); i++)
		{
			if ( _Missions[i] == mission )
			{
				removeMission(i, result);
			}
		}
	}
	void removeMission( uint idx, TMissionResult result);
	void addSuccessfulMission(CMissionTemplate * templ);
	void clearSuccessfulMissions();
	void updateMissionHistories(TAIAlias missionAlias, uint32 result);
	bool processMissionEvent( CMissionEvent & event, TAIAlias alias = CAIAliasTranslator::Invalid);
	bool processGuildMissionEvent(std::list< CMissionEvent * > & eventList, TAIAlias missionAlias );
	bool processGuildMissionStepEvent(std::list< CMissionEvent* > & eventList, TAIAlias missionAlias, uint32 stepIndex);
	CMissionGuild* getMissionByAlias( TAIAlias missionAlias );
	bool isMissionSuccessfull(TAIAlias alias);
	void sendDynamicMessageToMembers(const std::string &msgName, const TVectorParamCheck &params, const std::set<NLMISC::CEntityId> &excluded) const;
	///\return the mission
	inline std::vector<CMissionGuild*> & getMissions()
	{
		return _Missions;
	}
	void addMission(CMissionGuild* guildMission)
	{
		_Missions.push_back(guildMission);
		guildMission->updateUsersJournalEntry();
	}
	//@}

	/// inventory management
	///\name inventory management
	//@{
	/// take an item from guild inventory (set quantity to UINT_MAX for 'all stack')
	void	takeItem( CCharacter * user, uint32 slot, uint32 quantity, uint16 session );
	/// put an item in guild inventory (set quantity to UINT_MAX for 'all stack')
	void	putItem( CCharacter * user, uint32 slot, uint32 quantity, uint16 session );
	/// user wanna take money
	void	takeMoney( CCharacter * user, uint64 money, uint16 session );
	/// user wanna put money
	void	putMoney( CCharacter * user, uint64 money, uint16 session );
	/// get a guild item
	const CGameItemPtr getItem( uint32 slot ) const
	{
		if ( slot >= _Inventory->getSlotCount() )
		{
			nlwarning( "guild %u : invalid slot %u : count %u", _Id, slot, _Inventory->getSlotCount() );
			return NULL;
		}
		return _Inventory->getItem(slot);
	}
	/// add an item in the guild inventory (item can be deleted if not inserted : do not use it anymore in any case!)
	bool putItem( CGameItemPtr item );

	class CItemSlotId
	{
	public:
		uint32						Slot;
		uint32						Quality;
		bool	operator<(const CItemSlotId &o) const
		{
			return Quality<o.Quality;
		}
	};

	/// check the presence of an item (or several items in a stack) by its sheetId/quality
	uint selectItems(NLMISC::CSheetId itemSheetId, uint32 quality, std::vector<CItemSlotId> *itemList= NULL);
	/// destroy a list of items (up to maxQuantity to destroy)
	uint destroyItems(const std::vector<CItemSlotId> &itemSlots, uint32 maxQuantity=-1);

	/// return the inventory (const)
	const NLMISC::CSmartPtr<CGuildInventory>& getInventory() const { return _Inventory; }
	/// store for a character and return the current info version for an item of the guild inventory
	uint8	getAndSyncItemInfoVersion( uint32 slot, const NLMISC::CEntityId& characterId );
	//@}

	///\name outposts
	//@{
	/// return true if the guild can get one more outpost
	bool canAddOutpost() const;
	/// add an outpost that guild owns (it also updates outpost client database)
	void addOwnedOutpost(TAIAlias outpostAlias);
	/// remove an outpost that guild owned (it also updates outpost client database)
	void removeOwnedOutpost(TAIAlias outpostAlias);
	/// add an outpost that guild challenges (it also updates outpost client database)
	void addChallengedOutpost(TAIAlias outpostAlias);
	/// remove an outpost that guild challenged (it also updates outpost client database)
	void removeChallengedOutpost(TAIAlias outpostAlias);

	/// return true if the guild is allowed to give up an outpost
	bool canGiveUpOutpost() const;
	/// update the flag GUILD:OUTPOST:CANDEL in client database
	void updateGUILD_OUTPOST_CANDEL();

	/// get the total number of outposts owned or challenged by the guild
	uint32 getOutpostCount() const;
	/// get owned outposts
	void getOwnedOutposts(std::vector<TAIAlias> & ownedOutposts) const;
	/// get challenged outposts
	void getChallengedOutposts(std::vector<TAIAlias> & challengedOutposts) const;

	/// get a guild database updater for the given outpost
	/// WARNING: use it as an accessor (DO NOT keep the object after you used it
	/// because it will not be kept up to date)
	/// \return the updater (can be NULL if the guild does not own or challenge the outpost)
	COutpostGuildDBUpdaterPtr getOutpostGuildDBUpdater(COutpost * outpost);
	//@}

	///\PvP Allegiance
	//@{
	/// get allegiance of guild
	typedef std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan>	TAllegiances;
	/// first member of pair is religion allegiance, second member of pair is people allegiance
	TAllegiances getAllegiance() const;
	/// Sets new declared cult, returns if parameter was within bounds, and thus if new clan as set.
	bool setDeclaredCult(PVP_CLAN::TPVPClan newClan, bool noCheck = false);
	/// Sets new declared civ, returns if parameter was within bounds, and thus if new clan as set.
	bool setDeclaredCiv(PVP_CLAN::TPVPClan newClan, bool noCheck = false);
	/// Set an allegiance to neutral from indetermined status, used for process message from client want set it's allegiance to neutral when it's idetermined, over case are managed by missions.
	void setAllegianceFromIndeterminedStatus(PVP_CLAN::TPVPClan allegiance);
	/// Verifies that can can retain its declared clan allegiances.
	/// returns true if allegiances were kept, false if an allegiance was ended.
	bool verifyClanAllegiance(PVP_CLAN::TPVPClan theClan, sint32 newFameValue);
	//@}

	/// Sets the properties DB for guild fame.
	void setFameValueGuild(uint32 factionIndex, sint32 guildFame, sint32 fameMax, uint8 fameTrend);
	/// Resends all fame value information
	void resetFameDatabase();
	/// init start fame value & allegiance with guild creator fames and allegiance
	void setStartFameAndAllegiance( const NLMISC::CEntityId& guildCreator );

	/// Return a pointer on the client database instance
	CCDBSynchronised &getClientDB();
	/// get a guild client database property
//	sint64 getClientDBProp(const std::string & prop ) const;

protected:
	friend class NLMISC::ICommand;

	/// set a guild client database property
//	void setClientDBProp(const std::string & prop, sint64 value );
	/// set a guild client database property as string
//	void setClientDBPropString(const std::string & prop, const ucstring &value );

private:
	/// add a player in the guild chat group
	//void addPlayerToChatGroup( const EGSPD::TCharacterId & userId );

	/// return true if player character is in guild building
	bool isInGuildBuilding( const TDataSetRow & user );

	/// return true if user can access to it's guild inventory
	bool canAccessToGuildInventory( CCharacter * user );

	/// Go through the guild membership and fix members with mistmatched allegiances.
	///  Called after the Guild's allegiance changes, only affects online members.
	void verifyGuildmembersAllegiance();

	/// increment the guild member session state counter
	void incMemberSession();

	/// get the outpost index in database from alias
	/// \param outpostAlias : outpost alias
	/// \param outpostIndex : return the outpost index
	/// \param ownedOutpost : return if the outpost is owned or challenged by the guild
	/// \return false if outpost was not found
	bool getOutpostDBIndex(TAIAlias outpostAlias, uint32 & outpostIndex, bool & ownedOutpost) const;

	/// update the outpost at the given index in database
	bool updateOutpostDB(uint32 outpostIndex);

	/// get a guild database updater for the given outpost (INTERNAL USE ONLY)
	COutpostGuildDBUpdaterPtr getOutpostGuildDBUpdaterDetailed(COutpost * outpost, uint32 outpostIndex, bool ownedOutpost);

private:

	/// Flag indicating whether this guild is local or proxyfied
	bool								_Proxy;

	/// member session : incremented each time member data changes. Used to ensure that client
	uint8								_MembersSession;
	/// for each grade we keep a counter of the members having this grade
	NLMISC::CObjectVector<uint16>		_GradeCounts;
	/// the guild client database node
//	CCDBGroup							_DbGroup;
	CBankAccessor_GUILD					_DbGroup;

	/// member index that have been free because of user removal
	std::set<uint16>					_FreeMemberIndexes;
	/// next member index to use( = max index + 1 )
	uint16								_NextMemberIndex;
	/// entity id of the guild
	NLMISC::CEntityId					_EId;

	/// The name of the guild
	ucstring							_Name;
	/// The description of the guild
	ucstring							_Description;
	// words of the day
	ucstring							_MessageOfTheDay;

	// The declared Cult and Civilization information for the guild for fame purposes.
	PVP_CLAN::TPVPClan					_DeclaredCult;
	PVP_CLAN::TPVPClan					_DeclaredCiv;

	/// the inventory of the guild
	NLMISC::CSmartPtr<CGuildInventory>	_Inventory;
	/// view for guild inventory
	NLMISC::CSmartPtr<CGuildInventoryView> _GuildInventoryView;

	/// list of outposts owned by guild
	std::vector<TAIAlias>				_OwnedOutposts;
	/// list of outposts challenged by guild
	std::vector<TAIAlias>				_ChallengedOutposts;

	///the missions took by the guild
	std::vector<CMissionGuild*>			_Missions;
	/// Successful missions
	std::map<TAIAlias, TMissionHistory> _MissionHistories;

	NLMISC_COMMAND_FRIEND( guildDB );
};
#endif // RY_GUILD_H

/* End of guild.h */
