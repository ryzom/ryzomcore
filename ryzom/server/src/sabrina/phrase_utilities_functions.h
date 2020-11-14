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


// misc
#include "nel/misc/types_nl.h"
// game_share
#include "game_share/mode_and_behaviour.h"
#include "game_share/armor_types.h"
#include "game_share/damage_types.h"
#include "game_share/shield_types.h"
#include "game_share/slot_equipment.h"
#include "game_share/weapon_types.h"
#include "game_share/creature_size.h"
#include "game_share/intensity_types.h"
#include "game_share/entity_action.h"
#include "game_share/sentence_appraisal.h"
//
#include "entity_base.h"



// Stuff used for management of log messages
extern bool VerboseBrickManagerInfo;
#define INFOLOG if (!VerboseBrickManagerInfo) {} else nlinfo

extern bool VerboseBrickManagerDbg;
#define DEBUGLOG if (!VerboseBrickManagerDbg) {} else nldebug

extern bool AggroLog;
#define AGGROLOG if (!AggroLog) {} else nlinfo


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


/**
 * convert a ERange value to the corresponding string
 */
std::string toString(ERange range);

#define HANDS_DAMAGE_FACTOR_MULTIPLIER 0.8

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
 * get a pointer on the CEntityBase object related to the specified entity Id
 * \param id the entity id
 * \return pointer on the CEntityBase object, or NULL if not found
 */
CEntityBase* entityPtrFromId( const NLMISC::CEntityId &id );

/**
 * get a pointer on the CEntityBase object related to the specified datasetrow
 * \param entityRowId the entity datasetrow
 * \return pointer on the CEntityBase object, or NULL if not found
 */
inline CEntityBase* entityPtrFromId( const TDataSetRow &entityRowId )
{
	if (entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId))
	{
		return entityPtrFromId( TheDataset.getEntityId( entityRowId ) );
	}
	else
		return NULL;
}


/**
 * get the chance of sucess for a relative level
 * \param playerRelativeLevel relative level of the player
 * \return chance of success
 */
uint8 getSuccessChance( sint32 playerRelativeLevel );


/**
 * get Success Factor
 * \param chance of success (5 - 95)
 * \param tirage (0 - 100 )
 * \return 1.1 for critical success
 * \return 1.0 for success
 * \return 1 < SF < 0 for partial success
 * \return  for failure
 * \return SF < 0 for critical failure
*/
float getSucessFactor( uint8 chances, uint8 tirage );


/**
 * test the target is a valid target for Combat
 * \param targetId the target entity Id
 * \param errorCode reference on the string that will receive the error code if any
 * \return true if the target is valid, false otherwise
 */
bool testTargetValidityForCombat( const NLMISC::CEntityId &targetId, std::string &errorCode );

/**
 * test the target is a valid target for Combat
 * \param targetId the target entity Id
 * \param errorCode reference on the string that will receive the error code if any
 * \return true if the target is valid, false otherwise
 */
inline bool testTargetValidityForCombat( const TDataSetRow &entityRowId, std::string &errorCode )
{
	if ( entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
		return testTargetValidityForCombat( TheDataset.getEntityId(entityRowId), errorCode );
	else
		return false;
}

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
	if (attackerRowId.isValid() && TheDataset.isDataSetRowStillValid(attackerRowId) && defenderRowId.isValid() && TheDataset.isDataSetRowStillValid(defenderRowId))
		return getLocalisationSizeAdjustement( TheDataset.getEntityId(attackerRowId), TheDataset.getEntityId(defenderRowId) );
	else
		return 0;
}


/**
 * get the weapon damage factor
 * \param type the weapon type (LIGHT, MEDIUM, HEAVY...)
 * \param quality the weapon quality
 * \param entityLevel the level of the entity using this weapon (default = 0 -> no influence on damage factor)
 * \return the weapon damage factor
 */
float getWeaponDamageFactor( WEAPONTYPE::EWeaponType type, uint16 quality, uint16 entityLevel = 0 );

/**
 * get the armor protection characteritics (protection in % and max protection points)
 * \param type the armor type (LIGHT, MEDIUM, HEAVY...)
 * \param quality the armor quality
 * \return the armor charac
 */
