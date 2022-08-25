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



#ifndef EGS_PHRASE_UTILITIES_FUNCTIONS_H
#define EGS_PHRASE_UTILITIES_FUNCTIONS_H

// game_share
#include "game_share/mode_and_behaviour.h"
#include "game_share/shield_types.h"
#include "game_share/slot_equipment.h"
#include "game_share/string_manager_sender.h"
#include "game_share/body.h"
#include "game_share/action_nature.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/scores.h"
#include "game_share/item_special_effect.h"
//
#include "egs_mirror.h"

// Stuff used for management of log messages
extern bool VerboseBrickManagerInfo;
#define INFOLOG if (!VerboseBrickManagerInfo) {} else nlinfo

extern bool VerboseBrickManagerDbg;
#define DEBUGLOG if (!VerboseBrickManagerDbg) {} else nldebug

extern bool AggroLog;
#define AGGROLOG if (!AggroLog) {} else nlinfo

extern bool AiActionLog;
#define AIACTIONLOG if (!AiActionLog) {} else nlinfo

//forward declaration
class CEntityBase;


namespace PHRASE_UTILITIES
{

enum ERange
{	
	UNDEFINED = 0,
	POINT_BLANK, // bout-portant
	SHORT_RANGE,
	MEDIUM_RANGE,
	LONG_RANGE,
	OUT_OF_RANGE,
};

enum ECombatFailDisplay
{	
	FailMelee= 0,		// the attacker failed a melee attack
	FailRange,			// the attacker failed a range attack
	FailNoFlyingText	// the attacker failed an attack and we don't want to display any flying text
};


/**
 * convert a ERange value to the corresponding string
 */
std::string toString(ERange range);

typedef std::pair<SLOT_EQUIPMENT::TSlotEquipment,bool>	TPairSlotShield;
typedef std::pair<uint8, uint16>						TPairUInt8UInt16;

/**
 * struct for damage localisation
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
struct SLocalisation
{
	uint8							LocalisationNumber;
	
	SHIELDTYPE::EShieldType			ShieldType;
	SLOT_EQUIPMENT::TSlotEquipment	Slot;
	bool							ShieldIsEffective;

	
	SLocalisation()
	{
		Slot				= SLOT_EQUIPMENT::UNDEFINED;
		ShieldType			= SHIELDTYPE::NONE;
		ShieldIsEffective	= false;
		LocalisationNumber	= 0;
	}

	/// egality operator
	inline bool operator == ( const SLocalisation& p ) const
	{
		return ( ( ShieldType == p.ShieldType ) && ( LocalisationNumber == p.LocalisationNumber ) );
	}

	/// < operator
	inline bool operator <( const SLocalisation& p ) const
	{
		if ( LocalisationNumber < p.LocalisationNumber )
			return true;
		else if ( (LocalisationNumber == p.LocalisationNumber ) && ShieldType < p.ShieldType )
			return true;
		else 
			return false;
	}
};


/**
 * get the damage localisation
 * \param shield the shield used by the target
 * \param forcedSlot the hit slot MUST be of this type, but the effectiveness of the shield is still to be determined
 * \param adjustement the adjustement (for size for exemple)
 * \return a pair indicating the slot hit and the effectiveness of the shield (true = shield is effective)
 */
TPairSlotShield getLocalisation( SHIELDTYPE::EShieldType shield = SHIELDTYPE::NONE, sint8 adjustement = 0, SLOT_EQUIPMENT::TSlotEquipment forcedSlot = SLOT_EQUIPMENT::UNDEFINED);

/**
 * get the localisation size adjustment 
 * \param attacker Id of the attacker
 * \param defender Id of the defender
 * \return the adjustement value
 */
sint8 getLocalisationSizeAdjustement(  const NLMISC::CEntityId &attacker,  const NLMISC::CEntityId &defender );

inline sint8 getLocalisationSizeAdjustement(  const TDataSetRow &attackerRowId,  const TDataSetRow &defenderRowId )
{
	if (TheDataset.isAccessible(attackerRowId) && TheDataset.isAccessible(defenderRowId))
		return getLocalisationSizeAdjustement( TheDataset.getEntityId(attackerRowId), TheDataset.getEntityId(defenderRowId) );
	else
		return 0;
}


/**
 * get the defense modifier on hit slot
 * \param hitSlot the hit slot
 * \param defended slot the defended slot
 * \return the modifier on defender skills (sint32)
 */
sint32 getDefenseLocalisationModifier( SLOT_EQUIPMENT::TSlotEquipment hitSlot, SLOT_EQUIPMENT::TSlotEquipment protectedSlot );

/**
 * get the defense modifier on every slot
 * \param indexProtection the index of the defended slot (255 = none)
 * \return the modifiers on defender skills as a vector of sint8 (of size 6)
 */
const std::vector<sint8> &getDefenseLocalisationModifiers( uint8 indexProtection );

/**
 * load the localisation table
 * \param name and path of the file to load
 */
void loadLocalisationTable( const std::string &tableName );


/**
 * load the localisation size adjusment table
 * \param name and path of the file to load
 */
void loadLocalisationSizeAdjusmentTable( const std::string &tableName );

/**
 * get the distance between two entities
 * \param entity1 the first entity
 * \param entity2 the second entity
 * \return the distance in meters, -1 if invalid entities or properties not found
 */
double getDistance( const NLMISC::CEntityId &entity1, const NLMISC::CEntityId &entity2 );
inline double getDistance( const TDataSetRow &entity1RowId, const TDataSetRow &entity2RowId )
{
	if ( TheDataset.isAccessible(entity1RowId) && TheDataset.isAccessible(entity2RowId) )
	{
		return getDistance( TheDataset.getEntityId(entity1RowId), TheDataset.getEntityId(entity2RowId) );
	}
	else
		return 0.0;
}

/**
 * test the range between two entities, take the collision size of the target into account.
 * \param source the 'source' entity (caster if magic)
 * \param source the 'target' entity
 * \param maxRange the range to check (in mm !)
 * \return true if target is within source max range, false if otherwise
 */
bool testRange( CEntityBase &source, CEntityBase &target, uint32 maxRange );

/**
 * send Death messages when an entity kills another one
 * \param killerId NLMISC::CEntityId of the acting entity (the one which kills the other one), or CEntityId::Unknown
 * \param deadId NLMISC::CEntityId of the dead entity
 */
void sendDeathMessages( const NLMISC::CEntityId &killerId, const NLMISC::CEntityId &deadId );
inline void sendDeathMessages( const TDataSetRow &killerRowId, const TDataSetRow &deadRowId )
{
	if (TheDataset.isAccessible(deadRowId))
	{
		if (TheDataset.isAccessible(killerRowId))
			sendDeathMessages( TheDataset.getEntityId( killerRowId ), TheDataset.getEntityId( deadRowId ) );
		else
			sendDeathMessages( NLMISC::CEntityId::Unknown, TheDataset.getEntityId( deadRowId ) );
	}
}


/**
 * an entity hits another entity, send all relevant messages to the entities around
 * \param aggressorId NLMISC::CEntityId of the acting entity (the one which deals damage)
 * \param victimId NLMISC::CEntityId of the victim entity
 * \param self flag indicating of aggressor self hitting
 * \param sentence pointer on the calling sentence if any
 * \param amount the amount of damage dealt to the victim entity
 * \param lostStamina the amount of stamina lost by the victim entity
 * \param lostSap the amount of sap lost by the victim entity
 */
void sendHitMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, bool self, sint32 amount, sint32 maxDamage, sint32 lostStamina = 0, sint32 lostSap = 0, BODY::TBodyPart bodyPart= BODY::UnknownBodyPart);

