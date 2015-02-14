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


#include <sstream>
#include <map>

#include "client_edition_module.h"

#include "dmc.h"

#include "../../interface_v3/interface_manager.h"


#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"

#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"

#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "nel/ligo/primitive_utils.h"

#include "game_share/utils.h"

#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/r2_messages.h"
#include "game_share/ring_access.h"

#include "property_accessor.h"
#include "palette.h"

#include "../object_factory_client.h"

#include "com_lua_module.h"

#include "../../session_browser_impl.h"
#include "../../client_cfg.h"
#include "../../user_entity.h"
#include "../../view.h"
#include "../../init_main_loop.h"
#include "../../continent_manager.h"
#include "../../world_database_manager.h"
#include "../../far_tp.h"
#include "../../net_manager.h"

#include <zlib.h>



using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;

CVariable<std::string> UserComponentsExamplesDirectory ("DSS", "UserComponentsExampleDirectory", "", "examples/user_componts", 0, true );
CVariable<std::string> UserComponentsComponentsDirectory ("DSS", "UserComponentsComponentsDirectory", "", "examples/user_componts", 0, true );
CVariable<std::string> UserComponentsComponentExtension ("DSS", "UserComponentsComponentExtension", "", "gz", 0, true );
CVariable<std::string> UserComponentsSourceExtension ("DSS", "UserComponentsSourceExtension", "", "lua", 0, true );


namespace R2 {

	// if client request an operation, and the server accept then the client do not need to received the same msg he send
	// other client must receive the answer
	class IServerAnswerMsg;
	class CServerAnswerForseener
	{
	public:
		typedef uint32 TMessageId ;

	public:
		CServerAnswerForseener():_MaxMessageId(0){}
		~CServerAnswerForseener();
		// if ok == true, then simulate the replay that The Server would do for other client
		// if ok == false, remove the  Forseen Answer from the answer queue.
		void ack(CClientEditionModule* client, NLNET::IModuleProxy *server, TMessageId msgId, bool ok );

		// Add to the Forssen Answer queue (we do not take the ownership of value)
		TMessageId onScenarioUploaded( const CObject* hlScenario) ;
		TMessageId onNodeSet(const std::string &instanceId, const std::string &attrName, const R2::CObject *value);
		TMessageId onNodeInserted(const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObject *value);
		TMessageId onNodeErased(const std::string &instanceId, const std::string &attrName, sint32 position);
		TMessageId onNodeMoved(const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);

	private:
		typedef std::map<TMessageId, IServerAnswerMsg*> TAnswers;

	private:
		uint32 _MaxMessageId;
		TAnswers _Answers;

	};

	class IServerAnswerMsg
	{
	public:
		virtual void ok(CClientEditionModule* client, NLNET::IModuleProxy *server) = 0;
		virtual ~IServerAnswerMsg()	{}
	};

	class CServerAnswerMsgScenarioUploaded: public IServerAnswerMsg
	{
	public:
		CServerAnswerMsgScenarioUploaded( const R2::CObject* value)
			:_Value( value?value->clone():0){}

		void ok(CClientEditionModule* client, NLNET::IModuleProxy *server)
		{
			client->onScenarioUploaded(server, _Value.get());
		}
	private:
		std::auto_ptr<R2::CObject> _Value;
	};

	class CServerAnswerMsgSet: public IServerAnswerMsg
	{
	public:
		CServerAnswerMsgSet(const std::string &instanceId, const std::string &attrName, const R2::CObject* value)
			:_InstanceId(instanceId), _AttrName(attrName), _Value( value?value->clone():0){}

		void ok(CClientEditionModule* client, NLNET::IModuleProxy *server)
		{
			client->onNodeSet(server, _InstanceId, _AttrName, _Value.get());
		}
	private:
		std::string _InstanceId;
		std::string _AttrName;
		std::auto_ptr<R2::CObject> _Value;
	};

	class CServerAnswerMsgInserted: public IServerAnswerMsg
	{
	public:
		CServerAnswerMsgInserted(const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObject* value)
			:_InstanceId(instanceId), _AttrName(attrName), _Position(position), _Key(key), _Value( value?value->clone():0){}

		void ok(CClientEditionModule* client, NLNET::IModuleProxy *server)
		{
			client->onNodeInserted(server, _InstanceId, _AttrName, _Position, _Key, _Value.get());
		}
	private:
		std::string _InstanceId;
		std::string _AttrName;
		sint32 _Position;
		std::string _Key;
		std::auto_ptr<R2::CObject> _Value;
	};

	class CServerAnswerMsgErased: public IServerAnswerMsg
	{
	public:
		CServerAnswerMsgErased(const std::string &instanceId, const std::string &attrName, sint32 position)
			:_InstanceId(instanceId), _AttrName(attrName), _Position(position){}

		void ok(CClientEditionModule* client, NLNET::IModuleProxy *server)
		{
			client->onNodeErased(server, _InstanceId, _AttrName, _Position);
		}
	private:
		std::string _InstanceId;
		std::string _AttrName;
		sint32 _Position;
	};

	class CServerAnswerMsgMoved : public IServerAnswerMsg
	{
	public:
		CServerAnswerMsgMoved(const std::string &instanceId1, const std::string &attrName1, sint32 position1,
				const std::string &instanceId2, const std::string &attrName2, sint32 position2)
			:_InstanceId1(instanceId1), _AttrName1(attrName1), _Position1(position1),
			_InstanceId2(instanceId2), _AttrName2(attrName2), _Position2(position2){}

		void ok(CClientEditionModule* client, NLNET::IModuleProxy *server)
		{
			client->onNodeMoved(server, _InstanceId1, _AttrName1, _Position1, _InstanceId2, _AttrName2, _Position2);
		}
	private:
		std::string _InstanceId1;
		std::string _AttrName1;
		sint32 _Position1;
		std::string _InstanceId2;
		std::string _AttrName2;
		sint32 _Position2;


	};
}

using namespace R2;


NLNET_REGISTER_MODULE_FACTORY(CClientEditionModule, "ClientEditionModule");


//----------------------------- <CServerAnswer>  -------------------------------
CServerAnswerForseener::~CServerAnswerForseener()
{
	TAnswers::iterator first(_Answers.begin()), last(_Answers.end());
	for (; first != last; ++first)
	{
		IServerAnswerMsg* msg = first->second;
		delete msg;
	}
	_Answers.clear();


}

void CServerAnswerForseener::ack(CClientEditionModule* client, NLNET::IModuleProxy *server, TMessageId msgId, bool ok)
{
	TAnswers::iterator found = _Answers.find(msgId);
	BOMB_IF( found == _Answers.end(), "Message not found", return);

	IServerAnswerMsg* msg = found->second;
	if (ok)
	{
		msg->ok(client, server);
	}
	delete msg;
	_Answers.erase(found);
}

// Add to the Forseen Answer queue
CServerAnswerForseener::TMessageId CServerAnswerForseener::onScenarioUploaded( const R2::CObject* hlScenario)
{
	bool ok = _Answers.insert( std::make_pair(++_MaxMessageId, new CServerAnswerMsgScenarioUploaded(hlScenario))).second;
	nlassert(ok);
	return _MaxMessageId;
}

CServerAnswerForseener::TMessageId CServerAnswerForseener::onNodeSet(const std::string &instanceId, const std::string &attrName, const R2::CObject* value)
{
	bool ok = _Answers.insert(std::make_pair(++_MaxMessageId, new CServerAnswerMsgSet(instanceId, attrName, value))).second;
 	nlassert(ok);
	return _MaxMessageId;
}

CServerAnswerForseener::TMessageId CServerAnswerForseener::onNodeInserted(const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObject* value)
{
	bool ok = _Answers.insert(std::make_pair(++_MaxMessageId, new CServerAnswerMsgInserted(instanceId, attrName, position, key, value))).second;
	nlassert(ok);
	return _MaxMessageId;
}

CServerAnswerForseener::TMessageId CServerAnswerForseener::onNodeErased(const std::string &instanceId, const std::string &attrName, sint32 position)
{
	bool ok = _Answers.insert(std::make_pair(++_MaxMessageId, new CServerAnswerMsgErased(instanceId, attrName, position ))).second;
	nlassert(ok);
	return _MaxMessageId;
}

