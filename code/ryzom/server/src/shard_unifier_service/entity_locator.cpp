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

/** The entity locator module keep track of each connected
 *	user and character and remember on witch shard they are.
 *
 *	It can be enhanced in the future to keep track of more run
 *	time data about connected user and active guilds.
 *
 *	This module is designed to be instantiated once in the
 *	context of the shard unifier.
 *
 *	It act as a server for modules that manifest to be 
 *	locator client.
 *
 *	This module also have a local interface that can be used 
 *	by collocated module to retrieve locator client module
 *	for a connected character.
 *
 *	Locator client must be instantiated once by shard and
 *	are likely to be run in the context of the EGS.
 *
 *	This is not required by the entity locator, but other
 *	modules (e.g. the mail forum notifier forwarder) may
 *	requires that entity locator client implements some
 *	specific interfaces.
 */


#include "stdpch.h"
#include "nel/net/module.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"

#include "server_share/entity_locator_itf.h"
#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"
#include "game_share/shard_names.h"
#include "game_share/ryzom_entity_id.h"

#include "database_mapping.h"
#include "nel_database_mapping.h"

#include "entity_locator.h"
#include "character_sync.h"

#include <functional>



using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace RSMGR;
using namespace CHARSYNC;

CVariable<uint32>	TotalConcurentUser("ls", "TotalConcurentUser", "The total number od user connected over the domain", false, 0);
CVariable<bool>		LSIgnoreBadDisconnection("ls", "LSIgnoreBadDisconnection", "Set to true to ignore disconnection message from EGS that arrive in wrong order", true, 0, true);

namespace ENTITYLOC
{