inline void sendHitMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, bool self, sint32 amount, sint32 maxDamage, sint32 lostStamina = 0, sint32 lostSap = 0, BODY::TBodyPart bodyPart= BODY::UnknownBodyPart)
{
	if (TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId))
	{
		sendHitMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ), self, amount, maxDamage, lostStamina, lostSap, bodyPart );
	}
}

/**
 * an entity is wounded by a damage shield
 * \param woundedEntity the entity wounded by the damage shield
 * \param defender the entity dealing the damage via it's DS
 * \param damage the amount of damage dealt to the wounded entity
 * \param hpDrain the amount of HP  gained by the defender
 */
void sendDamageShieldDamageMessages(const NLMISC::CEntityId &woundedEntity, const NLMISC::CEntityId &defender, uint16 damage, uint16 hpDrain);

/**
 * a vampirism proc was triggered, damage done are converted to hp transfer
 * \param actingEntity the entity inflicting the damages and receiving the hp
 * \param defender the entity receiving the damage and losing hp
 * \param hpDrain the amount of HP transfered
 */
void sendVampirismProcMessages(const NLMISC::CEntityId &actingEntity, const NLMISC::CEntityId &defender, sint32 hpDrain);

void sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::TItemSpecialEffect type, CEntityBase* actor, CEntityBase* target=NULL, sint32 param=0, sint32 param2=0);