CServerAnswerForseener::TMessageId CServerAnswerForseener::onNodeMoved(const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
{
	bool ok = _Answers.insert(std::make_pair(++_MaxMessageId, new CServerAnswerMsgMoved(instanceId1, attrName1, position1, instanceId2, attrName2, position2 ))).second;
	nlassert(ok);
	return _MaxMessageId;
}

//------------------------------- <CEditorConfig> -------------------------------------------


std::map<std::string, uint32> CEditorConfig::_NameToId;

CEditorConfig::CEditorConfig()
{
	if (_NameToId.empty())
	{
		_NameToId["BossSpawner"] = 0;
		_NameToId["Timer"] = 1;
		_NameToId["ZoneTrigger"] = 2;
		_NameToId["UserTrigger"] = 3;
		_NameToId["TalkTo"] = 4;
		_NameToId["RequestItem"] = 5;
		_NameToId["GiveItem"] = 6;
		_NameToId["EasterEgg"] = 7;
		_NameToId["LootSpawner"] = 8;
		_NameToId["Fauna"] = 9;
		_NameToId["BanditCamp"] = 10;
		_NameToId["ChatSequence"] = 11;
		_NameToId["TimedSpawner"] = 12;
		_NameToId["Ambush"] = 13;
		_NameToId["ManHunt"] = 14;
		_NameToId["VisitZone"] = 15;
		_NameToId["KillNpc"] = 16;
		_NameToId["HuntTask"] = 17;
		_NameToId["DeliveryTask"] = 18;
		_NameToId["TargetMob"] = 19;
		_NameToId["HiddenChest"] = 20;
		_NameToId["RandomChest"] = 21;
		_NameToId["UserComponent"] = 22;
		_NameToId["Npc"] = 23;
		_NameToId["GetItemFromSceneryObjectTaskStep"] = 24;
		_NameToId["GetItemFromSceneryObject"] = 25;
		_NameToId["SceneryObjectInteractionTaskStep"] = 26;
		_NameToId["SceneryObjectInteraction"] = 27;
		_NameToId["SceneryObjectRemover"] = 28;
		_NameToId["RewardProvider"] = 29;
		_NameToId["NpcInteraction"] = 30;
		_NameToId["Quest"] = 31;
		_NameToId["ProximityDialog"] = 32;

	}
}

void CEditorConfig::setDisplayInfo(const std::string& formName, bool displayInfo)
{
	//H_AUTO(R2_CEditorConfig_setDisplayInfo)
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	uint32 index = _NameToId[formName];
	uint32 newValue = static_cast<uint32>(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:R2:DISPLAYINFO")->getValue32());
	if (displayInfo == false )
		newValue &= ~(1 << index);
	else
		newValue |= (1 << index);
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:R2:DISPLAYINFO")->setValue32(static_cast<sint32>(newValue));

}

void CEditorConfig::setDisplayInfo(uint32 displayInfo)
{
	//H_AUTO(R2_CEditorConfig_setDisplayInfo)
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:R2:DISPLAYINFO")->setValue32(static_cast<sint32>(displayInfo));
}



bool CEditorConfig::mustDisplayInfo(const std::string& formName) const
{
	//H_AUTO(R2_CEditorConfig_mustDisplayInfo)
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	std::map<std::string, uint32>::const_iterator found = _NameToId.find(formName);
	if (found == _NameToId.end())
		return false;
	uint32 index = (*found).second;
	uint32 newValue = static_cast<uint32>(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:R2:DISPLAYINFO")->getValue32());
	uint32 ok = (newValue >> index) & 0x00000001;
	if (ok != 0)
		return true;
	return false;
}

bool CEditorConfig::hasDisplayInfo(const std::string& formName) const
{
	//H_AUTO(R2_CEditorConfig_hasDisplayInfo)
	std::map<std::string, uint32>::const_iterator found = _NameToId.find(formName);
	if (found != _NameToId.end())
		return true;
	return false;
}

uint32 CEditorConfig::getDisplayInfo() const
{
	//H_AUTO(R2_CEditorConfig_getDisplayInfo)
	CInterfaceManager *IM = CInterfaceManager::getInstance ();
	return static_cast<uint32>(NLGUI::CDBManager::getInstance()->getDbProp("UI:SAVE:R2:DISPLAYINFO")->getValue32());
}



//----------------------------<CClientEditionModule>-------------------------------------------------------------
CClientEditionModule::CClientEditionModule()

{
	CShareClientEditionItfSkel::init(this);
	_Initialized = false;
	_ChannelId = TChanID::Unknown;
	_Mute = false;

	_ServerAnswerForseener = 0;
}

CClientEditionModule::~CClientEditionModule()
{
	release();
}

void CClientEditionModule::init(NLNET::IModuleSocket* clientGw, CDynamicMapClient* client)
{
	//H_AUTO(R2_CClientEditionModule_init)
	_Client = client;

//	_ClientGw = clientGw;

	_Eid = "Client0";
	_TranslationModule = 0;
	_SessionId = TSessionId(0);


	init();
	this->plugModule(clientGw);
}

void CClientEditionModule::init()
{
	//H_AUTO(R2_CClientEditionModule_init)
	if( _Initialized )
		return;
	_Emotes.reset( new CEmoteBehavior() );
	_Palette = new CPalette();
	_Scenario = new CScenario(0);
	_Factory = new CObjectFactoryClient(_Eid);
	_PropertyAccessor = new CPropertyAccessor(_Client, _Factory);
//	CObjectSerializer::Factory = _Factory;
	CObjectSerializerClient::setClientObjectFactory(_Factory);
	_Initialized = true;
	_MaxNpcs = 100;
	_MaxStaticObjects = 100;

	_ClientEditorConfig = new CEditorConfig();

	_MustStartScenario = false;
	_ScenarioUpToDate = false;
	_IsSessionOwner = false;
	_ServerAnswerForseener = new CServerAnswerForseener();

}

// when server is launched locally, it shouldn't use the client factory,
// this class disable when local server is called
//class CSerialFactoryBackup
//{
//public:
//	CSerialFactoryBackup(CObjectFactory *newValue = NULL) : ClientFactory(CObjectSerializer::Factory)
//	{
//		CObjectSerializer::Factory = newValue;
//	}
//	~CSerialFactoryBackup()
//	{
//		CObjectSerializer::Factory = ClientFactory;
//	}
//	CObjectFactory *ClientFactory;
//};
//

void CClientEditionModule::release()
{
	//H_AUTO(R2_CClientEditionModule_release)
	delete _ServerAnswerForseener; _ServerAnswerForseener = 0;
	delete _Palette; _Palette = 0;
	delete _Scenario;  _Scenario = 0;
	delete _Factory; _Factory = 0;
	delete _PropertyAccessor;	_PropertyAccessor = 0;
	_Initialized = false;
	_Emotes.reset();
}



void CClientEditionModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{
	//H_AUTO(R2_CClientEditionModule_onModuleUp)
	std::string moduleClassName = moduleProxy->getModuleClassName();

	if (moduleClassName == "ServerEditionModule")
	{
		_ServerEditionProxy = moduleProxy;
		return;
	}

	if (moduleClassName == "ServerAnimationModule")
	{
		_ServerAnimationProxy = moduleProxy;
		return;
	}


}

void CClientEditionModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	//H_AUTO(R2_CClientEditionModule_onModuleDown)
	std::string moduleClassName = moduleProxy->getModuleClassName();

	if (moduleClassName == "ServerEditionModule")
	{
		_ServerEditionProxy = NULL;
	}
	else if ( moduleClassName == "ServerAnimationModule")
	{
		_ServerAnimationProxy = NULL;
	}
	else
		return;
}

bool CClientEditionModule::onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &message)
{
	//H_AUTO(R2_CClientEditionModule_onProcessModuleMessage)

//	CSerialFactoryBackup fb(_Factory); // restore client factory the time to process server msg ...
//
//	if (CShareClientEditionItfSkel::onDispatchMessage(senderModuleProxy, message))
//	{
//		return;
//	}
//
//
//
	std::string operationName = message.getName();

	if (operationName == "CUCD") //onUserComponentDownloaded
	{
		CUserComponent* component = new CUserComponent();
		//message.serial(const_cast<CUserComponent&>(*component));
		component->serial( const_cast<CMessage&>(message));
		onUserComponentDownloaded(senderModuleProxy, component);
		return true;
	}

	if (operationName == "HELLO")
	{
		// this is just the firewall opening message, nothing to do
		return true;
	}

	if (operationName == "ADV_CONN")
	{
		CEditor::connexionMsg("");
		CClientMessageAdventureUserConnection  bodyConnection;
		nlRead(message,serial,bodyConnection);
		onRingAccessUpdated(0, bodyConnection.RingAccess);

		nlinfo("R2CED: user Connected as Client %d In Mode %d", bodyConnection.EditSlotId, bodyConnection.Mode);
		_SessionId = bodyConnection.SessionId;
		_AiInstanceId = bodyConnection.AiInstance;
		_SessionType = bodyConnection.SessionType;
		_IsSessionOwner = bodyConnection.IsSessionOwner;
		_EditSessionLink = bodyConnection.EditSessionLink;

		switch(bodyConnection.SessionType)
		{
		case st_edit: getEditor().setAccessMode(CEditor::AccessEditor); break;
		case st_anim: getEditor().setAccessMode(CEditor::AccessDM); break;
			default: nlassert(0 && "SessionType not handled.");
		}

		if (bodyConnection.SessionType == st_edit)
		{

			if (bodyConnection.Mode == 1)
			{
				_Client->onResetEditionMode();
			}


			CObject* data = bodyConnection.HighLevel.getData();
			if (!data && bodyConnection.InCache)
			{

				CScenarioValidator sv;
				CScenarioValidator::TValues values;
				std::string md5, signature;

				if ( sv.setScenarioToLoad("save/r2_buffer.dat", values, md5, signature, true)
					&& !md5.empty()
					&& CScenarioValidator::AutoSaveSignature == signature)
				{
					data = _Client->getComLuaModule().loadLocal("save/r2_buffer.dat", values);
				}
				else
				{
					nlinfo("Data corrupted (the server has refused to accept the scenario).");
				}
			}
			_Client->onEditionModeConnected(bodyConnection.EditSlotId, bodyConnection.SessionId.asInt(), data, bodyConnection.VersionName, bodyConnection.MustTp,  bodyConnection.InitialActIndex);
			if (bodyConnection.Mode == 2)
			{
				_Client->onEditionModeDisconnected();
				_Client->onTestModeConnected();
			}
			// warn server that adventure has been connected, so we are ready to receive the TP message
			CShareServerEditionItfProxy proxy(_ServerEditionProxy);
			proxy.advConnACK(this);
		}
		else if (bodyConnection.SessionType == st_anim)
		{
			 // 0 -> wait loading animation
			 // 1 -> waiting the other to load animation
			 // 2 -> play (dm)
			 //	3 -> play (simple player)
			_Client->onAnimationModeConnected(bodyConnection);

			// warn server that advneutre has been connected, so we are ready to receive the TP message
		}
		return true;

	}


	if (operationName == "EDITOR_STOP")
	{
		_Client->onEditionModeDisconnected();
		_Client->onTestModeConnected();
		return true;
	}


	if ( operationName=="stringTable")
	{
		nlwarning("received string table!");
		uint32 nb,i;
		nlRead(message, serial, nb);
		i=nb;
		nlwarning("%d entries! ",nb);
		while(i)
		{
			std::string localId;
			std::string value;
			nlRead(message, serial,localId);
			nlRead(message,serial,value);
			nlwarning("{ %s , %s}",localId.c_str(),value.c_str());
			i--;
		}
		return true;
	}

	if(operationName == "stringValue")
	{
		std::string localId;
		std::string value;
		nlRead(message,serial,localId);
		nlRead(message,serial,value);
		nlwarning("received {%s , %s}",localId.c_str(),value.c_str());
		return true;
	}

	if (operationName == "idList")
	{
		uint32 nb,i;
		nlRead(message,serial,nb);
		i=nb;
		nlwarning("%d entries! ",nb);
		while(i)
		{
			std::string localId;
			nlRead(message,serial,localId);
			nlwarning("{ %s }",localId.c_str());
			i--;
		}
		return true;
	}
	if (operationName == "HELLO")
	{
		// this is just the firewall opening message, nothing to do
		return true;
	}

	if (operationName == "NPC_APROP")
	{
		uint32 modes;
		nlRead(message,serial,modes);
		_Client->onNpcAnimationTargeted(modes);
		return true;
	}


	return false;

}

void CClientEditionModule::onTestModeDisconnected(NLNET::IModuleProxy * /* moduleProxy */, TSessionId sessionId, uint32 lastAct, TScenarioSessionType sessionType)
{
	//H_AUTO(R2_CClientEditionModule_onTestModeDisconnected)
	// indicate the Editor that the animation has stopped.
	_Client->onTestModeDisconnected(sessionId, lastAct, sessionType);


	if ( sessionType == st_edit )
	{
		this->requestReconnection();
	}

}



void CClientEditionModule::onModuleSecurityChange(IModuleProxy * /* moduleProxy */)
{
	//H_AUTO(R2_CClientEditionModule_onModuleSecurityChange)
}

void CClientEditionModule::requestCreateScenario(CObject* scenario)
{
	//H_AUTO(R2_CClientEditionModule_requestCreateScenario)
	this->requestUploadScenario(scenario);
}


bool CClientEditionModule::requestTeleportOneCharacterToAnother(uint32 sessionId, uint32 sourceCharId, uint32 destCharId)
{
	//H_AUTO(R2_CClientEditionModule_requestTeleportOneCharacterToAnother)
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.teleportOneCharacterToAnother(this, (TSessionId)sessionId, sourceCharId, destCharId);
	return true;
}


void CClientEditionModule::requestUpdateRtScenario( CObject* scenario)
{
	//H_AUTO(R2_CClientEditionModule_requestUpdateRtScenario)
	//	CSerialFactoryBackup fb;
	CMessage message ("requestUpdateRtScenario");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CObjectSerializerServer obj(scenario);
	obj.compress();
	message.serial(obj);

	_ServerEditionProxy->sendModuleMessage(this, message );

}


void CClientEditionModule::requestCreatePrimitives()
{
	//H_AUTO(R2_CClientEditionModule_requestCreatePrimitives)
	CMessage message ("DBG_CREATE_PRIMITIVES");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	_ServerEditionProxy->sendModuleMessage(this, message );
}


void CClientEditionModule::requestStopTest()
{
	//H_AUTO(R2_CClientEditionModule_requestStopTest)
	CMessage message ("STOP_TEST");
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	_ServerEditionProxy->sendModuleMessage(this, message );
}




bool CClientEditionModule::requestUploadScenario(CObject* scenario)
{
	//H_AUTO(R2_CClientEditionModule_requestUploadScenario)
	//	CSerialFactoryBackup fb;
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	CObjectSerializerServer body(scenario);
	body.compress();
	/*
	proxy.onScenarioUploadAsked(this, body, true);
*/

	uint32 messageId = _ServerAnswerForseener->onScenarioUploaded( scenario);
	NLNET::CMessage msg;
	sendMsgToDss(CShareServerEditionItfProxy::buildMessageFor_onScenarioUploadAsked(msg, messageId, body, true));
	return true;
}


void CClientEditionModule::requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CClientEditionModule_requestSetNode)
	if (value)
	{
		CObject *clObj = getEditor().getDMC().find(instanceId, attrName);
		if (clObj && clObj->getGhost())
		{
			value->setGhost(true);
		}
		if (value->getGhost())
		{
			// this is a local value -> forward directly to client
			CObject *temp = value->clone();
			getEditor().getDMC().nodeSet(instanceId, attrName, temp);
			delete temp; // AJM
			return;
		}
	}
	if (!attrName.empty())
	{
		CObject* instance = _Scenario->find(instanceId);

		if (instance)
		{
			CObject* property = _PropertyAccessor->getPropertyValue(instance, attrName);
			if (property && property->equal(value))
			{
				//nlinfo("R2Cl: Optimisation(message not send)");
				return;
			}
		}
	}

	requestSetNodeNoTest(instanceId, attrName, value);
}


