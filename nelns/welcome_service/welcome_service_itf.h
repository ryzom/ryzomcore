
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef WELCOME_SERVICE_ITF
#define WELCOME_SERVICE_ITF
#include "nel/misc/types_nl.h"
#ifdef NL_COMP_VC8
  #include <memory>
#endif
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/net/login_cookie.h"
	
namespace WS
{
	


	struct TUserRole
	{
		enum TValues
		{
			ur_player,
			ur_editor,
			ur_animator,
			/// the highest valid value in the enum
			last_enum_item = ur_animator,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 3
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ur_player, 0));
				indexTable.insert(std::make_pair(ur_editor, 1));
				indexTable.insert(std::make_pair(ur_animator, 2));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_player)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_editor)
				NL_STRING_CONVERSION_TABLE_ENTRY(ur_animator)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TUserRole()
			: _Value(invalid_val)
		{
		}
		TUserRole(TValues value)
			: _Value(value)
		{
		}

		TUserRole(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TUserRole &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TUserRole &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TUserRole &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TUserRole &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TUserRole &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TUserRole &other) const
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
	class CWelcomeServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CWelcomeServiceSkel>	TInterceptor;
	protected:
		CWelcomeServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CWelcomeServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CWelcomeServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void welcomeUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void disconnectUser_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CWelcomeServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// ask the welcome service to welcome a character
		virtual void welcomeUser(NLNET::IModuleProxy *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId) =0;
		// ask the welcome service to disconnect a user
		virtual void disconnectUser(NLNET::IModuleProxy *sender, uint32 userId) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CWelcomeServiceSkel	*_LocalModuleSkel;


	public:
		CWelcomeServiceProxy(NLNET::IModuleProxy *proxy)
		{
			nlassert(proxy->getModuleClassName() == "WelcomeService");
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CWelcomeServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CWelcomeServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// ask the welcome service to welcome a character
		void welcomeUser(NLNET::IModule *sender, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId);
		// ask the welcome service to disconnect a user
		void disconnectUser(NLNET::IModule *sender, uint32 userId);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_welcomeUser(NLNET::CMessage &__message, uint32 charId, const std::string &userName, const NLNET::CLoginCookie &cookie, const std::string &priviledge, const std::string &exPriviledge, WS::TUserRole mode, uint32 instanceId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_disconnectUser(NLNET::CMessage &__message, uint32 userId);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CLoginServiceSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CLoginServiceSkel>	TInterceptor;
	protected:
		CLoginServiceSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CLoginServiceSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CLoginServiceSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void pendingUserLost_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CLoginServiceSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// An awaited user did not connect before the allowed timeout expire
		virtual void pendingUserLost(NLNET::IModuleProxy *sender, const NLNET::CLoginCookie &cookie) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CLoginServiceProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CLoginServiceSkel	*_LocalModuleSkel;


	public:
		CLoginServiceProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CLoginServiceSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CLoginServiceProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// An awaited user did not connect before the allowed timeout expire
		void pendingUserLost(NLNET::IModule *sender, const NLNET::CLoginCookie &cookie);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_pendingUserLost(NLNET::CMessage &__message, const NLNET::CLoginCookie &cookie);
	



	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CWelcomeServiceClientSkel>	TInterceptor;
	protected:
		CWelcomeServiceClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CWelcomeServiceClientSkel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}

		// unused interceptors 
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy * /* moduleProxy */)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy * /* moduleProxy */) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy * /* moduleProxy */) {}
	
		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
	private:

		typedef void (CWelcomeServiceClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void registerWS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void reportWSOpenState_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void welcomeUserResult_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateConnectedPlayerCount_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CWelcomeServiceClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
		virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline) =0;
		// WS report it's current open state
		virtual void reportWSOpenState(NLNET::IModuleProxy *sender, bool isOnline) =0;
		// return for welcome user
		virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg) =0;
		// transmits the current player counts
		virtual void updateConnectedPlayerCount(NLNET::IModuleProxy *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CWelcomeServiceClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CWelcomeServiceClientSkel	*_LocalModuleSkel;


	public:
		CWelcomeServiceClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CWelcomeServiceClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CWelcomeServiceClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// Register the welcome service in the ring session manager
		// The provided sessionId will be non-zero only for a shard with a fixed sessionId
		void registerWS(NLNET::IModule *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline);
		// WS report it's current open state
		void reportWSOpenState(NLNET::IModule *sender, bool isOnline);
		// return for welcome user
		void welcomeUserResult(NLNET::IModule *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg);
		// transmits the current player counts
		void updateConnectedPlayerCount(NLNET::IModule *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers);

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_registerWS(NLNET::CMessage &__message, uint32 shardId, uint32 fixedSessionId, bool isOnline);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_reportWSOpenState(NLNET::CMessage &__message, bool isOnline);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_welcomeUserResult(NLNET::CMessage &__message, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateConnectedPlayerCount(NLNET::CMessage &__message, uint32 nbOnlinePlayers, uint32 nbPendingPlayers);
	



	};

}
	
#endif
