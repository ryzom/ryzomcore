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

#include "string_mgr_module.h"



#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"

#include "game_share/synchronised_message.h"
#include "game_share/send_chat.h"
#include "game_share/ios_interface.h"

using namespace NLNET;
using namespace NLMISC;
using namespace R2;
using namespace std;

NLNET_REGISTER_MODULE_FACTORY(CStringManagerModule, "StringManagerModule");


/**
*Send a npc chat
*/
void sendChatGroup(TDataSetRow& sender,CChatGroup::TGroupType groupType,ucstring& sentence)
{
	CMessage msg("NPC_CHAT_SENTENCE");
	msg.serial(sender);
	msg.serialEnum(groupType);
	msg.serial(sentence);
	CUnifiedNetwork::getInstance()->send("IOS",msg);
}

void sendChatChannel(TDataSetRow& sender,TChanID& chanId,ucstring& ucsentence)
{
	nldebug("NPC_CHAT_SENTENCE_CHANNEL");
	CMessage msg("NPC_CHAT_SENTENCE_CHANNEL");
	msg.serial(chanId);
	msg.serial(sender);
	msg.serial(ucsentence);
	CUnifiedNetwork::getInstance()->send("IOS",msg);
}





/**
*Add a chat client to the EGS
*/
static void addClient(TDataSetRow id)
{
	NLNET::CMessage msg("DYN_CHAT:ADD_CLIENT");
	msg.serial(id);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}

/**
*Add a chat session for a channel in the EGS
*/
static void addSession(NLMISC::CEntityId id,TChanID chId)
{
	static bool writeRight = true;
	NLNET::CMessage msg("DYN_CHAT:ADD_SESSION_ENTITY");
	msg.serial(chId);
	msg.serial(id);
	msg.serial(writeRight);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}

static void addSession(TDataSetRow id,TChanID chId)
{
	static bool writeRight = true;
	NLNET::CMessage msg("DYN_CHAT:ADD_SESSION");
	msg.serial(chId);
	msg.serial(id);
	msg.serial(writeRight);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}

static void removeSession(TDataSetRow& id,TChanID& chId)
{
	NLNET::CMessage msg("DYN_CHAT:REMOVE_SESSION");
	msg.serial(chId);
	msg.serial(id);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}

static void removeSession(NLMISC::CEntityId& id,TChanID& chId)
{
	NLNET::CMessage msg("DYN_CHAT:REMOVE_SESSION_ENTITY");
	msg.serial(chId);
	msg.serial(id);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}



void CStringManagerModule::requestDsr( ucstring& name)
{
	NLNET::CMessage msg("REQUEST_DSR");
	msg.serial(name);
	NLNET::CUnifiedNetwork::getInstance()->send("IOS",msg);
}

CStringManagerModule::CStringManagerModule()
{
	_AnimChan = false;
}

CStringManagerModule::~CStringManagerModule()
{
	_ClientChannels.clear();

}

void CStringManagerModule::init(NLNET::IModuleSocket* clientGW,CDynamicMapService* server)
{
	this->plugModule(clientGW);
}


void CStringManagerModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{
	std::string moduleName = moduleProxy->getModuleClassName();
	if( moduleName == "ClientEditionModule")
	{
		insertClient(moduleProxy,TChanID());;
	}
}

void CStringManagerModule::onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy)
{}

/**
*Insert a new client for the dss incarn chat
*The param mId must be valid. The other ones could be changed later.
*/
void CStringManagerModule::insertClient(NLNET::IModuleProxy *moduleProxy,TChanID channelId)
{
	ClientInfo* cInfo=new ClientInfo();
	//cInfo.ChanId = channelId;
	cInfo->Proxy = moduleProxy;
	_ClientChannels.insert(std::pair<TModuleId,ClientInfo* >(moduleProxy->getModuleProxyId(),cInfo));
}

void CStringManagerModule::removeClient(NLNET::IModuleProxy *moduleProxy)
{
	std::map<TModuleId,ClientInfo* >::iterator found = _ClientChannels.find(moduleProxy->getModuleProxyId());

	if (found != _ClientChannels.end())
	{
		ClientInfo* c = found->second;
		delete c;
	}
}

void CStringManagerModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	std::string moduleName = moduleProxy->getModuleClassName();
	if( moduleName == "ClientEditionModule")
	{
		//release the channels used by this client
		std::map<TModuleId,ClientInfo* >::iterator found = _ClientChannels.find(moduleProxy->getModuleProxyId());
		if( found!=_ClientChannels.end())
		{
			delete found->second;
			_ClientChannels.erase(found);
		}
	}
}

