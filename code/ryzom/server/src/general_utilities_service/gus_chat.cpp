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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "gus_chat.h"
#include "nel/misc/command.h"
#include "nel/net/message.h"
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "game_share/singleton_registry.h"
#include "game_share/dyn_chat.h"
#include "game_share/ryzom_entity_id.h"
#include "game_share/synchronised_message.h"
#include "game_share/ios_interface.h"

#include "gus_mirror.h"
#include "gus_client_manager.h"


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//-----------------------------------------------------------------------------
// GUS namespace
//-----------------------------------------------------------------------------

namespace GUS
{
	//-----------------------------------------------------------------------------
	// class CChatChannelImplementation
	//-----------------------------------------------------------------------------

	class CChatChannelImplementation: public CChatChannel
	{
		friend class CChatManagerImplementation;
	public:
		CChatChannelImplementation(const NLMISC::CSString &channelName);

		void openChannel(const NLMISC::CSString& channelTitle, uint32 historySize, bool noBroadcast, bool forwardInput, bool autoInsertPlayer);
		void closeChannel();
		
		void addClient(GUS::TClientId clientId);
		void removeClient(GUS::TClientId clientId);

		void broadcastMessage(const ucstring& speakerName, const ucstring& txt);
		void broadcastMessage(const std::string& speakerNameUtf8, const std::string& txtUtf8);
		void sendMessage(GUS::TClientId clientId, const ucstring& speakerName,const ucstring& txt);
		void sendMessage(GUS::TClientId clientId, const std::string& speakerNameUtf8, const std::string& txtUtf8);

		void setChatCallback(IChatCallback *callback);
		const NLMISC::CSString& getChannelName() const;
		const NLMISC::CSString& getChannelTitle() const;
		void setChannelTitle(const NLMISC::CSString& title);

	private:

		IChatCallback*		_ChatCallback;

		NLMISC::CSString	_ChannelName;
		bool				_ChannelOpen;
		TChanID				_ChannelID;
		NLMISC::CSString	_ChannelTitle;
		uint32				_HistorySize;
		bool				_NoBroadcast;
		bool				_ForwardInput;
		bool				_AutoInsertPlayer;

		struct TClientValidation
		{
			GUS::TClientId	ClientId;
			TGameCycle		EntryDate;
		};
		// List of client waiting 'validation' in the chat
		list<TClientValidation>	_ClientToValidate;

		NLMISC_COMMAND_FRIEND(chatDisplayChannel);
		NLMISC_COMMAND_FRIEND(chatPlayerInput);
	};


	//-----------------------------------------------------------------------------
	// class CChatManagerImplementation
	//-----------------------------------------------------------------------------

	class CChatManagerImplementation
		: public CChatManager, 
		public CGusMirror::IMirrorModuleCallback,
		public CClientManager::IConnectionHandler,
		public IServiceSingleton
	{
	private:
		// prohibit instantiation with private ctor
		CChatManagerImplementation();

		//-----------------------------------------------------------------------------
		// specialisation of CGusMirror::IMirrorModuleCallback

		void mirrorIsReady(CGusMirror *mirrorModule) {};
		void serviceMirrorUp(CGusMirror *mirrorModule, const std::string &serviceName, NLNET::TServiceId serviceId);
		void serviceMirrorDown(CGusMirror *mirrorModule, const std::string &serviceName, NLNET::TServiceId serviceId) {};
		void mirrorTickUpdate(CGusMirror *mirrorModule);
		void entityAdded(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex) {};
		void entityRemoved(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, const NLMISC::CEntityId *entityId) {};
		void propertyChanged(CGusMirror *mirrorModule, CMirroredDataSet *dataSet, const TDataSetRow &entityIndex, TPropertyIndex propIndex) {};


		//-----------------------------------------------------------------------------
		// specialisation of CClientManager::IConnectionHandler

		void connect(TClientId);
		void disconnect(TClientId);


		//-----------------------------------------------------------------------------
		// specialisation of IServiceSingleton

		void init();


	public:
		//-----------------------------------------------------------------------------
		// specialisation of CChatManager

		TChatChannelPtr	createChatChannel(const NLMISC::CSString &channelName);
		void removeChannel(CChatChannel *channel);
		TChatChannelPtr getChatChannel(const NLMISC::CSString& channelName);
		bool setChatChannelTitle(CChatChannel *channel,const NLMISC::CSString& channelTitle);


		//-----------------------------------------------------------------------------
		// propriatory public interface

		// get singleton instance
		static CChatManagerImplementation* getInstance();

		uint32			generateChannelNumber();

		static void		cbDynChatForward( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId );

	private:
		//-----------------------------------------------------------------------------
		// private data

		typedef map<CSString, CSmartPtr<CChatChannelImplementation> > TChannels;
		TChannels _Channels;

		/// Channel numbering (incremented for each channel created)
		uint32		_CurrentChannelNumber;


		//-----------------------------------------------------------------------------
		// friendship for associated NLMISC_COMMANDs

		NLMISC_COMMAND_FRIEND(chatCreateChannel);
		NLMISC_COMMAND_FRIEND(chatRemoveChannel);
		NLMISC_COMMAND_FRIEND(chatSendMessage);
		NLMISC_COMMAND_FRIEND(chatDisplayChannel);
		NLMISC_COMMAND_FRIEND(chatPlayerInput);
	};