/**
 * A natural event (forage source explosion, toxic cloud...) hits an entity, send all relevant messages to the entities around.
 */
void sendNaturalEventHitMessages( RYZOMID::TTypeId aggressorType, const NLMISC::CEntityId &victimId, sint32 amount, sint32 amountWithoutArmor, sint32 avoided=0 );

inline void sendNaturalEventHitMessages( RYZOMID::TTypeId aggressorType, const TDataSetRow &victimRowId, sint32 amount, sint32 amountWithoutArmor, sint32 avoided=0 )
{
	if (TheDataset.isAccessible(victimRowId))
	{
		sendNaturalEventHitMessages( aggressorType, TheDataset.getEntityId( victimRowId ), amount, amountWithoutArmor, avoided );
	}
}


/**
 * fumble messages
 * \param aggressorId NLMISC::CEntityId of the acting entity
 * \param victimId NLMISC::CEntityId of the targeted entity
 */
void sendFumbleMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );
inline void sendFumbleMessage( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId )
{
	if (TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId))
	{
		sendFumbleMessage( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ) );
	}
}

/**
 * send dodge messages to client
 */
void sendDodgeMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, bool sendFlyingText  );
inline void sendDodgeMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, bool sendFlyingText )
{
	if (TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId))
	{
		sendDodgeMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ), sendFlyingText  );
	}
}

/**
 * send parry messages to client
 */
void sendParryMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, bool sendFlyingText  );
inline void sendParryMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, bool sendFlyingText  )
{
	if (TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId))
	{
		sendParryMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ), sendFlyingText );
	}
}

/**
 * an entity miss another entity, send all relevant messages to the entities around and the entites concerned
 * \param aggressorId NLMISC::CEntityId of the acting entity
 * \param victimId NLMISC::CEntityId of the victim entity
 */
void sendCombatFailedMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, ECombatFailDisplay failDisplay );

/**
 * an entity miss another entity, send all relevant messages to the entities around and the entites concerned
 * \param aggressorRowId row id of the acting entity
 * \param victimRowId row id of the victim entity
 */
inline void sendCombatFailedMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, ECombatFailDisplay failDisplay )
{
	if (TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId))
	{
		sendCombatFailedMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ), failDisplay );
	}
}


/**
 * send a simple (only a string) message to the specified entity (or chat group)
 * \param entityId CEntityId of the entity to whom the message is sent
 * \param msgName name of the message
 */
void sendSimpleMessage( const NLMISC::CEntityId &entityId, const std::string &msgName );
inline void sendSimpleMessage( const TDataSetRow &entityRowId, const std::string &msgName )
{
	if (TheDataset.isAccessible(entityRowId) )
	{
		sendSimpleMessage( TheDataset.getEntityId( entityRowId ), msgName );
	}
}


/**
 * send a message with a string param to the specified entity (or chat group)
 * \param entityId CEntityId of the entity to whom the message is sent 
 * \param msgName name of the message
 * \param txt the text param
 */
void sendMessage( const NLMISC::CEntityId &entityId, const std::string &msgName, const ucstring &txt );

/**
 * send a message with a string param to the specified entity (or chat group)
 * \param entityId CEntityId of the entity to whom the message is sent 
 * \param msgName name of the message
 * \param entityIdForText entityId which name appears in the message
 */
void sendMessage( const NLMISC::CEntityId &entityId, const std::string &msgName, const NLMISC::CEntityId &entityIdForText );
inline void sendMessage( const TDataSetRow &entityRowId, const std::string &msgName, const TDataSetRow &entityRowIdForText )
{
	if (TheDataset.isAccessible(entityRowId) && TheDataset.isAccessible(entityRowIdForText) )
		sendMessage( TheDataset.getEntityId( entityRowId ), msgName, TheDataset.getEntityId( entityRowIdForText ) );
}