bool CStringManagerModule::onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message)
{
	nlassert(message.isReading());

	std::string operationName = message.getName();
	if(operationName == "registerTable")
	{
		registerTableRequested(message);
	}
	else if(operationName == "unregisterTable")
	{
		TSessionId scenarioId;
		nlRead(message, serial, scenarioId);
		unregisterTableRequested(scenarioId);
	}
	else if(operationName == "translateAndForward")
	{

		TDataSetRow senderId;
		CChatGroup::TGroupType groupType;
		std::string id;
		TSessionId scenarioId;
		nlRead(message, serial, senderId);
		nlRead(message, serialEnum, groupType);
		nlRead(message, serial, id);
		nlRead(message, serial, scenarioId);
		translateAndForward(senderId,groupType,id,scenarioId);
	}
	else
	if(operationName == "translateAndForwardArg")
	{

		TDataSetRow senderId;
		CChatGroup::TGroupType groupType;
		std::string id;
		uint32 i=0,size;
		float value;
		TSessionId scenarioId;
		nlRead(message, serial, senderId);
		nlRead(message, serialEnum, groupType);
		nlRead(message, serial, id);
		nlRead(message, serial, scenarioId);
		nlRead(message, serial, size);
		std::vector<float> args;
		for(;i<size;++i)
		{
			nlRead(message, serial, value);
			args.push_back(value);
		}
		translateAndForwardWithArg(senderId,groupType,id,scenarioId,args);
	}
	else if(operationName=="TALK_AS")
	{
		TDataSetRow dsr ;
		TModuleId clientId;
		TSessionId sessionId;
		std::string name;
		nlRead(message, serial, clientId);
		nlRead(message, serial, dsr);
		nlRead(message, serial, name);
		nlRead(message, serial, sessionId);
		talkAs(clientId, dsr,name,sessionId);
	}
	else if(operationName == "CLEAR_CHANNELS")
	{
		TSessionId sessionId ;
		nlRead(message, serial, sessionId);
		releaseChannels(sessionId);
	}
	else if(operationName=="stopTalk")
	{
		TModuleId id ;
		TDataSetRow npcId;
		nlRead(message, serial,id);
		nlRead(message, serial,npcId);
		stopTalkAs(id,npcId);
	}
	else if(operationName=="requestStringTable")
	{
		TSessionId scenarioId;
		TModuleId mId;
		nldebug("string table requested!!");
		nlRead(message,serial,scenarioId);
		nlRead(message,serial,mId);
		map<NLNET::TModuleId,ClientInfo* >::const_iterator found = _ClientChannels.find(mId);
		if(found != _ClientChannels.end())
		{
			sendTable(scenarioId,found->second->Proxy);
		}
		else
		{
		}
	}
	else if(operationName=="requestSetValue")
	{
		TSessionId scenarioId;
		std::string localId;
		std::string value;
		nlRead(message, serial, scenarioId);
		nlRead(message, serial, localId);
		nlRead(message, serial, value);
		setValue(scenarioId.asInt(),localId,value);

	}
	else if(operationName == "requestStringValue")
	{
		TSessionId scenarioId;
		std::string localId;
		TModuleId mId;
		nlRead(message, serial, scenarioId);
		nlRead(message, serial, mId);
		nlRead(message, serial, localId);
		map<NLNET::TModuleId,ClientInfo* >::const_iterator found = _ClientChannels.find(mId);
		if(found != _ClientChannels.end())
		{
			sendStringValue(scenarioId,found->second->Proxy,localId);
		}
	}
	else if (operationName == "requestIdList")
	{
		TSessionId scenarioId;
		std::string eid;
		TModuleId mId;
		nlRead(message,serial,scenarioId);
		nlRead(message,serial,mId);
		map<NLNET::TModuleId,ClientInfo* >::const_iterator found = _ClientChannels.find(mId);
		if(found !=_ClientChannels.end())
		{
			sendIdList(scenarioId,found->second->Proxy);
		}
	}
	else
	{
		nlwarning("No operation '%s' defined !",operationName.c_str());
		nlassert(0);

		return false;
	}

	return true;
}

