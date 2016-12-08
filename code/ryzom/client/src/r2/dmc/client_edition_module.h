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

#ifndef R2_CLIENT_EDITON_MODULE_H
#define R2_CLIENT_EDITON_MODULE_H

#include "nel/misc/types_nl.h"

#include "nel/net/module.h"
#include "nel/misc/md5.h"
#include "game_share/r2_share_itf.h"
#include "game_share/dyn_chat.h"
#include "game_share/scenario.h"

#include <memory>


namespace R2
{
	class CObject;
	class CDynamicMapService;
	class CDynamicMapClient;
	class CPalette;
	class CTranslationModule;
	class CScenario;
	class CObjectFactory;

	class CPropertyAccessor;
	class CUserComponent;
	class CEmoteBehavior;
	class CEditorConfig;
	class CScenarioValidator;
	class CUserComponentValidator;
	class CServerAnswerForseener;



/*
Small class used to Send information in multiPacket
*/

class IMessageSender
{
public:
	virtual void operator()(const NLNET::CMessage& msg) = 0;
	virtual ~IMessageSender(){}
};


class CMessageSpliter
{
public:
	static void sendSplitedMsg(uint32 charId, const NLNET::CMessage& msg, IMessageSender& sender);

};




/**
The main goal of this class is to communicates with the DSS, so the CEditor class will no need to know specific server communication.
This class hold the scenario that is synchronised with DSS. This class is also used to transform High Level edition data to data that can be understand by ais.

I) Data
Most of edition data are kept in a the Scenario Tree. This tree contains nodes that are made with a generic data structure. This data are easy to changes and are light to send
to network (generic compression and application specific compression)

	Scenario Tree

- _Scenario  (CScenario)
The main data tree is the CScenario class. It contains high level data (scenario / act / components) that are updated, changed by CEditor / lua code.
He contains animation data (that are very similar to primitive data) that is use to run animation (to feed ais).
The lua function r2:translator is use to creat animation data from High level data.
It contains the notion of palette that is use to insert new element.
It contains an InstanceMap to give quick access to tree node.
This tree is synchronized between client and server. In a simplified model, client would send a message to server that would change his data tree
then call back the client that would do the same update in order to keep boot data synchronized.


	Data Node

- CObject
- CObjectString
- CObjectNumber
- CObjectTable
This generic data structure is used to manipulate data, it's look like (number, string , table).
You can add, element, remove element, move element, insert element in the table.
You can set value of an element.


	  Compression

- CSerializerClass
- CObjectSerializerServer
CObjectSerializerServer is used to compress data in order that the serial function to be the quickest possible.
The function CObjectSerializerServer::compress must call to use generic (zlib) compression. The uncompress function is call when read function is needed.
Other data specific compression is done via serial function. We can define SerializerClass for specific Node. Inded if we know needed property then it is not useful to send property name in data stream.
If we know optional property name we only send the index in  optional property table. We only send the property name if we do not know by the property name.
Other optimisation are done. we use the shortest type that can hold the data (unsigned char, short, fixed point variable...)
To have C++ CSerializerClass synchronized with lua element is needed in order to keep data the shortest. It is not the case now so data could be optimized ( divided by 3 bandwith?).

- CScenario


II) Data modification

Data BandwidthThe edition tree is synchronized between the client and the server. Bandwidth, genericity  and speed of reaction was kept in mind during the conception.

	Simple Node Modification:

- requestSetNode
- requestSetNodeNoTest
- requestEraseNode
- requestInsertNode
- requestMoveNode
This function are called by CEditor. CEditor modify its local tree then send the modification to server via these functions.
If we change a Ghost Node. We forward the call directly to CEditor that apply the modification.
If we change a Node with the same value then the messge is not send (bandwith optimisation).

- CServerAnswerForseener::onNode(Set|Erased|Inserted)
We create an unique MessageId for each message we send via CServerAnswerForseener that cached data.

- ackMsg
- onScenarioUploaded
- onNodeSet
- onNodeInserted
- onNodeErased
- onNodeMoved
We send a message to the Server. If the node modification is accepted by the server. The server send an ACK to the client that has send the initial message.
( so the client can apply the message with _ServerAnswerForseener backuped message). The server forward the message to other client connected to edition.

- setMute
Because multiEdition is disable, most of the time the server answer is not listen. We use the setMute function to not listen server result.


  Heavy modifications:
- requestUploadScenario(CObject* scenario);
If we want to update the scenario (uploading of scenario from file, start an animation). Uploading the scenario would be to heavy if use classic communication (via FE).
Inded FE has limited band width.

- sendMsgToDss
The solution is to cut packets via sendMsgToDss This functions cut the message then send it to dss via SBS.
The SBS has no bandwidth limitation.

- multiPartMsgHead
- multiPartMsgBody
- multiPartMsgFoot
Dss calls the client back to let him know the percentage of data received (multiPartMsgHead, multiPartMsgBody, multiPartMsgFoot).
Then C++ calls lua  back with  "r2:onMessageSending(Start|Update|Finish)". Lua print a string that show the percentage of data upload (must be changed to show download bar).

III) Communication Client/Server
Modification is done between client and server by differents ways.
- CModule communication (hand writtedn message and generated messges)
- Modification of Database.



	CModule communication

CClientEditionModule is a Nel module (it inherits from NLNET::CModuleBase) so can know if other module are up or down
- CClientEditionModule: the Client module (use to communicates with the DSS)

The client can know when DSS Module are up or down  _ServerEditionProxy and _ServerAnimationProxy are used as proxy to the dss updated by this functions.
- onModuleDown, onModuleUp: enable to know if a module is up or down, update the state of _ServerEditionProxy and _ServerAnimationProxy

The client communicates with two dss modules: ServerEditionModule, ServerAnimationModule. He use _ServerEditionProxy and _ServerAnimationProxy has proxy to communicates with the dss..
- "ServerEditionModule": data tree modification, main dss functionalities
- "ServerAnimationModule": mainly DM functions

Hand written function are received by onProcessModuleMessage (this is old messages that have not been updated to the new module system like "ADV_CONN" message)
- onProcessModuleMessage : handle hand written message received by client
Main messages are
- "ADV_CONN": full update to a scenario. (connection to a new session or the client chose to change the editor mode edition, test...)
- "HELLO" : the DSS open its firewall so client is allowed to speak to him.
- other obsolete message like string table manipulation

Communication between CClientEditionModule and CServerEditionModule is mainly done via generated module messages.
We use the file "r2_share/r2_share_itf.xml" to add / remove communication.
This file is used to generated "game_share/r2_share_itf.h" and "r2_share/r2_share_itf.cpp".
CClientEditionModule inherits from CShareClientEditionItf and Server(Edition|Animation)Module inherit from CShareServer(Edition|Animation)Itf.
Itf files are definied in r2_share/r2_share_itf.h.
f we want to add a method that can be call by server via ModuleProxy we had it in CShareClientEditionItf, if we want to add a function called by client in DSS
we add it in CShareServerAnimationtf or CShareServeerEditionItf.
e.g.
<module_interface name="CShareServerEditionItf" module_class='"ServerEditionModule"'>
	<method name="onNodeEraseAsked" msg="SNEA">
		<doc line="The client request to erase a node on a hl scenario."/>
		<param type="uint32"		name="msgId"/>
		<param type="std::string" name="instanceId" byref="true"/>
		<param type="std::string" name="attrName" byref="true"/>
		<param type="sint32" name="position"/>
	</method>
</module_interface>

Client would send message to dss via
  	CShareServerEditionItfProxy proxy(_ServerEditionProxy);
	proxy.onNodeEraseAsked(this, messageId, instanceId, attrName, position);

CServerEditionModule must implement the function to handle the client (and also to compile because this function is defined as pure virtual from the generated class he inherits)
virtual void onNodeEraseAsked(NLNET::IModuleProxy *senderModuleProxy,  uint32 messageId,
			const std::string& instanceId, const std::string& attrName, sint32 position)

IV) Connection to Edition


V) From Edition to test

We have seen previously how to create / update scenario.
First we ask the server the permission to startTheScenaro
- requestStartScenario: Ask the server the permission to start a scenario
The dss answer to all client connected to the scenario with the startingScenario function that has as parameter the charId of the client that has send the request.
- startingScenario: server indicates to all client connected that the scenario is about to start so client must go to test mode
If the client is not the one that has send the request then the editor quit the edition modes to go to test mode (in a waiting for secenario state)
If the client is the one that has send the request he begins the translation, then start the translation of the scenario then the upload
- CComLuaModule::loadLocal load the current scenario from the saved file
- CComLuaModule::translateFeatures execute the translation process that create RtData
- CShareServerEditionItfProxy::startScenario upload the RtData and ask the server to start the scenario
When the Dss receive the startScenario message he broadcast to other connecteds client
- startScenario: call by dss to signify the result of a CShareServerEditionItfProxy::startScenario return false in case of failure e.g. to much entities in the scene

When the user want to go from test back to edition he must first stop the current test (depop entites)
- requestStopTest: ask the dss  to depop entities
Then you have to ask Reconnection to the edition session.
-


VI) Test / DM functions

During animation session or test mode. The DM has access to some "DM" function.
He can tp himself to a specific position (also accessible in edition mode)
- requestTpPosition: teleport the editor / Dm to a specific position
He can trigger "User Triggers" that belongs to the current scenario during edition mode.
- getUserTriggers: get the list of description of different user trigger of the current scenario (animation only function)
- requestTriggerUserTrigger: trigger a user trigger
He can change the current act
- getRuntimeActs() : return act description
- requestStartAct(): ask the DSS to start another act
- updateActPositionDescriptions(): update the description of acts
He can use DM function such teleport himself to another character
- requestTeleportOneCharacterToAnother: teleport the dm to another plyer character.
He can dynamically change the Weather or the Season.
- requestSetWeather: change the current weather at runtime or edit mode
- requestSetSeason: change the current season at runtime or edit mode, need loading screen

 Most of dm function are done via the dssTarget function. Indeed dssTarget without option
Update the dm commands that are enable on the current target.
Tbe bitfield returned let us know if the selected target is a botobject, if it is grouped, if we can incarnate him and so.
- dssTarget: without parameter ask the server to update the list of command that are enable on the selected target
- onProcessModuleMessage with "NPC_APROP" message: return a bitfield that let know animation property of the selected entity
- CDynamicMapClient::onNpcAnimationTargeted: translate the bitfield given by the "NPC_APROP" message to known which are the allowed action
- r2:updateAnimBarActions: called by  CDynamicMapClient::onNpcAnimationTargeted, update the anim bar that contains action that can be done on the selecte npc

The DM can push button on the anim bar to do some DM functions like kill an npc, kill a group, despawn, heal, control, speak as
- dssTarget with a parameter DESPAWN_NPC, ADD_HP, KILL_NPC, ADD_HP, GRP_KILL, GRP_HEAL, CONTROL, STOP_CONTROL, TALK_AS, STOP_TALK let the dm do dm functions.

CONTROL / STOP_CONTROL
The DM can incarnate a npc: in this case he is tp at the npc position, he takes the speed of the npc.
The controlled npc would follow the DM and have the same behavior (emote...)
A DM can control one npc at a time.
- updateIncarningList: called by dss to update the list of npcs incarned by the dm
- dssTarget CONTROL or STOP_CONTROL parameter control / stop the control of one npc by the dm
- getIncarnatingList(): get the list of npc we control as (updated by updateIncarningList)
The teleportation is done in the CComLuaModule::luaDssTarget with "CONTROL" parameter.
It is just a local change of position. Which mean that we change the position of the current player on the client side.
The position is validate by the GPMS because he is friendly in Ring Shard. The client is not ban.
if there is already a npc controled than a message is send with the systemMsg with "uiR2EDAlreadyIncarningANpc" msg
When we control a npc the EGS change the "SERVER:USER:NPC_CONTROL:WALK", "SERVER:USER:NPC_CONTROL:RUN", "SERVER:USER:NPC_CONTROL:SHEET" properties
from the database. So during the mainloop the CUserEntity::updateNpcContolSpeed can change the dm speed.
When a npc is controlled and we can "speak as" the npc. We speak as the npc.
- CServerAnimationModule handle the controlled list (see CServerAnimationModule::setIncarningPlayer)
- Ais handle the notification of the death of bot (useful for updating the list of controlled bot) (see CAisWrapper::askBotDespawnNotification)
- Ais handle the control of a npc by a player (see CAisControl)

TALK_AS / STOP_TALK
When a npc speak as on other entity. A new dynamic channel is created. This new dynamic chat has the name of the controlled npc.
Each Sentence that the npc would say is written in  this dynamic channel.
- dssTarget TALK_AS or STOP_TALK parameter begin / stop the control of npc talk
- getTalkingAsList(): get the list of npc we talk as (updated by updateTalkingAsList)
- updateTalkingAsList: called by dss to update the list of npcs incarned by the dm
- CStringManagerModule handle to the management of channels (see CStringManagerModule::talkAs)
- CServerAnimationModule handle the management of list (CServerAnimationModule::setTalkingAsPlayer)
- Ais handle the notification of the death of bot (useful for updating the list of controlled bot) (see CAisWrapper::askBotDespawnNotification)

ADD_HP / KILL_NPC / ADD_HP / GRP_KILL / GRP_HEAL
It is possible for the Dm to add/remove hp to a group/npc.
- dssTarget with ADD_HP ok KILL_NPC or  GRP_KILL or GRP_HEAL as parameter: heal/kill a npc/group
Note: CServerAnimationModule will call CAiWrapper::setGrpHPLevel, CAiWrapper::setHPLevel that will call Ais native function (nf_npc_grp.cpp)
Theses native functions set the life of a npc.
Its simple to add other AIS native function the same way.

There is some functions that can download the current string table (text that can be said by different npc)
So the dm can change some in order to change what the npc has to say.(b.e. to let the npc to have a more RP text).
Warning! Gui for string table manipulation has not be done. And the code has not be tested.
These functions may be obsolete
- requestStringTable: ask the server to give a table of id of the differents texts that can be said by npcs.
- requestStringValue: get the content of a text by a TextId given by requestStringTable
- requestSetStringValue: set at run time a text said by a npc by its id given by requestStringTable
- requestStopTalkA: stop the control of npc
- onProcessModuleMessage : The return for dss is done via message "stringTable", "stringValue", "id"



VII) From test to edition

First you have to ask the server to stop the test.
- requestStopTest : make the DSS stop the current test (depop entities) use the proxy Command CShareServerEditionItfProxy::stopTestRequested
Then you have to reconnect to edition session. That means that you will change the CEditor mode in order to be able to put new object.
- requestReconnection: re init the editor in order to reload edition data.
The previous function call requestMapConnection. With parameter that says that we do not want to be tp at the entry point of the scenario. And that it is not necessary to download
the scenario because the client stile have it in cache.
- requestMapConnection: ask the server to reset the editor, we can indicates if we want the caller to come back to the start Position. We can also indicates if we want
the server to upload the scenario content ( or if the client use the one he has in cache)
- onAdventureConnected: called by the DSS when the client has done a requestMapConnection. It resest the edition in the correct edition mode. (also call when the client connect to a specific session)


VIII) Save and load

All save, and load of scenarios need to be done with asynchronous Message. Loading of a file, or saving a file can not be done directly because we need the server validation.
Indeed a md5 of the file is done. A hash is done between this md5 and a private key hold by the server. This process generates a signature that is put back in the file.
In order to verify if we are allowed to load a file. The md5, and the signature is send to the DSS to test with its privates key if the signature and
the md5 are valide.
In order to save a file, it is needed to send the md5 the server will return the signature, headers information is also send. Indeed some verifications
are made with this header. A non :DEV: account can not save some its scenario with special options. (Acked client)


Headers infos are info contained in a lua file that can be read without (lua interpretation). This kind of information is realy fast to load.
Save / Loading a file are done by lua function "r2.load", "r2.save".
This functions calls CComLuaModule::load, CComLuaModule::save
The function addToLoadList put the loaded file in memory and ask the dss the permition to load the file.
When the server accept or refuse the loading of the file the callback CScenarioValidatorLoadSuccededCallback is called.
- addToLoadList: add a file to a loading
- CScenarioValidatorLoadSuccededCallback: callback called when loading of a file (from addToLoadList) has been accepted / refuse by server
The server send 2 message loadScenarioFileAccepted, saveScenarioFileAccepted these messages are use to trigg the callback CScenarioValidatorLoadSuccededCallback
loadScenarioFileAccepted: answer from server to indicates if the loading of a file has been accepted (in awnswer from addToLoadList)
saveScenarioFileAccepted: answer from server to indicates if the save of a file has been accepted (in awnswer from addToSaveList)

If an animation scenario is loaded (from ring access poing) loadAnimationSucceded is called.
If an edition scenario is loaded loadScenarioSucceded is called.
- CComLuaModule::luaLoadAnimation call addToLoadList with CLoadAnimationSucceded callback, which call loadAnimationSucceded
- CComLuaModule::luaLoad call addToLoadList with CLoadScenarioSucceded callback, which call loadAnimationSucceded
- CLoadScenarioSucceded and CLoadScenarioSucceded inherits from CScenarioValidatorLoadSuccededCallback
The save process need also to extract header to send them to server for more verification.
- CComLuaModule::luaSave call addToSaveList


IX) Change of State
ClientEditionModule is directly connect to the DSS. This module holds functions that can be call back though DSS will.
One of the aim of this module is to keep state synchronise between client and server.
Data synchronisation:
- onQuotaUpdated: synchronisation of the quota
- updateMissionItemsDescription: synchronisation of mission item descriptions
- updateActPositionDescriptions: synchronisation of Act list (use for tp)
- updateUserTriggerDescriptions: synchronisation of User trigger liste (used for DM to trigger user trigger)
- onCurrentActIndexUpdated: synchronisation of CurrentActIndex (updated when current act changes in edition or animation)
State synchronisation:
- onCharModeUpdated: change of state of the client (loading / save / test ...)
- onDisconnected: the sever chose to disconnect this client
- onKicked: the sever chose to kick this client
- onTestModeDisconnected: the server ask the client to quit the test mode
- onAnimationModePlayConnected: the server inform the client that he must update its GUI to animation mode.

X) Getters
ClientEditionModule communicates with CEditor, or CComLuaModule. He has some getters to let them know
state of variable synchronized with server state.
Sessions infos:
- getSessionId: get the id of the current session
- getAiInstance: get the id of the current AiInstance. One AiInstance is created by session.
- getSessionType: get the type of the current session (edition / animation)
- isSessionOwner: is the player the owner of the current session
- getEditSessionLink: is the current session linked to another session
- getScenarioHeader: get header info of current scenario (animation function). Enable to know divers info set into the header of a scenario.
Quotas infos:
- getMaxNpcs: get the maximal number of npc allowed by scenario.
- getMaxStaticObjects: get the maximal number of scenari object allowed by scenario.
Plot items:
- getSheetIdByPlotItemIndex: get the sheetId of a plot item (animation function)
Internal:
- getMustStartScenario: if true the scenario will start in the next update loop
- getScenarioUpToDate: if false the scenario is not up to date so the update function will be called



XI) Divers
- onTpPositionSimulated
- systemMsg
- hasCharacterSameCharacterIdMd5

XII) Property accessor
	void addPaletteElement(const std::string& attrName, CObject* paletteElement);

	bool isInPalette(const std::string& key) const;

	CObject* getPropertyValue(const std::string& instanceId, const std::string& attrName) const;

	CObject* getPropertyValue(CObject* component, const std::string& attrName) const;

	CObject* getPropertyList(CObject* component) const;

	CObject* getPaletteElement(const std::string& key)const;

	CObject* newComponent(const std::string& type) const;

	void registerGenerator(CObject* classObject);

	CPropertyAccessor& getPropertyAccessor() const;


	/////////////////////////////////////////////////////



*/
class CClientEditionModule : public NLNET::CModuleBase, public CShareClientEditionItfSkel
{
public:
	typedef std::vector< std::pair< std::string, std::string > > TScenarioHeader;

public:
	CClientEditionModule();