TPairUInt8UInt16 getArmorCharacteristics( ARMORTYPE::EArmorType type, uint16 quality );


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
 * get the entity level in the specified skill and specialization
 * \param entityId the entity NLMISC::CEntityId
 * \param skill the skill used
 * \param skillModifier bonus/malus applied on skill level (-20 = -2 level)
 * \param skillMultiplier factor on skill value (>0)
 * \return the entity level (uint32)
 */
uint32 getEntityLevel( const NLMISC::CEntityId &entityId, SKILLS::ESkills skill, sint32 skillModifier = 0, float skillMultiplier = 1.0f);
inline uint32 getEntityLevel( const TDataSetRow &entityRowId, SKILLS::ESkills skill, sint32 skillModifier = 0,float skillMultiplier = 1.0f)
{
	if ( entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
	{
		return getEntityLevel( TheDataset.getEntityId(entityRowId), skill, skillModifier, skillMultiplier);
	}
	else
		return 0;
}


/**
 * get the entity level in the specified score
 * \param entityId the entity NLMISC::CEntityId
 * \param score the scor used
 * \param scoreModifier bonus/malus applied on score (-20 = -2 level)
 * \param scoreMultiplier factor applied on score
 * \return the entity level (uint32)
 */
uint32 getEntityLevel( const NLMISC::CEntityId &entityId, SCORES::TScores score, sint32 scoreModifier = 0, float scoreMultiplier = 1.0f );
inline uint32 getEntityLevel( const TDataSetRow &entityRowId, SCORES::TScores score, sint32 scoreModifier = 0, float scoreMultiplier = 1.0f )
{
	if ( entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
	{
		return getEntityLevel( TheDataset.getEntityId(entityRowId), score, scoreModifier, scoreMultiplier );
	}
	else
		return 0;
}

/**
 * get the distance between two entities
 * \param entity1 the first entity
 * \param entity2 the second entity
 * \return the distance in meters, -1 if invalid entities or properties not found
 */
double getDistance( const NLMISC::CEntityId &entity1, const NLMISC::CEntityId &entity2 );
inline double getDistance( const TDataSetRow &entity1RowId, const TDataSetRow &entity2RowId )
{
	if ( entity1RowId.isValid() && TheDataset.isDataSetRowStillValid(entity1RowId) && entity2RowId.isValid() && TheDataset.isDataSetRowStillValid(entity2RowId) )
	{
		return getDistance( TheDataset.getEntityId(entity1RowId), TheDataset.getEntityId(entity2RowId) );
	}
	else
		return 0.0;
}



/**
 * inflict damage to the specified Item
 * \param ownerId the NLMISC::CEntityId of the item's owner
 * \param slot the slot where this item is stored
 * \param damage the amount of damage dealt to the item
 */
//void inflictDamageOnItem( const NLMISC::CEntityId &itemId, SLOT_EQUIPMENT::TSlotEquipment slot, sint32 damage );

/**
 * inflict damage to the specified Item
 * \param itemPtr pointre on the CGameItem to damage
 * \param damage the amount of damage dealt to the item
 */
//void inflictDamageOnItem( CGameItemPtr itemPtr, sint32 damage );

/**
 * send Death messages when an entity kills another one
 * \param killerId NLMISC::CEntityId of the acting entity (the one which kills the other one)
 * \param deadId NLMISC::CEntityId of the dead entity
 */
void sendDeathMessages( const NLMISC::CEntityId &killerId, const NLMISC::CEntityId &deadId );
inline void sendDeathMessages( const TDataSetRow &killerRowId, const TDataSetRow &deadRowId )
{
	if (killerRowId.isValid() && TheDataset.isDataSetRowStillValid(killerRowId) && deadRowId.isValid() && TheDataset.isDataSetRowStillValid(deadRowId))
	{
		sendDeathMessages( TheDataset.getEntityId( killerRowId ), TheDataset.getEntityId( deadRowId ) );
	}
}


/**
 * an entity hits another entity, send all relevant messages to the entities around
 * \param aggressorId NLMISC::CEntityId of the acting entity (the one which deals damage)
 * \param victimId NLMISC::CEntityId of the victim entity
 * \param sentence pointer on the calling sentence if any
 * \param amount the amount of damage dealt to the victim entity
 * \param lostStamina the amount of stamina lost by the victim entity
 * \param lostSap the amount of sap lost by the victim entity
 */
void sendHitMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId, sint32 amount, sint32 lostStamina = 0, sint32 lostSap = 0);

inline void sendHitMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, sint32 amount, sint32 lostStamina = 0, sint32 lostSap = 0)
{
	if (aggressorRowId.isValid() && TheDataset.isDataSetRowStillValid(aggressorRowId) && victimRowId.isValid() && TheDataset.isDataSetRowStillValid(victimRowId) )
	{
		sendHitMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ), amount, lostStamina, lostSap );
	}
}

