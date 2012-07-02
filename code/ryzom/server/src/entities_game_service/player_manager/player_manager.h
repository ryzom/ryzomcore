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



#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

// misc
#include "nel/misc/eid_translator.h"
//game share
#include "server_share/pet_interface_msg.h"
#include "server_share/msg_ai_service.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/starting_point.h"
#include "game_share/generic_xml_msg_mngr.h"

#include "entity_manager/entity_manager.h"

#include "player.h"
#include "player_manager_interface.h"

class CCharacter;
class CPlayer;
class CAsyncPlayerLoad;

namespace NLNET
{
	class CLoginCookie;
}

#define USER_CHARS		0
#define NO_USER_CHAR	1
#define USER_CHAR		2

extern CGenericXmlMsgHeaderManager	GenericMsgManager;

class CServiceIdHash
{
public:
	size_t	operator () ( const NLNET::TServiceId &sid ) const { return sid.get(); }
};

class CCharIdReplaced
{
public:
	uint32	CharIndex;
	std::string	Filename;

	CCharIdReplaced() { CharIndex = ~0; }
};

typedef std::set<uint32>								TUint32Set;
typedef CHashMap<NLNET::TServiceId, TUint32Set, CServiceIdHash>	TMapServiceIdToUint32Set;
typedef std::map<uint32, CCharIdReplaced >				TMapUserCharFileReplaced;


//	send message of the day to new connected players
void broadcastMessageOfTheDay(NLMISC::IVariable &var);

//---------------------------------------------------
// Implementation of CPetSpawnConfirmationMsg
//
//---------------------------------------------------
class CPetSpawnConfirmationImp : public CPetSpawnConfirmationMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