void CClientEditionModule::requestSetNodeNoTest(const std::string& instanceId, const std::string& attrName, CObject* value)
{
	//H_AUTO(R2_CClientEditionModule_requestSetNodeNoTest)
	//	CSerialFactoryBackup fb;
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	CObjectSerializerServer value2(value);
	value2.compress();
	uint32 messageId = _ServerAnswerForseener->onNodeSet(instanceId, attrName, value);
	proxy.onNodeSetAsked(this, messageId, instanceId, attrName, value2);
}


void CClientEditionModule::requestEraseNode( const std::string& instanceId, const std::string& attrName, sint32 position)
{
	//H_AUTO(R2_CClientEditionModule_requestEraseNode)
	CObject *clObj = getEditor().getDMC().find(instanceId, attrName, position);
	if (clObj && clObj->getGhost())
	{
		// this is a ghost value -> forward directly to client
		getEditor().getDMC().nodeErased(instanceId, attrName, position);
		return;
	}

//	CSerialFactoryBackup fb;
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	uint32 messageId = _ServerAnswerForseener->onNodeErased(instanceId, attrName, position);
	proxy.onNodeEraseAsked(this, messageId, instanceId, attrName, position);
}

void CClientEditionModule::requestInsertNode(const std::string& instanceId, const std::string& attrName, sint32 position, const std::string& key, CObject* value)
{
	//H_AUTO(R2_CClientEditionModule_requestInsertNode)
	if (value)
	{
		// if value is inserted in a ghost node, then it inherits the 'ghost' flag
		CObject *clObj = getEditor().getDMC().find(instanceId, attrName, -1);
		if (clObj && clObj->getGhost())
		{
			value->setGhost(true);
		}
		if (value->getGhost())
		{
			// this is a ghost value -> forward directly to client
			getEditor().getDMC().nodeInserted(instanceId, attrName, position, key, value->clone());
			return;
		}
	}
	//	CSerialFactoryBackup fb;
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	CObjectSerializerServer value2(value);
	value2.compress();
	uint32 messageId = _ServerAnswerForseener->onNodeInserted(instanceId, attrName, position, key, value);
	proxy.onNodeInsertAsked(this, messageId, instanceId, attrName, position, key, value2);
}


void CClientEditionModule::requestMoveNode(
			const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& destInstanceId, const std::string& destAttrName, sint32 destPosition)
{
	//H_AUTO(R2_CClientEditionModule_requestMoveNode)
	CObject *src = getEditor().getDMC().find(instanceId, attrName, position);
	CObject *dest = getEditor().getDMC().find(destInstanceId, destAttrName);
	if (src && dest)
	{
		nlassert(src->getGhost() == dest->getGhost());
		if (src->getGhost())
		{
			// this is a ghost value -> forward directly to client
			getEditor().getDMC().nodeMoved(instanceId, attrName, position,
										   destInstanceId, destAttrName, destPosition);
			return;
		}
	}
	if (src) nlassert(!src->getGhost());
	if (dest) nlassert(!dest->getGhost());
//	CSerialFactoryBackup fb;

	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	uint32 messageId = _ServerAnswerForseener->onNodeMoved(instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
	proxy.onNodeMoveAsked(this, messageId, instanceId, attrName, position, destInstanceId, destAttrName, destPosition);
}


void CClientEditionModule::requestMapConnection( uint32 scenarioId, bool mustTp, bool mustUpdateHighLevel)
{
	//H_AUTO(R2_CClientEditionModule_requestMapConnection)
	//	CSerialFactoryBackup fb;
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.onMapConnectionAsked(this, (TSessionId)scenarioId, mustTp, mustUpdateHighLevel,  R2::TUserRole::ur_editor);
}


void CClientEditionModule::requestReconnection()
{
	//H_AUTO(R2_CClientEditionModule_requestReconnection)
	if (_SessionId.asInt() != 0)
	{
		this->requestMapConnection(_SessionId.asInt(), false, false);
	}
}


CScenario* CClientEditionModule::getCurrentScenario() const {return _Scenario;	}


void CClientEditionModule::addPaletteElement(const std::string& attrName, CObject* paletteElement)
{
	//H_AUTO(R2_CClientEditionModule_addPaletteElement)
	_Palette->addPaletteElement(attrName, paletteElement);
}

bool CClientEditionModule::isInPalette(const std::string& key) const
{
	//H_AUTO(R2_CClientEditionModule_isInPalette)
	return _Palette->isInPalette(key);
}

CObject* CClientEditionModule::getPropertyValue(CObject* component, const std::string& attrName) const
{
	//H_AUTO(R2_CClientEditionModule_getPropertyValue)
	return _PropertyAccessor->getPropertyValue(component, attrName);
}

CObject* CClientEditionModule::getPropertyValue(const std::string& instanceId, const std::string& attrName) const
{
	//H_AUTO(R2_CClientEditionModule_getPropertyValue)
	CObject* component = _Scenario->find(instanceId);
	if (!component) return 0;
	return _PropertyAccessor->getPropertyValue(component, attrName);
}

CObject* CClientEditionModule::getPropertyList(CObject* component) const
{
	//H_AUTO(R2_CClientEditionModule_getPropertyList)
	typedef std::list<std::string> TContainer;

	TContainer properties;

	_PropertyAccessor->getPropertyList(component, properties);
	TContainer::const_iterator first(properties.begin()), last(properties.end());
	for ( ; first != last; ++first)
	{
		component->add(new CObjectString(*first));
	}
	return component;
}

CObject* CClientEditionModule::getPaletteElement(const std::string& key)const
{
	//H_AUTO(R2_CClientEditionModule_getPaletteElement)
	return _Palette->getPaletteElement(key);
}


CObject* CClientEditionModule::newComponent(const std::string& type) const
{
	//H_AUTO(R2_CClientEditionModule_newComponent)
	return _Factory->newComponent(type);
}

void CClientEditionModule::registerGenerator(CObject* classObject)
{
	//H_AUTO(R2_CClientEditionModule_registerGenerator)
	_Factory->registerGenerator(classObject);
}

CPropertyAccessor& CClientEditionModule::getPropertyAccessor() const
{
	//H_AUTO(R2_CClientEditionModule_getPropertyAccessor)
	nlassert(_PropertyAccessor);
	return *_PropertyAccessor;
}

void CClientEditionModule::updateScenario(CObject* scenario)
{
	//H_AUTO(R2_CClientEditionModule_updateScenario)
	nlassert(_Scenario);
	_Scenario->setHighLevel(scenario);
	std::string eid = getEid();
	_Factory->setMaxId(eid, _Scenario->getMaxId(eid));
}

void CClientEditionModule::setEid(const std::string& eid)
{
	//H_AUTO(R2_CClientEditionModule_setEid)
	_Eid = eid;
	_Factory->setPrefix(_Eid);
}


bool CClientEditionModule::askUpdateCharMode(R2::TCharMode mode)
{
	//H_AUTO(R2_CClientEditionModule_askUpdateCharMode)
	//	CSerialFactoryBackup fb;
	if (_ServerEditionProxy == NULL) { return false; };

	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.onCharModeUpdateAsked(this, mode);
	return true;
}


void CClientEditionModule::onCharModeUpdated(NLNET::IModuleProxy * /* senderModuleProxy */, R2::TCharMode mode)
{
	//H_AUTO(R2_CClientEditionModule_onCharModeUpdated)
	_CharMode = mode;
	if(UserEntity)
		UserEntity->setR2CharMode(mode);

}



void CClientEditionModule::startScenario(class NLNET::IModuleProxy * proxy, bool ok, uint32 /* startingAct */, const std::string& errorReason)
{
	//H_AUTO(R2_CClientEditionModule_startScenario)
	if (ok)
	{
		if (_SessionType == st_edit)
		{
			_Client->onTestModeConnected();
			R2::getEditor().setMode(CEditor::DMMode);
		}
		else if(_SessionType == st_anim)
		{

			this->connectAnimationModePlay();

			CEditor::connexionMsg("");

			if (_CharMode == TCharMode::Dm)
			{
				R2::getEditor().setMode(CEditor::AnimationModeDm);
				askMissionItemsDescription();
			}
			else if (_CharMode == TCharMode::Tester || _CharMode == TCharMode::Player)
			{
				R2::getEditor().setMode(CEditor::AnimationModePlay);
			}
			else
			{
				nlwarning("Error Mode not handled %d", _CharMode.getValue());
				R2::getEditor().setMode(CEditor::AnimationModePlay);
			}
		}
		//:TODO: must tp?
	}
	else
	{
		CEditor::connexionMsg("uiR2EDR2StartTestError");
		requestReconnection();
	}

	if (!ok && errorReason.empty())
	{
		systemMsg(proxy, "ERR", "", errorReason);
	}
}



void CClientEditionModule::startingScenario(class NLNET::IModuleProxy * /* serverProxy */, uint32 charId)
{
	//H_AUTO(R2_CClientEditionModule_startingScenario)
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	bool ok = false;
	CObjectSerializerServer hlData;
	CObjectSerializerServer rtData;


	if (_Scenario)
	{
		// Some times save is refused: be NevraxScenario must not be changed
		{
			CScenarioValidator sv;
			CScenarioValidator::TValues values;
			std::string md5, signature;
			R2::getEditor().getLua().executeScriptNoThrow("r2.Translator.initStartingActIndex()");

			sv.setScenarioToLoad("save/r2_buffer.dat", values, md5, signature, true);
			_LastReadHeader = values;
			uint32 lastActIndex = _StartingActIndex;
			CObject* hlScenario2 =  _Client->getComLuaModule().loadLocal("save/r2_buffer.dat", _LastReadHeader);
			_StartingActIndex  = lastActIndex;
			if (hlScenario2)
			{
				_Factory->clear();
				_Client->scenarioUpdated(hlScenario2, false, lastActIndex);
			}
			_StartingActIndex  = lastActIndex;


		}


		uint32 myUserId = NetMngr.getUserId();
		std::string connectionState;

		if (myUserId == (charId>>4) || ClientCfg.Local)
		{

			std::string errorMsg;

			CObject* hlScenario = _Scenario->getHighLevel();
			hlData.setData(hlScenario); // clone before modify by translateFeatures
			// translateFeatures change _StartingActIndex

			_Factory->setMaxId("RtAct", 0);
			_Factory->setMaxId("RtAiState", 0);
			_Factory->setMaxId("RtNpcGrp", 0);
			_Factory->setMaxId("RtNpcEventHandlerAction", 0);
			_Factory->setMaxId("RtNpcEventHandler", 0);
			_Factory->setMaxId("RtLocation", 0);
			_Factory->setMaxId("RtTextManager", 0);
			_Factory->setMaxId("RtScenario", 0);
			_Factory->setMaxId("RtUserTrigger", 0);
			_Factory->setMaxId("RtScenario", 0);
			_Factory->setMaxId("RtEntryText", 0);
			_Factory->setMaxId("RtPlotItem", 0);

			std::auto_ptr<CObject> rtDataPtr(  _Client->getComLuaModule().translateFeatures(hlScenario , errorMsg) );
			rtData.setData(rtDataPtr.get());

			if (rtDataPtr.get())
			{
				ok = true;
				connectionState = "uiR2EDUploadScenario";
			}
			else
			{
				nlwarning("%s",errorMsg.c_str());
				connectionState = "uiR2EDR2StartTestError";
			}

			TScenarioSessionType sessionType = _SessionType;

			if (ok && sessionType == st_anim)
			{
				hlData.compress();

				NLNET::CMessage msg;
				uint32 messageId = _ServerAnswerForseener->onScenarioUploaded( hlScenario);
				sendMsgToDss(CShareServerEditionItfProxy::buildMessageFor_onScenarioUploadAsked(msg, messageId, hlData, false));
			}

			rtData.compress();

			TScenarioHeaderSerializer header(_LastReadHeader);
			{
				NLNET::CMessage msg;
				sendMsgToDss(CShareServerEditionItfProxy::buildMessageFor_startScenario(msg, ok, header, rtData, _StartingActIndex));
			}
		}
		else
		{
			connectionState = "uiR2EDR2WaitUploadScenario";
		}

		if (_SessionType == st_edit)
		{
			_Client->onEditionModeDisconnected();
			R2::getEditor().setMode(CEditor::GoingToDMMode);
		}
		else if (_SessionType == st_anim)
		{
			_Client->onEditionModeDisconnected();
			R2::getEditor().setMode(CEditor::AnimationModeGoingToDm);

		}



		CEditor::connexionMsg(connectionState);

	}

}

bool CClientEditionModule::requestStartScenario()
{
	//H_AUTO(R2_CClientEditionModule_	)

	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CEditor::connexionMsg("uimR2EDGoToDMMode");

	R2::getEditor().getLua().executeScriptNoThrow("r2.Version.save(\"save/r2_buffer.dat\")");
	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.startingScenario(this);
	return true;
}


void CClientEditionModule::updateUserComponentsInfo(const std::string & filename, const std::string& name, const std::string & description, uint32 timestamp, const std::string& md5hash)
{
	//H_AUTO(R2_CClientEditionModule_updateUserComponentsInfo)
	TUserComponents::iterator found = _UserComponents.find(filename);
	if (found == _UserComponents.end())
	{
		nlwarning("Error: try to update information on a unknown component '%s'", filename.c_str() );
		return;
	}
	found->second->Name = name;
	found->second->Description = description;
	found->second->TimeStamp = timestamp;
	found->second->Md5Id.fromString( md5hash );
}


void CClientEditionModule::registerUserComponent(const std::string& filename)
{
	//H_AUTO(R2_CClientEditionModule_registerUserComponent)
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);

	TUserComponents::iterator found = _UserComponents.find(filename);
	if (found == _UserComponents.end())
	{
		nlwarning("Error: try to upload unknown component '%s'", filename.c_str() );
		return;
	}

	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.onUserComponentRegistered(this, found->second->Md5);
}