void CStringManagerModule::addAnimSession(TModuleId id,TSessionId scenarioId)
{
	std::map<TModuleId,ClientInfo*>::iterator found = _ClientChannels.find(id);
	if(found!=_ClientChannels.end())
	{
		found->second->SessionId = scenarioId;
		uint32 charId;
		NLMISC::CEntityId clientEid;
		std::string userPriv;
		std::string extendedPriv;
		if (!getCharInfo(found->second->Proxy, charId, clientEid, userPriv, extendedPriv))
		{
			nlwarning("R2Ed: module '%s' has Invalid Security Info", found->second->Proxy->getModuleName().c_str());
		}
		else
		{
			addSession(clientEid,_AnimChans[scenarioId.asInt()]);
		}
	}
}

void CStringManagerModule::registerTableRequested(const CMessage& msgin)
{
	TSessionId scenarioId;

	uint32 nbEntry;
	std::string id,text;
	nlRead(msgin,serial,scenarioId);

	{
		uint32 size;
		nlRead(msgin,serial,size);
		for(uint32 i=0;i<size;++i)
		{
			TModuleId id;
			nlRead(msgin,serial,id);
			if(_AnimChan)
			{
				_AnimChans[scenarioId.asInt()] = initChannel("Animators",false);
				addAnimSession(id,scenarioId);
			}
		}
	}
	nlRead(msgin,serial,nbEntry);

	createTable(scenarioId.asInt());

	for(uint32 i=0;i<nbEntry;++i)
	{
		nlRead(msgin, serial, id);
		nlRead(msgin, serial, text);
		setValue(scenarioId.asInt(), id, text);
	}
}

//unused function
void CStringManagerModule::registerTableRequested(TSessionId sessionId, std::vector<std::pair<std::string,std::string> >& entries)
{
	std::string id,text;
	uint32 nbEntry = (uint32)entries.size();

	releaseTable(sessionId.asInt());
	for(uint i=0;i<nbEntry;++i)
	{
		setValue(sessionId.asInt(), entries[i].first, entries[i].second);
	}
}

//unused function
void CStringManagerModule::registerTableRequested(TSessionId sessionId,CObject* table)
{
	std::vector< std::pair<std::string,std::string> > entries;

	//verify that there is no table for this scenario

	CObject* textsTable = table->getAttr("Texts");
	uint32 max = textsTable->getSize();
	//fill the table
	//for each entry of the local table
	for(uint i=0;i<max;++i)
	{
		CObject* text = textsTable->getValue(i);
		CObject* tmp = text->getAttr("Id");
		nlassert(tmp);
		std::string localId = tmp->toString();
		tmp = text->getAttr("Text");
		nlassert(tmp);
		std::string value = tmp->toString();
		entries.push_back( std::make_pair(localId, value));

	}
	registerTableRequested(sessionId, entries);
}

void CStringManagerModule::unregisterTableRequested(TSessionId sessionId)
{

	releaseTable(sessionId.asInt());
	if(_AnimChan )
	{
		NLNET::CMessage msg("DYN_CHAT:REMOVE_SERVICE_CHAN");
		msg.serial(_AnimChans[sessionId.asInt()]);
		NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
		_AnimChans[sessionId.asInt()]=TChanID();
	}

}



static std::string formatString(std::string str,std::vector<float> args)
{
	uint32 size = (uint32)args.size();
	std::string ret="";
	{
		std::string::size_type pos = 0;
		CSString cstring(str);
		for(uint i=0;i<size;++i)
		{
			pos = cstring.find("$d",0);
			ret += cstring.substr(0,pos);
			ret += toString((uint32)args[i]);
			cstring = cstring.substr(pos+2);
		}
		if(cstring.length()!=0)
			ret = ret + cstring;
	}
	return ret;
}

void CStringManagerModule::translateAndForwardWithArg(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId,std::vector<float>& args)
{
	std::string text = getValue(sessionId.asInt(), id );
	if (text!="")
	{
		std::string toSend = formatString(text,args);
		send(senderId,groupType,toSend);
	}
}

void CStringManagerModule::translateAndForward(TDataSetRow senderId,CChatGroup::TGroupType groupType,std::string id,TSessionId sessionId)
{
	std::string toSend = getValue(sessionId.asInt(), id);
	if(toSend != "")
		send(senderId,groupType,toSend);
}


