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



#ifndef RY_PVP_ZONE_H
#define RY_PVP_ZONE_H

#include "nel/ligo/primitive.h"
#include "game_share/base_types.h"
#include "game_share/pvp_clan.h"
#include "server_share/pvp_relation.h"

#include "mission_manager/ai_alias_translator.h"

#include "pvp_base.h"

namespace PVP_ZONE_TYPE
{

	enum TPVPZoneType
	{
		FreeZone,
		VersusZone,
		GuildZone,
		OutpostZone,

		Unknown,
		NbPVPZoneTypes = Unknown
	};

	TPVPZoneType fromString(const std::string & str);
	const std::string & toString(TPVPZoneType type);

} // namespace PVP_ZONE_TYPE

namespace NLMISC
{

	class CLog;

} // namespace NLMISC

class CEntityBase;
class CCharacter;
class CPVPSafeZone;

/**
 * Base class of PVP zones
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2004
 */
class IPVPZone : private NLLIGO::CPrimZone, public IPVP
{
public:
	/// build a PVP zone from a primitive
	static NLMISC::CSmartPtr<IPVPZone> build(const NLLIGO::CPrimZone * zone);

	/// fill an outpost PVP zone from a primitive
	static NLMISC::CSmartPtr<IPVPZone> buildOutpostZone(const NLLIGO::CPrimZone * zone, NLMISC::CSmartPtr<IPVPZone> pvpZone);

	/// returns true if PVP zones overlap, dirty test not working if a zone does not contain a vertex of the other one!
	static bool overlap(NLMISC::CSmartPtr<IPVPZone> pvpZone1, NLMISC::CSmartPtr<IPVPZone> pvpZone2);

public:
	/// dtor
	virtual ~IPVPZone();

	/// is the zone active?
	bool isActive() const { return _Active; }

	/// set the zone active or not
	virtual void setActive(bool active);

	/// get name
	const std::string & getName() const { return _Name; }

	/// get persistent alias
	TAIAlias getAlias() const { return _Alias; }

	/// get center coords
	sint32 getCenterX() const { return _CenterX; }
	sint32 getCenterY() const { return _CenterY; }

	/// get zone type
	PVP_ZONE_TYPE::TPVPZoneType getPVPZoneType() const { return _PVPZoneType; }

	/** 
	 * Return true if a character killed by 'killer' must use deathPenaltyFactor().
	 * Precondition: kill not null.
	 * By default, it's true only when killed by a player character.
	 */
	virtual bool hasDeathPenaltyFactorForVictimsOf( CEntityBase *killer ) const;

	// return the death penalty factor a character gets when killed by another character in this zone
	float deathPenaltyFactor() const { return _DeathPenaltyFactor; }

	/// returns true if the PVP zone contains the given position
	bool contains(const NLMISC::CVector & v, bool excludeSafeZones = true) const;
	bool contains(CCharacter* user, bool excludeSafeZones = true) const;

	/**
	 * add a safe zone if the safe zone is inside the PVP zone and if it was not already added
	 * \return true if the safe zone has been added
	 */
	bool addSafeZone(NLMISC::CSmartPtr<CPVPSafeZone> safeZone);

	/// add a new player in the zone, call this each time a player enters the zone
	virtual void addPlayer(CCharacter * user) = 0;

	/// dump the zone
	virtual void dumpZone(NLMISC::CLog * log, bool dumpUsers = true) const;

protected:
	/// ctor
	IPVPZone();

	/// is the zone active?
	bool		_Active;

	/// users inside the zone
	std::set<TDataSetRow> _Users;

	/// when a character is killed by another character, how much does he death penalty in this zone? (0..1)
	float		_DeathPenaltyFactor;

private:
	/// name of the zone
	std::string	_Name;

	/// persistent alias
	TAIAlias	_Alias;

	/// center coords of the zone
	sint32		_CenterX;
	sint32		_CenterY;

	/// PVP zone type
	PVP_ZONE_TYPE::TPVPZoneType	_PVPZoneType;

	/// safe zones
	std::vector<NLMISC::CSmartPtr<CPVPSafeZone> > _SafeZones;
};

