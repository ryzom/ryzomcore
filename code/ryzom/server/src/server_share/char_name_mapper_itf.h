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

#ifndef CHAR_NAME_MAPPER_ITF
#define CHAR_NAME_MAPPER_ITF
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
	
namespace CNM
{
	
	class TCharNameInfo;

	class TCharMappedInfo;

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharNameInfo
	{
	protected:
		// 
		NLMISC::CEntityId	_CharEid;
		// 
		ucstring	_CharName;
	public:
		// 
		const NLMISC::CEntityId &getCharEid() const
		{
			return _CharEid;
		}

		NLMISC::CEntityId &getCharEid()
		{
			return _CharEid;
		}


		void setCharEid(const NLMISC::CEntityId &value)
		{


				_CharEid = value;

				
		}
			// 
		const ucstring &getCharName() const
		{
			return _CharName;
		}

		ucstring &getCharName()
		{
			return _CharName;
		}


		void setCharName(const ucstring &value)
		{


				_CharName = value;

				
		}
	
		bool operator == (const TCharNameInfo &other) const
		{
			return _CharEid == other._CharEid
				&& _CharName == other._CharName;
		}


		// constructor
		TCharNameInfo()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharEid);
			s.serial(_CharName);

		}
		

	private:
	

	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class TCharMappedInfo
	{
	protected:
		// 
		NLMISC::CEntityId	_CharEid;
		// 
		uint32	_StringId;
	public:
		// 
		const NLMISC::CEntityId &getCharEid() const
		{
			return _CharEid;
		}

		NLMISC::CEntityId &getCharEid()
		{
			return _CharEid;
		}


		void setCharEid(const NLMISC::CEntityId &value)
		{


				_CharEid = value;

				
		}
			// 
		uint32 getStringId() const
		{
			return _StringId;
		}

		void setStringId(uint32 value)
		{

				_StringId = value;

		}
	
		bool operator == (const TCharMappedInfo &other) const
		{
			return _CharEid == other._CharEid
				&& _StringId == other._StringId;
		}


		// constructor
		TCharMappedInfo()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_CharEid);
			s.serial(_StringId);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharNameMapperSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CCharNameMapperSkel>	TInterceptor;
	protected:
		CCharNameMapperSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CCharNameMapperSkel()
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

		typedef void (CCharNameMapperSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void mapCharNames_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CCharNameMapperSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void mapCharNames(NLNET::IModuleProxy *sender, const std::vector < TCharNameInfo > &charNameInfos) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharNameMapperProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CCharNameMapperSkel	*_LocalModuleSkel;


	public:
		CCharNameMapperProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CCharNameMapperSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CCharNameMapperProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void mapCharNames(NLNET::IModule *sender, const std::vector < TCharNameInfo > &charNameInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_mapCharNames(NLNET::CMessage &__message, const std::vector < TCharNameInfo > &charNameInfos);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharNameMapperClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CCharNameMapperClientSkel>	TInterceptor;
	protected:
		CCharNameMapperClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CCharNameMapperClientSkel()
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

		typedef void (CCharNameMapperClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void charNamesMapped_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CCharNameMapperClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// 
		virtual void charNamesMapped(NLNET::IModuleProxy *sender, const std::vector < TCharMappedInfo > &charMappedInfos) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharNameMapperClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CCharNameMapperClientSkel	*_LocalModuleSkel;


	public:
		CCharNameMapperClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CCharNameMapperClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CCharNameMapperClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// 
		void charNamesMapped(NLNET::IModule *sender, const std::vector < TCharMappedInfo > &charMappedInfos);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_charNamesMapped(NLNET::CMessage &__message, const std::vector < TCharMappedInfo > &charMappedInfos);
	



	};

}
	
#endif
