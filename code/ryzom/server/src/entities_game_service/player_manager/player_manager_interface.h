
#ifndef PLAYER_MANAGER_INTERFACE_H
#define PLAYER_MANAGER_INTERFACE_H

#include "nel/misc/entity_id.h"

class CPlayer;
class CCharacter;

/** Interface class to insulate module code from EGS inferno */
class IPlayerManager
{
	static IPlayerManager	*_Instance;
public:

	struct SCPlayer
	{
		CPlayer * Player;
		bool NextSaved;

		SCPlayer()
		{
			Player = 0;
			NextSaved = false;
		}

	};

	class CUint32Hash
	{
	public:
		size_t	operator () ( const uint32 &i ) const { return i; }
	};

	typedef uint32	TUserId;

	typedef CHashMap< TUserId, SCPlayer, CUint32Hash > TMapPlayers;


	IPlayerManager()
	{
		nlassert(_Instance == NULL);

		_Instance = this;
	}


	static IPlayerManager &getInstance()	{ nlassert(_Instance != NULL); return *_Instance; }

	/** Get the map of active players */
	virtual const TMapPlayers& getPlayers() = 0;

	/** Get the active character for the given player id */
	virtual CCharacter * getActiveChar( uint32 userId ) =0;

	/** Get the specified character of a player */
	virtual CCharacter * getChar( uint32 userId, uint32 index ) =0;

	/** A character has been renamed by name unifier */
	virtual void characterRenamed(uint32 charId, const std::string &newName) =0;

	virtual void sendImpulseToClient(const NLMISC::CEntityId & id, const std::string & msgName ) =0;

	/// force a complete check of all contact list against the eid translator content
	virtual void checkContactLists() =0;
	/// A player entity have been removed from eid translator, check all guild member list
	virtual void playerEntityRemoved(const NLMISC::CEntityId &eid) =0;

//	virtual void addAllCharForStringIdRequest() =0;

//	virtual void addEntityForStringIdRequest(const NLMISC::CEntityId &eid) =0;

};

// Send characters summary to client
void sendCharactersSummary( CPlayer *player, bool AllAutorized = false, uint32 bitfieldOwnerOfActiveAnimSession = 0, uint32 bitfieldOwnerOfEditSession = 0);


#endif // PLAYER_MANAGER_INTERFACE_H
