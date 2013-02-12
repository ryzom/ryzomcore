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

#ifndef RY_PVP_MANAGER_2_H
#define RY_PVP_MANAGER_2_H

#include "player_manager/character_respawn_points.h"
#include "dyn_chat_egs.h"

#include "game_share/pvp_clan.h"
#include "server_share/pvp_relation.h"
#include "pvp_manager/pvp_safe_zone.h"

class CCharacter;
class CEntityBase;
class CGameItemPtr;
class IPVPInterface;

// Comparator for case-insensitive comparison in STL assos. containers
struct ci_less : std::binary_function<std::string, std::string, bool>
{
	// case-independent (ci) compare_less binary function
	struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool> 
	{
		bool operator() (const unsigned char& c1, const unsigned char& c2) const
		{
			return tolower (c1) < tolower (c2); 
		}
	};
	bool operator() (const std::string & s1, const std::string & s2) const
	{
		return std::lexicographical_compare(
			s1.begin (), s1.end (),   // source range
			s2.begin (), s2.end (),    // dest range
			nocase_compare ());        // comparison
	}
};

/**
 * CPVPManager2 singleton.
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2005
 */
class CPVPManager2
{
	NL_INSTANCE_COUNTER_DECL(CPVPManager2);
public:
	typedef std::map< PVP_CLAN::TPVPClan, TChanID > TMAPFactionChannel;
	typedef std::map< std::string, TChanID, ci_less > TMAPExtraFactionChannel; // Names are case-rententive but not case-sensitive
	typedef std::map< TChanID, std::string > TMAPPassChannel;

	///\name LOW LEVEL MANAGEMENT
	//@{
	/// singleton init
	static void init();
	/// singleton release
	static void release();
	/// instance accessor
	static CPVPManager2* getInstance();
	/// create the faction chat channel when IOS mirror ready
	void onIOSMirrorUp();
	/// callback called at each tick
	void tickUpdate();
	/// return dynamic channel TChanID attributed to a faction
	TChanID getFactionDynChannel( const std::string& channelName );
	/// return user dynamic channel TChanID
	TChanID getUserDynChannel( const std::string& channelName);
	/// return user dynamic channel name
	std::string getUserDynChannel(const TChanID& channelId);
	/// return dynamic channel TChanID attribued to a faction
	const std::string &getPassUserChannel( TChanID channelId);
	/// return dynamic channel TChanID character must have, DYN_CHAT_INVALID_CHAN if he must don't have faction channel
	std::vector<TChanID> getCharacterChannels(CCharacter * user);
	/// return dynamic channel TChanId subscribed by character, DYN_CHAT_INVALID_CHAN if character have no faction channel
	std::vector<TChanID> getCharacterRegisteredChannels(CCharacter * user);
	/// return dynamic user channel TChanId subscribed by character, DYN_CHAT_INVALID_CHAN if character have no user channel
	std::vector<TChanID> getCharacterUserChannels(CCharacter * user);
	// brodcast message to channel
	void broadcastMessage(TChanID channel, const ucstring& speakerName ,const ucstring& txt);
	// send list of users to player
	void sendChannelUsers(TChanID channel, CCharacter * user, bool outputToSys = false);
	// add faction channel to character if needed
	void addFactionChannelToCharacter(TChanID channel, CCharacter * user, bool writeRight = true, bool userChannel = false);
	// remove faction channel for character
	void removeFactionChannelForCharacter(TChanID channel, CCharacter * user, bool userChannel = false);
	// add/remove faction channel to this character with privilege
	void addRemoveFactionChannelToUserWithPriviledge(TChanID channel, CCharacter * user, bool s = true );
	/// handle player connection
	void playerConnects(CCharacter * user);
	/// handle player disconnection
	void playerDisconnects(CCharacter * user);
	/// handle to add or remove faction channel to player of needed
	void updateFactionChannel(CCharacter * user, bool b = true );
	/// handle player death
	void playerDies(CCharacter * user);
	/// handle player teleportation
	void playerTeleports(CCharacter * user);
	/// set PVP mode in mirror
	void setPVPModeInMirror(const CCharacter * user) const;
	/// add a PvP safe zone
	void addPVPSafeZone( NLMISC::CSmartPtr<CPVPSafeZone> safeZone );
	/// return true if coordinate are in safe zone
	bool inSafeZone(const NLMISC::CVector & v) const;
	//@}