	//-----------------------------------------------------------------------------
	// NeL service callbacks 
	//-----------------------------------------------------------------------------
	TUnifiedCallbackItem ChatCbArray[]=
	{
		{	"DYN_CHAT:FORWARD",				CChatManagerImplementation::cbDynChatForward},
	};

	//-----------------------------------------------------------------------------
	// methods CChatManagerImplementation
	//-----------------------------------------------------------------------------

	CChatManagerImplementation::CChatManagerImplementation()
		: _CurrentChannelNumber(0)
	{
	}

	void CChatManagerImplementation::serviceMirrorUp(CGusMirror *mirrorModule, const std::string &serviceName, NLNET::TServiceId serviceId)
	{
		if (serviceName == "EGS")
		{
			// open all channel on the EGS server
			TChannels::iterator it(_Channels.begin()), last(_Channels.end());
			for(; it != last; ++it)
			{
				CChatChannelImplementation *cci = it->second;

				cci->openChannel(cci->_ChannelTitle, cci->_HistorySize, cci->_NoBroadcast, cci->_ForwardInput, cci->_AutoInsertPlayer);
			}
		}
	}

	void CChatManagerImplementation::mirrorTickUpdate(CGusMirror *mirrorModule)
	{
		// for each channel, update waiting clients

		TGameCycle now = CTickEventHandler::getGameCycle();

		TChannels::iterator it(_Channels.begin()), last(_Channels.end());
		for (; it != last; ++it)
		{
			CChatChannelImplementation *cci = it->second;

			while( !cci->_ClientToValidate.empty() 
				&& (cci->_ClientToValidate.front().EntryDate < (now - 50)))
			{
				if (cci->_ChatCallback != NULL)
					cci->_ChatCallback->clientReadyInChannel(cci, cci->_ClientToValidate.front().ClientId);

				cci->_ClientToValidate.pop_front();
			}
		}
	}


	void CChatManagerImplementation::connect(TClientId client)
	{
		// For now, we consider connecting all client in all chat channel
		TChannels::iterator it(_Channels.begin()), last(_Channels.end());
		for (; it != last; ++it)
		{
			CChatChannelImplementation *cci = it->second;

			if (cci->_AutoInsertPlayer ||
				(cci->_ChatCallback != NULL && cci->_ChatCallback->isClientAllowedInChatChannel(client, cci)))
			{
				cci->addClient(client);
//				// add the client into the chat
//				TChanID		chan = cci->_ChannelID;
//				bool        writeRight = true;
//
//				CMessage msgout("DYN_CHAT:ADD_SESSION");
//				msgout.serial(chan);
//				msgout.serial(client);
//				msgout.serial(writeRight);
//
//				sendMessageViaMirror( "EGS", msgout);
//
//				CChatChannelImplementation::TClientValidation cv;
//				cv.ClientId = client;
//				cv.EntryDate = CTickEventHandler::getGameCycle();
//
//				cci->_ClientToValidate.push_back(cv);
			}
		}
	}

