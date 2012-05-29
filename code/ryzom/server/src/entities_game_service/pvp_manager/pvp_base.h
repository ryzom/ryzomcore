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



#ifndef RY_PVP_BASE_H
#define RY_PVP_BASE_H

#include "game_share/pvp_mode.h"
#include "server_share/pvp_relation.h"

//#ifdef NL_DEBUG
// uncomment this if you want PVP stuff to be verbose
# define PVP_DEBUG
//#endif

class CCharacter;
class CEntityBase;

/**
 * abstract base class for PVP sessions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2004
 */
class IPVP : public NLMISC::CRefCount
{
	friend class CPVPInterface;

	NL_INSTANCE_COUNTER_DECL(IPVP);
public:

	/// reasons that can potentially stop PVP session
	enum TEndType
	{
		Disconnect,
		Death,
		Teleport,
		EnterPVPZone,
		LeavePVPZone,
		AbandonDuel,
		AbandonChallenge,
		QuitTeam,
	};

	virtual ~IPVP() {}

	/// get PVP mode to send to the client
	virtual PVP_MODE::TPVPMode getPVPMode() const = 0;

	/**
	 * Return true if the character takes part in the PVP conflict.
	 * By defaut, as long as he is in the PVP zone, he is taking part in the conflict.
	 * This method can be overrided to force a character being neutral.
	 * In this case (returning false), the player won't receive PVP enter/leave zone
	 * messages, although his PVPInterface will be set.
	 */
	virtual bool isCharacterInConflict(CCharacter *user) const { return true; }

	/// return pvp relation between the two players
	virtual PVP_RELATION::TPVPRelation getPVPRelation( CCharacter * actor, CEntityBase * target ) const = 0;

private:
	///\name interface callbacks
	//@{

	/// callback when user potentially leaves PVP. Return false if an error occured
	virtual bool leavePVP( CCharacter * user, IPVP::TEndType type) = 0;

	/// callback when a player hurts another, does nothing by default
	virtual void userHurtsTarget(CCharacter * user, CCharacter * target) {}


	//@}

	///\name test on interface member state
	//@{

	/**
	 * Returns true if user can hurt target in the PVP rules.
	 * If target is not a player character, this method can return true to enable
	 * hurting non-attackable creatures. If it returns false, the 'attackable'
	 * property will be used (default non-pvp rules).
	 * Preconditions: we are in the user PVP session and user != target
	 */
	//virtual bool canUserHurtTarget(CCharacter * user, CEntityBase * target) const = 0;

	/**
	 * Returns true if user can help target in the PVP rules.
	 * Preconditions: we are in the user PVP session and user != target
	 */
	//virtual bool canUserHelpTarget(CCharacter * user, CEntityBase * target) const = 0;

	/**
	 * Returns true if caster area effect can be applied on areaTarget in the PVP rules.
	 * The result will be used even if target is not a player character.
	 * Preconditions: we are in the caster PVP session
	 * Warning: caster can be the same character than areaTarget
	 */
	//virtual bool canApplyAreaEffect(CCharacter * caster, CEntityBase * areaTarget, bool offensive, bool ignoreMainTarget) const = 0;

	//@}

	/// return true if PVP session respawns players itself
	virtual bool doCancelRespawn() const { return false; }
};

#endif // RY_PVP_BASE_H

