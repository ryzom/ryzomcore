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



#ifndef PLAYER_H
#define PLAYER_H

// net
#include "nel/net/login_cookie.h"

// game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/character_summary.h"
#include "game_share/gender.h"
#include "game_share/backup_service_interface.h"
#include "server_share/entity_state.h"

//
// different level of priviliges for players
//

static const std::string PriviliegeGameMaster = "GM";

class CCharacter;
class CPlayer;

/**
 * AsyncLoad player class
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2004
 */
class CAsyncPlayerLoad : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CAsyncPlayerLoad);
public:

	CAsyncPlayerLoad();

	CPlayer*				Player;
	uint32					UserId;
	std::string				LanguageId;
	bool					AllAuthorized;
	NLNET::CLoginCookie		LoginCookie;

	struct CLoadChar
	{
		CLoadChar() : Ready(false)	{ }
		bool						Ready;
		std::vector<std::string>	Files;
	};

	std::vector<CLoadChar>	Chars;

	void			clear()
	{
		Player = NULL;
		UserId = 0;
		LanguageId.clear();
		AllAuthorized = false;
		Chars.clear();
	}

	/// Clear up struct for chars to be received
	void			initChars();

	/// Ask BS to transfer character data
	void			startLoading();


	/// Received characters file list
	void			receivedCharacterFileList(const CFileDescriptionContainer& fileList, uint charId);

	/// Received character file
	void			receivedCharacterFile(const CFileDescription& fileDescription, NLMISC::IStream& dataStream, uint charId);


	/// Start Char loading
	void			startCharLoading(uint charId);

	/// Set Char ready
	bool			setCharReady(uint charId)
	{
		if (Chars.size() > charId)
			Chars[charId].Ready = true;
		return allCharsReady();
	}

	/// Check All characters ready
	bool			allCharsReady();
	
private:

	class CFileClassCallback : public IBackupFileClassReceiveCallback
	{
	public:
		CFileClassCallback(CAsyncPlayerLoad* load, uint charId) : Load(load), CharId(charId)	{}
		uint								CharId;
		NLMISC::CSmartPtr<CAsyncPlayerLoad>	Load;
		void				callback(const CFileDescriptionContainer& fileList)
		{
			if (Load)
			{
				Load->receivedCharacterFileList(fileList, CharId);
				Load = NULL;
			}
		}
	};
/*
	class CFileListCallback : public IBackupFileListReceiveCallback
	{
	public:
		CFileListCallback(CAsyncPlayerLoad* load) : Load(load)	{}
		NLMISC::CSmartPtr<CAsyncPlayerLoad>	Load;
		void				callback(const CFileDescriptionContainer& fileList)
		{
			if (Load)
			{
				Load->receivedCharactersFileList(fileList);
				Load = NULL;
			}
		}
	};
*/
	class CFileCallback : public IBackupFileReceiveCallback
	{
	public:
		CFileCallback(CAsyncPlayerLoad* load, uint charId) : Load(load), CharId(charId)	{}
		uint								CharId;
		NLMISC::CSmartPtr<CAsyncPlayerLoad>	Load;
		void				callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
		{
			if (Load)
			{
				Load->receivedCharacterFile(fileDescription, dataStream, CharId);
				Load = NULL;
			}
		}
	};

};