	void CChatManagerImplementation::disconnect(TClientId)
	{
		// Nothing special to do
	}

	void CChatManagerImplementation::init()
	{
		NLNET::CUnifiedNetwork::getInstance()->addCallbackArray(ChatCbArray, sizeof(ChatCbArray) / sizeof(TUnifiedCallbackItem));

		// Register a callback in client manager
		CClientManager::getInstance()->setConnectionCallback(this);

		// Register a callback in the gus mirror 
		CGusMirror::getInstance()->registerModuleCallback(this);
	}

	CChatManagerImplementation* CChatManagerImplementation::getInstance()
	{
		static CChatManagerImplementation* ptr=NULL;
		if (ptr==NULL)
			ptr= new CChatManagerImplementation;
		return ptr;
	}

	TChatChannelPtr	CChatManagerImplementation::createChatChannel(const NLMISC::CSString &channelName)
	{
		if (_Channels.find(channelName) != _Channels.end())
		{
			nlwarning("A channel with name '%s' already exist, creation failed", channelName.c_str());
			return NULL;
		}
		CChatChannelImplementation *cc = new CChatChannelImplementation(channelName);
		_Channels[channelName] = cc;

		return cc;
	}

	void CChatManagerImplementation::removeChannel(CChatChannel *channel)
	{
		TChannels::iterator it(_Channels.begin()), last(_Channels.end());

		for (; it != last; ++it)
		{
			if (it->second == (CChatChannelImplementation*)channel)
			{
				_Channels.erase(it);
				break;
			}
		}
	}

	TChatChannelPtr CChatManagerImplementation::getChatChannel(const NLMISC::CSString& channelName)
	{
		if (_Channels.find(channelName) == _Channels.end())
		{
			nlwarning("Chat Channel '%s' is unknown", channelName.c_str());
			return TChatChannelPtr();
		}

		return &*_Channels[channelName];
	}

	bool CChatManagerImplementation::setChatChannelTitle(CChatChannel *channel,const NLMISC::CSString& channelTitle)
	{
		bool ok= true;
		CSString title= channelTitle;

		TChannels::iterator it(_Channels.begin()), last(_Channels.end());
		for (; it != last; ++it)
		{
			if (it->second!=NULL && it->second!= (CChatChannelImplementation*)channel &&
				(it->second->getChannelTitle()==title || it->second->getChannelName()==title) )
			{
				// the channel title is already assigned to another channel
				ok= false;
				title= channel->getChannelName();
				break;
			}
		}
		channel->setChannelTitle(title);
		return ok;
	}

	uint32 CChatManagerImplementation::generateChannelNumber()
	{
		return _CurrentChannelNumber++;
	}


	void CChatManagerImplementation::cbDynChatForward( CMessage& msgin, const string &serviceName, NLNET::TServiceId serviceId )
	{
		CChatManagerImplementation *cmi = CChatManagerImplementation::getInstance();

		// serial the player inputs
		TPlayerInputForward	pif;
		msgin.serial(pif);

		CEntityId eid = CGusMirror::getInstance()->getDataSet("fe_temp")->getEntityId(pif.Sender);

		nldebug("Receiving input '%s' from player %s %s", 
			pif.Content.toString().c_str(),
			eid.toString().c_str(),
			pif.Sender.toString().c_str());
	
		// find the channel to callback the creator
		TChannels::iterator it(cmi->_Channels.begin()), last(cmi->_Channels.end());
		for (; it != last; ++it)
		{
			CChatChannelImplementation *cc = it->second;

			if (cc->_ChannelID == pif.ChanID)
			{
				nldebug("Forwarding player %s input into channel '%s'",
					eid.toString().c_str(),
					cc->_ChannelName.c_str());

				// ok, we find it !
				if (cc->_ChatCallback != NULL)
				{
					cc->_ChatCallback->receiveMessage(pif.Sender, pif.Content);
					break;
				}
			}
		}
	}


	//-----------------------------------------------------------------------------
	// methods CChatManager
	//-----------------------------------------------------------------------------