void CStringManagerModule::send(TDataSetRow& senderId,CChatGroup::TGroupType groupType,const std::string& toSend)
{
	if(toSend != "")
	{
		ucstring uStr;
		uStr.fromUtf8(toSend);
		//for each client (animator)
		std::map<NLNET::TModuleId,ClientInfo* >::const_iterator first(_ClientChannels.begin()), last(_ClientChannels.end());
		for (;first!=last;first++)
		{
			TChanID chanId = first->second->getIncarnation(senderId);
			if( !chanId.isUnknownId() )
			{
				//ucstring tmp("{no_bubble}"+toSend);
				ucstring uStr2("{no_bubble}");
				uStr2 += uStr;
				sendChatChannel(senderId,chanId,uStr2);
				return;
			}
		}




		//if nobody incarn the npc, send the chat normally
		sendChatGroup(senderId,groupType,uStr);
	}
}


void CStringManagerModule::talkAs(TModuleId& id, TDataSetRow&  creaturRowId,std::string& name,TSessionId sessionId)
{
	//looking for the client's channel
	std::map<NLNET::TModuleId, ClientInfo* >::iterator found= _ClientChannels.find(id);
	if (found == _ClientChannels.end())
	{
		nlwarning("client unregistered! moduleId: %u",id);
		return;
	}
	found->second->SessionId = sessionId;
	//the player already incarn this npc
	if (found->second->getIncarnation(creaturRowId)!=TChanID())return;
	TChanID chanId = initChannel(name);
	found->second->addIncarnation(chanId,creaturRowId);
}


void CStringManagerModule::stopTalkAs(TModuleId& id,TDataSetRow& npcId)
{
	std::map<NLNET::TModuleId,ClientInfo* >::iterator found= _ClientChannels.find(id);
	if(found != _ClientChannels.end())
	{
		found->second->removeIncarnation(npcId);
	}
}

void CStringManagerModule::releaseChannels(TSessionId sessionId)
{
	std::map<NLNET::TModuleId,ClientInfo* >::iterator first( _ClientChannels.begin()),
			last(_ClientChannels.end());
	while(first != last)
	{
		if(first->second->SessionId==sessionId)
		{
			nldebug("<releaseChannels> : found an animation module with sessionId %u",sessionId.asInt());
			first->second->clear();
		}
		++first;
	}
}


void CStringManagerModule::onApplicationExit()
{

}


TChanID CStringManagerModule::initChannel(std::string name,bool forwardInput)
{
	TChanID channelId = CEntityId::getNewEntityId(RYZOMID::dynChatGroup);
	bool noBroadcast = false;
	static uint32 cpt=0;
	std::string chanName = "dssChan_" + toString(cpt);
	++cpt;
	if(channelId == TChanID::Unknown)
	{
		nlwarning("error while creating id for dyn chat channel!");
		return TChanID();
	}

	{
		CMessage msg("DYN_CHAT:ADD_SERVICE_CHAN");
		msg.serial(channelId);
		msg.serial(chanName);
		msg.serial(noBroadcast);
		msg.serial(forwardInput);
		CUnifiedNetwork::getInstance()->send("EGS",msg);
	}

	{
		bool hideBubble = true;
		CMessage msg("DYN_CHAT:SET_HIDE_BUBBLE");
		msg.serial(channelId);
		msg.serial(hideBubble);
		CUnifiedNetwork::getInstance()->send("EGS",msg);
	}
	CIOSMsgSetPhrase(chanName,name).send();



	return channelId;
}


//called when a DYN_CHAT:FORWARD message is received by the DSS
void CStringManagerModule::forwardIncarnChat(TChanID id,TDataSetRow senderId,ucstring sentence)
{
	nldebug("dyn chat '%s' in channel '%s'",sentence.c_str(),id.toString().c_str());
	map<NLNET::TModuleId,ClientInfo* >::const_iterator first(_ClientChannels.begin()),last(_ClientChannels.end());
	while(first != last)
	{
		//look for the npc registered in this channel
		TDataSetRow npcId = first->second->getIncarnation(id);

		//if the chat is not from this npc, it come from the player
		//and we must forward it
		if( (npcId.isValid()) && (npcId!=senderId) )
		{
			static ucstring noBubble("{no_bubble}");
			static ucstring::size_type noBubbleLen = noBubble.length();

			CChatGroup::TGroupType groupType = CChatGroup::say;

			ucstring tmp = sentence;
			ucstring::size_type pos = tmp.find(noBubble);
			if (pos != ucstring::npos && pos == 0)
			{
				tmp = tmp.substr(pos + noBubbleLen);
			}
			sendChatGroup(npcId,groupType,tmp);
			return;
		}

		++first;
	}
	nlwarning("unable to find npc incarnated !");
}



