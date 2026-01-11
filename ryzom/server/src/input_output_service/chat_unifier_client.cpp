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
#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "server_share/chat_unifier_itf.h"
#include "server_share/r2_variables.h"

#include "input_output_service.h"
#include "chat_manager.h"
#include "chat_unifier_client.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace CHATUNI;

extern CVariable<bool>		ForceFarChat;


class CChatUnifierClient : 
	public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
	public IChatUnifierClient,
	public CChatUnifierClientSkel
{
public:
	/// The module manifest, include the shard id
	mutable string		_Manifest;

	/// The chat unifier server (for tell message routing)
	TModuleProxyPtr		_ChatUnifierServer;

	/// The peer chat unifier clients (other IOS) for guild chat forwarding
	set<TModuleProxyPtr>	_Peers;
	

	CChatUnifierClient()
	{
		CChatUnifierClientSkel::init(this);
	}

	
	void onModuleUp(IModuleProxy *proxy)
	{
		if (proxy->getModuleClassName() == "ChatUnifierServer")
		{
			nlinfo("CChatUnifierClient::onModuleUp : using module '%s' as chat unifier server", proxy->getModuleName().c_str());
			_ChatUnifierServer = proxy;
		}
		else if (proxy->getModuleClassName() == "ChatUnifierClient")
		{
			nlinfo("CChatUnifierClient::onModuleUp : find a new peer '%s'", proxy->getModuleName().c_str());
			// this a one of our peer
			_Peers.insert(proxy);
		}
	}

	void onModuleDown(IModuleProxy *proxy)
	{
		if (proxy == _ChatUnifierServer)
		{
			nlinfo("CChatUnifierClient::onModuleDown : char unifier server '%s' is down.", proxy->getModuleName().c_str());
			_ChatUnifierServer = NULL;
		}
		else if (_Peers.find(proxy) != _Peers.end())
		{
			nlinfo("CChatUnifierClient::onModuleDown : peer module '%s' is down", proxy->getModuleName().c_str());
			_Peers.erase(proxy);
		}
	}

//	void onProcessModuleMessage(IModuleProxy *proxy, const CMessage &message)
//	{
//		if (CChatUnifierClientSkel::onDispatchMessage(proxy, message))
//			return;
//
//		nlwarning("CChatUnifierClient : failed to dispatch message '%s'", message.getName().c_str());
//	}

	std::string buildModuleManifest() const
	{
		_Manifest = "chatUnifierClient(shardId=";
		_Manifest += toString("%u", IService::getInstance()->getShardId());
		_Manifest += ")";

		return _Manifest;
	}

	/**************************************/
	/** IChatUnifierClient implementation */
	/**************************************/

	void sendFarTell(const CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text)
	{
		if (_ChatUnifierServer == NULL)
			return;

		CChatUnifierProxy cuc(_ChatUnifierServer);

		cuc.sendFarTell(this, senderCharId, havePrivilege, destName, text);
	}

	void sendFarGuildChat(const ucstring &senderName, uint32 guildId, const ucstring &text)
	{
		CChatUnifierClientProxy::broadcast_farGuildChat(_Peers.begin(), _Peers.end(), this, senderName, guildId, text);
	}

	void sendFarGuildChat2(const ucstring &senderName, uint32 guildId, const std::string &phraseName)
	{
		CChatUnifierClientProxy::broadcast_farGuildChat2(_Peers.begin(), _Peers.end(), this, senderName, guildId, phraseName);
	}

	void sendFarGuildChat2Ex(const ucstring &senderName, uint32 guildId, uint32 phraseId)
	{
		CChatUnifierClientProxy::broadcast_farGuildChat2Ex(_Peers.begin(), _Peers.end(), this, senderName, guildId, phraseId);
	}

	void sendUniverseChat(const ucstring &senderName, uint32 homeSessionId, const ucstring &text)
	{
		CChatUnifierClientProxy::broadcast_universeBroadcast(_Peers.begin(), _Peers.end(), this, senderName, homeSessionId, text);

		if (ForceFarChat)
		{
			// send back the chat locally
			CChatManager &cm = IOS->getChatManager();

			// rebuild a the universe group id and fake the creator and dynamic id
			TGroupId grpId(RYZOMID::chatGroup,0);
			cm.farChatInGroup(grpId, homeSessionId, text, senderName);
		}
	}

	void sendUnifiedDynChat(const NLMISC::CEntityId &dynCharId, const ucstring &senderName, const ucstring &text)
	{
#ifdef NL_OS_WINDOWS
#	pragma message (NL_LOC_WRN "Add the message in the interface")
#endif
		CChatUnifierClientProxy::broadcast_dynChanBroadcast(_Peers.begin(), _Peers.end(), this, dynCharId, senderName, text);
	}



	/******************************************/
	/** CChatUnifierClientSkel implementation */
	/******************************************/

	// SU send a far tell failure to IOS. This mean that the player is offline or unknow
	void recvFarTellFail(NLNET::IModuleProxy *sender, const CEntityId &senderCharId, const ucstring &destName, TFailInfo failInfo)
	{
		nldebug("IOSCU: recvFarTellFail : receiving a far tell failure from %s to '%s'", senderCharId.toString().c_str(), destName.toUtf8().c_str());
		// try to retrieve the sender char
		TDataSetRow dsr = TheDataset.getDataSetRow(senderCharId);
		if (!dsr.isValid())
		{
			nldebug("IOSCU: recvFarTellFail : can't found dataset row for character %s", senderCharId.toString().c_str());
			// nothing more to do
			return;
		}

		CChatManager &cm = IOS->getChatManager();
		if (!cm.checkClient(dsr))
		{
			// the client is not in the chat manager! 
			nldebug("IOSCU: recvFarTellFail : can't found client data in chat manager for sender character %s)", senderCharId.toString().c_str());
			// nothing more to do
			return;
		}

//		CChatClient &cc = cm.getClient(dsr);

		switch(failInfo.getValue())
		{
		case TFailInfo::fi_no_entity_locator:
		case TFailInfo::fi_no_ios_module:
		case TFailInfo::fi_no_char_sync:
		case TFailInfo::fi_sender_char_unknown:
		case TFailInfo::fi_dest_char_unknown:
		case TFailInfo::fi_char_offline:
			{
				SM_STATIC_PARAMS_1( vect, STRING_MANAGER::literal );
				vect[0].Literal = destName;
				uint32 phraseId = STRING_MANAGER::sendStringToClient( dsr, "TELL_PLAYER_UNKNOWN", vect, &IosLocalSender );
				cm.sendChat2Ex( CChatGroup::tell, dsr, phraseId );
			}
			break;
		}

	}

	// SU send a far tell to the IOS hosting the addressee character
	void recvFarTell(NLNET::IModuleProxy *sender, const CEntityId &senderCharId, const ucstring &senderName, bool havePrivilege, const ucstring &destName, const ucstring &text)
	{
		nldebug("IOSCU: recvFarTell : receiving a far tell from %s to '%s'", senderCharId.toString().c_str(), destName.toUtf8().c_str());
		CChatManager &cm = IOS->getChatManager();
		cm.farTell(senderCharId, senderName, havePrivilege, destName, text);
	}

	// SU forward a guild chat message to the IOS
	void farGuildChat(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, const ucstring &text)
	{
		CChatManager &cm = IOS->getChatManager();

		// rebuild a group ID and fake the creator and dynamic id
		TGroupId grpId(RYZOMID::chatGroup, guildId, 0, 0);
		cm.farChatInGroup(grpId, 0, text, senderName);
	}

	// SU forward a guild chat message to the IOS
	void farGuildChat2(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, const ucstring &phraseName)
	{
	}

	// SU forward a guild chat message to the IOS
	void farGuildChat2Ex(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 guildId, uint32 phraseId)
	{
	}

	// IOS forward a universe chat message to the IOS
	virtual void universeBroadcast(NLNET::IModuleProxy *sender, const ucstring &senderName, uint32 senderHomeSession, const ucstring &text)
	{
		CChatManager &cm = IOS->getChatManager();

//		if (!IsRingShard)
//		{
//			// universe broadcast is allowed on on ring shard
//			return;
//		}

		// rebuild a the universe group id
		TGroupId grpId(RYZOMID::chatGroup,0);
		cm.farChatInGroup(grpId, senderHomeSession, text, senderName);
	}

	// IOS forward a dyn chat chat message to the IOSs
	virtual void dynChanBroadcast(NLNET::IModuleProxy *sender, const NLMISC::CEntityId &chanId, const ucstring &senderName, const ucstring &text)
	{
		CChatManager &cm = IOS->getChatManager();

		// retreive the dyn chat (is it exist here)
		CDynChatChan *chan =  cm.getDynChat().getChan(chanId);

		if (chan ==  NULL)
		{
			nldebug("IOSCU : universeBroadcast : cannot find dynamic channel %s to broadcast chat", chanId.toString().c_str());
			return;
		}

		// broadcast to other client in the channel
		CDynChatSession *dcc = chan->getFirstSession();
		while (dcc)
		{
			cm.sendChat(CChatGroup::dyn_chat, dcc->getClient()->getID(), text, TDataSetRow(), chanId, senderName);
			dcc = dcc->getNextChannelSession(); // next session in this channel
		}						
	}

	// SU send a broadcast message to the IOS
	void recvBroadcastMessage(NLNET::IModuleProxy *sender, const ucstring &message)
	{
	}

	/*************************************************************************/
	/* Commands handler														 */
	/*************************************************************************/
	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CChatUnifierClient, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CChatUnifierClient, dump, "Dump the module internal state", "no param");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("-----------------------------------");
		log.displayNL("Dumping Chat unifier client state :");
		log.displayNL("-----------------------------------");

		log.displayNL("  Chat Unifier client have %u known peer modules", 
			_Peers.size());

		set<TModuleProxyPtr>::iterator first(_Peers.begin()), last(_Peers.end());
		for (; first != last; ++first)
		{
			log.displayNL("   + Peer module '%s' with manifest '%s'", 
				(*first)->getModuleName().c_str(),
				(*first)->getModuleManifest().c_str());
		}

		return true;

	}


};

NLNET_REGISTER_MODULE_FACTORY(CChatUnifierClient, "ChatUnifierClient");