CUserComponent* CClientEditionModule::getUserComponentByHashMd5( const NLMISC::CHashKeyMD5 & md5) const
{
	//H_AUTO(R2_CClientEditionModule_getUserComponentByHashMd5)

	TUserComponents::const_iterator first(_UserComponents.begin()), last(_UserComponents.end());
	for (; first != last &&  (first->second->Md5 != md5) ; ++first ) {}
	if (first == last)
	{
		nlwarning("Error: try to upload unknown component '%s'", md5.toString().c_str() );
		return 0;
	}

	return  first->second;
}


void CClientEditionModule::onUserComponentUploading(NLNET::IModuleProxy * /* senderModuleProxy */, const CHashKeyMD5 & md5)
{
	//H_AUTO(R2_CClientEditionModule_onUserComponentUploading)
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);

	CUserComponent* userComponent = getUserComponentByHashMd5(md5);

	if (userComponent)
	{
		NLNET::CMessage message;
		message.setType("SUCU"); //onUserComponentUploaded
		userComponent->serial(message);
		_ServerEditionProxy->sendModuleMessage(this, message);
	}

	return;
}


void CClientEditionModule::onUserComponentRegistered(NLNET::IModuleProxy * /* senderModuleProxy */, const CHashKeyMD5 & md5)
{
	//H_AUTO(R2_CClientEditionModule_onUserComponentRegistered)
	BOMB_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CUserComponent* userComponent = getUserComponentByHashMd5(md5);
    if (!userComponent)
	{
		CShareServerEditionItfProxy proxy(_ServerEditionProxy);
		proxy.onUserComponentDownloading(this, md5);
	}
	else
	{
		std::string filename = userComponent->getFilename();
		R2::getEditor().getLua().executeScriptNoThrow(NLMISC::toString("r2.UserComponentsManager:addUserComponent('%s')", filename.c_str()));
	}
}


void CClientEditionModule::onUserComponentDownloaded(NLNET::IModuleProxy *senderModuleProxy, CUserComponent* component)
{
	//H_AUTO(R2_CClientEditionModule_onUserComponentDownloaded)

	// case 1 data are compressed
	component->UncompressedData = new uint8[component->UncompressedDataLength + 1];
	uLongf dataLength = static_cast<uLongf>(component->UncompressedDataLength);
	sint32 decompressionState = uncompress (reinterpret_cast<Bytef*>(component->UncompressedData), &dataLength ,
		reinterpret_cast<Bytef*>(component->CompressedData), component->CompressedDataLength);

	component->UncompressedDataLength = static_cast<uint32>(dataLength);

	if (decompressionState != Z_OK)
	{
		delete  component;
		nlwarning("Error: the downloaded user component is corrupted '%s' ", component->Filename.c_str());
		return;
	}
	component->UncompressedData[component->UncompressedDataLength] = '\0';

	// insert user component into user components map

	TUserComponents::iterator found = _UserComponents.find(component->Filename);
	if (found != _UserComponents.end())
	{
		delete found->second;
		found->second = component;
	}

	_UserComponents[component->Filename] = component;


	saveUserComponentFile(component->Filename, true);

	onUserComponentRegistered(senderModuleProxy, component->Md5);


}