/**
 * Player class
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CPlayer
{
	NL_INSTANCE_COUNTER_DECL(CPlayer);
public:
	/**
	 * Default constructor
	 */
	CPlayer();

	/**
	 * \return the current version of the class. Useful for managing old versions of saved players
	 * WARNING : the version number should be incremented when the serial method is modified
	 */
	/// version 2 : for game master titles
	static inline uint16 getCurrentVersion(){ return 3; }
	
	/**
	 * Get the user unique id
	 */
	uint32 getUserId() const { return _UserId; }
	
	// CPlayer : createCharacter
	NLMISC::CEntityId createCharacter( const std::string& characterName, EGSPD::CPeople::TPeople people, GSGENDER::EGender gender );

	// get first free character slot index
	sint32 getFirstFreeCharacterIndex();

	/// \return character index corresponding to character number
	sint32 getCharacterIndex( uint32 number );

	/**
	 * provides access to a character
	 * \return pointer on character
	 */
	// By index
	CCharacter * getCharacter( uint32 c );

	// By CEntityId
	CCharacter * getCharacter( const NLMISC::CEntityId& id );

	/**
	 * provides acces to the active character
	 *
	 * \return reference on the active character 
	 */
	CCharacter * getActiveCharacter()
	{
		if( _ActiveCharIndex >= (sint32)_Characters.size() )
		{
			nlwarning("<BUG: CCharacter::getActiveCharacter> %d, %d", _ActiveCharIndex, _Characters.size() );
			return 0;
		}
		if( _ActiveCharIndex >= 0 )
		{
			return _Characters[_ActiveCharIndex];
		}
		else
		{
			return 0;
		}
	}

	// Return reference on character vector
	const std::vector<CCharacter *>& getCharacterReference() { return _Characters; }

	/**
	 * get the character count
	 *
	 * \return number of character
	 */
	uint8 getCharacterCount() const;

	/**
	 * Add a character
	 *
	 * \param ch is the new character
	 */
	void addCharacter( CCharacter * ch, uint32 index )
	{
		nlassert( ch );
		nlassert( index < _Characters.size() );
		nlassert( _Characters[ index ] == 0 );
		_Characters[ index ] = ch;
	}

	/**
	 * Delete a character
	 *
	 * \Index is index of character
	 */
	void deleteCharacter( uint32 index );

	/**
	 * Get the characters infos used at char selection menu
	 *
	 * \return reference on Id
	 */
	void getCharactersSummary( std::vector<CCharacterSummary>& chars );

	
	/**
	 * provides acces user Id
	 *
	 * \return reference on Id
	 */
	uint32 getId() const
	{
		return _UserId;
	}

	/**
	 * Set the unique id of this player
	 *
	 * \param id the player's unique id
	 */
	void setId( uint32 id )
	{
		_UserId = id;
	}
	
	/**
	 *	set player connection status
	 * \param status is true if player connected, false if disconnected
	 */
	inline void setPlayerConnection( bool status ) { _ConnexionStatus = status; }

	/**
	 *	Get the player connection status
	 * \return the status connxion of player
	 */
	inline bool getPlayerConnection() { return _ConnexionStatus; }

	/**
	 *	Return the current state of the character
	 * \param charIndex is the index of the player char
	 * \return state of this char
	 */
	const CEntityState& getState( sint32 charIndex = -1 );

	/**
	 *	Return the type of the character
	 * \param charIndex is the index of the player char
	 * \return type of this char (sheetId)
	 */
	NLMISC::CSheetId getType( sint32 charIndex = -1 );

	/**
	 * load all characters of the player
	 */
	void loadAllCharacters();

	/**
	 * load all characters of the player using the new PDR file format
	 */
	void loadAllCharactersPdr();

	/**
	 * Save a player active Char
	 */
	void saveCharacter(class NLMISC::IStream &f, sint32 index);

	/**
	 * Save a player active Char
	 */
	void storeCharacter(CPersistentDataRecord &pdr, sint32 index);

	/**
	 * Write crash marker file so if EGS crash on a saved char, it can wipe it next time
	 */
	static void	writeCrashMarker( uint32 userId, uint32 charId);

	/**
	 * Check EGS didn't crashed on last load (and wipe buggy file if so)
	 */
	static void	checkCrashMarker();

	/**
	 * get the active character index
	 * \return sint16 the active character index, -1 if none
	 */
	sint32 getActiveCharIndex() const { return _ActiveCharIndex; }

	/**
	 * set the active character index
	 * \param uint32 index the active character index
	 * \return bool true if the index was valid, false otherwise
	 */
	bool setActiveCharIndex( uint32 index, NLMISC::CEntityId charId );

	/**
	 *	Set the value of a var
	 * \param var is the name of the variable
	 * \param value is the new value for the variable
	 */
	void setValue( std::string var, std::string value );
	
	/**
	 *	Return the value of the variable
	 * \param var is the name of the variable
	 * \param value is the current value of the variable
	 */
	void getValue( std::string var, std::string& value );

	/**
     * Set disconnection time
     */
	void setDisconnectionTime() { _DisconnectionTime = CTickEventHandler::getGameCycle(); }

	/**
     * Get disconnection time
	 */
	NLMISC::TGameCycle getDisconnectionTime() { return _DisconnectionTime; }

	/**
	 * RemoveAllCharacters
	 */
	void removeAllCharacters();

	void setUserName( const std::string& name ) { _UserName = name; }
	const std::string& getUserName() { return _UserName; }

	void setUserPriv( const std::string& priv ) { _UserPriv = priv; }
	const std::string& getUserPriv() const { return _UserPriv; }

	// return true if the user have the privilege
	bool havePriv( const std::string &priv ) const;

	// return true is the user have any privilege
	bool haveAnyPriv() const { return !_UserPriv.empty(); }

	/// Get login cookie of the player
	const NLNET::CLoginCookie	&getLoginCookie() const
	{
		return _LoginCookie;
	}

	/// Set login cookie
	void	setLoginCookie(const NLNET::CLoginCookie &cookie)
	{
		_LoginCookie = cookie;
	}

	/// Send the characters info to the Shard unifier to resync the ring database
	void updateCharactersInRingDB();

	/**
	 * Destructor
	 */
	~CPlayer();

	/**
	 * Set Verbose mode for player
	 * \param verbose is true for verbose, false for quiet
	 */
	inline void setVerboseMode( bool verbose ) { _Verbose = verbose; }

	void setUserLanguage(const std::string &langId) { _LanguageId = langId; }
	const std::string &getUserLanguage()	{ return _LanguageId; }

	void isBetaTester(bool betaTester);
	inline bool isBetaTester() const { return _BetaTester; }

	void isPreOrder(bool preOrder) { _PreOrder = preOrder; }
	inline bool isPreOrder() const { return _PreOrder; }

	void isWindermeerCommunity(bool windermeerCommunity);
	inline bool isWindermeerCommunity() const { return _WindermeerCommunity; }

	void isTrialPlayer(bool trialPlayer) { _TrialPlayer = trialPlayer; }
	inline bool isTrialPlayer() const { return _TrialPlayer; }