	virtual ~CClientEditionModule();

	void init(NLNET::IModuleSocket* clientGW, CDynamicMapClient* client);

	void init();

	void release();


	/////////////////////////////////////////////////////
	//// CModuleBase message
	/////////////////////////////////////////////////////


	//  Module API
	// empty function needded by CModuleBase api
	virtual void onServiceUp(const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */) { }
	virtual void onServiceDown(const std::string &/* serviceName */, NLNET::TServiceId /* serviceId */) {}
	virtual void onModuleUpdate() {}
	virtual void onApplicationExit() { }
	virtual void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);
	virtual void onModuleSocketEvent(NLNET::IModuleSocket * /* moduleSocket */, TModuleSocketEvent /* eventType */) {}
	virtual bool isImmediateDispatchingSupported() const { return false; }


	virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);
	virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);




	//! Data modification - Sending commands
	//! {

	/*! Set a node from the current Scenario (in edition Mode)
	If we update a Ghost Node. The call is forwarded directly to CEditor to apply the update.
	If we update a Node with the same value then the messge is not send (bandwith optimisation).
	\param instanceId The instanceId of the node we want to update e.g. "Client2_1235"
	\param attrName The name of the subAttribut we want to update. Empty string means that we use the node and not a subnode.
	\param value The value we want to set. The function do not take the ownership of the pointer. Most of time CObject static type is CObjectString or CObjectNumber.
	*/
	void requestSetNode(const std::string& instanceId, const std::string& attrName, CObject* value);
	/*! Same as requestSetNode \see requestSetNode() but directyl send the message through network (not Ghost test, no local send)
	*/
	void requestSetNodeNoTest(const std::string& instanceId, const std::string& attrName, CObject* value);


	/*! Erase a node from the current Scenario (in edition Mode)
	If we update a Ghost Node. The call is forwarded directly to CEditor to apply the update.
	\param instanceId The instanceId of the node we want to update e.g. "Client2_1235"
	\param attrName The name of the subAttribut we want to update. Empty string means that we use the node and not a subnode.
	\param position The position of the element we want to select. The value -1 means that we want to select the currrent node and not a sub element of a CObjectStringTable
	*/
	void requestEraseNode(
			const std::string& instanceId, const std::string& attrName, sint32 position);

	/*! Insert a node into the current Scenario (in edition Mode)
	If we update a Ghost Node. The call is forwarded directly to CEditor to apply the update.
	\param instanceId The instanceId of the node we want to update e.g. "Client2_1235"
	\param attrName The name of the subAttribut we want to update. Empty string means that we use the node and not a subnode.
	\param position The position of the element we want to select. The value -1 means that we want to select the current node and not a sub element of a CObjectStringTable
	\param key The name of the attribute we want to add
	\param value The value of the node we want to add (the function does *NOT* take the ownership of the pointer
	eg requestInsertNode("Client1_1", "", -1, "Menu", &CObjectTable()) //add a subnode
	eg requestInsertNode("Client1_1", "Menu", -1, "element2", &CObjectNumber(4)) // inser elment in a subnode
	eg requestInsertNode("Client1_1", "Menu", 1, "element1", &CObjectNumber(4)) // insert element in a subnode at a specific position
	*/
	void requestInsertNode(
			const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& key, CObject* value);

	/*! Move a node form the current Scenario from a position to another (in edition Mode)
	The initial position is defined by instanceId, attrName, position.
	The final position is defined by desInstanceId, destAttrName, destPosition.
	*/
	void requestMoveNode(
			const std::string& instanceId, const std::string& attrName, sint32 position,
			const std::string& desInstanceId, const std::string& destAttrName, sint32 destPosition);

	/*! Update the RtScenario. The RtScenario is the primitive like scenario that is understable by AIS/EGS.
		param  scenario An Object that contains a RtScenario (a RtScenario contains, RtAct, RtState, and RtEventHandlers)
	*/
	virtual void requestUpdateRtScenario( CObject* scenario);
	/*! Update the edition scenario *server side*. The scenario is a simple data structure easy to manipulate by the editor
		\param scenario An Object that contains a RtScenario (a RtScenario contains, RtAct, RtState, and RtEventHandlers)
	*/
	bool requestUploadScenario(CObject* scenario);
	/*! Update the edition scenario *localy*. The scenario is a simple data structure easy to manipulate by the editor
		\param scenario An Object that contains a RtScenario (a RtScenario contains, RtAct, RtState, and RtEventHandlers)
	*/
	void updateScenario(CObject* scenario);
	//! }




	//! Data modification - Receving commands
	//! {
	/*! When received by client this function the function will forseen the answer given by the server to other clients
		like ...onNode(Inserted|Erased|*) or onScenarioUploaded
		\param ok If false remove the action from the queue, if true execute the action hold in a buffer.
		\param messageId The unique message id that enable to execute the buffered message.
	*/
	virtual void ackMsg(NLNET::IModuleProxy *sender, uint32 messageId, bool ok);
	/*! Server answer from a requestUploadScenario \see requestUploadScenario.
		Broadcast to all co-editor that a scenario has been uploaded.
		\param hlScenario The edition scenario (compressed)
	*/
	virtual void onScenarioUploaded(NLNET::IModuleProxy *sender, const R2::CObjectSerializerClient &hlScenario) ;
	/*! Server answer from a requestSetNode.
		Broadcast to all co-editor when a node is set
		Same options than requestSetNode
		\see requestSetNode
	*/
	virtual void onNodeSet(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, const R2::CObjectSerializerClient &value);
	/*! Server answer from a requestInsertNode.
		Broadcast to all co-editor when a node is insert
		Same options than requestInsertNode
		\see requestInsertNode
	*/
	virtual void onNodeInserted(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, sint32 position, const std::string &key, const R2::CObjectSerializerClient &value);
	/*! Server answer from a requestEraseNode
		Broadcast to all co-editor when a node is erased
		Same options than requestEraseNode
		\see requestEraseNode
	*/
	virtual void onNodeErased(NLNET::IModuleProxy *sender, const std::string &instanceId, const std::string &attrName, sint32 position);
	/*! Server answer from a requestMoveNode
		Broadcast to all co-editor when a node is moved
		Same options than requestMoveNode
		\see requestMoveNode
	*/
	virtual void onNodeMoved(NLNET::IModuleProxy *sender, const std::string &instanceId1, const std::string &attrName1, sint32 position1, const std::string &instanceId2, const std::string &attrName2, sint32 position2);
	//! }


	//! Data modification - Data fragmentation
	//! For some messages, data are cut in small message then send to DSS via SBS.
	//! Fragmentation enable to implement a progression bar.
	//! Communication via SBS disable bandwith limitation  caused by FS
	//! Using sendMsgToDss is useful only for big message like uploading Scenario or uploading runtime scenario
	//! {
	/*! Cut a messag in small chunk then send it to DSS via SBS.
	\param msg The message that will be cut in small chunk an send via sbs.
	*/
	void sendMsgToDss(const NLNET::CMessage& msg);

	/*! Answer from Dss to sendMsgToDss
		\param msgNam The name of the message as seen in r2_share_itf.xml
		\param nbPacket The number of packet that will be send
		\param size The size of the data that will be send.
	*/
	virtual void multiPartMsgHead(NLNET::IModuleProxy *sender,  const std::string& msgName, uint32 nbPacket, uint32 size);

	/*! Answer from Dss to sendMsgToDss (this message always follow multiPartMsgHead)
		This call back when each chunk of data is received.
		\param packetId The id of the packet received. First value is 0 last value is nbPacke -1. nbPacket is given by multiPartMsgHead
		\param nbPacket The number of packet that will be send
		\param size The size of the data that will be send.
		\see multiPartMsgHead
		\see sendMsgToDss
	*/
	virtual void multiPartMsgBody(NLNET::IModuleProxy *sender, uint32 packetId, uint32 packetSize);

	/*! Answer from Dss to sendMsgToDss (this message always follow multiPartMsgHead and multiPartMsgBody)
		This call back when each chunk of data is received.
		\see multiPartMsgHead
		\see multiPartMsgBody
		\see sendMsgToDss
	*/
	virtual void multiPartMsgFoot(NLNET::IModuleProxy *sender);

	/*! setMute Mode. When mute is true then client do not apply scenario update messages. It is not necessary when ther is only on editor
	\param mute If true the client do not apply onNode(Set|Inset|Moved)
	*/
	void setMute(bool mute);
	//! }

	/////////////////////////////////////////////////////
	//// Edition Mode  to test mode
	/////////////////////////////////////////////////////
	/*! Edition Mode to test Mode*/
	//!{
	/*! Function call back by Dss when a scenario is about to start (after startScenario and request).
		\see startingScenario
		\param ok If true the scenario is about to start change the gui. If false go back in edition mode.
		\param startAct The start act. In edition we start test from the current Act.
		\param errorReason If the session owner was unable to start the session ( error in translation) a description message is send.
	*/
	void startScenario(class NLNET::IModuleProxy * proxy, bool ok, uint32 startAct, const std::string & errorReason);
	/*! This message appears short after requestStartScenario: the main animator will upload data and the other will draw a waiting screen
		When this message hapends the editor change is Gui. If the player is the session owner he will upload data.
		\param charId The id of the player that has "clicked" on a go start button
		\see requestStartScenario
	*/
	void startingScenario(class NLNET::IModuleProxy * proxy, uint32 charId);
	//! The User ask the dss to start an animation/test scenario (the user click on the "Go Test" button)
	bool requestStartScenario();
	//!}

	/////////////////////////////////////////////////////
	//// Test Mode to edition mode
	/////////////////////////////////////////////////////

	//! Ask the stop of the Animation / Test. Send a message to AIS to unload data.
	void requestStopTest();

	/*! Ask the connection to an edition session.
	\parm scenarioId The session number to which we want to connect.
	\param  mustTp If true then the player is teleport to the start of act when the scenario finish to download
	\param  mustUpdateHighLevel If false then the scenario is not download but is reload from buffer.
	*/
	void requestMapConnection( uint32 scenarioId, bool mustTp = true, bool mustUpdateHighLevel = true );

	/*! Same as requestMapConnection(currentScenarioId, true, false);
	\see requestMapConnection()
	*/
	void requestReconnection();

	/*! Sever answer to requestMapConnection. Updates the gui.
		\param userSlot The slot Id of the user. If one editor userSlotId equal 1.
		\param scenarioId The id of the scenario we want to connect
		\param connectedAs The state of the client when he connect (edition, animation
	*/
	void onAdventureConnected(uint32 userSlotId, uint32 scenarioId, uint32 connectedAs, CObject* scenario );
	//!}


	/////////////////////////////////////////////////////
	//// DM / Animation function
	/////////////////////////////////////////////////////
	// Function use in test/animation
	//!{

	/*! DM function to start another act (and stop the current).
	This function is linked to gui with the button to change (The current Act).
	If the next act is at the same location npc of current act will depop then npc from next act will pop.
	If the two act are at different location then all member of the session are telported to the next act.
	Accessible by the DM tool bar.
	\param
	*/
	void requestStartAct(uint32 actId);

	/*! DM function that
	*/
	const TActPositionDescriptions& getRuntimeActs() const { return _ActPositionDescriptions; }

	/*!
		DM function that enable the dm to teleport to another character (player).
		(Right click in the participant list)
		\param sessionId The id of the session
		\param sourceCharId The id of the character that will be teleport
		\param destCharId The id of the character where the player will be teleport
	*/
	bool requestTeleportOneCharacterToAnother(uint32 sessionId, uint32 sourceCharId, uint32 destCharId);

	/*!
		DM function that enable the dm to trigger an user trigger
		The list of trigger description is updated at start of scenario by updateUserTriggerDescriptions
		The access to this function is don via the action bar of a DM.
		\param actId The act where the trigger is defined (0 = permanent content)
		\param triggerId The id of the trigger to launch
		\see updateUserTriggerDescriptions
	*/
    bool requestTriggerUserTrigger(uint32 actId, uint triggerId);

	/*!
		Function called by the DSS at scenario start during animation. It give the description of UserTrigger of the scenario.
		This description is used to build the menu in the trigger selecter of DM anim bar.
		\param userTriggerDescriptions The description of triggers that can be launched by clients
	*/
	virtual void updateUserTriggerDescriptions(NLNET::IModuleProxy *sender,  const TUserTriggerDescriptions &userTriggerDescriptions);

	/*!
		Returns the descriptions of differents User trigger that can be launch by DM
		\see TUserTriggerDescriptions
		\see TUserTriggerDescription
		\return  The description of user triggers
	*/
	const TUserTriggerDescriptions& getUserTriggers() const { return _UserTriggerDescriptions; }


	/*!
		Changes the current weather during animation mode (light...)
		\param weatherValue The weather value (see client code to known the meaning of this value)
	*/
	void requestSetWeather(uint16 weatherValue);

	/*!
		Changes the current season (winter, snow). Not to use to often because changing the season means loading screen
		\param seasonValue The season value  See client code for meaning of this value (0 is automatique)
	*/
	void requestSetSeason(uint8 seasonValue);

	/*!
		Teleports the character to the entry point of an act (or to the entry point of current act)
		\param actId The index of the act. (zero means the current act index)
	*/
	virtual bool requestTpToEntryPoint(uint32 actId = 0);

	/*!
		Teleports the character to a specific position
		\param x, y, z the position to teleport to.
	*/
	virtual void requestTpPosition(float x, float y, float z);



	/*! Updates the DM admin bar. Send DM commands.
		Called withou param this function update the DM action bar.
		Called with as parameter DESPAWN_NPC, ADD_HP, KILL_NPC, ADD_HP, GRP_KILL, GRP_HEAL, CONTROL, STOP_CONTROL, TALK_AS, STOP_TALK it launch DM function
		\see CClientEditionModule for more info
		\parm args a list of optional argument may be empty or one of "DESPAWN_NPC" "ADD_HP" "KILL_NPC" "ADD_HP" "GRP_KILL" "GRP_HEAL" "CONTROL" "STOP_CONTROL" "TALK_AS" "STOP_TALK". Multi param could be useful for setting the aggro distance (NIY).
	*/
	void dssTarget( std::vector<std::string>& args);


	/*! Called by Dss when a npc is incarned. It synchronized the list of incarnated bot.
	This list is shown on the own left of the scenn near the DM Action bar.
	\param botId The updated list of id of npc controlled
	*/
	void updateIncarningList(NLNET::IModuleProxy *sender, const std::vector<uint32> & botId);

	/*! Called by Dss at start when a npc talk is controlled. It synchronized the list of controle bot.
	This list is shown on the own left of the scenn near the DM Action bar.
	\param botId The updated list of id of npc controlled
	*/
	void updateTalkingAsList(NLNET::IModuleProxy *sender, const std::vector<uint32> & botId);

	/*! Gets the list of Incarning Bot (this list is updated by updateIncarningList called by server)
	\see updateIncarningList
	*/
	std::vector<uint32>	getIncarnatingList() const;

	/*! Gets the list of Controlled Bot (this list is updated by updateTalkingAsList called by server)
	\see updateTalkingAsList
	*/
	std::vector<uint32>	getTalkingAsList() const;





	/*!
	OBSOLETE, NIY, Not tested
	This function enable to dynamically manage string table of a scenario.
	It can let a DM dynamically change the dialog of npcs.
	{
	*/

	virtual void requestTalkAs(const std::string& npcname);

	virtual void requestStopTalkAs();

	virtual void requestStringTable();

	virtual void requestSetStringValue(std::string& id,std::string& value );

	virtual void requestStringValue(std::string& localId );

	virtual void requestIdList();
	/*!
	}
	*/



	/////////////////////////////////////////////////////
	//// Save / Loading functions
	/////////////////////////////////////////////////////
	/*!


	All save, and load of scenarios need to be done with asynchronous Message. Loading of a file, or saving a file can not be done directly because we need the server validation.
	Because we need the server to allow a files or to sign scenarios.
	see \see CClientEditionModule from more infos

	Save / Loading functions
	{
	*/
	/*! add a scenario file to the Saving queue. The param of this function are the name of the file and Header info.
		This function is called by a lua wrapper that load header infos from filename.
		\see CComLuaModule::luaLoad
		\param filename The name of the file to load.
		\param values The header infos.
	*/
	void addToSaveList(const std::string& filename, const std::vector< std::pair < std::string, std::string> >& values);

	/*! Adds a scenario file to the loading queue.
		The param of this function are the name of the file and a Functor that will be called when the loading is finished. (The loading is a asyncrhonous proccess)
		\see CComLuaModule::luaLoad
		\param filename The name of the file to load.
		\param cb The functor that is called when the file is loaded.(can be null)
		\see CScenarioValidatorLoadSuccededCallback
	*/
	virtual bool addToLoadList( const std::string& filename, CScenarioValidatorLoadSuccededCallback* cb=0);

	/*! DSS message that indicates if the loading of the scenario was allowed or not by the server
		\param md5 The md5 of the file that we wanted to load.
		\param ok If true the server has allowed the loading of the file otherwise the server has refused ( maybe the file was manually changed).
	*/
	virtual void loadScenarioFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, bool ok);


	/*! DSS message that indicates if the save of the scenario was allowed or not by the server
		\param md5 The md5 of the file that we wanted to save.
		\param signature The signature of the file. This value must be added to the file in order to be sure that the file was not manually generated/
		\param ok If true the server has allowed the save of the file otherwise the server has refused ( maybe try to save the scenario with LD specific options).
	*/
	virtual void saveScenarioFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, const std::string& signature, bool ok);



	/*
		User Component load/save mehods
	*/
	virtual void addToUserComponentSaveList(const std::string& filename, const std::vector< std::pair < std::string, std::string> >& values, std::string &body);
	virtual void saveUserComponentFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, const std::string& signature, bool ok);
	virtual void loadUserComponentFileAccepted(NLNET::IModuleProxy *senderModuleProxy, const std::string& md5, bool ok);
	virtual bool addToUserComponentLoadList( const std::string& filename, CUserComponentValidatorLoadSuccededCallback* cb=0);
	/*!
	*/
	void loadScenarioSucceded(const std::string& filename, const std::string& body, const CScenarioValidator::TValues& values);

	void loadAnimationSucceded(const std::string& filename, const std::string& body, const CScenarioValidator::TValues& values);

	/*!
	}
	*/



	virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);


	virtual void scheduleStartAct(NLNET::IModuleProxy *sender, uint32 errorId, uint32 actId, uint32 nbSeconds);



	uint32 getCurrentActIndex() const { return _CurrentActIndex; }





	void updateScenarioRingAccess(bool ok, const std::string& ringAccess, const std::string& errMsg);

	std::string getCharacterRingAccess() const;

	virtual void onRingAccessUpdated(NLNET::IModuleProxy *sender, const std::string &ringAccess) ;


	void requestStopAct();

	void requestCreatePrimitives();

	void requestCreateScenario(CObject* scenario);



	/*!
	Property accessor.
	This system enables to register class definition by registerGenerator.
	Then the call to newComponent generates a CObjectTable using the template that have been save by registerGenerator.
	The method getPropertyValue on a CObjectTable first look in the table of the object. Then in the palette. Then in its default value of the property then an thoses of his parents.
	The function getPropertyList enables to know all properties of an object (useful for debug)

	You can add an palette Element to the editor by addPaletteElement. A palette element is like a template that you clone and put in the scene.
	Palette element are mainly defined in r2_palette.lua (binding is done via CComLuaModule
	You can check if a paletteId (eg "palette.entities.botobjects.fo_s2_bigroot_a") has already been defined.




	*/

	/////////////////////////////////////////////////////
	//// Palette / Property accessor
	/////////////////////////////////////////////////////

	//! Palette / Property accessor
	//! {
	/*! Registers a palette Element. A palette element is like a template that can be clone. They are defined in "r2_palette.lua". And accessible via the Gui through the "Palette" Component.
		\param attrName A unique paletteId like palette.entities.botobjects.fo_s2_bigroot_a"
		\param paletteElement The template that will be register in the system
	*/
	void addPaletteElement(const std::string& attrName, CObject* paletteElement);

	/*!
		Test if a palette id has been registered.
		\param key A paletteId like "palette.entities.botobjects.fo_s2_bigroot_a"
	*/
	bool isInPalette(const std::string& key) const;

	/*!
	Get by its name the value of a property of an object identified by its instance id.
	\param instanceId The instanceId of the object.
	\param attrName The name of the property.
	*/
	CObject* getPropertyValue(const std::string& instanceId, const std::string& attrName) const;

	/*!
	Get by its name the value of a property of an object.
	\param component The object.
	\param attrName The name of the property.
	*/
	CObject* getPropertyValue(CObject* component, const std::string& attrName) const;


	/*!
	Get the list of all property of an object. Mainly used for debug fonction (to dump an object)
	\param component The object.
	\return the list of property (an ObjectTable)
	*/
	CObject* getPropertyList(CObject* component) const;

	/*!
	Get an element of the palette by its id. A palette element is a kind of template that is clone an put in the current act.
	Id looks like "palette.entities.botobjects.fo_s2_bigroot_a" and are defined in r2_palette.lua
	\param key The name of the palette element.
	\return The palette element that is identified by its name
	*/
	CObject* getPaletteElement(const std::string& key)const;

	/*!
	Create a new component by its class. Components are mainly defined in lua in the file r2_components.lua
	eg "NpcCustom"
	\param type The class of the component we want to create (like "NpcCustom", "Act"=
	\return The component that is created.
	*/
	CObject* newComponent(const std::string& type) const;

	/*!

	*/
	void registerGenerator(CObject* classObject);

	CPropertyAccessor& getPropertyAccessor() const;
	//! }




	CScenario* getCurrentScenario() const;

	std::string getEid() const { return _Eid; }

	void setEid(const std::string& eid);

	TSessionId getCurrentAdventureId() const { return _SessionId; }

	void setCurrentAdventureId( TSessionId sessionId) { _SessionId = sessionId; }


		CPalette *getPalette() const { return _Palette; }

	// incoming message from servers


	/////////////////////////////////////////////////////
	//// Component Tooltips
	/////////////////////////////////////////////////////
	void resetDisplayInfo();
	void setDisplayInfo(const std::string& formName, bool displayInfo);
	bool mustDisplayInfo(const std::string& formName) const;
	bool hasDisplayInfo(const std::string& formName) const;
	virtual void onDisplayInfoUpdated(NLNET::IModuleProxy *senderModuleProxy, uint32 displayInfo);

	/////////////////////////////////////////////////////
	//// UserComponent (Not Finished)
	/////////////////////////////////////////////////////

	std::string readUserComponentFile(const std::string& filename);

	void registerUserComponent(const std::string& filename);

	void updateUserComponentsInfo(const std::string & filename, const std::string& name, const std::string & description,
		uint32 timestamp, const std::string& md5hash);

	void saveUserComponentFile(const std::string& filename, bool mustCompress = true);


	bool loadUserComponent(const std::string& filename, bool mustReload = false);

	CUserComponent* getUserComponentByHashMd5( const NLMISC::CHashKeyMD5 & md5) const;

	CUserComponent* getUserComponentByFilename(const std::string& filename) const;

	virtual void onUserComponentRegistered(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CHashKeyMD5 & md5);

	virtual void onUserComponentUploading(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CHashKeyMD5 & md5);

	virtual void onUserComponentDownloaded(NLNET::IModuleProxy *senderModuleProxy, CUserComponent* component);

	std::string getUserComponentExtension() const {  return ".lua"; }

	std::string getUserComponentCompiledExtension() const {  return ".lua.gz"; }

	/////////////////////////////////////////////////////
	//// Change of state
	/////////////////////////////////////////////////////


