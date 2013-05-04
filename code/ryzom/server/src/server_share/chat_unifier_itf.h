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

#ifndef CHAT_UNIFIER_ITF
#define CHAT_UNIFIER_ITF
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
	
namespace CHATUNI
{
	
	// The player locator is not available, could'nt do the job
	// No IOS module for the addresse hosting shard
	// No character synchronizer to retreive sender name
	// The sender character is unknown
	// The addressee character is unknown
	// The addressee character is offline


	struct TFailInfo
	{
		enum TValues
		{
			fi_no_entity_locator,
			fi_no_ios_module,
			fi_no_char_sync,
			fi_sender_char_unknown,
			fi_dest_char_unknown,
			fi_char_offline,
			/// the highest valid value in the enum
			last_enum_item = fi_char_offline,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 6
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(fi_no_entity_locator, 0));
				indexTable.insert(std::make_pair(fi_no_ios_module, 1));
				indexTable.insert(std::make_pair(fi_no_char_sync, 2));
				indexTable.insert(std::make_pair(fi_sender_char_unknown, 3));
				indexTable.insert(std::make_pair(fi_dest_char_unknown, 4));
				indexTable.insert(std::make_pair(fi_char_offline, 5));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_no_entity_locator)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_no_ios_module)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_no_char_sync)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_sender_char_unknown)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_dest_char_unknown)
				NL_STRING_CONVERSION_TABLE_ENTRY(fi_char_offline)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TFailInfo()
			: _Value(invalid_val)
		{
		}
		TFailInfo(TValues value)
			: _Value(value)
		{
		}

		TFailInfo(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TFailInfo &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TFailInfo &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TFailInfo &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TFailInfo &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TFailInfo &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TFailInfo &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CChatUnifierSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CChatUnifierSkel>	TInterceptor;
	protected:
		CChatUnifierSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CChatUnifierSkel()
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

		typedef void (CChatUnifierSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void sendFarTell_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CChatUnifierSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// IOS forward a tell message to the unifier
		// If IOS can't find the player localy, it forward
		// the tell to the unifier
		virtual void sendFarTell(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CChatUnifierProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CChatUnifierSkel	*_LocalModuleSkel;


	public:
		CChatUnifierProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CChatUnifierSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CChatUnifierProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// IOS forward a tell message to the unifier
		// If IOS can't find the player localy, it forward
		// the tell to the unifier
		void sendFarTell(NLNET::IModule *sender, const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_sendFarTell(NLNET::CMessage &__message, const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CChatUnifierClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CChatUnifierClientSkel>	TInterceptor;
	protected:
		CChatUnifierClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CChatUnifierClientSkel()
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

		typedef void (CChatUnifierClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void recvFarTellFail_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void recvFarTell_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void farGuildChat_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void farGuildChat2_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void farGuildChat2Ex_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void universeBroadcast_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void dynChanBroadcast_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void recvBroadcastMessage_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CChatUnifierClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// SU send a far tell failure to IOS. This mean that the player is offline or unknow
		virtual void recvFarTellFail(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, const ucstring &destName, TFailInfo failInfo) =0;
		// SU send a far tell to the IOS hosting the addresse character
		virtual void recvFarTell(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring &destName, const ucstring &text) =0;
		// IOS forward a guild chat message to the IOS
		virtual void farGuildChat(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, const ucstring &text) =0;
		// IOS forward a guild chat message to the IOS
		virtual void farGuildChat2(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, const ucstring &phraseName) =0;
		// IOS forward a guild chat message to the IOS
		virtual void farGuildChat2Ex(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, uint32 phraseId) =0;
		// IOS forward a univers chat message to the IOSs
		virtual void universeBroadcast(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 senderHomeSession, const ucstring &text) =0;
		// IOS forward a dyn chat chat message to the IOSs
		virtual void dynChanBroadcast(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &chanId, const ucstring &senderName, const ucstring &text) =0;
		// SU send a broadcast message to the IOS
		virtual void recvBroadcastMessage(NLNET::IModuleProxy *sender, const ucstring &message) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CChatUnifierClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CChatUnifierClientSkel	*_LocalModuleSkel;


	public:
		CChatUnifierClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CChatUnifierClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CChatUnifierClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// SU send a far tell failure to IOS. This mean that the player is offline or unknow
		void recvFarTellFail(NLNET::IModule *sender, const NLMISC::CEntityId &senderCharId, const ucstring &destName, TFailInfo failInfo);
		// SU send a far tell to the IOS hosting the addresse character
		void recvFarTell(NLNET::IModule *sender, const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring &destName, const ucstring &text);
		// IOS forward a guild chat message to the IOS
		void farGuildChat(NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, const ucstring &text);
		// IOS forward a guild chat message to the IOS
		void farGuildChat2(NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, const ucstring &phraseName);
		// IOS forward a guild chat message to the IOS
		void farGuildChat2Ex(NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, uint32 phraseId);
		// IOS forward a univers chat message to the IOSs
		void universeBroadcast(NLNET::IModule *sender, const ucstring &senderName, uint32 senderHomeSession, const ucstring &text);
		// IOS forward a dyn chat chat message to the IOSs
		void dynChanBroadcast(NLNET::IModule *sender, const NLMISC::CEntityId &chanId, const ucstring &senderName, const ucstring &text);
		// SU send a broadcast message to the IOS
		void recvBroadcastMessage(NLNET::IModule *sender, const ucstring &message);
		// IOS forward a guild chat message to the IOS

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_farGuildChat(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, const ucstring &text)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_farGuildChat(message , senderName, guildId, text);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// IOS forward a guild chat message to the IOS

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_farGuildChat2(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, const ucstring &phraseName)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_farGuildChat2(message , senderName, guildId, phraseName);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// IOS forward a guild chat message to the IOS

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_farGuildChat2Ex(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const ucstring &senderName, uint32 guildId, uint32 phraseId)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_farGuildChat2Ex(message , senderName, guildId, phraseId);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// IOS forward a univers chat message to the IOSs

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_universeBroadcast(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const ucstring &senderName, uint32 senderHomeSession, const ucstring &text)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_universeBroadcast(message , senderName, senderHomeSession, text);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// IOS forward a dyn chat chat message to the IOSs

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_dynChanBroadcast(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const NLMISC::CEntityId &chanId, const ucstring &senderName, const ucstring &text)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_dynChanBroadcast(message , chanId, senderName, text);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_recvFarTellFail(NLNET::CMessage &__message, const NLMISC::CEntityId &senderCharId, const ucstring &destName, TFailInfo failInfo);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_recvFarTell(NLNET::CMessage &__message, const NLMISC::CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring &destName, const ucstring &text);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_farGuildChat(NLNET::CMessage &__message, const ucstring &senderName, uint32 guildId, const ucstring &text);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_farGuildChat2(NLNET::CMessage &__message, const ucstring &senderName, uint32 guildId, const ucstring &phraseName);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_farGuildChat2Ex(NLNET::CMessage &__message, const ucstring &senderName, uint32 guildId, uint32 phraseId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_universeBroadcast(NLNET::CMessage &__message, const ucstring &senderName, uint32 senderHomeSession, const ucstring &text);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_dynChanBroadcast(NLNET::CMessage &__message, const NLMISC::CEntityId &chanId, const ucstring &senderName, const ucstring &text);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_recvBroadcastMessage(NLNET::CMessage &__message, const ucstring &message);
	



	};

}
	
#endif