/**
 * send a message to the GPMS requesting all the entities of chosen types in a circle of specified radius around the target entity
 * \param target Id of the targeted entity (must be valid)
 * \param radius radius of the circle in meters
 * \param validTargets list of valid target types
 * \param sentence the calling sentence (needed to find it when the GPMS answer)
 */
//void sendAreaRequest( const NLMISC::CEntityId &targetEntity, float radius, const std::list<RYZOMID::ETypeId> &validTypes, CSentence *sentence);


/**
 * send a message to update an entity behaviour, sent to the OPS or to the AGG
 * \param entityId id of the entity changing it's behaviour
 * \param behaviour the new entity behaviour
 */
void sendUpdateBehaviour( const NLMISC::CEntityId &entityId, const MBEHAV::CBehaviour &behaviour, bool forceUpdate = false );

/**
 * send a message to update an entity behaviour, sent to the OPS or to the AGG
 * \param entityRowId row id of the entity changing it's behaviour
 * \param behaviour the new entity behaviour
 */
inline void sendUpdateBehaviour( const TDataSetRow &entityRowId, const MBEHAV::CBehaviour &behaviour, bool forceUpdate = false  )
{
	if (TheDataset.isAccessible(entityRowId))
	{
		sendUpdateBehaviour( TheDataset.getEntityId( entityRowId ), behaviour, forceUpdate );
	}
}


/**
 * an entity engage another entity in melee combat
 * \param entityId the active entity Id
 * \param targetId the entity being engaged
 * \param mode the mode of the engagement (default = 1 = normal)
 * \return true if the target has been engaged, false otherwise
 */
bool engageTargetInMelee( const NLMISC::CEntityId &entityId , const NLMISC::CEntityId &targetId , sint8 mode = 1);


/**
 * an entity engage another entity in range combat
 * \param entityId the active entity Id
 * \param targetId the entity being engaged
  * \return true if the target has been engaged, false otherwise
 */
bool engageTargetRange( const NLMISC::CEntityId &entityId , const NLMISC::CEntityId &targetId );


/**
 * send engage messages to clients
 * \param aggressorId id of the attacking entity
 * \param victimId id of the attacked entity
 */
void sendEngageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );

/**
 * send disengage messages to clients
 * \param aggressorId id of the attacking entity now disengaging from combat
 * \param victimId id of the attacked entity
 */
void sendDisengageMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );

/**
 * send spell resists message
 * \param aggressorId id of the attacking entity
 * \param victimId id of the attacked entity
 */
void sendSpellResistMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId  );

/**
 * send spell begin cast message
 */
void sendSpellBeginCastMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, ACTNATURE::TActionNature nature );


void sendSpellSuccessMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId );
void sendSpellFailedMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId );
void sendSpellFumbleMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId );

/**
 * send critical hit messages
 * \param aggressorId id of the attacking entity 
 * \param victimId id of the attacked entity
 */
void sendCriticalHitMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );
inline void sendCriticalHitMessage( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId )
{
	if ( TheDataset.isAccessible(aggressorRowId) && TheDataset.isAccessible(victimRowId) )
	{
		sendCriticalHitMessage( TheDataset.getEntityId(aggressorRowId), TheDataset.getEntityId(victimRowId) );
	}
}


/**
 * test if PVP or any other offensive action is allowed against target entity
 * \param actorId if the acting entity
 * \param targetId if the target entity
 * \param errorCode the string that will receive the errorCode if any
 * \return true if an offensive action is allowed
 */
bool testOffensiveActionAllowed( const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, std::string &errorCode, bool mainTarget = true);
inline bool testOffensiveActionAllowed( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, std::string &errorCode, bool mainTarget = true)
{
	if ( TheDataset.isAccessible(actorRowId) && TheDataset.isAccessible(targetRowId) )
	{
		return testOffensiveActionAllowed( TheDataset.getEntityId(actorRowId), TheDataset.getEntityId(targetRowId), errorCode, mainTarget);
	}
	else
	{
		return false;
	}
}

/**
 * test if the target of a spell is valid
 * \param actorRowId spell caster
 * \param targetRowId target id
 * \param action nature of the spelll
 * \param mainTarget true if it's spell main target
 * \return true if allowed
 */
