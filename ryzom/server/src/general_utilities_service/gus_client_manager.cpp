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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/entity_id.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"

#include "game_share/singleton_registry.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/mirror_prop_value.h"

#include "gus_client_manager.h"
#include "gus_mirror.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class CClientManagerImplementation
	//-----------------------------------------------------------------------------

	class CClientManagerImplementation
		: public CClientManager,
		public CGusMirror::IMirrorModuleCallback,
		public IServiceSingleton
	{
	private:
		// prohibit instantiation with private ctor
		CClientManagerImplementation();

		// Mirror callback overload
		void mirrorIsReady(CGusMirror *mirrorModule);
		void entityAdded(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex);
		void entityRemoved(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, const NLMISC::CEntityId *entityId);
		void propertyChanged(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, TPropertyIndex propIndex);

	public:
		// virtual dtor
		virtual ~CClientManagerImplementation();

		// get singleton instance
		static CClientManagerImplementation* getInstance();

		// Specialisation of CServiceSingleton
		void init();

	public:
		// CClientManager API
		void setConnectionCallback(TConnectionHandlerPtr callback);
		const NLMISC::CSString& getCharacterName(TClientId clientId) const;
		const NLMISC::CEntityId& getEntityId(TClientId clientId) const;

		TClientId getClientId(const NLMISC::CSString&) const;
		TClientId getClientId(const CCharacterId& id) const;
		TClientId getClientIdFromAccount(uint32 accountId) const;

		static void		cbRecvString( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId );


	private:
		TConnectionHandlerPtr	_ConnectionHandler;

		// a player information record
		struct TClientInfo
		{
			uint32		NameIndex;
			CSString	CharacterName;
			CEntityId	EntityId;
		};
		typedef map<TClientId, TClientInfo>	TClientCont;
		// Storage for preconnected player, ie, thoose for witch we dont have the name ready
		TClientCont		_PreconClient;
		// Storage for connected player (those with theire name ready)
		TClientCont		_ConnectedClient;

		// known character names
		typedef std::map<CSString,CEntityId> TNameToEidMap;
		TNameToEidMap _NameToEidMap;

		// connected characters by character id
		typedef std::map<CCharacterId,TClientId> TCharIdToClientIdMap;
		TCharIdToClientIdMap _CharIdToClientIdMap;

		// To speedup thinks
		TPropertyIndex			_NamePropIndex;

	};

	//-----------------------------------------------------------------------------
	// NeL service callbacks 
	//-----------------------------------------------------------------------------
	TUnifiedCallbackItem ClientMgrCbArray[]=
	{
		{	"RECV_STRING",					CClientManagerImplementation::cbRecvString}
	};


	//-----------------------------------------------------------------------------
	// methods CClientManagerImplementation
	//-----------------------------------------------------------------------------

	CClientManagerImplementation::CClientManagerImplementation()
	{
	}

	CClientManagerImplementation::~CClientManagerImplementation()
	{
		CGusMirror::getInstance()->unregisterModuleCallback(this);
	}


	CClientManagerImplementation* CClientManagerImplementation::getInstance()
	{
		static CClientManagerImplementation* ptr=NULL;
		if (ptr==NULL)
			ptr= new CClientManagerImplementation;
		return ptr;
	}

	void CClientManagerImplementation::init()
	{
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(ClientMgrCbArray, sizeof(ClientMgrCbArray) / sizeof(TUnifiedCallbackItem));

		CGusMirror::getInstance()->registerModuleCallback(this);
	}

	void CClientManagerImplementation::mirrorIsReady(CGusMirror *mirrorModule)
	{
		// Pre store the name index property index
		CMirroredDataSet *dataSet = mirrorModule->getDataSet("fe_temp");

		nlassert(dataSet != NULL);

		_NamePropIndex = dataSet->getPropertyIndex("NameIndex");
	}

	void CClientManagerImplementation::entityAdded(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex)
	{
		CEntityId eid = dataSet->getEntityId(entityIndex);

		if (eid.getType() == RYZOMID::player)
		{
			// Ok, the new entity is a player, store it
			TClientInfo ci;

			ci.EntityId = eid;
			ci.NameIndex = 0;
			_PreconClient.insert(make_pair(const_cast<TDataSetRow&>(entityIndex), ci));
		}
	}

	void CClientManagerImplementation::entityRemoved(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, const NLMISC::CEntityId *entityId)
	{
		CEntityId eid = dataSet->getEntityId(entityIndex);

		if (eid.getType() == RYZOMID::player)
		{
			// This is a player, remove it from the containers
			if (_PreconClient.find(entityIndex) != _PreconClient.end())
			{
				_PreconClient.erase(entityIndex);
			}
			else if (_ConnectedClient.find(entityIndex) != _ConnectedClient.end())
			{
				_ConnectedClient.erase(entityIndex);
				_CharIdToClientIdMap.erase(eid);
				if (_ConnectionHandler != NULL)
					_ConnectionHandler->disconnect(entityIndex);
			}
		}
	}

	void CClientManagerImplementation::propertyChanged(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, TPropertyIndex propIndex)
	{
		CEntityId eid = dataSet->getEntityId(entityIndex);

		if (eid.getType() == RYZOMID::player)
		{
			// This is a player property
			if (propIndex == _NamePropIndex)
			{
				// The name have changed, we can read it and ask IOS for the string
				CMirrorPropValueRO<uint32> nameId(*dataSet, entityIndex, _NamePropIndex);							CMessage msgOut("GET_STRING");
				uint32 id = nameId;
				msgOut.serial(id);

				CUnifiedNetwork::getInstance()->send("IOS", msgOut);

				// Store the name
				if (_PreconClient.find(entityIndex) != _PreconClient.end())
				{
					_PreconClient[entityIndex].NameIndex = nameId;
				}
				else 
				{
					nlassert(_ConnectedClient.find(entityIndex) != _ConnectedClient.end());
					_ConnectedClient[entityIndex].NameIndex = nameId;
				}
			}
		}
	}

	void CClientManagerImplementation::cbRecvString( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
	{
		CClientManagerImplementation *cmi = CClientManagerImplementation::getInstance();
		// We receive response from IOS for the requested string
		uint32		nameIndex;
		ucstring	charName;

		msgin.serial(nameIndex);
		msgin.serial(charName);

		bool	found = false;
		// Look in the preconnected client
		TClientCont::iterator it(cmi->_PreconClient.begin()), last(cmi->_PreconClient.end());
		for (; it != last; ++it)
		{
			TClientInfo &ci = it->second;

			if (ci.NameIndex == nameIndex)
			{
				// Ok, we found the correct one
				ci.CharacterName = charName.toString();
				cmi->_NameToEidMap[ci.CharacterName]= ci.EntityId;
				cmi->_CharIdToClientIdMap[ci.EntityId]=it->first;
				found = true;

				// Move the client info to the other container
				cmi->_ConnectedClient[it->first] = ci;

				// Callback the client connected
				if (cmi->_ConnectionHandler != NULL)
					cmi->_ConnectionHandler->connect(it->first);

				cmi->_PreconClient.erase(it);

				break;
			}
		}
		if (!found)
		{
			// TODO : The player is already connected, should I update the name ? => YES!! of course!!
		}
	}

	
	void CClientManagerImplementation::setConnectionCallback(TConnectionHandlerPtr callback)
	{
		_ConnectionHandler = callback;
	}

	const NLMISC::CSString& CClientManagerImplementation::getCharacterName(TClientId clientId) const
	{
		// if no name was found then return an empty string
		static NLMISC::CSString emptyString;
		if (_ConnectedClient.find(clientId) != _ConnectedClient.end())
		{
			TClientCont::const_iterator it(_ConnectedClient.find(clientId));
			return it->second.CharacterName;
		}
		return emptyString;
	}

	const NLMISC::CEntityId& CClientManagerImplementation::getEntityId(TClientId clientId) const
	{
		static NLMISC::CEntityId emptyId;
		if (_ConnectedClient.find(clientId) != _ConnectedClient.end())
		{
			TClientCont::const_iterator it(_ConnectedClient.find(clientId));
			return it->second.EntityId;
		}
		return emptyId;
	}

	TClientId CClientManagerImplementation::getClientId(const NLMISC::CSString& name) const
	{
		// lookup the name in the name to id map
		TNameToEidMap::const_iterator it=_NameToEidMap.find(name);

		// if name not found return
		if (it==_NameToEidMap.end())
			return BadTClientId;

		// lookup the client id by entity id (character id)...
		return getClientId((*it).second);
	}

	TClientId CClientManagerImplementation::getClientId(const CCharacterId& id) const
	{
		TClientId result= BadTClientId;

		{	// lookup the client id in the CharIdToClientIdMap map - return BadTClientId if not found
			TCharIdToClientIdMap::const_iterator it= _CharIdToClientIdMap.find(id);
			if (it==_CharIdToClientIdMap.end())
				return BadTClientId;
			result= it->second;
		}

		{	// lookup the character id in the pre-connected client list
			TClientCont::const_iterator it;
			
			it=_PreconClient.find(result);
			if (it!=_PreconClient.end())
			{
				if (id!=CCharacterId(it->second.EntityId))
					return BadTClientId;
				return result;
			}
		}

		{	// lookup the character id in the connected client list
			TClientCont::const_iterator it;
			
			it=_ConnectedClient.find(result);
			if (it!=_ConnectedClient.end())
			{
				if (id!=CCharacterId(it->second.EntityId))
					return BadTClientId;
				return result;
			}
		}

		// the value of 'result' didn't correspond to an active record in either pre-connected or connected
		// player maps
		return BadTClientId;
	}

	TClientId CClientManagerImplementation::getClientIdFromAccount(uint32 accountId) const
	{
		// try all slot values for the account, looking for one for which there is a valid TClientId value
		for (uint32 i=0;i<16;++i)
		{
			TClientId id= getClientId(CCharacterId(accountId,i));
			if (id!=BadTClientId)
				return id;
		}

		// no match found so return 'bad id'
		return BadTClientId;
	}


	//-----------------------------------------------------------------------------
	// methods CClientManager
	//-----------------------------------------------------------------------------

	CClientManager* CClientManager::getInstance()
	{
		return CClientManagerImplementation::getInstance();
	}

	//-----------------------------------------------------------------------------
	// CClientManager instantiator
	//-----------------------------------------------------------------------------

	class CClientManagerImplementationInstantiator
	{
	public:
		CClientManagerImplementationInstantiator()
		{
			CClientManagerImplementation::getInstance();
		}
	};
	static CClientManagerImplementationInstantiator CClientManagerInstantiator;
	


}
//-----------------------------------------------------------------------------