protected:
	friend class CAsyncPlayerLoad;
	friend class CPlayerManager;
	inline void clearActivePlayerPointer() 
	{ 
		if (_ActiveCharIndex >= 0 && (uint32)_ActiveCharIndex < _Characters.size() )
		{
			_Characters[_ActiveCharIndex] = 0;
		}
	}

private:

	/// load the player ( old player format ( version < 3 : all characters in the same player )
	void loadOldFormat(class NLMISC::IStream &f);

	// user id & name
	uint32						_UserId;	// Id of the acount
	std::string					_UserName;	// Name of the account (not the name of the player)
	std::string					_UserPriv;	// Privilege of the account
	std::string					_LanguageId;	// Language used by the player (default to 'en').

	/// beta tester?
	bool						_BetaTester;

	/// pre order?
	bool						_PreOrder;

	// old windermeer community player ?
	bool						_WindermeerCommunity;

	// Trial player ?
	bool						_TrialPlayer;

	// id that will be used for the next created player
	static uint32				_PlayerIdCount;

	/// the active character index (curently used by this player, -1 = none)
	sint32						_ActiveCharIndex;

	/// characters of the player
	std::vector<CCharacter *>	_Characters;

	/// Verbose commuted if true, for debug
	bool						_Verbose;

	/// connexion status
	bool						_ConnexionStatus;

	/// login cookie copy
	NLNET::CLoginCookie			_LoginCookie;

	/// disconnection time
	NLMISC::TGameCycle			_DisconnectionTime;
};

#endif // PLAYER_H

/* End of player.h */