bool validateSpellTarget( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, ACTNATURE::TActionNature action, std::string &errorCode, bool mainTarget );


/**
 * send a dynamic string message and display it in client system info window
 * \param playerId CEntityId of the character
 * \param msgName the message name
 * \param params vector of the message params
 * \return the dyn string id (uint32)
 */
//uint32 sendDynamicSystemMessage(const NLMISC::CEntityId &playerId, const std::string &msgName, const TVectorParamCheck &params);
uint32 sendDynamicSystemMessage(const TDataSetRow &playerRowId, const std::string &msgName, const TVectorParamCheck &params);
inline uint32 sendDynamicSystemMessage(const TDataSetRow &playerRowId, const std::string &msgName)
{
	if( TheDataset.isAccessible(playerRowId) )
	{
		static const TVectorParamCheck noparams;
		return sendDynamicSystemMessage(playerRowId, msgName, noparams);
	}
	return 0;
}


/**
 * send a dynamic string message and display it in client system info window
 * \param playerId CEntityId of the character
 * \param stringId the dynamic message string id
 */
void sendDynamicSystemMessage(const TDataSetRow &playerRowId, uint32 stringId);


/**
 * send a dynamic string message to a chat group
 * \param senderId CEntityId of the sender (center of the chat group)
 * \param excluded the CEntityId of the entities excluded from the chat group
 * \param msgName the message name
 * \param params vector of the message params
 */
		void	sendDynamicGroupSystemMessage(const TDataSetRow &senderId, const std::vector<NLMISC::CEntityId> &excluded, const std::string &msgName, const TVectorParamCheck &params);
inline void	sendDynamicGroupSystemMessage(const TDataSetRow &senderId, const std::string &msgName, const TVectorParamCheck &params )
{
	static const std::vector<NLMISC::CEntityId> excluded;
	sendDynamicGroupSystemMessage(senderId,excluded,msgName,params);
}

/**
 * send effects begin message for standard messages
 */
void sendEffectStandardBeginMessages(const NLMISC::CEntityId &creatorId, const NLMISC::CEntityId &targetId, const std::string &effectName);
inline void sendEffectStandardBeginMessages(TDataSetRow creatorRowId, TDataSetRow targetRowId, const std::string &effectName)
{
	if ( !( TheDataset.isAccessible(creatorRowId) && TheDataset.isAccessible(targetRowId)) )
	{
		sendEffectStandardBeginMessages(TheDataset.getEntityId(creatorRowId), TheDataset.getEntityId(targetRowId), effectName);
	}
}

/**
 * send effects ends message for standard messages
 */
void sendEffectStandardEndMessages(const NLMISC::CEntityId &creatorId, const NLMISC::CEntityId &targetId, const std::string &effectName);
inline void sendEffectStandardEndMessages(TDataSetRow creatorRowId, TDataSetRow targetRowId, const std::string &effectName)
{
	if ( !( TheDataset.isAccessible(creatorRowId) && TheDataset.isAccessible(targetRowId)) )
	{
		sendEffectStandardEndMessages(TheDataset.getEntityId(creatorRowId), TheDataset.getEntityId(targetRowId), effectName);
	}
}

/////////////////////////// temp /////////////////////////////////
// send dmg spell message
void sendScoreModifierSpellMessage( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, sint32 damage, sint32 maxDamage, SCORES::TScores score, ACTNATURE::TActionNature nature );

// send heal spell message

// send resist message


inline void updateMirrorTargetList(CMirrorPropValueList<uint32>& targetList, const TDataSetRow & target, float distance,bool resist)
{
	static float maxDistance = 100.0f;
	uint8 byte = (distance >= maxDistance) ? 127 : uint8(127 * distance / maxDistance);
	if ( resist )
		byte |= 0x80;
	 
	targetList.push_front(TDataSetIndex(byte)); // distance!
	uint32 utarget = *((uint32*)(&target));
	targetList.push_front(utarget);
	// TEMP: call to testList() assumes there are 2 push_front() in this function
}

/// return true if the entity is FORCED to fail an action (cast, craft, combat...) (due to blindness for exemple)
bool forceActionFailure(CEntityBase *entity);


}; // PHRASE_UTILITIES



#endif // EGS_PHRASE_UTILITIES_FUNCTIONS_H

/* End of phrase_utilities_functions.h */