	///\name PVP MODE FEATURES DEPENDANTS
	//@{
	/// get the pvp mode(bitfield)
	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * actor, CEntityBase * target, bool curative = false ) const;
	/// return true if actor can use a curative action on target
	bool isCurativeActionValid( CCharacter * actor, CEntityBase * target, bool checkMode = false ) const;
	/// return true if actor can use an offensive action on target
	bool isOffensiveActionValid( CCharacter * actor, CEntityBase * target, bool checkMode = false  ) const;
	/// returns true if owner area effect can be applied on areaTarget
	bool canApplyAreaEffect(CCharacter * actor, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;
	/// return true if actor can use gived teleport point
	bool isTPValid( CCharacter * actor, CGameItemPtr TeleportTicket ) const;
	/// return true is respawn point is valid (use the same continent than character)
	bool isRespawnValid( CCharacter * actor, CCharacterRespawnPoints::TRespawnPoint respawnPoint ) const;
	/// final blower killer in pvp faction
	void finalBlowerKillerInPvPFaction( CCharacter * killer, PVP_CLAN::TPVPClan finalBlowerFaction, CCharacter * victimChar ) const;
	/// killer in PvP faction
	void characterKillerInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan winnerFaction, sint32 factionPoint ) const;
	/// killed character in PvP faction
	void characterKilledInPvPFaction( CCharacter * character, PVP_CLAN::TPVPClan looserFaction, sint32 factionPoint ) const;
	//@}

	///\name PVP FACTION SPECIFICS METHODES
	//@{
	// add a war between two factions, return true if added, false if this war already occurs
	bool addFactionWar( PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2 );
	// stop a war between two factions, return true if war stopped, false if they are no war occurs between these two factions
	bool stopFactionWar( PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2 );
	// create a faction channel if not already exist
	void createFactionChannel(PVP_CLAN::TPVPClan clan);
	// create an extra faction channel if not already exist (for marauders, agnos, urasiens and hominits)
	void createExtraFactionChannel(const std::string & channelName, bool universalChannel = false);
	// create an user channel if not already exist
	TChanID createUserChannel(const std::string & channelName, const std::string & pass);
	// remove a user channel
	void deleteUserChannel(const std::string & channelName);
	// remove a fation channel if faction is no more involved in a war
	void removeFactionChannel(PVP_CLAN::TPVPClan clan);
	// return true if faction war occurs between the 2 factions
	bool factionWarOccurs( PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2 ) const;
	// return true if one part on allegiance1 is in war with one part of allegiance2
	bool factionWarOccurs( std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance1, std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance2 ) const;
	// return true if clan is in war with another faction
	bool isFactionInWar( PVP_CLAN::TPVPClan clan );
	// send the faction wars to client
	void sendFactionWarsToClient( CCharacter * user );
	// if a player does an offensive(curative) action on a faction pvp enemy(ally) we must update flag
	void setPVPFactionAllyReminder( bool isAlly ) { _PVPFactionAllyReminder = isAlly; }
	void setPVPFactionEnemyReminder( bool isEnemy ) { _PVPFactionEnemyReminder = isEnemy; }
	//@}

	///\name PVP DUEL SPECIFICS METHODES
	//@{
	/// player ask target in duel
	void askForDuel( const NLMISC::CEntityId & userId );
	/// player accept a duel
	void acceptDuel( const NLMISC::CEntityId & userId );
	/// player refuses a duel
	void refuseDuel( const NLMISC::CEntityId & userId );
	/// player abandons a duel
	void abandonDuel( const NLMISC::CEntityId & userId );
	/// remove a user from the duel invitations
	void removeDuelInvitor( const NLMISC::CEntityId & userId );
	// end a duel
	void endDuel( CCharacter * user, const std::string& userTxt, const std::string& opponentTxt );
	//@}

	///\name PVP OUTPOST SPECIFICS METHODS
	//@{
	// if a player does an offensive(curative) action on an outpost pvp enemy(ally) we must reset timer
	void setPVPOutpostAllyReminder( bool isAlly ) { _PVPOutpostAllyReminder = isAlly; }
	void setPVPOutpostEnemyReminder( bool isEnemy ) { _PVPOutpostEnemyReminder = isEnemy; }
	//@}

private:
	/// ctor
	CPVPManager2(){}
	/// dtor
	~CPVPManager2();

	/// unique instance
	static CPVPManager2 * _Instance;

	typedef std::vector< PVP_CLAN::CFactionWar >	TFactionWars;
	/// current faction wars
	TFactionWars		_FactionWarOccurs;
	/// channel for faction in war
	TMAPFactionChannel	_FactionChannel;
	TMAPExtraFactionChannel	_ExtraFactionChannel;
	TMAPExtraFactionChannel	_UserChannel;
	typedef std::map< TChanID, std::vector<NLMISC::CEntityId> > TChannelsCharacter;
	TChannelsCharacter	_UserChannelCharacters;
	TMAPPassChannel _PassChannels;
	/// character registered channel
	typedef std::map< NLMISC::CEntityId, std::vector<TChanID> > TCharacterChannels;
	TCharacterChannels	_CharacterChannels;
	TCharacterChannels	_CharacterUserChannels;
	/// if a player does an offensive(curative) action on a faction pvp enemy(ally) we must update flag
	bool _PVPFactionAllyReminder;
	bool _PVPFactionEnemyReminder;

	/// safe zones
	std::vector<NLMISC::CSmartPtr<CPVPSafeZone> > _SafeZones;

	/// Interfaces for specialized PvP
	std::vector<IPVPInterface*> _PVPInterface;

	/// duel propositions ( pair invitor-invited )
	struct CDuelAsked
	{
		TDataSetRow			Invitor;
		TDataSetRow			Invited;
		NLMISC::TGameCycle	ExpirationDate;
	};

	/// list of duel propositions, sorted by ascending end date
	std::list< CDuelAsked >	_DuelsAsked;

	/// if a player does an offensive(curative) action on an outpost pvp enemy(ally) we must reset timer
	bool _PVPOutpostAllyReminder;
	bool _PVPOutpostEnemyReminder;
};

#endif // RY_PVP_MANAGER_2_H