	class CEntityLocator: 
		public CEmptyModuleCommBehav<CEmptyModuleServiceBehav<CEmptySocketBehav<CModuleBase> > >,
		public CEntityLocatorSkel,
		public IEntityLocator,
		public ICharacterSyncCb,
		IModuleTrackerCb
	{
		// Database connection
		MSW::CConnection _RingDB;
		MSW::CConnection _NelDb;

		typedef uint32 TUserId;
		typedef uint32 TCharId;
		typedef uint32 TShardId;

/*		struct CHash
		{
			size_t	operator () (const uint32 id) const 
			{
				// recall the standard hashing function
				std::hash<size_t> h;
				return h((size_t)id);
			}
		};

		struct CHashUCString
		{
			size_t	operator () (const ucstring &str) const 
			{
			  unsigned long __h = 0; 
			  ucstring::const_iterator first(str.begin()), last(str.end());
			  for ( ; first != last; ++first)
				__h = 5*__h + *first;

			  return size_t(__h);
			}
		};
*/
		struct TCharInfo
		{
			TShardId	ShardId;
			ucstring	CharName;
		};

		typedef map<TModuleProxyPtr, TShardId>	TLocatorClient;
		typedef map<TShardId, IModuleProxy*>	TLocatorIndex;

		// List of entity locator client modules
		TLocatorClient		_LocatorClients;
		TLocatorIndex		_LocatorIndex;

		typedef CHashMap<TUserId, TShardId>		TUserMap;

		// Current connected players
		TUserMap			_ConnectedUsers;

		typedef CHashMap<TCharId, TCharInfo>		TCharMap;
		// current connected character
		TCharMap			_ConnectedChars;

		typedef CHashMap<ucstring, TCharId, CUCStringHashMapTraits>	TCharNameMap;
		// Index of connected character by name
		TCharNameMap		_ConnectedCharsByName;


		typedef CModuleTracker<TModuleClassPred>	TClientTracker;
		// Tracker for EGS's ShardUnifierClient modules
		TClientTracker		_ClientTracker;

	public:

		CEntityLocator()
			:	_ClientTracker(TModuleClassPred("ShardUnifierClient"))
		{
			CEntityLocatorSkel::init(this);
			_ClientTracker.init(this, this);
		}


		bool initModule(const TParsedCommandLine &pcl)
		{
			// recall base class
			if (!CModuleBase::initModule(pcl))
				return false;

			// init ring db
			const TParsedCommandLine *initRingDb = pcl.getParam("ring_db");
			if (initRingDb  == NULL)
			{
				nlwarning("RSM : missing ring db connection information");
				return false;
			}

			// connect to the database
			if (!_RingDB.connect(*initRingDb))
			{
				nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
				return false;
			}

			// init nel db
			const TParsedCommandLine *initNelDb = pcl.getParam("nel_db");
			if (initNelDb  == NULL)
			{
				nlwarning("RSM : missing nel db connection information");
				return false;
			}

			// connect to the database
			if (!_NelDb.connect(*initNelDb))
			{
				nlwarning("Failed to connect to database using %s", initNelDb->toString().c_str());
				return false;
			}

			return true;
		}

		void onModuleUp(IModuleProxy *proxy)
		{
			// check if the module is the local char synchronizer
			if (proxy->getModuleDistance() == 0
				&& proxy->getModuleClassName() == "CharacterSynchronisation")
			{
				// it's it, register this module as a callback clients
				if (ICharacterSync::getInstance())
					ICharacterSync::getInstance()->registerListener(this);
			}

			// look in the manifest to check for clients
			const std::string &manifest = proxy->getModuleManifest();

			// parse the manifest. We are looking for "locatorClient(shardId=<shardId>)
			TParsedCommandLine pcl;
			if (!pcl.parseParamList(manifest))
			{
				// failed to parse the manifest, this module certainly is not one of our client
				return;
			}
			
			const TParsedCommandLine *shardIdParam = pcl.getParam("locatorClient.shardId");
			if (shardIdParam == NULL)
			{
				// not one of our client
				return;
			}

			// ok, we have a valid client, add it to the list
			uint32 shardId;
			NLMISC::fromString(shardIdParam->ParamValue, shardId);

			// check that we don't have conflict
			TLocatorIndex::iterator locIt(_LocatorIndex.find(shardId));
			{
				if (locIt != _LocatorIndex.end())
				{
					nlwarning("CEntityLocator:onModuleUp : module '%s' claims to be client for shard %u, but already have one from module '%s', second ignored",
						locIt->second->getModuleName().c_str(),
						locIt->first,
						proxy->getModuleName().c_str());

					return;
				}
			}

			nldebug("CEntityLocator:onModuleUp : added module '%s' as locator client for shard %u", proxy->getModuleName().c_str(), shardId);
			// checking ok.
			_LocatorClients.insert(make_pair(proxy, shardId));
			_LocatorIndex.insert(make_pair(shardId, proxy));
		}

		void onModuleDown(IModuleProxy *proxy)
		{
			// check if the module is the local char synchronizer
			if (proxy->getModuleDistance() == 0
				&& proxy->getModuleClassName() == "CharacterSynchronisation")
			{
				// unregister from the character synchronizer
				// it's it, register this module as a callback clients
				ICharacterSync *charSync = ICharacterSync::getInstance();
				if (charSync != NULL)
				{
					charSync->unregisterListener(this);
				}

				return;
			}

			// check if this module is one of our clients
			TLocatorClient::iterator it(_LocatorClients.find(proxy));
			if (it != _LocatorClients.end())
			{
				IModuleProxy *proxy = it->first;
				TShardId shardId = it->second;
				nldebug("CLocatorClient::onModuleDown : removing module '%s' as client for shard %u", proxy->getModuleName().c_str(), shardId);

				// remove any connected char
				{
					vector<TCharConnectionEvent> connections;
					TCharConnectionEvent cce;
					cce.setConnection(false);

					TCharMap::iterator first(_ConnectedChars.begin()), last(_ConnectedChars.end());
					while ( first != last )
					{
						TCharId charId = first->first;
						if (first->second.ShardId == shardId)
						{
							TCharMap::iterator next = first;
							++next;
							// erase the name mapping
							_ConnectedCharsByName.erase(toLower(first->second.CharName));
							// erase the connected char info
							_ConnectedChars.erase(first);
							// add it to the (dis)connection event list
							cce.setCharId(charId);
							connections.push_back(cce);

							// continue on next
							first = next;
						}
						else
							++first;
					}

					// send the disconnection event to EGSs
					CEntityLocatorClientProxy::broadcast_connectionEvents(_ClientTracker.getTrackedModules().begin(), _ClientTracker.getTrackedModules().end(),
						this, connections);		
				}
				// remove any connected user
				{
					TUserMap::iterator first(_ConnectedUsers.begin()), last(_ConnectedUsers.end());
					while ( first != last )
					{
						if (first->second == shardId)
						{
							TUserMap::iterator next = first;
							++next;
							_ConnectedUsers.erase(first);
							// continue on next
							first = next;
						}
						else
							++first;
					}
				}
				
				// remove the client info
				nlassert(_LocatorIndex.find(it->second) != _LocatorIndex.end());
				_LocatorIndex.erase(it->second);
				_LocatorClients.erase(it);
			}
		}

		void onModuleUpdate()
		{
			// update the Total concurent user varialb
			TotalConcurentUser.set((uint32)_ConnectedUsers.size());
		}

		/////////////////////////////////////////////////////////////
		///////// IModuleTrackerCb virtuals implementations
		/////////////////////////////////////////////////////////////

		virtual void onTrackedModuleUp(IModuleProxy *moduleProxy)
		{
			nldebug("ENTLOC: connection of tracked module %s, sending %u connected chars",
				moduleProxy->getModuleName().c_str(),
				_ConnectedChars.size());
			// send the full online character to the client
			CEntityLocatorClientProxy elc(moduleProxy);

			vector<TCharConnectionEvent> connections;
			connections.reserve(_ConnectedChars.size());
			TCharMap::iterator first(_ConnectedChars.begin()), last(_ConnectedChars.end());
			for (; first != last; ++first)
			{
				TCharConnectionEvent	cce;
				cce.setCharId(first->first);
				// always true because we send the initial set of connection character
				cce.setConnection(true);
				connections.push_back(cce);
			}

			// send the initial set of connected char
			elc.connectionEvents(this, connections);
		}

		virtual void onTrackedModuleDown(IModuleProxy *moduleProxy)
		{
			// nothing for now
		}

		/////////////////////////////////////////////////////////////
		///////// IEntityLocator virtuals implementations
		/////////////////////////////////////////////////////////////

		/// Check if a user is online somewhere
		virtual bool isUserOnline(uint32 userId)
		{
			return _ConnectedUsers.find(userId) != _ConnectedUsers.end();
		}

		IModuleProxy *getLocatorModuleForChar(uint32 charId)
		{
			TCharMap::iterator it(_ConnectedChars.find(charId));

			if (it == _ConnectedChars.end())
				return NULL;

			// ok the character is online, retrieve the locator module
			TLocatorIndex::iterator locIt(_LocatorIndex.find(it->second.ShardId));

			nlassert(locIt != _LocatorIndex.end());

			return locIt->second;
		}

		NLNET::IModuleProxy *getLocatorModuleForChar(const ucstring &charName)
		{
			TCharNameMap::iterator it(_ConnectedCharsByName.find(toLower(charName)));
			if (it == _ConnectedCharsByName.end())
			{
				// not online
				return NULL;
			}

			// ok the character is online, retrieve the locator module
			TLocatorIndex::iterator locIt(_LocatorIndex.find(it->second));

			nlassert(locIt != _LocatorIndex.end());

			return locIt->second;
		}

		uint32 getShardIdForChar(const ucstring &charName)
		{
			TCharNameMap::iterator it(_ConnectedCharsByName.find(toLower(charName)));
			if (it == _ConnectedCharsByName.end())
			{
				// not online
				return 0;
			}

			// online
			uint32 charId = it->second;
			TCharMap::iterator it2(_ConnectedChars.find(charId));
			BOMB_IF(it2 == _ConnectedChars.end(), "Char name online, but not in connected char map !", return 0);

			return it2->second.ShardId;
		}

		/** Return the module for a given shard id
		 *	return NULL if no module available for the specified shard id
		*/
		NLNET::IModuleProxy *getLocatorModuleForShard(uint32 shardId)
		{
			TLocatorIndex::iterator it(_LocatorIndex.find(shardId));
			if (it == _LocatorIndex.end())
				return NULL;

			return it->second;
		}


		/////////////////////////////////////////////////////////////
		///////// ICharacterSyncCb virtuals implementations
		/////////////////////////////////////////////////////////////

		/** Callback called when the name of a character have been changed */
		void onCharacterNameUpdated(uint32 charId, const std::string &oldName, const std::string &newName)
		{
			// lookup this char in online players map
			TCharNameMap::iterator it(_ConnectedCharsByName.find(toLower(oldName)));

			if (it != _ConnectedCharsByName.end())
			{
				// this character is online, update the container
				nlassert(charId == it->second);

				_ConnectedCharsByName.erase(it);
				_ConnectedCharsByName.insert(make_pair(toLower(newName), charId));
			}
		}

		/** Callback called when a character is deleted/removed */
		virtual void onBeforeCharacterDelete(uint32 charId)
		{
			// nothing
		}

		/////////////////////////////////////////////////////////////
		///////// EntityLocatorSkel virtuals implementations
		/////////////////////////////////////////////////////////////

		// The locator client send the initial state of active player and character connections
		void initState(NLNET::IModuleProxy *sender, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars)
		{
			nldebug("ENTLOC : initState : receive initial state from module '%s' with %u players and %u characters connected", 
				sender->getModuleName().c_str(),
				connectedUsers.size(),
				connectedChars.size());

			for (uint i=0; i<connectedUsers.size(); ++i)
			{
				playerConnected(sender, connectedUsers[i]);
			}

			for (uint i=0; i<connectedChars.size(); ++i)
			{
				charConnected(sender, connectedChars[i].getCharEId(), connectedChars[i].getLastDisconnectionDate());
			}
		}


		// A player has connected on a shard
		void playerConnected(NLNET::IModuleProxy *sender, uint32 userId)
		{
			nldebug("ENTLOC : playerConnected : player %u connected from '%s'", userId, sender->getModuleName().c_str());

			// look for the corresponding shard id
			TLocatorClient::iterator locIt(_LocatorClients.find(sender));
			if (locIt == _LocatorClients.end())
			{
				nlwarning("ENTLOC : playerConnected : receive message from not recognized module '%s', ignoring", sender->getModuleName().c_str());
				return;
			}

			TShardId shardId = locIt->second;

			TUserMap::iterator it(_ConnectedUsers.find(userId));
			if (it != _ConnectedUsers.end())
			{
				nlwarning("ENTLOC : playerConnected : the user %u is already connected to shard %u, changing it anyway",
					it->first, it->second);

				it->second = shardId;
			}
			else
			{
				_ConnectedUsers.insert(make_pair(userId, shardId));
			}

			// send user event to cb interfaces
			NLMISC_BROADCAST_TO_LISTENER(ICharacterEventCb, onUserConnection(sender, userId));
		}
		// A player has disconnected from a shard
		void playerDisconnected(NLNET::IModuleProxy *sender, uint32 userId)
		{
			nldebug("ENTLOC : playerDisconnected : player %u disconnected from '%s'", userId, sender->getModuleName().c_str());

			// look for the corresponding shard id
			TLocatorClient::iterator locIt(_LocatorClients.find(sender));
			if (locIt == _LocatorClients.end())
			{
				nlwarning("ENTLOC : playerDisconnected : receive message from not recognized module '%s', ignoring", sender->getModuleName().c_str());
				return;
			}

			TShardId shardId = locIt->second;


			TUserMap::iterator it(_ConnectedUsers.find(userId));
			if (it == _ConnectedUsers.end())
			{
				nlwarning("ENTLOC : playerDisconnected : the user %u is not connected to shard %u",
					userId, locIt->second);
			}
			else
			{
				if (LSIgnoreBadDisconnection && it->second != shardId)
				{
					nlwarning("ENTLOC : playerDisconnected : the user %u is connected to shard %u, ignoring disconnection from shard %u",
						it->first, 
						it->second,
						shardId);

					return;
				}
				// erase the information
				_ConnectedUsers.erase(it);
				// send user event to cb interfaces
				NLMISC_BROADCAST_TO_LISTENER(ICharacterEventCb, onUserDisconnection(sender, userId));
			}

		}
		// A character has connected
		void charConnected(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate)
		{
			TCharId charId = (TCharId)charEId.getShortId();

			nldebug("ENTLOC : charConnected : character %u connected from '%s'", charId, sender->getModuleName().c_str());

			// look for the corresponding shard id
			TLocatorClient::iterator locIt(_LocatorClients.find(sender));
			if (locIt == _LocatorClients.end())
			{
				nlwarning("ENTLOC : charConnected : receive message from not recognized module '%s', ignoring", sender->getModuleName().c_str());
				return;
			}

			TShardId shardId = locIt->second;

			_charConnected(sender, charId, lastDisconnectionDate, shardId);
		}

		void _charConnected(NLNET::IModuleProxy *sender, uint32 charId, uint32 lastDisconnectionDate, uint32 shardId)
		{
			uint32 lastConnectionDate = CTime::getSecondsSince1970();

			TCharMap::iterator it(_ConnectedChars.find(charId));
			if (it != _ConnectedChars.end())
			{
				nlwarning("ENTLOC : charConnected : the character %u is already connected to shard %u, changing it anyway",
					it->first, it->second.ShardId);

				it->second.ShardId = shardId;
			}
			else
			{
				// store the new information
				TCharInfo ci;
				ci.ShardId = shardId;
				// load the character info
				CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
				if (character !=NULL)
				{
					string fullName = CShardNames::getInstance().makeFullName(character->getCharName(), (TSessionId)character->getHomeMainlandSessionId());


					ci.CharName = toLower(fullName);

					// update the last played date
					character->setLastPlayedDate(lastConnectionDate);
					character->update(_RingDB);
				}

				// store the new information
				_ConnectedChars.insert(make_pair(charId, ci));

				// store char by name info
				_ConnectedCharsByName.insert(make_pair(toLower(ci.CharName), charId));
			}

			// load the nel user to read privileges
			string privilege;
			CNelUserPtr nelUser = CNelUser::load(_NelDb, charId>>4, __FILE__, __LINE__);
			STOP_IF(nelUser == NULL, "Failed to load nel user "<<(charId>>4)<<" from database");
			if (nelUser != NULL)
			{
				privilege = nelUser->getPrivilege();
			}

			// send character event to cb interfaces
			NLMISC_BROADCAST_TO_LISTENER(ICharacterEventCb, onCharacterConnection(sender, charId, lastDisconnectionDate));

			// send the connection event to EGS
			vector<TCharConnectionEvent> connections;
			TCharConnectionEvent cce;
			cce.setCharId(charId);
			cce.setConnection(true);
			cce.setPrivilege(privilege);
			cce.setlastConnectionDate(lastConnectionDate);
			connections.push_back(cce);

			CEntityLocatorClientProxy::broadcast_connectionEvents(_ClientTracker.getTrackedModules().begin(), _ClientTracker.getTrackedModules().end(),
				this, connections);
		}
		// A character has disconnected
		void charDisconnected(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId)
		{
			TCharId charId = (TCharId)charEId.getShortId();
			nldebug("ENTLOC : charDisconnected : character %u disconnected from '%s'", charId, sender->getModuleName().c_str());

			// look for the corresponding shard id
			TLocatorClient::iterator locIt(_LocatorClients.find(sender));
			if (locIt == _LocatorClients.end())
			{
				nlwarning("ENTLOC : charDisconnected : receive message from not recognized module '%s', ignoring", sender->getModuleName().c_str());
				return;
			}

			// load the character info
			CCharacterPtr character = CCharacter::load(_RingDB, charId, __FILE__, __LINE__);
			if (character !=NULL)
			{
				// update the last played date
				character->setLastPlayedDate(0);
				character->update(_RingDB);
			}

			TShardId shardId = locIt->second;

			_charDisconnected(sender, charId, shardId);
		}

		void _charDisconnected(NLNET::IModuleProxy *sender, uint32 charId, uint32 shardId)
		{
			TCharMap::iterator it(_ConnectedChars.find(charId));
			if (it == _ConnectedChars.end())
			{
				nlwarning("ENTLOC : charDisconnected : the character %u is not connected to shard %u",
					charId, shardId);
			}
			else
			{
				// erase the information
				_ConnectedCharsByName.erase(toLower(it->second.CharName));
				_ConnectedChars.erase(it);
			}

			// send character event to cb interfaces
			NLMISC_BROADCAST_TO_LISTENER(ICharacterEventCb, onCharacterDisconnection(sender, charId));

			// send the connection event to EGS
			vector<TCharConnectionEvent> connections;
			TCharConnectionEvent cce;
			cce.setCharId(charId);
			cce.setConnection(false);
			connections.push_back(cce);

			CEntityLocatorClientProxy::broadcast_connectionEvents(_ClientTracker.getTrackedModules().begin(), _ClientTracker.getTrackedModules().end(),
				this, connections);		
		}

		IModuleProxy *getLocalProxyForMe()
		{
			vector<IModuleSocket *> sockets;
			this->getPluggedSocketList(sockets);

			if (sockets.empty())
				return NULL;

			IModuleGateway *gw = IModuleManager::getInstance().getModuleGateway(sockets[0]->getSocketName());
			if (gw == NULL)
				return NULL;

			return gw->getPluggedModuleProxy(this);
		}


		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CEntityLocator, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, dump, "Dump the internal module state", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, listUsers, "List the connected user and their respective shard", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, listCharacters, "List the connected characters and their respective shard", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, simUserConn, "Simulate a user connection", "<uid>");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, simUserDisconn, "Simulate a user disconnection", "<uid>");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, simCharConn, "Simulate a character connection", "<charId>|<eid> <shardId>");
			NLMISC_COMMAND_HANDLER_ADD(CEntityLocator, simCharDisconn, "Simulate a character disconnection", "<charId>|<eid>");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(simCharDisconn)
		{
			if (args.size() != 2)
				return false;

			uint32 charId;
			if (args[0][0] == '(')
			{
				// we have an EID
				CEntityId eid(args[0].c_str());
				charId = uint32(eid.getShortId());
			}
			else
			{
				// parse a char id
				NLMISC::fromString(args[0], charId);
			}

			uint32 shardId;
			NLMISC::fromString(args[1], shardId);


			log.displayNL("Simulate character %u / %s disconnection from shard %u", charId, CEntityId(RYZOMID::player, charId).toString().c_str(), shardId);

//			IModuleProxy *proxy = getLocalProxyForMe();
//			if (proxy == NULL)
//			{
//				log.displayNL("Can't find local proxy for entity locator %s, can't execute command", this->getModuleFullyQualifiedName().c_str());
//				return true;
//			}

			_charDisconnected(NULL, charId, shardId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(simCharConn)
		{
			if (args.size() != 2)
				return false;

			uint32 charId;
			if (args[0][0] == '(')
			{
				// we have an EID
				CEntityId eid(args[0].c_str());
				charId = uint32(eid.getShortId());
			}
			else
			{
				// parse a char id
				NLMISC::fromString(args[0], charId);
			}

			uint32 shardId;
			NLMISC::fromString(args[1], shardId);


			log.displayNL("Simulate character %u / %s connection from shard %u", charId, CEntityId(RYZOMID::player, charId).toString().c_str(), shardId);

//			IModuleProxy *proxy = getLocalProxyForMe();
//			if (proxy == NULL)
//			{
//				log.displayNL("Can't find local proxy for entity locator %s, can't execute command", this->getModuleFullyQualifiedName().c_str());
//				return true;
//			}

			_charConnected(NULL, charId, CTime::getSecondsSince1970()-10, shardId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(simUserDisconn)
		{
			if (args.size() != 1)
				return false;

			uint32 userId;
			NLMISC::fromString(args[0], userId);

			log.displayNL("Simulate user %u disconnection", userId);

			IModuleProxy *proxy = getLocalProxyForMe();
			if (proxy == NULL)
			{
				log.displayNL("Can't find local proxy for entity locator %s, can't execute command", this->getModuleFullyQualifiedName().c_str());
				return true;
			}

			playerDisconnected(proxy, userId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(simUserConn)
		{
			if (args.size() != 1)
				return false;

			uint32 userId;
			NLMISC::fromString(args[0], userId);

			log.displayNL("Simulate user %u connection", userId);

			IModuleProxy *proxy = getLocalProxyForMe();
			if (proxy == NULL)
			{
				log.displayNL("Can't find local proxy for entity locator %s, can't execute command", this->getModuleFullyQualifiedName().c_str());
				return true;
			}

			playerConnected(proxy, userId);

			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("-----------------------------");
			log.displayNL("Dumping EntityLocator state :");
			log.displayNL("-----------------------------");

			log.displayNL(" There are currently %u locator clients :", _LocatorClients.size());
			TLocatorClient::iterator first(_LocatorClients.begin()), last(_LocatorClients.end());
			for (; first != last; ++first)
			{
				IModuleProxy *proxy = first->first;
				TShardId shardId = first->second;

				log.displayNL("  + Module '%s' client for shard %u", proxy->getModuleName().c_str(), shardId);
			}

			log.displayNL(" There are %u connected users and %u connected characters", _ConnectedUsers.size(), _ConnectedChars.size());
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(listUsers)
		{
			log.displayNL("Listing %u connected users :", _ConnectedUsers.size());
			TUserMap::iterator first(_ConnectedUsers.begin()), last(_ConnectedUsers.end());
			for (; first != last; ++first)
			{
				log.displayNL("  User %u '%s' connected on shard %u", 
					first->first, 
					ICharacterSync::getInstance()->getUserName(first->first).c_str(),
					first->second);
			}
			return true;
		}

		NLMISC_CLASS_COMMAND_DECL(listCharacters)
		{
			log.displayNL("Listing %u connected characters :", _ConnectedChars.size());
			TCharMap::iterator first(_ConnectedChars.begin()), last(_ConnectedChars.end());
			for (; first != last; ++first)
			{
				log.displayNL("  Character %u '%s' [%u:%u] connected on shard %u", 
					first->first, 
					first->second.CharName.toUtf8().c_str(),
					first->first>>4, 
					first->first&0xf, 
					first->second.ShardId);
			}
			return true;
		}

	};

	NLNET_REGISTER_MODULE_FACTORY(CEntityLocator, "EntityLocator");



	

} // namespace ENTITYLOC

// force module linking
void forceMailForumFwdLink()
{
}