bool CClientEditionModule::loadUserComponent(const std::string& filename, bool mustReload)
{
	//H_AUTO(R2_CClientEditionModule_loadUserComponent)
	if (! mustReload)
	{
		TUserComponents::const_iterator  found = _UserComponents.find(filename);
		if (found != _UserComponents.end())
		{
			return true;;
		}
	}

	std::string sourceExtension = UserComponentsSourceExtension;
	std::string componentExtension = UserComponentsComponentExtension;


	uint32 uncompressedFileLength = 0;
	uint8* uncompressedFile=0;
	uint32 compressedFileLength = 0;
	uint8* compressedFile=0;
	bool compressed = false;
	bool ok = false;

	if (CFile::getExtension(filename) == sourceExtension)
	{
		compressed = false;
		ok = true;
	}
	else if (CFile::getExtension(filename) == componentExtension)
	{
		compressed = true;
		ok = true;
	}
	if (!ok)
	{
		nlwarning("Wrong file extension '%s'", filename.c_str() );
		nlwarning("Allowed file extension '%s' '%s'", sourceExtension.c_str(), componentExtension.c_str());
		return false;
	}

	CHashKeyMD5 md5Id;
	uint32 timeStamp = 0;
	if (! compressed)
	{
		FILE* file = fopen(filename.c_str(),"rb");
		if (!file)
		{
			nlwarning("Try to open an invalid file %s (access error)", filename.c_str());
			return false;
		}

		// file length are the last uint32 of a file
		if (fseek(file,0, SEEK_END) != 0)
		{
			nlwarning("Try to open an invalid file %s (size error)", filename.c_str());
			fclose(file);
			return false;
		}
		uncompressedFileLength = ftell(file); // size of file


		if (fseek(file, 0, SEEK_SET) != 0)
		{
			nlwarning("Try to open an invalid file %s (size error)", filename.c_str());
			fclose(file);
			return false;
		}

		uncompressedFile = new uint8[uncompressedFileLength];
		int length = (int)fread(uncompressedFile, sizeof(char), uncompressedFileLength, file);
		if (length <0)
		{
			nlwarning("Error while reading %s", filename.c_str());
			delete[] uncompressedFile;
			fclose(file);
			return false;

		}

		if ( length < static_cast<sint32>(uncompressedFileLength))
		{
			nlwarning("Error while reading %s (corrupted data)", filename.c_str());
			delete[] uncompressedFile;
			fclose(file);
			return false;
		}

		fclose(file);


		// Test if data are not too big
		if (uncompressedFileLength > 200*1024)
		{
			nlwarning("Try to open an invalid file %s (size error)", filename.c_str());
			delete [] uncompressedFile;
			return false;
		}

		std::string compiledInfoHeader = "--COMPONENT HEADER\n";
		std::string compiledInfoFooter = "--COMPONENT BODY\n";

		// REMOVE HEADER
		std::string data((const char*)&uncompressedFile[0], (const char*)&uncompressedFile[uncompressedFileLength]);
		std::string::size_type start = data.find(compiledInfoHeader);
		if ( start != std::string::npos)
		{
			std::string::size_type finish = data.find(compiledInfoFooter, start +  compiledInfoHeader.length());
			if (finish != std::string::npos)
			{
				finish += compiledInfoFooter.size();
				data = data.substr(0, start) + data.substr(finish, data.size() - finish);
			}
		}

		// REGENERATE HEADER
		md5Id =	getMD5((uint8*)data.data(), (uint32)data.size());
		timeStamp = NLMISC::CTime::getSecondsSince1970();

		//std::stringstream ss;
		//ss << compiledInfoHeader;
		//ss << "local fileinfo = {}\n";
		//ss << toString("fileinfo.Package='%s'\n", filename.c_str());
		//ss << toString("fileinfo.Version='%s'\n", "1");
		//ss << toString("fileinfo.MD5='%s'\n", md5Id.toString().c_str());
		//ss << toString("fileinfo.TimeStamp='%u'\n",  timeStamp);
		//ss << toString("r2.UserComponentsManager:registerFileInfo(fileinfo)\n");
		//ss << compiledInfoFooter;
		//ss << data;

		//data = ss.str();

		std::string str;
		str += compiledInfoHeader;
		str += "local fileinfo = {}\n";
		str += toString("fileinfo.Package='%s'\n", filename.c_str());
		str += toString("fileinfo.Version='%s'\n", "1");
		str += toString("fileinfo.MD5='%s'\n", md5Id.toString().c_str());
		str += toString("fileinfo.TimeStamp='%u'\n",  timeStamp);
		str += toString("r2.UserComponentsManager:registerFileInfo(fileinfo)\n");
		str += compiledInfoFooter;

		data = str + data;

		delete [] uncompressedFile;
		uncompressedFile = new uint8[ data.size() ];
		memcpy(uncompressedFile, data.c_str(), data.size());
		uncompressedFileLength = (uint32)data.size();
	}
	else
	{
		// Get Uncompressed File length (4 last byte of a gz)
		FILE* file = fopen(filename.c_str(),"rb");
		if (!file)
		{
			nlwarning("Try to open an invalid file %s (access error)", filename.c_str());
			return false;
		}

		// file length are the last uint32 of a file
		if (fseek(file, -4, SEEK_END) != 0)
		{
			nlwarning("Try to open an invalid file %s (size error)", filename.c_str());
			fclose(file);
			return false;
		}

		if (fread((void*)&uncompressedFileLength,  sizeof(uncompressedFileLength),  1, file) != 1)
		{
			nlwarning("Error while reading %s", filename.c_str());
		}

		fclose(file);

		// Test if data are not too big
		if (uncompressedFileLength > 200*1024)
		{
			nlwarning("Try to open an invalid file %s (size error)", filename.c_str());
			delete [] compressedFile;
			return false;
		}

		// Read the compressed File
		{
			gzFile file = gzopen  ( filename.c_str(), "rb");
			uncompressedFile = new uint8[uncompressedFileLength+1];

			int length = gzread(file, uncompressedFile, uncompressedFileLength);
			if (length <0)
			{
				nlwarning("Error while reading %s", filename.c_str());
				delete [] uncompressedFile;
				delete [] compressedFile;
				gzclose(file);
				return false;

			}

			if ( length < static_cast<sint32>(uncompressedFileLength))
			{
				nlwarning("Error while reading %s (corrupted data)", filename.c_str());
				delete [] uncompressedFile;
				delete [] compressedFile;
				gzclose(file);
				return false;
			}
			gzclose(file);
		}
		// Read the compressed File

	}

	delete  [] compressedFile; compressedFile = 0;

	//size of the destination buffer, which must be at least 0.1% larger than sourceLen plus 12 bytes

	{
		uLongf destLen = uncompressedFileLength + uncompressedFileLength / 1000 + 12;
		Bytef *dest = new Bytef[destLen];
		int ok = compress(dest, &destLen, (Bytef *)uncompressedFile, uncompressedFileLength);
		if (ok != Z_OK)
		{
			delete [] uncompressedFile;
			delete [] compressedFile;
			nlwarning("Error while reading %s (can't compress data)", filename.c_str());
			return false;
		}
		compressedFile = reinterpret_cast<uint8*>(dest);
		compressedFileLength = static_cast<uint32>(destLen);

		uncompressedFile[uncompressedFileLength] = '\0';
	}

	// TODO: compute md5Id and timeStamp

	_UserComponents[filename] = new CUserComponent(filename, uncompressedFile, uncompressedFileLength, compressedFile, compressedFileLength);
	_UserComponents[filename]->Md5Id = md5Id;
	_UserComponents[filename]->TimeStamp = timeStamp;

	return true;
}

std::string CClientEditionModule::readUserComponentFile(const std::string& filename)
{
	//H_AUTO(R2_CClientEditionModule_readUserComponentFile)

	if (!loadUserComponent(filename, false))
	{
		return "";
	}

	CUserComponent* component = getUserComponentByFilename(filename);


	if (component)
	{
		const char * str = (const char*)component->getUncompressedData();
		std::string value(str);
		return str;
	}
	return "";
}

CUserComponent* CClientEditionModule::getUserComponentByFilename(const std::string& filename) const
{
	//H_AUTO(R2_CClientEditionModule_getUserComponentByFilename)
	TUserComponents::const_iterator  found = _UserComponents.find(filename);
	if (found != _UserComponents.end())
	{
		return found->second;
	}
	return 0;
}

void CClientEditionModule::saveUserComponentFile(const std::string& filename, bool mustCompress)
{
	//H_AUTO(R2_CClientEditionModule_saveUserComponentFile)

	bool ok = loadUserComponent(filename);

	if (ok)
	{
		CUserComponent* component = getUserComponentByFilename(filename);


		std::string compressedName;
		std::string uncompressedName;


		if (CFile::getExtension(filename) == UserComponentsSourceExtension.toString())
		{
			uncompressedName = filename;
			compressedName = toString("%s/%s.%s",
				UserComponentsComponentsDirectory.c_str(),
				component->Md5Id.toString().c_str(),
				UserComponentsComponentExtension.c_str());
		}
		else if (CFile::getExtension(filename) == UserComponentsComponentExtension.toString())
		{
			compressedName = filename;
			uncompressedName = toString("%s/%s.%s",
				UserComponentsExamplesDirectory.c_str(),
				CFile::getFilenameWithoutExtension(component->Filename).c_str(),
				UserComponentsSourceExtension.c_str());
		}


		if (!mustCompress)
		{
			{
				FILE* output = fopen(uncompressedName.c_str(), "wb");
				if (output)
				{
					fwrite(component->UncompressedData, sizeof(char) , component->UncompressedDataLength, output);
					fclose(output);
				}
			}
		}
		else
		{
			{
				gzFile output = gzopen(compressedName.c_str(), "wb");
				if (output)
				{
					gzwrite(output,  (const voidp) component->UncompressedData,  component->UncompressedDataLength);
					gzclose(output);
				}
			}
		}
	}
}

void CClientEditionModule::refreshComponents()
{
	//H_AUTO(R2_CClientEditionModule_refreshComponents)
/*
	// verify directory exist
	bool createDirectorySuccess = false;
	if ( !CFile::isDirectory(_ScriptDirectory))
	{
		createDirectorySuccess = CFile::createDirectory(_ScriptDirectory);
	}

	// verify content of directory has not changed
	uint32 lastDirectoryModificationDate = CFile::getFileModificationDate(const std::string &filename);
	if (_LastDirectoryModificationDate == lastDirectoryModificationDate )
	{
		return;
	}
	_LastDirectoryModificationDate = lastDirectoryModificationDate

	// get Developper Component Script;
	std::vector<std::string> result;
	CPath::getPathContent (_ScriptDirectory, false, false, true, result, false, false);

	CInterfaceManager	*pIM= CInterfaceManager::getInstance();
	std::vector<std::string> script(1);
	bool ok = pIM->pars  veInterface(script,    true,    false);


	_LastRefreshComponents = NLMISC::CTime::getLocalTime();
	*/
}

void CClientEditionModule::ackMsg( NLNET::IModuleProxy *sender, uint32 msgId, bool ok)
{
 	_ServerAnswerForseener->ack(this, sender, msgId, ok);
}

void CClientEditionModule::onScenarioUploaded(NLNET::IModuleProxy * /* sender */, const R2::CObjectSerializerClient &hlScenario)
{
	//H_AUTO(R2_CClientEditionModule_onScenarioUploaded)
	_Factory->clear();
	_Client->scenarioUpdated(hlScenario.getData(), false, 1);//give ownership
}

// The client request to set a node on a hl scenario.
void CClientEditionModule::onNodeSet(NLNET::IModuleProxy * /* sender */, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value)
{
	//H_AUTO(R2_CClientEditionModule_onNodeSet)
	if (_Mute) return;
	_Client->nodeSet(instanceId, attrName, value.getData()); //todo ownership
}

// The ServerEditionMode inserts a node on a hl scenario.
void CClientEditionModule::onNodeInserted(NLNET::IModuleProxy * /* sender */, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value)
{
	//H_AUTO(R2_CClientEditionModule_onNodeInserted)
	if (_Mute) return;
	_Client->nodeInserted( instanceId, attrName, position, key, value.getData()); //todo ownership
}

// The ServerEditionMode erases a node on a hl scenario.
void CClientEditionModule::onNodeErased(NLNET::IModuleProxy * /* sender */, const std::string &instanceId, const std::string &attrName, sint32 position)
{
	//H_AUTO(R2_CClientEditionModule_onNodeErased)
	if (_Mute) return;
	_Client->nodeErased( instanceId, attrName, position);
}

// The ServerEditionMode a move node on a hl scenario.
void CClientEditionModule::onNodeMoved(NLNET::IModuleProxy * /* sender */, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2)
{
	//H_AUTO(R2_CClientEditionModule_onNodeMoved)
	if (_Mute) return;
	_Client->nodeMoved(instanceId1, attrName1, position1,
			instanceId2, attrName2, position2);
}

void CClientEditionModule::onQuotaUpdated(NLNET::IModuleProxy * /* senderModuleProxy */, uint32 maxNpcs, uint32 maxStaticObjects)
{
	//H_AUTO(R2_CClientEditionModule_onQuotaUpdated)
	//R2::getEditor().getLua().executeScriptNoThrow(toString("r2.QuotaMgr.onQuotaUpdated(%u, %u)", maxNpcs, maxStaticObjects));
	_MaxNpcs = maxNpcs;
	_MaxStaticObjects = maxStaticObjects;

}