/**
 * an entity hits another entity but does no damage, send all relevant messages to the entities around
 * \param aggressorId NLMISC::CEntityId of the acting entity (the one which deals damage)
 * \param victimId NLMISC::CEntityId of the victim entity
 */
void sendHitNullMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );
inline void sendHitNullMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId )
{
	if (aggressorRowId.isValid() && TheDataset.isDataSetRowStillValid(aggressorRowId) && victimRowId.isValid() && TheDataset.isDataSetRowStillValid(victimRowId))
	{
		sendHitNullMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ) );
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
	if (aggressorRowId.isValid() && TheDataset.isDataSetRowStillValid(aggressorRowId) && victimRowId.isValid() && TheDataset.isDataSetRowStillValid(victimRowId))
	{
		sendFumbleMessage( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ) );
	}
}



/**
 * an entity resits the use of a strategy of another entity, send all relevant messages to the entities around and the entites concerned
 * \param aggressorId NLMISC::CEntityId of the acting entity
 * \param victimId NLMISC::CEntityId of the victim entity
 */
void sendCombatResistMessages( const NLMISC::CEntityId &aggressorId, const NLMISC::CEntityId &victimId );

/**
 * an entity resits the use of a strategy of another entity, send all relevant messages to the entities around and the entites concerned
 * \param aggressorRowId row id of the acting entity
 * \param victimRowId row id of the victim entity
 */
inline void sendCombatResistMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId )
{
	if (aggressorRowId.isValid() && TheDataset.isDataSetRowStillValid(aggressorRowId) && victimRowId.isValid() && TheDataset.isDataSetRowStillValid(victimRowId))
	{
		sendCombatResistMessages( TheDataset.getEntityId( aggressorRowId ), TheDataset.getEntityId( victimRowId ) );
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
	if (entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) )
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
	if (entityRowId.isValid() && TheDataset.isDataSetRowStillValid(entityRowId) &&  entityRowIdForText.isValid() && TheDataset.isDataSetRowStillValid(entityRowIdForText) )
		sendMessage( TheDataset.getEntityId( entityRowId ), msgName, TheDataset.getEntityId( entityRowIdForText ) );
}


/**
 * send a dynamic message to the chat
 * \param entityId CEntityId of the entity executiong a sentence
 * \param msgText text of the message
 */
void sendDynamicChatMessage( const NLMISC::CEntityId &entityId, const std::string &msgText );


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
	if (entityRowId.isValid())
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
void sendSpellBeginCastMessages( const TDataSetRow &aggressorRowId, const TDataSetRow &victimRowId, ACTNATURE::EActionNature nature );


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
	if ( aggressorRowId.isValid() && TheDataset.isDataSetRowStillValid(aggressorRowId) && victimRowId.isValid() && TheDataset.isDataSetRowStillValid(victimRowId) )
	{
		sendCriticalHitMessage( TheDataset.getEntityId(aggressorRowId), TheDataset.getEntityId(victimRowId) );
	}
}

/**
 * get the relative intensity of the impact according to target
 * \param impact strength of the impact
 * \param targetId the target id
 * \param score the affected score
 * \return the impact intensity
 */
INTENSITY_TYPE::TImpactIntensity getImpactIntensity( sint32 impact, const TDataSetRow &victimRowId, SCORES::TScores score = SCORES::hit_points );

/**
 * get the intensity of the attack
 * \param attackLevel level of the attack
 * \return the attack intensity
 */
