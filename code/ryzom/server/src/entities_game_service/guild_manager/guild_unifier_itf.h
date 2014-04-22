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

#ifndef GUILD_UNIFIER_ITF
#define GUILD_UNIFIER_ITF
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
	
#include "game_share/people_pd.h"
	
#include "game_share/pvp_clan.h"
	
#include "game_share/string_manager_sender.h"
	
#include "egs_pd.h"
	
namespace GU
{
	
	class CGuildMemberDesc;

	class CFameEntryDesc;

	class CGuildDesc;

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildMemberDesc
	{
	protected:
		// 
		NLMISC::CEntityId	_MemberId;
		// 
		NLMISC::TGameCycle	_MemberEnterTime;
		// 
		EGSPD::CGuildGrade::TGuildGrade	_MemberGrade;
	public:
		// 
		const NLMISC::CEntityId &getMemberId() const
		{
			return _MemberId;
		}

		NLMISC::CEntityId &getMemberId()
		{
			return _MemberId;
		}


		void setMemberId(const NLMISC::CEntityId &value)
		{


				_MemberId = value;

				
		}
			// 
		NLMISC::TGameCycle getMemberEnterTime() const
		{
			return _MemberEnterTime;
		}

		void setMemberEnterTime(NLMISC::TGameCycle value)
		{

				_MemberEnterTime = value;

		}
			// 
		EGSPD::CGuildGrade::TGuildGrade getMemberGrade() const
		{
			return _MemberGrade;
		}

		void setMemberGrade(EGSPD::CGuildGrade::TGuildGrade value)
		{

				_MemberGrade = value;

		}
	
		bool operator == (const CGuildMemberDesc &other) const
		{
			return _MemberId == other._MemberId
				&& _MemberEnterTime == other._MemberEnterTime
				&& _MemberGrade == other._MemberGrade;
		}


		// constructor
		CGuildMemberDesc()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_MemberId);
			s.serial(_MemberEnterTime);
			s.serialEnum(_MemberGrade);

		}
		

	private:
	

	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFameEntryDesc
	{
	protected:
		// 
		NLMISC::CSheetId	_FactionSheetId;
		// 
		sint32	_FameValue;
		// 
		sint32	_FameMemory;
		// 
		EGSPD::CFameTrend::TFameTrend	_FameTrend;
	public:
		// 
		NLMISC::CSheetId getFactionSheetId() const
		{
			return _FactionSheetId;
		}

		void setFactionSheetId(NLMISC::CSheetId value)
		{

				_FactionSheetId = value;

		}
			// 
		sint32 getFameValue() const
		{
			return _FameValue;
		}

		void setFameValue(sint32 value)
		{

				_FameValue = value;

		}
			// 
		sint32 getFameMemory() const
		{
			return _FameMemory;
		}

		void setFameMemory(sint32 value)
		{

				_FameMemory = value;

		}
			// 
		EGSPD::CFameTrend::TFameTrend getFameTrend() const
		{
			return _FameTrend;
		}

		void setFameTrend(EGSPD::CFameTrend::TFameTrend value)
		{

				_FameTrend = value;

		}
	
		bool operator == (const CFameEntryDesc &other) const
		{
			return _FactionSheetId == other._FactionSheetId
				&& _FameValue == other._FameValue
				&& _FameMemory == other._FameMemory
				&& _FameTrend == other._FameTrend;
		}


		// constructor
		CFameEntryDesc()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_FactionSheetId);
			s.serial(_FameValue);
			s.serial(_FameMemory);
			s.serialEnum(_FameTrend);

		}
		

	private:
	

	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildDesc
	{
	protected:
		// 
		uint32	_GuildId;
		// 
		ucstring	_GuildName;
		// 
		ucstring	_GuildDesc;
		// 
		uint64	_GuildMoney;
		// 
		uint32	_GuildCreationDate;
		// 
		EGSPD::CPeople::TPeople	_GuildRace;
		// 
		uint64	_GuildIcon;
		// 
		PVP_CLAN::TPVPClan	_DeclaredCult;
		// 
		PVP_CLAN::TPVPClan	_DeclaredCiv;
		// 
		std::vector < CFameEntryDesc >	_Fames;
		// 
		std::vector < CGuildMemberDesc >	_Members;
	public:
		// 
		uint32 getGuildId() const
		{
			return _GuildId;
		}

		void setGuildId(uint32 value)
		{

				_GuildId = value;

		}
			// 
		const ucstring &getGuildName() const
		{
			return _GuildName;
		}

		ucstring &getGuildName()
		{
			return _GuildName;
		}


		void setGuildName(const ucstring &value)
		{


				_GuildName = value;

				
		}
			// 
		const ucstring &getGuildDesc() const
		{
			return _GuildDesc;
		}

		ucstring &getGuildDesc()
		{
			return _GuildDesc;
		}


		void setGuildDesc(const ucstring &value)
		{
				_GuildDesc = value;
		}
			// 
		uint64 getGuildMoney() const
		{
			return _GuildMoney;
		}

		void setGuildMoney(uint64 value)
		{
				_GuildMoney = value;
		}
			// 
		uint32 getGuildCreationDate() const
		{
			return _GuildCreationDate;
		}

		void setGuildCreationDate(uint32 value)
		{

				_GuildCreationDate = value;

		}
			// 
		EGSPD::CPeople::TPeople getGuildRace() const
		{
			return _GuildRace;
		}

		void setGuildRace(EGSPD::CPeople::TPeople value)
		{

				_GuildRace = value;

		}
			// 
		uint64 getGuildIcon() const
		{
			return _GuildIcon;
		}

		void setGuildIcon(uint64 value)
		{

				_GuildIcon = value;

		}
			// 
		PVP_CLAN::TPVPClan getDeclaredCult() const
		{
			return _DeclaredCult;
		}

		void setDeclaredCult(PVP_CLAN::TPVPClan value)
		{

				_DeclaredCult = value;

		}
			// 
		PVP_CLAN::TPVPClan getDeclaredCiv() const
		{
			return _DeclaredCiv;
		}

		void setDeclaredCiv(PVP_CLAN::TPVPClan value)
		{

				_DeclaredCiv = value;

		}
			// 
		const std::vector < CFameEntryDesc > &getFames() const
		{
			return _Fames;
		}

		std::vector < CFameEntryDesc > &getFames()
		{
			return _Fames;
		}


		void setFames(const std::vector < CFameEntryDesc > &value)
		{


				_Fames = value;

				
		}
			// 
		const std::vector < CGuildMemberDesc > &getMembers() const
		{
			return _Members;
		}

		std::vector < CGuildMemberDesc > &getMembers()
		{
			return _Members;
		}


		void setMembers(const std::vector < CGuildMemberDesc > &value)
		{


				_Members = value;

				
		}
	
		bool operator == (const CGuildDesc &other) const
		{
			return _GuildId == other._GuildId
				&& _GuildName == other._GuildName
				&& _GuildDesc == other._GuildDesc
				&& _GuildMoney == other._GuildMoney
				&& _GuildCreationDate == other._GuildCreationDate
				&& _GuildRace == other._GuildRace
				&& _GuildIcon == other._GuildIcon
				&& _DeclaredCult == other._DeclaredCult
				&& _DeclaredCiv == other._DeclaredCiv
				&& _Fames == other._Fames
				&& _Members == other._Members;
		}


		// constructor
		CGuildDesc()
		{

		}
		
		void serial(NLMISC::IStream &s)
		{
			s.serial(_GuildId);
			s.serial(_GuildName);
			s.serial(_GuildDesc);
			s.serial(_GuildMoney);
			s.serial(_GuildCreationDate);
			s.serialEnum(_GuildRace);
			s.serial(_GuildIcon);
			s.serialEnum(_DeclaredCult);
			s.serialEnum(_DeclaredCiv);
			s.serialCont(_Fames);
			s.serialCont(_Members);

		}
		

	private:
	

	};


	
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildUnifierClientSkel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder < CGuildUnifierClientSkel>	TInterceptor;
	protected:
		CGuildUnifierClientSkel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~CGuildUnifierClientSkel()
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

		typedef void (CGuildUnifierClientSkel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &message);
		typedef std::map<std::string, TMessageHandler>	TMessageHandlerMap;

		const TMessageHandlerMap &getMessageHandlers() const;

		
		void guildReady_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void receiveForeignGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateMemberList_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateMemberInfo_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void updateGuild_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void guildDeleted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		void messageToGuildMembers_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message);

		// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder < CGuildUnifierClientSkel>;
	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

		// A client says to others clients that it is ready to send/receive guild data
		virtual void guildReady(NLNET::IModuleProxy *sender) =0;
		// The server send it local guilds to the client
		virtual void receiveForeignGuild(NLNET::IModuleProxy *sender, const std::vector < CGuildDesc > &guilds) =0;
		// The member list have changed, each guild unifier receive a copy of the new list
		virtual void updateMemberList(NLNET::IModuleProxy *sender, uint32 guildId, const std::vector < CGuildMemberDesc > &members) =0;
		// A member in the guild has changed, update it's info
		virtual void updateMemberInfo(NLNET::IModuleProxy *sender, uint32 guildId, const CGuildMemberDesc &membersInfo) =0;
		// The guild has been saved, the guild host send an update of the guild status (with fames, but no members)
		virtual void updateGuild(NLNET::IModuleProxy *sender, const CGuildDesc &guildInfo) =0;
		// A guild have been deleted
		virtual void guildDeleted(NLNET::IModuleProxy *sender, uint32 guildId) =0;
		// Send a message to all the guild members
		virtual void messageToGuildMembers(NLNET::IModuleProxy *sender, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params) =0;


	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildUnifierClientProxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		CGuildUnifierClientSkel	*_LocalModuleSkel;


	public:
		CGuildUnifierClientProxy(NLNET::IModuleProxy *proxy)
		{

			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				CGuildUnifierClientSkel::TInterceptor *interceptor = NULL;
				interceptor = static_cast < NLNET::CModuleBase* >(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~CGuildUnifierClientProxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

		// A client says to others clients that it is ready to send/receive guild data
		void guildReady(NLNET::IModule *sender);
		// The server send it local guilds to the client
		void receiveForeignGuild(NLNET::IModule *sender, const std::vector < CGuildDesc > &guilds);
		// The member list have changed, each guild unifier receive a copy of the new list
		void updateMemberList(NLNET::IModule *sender, uint32 guildId, const std::vector < CGuildMemberDesc > &members);
		// A member in the guild has changed, update it's info
		void updateMemberInfo(NLNET::IModule *sender, uint32 guildId, const CGuildMemberDesc &membersInfo);
		// The guild has been saved, the guild host send an update of the guild status (with fames, but no members)
		void updateGuild(NLNET::IModule *sender, const CGuildDesc &guildInfo);
		// A guild have been deleted
		void guildDeleted(NLNET::IModule *sender, uint32 guildId);
		// Send a message to all the guild members
		void messageToGuildMembers(NLNET::IModule *sender, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params);
		// A client says to others clients that it is ready to send/receive guild data

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_guildReady(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_guildReady(message );

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The server send it local guilds to the client

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_receiveForeignGuild(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const std::vector < CGuildDesc > &guilds)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_receiveForeignGuild(message , guilds);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The member list have changed, each guild unifier receive a copy of the new list

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_updateMemberList(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 guildId, const std::vector < CGuildMemberDesc > &members)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_updateMemberList(message , guildId, members);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// A member in the guild has changed, update it's info

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_updateMemberInfo(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 guildId, const CGuildMemberDesc &membersInfo)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_updateMemberInfo(message , guildId, membersInfo);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// The guild has been saved, the guild host send an update of the guild status (with fames, but no members)

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_updateGuild(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, const CGuildDesc &guildInfo)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_updateGuild(message , guildInfo);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// A guild have been deleted

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_guildDeleted(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 guildId)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_guildDeleted(message , guildId);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}
		// Send a message to all the guild members

		// This is the broadcast version of the method.
		template < class ProxyIterator >
		static void broadcast_messageToGuildMembers(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params)
		{
			NLNET::CMessage message;
			
			// create the message to send to multiple dest
			buildMessageFor_messageToGuildMembers(message , guildId, messageName, params);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}

		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_guildReady(NLNET::CMessage &__message);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_receiveForeignGuild(NLNET::CMessage &__message, const std::vector < CGuildDesc > &guilds);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateMemberList(NLNET::CMessage &__message, uint32 guildId, const std::vector < CGuildMemberDesc > &members);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateMemberInfo(NLNET::CMessage &__message, uint32 guildId, const CGuildMemberDesc &membersInfo);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_updateGuild(NLNET::CMessage &__message, const CGuildDesc &guildInfo);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_guildDeleted(NLNET::CMessage &__message, uint32 guildId);
	
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &buildMessageFor_messageToGuildMembers(NLNET::CMessage &__message, uint32 guildId, const std::string &messageName, const TVectorParamCheck &params);
	



	};

}
	
#endif