uint32 CClientEditionModule::getCurrentMaxId()
{
	//H_AUTO(R2_CClientEditionModule_getCurrentMaxId)
	if (_Scenario == NULL)
		return 1000;
	std::string eid = getEid();
	sint32 currentId = _Factory->getMaxId(eid);
	nlassert(currentId >= -1);

	return static_cast<uint32>(currentId + 1);
}


void CClientEditionModule::reserveIdRange(uint32 range)
{
	//H_AUTO(R2_CClientEditionModule_reserveIdRange)
	nlassert(_Scenario);
	std::string eid = getEid();
	sint32 currentId = _Scenario->getMaxId(eid);
	nlassert(currentId >= -1);

	sint32 maxId = currentId + static_cast<sint32>(range + 1);

	//_Scenario->setMaxId(eid, maxId);
	_Factory->setMaxId(eid, maxId);
}

std::string CClientEditionModule::getEmoteBehaviorFromEmoteId(const std::string & emoteId) const
{
	//H_AUTO(R2_CClientEditionModule_getEmoteBehaviorFromEmoteId)
	return _Emotes->get(emoteId);
}




void CClientEditionModule::resetDisplayInfo()
{
	//H_AUTO(R2_CClientEditionModule_resetDisplayInfo)
	this->_ClientEditorConfig->setDisplayInfo(0xFFFFFFFF);
}

void CClientEditionModule::setDisplayInfo(const std::string& formName, bool displayInfo)
{
	//H_AUTO(R2_CClientEditionModule_setDisplayInfo)
	this->_ClientEditorConfig->setDisplayInfo(formName, displayInfo);
	//CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	//uint32 newDisplayInfo = this->_ClientEditorConfig->getDisplayInfo();
	//proxy.setDisplayInfo(this, newDisplayInfo);
}


bool CClientEditionModule::mustDisplayInfo(const std::string& formName) const
{
	//H_AUTO(R2_CClientEditionModule_mustDisplayInfo)
	bool ok = this->_ClientEditorConfig->mustDisplayInfo(formName);
	return ok;
}

bool CClientEditionModule::hasDisplayInfo(const std::string& formName) const
{
	//H_AUTO(R2_CClientEditionModule_hasDisplayInfo)
	bool ok = this->_ClientEditorConfig->hasDisplayInfo(formName);
	return ok;
}

void CClientEditionModule::onDisplayInfoUpdated(NLNET::IModuleProxy * /* senderModuleProxy */, uint32 displayInfo)
{
	//H_AUTO(R2_CClientEditionModule_onDisplayInfoUpdated)
	this->_ClientEditorConfig->setDisplayInfo(displayInfo);
}


void CClientEditionModule::setStartingActIndex(uint32 startingActIndex)
{
	//H_AUTO(R2_CClientEditionModule_setStartingActIndex)
	_StartingActIndex = startingActIndex;
}

void CClientEditionModule::onTpPositionSimulated(NLNET::IModuleProxy * /* sender */,  TSessionId /* sessionId */, uint64 /* characterId64 */, sint32 x, sint32 y, sint32 z, uint8 /* scenarioSeason */)
{
	//H_AUTO(R2_CClientEditionModule_onTpPositionSimulated)

	CVector dest((float)x, (float)y, (float) z);
	//Season ?

	UserEntity->pos(dest); // change position in pacs
	// Select the closest continent from the new position.
	beginLoading (LoadingBackground);
	#define BAR_STEP_TP 2 // fixme : this define is duplicated....
	ProgressBar.reset (BAR_STEP_TP);
	ucstring nmsg("Loading...");
	ProgressBar.newMessage ( ClientCfg.buildLoadingString(nmsg) );
	ProgressBar.progress(0);
	ContinentMngr.select(dest, ProgressBar);
	endLoading();
	// Teleport the User.
	UserEntity->tp(dest);
	ProgressBar.finish();

}

void CClientEditionModule::onDisconnected(NLNET::IModuleProxy * /* sender */)
{
	//H_AUTO(R2_CClientEditionModule_onDisconnected)
	R2::getEditor().getLua().executeScriptNoThrow(NLMISC::toString("r2.onDisconnected()"));
}

void CClientEditionModule::onKicked(NLNET::IModuleProxy * /* sender */, uint32 timeBeforeDisconnection, bool mustKick)
{
	//H_AUTO(R2_CClientEditionModule_onKicked)

	R2::getEditor().getLua().push((double)timeBeforeDisconnection);
	R2::getEditor().getLua().push((bool)mustKick);
	R2::getEditor().callEnvFunc( "onKicked", 2, 0);

}


// Target : Animation Module