/**
 * A PVP free zone : everybody attacks everybody
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class CPVPFreeZone : public IPVPZone
{
public:
	PVP_MODE::TPVPMode getPVPMode() const { return PVP_MODE::PvpZoneFree; }
	void addPlayer(CCharacter * user);

	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * user, CEntityBase * target ) const;

private:
	bool leavePVP(CCharacter * user, IPVP::TEndType type);

	/*
	/// Return true for players in the pvp zone, false for anyone else (including non-players) ('attackable' will be used instead)
	bool canUserHurtTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players in the pvp zone, false for anyone else (including non-players)
	bool canUserHelpTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players according the rules, and for non-players if offensive
	bool canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;
	*/
};

/**
 * A PVP versus zone : 2 clans fight there
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2004
 */
class CPVPVersusZone : public IPVPZone
{
public:
	/// ctor
	CPVPVersusZone(PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2, bool giveFP);

	PVP_MODE::TPVPMode getPVPMode() const { return PVP_MODE::PvpZoneFaction; }
	void setActive(bool active);
	void addPlayer(CCharacter * user);

	void giveFactionPoints(bool giveFP);
	bool giveFactionPoints() const { return _GiveFactionPoints; }

	PVP_CLAN::TPVPClan getClan( uint32 clanNumber ) const { return clanNumber == 1 ? _Clan1 : _Clan2; }

	// return pvp clan of a character
	PVP_CLAN::TPVPClan getCharacterClan( const NLMISC::CEntityId& character ) const;

	void dumpZone(NLMISC::CLog * log, bool dumpUsers = true) const;

	static PVP_CLAN::TPVPClan getPlayerClan(CCharacter * user, PVP_CLAN::TPVPClan clan1, PVP_CLAN::TPVPClan clan2 );

	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * user, CEntityBase * target ) const;
	
private:

	bool isOverridedByARunningEvent( CCharacter * user );

	bool setPlayerClan(CCharacter * user/*, PVP_CLAN::TPVPClan clan*/);
	
	static PVP_CLAN::TPVPClan determinatePlayerClan( CCharacter *user, PVP_CLAN::TPVPClan clan1, sint32 fame1, PVP_CLAN::TPVPClan clan2, sint32 fame2 );

	void setPlayerClanInMirror(CCharacter * user, PVP_CLAN::TPVPClan clan) const;

	bool leavePVP(CCharacter * user, IPVP::TEndType type);

	void userHurtsTarget(CCharacter * user, CCharacter * target);

	/*
	/// Return false for players according to the rules, and for non-players ('attackable' will be used instead)
	bool canUserHurtTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players in the same clan or neutral, false for anyone else (including non-players)
	bool canUserHelpTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players according the rules, and for non-players if offensive
	bool canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;
	*/

	const PVP_CLAN::TPVPClan _Clan1;
	const PVP_CLAN::TPVPClan _Clan2;

	/// neutral users aggressors
	typedef std::set<TDataSetRow> TAggressors;
	std::map<TDataSetRow,TAggressors> _AggressedNeutralUsers;

	/// users clan
	std::map<NLMISC::CEntityId,PVP_CLAN::TPVPClan> _UsersClan;

	bool _GiveFactionPoints;
};

/**
 * A PVP guild zone : guilds fight there
 * \author Sebastien Guignot
 * \author Nevrax France
 * \date 2004
 */
class CPVPGuildZone : public IPVPZone
{
public:
	PVP_MODE::TPVPMode getPVPMode() const { return PVP_MODE::PvpZoneGuild; }
	void addPlayer(CCharacter * user);

	/// return pvp relation between the two players
	PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * user, CEntityBase * target ) const;
	
private:
	bool leavePVP(CCharacter * user, IPVP::TEndType type);

	/*
	/// Return true for players in the pvp zone (everyone for training purpose), false for anyone else (including non-players) ('attackable' will be used instead)
	bool canUserHurtTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players in the same team or guild, false for anyone else (including non-players)
	bool canUserHelpTarget(CCharacter * user, CEntityBase * target) const;

	/// Return true for players according the rules, and for non-players if offensive
	bool canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const;
	*/
};


/**
 * A PVP outpost zone : guilds fight for the control of an outpost. Defenders are helped by npc squads.
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2005
 */
class CPVPOutpostZone : public IPVPZone
{
public:
	
	/** 
	 * Return true if a character killed by 'killer' must use deathPenaltyFactor().
	 * Precondition: kill not null.
	 * For outpost zones, it's true whether the killer is a player character or a bot.
	 */
	virtual bool hasDeathPenaltyFactorForVictimsOf( CEntityBase *killer ) const;
};

#endif // RY_PVP_ZONE