	CChatManager* CChatManager::getInstance()
	{
		return CChatManagerImplementation::getInstance();
	}

	//-----------------------------------------------------------------------------
	// methods CChatChannelImplementation
	//-----------------------------------------------------------------------------

	CChatChannelImplementation::CChatChannelImplementation(const CSString &channelName)
	:_ChannelOpen(false),
	_ChannelName(channelName)
	{
	}

	void CChatChannelImplementation::openChannel(const NLMISC::CSString& channelTitle, uint32 historySize, bool noBroadcast, bool forwardInput, bool autoInsertPlayer)
	{
		if (_ChannelOpen)
			return;

		// create the channel id
		uint32 number = CChatManagerImplementation::getInstance()->generateChannelNumber();
		_ChannelID = CEntityId(RYZOMID::dynChatGroup, uint64(number));
		_ChannelTitle = channelTitle;
		_HistorySize = historySize;
		_NoBroadcast = noBroadcast;
		_ForwardInput = forwardInput;
		_AutoInsertPlayer = autoInsertPlayer;

		CMessage	msgout("DYN_CHAT:ADD_SERVICE_CHAN");
		msgout.serial(_ChannelID);
		msgout.serial(_ChannelTitle);
		msgout.serial(_NoBroadcast);
		msgout.serial(_ForwardInput);

		sendMessageViaMirror( "EGS", msgout);

		if (_HistorySize != 0)
		{
			CMessage msgout("DYN_CHAT:SET_CHAN_HISTORY");
			msgout.serial(_ChannelID);
			msgout.serial(_HistorySize);

			sendMessageViaMirror("EGS", msgout);
		}

		_ChannelOpen = true;

		// put all existing client into this new channel
		CMirroredDataSet *ds = CGusMirror::getInstance()->getDataSet("fe_temp");

		TEntityIdToEntityIndexMap::const_iterator it(ds->entityBegin()), last(ds->entityEnd());
		for (; it != last; ++it)
		{
			if (it->first.getType() == RYZOMID::player)
			{
				TDataSetRow	clientID = ds->getDataSetRow(it->first);

				if (_AutoInsertPlayer 
					|| (_ChatCallback && _ChatCallback->isClientAllowedInChatChannel(clientID, this)))
				{
					addClient(clientID);
//					// We have a player here, open a session
//
//					TChanID		chan = _ChannelID;
//					bool        writeRight = true;
//
//					CMessage msgout("DYN_CHAT:ADD_SESSION");
//					msgout.serial(chan);
//					msgout.serial(clientID);
//					msgout.serial(writeRight);
//
//					sendMessageViaMirror( "EGS", msgout);
//
//					TClientValidation cv;
//					cv.ClientId = clientID;
//					cv.EntryDate = CTickEventHandler::getGameCycle();
//
//					_ClientToValidate.push_back(cv);
				}
			}
		}
	}

	void CChatChannelImplementation::closeChannel()
	{
		if (!_ChannelOpen)
			return;

		CMessage	msgout("DYN_CHAT:REMOVE_SERVICE_CHAN");
		msgout.serial(_ChannelID);

		sendMessageViaMirror( "EGS", msgout);

		_ChannelOpen = false;
	}

	void CChatChannelImplementation::addClient(GUS::TClientId clientId)
	{
		TChanID		chan = _ChannelID;
		bool        writeRight = true;

		CMessage msgout("DYN_CHAT:ADD_SESSION");
		msgout.serial(chan);
		msgout.serial(clientId);
		msgout.serial(writeRight);

		sendMessageViaMirror( "EGS", msgout);

		TClientValidation cv;
		cv.ClientId = clientId;
		cv.EntryDate = CTickEventHandler::getGameCycle();

		_ClientToValidate.push_back(cv);
	}

	void CChatChannelImplementation::removeClient(GUS::TClientId clientId)
	{
		TChanID		chan = _ChannelID;

		CMessage msgout("DYN_CHAT:REMOVE_SESSION");
		msgout.serial(chan);
		msgout.serial(clientId);

		sendMessageViaMirror( "EGS", msgout);
	}