INTENSITY_TYPE::TAttackIntensity getAttackIntensity( uint32 attackLevel);


// get entity skill value
inline uint32 getEntitySkill( const NLMISC::CEntityId &entityId, SKILLS::ESkills skill )
{
	CEntityBase* entity = entityPtrFromId( entityId );
	if (entity == NULL)
	{
		nlwarning("<getEntitySkill> Invalid entity Id %s", entityId.toString().c_str() );
		return 0;
	}
	if ( skill < SKILLS::NUM_SKILLS )
	{
		return entity->getSkills()._Skills[ skill ].Current;
	}
	return 0;
}

/**
 * get the level for a given 'skill' 
 */
inline uint32 getLevelFromSkill(uint32 comp)
{
/*	if (comp % 10 != 0)
	{
		comp /= 10;
		++comp;
	}
	else
	{
		comp /= 10;
	}

	return comp;
*/
	// 0 -> locked skill 1-9 -> level 0 but unlocked, 10-19 level 1, etc...)
	return comp /= 10;
}

/**
 * get the level for a given 'skill' 
 */
inline uint32 getLevelFromScore(uint32 scoreValue)
{
	//for now we consider scores as skills
	return getLevelFromSkill(scoreValue);
}


/**
 * get the direction (as text) pointed by the specified vector
 * \param dir the direction vector
 * \return the direction as a string
 */
std::string getOrientationString( const NLMISC::CVector &dir);

/**
 * get the direction (as text)
 */
std::string getOrientationString( sint32 x, sint32 y);

/**
 * get the current action type for specified entity
 * \param entityId id of the entity to check
 * \return the entity action as an enum value
 */
ENTITY_ACTION::EEntityAction getEntityCurrentAction( const NLMISC::CEntityId &entityId);


/**
 * test if an entity spell casting is broken when it has been hit
 * \param entity pointer on the entity being hit
 * \param attacker pointer on the entity dealing damage
 * \param damage amount of damage done
 * \param weaponType the type of weapon used (if any, optionnal param)
 */
void testSpellBreakOnDamage( CEntityBase *entity, CEntityBase * attacker, sint32 damage, WEAPONTYPE::EWeaponType weaponType = WEAPONTYPE::UNKNOWN );

/**
 * test if PVP or any other offensive action is allowed against target entity
 * \param actorId if the acting entity
 * \param targetId if the target entity
 * \param errorCode the string that will receive the errorCode if any
 * \return true if an offensive action is allowed
 */
bool testOffensiveActionAllowed( const NLMISC::CEntityId &actorId, const NLMISC::CEntityId &targetId, std::string &errorCode);
inline bool testOffensiveActionAllowed( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, std::string &errorCode)
{
	return testOffensiveActionAllowed( TheDataset.getEntityId(actorRowId), TheDataset.getEntityId(targetRowId), errorCode);
}

/**
 * test if the target of a spell is valid
 * \param actorRowId spell caster
 * \param targetRowId target id
 * \param action nature of the spelll
 * \return true if allowed
 */
inline bool validateSpellTarget( const TDataSetRow &actorRowId, const TDataSetRow &targetRowId, ACTNATURE::EActionNature action )
{
	///\todo nico do something with the error code
	std::string errorCode;
	if ( action == ACTNATURE::OFFENSIVE  )
	{
		return PHRASE_UTILITIES::testOffensiveActionAllowed(actorRowId, targetRowId, errorCode);
	}
	else
	{
		// defensive spells are allowed on everybody except attackable ones
		CEntityBase * target = CEntityBaseManager::getEntityBasePtr( targetRowId );
		return !(target->getContextualProperty().getValue().attackable());
	}
}

/////////////////////////// temp /////////////////////////////////
// send dmg spell message
void sendScoreModifierSpellMessage( CEntityBase * aggressor, CEntityBase* victim, sint32 value, SCORES::TScores score, ACTNATURE::EActionNature nature );

// send heal spell message

// send resist message



}; // PHRASE_UTILITIES



#endif // EGS_PHRASE_UTILITIES_FUNCTIONS_H

/* End of phrase_utilities_functions.h */