void CClientEditionModule::requestTalkAs(const std::string& npcname)
{
	//H_AUTO(R2_CClientEditionModule_requestTalkAs)
	CMessage msg("talk_as");
	msg.serial(const_cast<std::string&>(npcname));
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CClientEditionModule::requestStringTable()
{
	//H_AUTO(R2_CClientEditionModule_requestStringTable)
	CMessage msg("requestStringTable");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CClientEditionModule::requestSetStringValue(std::string& id,std::string& value )
{
	//H_AUTO(R2_CClientEditionModule_requestSetStringValue)
	CMessage msg("requestSetValue");
	msg.serial(id);
	msg.serial(value);
	_ServerAnimationProxy->sendModuleMessage(this,msg);
}

void CClientEditionModule::requestStartAct(uint32 actId)
{
	//H_AUTO(R2_CClientEditionModule_requestStartAct)
	CMessage message ("requestStartAct");
	message.serial(actId);
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CClientEditionModule::requestSetWeather(uint16 weatherValue)
{
	//H_AUTO(R2_CClientEditionModule_requestSetWeather)
	CMessage message ("requestSetWeather");
	message.serial(weatherValue);
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CClientEditionModule::requestSetSeason(uint8 seasonValue)
{
	//H_AUTO(R2_CClientEditionModule_requestSetSeason)
	CMessage message ("requestSetSeason");
	message.serial(seasonValue);
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CClientEditionModule::requestStopAct()
{
	//H_AUTO(R2_CClientEditionModule_requestStopAct)
	CMessage message ("requestStopAct");
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	_ServerAnimationProxy->sendModuleMessage(this, message );
}

void CClientEditionModule::requestStopTalkAs()
{
	//H_AUTO(R2_CClientEditionModule_requestStopTalkAs)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);

	CMessage msg("stopTalk");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}

void CClientEditionModule::requestStringValue(std::string& localId )
{
	//H_AUTO(R2_CClientEditionModule_requestStringValue)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	CMessage msg("requestStringValue");
	msg.serial(localId);
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}
void CClientEditionModule::requestTpPosition(float x, float y, float z)
{
	//H_AUTO(R2_CClientEditionModule_requestTpPosition)
	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);
	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.onTpPositionAsked(this, x, y, z);
	return ;
}


void CClientEditionModule::requestIdList()
{
	//H_AUTO(R2_CClientEditionModule_requestIdList)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);
	CMessage msg("requestIdList");
	_ServerAnimationProxy->sendModuleMessage(this, msg );
}



bool  CClientEditionModule::requestTpToEntryPoint(uint32 actIndex)
{
	//H_AUTO(R2_CClientEditionModule_requestTpToEntryPoint)
	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);
	if (R2::getEditor().isClearingContent()) { return true;}

	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.tpToEntryPoint(this, actIndex);
	return true;
}


bool  CClientEditionModule::requestSetStartingAct(uint32 actIndex)
{
	//H_AUTO(R2_CClientEditionModule_requestSetStartingAct)
	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.setStartingAct(this, actIndex);
	return true;
}



bool CClientEditionModule::connectAnimationModePlay()
{
	//H_AUTO(R2_CClientEditionModule_connectAnimationModePlay)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return false);

	CShareServerAnimationItfProxy serverAnimationModule(_ServerAnimationProxy);
	serverAnimationModule.connectAnimationModePlay(this);
	return true;
}


void CClientEditionModule::onAnimationModePlayConnected(NLNET::IModuleProxy * /* senderModuleProxy */)
{
	//H_AUTO(R2_CClientEditionModule_onAnimationModePlayConnected)
	_Client->onAnimationModePlayConnected();
}


void CClientEditionModule::scheduleStartAct(NLNET::IModuleProxy * /* sender */, uint32 errorId, uint32 actId, uint32 nbSeconds)
{
	//H_AUTO(R2_CClientEditionModule_scheduleStartAct)
	R2::getEditor().getLua().push((double)errorId);
	R2::getEditor().getLua().push((double)actId);
	R2::getEditor().getLua().push((double)nbSeconds);
	R2::getEditor().callEnvFunc( "onScheduleStartAct", 3, 0);

}


void CClientEditionModule::updateScenarioHeader(NLNET::IModuleProxy * /* sender */, const TScenarioHeaderSerializer& header)
{
	//H_AUTO(R2_CClientEditionModule_updateScenarioHeader)
	_ScenarioHeader = header.Value;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onScenarioHeaderUpdated(r2.getScenarioHeader())" );
}

void CClientEditionModule::updateMissionItemsDescription(NLNET::IModuleProxy * /* sender */, TSessionId /* sessionId */, const std::vector<R2::TMissionItem> &missionItem)
{
	//H_AUTO(R2_CClientEditionModule_updateMissionItemsDescription)
	uint i;
	uint maxNumPlotItems = CEditor::getMaxNumPlotItems();
	for(i = 0; i < std::min((uint) missionItem.size(), maxNumPlotItems); ++i)
	{
		CCDBNodeLeaf *leaf = CEditor::getPlotItemSheetDBLeaf(i);
		if (leaf)
		{
			uint32 sheetId = (uint32) missionItem[i].SheetId.asInt();
			leaf->setValue32(sheetId);
			getEditor().setPlotItemInfos(missionItem[i]);
		}
	}
	for(; i < maxNumPlotItems; ++i)
	{
		CCDBNodeLeaf *leaf = CEditor::getPlotItemSheetDBLeaf(i);
		if (leaf)
		{
			leaf->setValue32(0);
		}
	}
}

void CClientEditionModule::updateActPositionDescriptions(NLNET::IModuleProxy * /* sender */, const TActPositionDescriptions &actPositionDescriptions)
{
	//H_AUTO(R2_CClientEditionModule_updateActPositionDescriptions)
	this->_ActPositionDescriptions = actPositionDescriptions;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onRuntimeActUpdated(r2.getRuntimeActs())" );

}

void CClientEditionModule::updateUserTriggerDescriptions(NLNET::IModuleProxy * /* sender */,  const TUserTriggerDescriptions &userTriggerDescriptions)
{
	//H_AUTO(R2_CClientEditionModule_updateUserTriggerDescriptions)
	this->_UserTriggerDescriptions = userTriggerDescriptions;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onUserTriggerDescriptionUpdated(r2.getUserTriggers())" );
}


bool CClientEditionModule::askMissionItemsDescription()
{
	//H_AUTO(R2_CClientEditionModule_askMissionItemsDescription)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return false);
	CShareServerAnimationItfProxy serverAnimationModule(_ServerAnimationProxy);
	serverAnimationModule.askMissionItemsDescription(this);
	return true;
}

bool CClientEditionModule::requestTriggerUserTrigger(uint32 actId, uint triggerId)
{
	//H_AUTO(R2_CClientEditionModule_requestTriggerUserTrigger)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return false);
	CShareServerAnimationItfProxy serverAnimationModule(_ServerAnimationProxy);
	serverAnimationModule.onUserTriggerTriggered(this, actId, triggerId);
	return true;
}
void CClientEditionModule::onCurrentActIndexUpdated(NLNET::IModuleProxy * /* sender */, uint32 actId)
{
	//H_AUTO(R2_CClientEditionModule_onCurrentActIndexUpdated)
	this->_CurrentActIndex = actId;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onCurrentActIndexUpdated(r2.getCurrentActIndex())" );
}


void CClientEditionModule::dssTarget( std::vector<std::string>& args)
{
	//H_AUTO(R2_CClientEditionModule_dssTarget)
	DROP_IF(_ServerAnimationProxy == NULL, "Server Animation Module not connected", return);

	CShareServerAnimationItfProxy serverAnimationModule(_ServerAnimationProxy);
	serverAnimationModule.onDssTarget(this, args);
}


void CClientEditionModule::updateIncarningList(NLNET::IModuleProxy * /* sender */, const std::vector<uint32> & botId)
{
	//H_AUTO(R2_CClientEditionModule_updateIncarningList)
	this->_IncarnatingList = botId;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onIncarnatingListUpdated()" );
}

void CClientEditionModule::updateTalkingAsList(NLNET::IModuleProxy * /* sender */, const std::vector<uint32> & botId)
{
	//H_AUTO(R2_CClientEditionModule_updateTalkingAsList)
	this->_TalkingAsList = botId;
	R2::getEditor().getLua().executeScriptNoThrow( "r2.onTalkingAsListUpdated()" );
}

std::vector<uint32>	CClientEditionModule::getIncarnatingList() const
{
	//H_AUTO(R2_CClientEditionModule_getIncarnatingList)
	return this->_IncarnatingList;
}

std::vector<uint32>	CClientEditionModule::getTalkingAsList() const
{
	//H_AUTO(R2_CClientEditionModule_getTalkingAsList)
	return this->_TalkingAsList;
}


void CClientEditionModule::systemMsg(NLNET::IModuleProxy * /* sender */,  const std::string& msgType, const std::string& who, const std::string& msg)
{
	//H_AUTO(R2_CClientEditionModule_systemMsg)
	R2::getEditor().getLua().push(msgType);
	R2::getEditor().getLua().push(who);
	R2::getEditor().getLua().push(msg);
	R2::getEditor().callEnvFunc("onSystemMessageReceived", 3 , 0);
}


void CClientEditionModule::updateScenarioRingAccess(bool ok, const std::string& ringAccess, const std::string& errMsg)
{
	//H_AUTO(R2_CClientEditionModule_updateScenarioRingAccess)

	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);

	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.onScenarioRingAccessUpdated(this, ok, ringAccess, errMsg);
}

std::string CClientEditionModule::getCharacterRingAccess() const
{
	//H_AUTO(R2_CClientEditionModule_getCharacterRingAccess)
	return _RingAccess;
}

void CClientEditionModule::onRingAccessUpdated(NLNET::IModuleProxy * /* moduleSocket */, const std::string& ringAccess)
{
	//H_AUTO(R2_CClientEditionModule_onRingAccessUpdated)

	_RingAccess = ringAccess;
	//Remove first and last char
	if (!_RingAccess.empty())
	{
		if (_RingAccess[0] == '\'' || _RingAccess[0] == '"')
		{
			_RingAccess = _RingAccess.substr(1);
		}
	}
	if (!_RingAccess.empty())
	{
		if (_RingAccess[_RingAccess.size()-1] == '\''
			|| _RingAccess[_RingAccess.size()-1] == '"')
		{
			_RingAccess = _RingAccess.substr(0, _RingAccess.size()-1);
		}
	}
	R2::getEditor().getLua().push(_RingAccess);
	R2::getEditor().callEnvFunc( "onRingAccessUpdated",1 ,0);
}


/*
*/
void CClientEditionModule::addToSaveList(const std::string& filename, const std::vector< std::pair < std::string, std::string> >& values)
{
	//H_AUTO(R2_CClientEditionModule_addToSaveList)


	std::string name = filename; // Must use prefix or suffix?

	NLMISC::COFile out;

	CObject* scenario =  getCurrentScenario()->getHighLevel();

	if (!scenario)
	{
		nlwarning("Can't save: no scenario yet");
		return;
	}

	CScenarioValidator* sv = new CScenarioValidator();
	CScenarioValidator::TValues v1(values);
	std::string md5;
	bool ok = true;
	ok = sv->setScenarioToSave(filename, scenario, v1, md5);
	if (!ok)
	{
		delete sv;
	}
	else
	{
		if ( !_ScenarioToSave.insert( std::make_pair(md5, sv)).second )
		{
			nlwarning("Ask 2 time the save of the same scenario?");
			delete sv;
			return;
		}
		DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);

		CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
		serverEditionModule.saveScenarioFile(this, md5 ,R2::TScenarioHeaderSerializer (v1) );

	}

	// :TODO: VIANNEY add all
	_LastReadHeader.clear();
	_LastReadHeader.push_back( std::pair<std::string, std::string>("BodyMD5", sv->getBodyMd5() ));
	_LastReadHeader.insert(_LastReadHeader.end(), values.begin(), values.end());



//	sv.openHeader(filename, v2, true);
	return;
}

/**********************************************************/
/****** ADD TO USER COMPONENT SAVE LIST *******************/
/**********************************************************/
void CClientEditionModule::addToUserComponentSaveList(const std::string& filename, const std::vector< std::pair < std::string, std::string> >& values, std::string &body)
{
	//H_AUTO(R2_CClientEditionModule_addToSaveList)


	std::string name = filename; // Must use prefix or suffix?

	NLMISC::COFile out;

	CUserComponentValidator* ucv = new CUserComponentValidator();
	CUserComponentValidator::TValues v1(values);
	std::string md5;
	bool ok = true;
	ok = ucv->setUserComponentToSave(filename, v1, md5, body);
	if (!ok)
	{
		delete ucv;
	}
	else
	{
		if ( !_UserComponentToSave.insert( std::make_pair(md5, ucv)).second )
		{
			nlwarning("Ask 2 time the save of the same user component?");
			delete ucv;
			return;
		}
		DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return);

		CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
		serverEditionModule.saveUserComponentFile(this, md5 ,R2::TScenarioHeaderSerializer (v1) );

	}

	return;
}

void CClientEditionModule::saveUserComponentFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, const std::string& signature, bool ok)
{
	//H_AUTO(R2_CClientEditionModule_saveScenarioFileAccepted)

	TUserComponentToSave::iterator found(_UserComponentToSave.find(md5));
	if (found == _UserComponentToSave.end())
	{
		nlwarning("The client has ask more than one time to save the same file");
		return;
	}

	CUserComponentValidator* userComponentToSave = found->second;
	_UserComponentToSave.erase(found);

	if (ok)
	{
		userComponentToSave->applySave(signature);
		addToUserComponentLoadList(userComponentToSave->getFilename(), new CLoadUserComponentSucceeded(this));
	}
	else
	{
		systemMsg(senderModuleProxy, "ERR", "", "uiR2EDServerRefuseToSave");
	}

}


bool CClientEditionModule::addToUserComponentLoadList( const std::string& filename, CUserComponentValidatorLoadSuccededCallback* cb)
{
	//H_AUTO(R2_CClientEditionModule_addToLoadList)
	CUserComponentValidator* ucv = new CUserComponentValidator(cb);
	CUserComponentValidator::TValues values;
	std::string md5;
	std::string signature;
	bool ok = true;
	nlinfo("Adding user component '%s' to load list", filename.c_str());
	ok = ucv->setUserComponentToLoad(filename, values, md5, signature, ClientCfg.CheckR2ScenarioMD5);
	if (!ok)
	{
		R2::getEditor().callEnvFunc( "displayModifiedUserComponentFileError", 0, 0);
		nlwarning("setUserComponentToLoad failed");
		return false;
	}

	if ( !_UserComponentToLoad.insert( std::make_pair(md5, ucv)).second )
	{
		nlwarning("Ask 2 time the load of the same user component?"); //or the same empty uc file
		delete ucv;
		return false;
	}
	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.loadUserComponentFile(this, md5, signature);
	return true;
}


void CClientEditionModule::loadUserComponentFileAccepted(NLNET::IModuleProxy * /* senderModuleProxy */, const std::string& md5, bool ok)
{
	//H_AUTO(R2_CClientEditionModule_loadScenarioFileAccepted)

	TUserComponentToLoad::iterator found(_UserComponentToLoad.find(md5));
	if (found == _UserComponentToLoad.end())
	{
		nlwarning("The client has ask more than one time to save the same file");
		return;
	}

	auto_ptr<CUserComponentValidator> userComponentToLoad(found->second);
	_UserComponentToLoad.erase(found);

	if (!ok)
	{
		nlwarning("The server has refuse the client to load the a file"); //should (can) not happen
		this->systemMsg(0, "ERR", "", "The server has refuse the client to load the a file");
		return;
	}

	std::string filename;
	std::string body;
	CUserComponentValidator::TValues values;
	userComponentToLoad->applyLoad(filename, body, values);
	_LastReadHeader = values;
	//return body;

}

/**************************************************************************************/

void CClientEditionModule::saveScenarioFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, const std::string& signature, bool ok)
{
	//H_AUTO(R2_CClientEditionModule_saveScenarioFileAccepted)

	TScenarioToSave::iterator found(_ScenarioToSave.find(md5));
	if (found == _ScenarioToSave.end())
	{
		nlwarning("The client has ask more than one time to save the same file");
		return;
	}

	auto_ptr<CScenarioValidator> scenarioToSave(found->second);
	_ScenarioToSave.erase(found);

	if (ok)
	{
		scenarioToSave->applySave(signature);
	}
	else
	{
		systemMsg(senderModuleProxy, "ERR", "", "uiR2EDServerRefuseToSave");
	}

}

bool CClientEditionModule::addToLoadList( const std::string& filename, CScenarioValidatorLoadSuccededCallback* cb)
{
	//H_AUTO(R2_CClientEditionModule_addToLoadList)
	CScenarioValidator* sv = new CScenarioValidator(cb);
	CScenarioValidator::TValues values;
	std::string md5;
	std::string signature;
	bool ok = true;
	ok = sv->setScenarioToLoad(filename, values, md5, signature, ClientCfg.CheckR2ScenarioMD5);
	if (!ok){ return false; }

	if ( !_ScenarioToLoad.insert( std::make_pair(md5, sv)).second )
	{
		nlwarning("Ask 2 time the load of the same scenario?"); //or the same empty scenario
		delete sv;
		return false;
	}
	DROP_IF(_ServerEditionProxy == NULL, "Server Edition Module not connected", return false);

	CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
	serverEditionModule.loadScenarioFile(this, md5, signature);
	return true;
}


