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


#include "stdpch.h"
#include "mission_manager/ai_alias_translator.h"
#include "nel/net/service.h"
#include "nel/ligo/primitive_utils.h"
#include "nel/misc/command.h"
#include "nel/net/message.h"
#include "mission_log.h"
#include "primitives_parser.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace NLLIGO;

CAIAliasTranslator* CAIAliasTranslator::_Instance = NULL;
const TAIAlias CAIAliasTranslator::Invalid = 0;

//-----------------------------------------------
// CAIAliasTranslator init
//-----------------------------------------------
void CAIAliasTranslator::init()
{
	nlassert(_Instance == NULL);
	_Instance = new CAIAliasTranslator();
}// CAIAliasTranslator init

//-----------------------------------------------
// CAIAliasTranslator ctor
//-----------------------------------------------
CAIAliasTranslator::CAIAliasTranslator()
{
	CConfigFile::CVar *varPtr = IService::getInstance()->ConfigFile.getVarPtr("StoreBotNames");
	if ( varPtr && varPtr->asInt() != 0 )
	{
		_KeepNames = true;
	}
	else
	{
		_KeepNames = false;
	}
	
	// get the loaded primitives
	const CPrimitivesParser::TPrimitivesList & primsList = CPrimitivesParser::getInstance().getPrimitives();
	nlinfo("loading bot names and mission names");
	CPrimitivesParser::TPrimitivesList::const_iterator first, last;
	for (first = primsList.begin(), last = primsList.end(); first != last; ++first)
	{
		buildBotTree(first->Primitive.RootNode);
		buildMissionTree(first->Primitive.RootNode);
	}
	nlinfo("bot names and mission names loaded");
}// CAIAliasTranslator ctor

//-----------------------------------------------
// CAIAliasTranslator release
//-----------------------------------------------
void CAIAliasTranslator::release()
{
	delete _Instance;
	_Instance = NULL;
}// CAIAliasTranslator release

//-----------------------------------------------
// CAIAliasTranslator destructor
//-----------------------------------------------
CAIAliasTranslator::~CAIAliasTranslator()
{
	_MissionNamesToIds.clear();
	_AIGroupNamesToIds.clear();
	_HashTableAiId.clear();
	_HashTableEntityId.clear();
	
}// CAIAliasTranslator destructor

//-----------------------------------------------
// CAIAliasTranslator buildBotTree
//-----------------------------------------------
void CAIAliasTranslator::buildBotTree(const NLLIGO::IPrimitive* prim)
{
	// look for bot nodes in the primitives
	std::string value,name,aiClass;
	if (prim->getPropertyByName("class",aiClass)  )
	{
		if ( !nlstricmp(aiClass.c_str(),"npc_bot") ||!nlstricmp(aiClass.c_str(),"group_fauna") ||!nlstricmp(aiClass.c_str(),"npc_group") )
		{
			bool error = false;
			if ( !prim->getPropertyByName("name",name) )
			{
				nlwarning("<CAIAliasTranslator buildBotTree> no name property in an AI node '%s'", buildPrimPath(prim).c_str());
				error = true;
			}
			TAIAlias id;
			if ( !CPrimitivesParser::getAlias(prim, id))
			{
				nlwarning("<CAIAliasTranslator buildBotTree> no alias property in an AI node '%s'", buildPrimPath(prim).c_str());
				error = true;
			}
			if (error)
			{
				nlwarning("<CAIAliasTranslator buildBotTree> errors : name='%s' alias='%s' in '%s'",name.c_str(),value.c_str(), buildPrimPath(prim).c_str());
			}

			NLMISC::strlwr(name);
			//remove AI name parameters
			string::size_type trash = name.find('$');
			if ( trash != string::npos )
			{
				name.resize(trash);
			}
			if (!error)
			{
				if ( !nlstricmp(aiClass.c_str(),"npc_bot") )
				{
					_BotIdsToNames.insert( make_pair(id,name) );
					_BotNamesToIds.insert( make_pair(name,id) );
				}
				else
					_AIGroupNamesToIds.insert( make_pair(name,id) );
			}
		}
		if ( !nlstricmp(aiClass.c_str(),"spire") )
		{
			bool error = false;
			// For spires name of the group is "spire_group_"+name where name is the nams of the spire primitive
			if ( !prim->getPropertyByName("name", name) )
			{
				nlwarning("<CAIAliasTranslator buildBotTree> no region property in an spire node '%s'", buildPrimPath(prim).c_str());
				error = true;
			}
			TAIAlias id;
			if ( !CPrimitivesParser::getAlias(prim, id))
			{
				nlwarning("<CAIAliasTranslator buildBotTree> no alias property in an spire node '%s'", buildPrimPath(prim).c_str());
				error = true;
			}
			if (error)
			{
				nlwarning("<CAIAliasTranslator buildBotTree> errors : name='%s' alias='%s' in '%s'",name.c_str(),value.c_str(), buildPrimPath(prim).c_str());
			}

		//	NLMISC::strlwr(name);
			//remove AI name parameters
		//	string::size_type trash = name.find('$');
		//	if ( trash != string::npos )
		//	{
		//		name.resize(trash);
		//	}
			if (!error)
			{
				_AIGroupNamesToIds.insert( make_pair("spire_group_"+name,id) );
			}
		}
	}
	//this is not a mission node, so lookup recursively in the children
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			buildBotTree(child);
	}
}// CAIAliasTranslator buildBotTree

