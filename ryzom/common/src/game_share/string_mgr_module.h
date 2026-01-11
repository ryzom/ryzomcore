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

#ifndef R2_STRING_MGR_MODULE_H
#define R2_STRING_MGR_MODULE_H

#include "stdpch.h"

#include "nel/misc/types_nl.h"

#include "nel/net/module.h"
#include "nel/net/module_socket.h"
#include "nel/net/message.h"

#include "game_share/small_string_manager.h"
#include "game_share/r2_messages.h"
#include "game_share/dms.h"

#include "game_share/base_types.h"
#include "game_share/chat_group.h"
#include "game_share/dyn_chat.h"

#include <map>
#include <vector>
namespace R2
{



	/*
	Used to manage the creation / Destruction of Dynamic channel channel.
	When we can speak as a npc 'In this case we open a channel' and when we write in this channel we simulate that the npc is speaking
	When a npc is controlled and he want to talk. Its message are forwarded to this channel
	*/
	class CStringManagerModule;

	struct CDsrHandler
	{
	public:
		CStringManagerModule* Module;
		NLNET::TModuleId ModuleId;
		virtual void execute(TDataSetRow& dsr)=0;
	};

	struct TIncarn{
	public:
		TChanID ChanId;
		TDataSetRow NpcId;
		void release();
	};

	struct ClientInfo{
	public:
		NLNET::IModuleProxy* Proxy;
		TSessionId SessionId;

		~ClientInfo();

		TDataSetRow getIncarnation(TChanID chanId);

		TChanID getIncarnation(TDataSetRow npcId);

		bool incarn(TDataSetRow npcId);

		void addIncarnation(TChanID chanId,TDataSetRow& npcId);

		//release all channels used by this client
		void clear();

		void removeIncarnation(TDataSetRow& npcId);

	private:

		std::vector<TIncarn> _Incarnations;
	};




	class CStringManagerModule : public NLNET::CModuleBase, public CStringTableManager
	{
	public:
		CStringManagerModule();

		~CStringManagerModule();

		void init(NLNET::IModuleSocket* clientGW,CDynamicMapService* server);

		virtual void onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId) { }

		virtual void onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId) {}

		virtual void onModuleUpdate() {}

		virtual void onApplicationExit();

		virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);

		virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);

		virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);

		virtual void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);

		virtual void onModuleSocketEvent(NLNET::IModuleSocket *moduleSocket, TModuleSocketEvent eventType) {}

		virtual void registerTableRequested(TSessionId sessionId,CObject* table);

		virtual void registerTableRequested(const NLNET::CMessage& msgin);

		virtual void registerTableRequested(TSessionId sessionId,std::vector<std::pair<std::string,std::string> >& entries);

		virtual void unregisterTableRequested(TSessionId sessionId);

		virtual void translateAndForward(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId);

		virtual void translateAndForwardWithArg(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId,std::vector<float>& val);

		virtual void talkAs(NLNET::TModuleId& id, TDataSetRow& creaturRowId,std::string& name,TSessionId sessionId);

		virtual void stopTalkAs(NLNET::TModuleId& id,TDataSetRow& npcId);

		virtual void forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence);

		virtual void sendTable(TSessionId sessionId,NLNET::IModuleProxy *moduleProxy);

		virtual void sendIdList(TSessionId sessionId,NLNET::IModuleProxy* senderModuleProxy);

		virtual void sendStringValue(TSessionId sessionId,NLNET::IModuleProxy* moduleProxy,std::string id);

		virtual void releaseChannels(TSessionId sessionId);


	private:
		static TChanID initChannel(std::string name="",bool forwardInput=true);

		virtual void send(TDataSetRow& senderId,CChatGroup::TGroupType groupType,const std::string& toSend);

		virtual void insertClient(NLNET::IModuleProxy *moduleProxy,TChanID channelId);

		virtual void removeClient(NLNET::IModuleProxy *moduleProxy);

		virtual void addAnimSession(NLNET::TModuleId id,TSessionId scenarioId);

		void requestDsr( ucstring& name);

	private:
		std::map<uint32,TChanID> _AnimChans;

		std::map<NLNET::TModuleId,ClientInfo* > _ClientChannels;
		bool _AnimChan;
	};
}




#endif //STRING_MGR_MODULE_H
