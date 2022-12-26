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



#ifndef CREATURE_MANAGER_H
#define CREATURE_MANAGER_H

//game share
#include "game_share/ryzom_entity_id.h"
#include "server_share/npc_description_messages.h"

#include "entity_manager/entity_manager.h"
#include "creature_manager/creature.h"
#include "server_share/msg_ai_service.h"

extern NLNET::TUnifiedCallbackItem	GenNpcDescCbTable[];


/**
 * Implementation of the bot description Transport class
 */
//class CNpcBotDescriptionImp : public CNpcBotDescription
//{
//public:
//	virtual void callback (const std::string &name, uint8 id);
//};
class CGenNpcDescMsgImp : public RYMSG::TGenNpcDescMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId sid);
};

/**
 * Implementation of the fauna description Transport class
 */
class CFaunaBotDescriptionImp : public CFaunaBotDescription
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Implementation of the gain aggro of a creature against player
 */
class CAIGainAggroMsgImp : public CAIGainAggroMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Implementation of the restor full life message for creatures
 */
class CCreatureCompleteHealImp : public CCreatureCompleteHealMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Implementation of the set Max HP message for creatures
 */
class CChangeCreatureMaxHPImp : public CChangeCreatureMaxHPMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

/**
 * Implementation of the change HP message for creatures
 */
class CChangeCreatureHPImp : public CChangeCreatureHPMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


/**
 * Implementation of the change setUrl message for creatures
 */
class CCreatureSetUrlImp : public CCreatureSetUrlMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};


typedef CHashMap< NLMISC::CEntityId, CCreature *, NLMISC::CEntityIdHashMapTraits> TMapCreatures;

/**
 * CCreatureManager
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CCreatureManager : public CEntityBaseManager
{
	/// map of pointers on all creatures in the game
	TMapCreatures _Creatures;

public:
	/// temporary anti-bug member used by NLMISC_COMMAND: fixAltar
	void fixAltar();

	/// exception thrown when creature is unknown
	struct ECreature : public NLMISC::Exception
	{
		ECreature( const NLMISC::CEntityId& id ) : Exception ("The creature "+id.toString()+" doesn't exist") {}
	};

	/// structure describing an unaffected fauna group ( because the description is received before mirror update )
	struct SUnaffectedFaunaGroup
	{
		SUnaffectedFaunaGroup(const TDataSetRow& entityIndex,TAIAlias	groupAlias)
			:EntityIndex( entityIndex ),GroupAlias( groupAlias ){}
		TDataSetRow EntityIndex;
		TAIAlias	GroupAlias;
	};

	/// Constructor / destructor
	CCreatureManager();
	~CCreatureManager();

	/// Get Number of creaturein manager
	uint32 getNumberCreature() { return (uint32)_Creatures.size(); }

	/// Get a reference on creature in manager
	const TMapCreatures& getCreature() { return _Creatures; }

	/**
	 * Add callback for creatures management
	 */
	void addCreatureCallback();

	/**
	 * Add a creature in the service
	 * \param id is the unique id of the creature
	 * \param creature contains the player info
	 */
	void addCreature( const NLMISC::CEntityId& Id, CCreature *creature );

	/**
	 * Get creature
	 * \param Id is the unique id of the creature
	 * \return the pointer on creature, or null if not found
	 */
	CCreature* getCreature( const NLMISC::CEntityId& Id );

	/**
	 * Get a creature using its dataset row
	 * \param Id is the dataset row of the creature
	 * \return the pointer on creature, or null if not found
	 */
	CCreature* getCreature( const TDataSetRow &entityRowId )	
	{
		return getCreature( getEntityIdFromRow( entityRowId ) );
	}

	/**
	 *  Remove a creature
	 * \param id is the unique id of the creature
	 */
	void removeCreature( const NLMISC::CEntityId& Id );

	/**
	 *  when a Ags has been disconnected, backup and remove all creature managed by him
	 * \param serviceId is the unique id of the Ags
	 */
	void agsDisconnect( NLNET::TServiceId serviceId );

	/**
	 * When GPMS is up, init subscribtion for positions
	 */
	void gpmsConnexion();

	/**
	 * Return the type of the creature (sheet id)
	 */
	NLMISC::CSheetId getType( const NLMISC::CEntityId& id );

	/**
	 *	Set the value of a var
	 * \param Id is the creature id
	 * \param var is the name of the variable
	 * \param value is the new value for the variable
	 */
	void setValue( const NLMISC::CEntityId& Id, const std::string& var, const std::string& value );

	/**
	 *	Modify a var
	 * \param Id is the creature id
	 * \param var is the name of the variable
	 * \param value is the modifier to apply to the variable value
	 */
	void modifyValue( const NLMISC::CEntityId& Id, const std::string& var, const std::string& value );
	
	/**
	 *	Return the value of the variable
	 * \param Id is the creature id
	 * \param var is the name of the variable
	 */
	std::string getValue( const NLMISC::CEntityId& Id, const std::string& var );

	/**
	 * tick update, called every tick
	 */
	void tickUpdate();

	inline void addUnaffectedDescription(const CGenNpcDescMsgImp & desc) { _UnaffectedDescription.push_back(desc); }

	/// add an unaffected fauna group
	inline void addUnaffectedFaunaGroup(const TDataSetRow& entityIndex,TAIAlias groupAlias) { _UnaffectedFaunaGroups.push_back( SUnaffectedFaunaGroup(entityIndex,groupAlias) ); }

	/// dump unaffected description in the specified log
	void dumpUnaffectedFaunaGroups(NLMISC::CLog& log);

	/// get a group of NPCS
	CNPCGroup * getNPCGroup(TAIAlias alias)
	{
		CHashMap< unsigned int,CNPCGroup>::iterator it = _NpcGroups.find( alias );
		if ( it == _NpcGroups.end() )
		{
			return NULL;
		}
		else
		{
			return &( (*it).second );
		}
	}

	/// add an npc to a group or create it (can be called multiple times for the same bot)
	void addNpcToGroup( TAIAlias groupAlias, TAIAlias npcAlias )
	{
		CHashMap< unsigned int,CNPCGroup>::iterator it = _NpcGroups.find( groupAlias );
		if ( it == _NpcGroups.end() )
		{
			CNPCGroup npcs;
			npcs.Members.insert( npcAlias ); // set.insert() to avoid readding the same member
			_NpcGroups.insert(std::make_pair(groupAlias,npcs));
		}
		else
		{
			(*it).second.Members.insert( npcAlias );
		}
	}
	
	/// remove an NPC from a group.
	void removeNpcFromGroup( TAIAlias groupAlias, TAIAlias npcAlias );

private:
	std::list< CGenNpcDescMsgImp >		_UnaffectedDescription;
	std::vector< SUnaffectedFaunaGroup >	_UnaffectedFaunaGroups;
	uint32 _SlideUpdate;
	uint32 _StartCreatureRegen;
	/// map used to store the NPC groups
	CHashMap<unsigned int,CNPCGroup >	_NpcGroups;
};

extern CCreatureManager	CreatureManager;

#endif //CREATURE_MANAGER