	void CChatChannelImplementation::broadcastMessage(const ucstring& speakerName, const ucstring& txt)
	{
		nldebug("Channel %s : broadcasting \"'%s' says '%s'\"",
			_ChannelTitle.c_str(),
			speakerName.toString().c_str(),
			txt.toString().c_str());

		CMessage msgout("DYN_CHAT:SERVICE_CHAT");
		msgout.serial(_ChannelID);
		msgout.serial(const_cast<ucstring&>(speakerName));
		msgout.serial(const_cast<ucstring&>(txt));

		sendMessageViaMirror( "IOS", msgout);
	}

	void CChatChannelImplementation::broadcastMessage(const std::string& speakerNameUtf8, const std::string& txtUtf8)
	{
		ucstring speakerName, txt;
		speakerName.fromUtf8(speakerNameUtf8);
		txt.fromUtf8(txtUtf8);

		broadcastMessage(speakerName, txt);
	}

	void CChatChannelImplementation::sendMessage(GUS::TClientId clientId, const ucstring& speakerName,const ucstring& txt)
	{
		CEntityId eid = CGusMirror::getInstance()->getDataSet("fe_temp")->getEntityId(clientId);
		nldebug("Channel %s : sending \"'%s' says '%s'\" to client %s",
			_ChannelTitle.c_str(),
			speakerName.toString().c_str(),
			txt.toString().c_str(),
			eid.toString().c_str());

		CMessage msgout("DYN_CHAT:SERVICE_TELL");
		msgout.serial(_ChannelID);
		msgout.serial(const_cast<ucstring&>(speakerName));
		msgout.serial(clientId);
		msgout.serial(const_cast<ucstring&>(txt));

		sendMessageViaMirror( "IOS", msgout);
	}

	void CChatChannelImplementation::sendMessage(GUS::TClientId clientId, const std::string& speakerNameUtf8, const std::string& txtUtf8)
	{
		ucstring speakerName, txt;
		speakerName.fromUtf8(speakerNameUtf8);
		txt.fromUtf8(txtUtf8);

		sendMessage(clientId, speakerName, txt);
	}

	void CChatChannelImplementation::setChatCallback(IChatCallback* callback)
	{
		_ChatCallback = callback;
	}

	const NLMISC::CSString& CChatChannelImplementation::getChannelName() const
	{
		return _ChannelName;
	}

	const NLMISC::CSString& CChatChannelImplementation::getChannelTitle() const
	{
		return _ChannelTitle;
	}

	void CChatChannelImplementation::setChannelTitle(const NLMISC::CSString& title)
	{
		_ChannelTitle= title;
		CIOSMsgSetPhrase(_ChannelName,_ChannelTitle).send();
	}


	//-----------------------------------------------------------------------------
	// CChat instantiator
	//-----------------------------------------------------------------------------

	class CChatManagerImplementationInstantiator
	{
	public:
		CChatManagerImplementationInstantiator()
		{
			CChatManagerImplementation::getInstance();
		}
	};
	static CChatManagerImplementationInstantiator ChatManagerInstantiator;


	//-----------------------------------------------------------------------------
	// NLMISC_COMMAND commands
	//-----------------------------------------------------------------------------

	NLMISC_COMMAND(chatCreateChannel, "create a new chat channel", "<channelName> <channelTitle>")
	{
		if (args.size() != 2)
			return false;

		CChatManagerImplementation *cmi = CChatManagerImplementation::getInstance();

		TChatChannelPtr channel = cmi->createChatChannel(args[0]);

		if (channel != NULL)
		{
			channel->openChannel(args[1], 0, true, true, true);
		}
		else
		{
			nlwarning("Failed to create channel '%s'", args[1].c_str());
		}

		return true;
	}

	NLMISC_COMMAND(chatRemoveChannel, "remove an existing chat channel", "<channelName>")
	{
		if (args.size() != 1)
			return false;

		CChatManagerImplementation *cmi = CChatManagerImplementation::getInstance();

		TChatChannelPtr channel = cmi->getChatChannel(args[0]);

		if (channel == NULL)
		{
			nlwarning ("The channel '%s' is unknow", args[0].c_str());
		}
		else
		{
			channel->closeChannel();
			cmi->removeChannel(channel);
		}

		return true;
	}