// End of Test or Animation mode
	// Reconnect if sessionType was Edition
	void onTestModeDisconnected(NLNET::IModuleProxy *moduleProxy, TSessionId sessionId, uint32 lastAct, TScenarioSessionType sessionType);

	// Update the user quota (static quota and dynamic quota)
	virtual void onQuotaUpdated(NLNET::IModuleProxy *senderModuleProxy, uint32 maxNpcs, uint32 maxStaticObjects);

	// The player mode has changed (he must chagnge its speed)
	virtual void onCharModeUpdated(NLNET::IModuleProxy *senderModuleProxy, R2::TCharMode mode);

	virtual void onDisconnected(NLNET::IModuleProxy *sender);

	virtual void onKicked(NLNET::IModuleProxy *sender, uint32 timeBeforeDisconnection, bool mustKick);

	virtual void onAnimationModePlayConnected(NLNET::IModuleProxy *senderModuleProxy);

	virtual void updateMissionItemsDescription(NLNET::IModuleProxy *sender, TSessionId sessionId, const std::vector<R2::TMissionItem> &missionItem);

	virtual void updateActPositionDescriptions(NLNET::IModuleProxy *sender, const TActPositionDescriptions &actPositionDescriptions);



	virtual void updateScenarioHeader(NLNET::IModuleProxy *sender,  const TScenarioHeaderSerializer &scenarioHeader);

	virtual void onCurrentActIndexUpdated(NLNET::IModuleProxy *sender,  uint32 currentActIndex);



	/////////////////////////////////////////////////////
	//// Divers
	/////////////////////////////////////////////////////

	virtual void systemMsg(NLNET::IModuleProxy *sender,  const std::string& msgType, const std::string& who, const std::string& msg);
	// Simulate local a tp
	virtual void onTpPositionSimulated(NLNET::IModuleProxy *sender,  TSessionId sessionId, uint64 characterId64, sint32 x, sint32 y, sint32 z, uint8 scenarioSeason);

	// Verify if the "crypted" charId (xox on bit rotated) is the same user that the current user.
	bool hasCharacterSameCharacterIdMd5(const std::string & charIdMd5) const;

	std::string getEmoteBehaviorFromEmoteId(const std::string& emote) const;

	void setMustStartScenario(bool state){ _MustStartScenario = state; }

	void setScenarioUpToDate(bool state) { _ScenarioUpToDate = state; }

	bool isServerEditionModuleUp() const {return _ServerEditionProxy != NULL; }
	/////////////////////////////////////////////////////
	//// Getter
	/////////////////////////////////////////////////////

	const TScenarioHeader& getScenarioHeader() const { return _ScenarioHeader; }


	bool isSessionOwner() const { return _IsSessionOwner; }
	// return the EditSession Linked (SessionType == st_anim) if no session are linked return 0;
	TSessionId getEditSessionLink() const { return _EditSessionLink;}

	TScenarioSessionType getSessionType() const { return _SessionType; }

	bool getMustStartScenario() const { return _MustStartScenario; }

	bool getScenarioUpToDate() const { return _ScenarioUpToDate; }

	uint32 getMaxStaticObjects() const { return _MaxStaticObjects;}

	uint32 getSheetIdByPlotItemIndex(uint32 index) const;


	TSessionId getSessionId() const { return _SessionId; }

	uint32 getAiInsanceId() const { return _AiInstanceId;}

	uint32 getMaxNpcs() const { return _MaxNpcs;}







	uint32 getCurrentMaxId();
	void refreshComponents();

	void reserveIdRange(uint32 range);


	bool askUpdateCharMode(R2::TCharMode mode);


	// set by lua just before a request translate Features
	void setStartingActIndex(uint32 startingActIndex);

	virtual bool requestSetStartingAct(uint32 actId );

	// Connect to a animation session in Play Mode
	virtual bool connectAnimationModePlay();

	// start an act (if an act is runing start stop the previous one)
	bool askMissionItemsDescription();

	void resetNameGiver();



