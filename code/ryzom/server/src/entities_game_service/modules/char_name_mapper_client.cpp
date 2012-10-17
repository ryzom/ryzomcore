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

#include "nel/misc/eid_translator.h"
#include "nel/net/service.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/char_name_mapper_itf.h"

#include "egs_pd.h"
#include "player_manager/player_manager_interface.h"
#include "player_manager/character_interface.h"
#include "guild_manager/guild_manager_interface.h"
#include "guild_manager/guild_interface.h"
#include "char_name_mapper_client.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CNM;
using namespace EGSPD;


class CCharNameMapperClient
	:	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav <CModuleBase> > >,
		public CCharNameMapperClientSkel,
		public ICharNameMapperClient
{

	struct TPendingCharName
	{
		CEntityId	CharEid;
		ucstring	CharName;
	};

	typedef list<TPendingCharName>	TPendingcharNames;

	/// A list of character that need to be send to IOS for mapping.
	TPendingcharNames	_PendingCharNames;

	/// The IOS char name mapper module
	TModuleProxyPtr		_CharNameMapper;

	// Number of character mapped by tick
	enum
	{
		NB_NAME_MAPPED_BY_TICK = 1000
	};

	// Received charNames changes from IOS, not still applied to update of contact list
	std::vector<CEntityId>	_ReceivedCharNames;

public:

	CCharNameMapperClient()
	{
		CCharNameMapperClientSkel::init(this);
	}

	void onModuleUp(IModuleProxy *module)
	{
		if (module->getModuleClassName() == "CharNameMapper")
		{
			_CharNameMapper = module;

			// we need to remap all character names
			const CEntityIdTranslator::TEntityCont	&entities = CEntityIdTranslator::getInstance()->getRegisteredEntities();
			CEntityIdTranslator::TEntityCont::const_iterator first(entities.begin()), last(entities.end());

			for (; first != last; ++first)
			{
				mapCharacterName(first->first, first->second.EntityName);
			}
		}
	}

	void onModuleDown(IModuleProxy *module)
	{
		if (module == _CharNameMapper)
		{
			// we have lost the char name mapper.
			_CharNameMapper = NULL;
		}
	}

	void onModuleUpdate()
	{
		H_AUTO(CCharNameMapperClient_onModuleUpdate);

		if (_CharNameMapper != NULL && !_PendingCharNames.empty())
		{
			// send a batch of name each frame
			std::vector < TCharNameInfo > charNameInfos;
			charNameInfos.reserve(NB_NAME_MAPPED_BY_TICK);

			for (uint i=0; i<NB_NAME_MAPPED_BY_TICK && !_PendingCharNames.empty(); ++i)
			{
				const TPendingCharName &pcn = _PendingCharNames.front();
				charNameInfos.push_back(TCharNameInfo());
				charNameInfos.back().setCharEid(pcn.CharEid);
				charNameInfos.back().setCharName(pcn.CharName);

				_PendingCharNames.pop_front();
			}

			nldebug("Sending %u names to IOS for mapping, left %u to map", charNameInfos.size(), _PendingCharNames.size());

			CCharNameMapperProxy cnm(_CharNameMapper);
			cnm.mapCharNames(this, charNameInfos);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Virtual from ICharNameMapperClient
	///////////////////////////////////////////////////////////////////////////

	virtual void mapCharacterName(const NLMISC::CEntityId &charEid, const ucstring &charName)
	{
		_PendingCharNames.push_back(TPendingCharName());

		_PendingCharNames.back().CharEid = charEid;
		_PendingCharNames.back().CharName = charName;

		IService::getInstance()->setCurrentStatus("Mapping names");
	}

	///////////////////////////////////////////////////////////////////////////
	// Virtual from CCharNameMapperClientSkel
	///////////////////////////////////////////////////////////////////////////

	// 
	virtual void charNamesMapped(NLNET::IModuleProxy *sender, const std::vector < TCharMappedInfo > &charMappedInfos)
	{
		nldebug("Receveived %u mapped names from IOS", charMappedInfos.size());


		// ok, a series of name have been mapped, store the result in the 
		// eid translator

		for (uint i=0; i<charMappedInfos.size(); ++i)
		{
			const TCharMappedInfo &cmi = charMappedInfos[i];
			CEntityIdTranslator::getInstance()->setEntityNameStringId(cmi.getCharEid(), cmi.getStringId());

			// Must store in _ReceivedCharNames, for delayed update of contact list below
			_ReceivedCharNames.push_back(cmi.getCharEid());
		}

		
		// When all name request sent, update guild and contact list
		if (_PendingCharNames.empty())
		{
			// and update the object that depends on char string id
			// (for now, it's contact lists and guilds)

			// update the guild member lists (if loading terminated)
			if (IGuildManager::getInstance().guildLoaded())
			{
				const EGSPD::CGuildContainerPD *guilds = IGuildManager::getInstance().getGuildContainer();
				if (guilds != NULL)
				{
					std::map<TGuildId, CGuildPD*>::const_iterator first(guilds->getGuildsBegin()), last(guilds->getGuildsEnd());
					for (; first != last; ++first)
					{
						IGuild *guild = IGuild::getGuildInterface(IGuildManager::getInstance().getGuildFromId(first->first));
						BOMB_IF(guild == NULL, "CCharNameMapperClient::charNamesMapped : failed to retrieve guild "<<first->first, continue);
						
						// update the member list
						guild->updateMembersStringIds();
					}
				}
			}

			// update the character contact list
			if(!_ReceivedCharNames.empty())
			{
				const IPlayerManager::TMapPlayers &players = IPlayerManager::getInstance().getPlayers();

				// update all connected players
				IPlayerManager::TMapPlayers::const_iterator first(players.begin()), last(players.end());
				for (; first != last; ++first)
				{
					CCharacter *character = IPlayerManager::getInstance().getActiveChar(first->first);
					if (character == NULL)
						continue;

					ICharacter *icharacter = ICharacter::getInterface(character, true);
					if (icharacter == NULL)
						// should never append, but whoe knows
						continue;

					icharacter->syncContactListWithCharNameChanges(_ReceivedCharNames);
				}

				// clear the received names
				_ReceivedCharNames.clear();
			}
		}

		if (_PendingCharNames.empty())
		{
			IService::getInstance()->clearCurrentStatus("Mapping names");
		}
	}

};


NLNET_REGISTER_MODULE_FACTORY(CCharNameMapperClient, "CharNameMapperClient");