void CClientEditionModule::loadScenarioSucceded(const std::string& filename, const std::string& body, const CScenarioValidator::TValues& values)
{
	//H_AUTO(R2_CClientEditionModule_loadScenarioSucceded)
	string initialIsland, initialEntryPoint, initialSeason;
	string creatorMD5, modifiedByMD5;
	string name;
	string locked;
	for(uint i=0; i<values.size(); i++)
	{
		const std::pair<std::string, std::string>& pair = values[i];
		if(pair.first == "InitialIsland")	initialIsland = pair.second;
		else if(pair.first == "InitialEntryPoint")	initialEntryPoint = pair.second;
		else if(pair.first == "InitialSeason")	initialSeason = pair.second;
		else if(pair.first == "OwnerMd5")	initialSeason = pair.second;
		else if(pair.first == "CreatorMD5")	creatorMD5 = pair.second;
		else if(pair.first == "OtherCharAccess")	locked = pair.second;
		else if(pair.first == "ModifierMD5")	modifiedByMD5 = pair.second;
		else if(pair.first == "Name") name = pair.second;


	}

	if (!modifiedByMD5.empty() && !locked.empty() && locked == "RunOnly")
	{
		bool ok = hasCharacterSameCharacterIdMd5(modifiedByMD5);
		if (!ok)
		{
			this->systemMsg(0, "ERR", "", "uiR2EDLoadingLockedScenario");
			return;
		}
	}

	R2::getEditor().getUI().displaySystemInfo(CI18N::get("uiR2EDLoadingScenario"), "BC");
	CObject* object = _Client->getComLuaModule().loadFromBuffer(body, filename, values);
	_LastReadHeader = values;
	if (object)
	{
		if (!initialIsland.empty() && !initialEntryPoint.empty() && !initialSeason.empty())
		{
//			CShareServerEditionItfProxy serverEditionModule(_ServerEditionProxy);
//	 			serverEditionModule.teleportWhileUploadingScenario(this, initialIsland, initialEntryPoint, initialSeason);
			R2::getEditor().clearContent();
		}
		_Client->requestUploadScenario(object);
		if (!name.empty() && object && object->isTable() && object->isString("InstanceId"))
		{
			CObjectString objectName(name);
			_Client->requestSetNode(object->toString("InstanceId"), "Ghost_Name", &objectName);
		}

	}
	if (CFile::fileExists(filename))
	{
		CFile::copyFile("save/r2_buffer.dat", filename.c_str());
	}
}


void CClientEditionModule::loadAnimationSucceded(const std::string& filename, const std::string& body, const CScenarioValidator::TValues& values)
{
	//H_AUTO(R2_CClientEditionModule_loadAnimationSucceded)
	std::string errMsg;
	_Client->loadAnimationFromBuffer(body, filename, errMsg, values);
}

void CClientEditionModule::loadScenarioFileAccepted(NLNET::IModuleProxy * /* senderModuleProxy */, const std::string& md5, bool ok)
{
	//H_AUTO(R2_CClientEditionModule_loadScenarioFileAccepted)

	TScenarioToLoad::iterator found(_ScenarioToLoad.find(md5));
	if (found == _ScenarioToLoad.end())
	{
		nlwarning("The client has ask more than one time to save the same file");
		return;
	}

	auto_ptr<CScenarioValidator> scenarioToLoad(found->second);
	_ScenarioToLoad.erase(found);

	if (!ok)
	{
		nlwarning("The server has refuse the client to load the a file"); //should (can) not append
		this->systemMsg(0, "ERR", "", "The server has refuse the client to load the a file");
		return;
	}

	std::string filename;
	std::string body;
	CScenarioValidator::TValues values;
	scenarioToLoad->applyLoad(filename, body, values);
	_LastReadHeader = values;

}

void CClientEditionModule::setMute(bool mute)
{
	//H_AUTO(R2_CClientEditionModule_setMute)
	_Mute = mute;
}

void CLoadUserComponentSucceeded::doOperation(const std::string& filename,const std::string& body,const CScenarioValidator::TValues& header)
{
	CLuaState& state = R2::getEditor().getLua();
	state.push((std::string)filename);
	state.push((std::string)body);

	state.newTable();
	CClientEditionModule::TScenarioHeader::const_iterator first(header.begin()), last(header.end());

	for ( ; first != last; ++first)
	{

		state.push(first->first);
		state.push(first->second);

		state.setTable(-3);
	}
	//R2::getEditor().getLua().push((bool)mustKick);
	R2::getEditor().callEnvFunc( "loadUserComponentCallback", 3, 0);
}



void CLoadAnimationSucceded::doOperation(const std::string& filename,const std::string& body,const CScenarioValidator::TValues& values)
{
	//H_AUTO(R2_CLoadAnimationSucceded_doOperation)
	_Module->loadAnimationSucceded(filename, body, values);
}

void CLoadScenarioSucceded::doOperation(const std::string& filename,const std::string& body,const CScenarioValidator::TValues& values)
{
	//H_AUTO(R2_CLoadScenarioSucceded_doOperation)
	_Module->loadScenarioSucceded(filename, body, values);
}


void CClientEditionModule::resetNameGiver()
{
	//H_AUTO(R2_CClientEditionModule_resetNameGiver)
	_Factory->clear();
}

bool CClientEditionModule::hasCharacterSameCharacterIdMd5(const std::string & charIdMd5) const
{
	if (ClientCfg.Local == 1 )
	{
		return true;
	}

	uint32 val = 0;
	uint32 first=0,last=(uint32)charIdMd5.size();
	for (;first != last; ++first)
	{

		val *= 16;
		char c = charIdMd5[first];
		if ('0' <= c && c <= '9')
		{
			val += (c-'0');
		}
		else if ('A' <= c && c <= 'F')
		{
			val += 10+(c-'A');
		}
	}
	uint32 charId = CRingAccess::uncypherCharId(val);

	uint32 myUserId = NetMngr.getUserId();
	if ((charId>>4) != myUserId)
	{

		return false;
	}
	return true;
}

class CModuleMessageSender : public R2::IMessageSender
{
public:
	CModuleMessageSender(const NLNET::TModuleProxyPtr& proxy, NLNET::IModule* senderModule):_Proxy(proxy), _Sender(senderModule){}

	void operator()(const NLNET::CMessage & msg)
	{
		if (!_Proxy)
		{
			nlwarning("Can not send Message %s to destination", msg.getName().c_str());
			return;
		}

		// Send msg to DSS (after cuting msg)
		if (ClientCfg.R2EDDssNetwork == 1)
		{
			_Proxy->sendModuleMessage(_Sender, msg);
			return;
		}

		TCharId charId = 0;
		if (ClientCfg.Local )
		{
			charId = 999 << 4;
		}
		else
		{
			charId = CSessionBrowserImpl::getCharId();
		}


		// Send msg to DSS (Simulate forward system)
		if (ClientCfg.R2EDDssNetwork == 2)
		{

			NLNET::CMessage handle;
			R2::CShareServerEditionItfProxy::buildMessageFor_forwardToDss(handle, charId, msg);
			_Proxy->sendModuleMessage(_Sender, handle);
			return;
		}


		// Send msg to DSS (Simulate forward system)
		if (ClientCfg.R2EDDssNetwork == 3)
		{

			NLNET::CMessage handle;
			R2::CShareServerEditionItfProxy::buildMessageFor_forwardToDss(handle, charId, msg);
			CSessionBrowserImpl::getInstance().send(handle);
			return;
		}
		// wrong value
		nlwarning("Error Msg lost");
		return;
	}
protected:
	NLNET::TModuleProxyPtr _Proxy;
	NLNET::IModule* _Sender;
};


void CClientEditionModule::sendMsgToDss(const NLNET::CMessage& msg)
{

 	TCharId charId = 0;
	if (ClientCfg.Local )
	{
		charId = 999 << 4;
	}
	else
	{
		charId = CSessionBrowserImpl::getCharId();
	}
	// Direct send msg to DSS (not cut)
	if (ClientCfg.R2EDDssNetwork == 0)
	{
		_ServerEditionProxy->sendModuleMessage(this, msg);
		return;
	}

	CModuleMessageSender sender(_ServerEditionProxy, this);
	CMessageSpliter::sendSplitedMsg(charId, msg, sender);
}


//----------------------------------------------------------------------------------------------------------------





void CMessageSpliter::sendSplitedMsg(uint32 charId, const NLNET::CMessage& msg, IMessageSender& sender)
{

	static const uint32 packetSize = 512; // 16K by packet
	uint32 size = msg.length();
	uint32 nbPacket = 1 + (size-1) / packetSize;

	// :TODO: use the brodcast form of the message instead of manual message


	nldebug("Sending DSS message");

	{

		NLNET::CMessage subMsg;
		R2::CShareServerEditionItfProxy::buildMessageFor_multiPartMsgHead(subMsg, charId, msg.getName(), nbPacket, size);

		// dssMsgForward
		sender(subMsg);



	}
	const uint8* buffer = msg.buffer();


	uint32 i=0;
	for (; i != nbPacket; ++i)
	{


		// send packetSize octet except for the last packet that just send the remainings data
		uint32 dataSend = (i+1)*packetSize > size ? size - packetSize*i:packetSize;

		std::vector<uint8> data(const_cast<uint8*>(&buffer[i*packetSize]),  const_cast<uint8*>(&buffer[i*packetSize + dataSend]) );
		NLNET::CMessage subMsg;
		R2::CShareServerEditionItfProxy::buildMessageFor_multiPartMsgBody(subMsg, charId, i,  data);


		sender(subMsg);

		// TODO -> add callback send( (size - packetSize*i) / size)
	}

	//
	{
		NLNET::CMessage subMsg;
		R2::CShareServerEditionItfProxy::buildMessageFor_multiPartMsgFoot(subMsg, charId);

		sender(subMsg);

	}
}

// Messages from dss that notify that he has received data
// We received first a multiPartMsgHead, then X multiPartMsgBody, then one multiPartMsgFoot
void CClientEditionModule::multiPartMsgHead(NLNET::IModuleProxy * /* sender */,  const std::string& msgName, uint32 nbPacket, uint32 size)
{
	R2::getEditor().getLua().executeScriptNoThrow( NLMISC::toString("r2:onMessageSendingStart('%s', %u, %u)",msgName.c_str(), nbPacket, size ) );
}

void CClientEditionModule::multiPartMsgBody(NLNET::IModuleProxy * /* sender */, uint32 packetId, uint32 packetSize)
{
	R2::getEditor().getLua().executeScriptNoThrow( NLMISC::toString("r2:onMessageSendingUpdate(%u, %u)", packetId,  packetSize));
}

void CClientEditionModule::multiPartMsgFoot(NLNET::IModuleProxy * /* sender */)
{
	R2::getEditor().getLua().executeScriptNoThrow( NLMISC::toString("r2:onMessageSendingFinish()"));
}

