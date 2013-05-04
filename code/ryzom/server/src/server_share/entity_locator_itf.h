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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef ENTITY_LOCATOR_ITF
#define ENTITY_LOCATOR_ITF
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/entity_id.h"
	
namespace ENTITYLOC
{
	
	class TConnectedCharInfo;

	class TCharConnectionEvent;

	// Info about a connected character, used for block tranfert
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TConnectedCharInfo
	{
	protected:
		// The entity id the the character
		NLMISC::CEntityId	_CharEId;
		// the date of last disconnection of the character
		uint32	_LastDisconnectionDate;
	public:
		// The entity id the the character
		const NLMISC::CEntityId &getCharEId() const
		{
			return _CharEId;
		}

		NLMISC::CEntityId &getCharEId()
		{
			return _CharEId;
		}


		void setCharEId(const NLMISC::CEntityId &value)
		{


				_CharEId = value;

				
		}
			// the date of last disconnection of the character
		uint32 getLastDisconnectionDate() const
		{
			return _LastDisconnectionDate;
		}

		void setLastDisconnectionDate(uint32 value)
		{

				_LastDisconnectionDate = value;

		}
	
		bool operator == (const TConnectedCharInfo &other) const
		{
			return _CharEId == other._CharEId
				&& _LastDisconnectionDate == other._LastDisconnectionDate;
		}


		// constructor
		TConnectedCharInfo()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharEId);
			s.serial(_LastDisconnectionDate);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CEntityLocatorSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CEntityLocatorSkel>	TInterceptor;
	protected:
		CEntityLocatorSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CEntityLocatorSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CEntityLocatorSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void initState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void playerConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void playerDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void charConnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void charDisconnected_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CEntityLocatorSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The locator client send the initial state of active player and character connections
		virtual void initState(NLNET::IModuleProxy *sender, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars) =0;
		// A player has connected on a shard
		virtual void playerConnected(NLNET::IModuleProxy *sender, uint32 userId) =0;
		// A player has disconnected from a shard
		virtual void playerDisconnected(NLNET::IModuleProxy *sender, uint32 userId) =0;
		// A character has connected
		virtual void charConnected(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate) =0;
		// A character has disconnected
		virtual void charDisconnected(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &charEId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CEntityLocatorProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CEntityLocatorSkel	*_LocalModuleSkel;


	public:
		CEntityLocatorProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CEntityLocatorSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CEntityLocatorProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The locator client send the initial state of active player and character connections
		void initState(NLNET::IModule *sender, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars);
		// A player has connected on a shard
		void playerConnected(NLNET::IModule *sender, uint32 userId);
		// A player has disconnected from a shard
		void playerDisconnected(NLNET::IModule *sender, uint32 userId);
		// A character has connected
		void charConnected(NLNET::IModule *sender, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate);
		// A character has disconnected
		void charDisconnected(NLNET::IModule *sender, const NLMISC::CEntityId &charEId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_initState(NLNET::CMessage &__message, const std::vector < uint32 > &connectedUsers, const std::vector < TConnectedCharInfo > &connectedChars);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_playerConnected(NLNET::CMessage &__message, uint32 userId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_playerDisconnected(NLNET::CMessage &__message, uint32 userId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_charConnected(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId, uint32 lastDisconnectionDate);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_charDisconnected(NLNET::CMessage &__message, const NLMISC::CEntityId &charEId);
	



	};
	// Info dis/connection of a character
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharConnectionEvent
	{
	protected:
		// The character id of the character
		uint32	_CharId;
		// Type of the event : true for a connection, false otherwise
		bool	_Connection;
		// The privilege of the character (e.g :GM:DEV:)
		std::string	_Privilege;
		// Last Connection Date
		uint32	_lastConnectionDate;
	public:
		// The character id of the character
		uint32 getCharId() const
		{
			return _CharId;
		}

		void setCharId(uint32 value)
		{

				_CharId = value;

		}
			// Type of the event : true for a connection, false otherwise
		bool getConnection() const
		{
			return _Connection;
		}

		void setConnection(bool value)
		{

				_Connection = value;

		}
			// The privilege of the character (e.g :GM:DEV:)
		std::string getPrivilege() const
		{
			return _Privilege;
		}

		void setPrivilege(std::string value)
		{

				_Privilege = value;

		}
			// Last Connection Date
		uint32 getlastConnectionDate() const
		{
			return _lastConnectionDate;
		}

		void setlastConnectionDate(uint32 value)
		{

				_lastConnectionDate = value;

		}
	
		bool operator == (const TCharConnectionEvent &other) const
		{
			return _CharId == other._CharId
				&& _Connection == other._Connection
				&& _Privilege == other._Privilege
				&& _lastConnectionDate == other._lastConnectionDate;
		}


		// constructor
		TCharConnectionEvent()
		{

		}

		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharId);
			s.serial(_Connection);
			s.serial(_Privilege);
			s.serial(_lastConnectionDate);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CEntityLocatorClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CEntityLocatorClientSkel>	TInterceptor;
	protected:
		CEntityLocatorClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CEntityLocatorClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CEntityLocatorClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void connectionEvents_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CEntityLocatorClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// The entity locator send a list of connection event to EGS
		virtual void connectionEvents(NLNET::IModuleProxy *sender, const std::vector < TCharConnectionEvent > &events) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CEntityLocatorClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CEntityLocatorClientSkel	*_LocalModuleSkel;


	public:
		CEntityLocatorClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CEntityLocatorClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CEntityLocatorClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// The entity locator send a list of connection event to EGS
		void connectionEvents(NLNET::IModule *sender, const std::vector < TCharConnectionEvent > &events);
		// The entity locator send a list of connection event to EGS

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_connectionEvents(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::vector < TCharConnectionEvent > &events)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_connectionEvents(message , events);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_connectionEvents(NLNET::CMessage &__message, const std::vector < TCharConnectionEvent > &events);
	



	};

}
	
#endif