	NLMISC_COMMAND(chatSendMessage, "send a chat message in a channel", "<channelName> <senderName> <text>")
	{
		if (args.size() < 3)
			return false;

		CChatManagerImplementation *cmi = CChatManagerImplementation::getInstance();

		TChatChannelPtr channel = cmi->getChatChannel(args[0]);
		if (channel == NULL)
		{
			nlwarning ("The channel '%s' is unknow", args[0].c_str());
		}
		else
		{
			ucstring text;
			for (uint i=2; i<args.size(); ++i)
				text += args[i] + " ";
			channel->broadcastMessage(args[1], text);
		}

		return true;
	}

	NLMISC_COMMAND(chatDisplayChannel, "list all the chat channel and their states", "")
	{
		if (!args.empty())
			return false;

		CChatManagerImplementation *cmi = CChatManagerImplementation::getInstance();

		log.displayNL("Listing %u chat channel :", cmi->_Channels.size());
		CChatManagerImplementation::TChannels::iterator it(cmi->_Channels.begin()), last(cmi->_Channels.end());
		for (;it != last; ++it)
		{
			CChatChannelImplementation *cmi = it->second;

			log.displayNL("  Name '%s', Title '%s', ChanId %s, State '%s'",
				cmi->_ChannelName.c_str(),
				cmi->_ChannelTitle.c_str(),
				cmi->_ChannelID.toString().c_str(),
				cmi->_ChannelOpen ? "OPEN" : "CLOSED");
		}

		return true;
	}

	NLMISC_COMMAND(chatPlayerInput, "simulate a player input in a channel", "<player_eid> <channelName> <message>+")
	{
		if (args.size() < 3)
			return false;

		CEntityId eid(args[0]);
		TDataSetRow index = CGusMirror::getInstance()->getDataSet("fe_temp")->getDataSetRow(eid);
		if (index == INVALID_DATASET_ROW)
		{
			log.displayNL("Unknow player '%s'", eid.toString().c_str());
			return true;
		}

		// Retreive the channel
		TChatChannelPtr channel = CChatManager::getInstance()->getChatChannel(args[1]);
		if (channel == NULL)
		{
			log.displayNL("Unknown channel '%s'", args[1].c_str());
			return true;
		}
		CChatChannelImplementation *cmi = static_cast<CChatChannelImplementation*>(static_cast<CChatChannel*>(channel));

		// concatenate the message
		ucstring txt;
		for (uint i=2; i<args.size(); ++i)
		{
			txt += args[i];
			if (i < args.size() -1)
				txt += " ";
		}

		TPlayerInputForward pif;
		pif.ChanID = cmi->_ChannelID;
		pif.Content = txt;
		pif.Sender = index;

		CMessage msg("DYN_CHAT:FORWARD");
		msg.serial(pif);
		msg.invert();

		CChatManagerImplementation::getInstance()->cbDynChatForward(msg, IService::getInstance()->getServiceAliasName(), IService::getInstance()->getServiceId());

		return true;
	}

	//-----------------------------------------------------------------------------
	// methods CChatBroadcastDisplayer
	//-----------------------------------------------------------------------------

	CChatBroadcastDisplayer::CChatBroadcastDisplayer(CChatChannel* channel): IDisplayer("gusChatBroadcast")
	{
		_Channel= channel;
	}

	void CChatBroadcastDisplayer::doDisplay(const NLMISC::CLog::TDisplayInfo& args,const char *message)
	{
		_Channel->broadcastMessage("*",message);
	}


	//-----------------------------------------------------------------------------
	// methods CChatDisplayer
	//-----------------------------------------------------------------------------

	CChatDisplayer::CChatDisplayer(CChatChannel* channel,TClientId clientId): IDisplayer("gusChat")
	{
		_Channel=	channel;
		_ClientId=	clientId;
	}

	void CChatDisplayer::doDisplay(const NLMISC::CLog::TDisplayInfo& args,const char *message)
	{
		_Channel->sendMessage(_ClientId,"*",message);
	}
}
//-----------------------------------------------------------------------------
