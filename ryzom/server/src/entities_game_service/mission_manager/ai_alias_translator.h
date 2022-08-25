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



#ifndef RY_AI_ALIAS_TRANSLATOR_H
#define RY_AI_ALIAS_TRANSLATOR_H

#include "nel/ligo/primitive.h"
#include "game_share/misc_const.h"
#include "game_share/ryzom_entity_id.h"


/**
 * Singleton used to get a bot entity id from its AI id and vice-versa
 * Useful because AIIDs are guaranteed to be the same between two server sessions
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
class CAIAliasTranslator
{
public:
	static const TAIAlias Invalid;

	///\return the singleton's instance and create it if it does not exist
	static inline CAIAliasTranslator* getInstance();

	///\init the translator
	static void init();

	// release the singleton's data
	static void release();

	/**
	 * update the association between a bot AI id and its entity id
	 * \param AIId : the AI id of the entity
	 * \param entityId : the CEntityId of the entity
	 */
	inline void updateAssociation(TAIAlias aiid, const NLMISC::CEntityId& entityId);

	/// remove an association
	inline void removeAssociation(NLMISC::CEntityId& entityId);
	
	/**
	 * Get a bot entityId
	 * \param aiid : the AI id of the bot
	 * \return the entityId of the searched bot
	 */
	inline const NLMISC::CEntityId & getEntityId(TAIAlias aiid) const;

	/**
	 * Get a bot AIId
	 * \param entityId: the entityId of the searched bot
	 * \return he AI id of the bot	 
	 */
	TAIAlias getAIAlias(const NLMISC::CEntityId & entityId) const;


	/**
	 * Get a bot unique id from its name
	 * \param botName : name of the bot
	 * \param ret : vector of names
	 */
	inline void getNPCAliasesFromName(const std::string & botName, std::vector<TAIAlias> & ret) const;

	/**
	 * Get a bot unique id from its name
	 * \param botName : name of the bot
	 * \param ret : vector of names
	 */
	inline void getNPCAliasesFromNameInSet(const std::string & botName, std::set<TAIAlias> & ret) const;

	/**
	 * get the name of a bot from its alias
	 * \param alias alias of the bot
	 * \return true on success
	 * \param ret: ref on a string storing the bot name
	 */
	inline bool getNPCNameFromAlias(TAIAlias alias,std::string & ret) const;

	/**
	 * Get a mission unique id from its name
	 * \param botName : name of the bot
	 * \return the id of the bot (invalid if not found
	 */
	inline TAIAlias getMissionUniqueIdFromName(const std::string & missionName) const;

	/**
	 * Retreive the mission name given the alias
	 * WARNING : SLOW : use it only for commands / stats ( iteration through a hash_map )
	 */
	inline const std::string &getMissionNameFromUniqueId(TAIAlias alias) const;
	
	/**
	 * Get an AI groupalias from its name
	 * \param botName : name of the group
	 * \param aliases: vrctor receiving the aliases
	 */
	inline void getGroupAliasesFromName(const std::string & name, std::vector< TAIAlias >& aliases) const;


	/**
	 * Send alias, and eid to ios via mirror.	 
	 */

	void sendAliasToIOS() const	;


private:
	/**
	 * build the bot tree from a primitive ( set the association between bots and id );
	 * \param prim : the primitive to be parsed
	 */
	void buildBotTree(const NLLIGO::IPrimitive* prim);

	/**
	 * build the mission tree from a primitive ( set the association between mission names and id );
	 * \param prim : the primitive to be parsed
	 */
	void buildMissionTree(const NLLIGO::IPrimitive* prim);
	
	/// Constructor (private because it is a singleton)
	CAIAliasTranslator();

	/// destructor (private because this singleton must be deleted through the release method)
	~CAIAliasTranslator();

	/// singleton instance
	static CAIAliasTranslator* _Instance;

	/// hash table using AI id as keys
	CHashMap< uint, NLMISC::CEntityId >						_HashTableAiId;
	CHashMap< NLMISC::CEntityId, TAIAlias,NLMISC::CEntityIdHashMapTraits>	_HashTableEntityId;
	
	/// map linking bot names to ids
	CHashMultiMap< std::string, TAIAlias >		_BotNamesToIds;
	/// map linking bot ids to names
	CHashMap< uint, std::string >			_BotIdsToNames;
	/// map linking mission names to ids
	CHashMap< std::string, TAIAlias >		_MissionNamesToIds;
	/// map linking AI group names to IDS
	CHashMultiMap< std::string, TAIAlias > _AIGroupNamesToIds;

	/// bool set to true if the mission and bot names must be kept
	bool _KeepNames;
};

//-----------------------------------------------
// CAIAliasTranslator getNPCAliasesFromName
//-----------------------------------------------
inline void CAIAliasTranslator::getNPCAliasesFromName(const std::string & botName, std::vector<TAIAlias> & ret) const
{
	ret.clear();
	std::string lwr = NLMISC::strlwr(botName);
	std::pair< CHashMultiMap< std::string, TAIAlias>::const_iterator, CHashMultiMap< std::string, TAIAlias>::const_iterator > result = _BotNamesToIds.equal_range(lwr);
	for ( CHashMultiMap< std::string, TAIAlias>::const_iterator it = result.first; it != result.second; ++it )
	{
		ret.push_back( (*it).second );
	}
}

