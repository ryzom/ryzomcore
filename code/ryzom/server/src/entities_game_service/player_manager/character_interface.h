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

#include "nel/misc/entity_id.h"
#include "game_share/r2_types.h"

class CCharacter;
class CEntityState;
class COfflineEntityState;
class CCharacterRespawnPoints;
class CFarPosition;
class CModuleParent;

namespace NLNET
{
	class CMessage;
}

namespace R2
{
	struct TUserRole; 
}

class CRingRewardPoints;

/** Interface class to insulate module code from EGS inferno */
class ICharacter
{
public:

//	typedef uint32 TSessionId;

	static ICharacter *getInterface(::CCharacter *character, bool onlyOnline);
	static ICharacter *getInterface(uint32 charId, bool onlyOnline);
	static ICharacter *getInterface(NLMISC::CEntityId charEId, bool onlyOnline);
	static ICharacter *getInterfaceFromUser(uint32 userId, bool onlyOnline);

	CCharacter		*getCharacter();
	CModuleParent	&getModuleParentWrap();
	CEntityState	&getStateWrap();
	void			setStateWrap( const COfflineEntityState& es );

	virtual const NLMISC::CEntityId& getCharId() const =0;

	virtual void setName(const ucstring &name) =0;
	/// Register the character name in the IOS
	virtual void registerName(const ucstring &newName = std::string("")) =0;

	virtual bool getEnterFlag() const =0;

	virtual uint32 getLastDisconnectionDate() =0;

	virtual void setSessionId( TSessionId sessionId ) = 0;
	// like setSessionId but works even in edition session
	virtual void setCurrentSessionId( TSessionId sessionId ) = 0;


	virtual void setSessionUserRole( R2::TUserRole userRole ) =0;
	virtual CCharacterRespawnPoints &getRespawnPoints() =0;

	virtual void sendMessageToClient( uint32 userId, NLNET::CMessage& msgout ) =0;

	// Send USER_CHAR (by IMPULSION_ID: Precondition: CL_ID must have been sent to the FS)
	virtual void sendUserChar( uint32 userId, uint8 scenarioSeason, const R2::TUserRole& userRole ) =0;

	// Return the home mainland session id for a character
	virtual TSessionId getHomeMainlandSessionId() const =0;

	// Set the home mainland session id
	virtual void setHomeMainlandSessionId(TSessionId homeSessionId) = 0;

	// check if the character is a newbie by checking the base of the position stack
	virtual bool isNewbie() const =0;

	// Assign current position with the top of the position stack.
	// Preconditions: PositionStack not empty, EnterFlag must be false.
	virtual void applyTopOfPositionStack() =0;

	// Push the current position to the stack
	virtual void pushCurrentPosition() =0;

	// Assign the current position with a new position and push it to the position stack (so that the previous pos is not lost)
	// Preconditions: EnterFlag must be false.
	virtual void applyAndPushNewPosition( const CFarPosition& farPos ) =0;

	// Assign the current position with a new position, but don't push it to the position stack (so that it won't be stored).
	// Lock the stack to prevent from overwriting the stored position.
	// Preconditions: EnterFlag must be false.
	// Warning: After calling this method, sessionId() becames unavailable.
	virtual void applyEditorPosition( const CFarPosition& farPos ) =0;

	// Assign the current position with the second position in the stack (the top position is lost).
	// Precondition: PositionStack.size()>1, EnterFlag must be false.
	virtual void popAndApplyPosition() =0;

	// remove the previous Session from the Position Stack if the previous session has the correct sessionId
	virtual void leavePreviousSession (TSessionId previousSessionId) = 0;

	/// Send a Far TP request to the client (by IMPULSION_ID: Precondition: CL_ID must have been sent to the FS)
	virtual void requestFarTP( TSessionId destSessionId, bool allowRetToMainlandIfFails=true, bool sendViaUid=false ) =0;

	// Return to previous session (ex: adventure to mainland Ryzom) (another server) from a ring session (this server) if applicable
	// If userId if provided, charIndex must be provided and they will be used (we assume the character has not been selected yet by the player)
	// If userId is ~0, the active character of this user will be used (we assume the character has been selected)
	// If rejectedSessionId is non zero, the top position will be popped in any cases if it matches rejectedSessionId
	virtual void returnToPreviousSession( uint32 userId=~0, sint32 charIndex=~0, TSessionId rejectedSessionId=0 ) =0;

	virtual const NLMISC::CEntityId& getId() const =0;

	virtual bool isDead() const =0;

	virtual void teleportCharacter( sint32 x, sint32 y) =0;

	virtual void respawn( uint16 index ) =0;

	virtual void sendContactListInit()=0;

	virtual void setContactOnlineStatus(const NLMISC::CEntityId &charEid, bool connection) =0;

	virtual void setLastConnectionDate(uint32 date) =0;


	virtual void syncContactListWithCharNameChanges(const std::vector<NLMISC::CEntityId> &charNameChanges)=0;

	virtual void updateTargetingChars()=0;

	// return a mutable reference on the ring point reward object attached to the character
	virtual CRingRewardPoints &getRingRewardPoints() =0;

	// Store the current active animation session returned by SU after char synchronisation.
	virtual void setActiveAnimSessionId(TSessionId activeAnimSessionId) =0;
	// read the current active animation session returned by SU after char synchronisation.
	virtual TSessionId getActiveAnimSessionId() =0;
};