private:
	typedef std::map<std::string, CUserComponent*> TUserComponents;
	typedef std::map<std::string, CScenarioValidator*> TScenarioToSave;
	typedef std::map<std::string, CScenarioValidator*> TScenarioToLoad;
	typedef std::map<std::string, CUserComponentValidator*> TUserComponentToSave;
	typedef std::map<std::string, CUserComponentValidator*> TUserComponentToLoad;

private:
	bool _Initialized;
//	NLMISC::TTime _LastRefreshComponents;
	std::string _ScriptDirectory;


	TUserComponents _UserComponents;

	CTranslationModule* _TranslationModule;
	CScenario* _Scenario;
	TScenarioSessionType _SessionType;
	CObjectFactory* _Factory;
	CPalette* _Palette;
	CServerAnswerForseener* _ServerAnswerForseener;

	CPropertyAccessor* _PropertyAccessor;

	NLNET::TModuleProxyPtr _ServerEditionProxy;
	CDynamicMapClient* _Client;
	std::string _Eid;
	TSessionId _SessionId;
	uint32 _MaxNpcs;
	uint32 _MaxStaticObjects;

	std::auto_ptr<R2::CEmoteBehavior> _Emotes;


	CEditorConfig* _ClientEditorConfig;
	uint32 _StartingActIndex;
	TChanID _ChannelId;
	//		NLNET::IModuleSocket* _ClientGw;
	//		NLNET::TModuleId _ServerAnimationModuleId;
	NLNET::TModuleProxyPtr _ServerAnimationProxy;
	TActPositionDescriptions _ActPositionDescriptions;
	TUserTriggerDescriptions _UserTriggerDescriptions;
	uint32 _CurrentActIndex;

	std::vector<uint32> _IncarnatingList;
	std::vector<uint32> _TalkingAsList;

	std::string _RingAccess;
	bool _Mute;

	bool _MustStartScenario;
	bool _ScenarioUpToDate;

	uint32 _AiInstanceId;

	TUserComponentToSave _UserComponentToSave;
	TUserComponentToLoad _UserComponentToLoad;
	TScenarioToSave _ScenarioToSave;
	TScenarioToLoad _ScenarioToLoad;
	R2::TCharMode _CharMode;

	TScenarioHeader _LastReadHeader;
	TScenarioHeader _ScenarioHeader;
	bool _IsSessionOwner;
	TSessionId _EditSessionLink;

};

	class CLoadAnimationSucceded: public CScenarioValidatorLoadSuccededCallback
	{
	public:
		CLoadAnimationSucceded(CClientEditionModule* module):_Module(module){}

		virtual void doOperation(const std::string& filename,const std::string& body,const CScenarioValidator::TValues& values);
	private:

		CClientEditionModule* _Module;
	};

	class CLoadScenarioSucceded: public CScenarioValidatorLoadSuccededCallback
	{
	public:
		CLoadScenarioSucceded(CClientEditionModule* module):_Module(module){}

		virtual void doOperation(const std::string& filename,const std::string& body,const CScenarioValidator::TValues& values);

	private:

		CClientEditionModule* _Module;
	};



	class CLoadUserComponentSucceeded: public CUserComponentValidatorLoadSuccededCallback
	{
	public:
		CLoadUserComponentSucceeded(CClientEditionModule* module):_Module(module){}

		virtual void doOperation(const std::string& filename,const std::string& body,const CUserComponentValidator::TValues& values);

	private:

		CClientEditionModule* _Module;
	};

	class CEditorConfig
	{
	public:
		CEditorConfig();
		void setDisplayInfo(const std::string& formName, bool displayInfo);
		void setDisplayInfo(uint32 displayInfo);
		bool mustDisplayInfo(const std::string& formName) const;
		bool hasDisplayInfo(const std::string& formName) const;
		uint32 getDisplayInfo() const;

	private:
		static std::map<std::string, uint32> _NameToId;
	};





} //namespace R2



#endif //R2_CLIENT_EDITON_MODULE_H