//-----------------------------------------------
// CAIAliasTranslator buildMissionTree
//-----------------------------------------------
void CAIAliasTranslator::buildMissionTree(const NLLIGO::IPrimitive* prim)
{
	// look for mission nodes in the primitives
	std::string value,name;
	if (prim->getPropertyByName("class",value)  )
	{
		if ( !nlstricmp(value.c_str(),"mission") )
		{
			bool error = false;
			TAIAlias id;
			if ( !CPrimitivesParser::getAlias(prim, id) )
			{
				nlwarning("<CAIAliasTranslator buildMissionTree> no alias property in a mission node");
				error = true;
			}
//			TAIAlias id;
//			NLMISC::fromString(value, id);
			if ( !prim->getPropertyByName("name",name) )
			{
				nlwarning("<CAIAliasTranslator buildMissionTree> no name property in a mission node");
				error = true;
			}
			NLMISC::strlwr(name);
			if ( error )
			{
				nlwarning("<CAIAliasTranslator buildMissionTree> errors : name='%s' alias='%s'",name.c_str(),value.c_str());
			}
			else if ( _MissionNamesToIds. insert( make_pair(name,id) ).second == false )
			{
				nlwarning("<CAIAliasTranslator buildMissionTree> The name %s is already assigned, we overwrite it",name.c_str());
			}
			
		}
	}
	//this is not a mission node, so lookup recursively in the children
	for (uint i=0;i<prim->getNumChildren();++i)	
	{
		const IPrimitive *child;
		if ( prim->getChild(child,i) )
			buildMissionTree(child);
	}
}// CAIAliasTranslator buildMissionTree


void CAIAliasTranslator::sendAliasToIOS() const		
{

	NLNET::CMessage msg("UPDATE_AIALIAS");	
	enum {Set=0, Add = 1, Delete=2  };
	uint32 subcommand = static_cast<uint32>(Set);
	msg.serial( subcommand );
	typedef CHashMap< uint, std::string > TContainer;
	TContainer::const_iterator first(_BotIdsToNames.begin());
	TContainer::const_iterator last(_BotIdsToNames.end());		
	uint32 size = static_cast<uint32>(_BotIdsToNames.size());
	msg.serial(size);
	for (; first != last; ++first)
	{
		uint32 alias ((*first).first);
		std::string name ((*first).second);
		msg.serial( alias );
		msg.serial( name );
	}
	sendMessageViaMirror ("IOS", msg);
}

TAIAlias CAIAliasTranslator::getAIAlias(const NLMISC::CEntityId & entityId) const
{

	typedef CHashMap< NLMISC::CEntityId, TAIAlias,NLMISC::CEntityIdHashMapTraits> TContainer;
	TContainer::const_iterator it(_HashTableEntityId.find(entityId));	
	if(  it != _HashTableEntityId.end() )
		return (*it).second;
	return Invalid;
}