/**
 * CPlayerManager
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CPlayerManager 
	:	public CEntityBaseManager,
		public IPlayerManager
{
public:

	typedef CHashMap< uint32, NLNET::TServiceId, IPlayerManager::CUint32Hash >	TMapPlayerFrontEndId;
	typedef std::vector< uint32 >							TVectorDisconnectedUser;

	typedef std::map<uint32, NLMISC::CSmartPtr<CAsyncPlayerLoad> >				TAsyncLoadMap;

	TMapUserCharFileReplaced	UserCharIdReplaced;

private:
	TVectorDisconnectedUser	_DisconnectedUser;
	
	/// map of pointers on all connected player
	IPlayerManager::TMapPlayers	_Players;

	/// keep for each connected client the id of the front end which manages him
	TMapPlayerFrontEndId		_PlayerFrontEndId;

	/// map frontend Ids to the ids of the players connected to this frontend
	TMapServiceIdToUint32Set		_MapFrontendToPlayers;

	/// Character create level, normaly egal to 1
	uint16						_CharacterCreateLevel;

	// the entities waiting name registration in the IOS, process X entities each tick till list empty
//	std::list<NLMISC::CEntityId>	_EntitiesWaitingNameRegistration;

	/// true if eid translator is ready (ie all string ids have been registered)
	bool _EIdTranslatorReady;

	/// online users having special privileges
	std::vector< std::pair<std::string, std::vector<NLMISC::CEntityId> > >	_SpecialUsers;

	/// users summoned by a CSR
	std::map< TDataSetRow, COfflineEntityState >				_SummonedUsers;

	/// Loading players
	TAsyncLoadMap				_LoadingPlayers;

	/// users curesed by a GM
	struct CUserCursedByGM
	{
		CUserCursedByGM( const NLMISC::CEntityId & id, NLMISC::TGameCycle	endDate )
			:Id(id),EndDate(endDate){}
		NLMISC::CEntityId	Id;
		NLMISC::TGameCycle	EndDate;	
	};
	std::list< CUserCursedByGM >	_UsersRootedByGM;
	std::list< CUserCursedByGM >	_UsersMutedByGM;
	std::list< CUserCursedByGM >	_UsersUniversChatMutedByGM;

	/// set containing row ids of player for whom xp gain logging is ON
	std::set<TDataSetRow>	_LogXPGain;

	/// broadcast message variables
	std::string						_BroadcastMessage;
	uint32							_RepeatBroadcastMessage;
	uint32							_EveryBroadcastMessage;
	uint32							_NextBroadcastMessage;

	// stall mode, if true only user with privilege are accepted
	bool							_Stall;

	// temporary for disconnect player with character with bug cant' be handle
	std::vector< uint32 >			_PlayerMustBeDisconnected;

public :
	// Default constructor
	CPlayerManager() { _CharacterCreateLevel = 1; }

	/// exception thrown when player is unknown
	struct EPlayer : public NLMISC::Exception
	{
		EPlayer( uint32 userId ) : Exception ("Player "+NLMISC::toString(userId)+" not found") {}
	};

	/// exception thrown when player's char is unknown
	struct EChar : public NLMISC::Exception
	{
		EChar( const NLMISC::CEntityId& id ) : Exception ("The char "+id.toString()+" doesn't exist") {}
	};

	///init the manager
	void init();

	/// get number of player character in manager
	uint32 getNumberPlayers() { return (uint32)_Players.size(); }

	/// get a reference on player in manager
	const IPlayerManager::TMapPlayers& getPlayers() { return _Players; }

	/**
	 * Add callback for client & characters management
	 */
	void addClientCallback();

	/**
	 * Create a new character index
	 */
	NLMISC::CEntityId createCharacterId( uint32 userId, uint32 index );

	/**
	 *	Set the front end id of a user
	 * \param id is the id of the user
	 * \param serviceId is the id of the front end
	 */
	void setPlayerFrontEndId( uint32 userId, NLNET::TServiceId serviceId );

	/**
	 *	Get the front end id of a player
	 * \param id is the id of the user
	 * \return the id of the front end
	 */
	NLNET::TServiceId getPlayerFrontEndId( uint32 id );

	/** Construct the save path for a character file 
	 *	Build a hierarchical path based on the user id
	 *	in order to spread the character files accros many
	 *	folder (instead of store all in one folder)
	 */
	std::string getCharacterPath(uint32 userId, bool returnRemotePath);

	/** Construct the save path for a character offline command file 
	 *	Build a hierarchical path based on the user id
	 *	in order to spread the character files accros many
	 *	folder (instead of store all in one folder)
	 */
	std::string getOfflineCommandPath(uint32 userId, bool returnRemotePath);

	/**
	 * Load the player infos
	 * \param player will contain the player info
	 */
	void loadPlayer( CPlayer * player );


	/**
	 * Async Load player infos
	 */
	void	asyncLoadPlayer( CPlayer* player, uint32 userId, const std::string& languageId, const NLNET::CLoginCookie& cookie, bool allAuthorized );

	/**
	 * Cancel Async Loading of a player infos
	 */
	void	cancelAsyncLoadPlayer( uint32 userId );

	/// Save the character. Idx is the character index in the player
	void savePlayerChar( uint32 userId, sint32 idx, const std::string *filename = 0);

	/// Save the player active char. 
	void savePlayerActiveChar( uint32 userId, const std::string *filename = 0 );

	// save all player
	void saveAllPlayer();

	// reload active character
	void reloadPlayerActiveChar( uint32 userId, uint32 charIndex = (uint32)~0, const std::string *filename = 0 );

	/**
	 * Add a player in the service
	 * \param id is the unique id of the user
	 * \param player contains the player info
	 */
	void addPlayer( uint32 userId, CPlayer * player );

	/// A player entity have been removed from eid translator, check all guild member list
	virtual void checkContactLists();

	/// A player entity have been removed from eid translator, check all contact list
	virtual void playerEntityRemoved(const NLMISC::CEntityId &eid);

	/**
	 * Get player
	 * \param userId is the unique id of the user
	 * \return the pointer on player, or null if not found
	 */
	CPlayer* getPlayer( uint32 userId );

	/** Get player Id
	 * Get player Id from a character
	 * \param CEntityId of a character
	 * \return corresponding player id
	 */
	inline uint32 getPlayerId( const NLMISC::CEntityId& entityId ) const { return (uint32) ( entityId.getShortId() >> 4 ); }

	/** Get char index
	 * Get char Id from a character's EntityId
	 * \param CEntityId of a character
	 * \return corresponding char id
	 */
	inline uint8 getCharIndex( const NLMISC::CEntityId& entityId ) const { return (uint8)(entityId.getShortId()&0xf); }

	/**
	 * Add a character to the player characters
	 * \param userId is the unique id of the user
	 * \param ch is the new character
	 */
	void addChar( uint32 userId, NLMISC::CEntityId charId, CCharacter * ch, uint32 index );

	/**
	 * Delete character
	 * \param userId is the unique id of the user
	 * \param characterIndex is the index of character
	 */
	void deleteCharacter( uint32 userId, uint32 characterIndex );

	/**
	 * Get the last char index
	 * \return the first free char slot index
	 */
	sint32 getFirstFreeCharacterIndex( uint32 userId );

	/**
	 * Get the character
	 * \param userId is the unique id of the user
	 * \param index is the index of the character
	 * \return pointer on asked CCharacter
	 */
	CCharacter * getChar( uint32 userId, uint32 index );

	/**
	 * Get the character
	 * \param charId is the unique id of the character
	 * \return pointer on asked CCharacter
	 */
	CCharacter * getChar( const NLMISC::CEntityId &charId );

	/**
	 * Get online character
	 * \param charId is the unique id of the character
	 * \return CCharacter* if online and exist
	 */
	CCharacter * getOnlineChar( const NLMISC::CEntityId &charId );

	/**
	 * Get the character
	 * \return CCharacter* the character
	 */
	CCharacter * getChar( const TDataSetRow &rowId );

	/**
	 * Get online character
	 * \return CCharacter* character
	 */
	inline CCharacter * getOnlineChar( const TDataSetRow &rowId )
	{
		return getChar( rowId );
	}
	
	/**
	 * Get the active character
	 * \param userId is the unique id of the user
	 * \return the character
	 */
	CCharacter * getActiveChar( uint32 userId );

	/**
	 * Set the active character for the specified player
	 * \param userId is the unique id of the user
	 * \param index is the index of the character
	 */
	void setActiveCharForPlayer( uint32 userId, uint32 index, NLMISC::CEntityId charId );

	/**
	 *  Disconnect a user, remove him from the _Players list
	 * \param id is the unique id of the char
	 */
	void disconnectPlayer( uint32 userId );

	/**
	 *  when a frontend has been disconnected, backup and remove all player connected to it
	 * \param serviceId is the unique id of the frontend
	 */
	void disconnectFrontend( NLNET::TServiceId serviceId );

	/**
	 * When GPMS is up, init subscribtion for positions and insert entity in player manager in GPMS
	 */
	void gpmsConnexion();

	/**
	 * Return the state of the active char
	 */
	const CEntityState& getState( const NLMISC::CEntityId& id );

	/**
	 * Return the type of the active char (sheet Id)
	 */
	NLMISC::CSheetId getType( const NLMISC::CEntityId& id );

	/**
	 * Set the enter flag 
	 * \param playerId is the player id
	 * \param b true if the player entered the game, false if he left
	 */
	void setEnterFlag( const NLMISC::CEntityId& playerId, bool b );

	/**
	 *	Set the value of a var
	 * \param playerId is the player id
	 * \param var is the name of the variable
	 * \param value is the new value for the variable
	 */
	void setValue( NLMISC::CEntityId charId, std::string var, std::string value );

	/**
	 *	Modify a var
	 * \param charId is the character id
	 * \param var is the name of the variable
	 * \param value is the modifier to apply to the variable value
	 */
	void modifyValue( NLMISC::CEntityId charId, std::string var, std::string value );
	
	/**
	 *	Return the value of the variable
	 * \param playerId is the player id
	 * \param var is the name of the variable
	 */
	std::string getValue( NLMISC::CEntityId playerId, std::string var );

	/**
	 *	Register all character name to IOS when service is up
	 */
	void registerCharacterName( );

	/**
	 * tick update, called every tick
	 */
	void tickUpdate();

	/**
	 * return reference on character map
	 */
	//std::map< NLMISC::CEntityId, CCharacter *>& getCharacter() { return _Characters; }

	/**
	 * Synchronize client ryzom time and day
	 */
	void synchronizeClientRyzomTime( uint32 ticks, float time, uint32 day );

	
	/**
	 * Re-spawn pet of characters for AI instance
	 */
	void respawnPetForInstance( uint32 InstanceNumber, const std::string& InstanceContinent );
	/**
	 * Respawn the groups handled (mission hold a handle on those groups) when ais restart
	 */
	void respawnHandledAIGroupForInstance( uint32 InstanceNumber );
	

	inline uint16 characterCreateLevel() { return _CharacterCreateLevel; }
	inline void characterCreateLevel( uint16 l ) { _CharacterCreateLevel = l; }

	/// return character corresponding to name, use only for tests TIME consuming !
	CCharacter * getCharacterByName( const std::string& name );

	/// update regen value dur to variable change in cfg file
	void updateRegen();

	// return true if the user have the privilege
	bool havePriv (uint32 userId, const std::string &priv) const;
	bool havePriv (const NLMISC::CEntityId &eid, const std::string &priv) const
	{
		return havePriv(getPlayerId(eid), priv );
	}

	// return true is the user have any privilege
	bool haveAnyPriv(uint32 userId) const;
	bool haveAnyPriv(const NLMISC::CEntityId &eid) const
	{
		return haveAnyPriv(getPlayerId(eid));
	}

	/**
	 * Set name/stringId association, returns true if association has been succesfully set.
	 */
	bool setStringId( const ucstring &str, uint32 stringId);

	// add an entity for string ids request
