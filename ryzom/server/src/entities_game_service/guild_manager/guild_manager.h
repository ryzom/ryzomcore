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



#ifndef RY_GUILD_MANAGER_H
#define RY_GUILD_MANAGER_H

#include "game_share/backup_service_interface.h"

#include "common_pd.h"
#include "egs_utils.h"
#include "guild_manager_interface.h"


class CGuild;
class CGuildInvitation;
class CGuildCharProxy;
class CCharacter;
class CCDBSynchronised;
class ICDBStructNode;

namespace RY_PDS
{ 
	class IPDBaseData;
}

namespace EGSPD
{
	class CGuildContainerPD;
}


/**
 * Singleton used to manage guilds
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CGuildManager : 
	public IGuildManager, 
	public NLMISC::ICommandsHandler
{

	friend class CGuildUnifier;

	CGuildManager();
public:

	virtual const std::string &getCommandHandlerName() const;
	
	///\name Low level/basic features management, interaction with the shard
	//@{
	/// get the singleton instance
	static inline CGuildManager* getInstance();
	// init the guild manager
	static void init();
	/// release the manager
	static void release();
	/// callback for IOS connection
	void onIOSConnection();
	/// callback for PDS connection
//	void onPDSConnection();
	/// Initial load of the guild
	void loadGuilds();
	/// load a guild file
	void loadGuild(const std::string &fileName);
	/// update client database 
	void clientDBUpdate();
	/// return true if guilds have been loaded
	bool loadedGuilds() const;
	/// Create a guild proxy for guild unification
	CGuild *createGuildProxy(uint32 guildId, 
		const ucstring & guildName,
		const uint64 &icon,
		const ucstring & description, 
		EGSPD::CPeople::TPeople race,
		NLMISC::TGameCycle creationDate);
	//@}

	///\name Display infos on guilds
	//@{
	/// dump all guilds names
	void dumpGuilds( bool onlyLocal, NLMISC::CLog & log )const;
	//@}

	///\name guild management
	//@{
	/// Check if all the guild have been loaded from the BS.
	bool guildLoaded()	{ return _GuildLoaded; }

	/// Return a valid free guild id for new guild creation
	uint32 getFreeGuildId();

//	void addGuildsAwaitingString( const ucstring & guildStr, uint32 guildId );
	/// get a guild from its name
	CGuild * getGuildByName( const ucstring & name );
	/// get a guild from its id
	CGuild * getGuildFromId( EGSPD::TGuildId id );
	/// update the guild strings. This method checks if a guild was awaiting for the string id corresponding to the param str. If yes, it updates the guild string id
//	bool updateGuildStringIds( const ucstring & str );
	/// check coherency between character and guild data
	bool checkGuildMemberShip( const EGSPD::TCharacterId & userId, const EGSPD::TGuildId & guildId )const;
	/// a users connects to the game. Register it in our system by creating the appropriate modules
	void playerConnection( CGuildCharProxy & proxy );
	/// add a new invitation to the manager
	inline void addInvitation( CGuildInvitation* invitation );
	/// remove a specific invitation
	void removeInvitation(CGuildInvitation* invitation);
	/// set the GM guild
	bool setGMGuild( uint32 guildId );
	/// return true if the guild is a GM guild
	bool isGMGuild( const EGSPD::TGuildId & guildId );


	/// guild creation user query
	void createGuild(CGuildCharProxy & proxy,const ucstring & guildName,const uint64& icon, const ucstring & description);
	/// Guild creation step 2, executed when SU return name validation
	void createGuildStep2(uint32 guildId, const ucstring &guildName, CHARSYNC::TCharacterNameResult result);

	/// guild deletion
	void deleteGuild(uint32 id);
	/// a player deletes one of his character
	void characterDeleted( CCharacter & user );
	/// update all guild members string ids in guild database
	void updateGuildMembersStringIds();
	/// Rebuild all guild client db
//	void rebuildCliendDB();

	// fill guild info descriptor with all local guilds
	void fillGuildInfos(std::vector<CHARSYNC::CGuildInfo> &guildInfos);

	/// Check all guild member lists against the entity translator
	void checkGuildMemberLists();

	// A character connect/disconnect on another shard, update the online tags
	void	characterConnectionEvent(const NLMISC::CEntityId &eid, bool online);

	/// get raw acces to the guild list
	const EGSPD::CGuildContainerPD *getGuildContainer() const;

	/// A player entity have been removed from eid translator, check all guild member list
	virtual void playerEntityRemoved(const NLMISC::CEntityId &eid);
	//@}

	// PDLIB factory for guild members
	static RY_PDS::IPDBaseData* guildMemberFactoryPD();
	// PDLIB factory for guilds
	static RY_PDS::IPDBaseData* guildFactoryPD();

	//@{
	//@name character to guild master list control
	void storeCharToGuildAssoc(const NLMISC::CEntityId &charId, EGSPD::TGuildId guildId);
	void removeCharToGuildAssoc(const NLMISC::CEntityId &charId, EGSPD::TGuildId guildId);
	EGSPD::TGuildId getCharGuildAssoc(const NLMISC::CEntityId &charId) const;
	//@}

private:

	friend class CGuildFileClassCb;
	friend class CGuildFileCb;

	/// Callback from BSI for file class list
	virtual void callback(const CFileDescriptionContainer& fileList);
	/// Callback from BSI for one guild file
	virtual void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream);

	/// factory to build the guild container through the PD system
//	static void guildManagerLoadCallback(const uint8 & idx,EGSPD::CGuildContainerPD * obj);

	// utility used during loading or importing of guild
	void checkMemberConsistency(CGuild *guildToCheck);
	// utility used during loading or importing of guild
	void registerGuildAfterLoading(CGuild *guildToRegister);
	/// save guild
	void saveGuild( CGuild* guild );

	/// A member have been changed in some way
	void guildMemberChanged(EGSPD::CGuildMemberPD *guildMemberPd);
	/// the member list has been changed (ether add/replace or delete)
	void guildMemberListChanged(EGSPD::CGuildPD *guildPd);

	/// callback from client database when guild name id changed in db (i.e when we set the IOS mapping)
	static void cb_guildNameIdAvailable(CCDBSynchronised *syncDb, ICDBStructNode *node);

	NLMISC_COMMAND_FRIEND(importGuildFile);


	/// check guild name and description
	bool checkGuildStrings(CGuildCharProxy & proxy,const ucstring & name, const ucstring & description);

	/// get raw access to the guild list (not const)
	EGSPD::CGuildContainerPD *getGuildContainer() { return _Container; }


	/// unique instance
	static CGuildManager*						_Instance;

	/// pointer on the GuildContainer. We have to use this trick because of asynchronous loading
	EGSPD::CGuildContainerPD*					_Container;

	/// Structure used to store info about guild file to load
	struct TFileInfo
	{
		std::string		FileName;
		uint32			Timestamp;
	};

	typedef std::map<EGSPD::TGuildId, TFileInfo>		TGuildToLoad;
	/// The list of guild that are waiting to load
	TGuildToLoad						_GuildToLoad;
	/// Flag set to true after all guild have been loaded
	bool								_GuildLoaded;

	/// set of free guild ids
//	std::set<uint32>							_FreeGuildIds;
	/// highest valid guild id
//	uint32										_HighestGuildId;
	/// guild invitations
	std::vector< CGuildInvitation* >			_Invitations;
	/// guild awaiting their names / description from IOS
//	std::multimap<ucstring,EGSPD::TGuildId>		_GuildsAwaitingString;
	/// guild names registered in system. Stored as strings as checks were donne before. Registered names are lower case
	std::set<std::string>						_ExistingGuildNames;
	/// if true, updateGuildMembersStringIds() will be called when IOS is up
	bool										_UpdateGuildMembersStringIds;

	typedef std::map<NLMISC::CEntityId, EGSPD::TGuildId>	TCharToGuildCont;
	/// Global map of character to guild
	TCharToGuildCont				_CharToGuildAssoc;


	// Management of guild creation name validation with SU
	struct TPendingGuildCreate
	{
		NLMISC::CEntityId	CreatorChar;
		ucstring			GuildName;
		uint64				Icon;
		ucstring			Description;
	};

	typedef std::map<uint32, TPendingGuildCreate>	TPendingGuildCreateInfos;
	TPendingGuildCreateInfos	_PendingGuildCreates;


	typedef uint32	TGuildId;
	// Guild unification management
	// changed guild member
	typedef std::map<TGuildId, std::set<NLMISC::CEntityId> >	TChangedMembers;
	TChangedMembers		_ChangedMembers;
	// Changed guild member list
	typedef std::set<TGuildId>		TChangedMemberList;
	TChangedMemberList	_ChangedMemberList;


	NLMISC_COMMAND_HANDLER_TABLE_BEGIN(CGuildManager)
		NLMISC_COMMAND_HANDLER_ADD(CGuildManager, addGuildMember, "Add a member to a guild", "<guildId> <characterEID> [<Leader|HighOfficer|Officer|Member>]");
		NLMISC_COMMAND_HANDLER_ADD(CGuildManager, unloadGuild, "unload a guild (must be local)", "<guildId>");
		NLMISC_COMMAND_HANDLER_ADD(CGuildManager, loadGuild, "load (or reload) a guild", "<guildFileName>  (e.g : guild_00005.bin)");
		NLMISC_COMMAND_HANDLER_ADD(CGuildManager, renameGuild, "rename a guild ", "<guildId> <newName>");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(renameGuild);
	NLMISC_CLASS_COMMAND_DECL(addGuildMember);
	NLMISC_CLASS_COMMAND_DECL(unloadGuild);
	NLMISC_CLASS_COMMAND_DECL(loadGuild);
};

//----------------------------------------------------------------------------
//inline void CGuildManager::addGuildsAwaitingString( const ucstring & guildStr, uint32 guildId )
//{
//	_GuildsAwaitingString.insert( std::make_pair( guildStr, guildId ) );
//}

//----------------------------------------------------------------------------
inline CGuildManager* CGuildManager::getInstance()
{
	nlassert(_Instance);
	return _Instance; 
}

//----------------------------------------------------------------------------
inline void CGuildManager::addInvitation( CGuildInvitation* invitation )
{
	nlassert(invitation);
	_Invitations.push_back(invitation);
}

#endif // RY_GUILD_MANAGER_H

/* End of guild_manager.h */