//-----------------------------------------------
// CAIAliasTranslator getNPCAliasesFromNameInSet
//-----------------------------------------------
inline void CAIAliasTranslator::getNPCAliasesFromNameInSet(const std::string & botName, std::set<TAIAlias> & ret) const
{
	ret.clear();
	std::string lwr = NLMISC::strlwr(botName);
	std::pair< CHashMultiMap< std::string, TAIAlias>::const_iterator, CHashMultiMap< std::string, TAIAlias>::const_iterator > result = _BotNamesToIds.equal_range(lwr);
	for ( CHashMultiMap< std::string, TAIAlias>::const_iterator it = result.first; it != result.second; ++it )
	{
		ret.insert( (*it).second );
	}
}

//-----------------------------------------------
// CAIAliasTranslator getNPCAliasFromName
//-----------------------------------------------
inline bool CAIAliasTranslator::getNPCNameFromAlias(TAIAlias alias,std::string & ret) const
{
	CHashMap< uint,std::string >::const_iterator it = _BotIdsToNames.find(alias);
	if ( it == _BotIdsToNames.end() )
		return false;
	ret = (*it).second;
	return true;
}

//-----------------------------------------------
// CAIAliasTranslator getGroupAliasesFromName
//-----------------------------------------------
inline void CAIAliasTranslator::getGroupAliasesFromName(const std::string & name, std::vector< TAIAlias >& aliases) const
{
	std::string lwr = NLMISC::strlwr(name);
	CHashMultiMap< std::string, TAIAlias>::const_iterator it = _AIGroupNamesToIds.find(lwr);
	while ( it != _AIGroupNamesToIds.end() && (*it).first == lwr )
	{
		aliases.push_back( (*it).second );
		++it;
	}
}

//-----------------------------------------------
// CAIAliasTranslator getInstance
//-----------------------------------------------
inline TAIAlias CAIAliasTranslator::getMissionUniqueIdFromName(const std::string & missionName) const
{
	std::string lwr = NLMISC::strlwr(missionName);
	CHashMap< std::string, TAIAlias>::const_iterator it = _MissionNamesToIds.find(lwr);
	if ( it == _MissionNamesToIds.end() )
		return Invalid;
	return (*it).second;
}

inline const std::string &CAIAliasTranslator::getMissionNameFromUniqueId(TAIAlias alias) const
{
	static const std::string emptyString;

	CHashMap< std::string, TAIAlias>::const_iterator first(_MissionNamesToIds.begin()), last(_MissionNamesToIds.end());
	for (; first != last; ++first)
	{
		if (first->second == alias)
			return first->first;
	}
	return emptyString;
}


//-----------------------------------------------
// CAIAliasTranslator getInstance
//-----------------------------------------------
inline CAIAliasTranslator* CAIAliasTranslator::getInstance()
{
	return _Instance;
}// CAIAliasTranslator getInstance

//-----------------------------------------------
// CAIAliasTranslator updateAssociation
//-----------------------------------------------
inline void CAIAliasTranslator::updateAssociation(TAIAlias aiid, const NLMISC::CEntityId& entityId)
{
	if ( entityId == NLMISC::CEntityId::Unknown )
	{
		nlwarning("Invalid entity %s (alias = %u)",entityId.toString().c_str(),aiid);
		return;
	}
	if ( aiid == Invalid )
	{
		// Doub: No warning because Invalid aliases (0) are allowed (e.g. dynamic fauna)
		//nlwarning("Invalid alias %u for entity %s",aiid, entityId.toString().c_str());
		return;
	}

	_HashTableAiId.insert( std::make_pair( (uint)aiid,entityId ) );
	_HashTableEntityId.insert( std::make_pair( entityId,aiid ) );
	
}// CAIAliasTranslator updateAssociation

//-----------------------------------------------
// CAIAliasTranslator removeAssociation
//-----------------------------------------------
inline void CAIAliasTranslator::removeAssociation(NLMISC::CEntityId& entityId)
{
	if ( entityId.getType() != RYZOMID::npc )
		return;
	CHashMap< NLMISC::CEntityId, TAIAlias, NLMISC::CEntityIdHashMapTraits>::iterator itEid = _HashTableEntityId.find(entityId);
	if ( itEid == _HashTableEntityId.end() )
	{
		// No warning because not always inserted, see updateAssociation() (+ other reason it's already commented out?)
		//nlwarning("<CAIAliasTranslator removeAssociation> alias not found for %s",entityId.toString().c_str());
		return;
	}
	uint alias = (*itEid).second;
	if( _HashTableAiId.erase( alias ) == 0 )
	{
		nlwarning("<CAIAliasTranslator removeAssociation> alias %u has no entityId. DUPLICATED ALIAS?",alias);
	}
	_HashTableEntityId.erase(itEid);
}// CAIAliasTranslator removeAssociation

//-----------------------------------------------
// CAIAliasTranslator getEntityId
//-----------------------------------------------
inline const NLMISC::CEntityId& CAIAliasTranslator::getEntityId(TAIAlias aiid) const
{
	CHashMap< uint, NLMISC::CEntityId >::const_iterator it = _HashTableAiId.find(aiid);
	if(  it != _HashTableAiId.end() )
		return (*it).second;
#ifndef FINAL_VERSION
	nlerror( "Illegal call to getEntityId on entity with no alias" ); // see 
#endif
	return NLMISC::CEntityId::Unknown;
}// CAIAliasTranslator getEntityId




#endif // RY_AI_ALIAS_TRANSLATOR_H

/* End of ai_alias_translator.h */