//	inline void addEntityForStringIdRequest( const NLMISC::CEntityId &entity)
//	{
//		// ace: don't add all players for loading all name
////		return;
//		_EntitiesWaitingNameRegistration.push_back(entity);
//	}

	// add ALL characters for string ids request
//	void addAllCharForStringIdRequest();

	/// add a special user
	void addSpecialUser( const std::string & priv, const NLMISC::CEntityId & user );

	/// return the special users with the appropriated privilege
	const std::vector<NLMISC::CEntityId> * getSpecialUsers(const std::string& priv);

	/// summon a user
	void addSummonnedUser( const TDataSetRow & userRow, const COfflineEntityState & gmState )
	{
		_SummonedUsers.insert( std::make_pair( userRow, gmState ) );
	}

	/// remove a summoned user
	void removeSummonedUser( const TDataSetRow & userRow )
	{
		_SummonedUsers.erase( userRow );
	}

	/// get the dissmiss coords of a user ( coords used to teleport back a player after a summon command )
	bool getDismissCoords( const TDataSetRow & userRow, COfflineEntityState & state );

	/// dismmiss a summoned user ( teleport back )
	void dismissSummonedUser( const NLMISC::CEntityId & gmId, const TDataSetRow & userRow );

	/// return true if the user was rooted by a GM 
	bool isRootedByGM( const NLMISC::CEntityId & userId );

	/// add a GM root power on a user
	void addGMRoot( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId, NLMISC::TGameCycle endDateInTicks );
	
	/// remove a GM root power on a user
	void removeGMRoot( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId );

	/// add a GM mute power on a user
	void addGMMute( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId, NLMISC::TGameCycle endDateInTicks );
	/// remove a GM mute power on a user
	void removeGMMute( const NLMISC::CEntityId & gmId , const NLMISC::CEntityId & targetId );

	// mute the universe channel for a duration
	void muteUniverse( const NLMISC::CEntityId & gmId, NLMISC::TGameCycle endDateInTicks, const NLMISC::CEntityId & targetId );
	// un-mute the universe channel
	void unmuteUniverse( const NLMISC::CEntityId & gmId, const NLMISC::CEntityId & targetId );

	/// return true if first param character has better CSR rights
	bool hasBetterCSRGrade( const NLMISC::CEntityId& entityId1, const NLMISC::CEntityId& entityId2, bool devIsNormalPlayer=false );

	/// return true if first param character has better CSR rights (pointers must be non-null)
	bool hasBetterCSRGrade( CCharacter * user1, CCharacter * user2, bool devIsNormalPlayer=false );

	/// return true if first param character has better CSR rights (pointers must be non-null)
	bool hasBetterCSRGrade( CPlayer* p1, CPlayer * p2, bool devIsNormalPlayer=false );

	/// return true if player xp gain log is ON
	inline bool logXPGain(const TDataSetRow &rowId )
	{
		return (_LogXPGain.find(rowId) != _LogXPGain.end());
	}

	/// set xp gain logging to ON/OFF for player
	inline void logXPGain(const TDataSetRow &rowId, bool b )
	{
		if (b)
			_LogXPGain.insert(rowId);
		else
			_LogXPGain.erase(rowId);
	}

	// local macro undef at the end of the file
	#define RY_SEND_IMPULSE_TO_CLIENT_BEGIN \
		NLNET::CMessage msgout( "IMPULSION_ID" ); \
		msgout.serial( const_cast<NLMISC::CEntityId&>(id) ); \
		NLMISC::CBitMemStream bms; \
		GenericMsgManager.pushNameToStream( msgName, bms);

	#define RY_SEND_IMPULSE_TO_CLIENT_END \
		msgout.serialBufferWithSize((uint8*)bms.buffer(), bms.length()); \
		NLNET::CUnifiedNetwork::getInstance()->send( NLNET::TServiceId(id.getDynamicId()), msgout );

	void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		RY_SEND_IMPULSE_TO_CLIENT_END
	}

	template <class P1> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1 )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	template <class P1, class P2> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1, const P2 & p2 )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		bms.serial( const_cast<P2&>(p2) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	template <class P1, class P2, class P3> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1, const P2 & p2, const P3 & p3 )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		bms.serial( const_cast<P2&>(p2) );
		bms.serial( const_cast<P3&>(p3) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	template <class P1, class P2, class P3, class P4> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1, const P2 & p2, const P3 & p3, const P4 & p4 )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		bms.serial( const_cast<P2&>(p2) );
		bms.serial( const_cast<P3&>(p3) );
		bms.serial( const_cast<P4&>(p4) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	template <class P1, class P2, class P3, class P4, class P5> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1, const P2 & p2, const P3 & p3, const P4 & p4, const P5 & p5)
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		bms.serial( const_cast<P2&>(p2) );
		bms.serial( const_cast<P3&>(p3) );
		bms.serial( const_cast<P4&>(p4) );
		bms.serial( const_cast<P5&>(p5) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	template <class P1, class P2, class P3, class P4, class P5, class P6> void sendImpulseToClient(const NLMISC::CEntityId & id,const std::string & msgName, const P1 & p1, const P2 & p2, const P3 & p3, const P4 & p4, const P5 & p5, const P6 & p6 )
	{
		RY_SEND_IMPULSE_TO_CLIENT_BEGIN
		bms.serial( const_cast<P1&>(p1) );
		bms.serial( const_cast<P2&>(p2) );
		bms.serial( const_cast<P3&>(p3) );
		bms.serial( const_cast<P4&>(p4) );
		bms.serial( const_cast<P5&>(p5) );
		bms.serial( const_cast<P6&>(p6) );
		RY_SEND_IMPULSE_TO_CLIENT_END
	}
	

	#undef RY_SEND_IMPULSE_TO_CLIENT_BEGIN
	#undef RY_SEND_IMPULSE_TO_CLIENT_END

	/// broadcast message
	void broadcastMessage( uint32 repeat, uint32 during, uint32 every, const std::string& message );
	void broadcastMessageUpdate();

	// set stall mode
	void setStallMode( bool stall ) { _Stall = stall; }

	// get stall mode 
	bool getStallMode() const { return _Stall; }

	// disconnect all users without privileges due to technical pb on server (as backup service can't save player)
	void forceDisconnectUserWithoutPrivileges();

	// add character must be disconnected
	void addPlayerMustBeDisconnected( uint32 userId ) { _PlayerMustBeDisconnected.push_back( userId ); }

	// a user are disconnected
	void userDisconnected( uint32 userId );

	// The name unifier as renamed a character
	void characterRenamed(uint32 charId, const std::string &newName);

	/// ClientNPCIconRefreshTimerDelay has changed
	static void onNPCIconTimerChanged(NLMISC::IVariable &var);

private:
	// init called after addAllCharForStringIdRequest(), when eid translator is ready (ie all char names have been registered)
	void initWhenEIdTranslatorReady();

	/// see savePlayerChar. Recurse because of the graph of players who made a exchange (must save the whole graph)
	void savePlayerCharRecurs( uint32 userId, sint32 idx, std::set<CCharacter*> &charAlreadySaved, const std::string *filename = 0);
};

extern CPlayerManager PlayerManager;

#endif //PLAYER_MANAGER