void CStringManagerModule::sendIdList(TSessionId scenarioId,NLNET::IModuleProxy* senderModuleProxy)
{
	TLocalTable* localTable = getLocalTable(scenarioId.asInt());

	if(localTable != NULL)
	{
		TLocalTable::const_iterator first(localTable->begin()),last(localTable->end());
		CMessage msg("idList");
		uint32 nb = (uint32)localTable->size();
		msg.serial(nb);
		for( ; first!=last; ++first)
		{
			std::string stringId = first->first ;
			msg.serial(stringId);
		}
		senderModuleProxy->sendModuleMessage(this,msg);
	}
}

void CStringManagerModule::sendStringValue(TSessionId scenarioId,NLNET::IModuleProxy* moduleProxy,std::string id)
{
	std::string val = getValue(scenarioId.asInt(),id);
	CMessage msg("stringValue");
	msg.serial(id);
	msg.serial(val);
	moduleProxy->sendModuleMessage(this,msg);
}


//send a local table to a client
void CStringManagerModule::sendTable(TSessionId scenarioId,IModuleProxy *moduleProxy)
{
	TLocalTable* localTable = getLocalTable(scenarioId.asInt());
	if (localTable != NULL)
	{
		uint32 nb = (uint32)localTable->size();
		if(nb==0)return;
		CMessage msg("stringTable");
		//eid of the client requesting the string table
		//number of entries
		msg.serial(nb);
		std::map<std::string,uint32>::const_iterator first(localTable->begin()), last(localTable->end());
		while(first!=last)
		{
			std::string localId = first->first;
			std::string text = getValue(scenarioId.asInt(), localId);
			//local id
			msg.serial(localId);
			//string
			msg.serial(text);
			first++;
		}
		moduleProxy->sendModuleMessage(this,msg);
	}
}







ClientInfo::~ClientInfo()
{
	clear();
}


TDataSetRow ClientInfo::getIncarnation(TChanID chanId)
{
	std::vector<TIncarn>::iterator first(_Incarnations.begin()),
	last(_Incarnations.end());
	while(first != last)
	{
		if((*first).ChanId == chanId)
		{
			return (*first).NpcId;
		}
		++first;
	}
	return TDataSetRow();
}


TChanID ClientInfo::getIncarnation(TDataSetRow npcId)
{
	std::vector<TIncarn>::iterator first(_Incarnations.begin()),
	last(_Incarnations.end());
	while(first != last)
	{
		if((*first).NpcId == npcId)
		{
			return first->ChanId;
		}
		++first;
	}
	return TChanID();
}


bool ClientInfo::incarn(TDataSetRow npcId)
{
	std::vector<TIncarn>::iterator first(_Incarnations.begin()),
	last(_Incarnations.end());
	while(first != last)
	{
		if((*first).NpcId == npcId)
		{
			return true;
		}
		++first;
	}
	return false;
}


void ClientInfo::addIncarnation(TChanID chanId,TDataSetRow& npcId)
{
	addClient(npcId);
	//add npc session to this channel
	addSession(npcId,chanId);
	uint32 charId;
	NLMISC::CEntityId eid;
	std::string userPriv;
	std::string extendedPriv;

	//add user session in this channel
	if(getCharInfo(Proxy, charId, eid, userPriv, extendedPriv))
	{
		addSession(eid,chanId);
	}
	TIncarn incarn;
	incarn.ChanId = chanId;
	incarn.NpcId = npcId;
	_Incarnations.push_back(incarn);
}


//release all channels used by this client
void ClientInfo::clear()
{
	uint32 size = (uint32)_Incarnations.size();
	for(uint32 i=0;i<size;++i)
	{
		_Incarnations[i].release();
	}
	_Incarnations.clear();
}


void ClientInfo::removeIncarnation(TDataSetRow& npcId)
{
	// Remove the npc from the _Incarnations list
	std::vector<TIncarn>::iterator first(_Incarnations.begin()), last(_Incarnations.end());
	for( ; first != last; ++first)
	{
		if((*first).NpcId == npcId)
		{
			first->release();
			_Incarnations.erase(first);
			return;
		}
	}
}


void TIncarn::release()
{
	NLNET::CMessage msg("DYN_CHAT:REMOVE_SERVICE_CHAN");
	msg.serial(ChanId);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS",msg);
}
