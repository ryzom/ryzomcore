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

#include "stdpch.h"

#include "ring_session_manager_itf.h"

namespace RSMGR
{

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CRingSessionManagerSkel::TMessageHandlerMap &CRingSessionManagerSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("RDSS"), &CRingSessionManagerSkel::registerDSS_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSC"), &CRingSessionManagerSkel::sessionCreated_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("RSE"), &CRingSessionManagerSkel::reportSessionEvent_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCS"), &CRingSessionManagerSkel::scenarioStarted_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("RCK"), &CRingSessionManagerSkel::reportCharacterKicked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SCE"), &CRingSessionManagerSkel::scenarioEnded_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CRingSessionManagerSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CRingSessionManagerSkel::registerDSS_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_registerDSS_RDSS);
		uint32	shardId;
			nlRead(__message, serial, shardId);
		std::vector < TRunningSessionInfo >	runningSessions;
			nlRead(__message, serialCont, runningSessions);
		registerDSS(sender, shardId, runningSessions);
	}

	void CRingSessionManagerSkel::sessionCreated_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_sessionCreated_SSC);
		RSMGR::TRunningSessionInfo	sessionInfo;
			nlRead(__message, serial, sessionInfo);
		sessionCreated(sender, sessionInfo);
	}

	void CRingSessionManagerSkel::reportSessionEvent_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_reportSessionEvent_RSE);
		RSMGR::TSessionEvent	event;
			nlRead(__message, serial, event);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	charId;
			nlRead(__message, serial, charId);
		reportSessionEvent(sender, event, sessionId, charId);
	}

	void CRingSessionManagerSkel::scenarioStarted_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_scenarioStarted_SCS);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		R2::TRunningScenarioInfo	scenarioInfo;
			nlRead(__message, serial, scenarioInfo);
		scenarioStarted(sender, sessionId, scenarioInfo);
	}

	void CRingSessionManagerSkel::reportCharacterKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_reportCharacterKicked_RCK);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	charId;
			nlRead(__message, serial, charId);
		reportCharacterKicked(sender, sessionId, charId);
	}

	void CRingSessionManagerSkel::scenarioEnded_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerSkel_scenarioEnded_SCE);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		R2::TRunningScenarioInfo	scenarioInfo;
			nlRead(__message, serial, scenarioInfo);
		uint32	rrpScored;
			nlRead(__message, serial, rrpScored);
		uint32	scenarioPointScored;
			nlRead(__message, serial, scenarioPointScored);
		uint32	timeTaken;
			nlRead(__message, serial, timeTaken);
		std::vector < uint32 >	participants;
			nlRead(__message, serialCont, participants);
		scenarioEnded(sender, sessionId, scenarioInfo, rrpScored, scenarioPointScored, timeTaken, participants);
	}
		// A edition or animation server module register in the session manager
		// It send the list of session hosted in the server
	void CRingSessionManagerProxy::registerDSS(NLNET::IModule *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->registerDSS(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), shardId, runningSessions);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_registerDSS(__message, shardId, runningSessions);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The session server report a session creation.
	void CRingSessionManagerProxy::sessionCreated(NLNET::IModule *sender, const RSMGR::TRunningSessionInfo &sessionInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->sessionCreated(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_sessionCreated(__message, sessionInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The session report an event.
		// charId is used only when the event is about a character.
	void CRingSessionManagerProxy::reportSessionEvent(NLNET::IModule *sender, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportSessionEvent(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), event, sessionId, charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportSessionEvent(__message, event, sessionId, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The DSS report that an animation scenario has just started
		// this allow SU to create the session log and scenario info record if needed.
	void CRingSessionManagerProxy::scenarioStarted(NLNET::IModule *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->scenarioStarted(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, scenarioInfo);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_scenarioStarted(__message, sessionId, scenarioInfo);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The session report that a DM has kicked a character from a session.
	void CRingSessionManagerProxy::reportCharacterKicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->reportCharacterKicked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_reportCharacterKicked(__message, sessionId, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// The DSS report the end of an animation session and
		// provides a bunch of data about the session life.
	void CRingSessionManagerProxy::scenarioEnded(NLNET::IModule *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo, uint32 rrpScored, uint32 scenarioPointScored, uint32 timeTaken, const std::vector < uint32 > &participants)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->scenarioEnded(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, scenarioInfo, rrpScored, scenarioPointScored, timeTaken, participants);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_scenarioEnded(__message, sessionId, scenarioInfo, rrpScored, scenarioPointScored, timeTaken, participants);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_registerDSS(NLNET::CMessage &__message, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions)
	{
		__message.setType("RDSS");
			nlWrite(__message, serial, shardId);
			nlWrite(__message, serialCont, const_cast < std::vector < TRunningSessionInfo >& > (runningSessions));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_sessionCreated(NLNET::CMessage &__message, const RSMGR::TRunningSessionInfo &sessionInfo)
	{
		__message.setType("SSC");
			nlWrite(__message, serial, const_cast < RSMGR::TRunningSessionInfo& > (sessionInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_reportSessionEvent(NLNET::CMessage &__message, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId)
	{
		__message.setType("RSE");
			nlWrite(__message, serial, event);
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_scenarioStarted(NLNET::CMessage &__message, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo)
	{
		__message.setType("SCS");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, const_cast < R2::TRunningScenarioInfo& > (scenarioInfo));


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_reportCharacterKicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId)
	{
		__message.setType("RCK");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerProxy::buildMessageFor_scenarioEnded(NLNET::CMessage &__message, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo, uint32 rrpScored, uint32 scenarioPointScored, uint32 timeTaken, const std::vector < uint32 > &participants)
	{
		__message.setType("SCE");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, const_cast < R2::TRunningScenarioInfo& > (scenarioInfo));
			nlWrite(__message, serial, rrpScored);
			nlWrite(__message, serial, scenarioPointScored);
			nlWrite(__message, serial, timeTaken);
			nlWrite(__message, serialCont, const_cast < std::vector < uint32 >& > (participants));


		return __message;
	}

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////


	const CRingSessionManagerClientSkel::TMessageHandlerMap &CRingSessionManagerClientSkel::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair < TMessageHandlerMap::iterator, bool > res;

			res = handlers.insert(std::make_pair(std::string("CSS"), &CRingSessionManagerClientSkel::createSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("AHC"), &CRingSessionManagerClientSkel::addCharacterInSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CLSDSS"), &CRingSessionManagerClientSkel::closeSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SH"), &CRingSessionManagerClientSkel::stopHibernation_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CK"), &CRingSessionManagerClientSkel::characterKicked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("CUK"), &CRingSessionManagerClientSkel::characterUnkicked_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("STOCTA"), &CRingSessionManagerClientSkel::teleportOneCharacterToAnother_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SHSA"), &CRingSessionManagerClientSkel::hibernateSession_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			res = handlers.insert(std::make_pair(std::string("SSSP"), &CRingSessionManagerClientSkel::setSessionStartParams_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);

			init = true;
		}

		return handlers;
	}
	bool CRingSessionManagerClientSkel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &message)
	{
		const TMessageHandlerMap &mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}


	void CRingSessionManagerClientSkel::createSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_createSession_CSS);
		uint32	ownerCharId;
			nlRead(__message, serial, ownerCharId);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		RSMGR::TSessionType	type;
			nlRead(__message, serial, type);
		createSession(sender, ownerCharId, sessionId, type);
	}

	void CRingSessionManagerClientSkel::addCharacterInSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_addCharacterInSession_AHC);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	charId;
			nlRead(__message, serial, charId);
		WS::TUserRole	enterAs;
			nlRead(__message, serial, enterAs);
		std::string	ringAccess;
			nlRead(__message, serial, ringAccess);
		bool	newcomer;
			nlRead(__message, serial, newcomer);
		addCharacterInSession(sender, sessionId, charId, enterAs, ringAccess, newcomer);
	}

	void CRingSessionManagerClientSkel::closeSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_closeSession_CLSDSS);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		closeSession(sender, sessionId);
	}

	void CRingSessionManagerClientSkel::stopHibernation_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_stopHibernation_SH);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	ownerId;
			nlRead(__message, serial, ownerId);
		stopHibernation(sender, sessionId, ownerId);
	}

	void CRingSessionManagerClientSkel::characterKicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_characterKicked_CK);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	charId;
			nlRead(__message, serial, charId);
		characterKicked(sender, sessionId, charId);
	}

	void CRingSessionManagerClientSkel::characterUnkicked_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_characterUnkicked_CUK);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	charId;
			nlRead(__message, serial, charId);
		characterUnkicked(sender, sessionId, charId);
	}

	void CRingSessionManagerClientSkel::teleportOneCharacterToAnother_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_teleportOneCharacterToAnother_STOCTA);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		uint32	sourceCharId;
			nlRead(__message, serial, sourceCharId);
		uint32	destCharId;
			nlRead(__message, serial, destCharId);
		teleportOneCharacterToAnother(sender, sessionId, sourceCharId, destCharId);
	}

	void CRingSessionManagerClientSkel::hibernateSession_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_hibernateSession_SHSA);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		hibernateSession(sender, sessionId);
	}

	void CRingSessionManagerClientSkel::setSessionStartParams_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &__message)
	{
		H_AUTO(CRingSessionManagerClientSkel_setSessionStartParams_SSSP);
		uint32	charId;
			nlRead(__message, serial, charId);
		TSessionId	sessionId;
			nlRead(__message, serial, sessionId);
		std::string	initialIslandLocation;
			nlRead(__message, serial, initialIslandLocation);
		std::string	initialEntryPointLocation;
			nlRead(__message, serial, initialEntryPointLocation);
		std::string	initialSeason;
			nlRead(__message, serial, initialSeason);
		setSessionStartParams(sender, charId, sessionId, initialIslandLocation, initialEntryPointLocation, initialSeason);
	}
		// Ask the client to create a new session modules
	void CRingSessionManagerClientProxy::createSession(NLNET::IModule *sender, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->createSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), ownerCharId, sessionId, type);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_createSession(__message, ownerCharId, sessionId, type);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask the client to allow a character in the session
	void CRingSessionManagerClientProxy::addCharacterInSession(NLNET::IModule *sender, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->addCharacterInSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, charId, enterAs, ringAccess, newcomer);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_addCharacterInSession(__message, sessionId, charId, enterAs, ringAccess, newcomer);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask the client to close a running session
	void CRingSessionManagerClientProxy::closeSession(NLNET::IModule *sender, TSessionId sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->closeSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_closeSession(__message, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask the client stop hibernation for the
		// specified session. This mean to remove any
		// hibernated scenario file from the backup.
	void CRingSessionManagerClientProxy::stopHibernation(NLNET::IModule *sender, TSessionId sessionId, uint32 ownerId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->stopHibernation(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, ownerId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_stopHibernation(__message, sessionId, ownerId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Session mananger report that a character has been kicked by the web
	void CRingSessionManagerClientProxy::characterKicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->characterKicked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_characterKicked(__message, sessionId, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Session mananger report that a character has been unkicked by the web
	void CRingSessionManagerClientProxy::characterUnkicked(NLNET::IModule *sender, TSessionId sessionId, uint32 charId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->characterUnkicked(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, charId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_characterUnkicked(__message, sessionId, charId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask to teleport on character to another (the 2 characters must be in the same season)
		// The character must arrived in the season
	void CRingSessionManagerClientProxy::teleportOneCharacterToAnother(NLNET::IModule *sender, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->teleportOneCharacterToAnother(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId, sourceCharId, destCharId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_teleportOneCharacterToAnother(__message, sessionId, sourceCharId, destCharId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Ask to hibernate a session
	void CRingSessionManagerClientProxy::hibernateSession(NLNET::IModule *sender, TSessionId sessionId)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->hibernateSession(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), sessionId);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_hibernateSession(__message, sessionId);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}
		// Set the start position of a session (eg while your are uploading an animation session)
	void CRingSessionManagerClientProxy::setSessionStartParams(NLNET::IModule *sender, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason)
	{
		if (_LocalModuleSkel && _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel->setSessionStartParams(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender), charId, sessionId, initialIslandLocation, initialEntryPointLocation, initialSeason);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_setSessionStartParams(__message, charId, sessionId, initialIslandLocation, initialEntryPointLocation, initialSeason);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_createSession(NLNET::CMessage &__message, uint32 ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type)
	{
		__message.setType("CSS");
			nlWrite(__message, serial, ownerCharId);
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, type);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_addCharacterInSession(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId, const WS::TUserRole &enterAs, const std::string &ringAccess, bool newcomer)
	{
		__message.setType("AHC");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, enterAs);
			nlWrite(__message, serial, const_cast < std::string& > (ringAccess));
			nlWrite(__message, serial, newcomer);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_closeSession(NLNET::CMessage &__message, TSessionId sessionId)
	{
		__message.setType("CLSDSS");
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_stopHibernation(NLNET::CMessage &__message, TSessionId sessionId, uint32 ownerId)
	{
		__message.setType("SH");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, ownerId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_characterKicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId)
	{
		__message.setType("CK");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_characterUnkicked(NLNET::CMessage &__message, TSessionId sessionId, uint32 charId)
	{
		__message.setType("CUK");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, charId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_teleportOneCharacterToAnother(NLNET::CMessage &__message, TSessionId sessionId, uint32 sourceCharId, uint32 destCharId)
	{
		__message.setType("STOCTA");
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, sourceCharId);
			nlWrite(__message, serial, destCharId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_hibernateSession(NLNET::CMessage &__message, TSessionId sessionId)
	{
		__message.setType("SHSA");
			nlWrite(__message, serial, sessionId);


		return __message;
	}

	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &CRingSessionManagerClientProxy::buildMessageFor_setSessionStartParams(NLNET::CMessage &__message, uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason)
	{
		__message.setType("SSSP");
			nlWrite(__message, serial, charId);
			nlWrite(__message, serial, sessionId);
			nlWrite(__message, serial, const_cast < std::string& > (initialIslandLocation));
			nlWrite(__message, serial, const_cast < std::string& > (initialEntryPointLocation));
			nlWrite(__message, serial, const_cast < std::string& > (initialSeason));


		return __message;
	}

}
