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

//#include "stdpch.h"
#include "simulation_service.h"	// for onInvokeResult, onDSSup
#include "simulated_ring_session_manager.h"
//#include "nel/net/module_builder_parts.h"
#include "nel/net/callback_server.h"
#include "game_share/ryzom_entity_id.h"
//#include "game_share/ring_session_manager_itf.h"
//#include "../../nelns/welcome_service/welcome_service_itf.h"
//#include "mysql_wrapper.h"
//#include "database_mapping.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

namespace RSMGR
{
	const char *const RingSessionManagerClassName = "RingSessionManager";

	CRingSessionManager::CRingSessionManager()
	{
		CRingSessionManagerSkel::init();
	}

	CRingSessionManager::~CRingSessionManager()
	{
	}


	bool CRingSessionManager::initModule(const TParsedCommandLine &initInfo)
	{
		nldebug("RSM : initModule with '%s'", initInfo.toString().c_str());
		// this module require task message dispatching
		TParsedCommandLine baseInfo(initInfo);

		TParsedCommandLine base;
		base.ParamName = "base";
		TParsedCommandLine taskDispatch;
		taskDispatch.ParamName = "useCoTaskDispatch";
		base.SubParams.push_back(taskDispatch);

		baseInfo.SubParams.push_back(base);

		if (!CModuleBase::initModule(baseInfo))
			return false;
/*
		// init ring db
		const TParsedCommandLine *initRingDb = initInfo.getParam("ring_db");
		if (initRingDb  == NULL)
		{
			nlwarning("RSM : missing ring db connection information");
			return;
		}

		// connect to the database
		if (!_RingDb.connect(*initRingDb))
		{
			nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
			return;
		}

		// init nel db
		const TParsedCommandLine *initNelDb = initInfo.getParam("nel_db");
		if (initNelDb  == NULL)
		{
			nlwarning("RSM : missing nel db connection information");
			return;
		}

		// connect to the database
		if (!_NelDb.connect(*initNelDb))
		{
			nlwarning("Failed to connect to database using %s", initNelDb->toString().c_str());
			return;
		}


		// init web interface
		const TParsedCommandLine *web = initInfo.getParam("web");
		if (web == NULL)
		{
			nlwarning("Missing 'web' param in module init string");
			return;
		}
		
		const TParsedCommandLine *portParam = web->getParam("port");
		if (portParam == NULL)
		{
			nlwarning("Missing 'web.port' param in module init string");
			return;
		}

		// cleanup the session state : for each open session, set it to locked and remember it as previously open
		string query("SELECT session_id FROM sessions WHERE state = 'ss_open'");
		nlverify(_RingDb.query(query));

		list<uint32> sessionIds;
		{
			// scope the result to make it destroyed at scope end
			auto_ptr<MSW::CUseResult> result(_RingDb.useResult());

			// for each open session, put it in the locked state
			while (result->fetchRow())
			{
				TSessionId sessionId;
				result->getField(0, sessionId);
				sessionIds.push_back(sessionId);
			}
		}

		while (!sessionIds.empty())
		{
			TSessionId sessionId = sessionIds.front();
			sessionIds.pop_front();

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
			nlassert(session != NULL);

			_TemporaryLockedSession.insert(sessionId);
			session->State = TSessionState::ss_locked;
			session->update(_RingDb);
		}

		uint16 port = atoi(portParam->ParamValue.c_str());

		openItf(port);
*/
		return true;
	}

//	void CRingSessionManager::onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//	{
//		if (CRingSessionManagerSkel::onDispatchMessage(sender, message))
//			return;
//	//			else if (CWelcomeServiceClientSkel::onDispatchMessage(sender, message))
//	//				return;
//
//			nlwarning("CRingSessionManager : Unknown message '%s' received", message.getName().c_str());
//	}


	void CRingSessionManager::onModuleUpdate()
	{
		// update the php interface
	//			CRingSessionManagerWebItf::update();
	}


	void CRingSessionManager::onModuleDown(IModuleProxy *proxy)
	{
		TSessionServers::iterator it(_SessionServers.find(proxy));

		if (it != _SessionServers.end())
		{
			// close all session hosted by this server
			TSessionServerInfo &ssi = it->second;

			nldebug("RSM : receive module down for session server '%s'", it->first->getModuleName().c_str());

			TSessionServerInfo::THostedSessions::iterator first(ssi.HostedSessions.begin()), last(ssi.HostedSessions.end());
			for (; first != last; ++first)
			{
				TSessionId sessionId = first->first;

				// check if we have a web callback pending
				TPendingSessions::iterator it(_PendingSessions.begin());
				while (it != _PendingSessions.end())
				{
					if (it->SessionId == sessionId)
					{
						// session server failure !
						invokeResult(it->From, 5, "Session server failure");
						_PendingSessions.erase(it);
						// restart because the iterator is invalid
						it = _PendingSessions.begin();
					}
					else
						++it;
				}

				// todo check for pending joins

				closeSession(sessionId);
			}

			// remove session info
			{
				TSessionServerInfo::THostedSessions::iterator first(ssi.HostedSessions.begin()), last(ssi.HostedSessions.end());
				for (; first != last; ++first)
				{
					nlassert(_SessionIndex.find(first->first) != _SessionIndex.end());
					_SessionIndex.erase(first->first);
				}
			}
			_SessionServers.erase(it);
		}
/*
		else if (proxy->getModuleClassName() == "WelcomeService")
		{
			nldebug("RSM : receive a module down for Welcome service module '%s'", proxy->getModuleName().c_str());
			// look in the welcome services
			TWelcomeServices::iterator first(_WelcomeServices.begin()), last(_WelcomeServices.end());
			for (; first != last; ++first)
			{
				if (first->second == proxy)
				{
					nlinfo("Removing WelcomeServer '%s' for ShardId %u", first->second->getModuleName().c_str(), first->first);

					_WelcomeServices.erase(first);
					break;
				}
			}
		}
*/
	}

	bool CRingSessionManager::closeSession(TSessionId sessionId)
	{
		nldebug("RSM : closeSession : closing session %u", sessionId);
/*
		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
			return false;

		// the session exist, close it
		if (session->State != TSessionState::ss_open
			&& session->State != TSessionState::ss_locked)
		{
			nldebug("RSM : closeSession : the session %u in in %s state, must be open or locked", session->State.toString().c_str());
			return false;
		}

		session->State = TSessionState::ss_closed;
		session->FinalDuration = uint32(session->StartDate - NLMISC::CTime::getSecondsSince1970());
		session->update(_RingDb);

		// remove all users from the session
		session->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
		for (uint i=0; i<session->SessionParticipants.size(); ++i)
		{
			uint32 userId = session->SessionParticipants[i]->CharId>>4;
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				nlwarning("Session %u have character [%u:%u], but user %u is unknown",
					sessionId,
					session->SessionParticipants[i]->CharId >> 4,
					session->SessionParticipants[i]->CharId & 0xf,
					userId);
			}
			else
			{
				ru->CurrentSession = 0;
				ru->update(_RingDb);
			}
		}
*/
		return true;
	}

	const std::string &CRingSessionManager::getInitStringHelp()
	{
		static string help("web(port=<port>)");
		return help;
	}

	/////////////////////////////////////////////////////////////
	//// Web Interface stubs -- CRingSessionManagerWebItf
	/////////////////////////////////////////////////////////////

	// Generic response to invoke.
	// result contains 0 if no error, more than 0 in case of error
	void CRingSessionManager::invokeResult(NLNET::TSockId dest, uint32 resultCode, const std::string &resultString)
	{
		nldebug("CSimRingSessionManager::invokeResult = %d, %s", resultCode, resultString.c_str());

//		NLNET::CMessage message("RET");
//		message.serial(result);
//		message.serial(const_cast < std::string& > (resultString));

//		send(message, dest);

		CSimulationService::getSS().onInvokeResult( resultCode, resultString );
	}

	// Return the result of the session joining attempt
	// If join is ok, the shardAddr contain <ip:port> of the
	// Front end that wait for the player to come in and the.
	// participation mode for the character (editor, animator or player).
	// If ok, the web must return a page with a lua script.
	// that trigger the action handler 'on_connect_to_shard' :
	// <lua>runAH(nul, "on_connect_to_shard", "cookie=cookieValue|fsAddr=shardAddr|mode=participantStatus");<lua>
	// result : 0 : ok the client can join the session
	//          1 : char not found
	//          2 : session not found
	//          3 : no session participant for this character
	//          4 : can't find session server
	//          5 : shard hosting session is not reachable
	//          6 : nel user info not found
	//          7 : ring user not found
	//          8 : welcome service rejected connection request
	void CRingSessionManager::joinSessionResult(NLNET::TSockId dest, uint32 result, const std::string &shardAddr, const std::string &participantStatus)
	{
		nldebug("CRingSessionManagerWeb::joinSessionResult = %d, %s, %s", result, shardAddr.c_str(), participantStatus.c_str());

//		NLNET::CMessage message("JSSR");
//		message.serial(result);
//		message.serial(const_cast < std::string& > (shardAddr));
//		message.serial(const_cast < std::string& > (participantStatus));

//		send(message, dest);
	}

	/////////////////////////////////////////////////////////////
	//// CRingSessionManager interface impl
	/////////////////////////////////////////////////////////////

	// A edition or animation server module register in the session manager
	void CRingSessionManager::registerDSS(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions)
	{
		nldebug("RSM : receive DSS registration from '%s'", sender->getModuleName().c_str());

		if (_SessionServers.find(sender) != _SessionServers.end())
		{
			nlwarning("registerDSS : I already have info about the session server '%s'",
				sender->getModuleName().c_str());
			return;
		}

		TSessionServerInfo ssi;
		ssi.ShardId = shardId;
		for (uint i=0; i<runningSessions.size(); ++i)
		{
			const TRunningSessionInfo &rsi = runningSessions[i];
			ssi.HostedSessions.insert(make_pair(rsi.getSessionId(), rsi));
			ssi.NbTotalPlayingChars += rsi.getNbPlayingChars();

			_SessionIndex.insert(make_pair(rsi.getSessionId(), sender));

			// reopen temporary locked session
			if (_TemporaryLockedSession.find(rsi.getSessionId()) != _TemporaryLockedSession.end())
			{
/*					CSessionPtr session = CSession::load(_RingDb, rsi.SessionId, __FILE__, __LINE__);
				if (session == NULL)
				{
					nlwarning("registerDSS : can't load the session %u hosted by '%s'", rsi.SessionId, sender->getModuleName().c_str());
				}
				else
				{
					session->State = TSessionState::ss_open;
					session->update(_RingDb);
				}
*/
				_TemporaryLockedSession.erase(rsi.getSessionId());
			}
		}

		_SessionServers[sender] = ssi;

		CSimulationService::getSS().setDSSup( true );
	}

	// The session server report a session creation.
	void CRingSessionManager::sessionCreated(NLNET::IModuleProxy *sender, const RSMGR::TRunningSessionInfo &sessionInfo)
	{
		nldebug("RSM : DSS '%s' report session %u created", sender->getModuleName().c_str(), sessionInfo.getSessionId());
		TSockId from = NULL;
		// retrieve the pending session creation
		TPendingSessions::iterator it3=_PendingSessions.begin();
		while (it3 != _PendingSessions.end() && it3->SessionId != sessionInfo.getSessionId())
			++it3;
		if( it3 != _PendingSessions.end())
		{
			from = it3->From;
		}
		else
		{
			nlwarning("reportSessionEvent : can't find the pending web session for session %u",
				sessionInfo.getSessionId());
		}



		TSessionServers::iterator it(_SessionServers.find(sender));
		if (it == _SessionServers.end())
		{
			nlwarning("Module '%s' send sessionCreated but is unknown here",
				sender->getModuleName().c_str());

			if (from != NULL)
				invokeResult(from, 5, "Session server failure");
			return;
		}
		
		TSessionServerInfo &ssi = it->second;
		TSessionServerInfo::THostedSessions::iterator it2(ssi.HostedSessions.find(sessionInfo.getSessionId()));

		if (it2 != ssi.HostedSessions.end())
		{
			nlwarning("Server '%s' send sessionCreated for session %u that I already set on it",
				sender->getModuleName().c_str(),
				sessionInfo.getSessionId());
			if (from != NULL)
				invokeResult(from, 5, "Session server failure");
			return;
		}
/*
		CSessionPtr session = CSession::load(_RingDb, sessionInfo.getSessionId(), __FILE__, __LINE__);
		if (session == NULL)
		{
			nlwarning("Module %s send sessionCreated for session %u, but session is unknown in db",
				sender->getModuleName().c_str(),
				sessionInfo.getSessionId());
			if (from != NULL)
				invokeResult(from, 1, "Session not found");
			return;
		}
*/
		// set session info
		ssi.HostedSessions.insert(make_pair(sessionInfo.getSessionId(), sessionInfo));
		// update the index
		pair<TSessionServersIdx::iterator, bool> ret = _SessionIndex.insert(make_pair(sessionInfo.getSessionId(), sender));
		nlassert(ret.second);
/*
		// update session database
		session->State = TSessionState::ss_open;
		session->StartDate = NLMISC::CTime::getSecondsSince1970();
		// save the changes
		session->update(_RingDb);
*/
		if (true)	//from != NULL)
		{
			/// callback the web that is waiting for session startup
			// ok, the session is created
			invokeResult( from, 0, NLMISC::toString("sessionCreated %d", sessionInfo.getSessionId()) );

			_PendingSessions.erase(it3);
		}
		
		// that all
	}

	// The session report an event.
	// char id is used only when the event is about a character.
	void CRingSessionManager::reportSessionEvent(NLNET::IModuleProxy *sender, RSMGR::TSessionEvent event,
		TSessionId sessionId, uint32 charId)
	{
		nldebug("RSM : DSS '%s' report session event %s for session %u, char %u",
			sender->getModuleName().c_str(),
			event.toString().c_str(),
			sessionId,
			charId);

		TSessionServers::iterator it(_SessionServers.find(sender));
		if (it == _SessionServers.end())
		{
			nlwarning("Module '%s' send session event but is unknown here",
				sender->getModuleName().c_str());
			return;
		}
		
		TSessionServerInfo &ssi = it->second;
		TSessionServerInfo::THostedSessions::iterator it2(ssi.HostedSessions.find(sessionId));

		if (it2 == ssi.HostedSessions.end())
		{
			nlwarning("Server '%s' report event on session %u that I don't set on it",
				sender->getModuleName().c_str(),
				sessionId);
			return;
		}
		TRunningSessionInfo &rsi = it2->second;
/*
		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			nlwarning("Module %s report event for session %u, but session is unknown in db",
				sender->getModuleName().c_str(),
				sessionId);
			return;
		}

		CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
		if (character == NULL && event != TSessionEvent::se_session_closing)
		{
			nlwarning("Module %s report event for session %u and character %u, but character is unknown in db",
				sender->getModuleName().c_str(),
				sessionId,
				charId);
			return;
		}
*/
		switch(event.getValue())
		{
		case TSessionEvent::se_session_closing:
			{
				closeSession(sessionId);

				/// remove the running session
				ssi.HostedSessions.erase(sessionId);
			}
			break;
		case TSessionEvent::se_char_enter:
			{
/*
				if (session->FreeSlots > 1)
				{
					session->FreeSlots--;
					session->update(_RingDb);
				}
*/
				++ssi.NbTotalPlayingChars;
				rsi.setNbPlayingChars(rsi.getNbPlayingChars()+1);
			}
			break;
		case TSessionEvent::se_char_leave:
			{
				--ssi.NbTotalPlayingChars;
				rsi.setNbPlayingChars(rsi.getNbPlayingChars()-1);
/*
				session->FreeSlots++;
				session->update(_RingDb);
*/
			}
			break;
		}
	}

	/////////////////////////////////////////////////////////////
	//// Welcome service client messages
	/////////////////////////////////////////////////////////////
/*
	// Register the welcome service in the ring session manager
	virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId)
	{
		nlinfo("Adding WelcomeServer '%s' for ShardId %u", sender->getModuleName().c_str(), shardId);
		// store the module proxy pinter
		_WelcomeServices[shardId] = sender;
	}

	// return for welcome user
	virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr)
	{
		nldebug("RSM : WS %u send welcome result for user %u : %s (address = %s)",
			sender->getModuleName().c_str(),
			userId,
			ok ? "OK" : "NOT OK",
			shardAddr.c_str());

		// retrieve the pending join
		TPendingJoins::iterator first(_PendingJoins.begin()), last(_PendingJoins.end());
		for (; first != last; ++first)
		{
			TPendingJoinSession &pjs = *first;
			if (pjs.UserId == userId)
			{
				// it's this one !
				if (ok)
				{
					// retrieve character data
					CCharacterPtr character = CCharacter::load(_RingDb, pjs.CharId, __FILE__, __LINE__);
					if (character == NULL)
					{
						nlwarning("welcomeUserResult : character %u is not found", pjs.CharId);

						// char not found
						joinSessionResult(pjs.From, 1, "Character not found", "");
					}
					else
					{
						// retrieve the session participant
						character->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
						TSessionPartStatus status;
						
						vector<CSessionParticipantPtr>::iterator first(character->SessionParticipants.begin()), last(character->SessionParticipants.end());
						for (; first != last; ++first)
						{
							CSessionParticipantPtr &sp = *first;
							if (sp->getSessionId() == pjs.SessionId)
							{
								status = sp->Status;
								break;
							}
						}
						if (first == last)
						{
							// session participant not found
							joinSessionResult(pjs.From, 3, "No participation found", "");
						}
						else
						{
							// return info to the web
							joinSessionResult(pjs.From, 0, shardAddr, status.toString());
						}
					}
				}
				else
				{
					// conn refused
					joinSessionResult(pjs.From, 8, "Wecome refused entry in shard", "");
				}
				
				// remove the pending join record
				_PendingJoins.erase(first);
				return;
			}
		}
		
		nlwarning("welcomeUserResult : received a result for unexpected userId %u from WS '%s'",
			userId,
			sender->getModuleName().c_str());
	}
*/
	/////////////////////////////////////////////////////////////
	//// Web callback implementation
	/////////////////////////////////////////////////////////////
/*
	/// Connection callback : a new interface client connect
	virtual void on_CRingSessionManagerWeb_Connection(NLNET::TSockId from)
	{}

	virtual void on_CRingSessionManagerWeb_Disconnection(NLNET::TSockId from)
	{
		// we have lost a browser connection

		// cleanup pending session creation
		{
  			TPendingSessions::iterator first(_PendingSessions.begin()), last(_PendingSessions.end());
			for (; first != last; ++first)
			{
				TPendingSessionCreateInfo &psci = *first;

				if (psci.From == from)
				{
					psci.From = NULL;
				}
			}
		}
		// cleanup pending join session
		{
			TPendingJoins::iterator first(_PendingJoins.begin()), last(_PendingJoins.end());
			for (; first != last; ++first)
			{
				TPendingJoinSession &pjs = *first;
				if (pjs.From == from)
				{
					pjs.From = NULL;
				}
			}
		}
	}


	virtual void on_scheduleSession(NLNET::TSockId from, 
		uint32 userId, 
		const std::string &sessionType, 
		const std::string &sessionTitle, 
		uint32 plannedDate, 
		const std::string &sessionDesc, 
		const std::string &sessionLevel, 
		const std::string &accessType, 
		const std::string &ruleType, 
		const std::string &estimatedDuration)
	{
		nldebug("RSM : web schedule a new '%s' session", sessionType.c_str());

		TSessionType st(sessionType);
		if (st == TSessionType::invalid)
		{
			// failed !, invalid session type
			scheduleSessionResult(from, userId, 0, 1);
		}
		TSessionLevel sl(sessionLevel);
		if (sl == TSessionLevel::invalid)
		{
			// failed !, invalid level
			scheduleSessionResult(from, userId, 0, 2);
		}
		TAccessType at(accessType);
		if (at == TAccessType::invalid)
		{
			// failed !, invalid access type
			scheduleSessionResult(from, userId, 0, 5);
		}
		TRuleType rt(ruleType);
		if (rt == TRuleType::invalid)
		{
			// failed !, invalid rule type
			scheduleSessionResult(from, userId, 0, 6);
		}
		TEstimatedDuration ed(estimatedDuration);
		if (ed == TEstimatedDuration::invalid)
		{
			// failed !, invalid rule type
			scheduleSessionResult(from, userId, 0, 7);
		}

		// first, check for session already in hands of this user
		CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// failed !, invalid user 
			scheduleSessionResult(from, userId, 0, 3);
			return;
		}
		// load all the session created by this user
	//			ru->loadSessions(_RingDb, __FILE__, __LINE__);
	//
	//			for (uint i=0; i<ru->Sessions.size(); ++i)
	//			{
	//				CSessionPtr session = ru->Sessions[i];
	//				if (session->State == TSessionState::ss_open)
	//				{
	//					// oups, there is already a running session for this user !
	//					scheduleSessionResult(from, userId, 0, 4);
	//					return;
	//				}
	//			}

		// ok, we can create a session
		CSessionPtr session = CSession::createTransient(__FILE__, __LINE__);
		session->OwnerId = userId;
		session->SessionType = st;
		session->Title = sessionTitle;
		session->PlanDate = plannedDate;
		session->StartDate = 0; // mean not started
		session->Description = sessionDesc;
		session->Level = sl;
		session->RuleType = rt;
		session->AccessType = at;
		session->State = TSessionState::ss_planned;
		session->FreeSlots = 0;
		session->EstimatedDuration = ed;
		session->FinalDuration = 0;
		session->FolderId = 0;

		// store the session
		session->create(_RingDb);
		
		// ok, return the result to web
		scheduleSessionResult(from, userId, session->getObjectId(), 0);
	}

	// Cancel a planned session
	// Return 'invokeResult' : 0 : ok, session canceled
	//                         1 : unknown user
	//                         2 : unknown session
	//						   3 : user don't own the session
	//						   4 : session not in planned state
	virtual void on_cancelSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId)
	{
		nldebug("RSM : web cancel planned session %u with user %u", sessionId, userId);

		CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			// unknown user
			invokeResult(from, 1, "User not found");
			return;
		}

		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			// unknown session
			invokeResult(from, 2, "Session not found");
			return;
		}

		if (session->OwnerId != userId)
		{
			// not owner !
			invokeResult(from, 3, "User not owner of session");
			return;
		}

		if (session->State != TSessionState::ss_planned 
			&& session->State != TSessionState::ss_locked)
		{
			// not planned
			invokeResult(from, 4, "Session not in planned state");
			return;
		}
		
		// remove the session from database
		session->remove(_RingDb);
		// remove from sessions vector of user if any
		vector<CSessionPtr>::iterator it = find(ru->Sessions.begin(), ru->Sessions.end(), session);
		if (it != ru->Sessions.end())
		{
			ru->Sessions.erase(it);
		}

		// return result ok
		invokeResult(from, 0, "");
	}
*/
	// start a planned session
	// Return 'invokeResult' : 0 : ok, session started
	//                         1 : user not found
	//                         2 : session not found
	//                         3 : session not owned by user
	//                         4 : user is already in a session
	void CRingSessionManager::on_startSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId)
	{
		nldebug("RSM : web start session %u with user %u", sessionId, userId);
/*
		CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			invokeResult(from, 1, "User not found");
			return;
		}

		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			invokeResult(from, 2, "Session not found");
			return;
		}

		if (session->OwnerId != userId)
		{
			invokeResult(from, 3, "User not owner of session");
			return;
		}

		if (ru->CurrentSession != 0)
		{
			// the user is already in a session!
			invokeResult(from, 4, "User already have a current session");
			return;
		}
*/
		// AJM: verify that a server has not reported this session as running
		//	(since we are not loading session info per user from the db)
		TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
		if (it != _SessionIndex.end())
		{
			invokeResult(from, 0, toString("startSession %d : session already started", sessionId) );
			return;
		}
		
		// look for the best compatible server
		TSessionServers::iterator best(_SessionServers.end());
		TSessionServers::iterator first(_SessionServers.begin()), last(_SessionServers.end());

		for (; first != last; ++first)
		{
			if ( true // AJM first->second.ServerType == session->SessionType
				&& (best == _SessionServers.end() || best->second.NbTotalPlayingChars > first->second.NbTotalPlayingChars) )
			{
				// this server good is better (i.e had less player in it)
				best = first;
			}
		}
		if (best == _SessionServers.end())
		{
			// oups! no server to host this session !
			invokeResult(from, 5, "No server to host the session");
			return;
		}

		IModuleProxy *server = best->first;

		uint32 ownerCharId = userId << 4;

		// ask the session server to open the session
		CRingSessionManagerClientProxy rsmc(server);
//		rsmc.createSession(this, session->getObjectId(), session->SessionType);
		rsmc.createSession(this, ownerCharId, sessionId, RSMGR::TSessionType::st_edit);	// AJM


		// insert pending session info
		TPendingSessionCreateInfo psci;
		psci.From = from;
		psci.SessionId = sessionId;
		_PendingSessions.push_back(psci);

		// the return to web site will be done when DSS respond
	}

	// Close a running session
	// Return 'invokeResult' : 0 : ok, session closed (or about to close)
	//                         1 : session not found
	//                         2 : user don't own the session
	void CRingSessionManager::on_closeSession(NLNET::TSockId from, uint32 userId, TSessionId sessionId)
	{
		nldebug("RSM : web close session %u with user %u", sessionId, userId);
/*
		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			invokeResult(from, 1, "session not found");
			return;
		}

		if (session->OwnerId != userId)
		{
			invokeResult(from, 2, "user not own the session");
			return;
		}

		if (session->State == TSessionState::ss_closed
			||session->State == TSessionState::ss_planned)
		{
			invokeResult(from, 3, "session not open");
			return;
		}
*/
		// ask the session server to close this session
		TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
		if (it == _SessionIndex.end())
		{
			nlwarning("RSM:closeSession : could not find server for open session %u, closing the session", sessionId);

			closeSession(sessionId);
		}
		else
		{
			TModuleProxyPtr proxy = it->second;

			CRingSessionManagerClientProxy ss(proxy);

			ss.closeSession(this, sessionId);
		}

		// ok, all is good
		invokeResult(from, 0, "");
	}

	// Add a character in a user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char already friend
	void CRingSessionManager::on_addFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
	{
		nlstop;
	}

	// Remove a character from a user friend list
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : friend char not found
	//                         3 : char not friend
	void CRingSessionManager::on_removeFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
	{
		nlstop;
	}
/*
	// Add a character to a user ban list. This ban the user that own the character
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : banned char not found
	//                         3 : char already banned by user
	virtual void on_addBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
	{
		nlstop;
	}

	// Remove a character from a user ban list.
	// Return 'invokeResult' : 0 : ok
	//                         1 : user not found
	//                         2 : banned char not found
	//                         3 : char not banned by user
	virtual void on_removeBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
	{
		nlstop;
	}
	*/
	/*
	// A user invite a character to help or play in his session
	// charRole is from enum TSessionPartStatus
	// invokeReturn : 0 : ok, character invited
	//                1 : char not found
	//                2 : session not found
	//                3 : char already invited
	//                4 : char role and session type don't match (edit/editor, anim/animator)
	//                5 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
	virtual void on_inviteCharacter(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 charId, const std::string &charRole)
	{
		nldebug("RSM : web invite character %u as %s in session %u with user %u",
			charId, charRole.c_str(), sessionId, userId);

		CRingUserPtr user = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
		if (user == NULL)
		{
			invokeResult(from, 1, "User not found");
			return;
		}

		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			invokeResult(from, 2, "Session not found");
			return;
		}

		if (session->OwnerId != userId)
		{
			invokeResult(from, 4, "User not owner of session");
			return;
		}

		CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
		if (character == NULL)
		{
			invokeResult(from, 3, "Character not found");
			return;
		}

		character->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
		for (uint i=0; i<character->SessionParticipants.size(); ++i)
		{
			if (character->getSessionParticipants()[i]->getSessionId() == sessionId)
			{
				invokeResult(from, 5, "Participation already exist in session");
				return;
			}
		}

		TSessionPartStatus	role(charRole);
		if (role == TSessionPartStatus::invalid
			|| (	role != TSessionPartStatus::sps_play_invited
				&& role != TSessionPartStatus::sps_edit_invited
				&& role != TSessionPartStatus::sps_anim_invited
				)
			)
		{
			invokeResult(from, 7, "Invalid participation status");
			return;
		}

		// TODO : check for role and session type coherency

		// ok, all is fine, we can create in invitation entry for the character
		CSessionParticipantPtr ss = CSessionParticipant::createTransient(__FILE__, __LINE__);

		ss->CharId = charId;
		ss->SessionId = sessionId;
		ss->Status = role;

		if (!ss->create(_RingDb))
		{
			invokeResult(from, 8, "Failed to create the participation entry");
			return;
		}

		// finally, all was good, return 0
		invokeResult(from, 0, "");
	}

	// A user remove an invitation in a session
	// invokeReturn : 0 : ok, character invited
	//                1 : char not found
	//                2 : session not found
	//                3 : character already entered in session
	//                4 : invitation not found
	virtual void on_removeInvitedCharacter(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 charId)
	{
		nlstop;
	}
*/
	// A character ask to join a session.
	void CRingSessionManager::on_joinSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
	{
		nldebug("RSM : web ask char %u to join the session %u", charId, sessionId);

		// load the user
		uint32 userId = charId >> 4;
/*
		CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
		if (ru == NULL)
		{
			joinSessionResult(from, 7, "User not found", "");
			return;
		}
		// load the character
		CCharacterPtr charac = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
		if (charac == NULL)
		{
			joinSessionResult(from, 1, "Character not found", "");
			return;
		}
		// load the session
		CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
		if (session == NULL)
		{
			joinSessionResult(from, 2, "Session not found", "");
			return;
		}
		
		// load the session participant from the character
		charac->loadSessionParticipants(_RingDb, __FILE__, __LINE__);

		CSessionParticipantPtr sessionPart;
		// look for the entry pertaining to the session
		// and also, leave any active session
		for (uint i=0; i<charac->SessionParticipants.size(); ++i)
		{
			if (charac->getSessionParticipants()[i]->getSessionId() == sessionId)
			{
				// we found it
				sessionPart.assign(charac->SessionParticipants[i], __FILE__, __LINE__);
			}
			else if (charac->SessionParticipants[i]->Status.getValue() >= TSessionPartStatus::sps_playing)
			{
				nlinfo("RSM:joinSession : for char %u, changing active participation in session %u because joining session %u",
					charac->getObjectId(),
					charac->getSessionParticipants()[i]->getSessionId(),
					sessionId);
				// replace the participation in invited mode
				switch (charac->SessionParticipants[i]->Status.getValue())
				{
				case TSessionPartStatus::sps_playing:
					charac->SessionParticipants[i]->Status = TSessionPartStatus::sps_play_invited;
					break;
				case TSessionPartStatus::sps_editing:
					charac->SessionParticipants[i]->Status = TSessionPartStatus::sps_edit_invited;
					break;
				case TSessionPartStatus::sps_animating:
					charac->SessionParticipants[i]->Status = TSessionPartStatus::sps_anim_invited;
					break;
				}
				charac->SessionParticipants[i]->update(_RingDb);
			}
		}
		if (sessionPart == NULL)
		{
			joinSessionResult(from, 3, "Participation not found", "");
			return;

		}


		// load Nel user information
		CNelUserPtr nelUser = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
		if (nelUser == NULL)
		{
			joinSessionResult(from, 6, "Billing account not found", "");
			return;
		}
*/
		// retrieve the session server
		TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
		if (it == _SessionIndex.end())
		{
			// can't find server for this session !
			joinSessionResult(from, 4, "No server hosting the session", "");
			return;
		}

		CRingSessionManagerClientProxy ss(it->second);
		TSessionServerInfo &ssi = _SessionServers[it->second];
/*
		// retrieve the welcome service
		TWelcomeServices::iterator it3(_WelcomeServices.find(ssi.ShardId));
		if (it3 == _WelcomeServices.end())
		{
			// can't find a welcome service for this shard
			joinSessionResult(from, 5, "No welcome service to connect to shard hosting the session", "");
			return;
		}
		WS::CWelcomeServiceProxy wsp(it3->second);

		// retrieve the session info
		TSessionServerInfo::THostedSessions::iterator it2(ssi.HostedSessions.find(sessionId));
		nlassert(it2 != ssi.HostedSessions.end());

		TRunningSessionInfo &rsi = it2->second;

		WS::TUserRole	enterHas = WS::TUserRole::invalid;

		if (	sessionPart->Status == TSessionPartStatus::sps_edit_invited
			||	sessionPart->Status == TSessionPartStatus::sps_editing)
		{
*/
			// warn the session server that an editor come in
		ss.addCharacterInSession(this, sessionId, charId, WS::TUserRole::ur_editor, false);
/*
			enterHas = WS::TUserRole::ur_editor;

			// update the participant status
			sessionPart->Status = TSessionPartStatus::sps_editing;
		}
		else if (	sessionPart->Status == TSessionPartStatus::sps_anim_invited
			||	sessionPart->Status == TSessionPartStatus::sps_animating)
		{
			// warn the session server that an animator come in
			ss.addHelperCharacterInSession(this, sessionId, charId);
			enterHas = WS::TUserRole::ur_animator;
			// update the participant status
			sessionPart->Status = TSessionPartStatus::sps_animating;
		}
		else
		{
			enterHas = WS::TUserRole::ur_player;
			// update the participant status
			sessionPart->Status = TSessionPartStatus::sps_playing;
		}
		// save the participant status
		sessionPart->update(_RingDb);

		CLoginCookie cookie;
		cookie.setFromString(ru->Cookie);

		// reserve an entry place on the welcome service of the shard and wait for the response
		wsp.welcomeUser(this, 
			userId, 
			nelUser->LoginName, 
			cookie, 
			nelUser->Privilege, 
			nelUser->ExtendedPrivilege, 
			enterHas, 
			rsi.InstanceId);

		/// store info for waiting WS response
		TPendingJoinSession pjs;
		pjs.UserId = userId;
		pjs.CharId = charId;
		pjs.From = from;
		pjs.SessionId = sessionId;

		_PendingJoins.push_back(pjs);
*/
	}

/*
	// A user invite a guild to play in a session
	// invokeReturn : 0 : ok, guild invited
	//                1 : guild not found
	//                3 : session not found
	//                4 : guild already invited
	//                5 : user don't own the session
	virtual void on_inviteGuild(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 guildId)
	{
		nlstop;
	}

	// Remove a guild invitation in a session
	// invokeReturn : 0 : ok, guild invited
	//                1 : guild not found
	//                3 : session not found
	//                4 : guild not invited
	//                5 : user don't own the session
	virtual void on_removeInvitedGuild(NLNET::TSockId from, uint32 userId, TSessionId sessionId, uint32 guildId)
	{
		nlstop;
	}

	// Set the additional scenario info
	// playType is the enumerated type TPlayType
	// invokeReturn : 0 : ok, info setted
	//                1 : scenario not found
	//                2 : user not owner of session
	virtual void on_setScenarioInfo(NLNET::TSockId from, uint32 userId, TSessionId sessionId, const std::string &title, const std::string &journal, const std::string &credits, uint32 numPlayer, const std::string &playType)
	{
		nlstop;
	}

	// Add an entry in the session journal
	// invokeReturn : 0 : ok, entry added
	//                1 : scenario not found
	//                2 : user can't post in this journal
	virtual void on_addJournalEntry(NLNET::TSockId from, uint32 userId, TSessionId sessionId, const std::string &entryType, const std::string &text)
	{
		nlstop;
	}

	// Set the rating of a terminated (closed) session
	// invokeReturn : 0 : ok, rating added
	//                1 : scenario not found
	//                2 : char is not found
	//                3 : char is not a participant of session
	//                4 : session is not closed
	//                5 : char has already rated this session
	virtual void on_setPlayerRating(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 rating, const std::string &comments)
	{
		nlstop;
	}

	NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CRingSessionManager, CModuleBase)
		NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, dump, "dump the session manager internal state", "no param");
		NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, forceSessionCleanup, "force a database synchronisation with current running session", "no param");
	NLMISC_COMMAND_HANDLER_TABLE_END

	NLMISC_CLASS_COMMAND_DECL(forceSessionCleanup)
	{
		if (!_RingDb.query(string("SELECT session_id FROM sessions WHERE state = ")+toString(uint32(TSessionState::ss_open))))
		{
			log.displayNL("Error while running database query");
			return true;
		}

		set<uint32> sessionToClose;

		auto_ptr<MSW::CUseResult> result = _RingDb.useResult();

		while (result->fetchRow())
		{
			TSessionId sessionId;
			result->getField(0, sessionId);

			if (_SessionIndex.find(sessionId) == _SessionIndex.end())
			{
				// this session is not running !
				sessionToClose.insert(sessionId);
			}
		}

		while (!sessionToClose.empty())
		{
			TSessionId sessionId = *sessionToClose.begin();
			log.displayNL("Closing session %u", sessionId);
			CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);

			session->State = TSessionState::ss_closed;
			session->update(_RingDb);

			sessionToClose.erase(sessionToClose.begin());
		}

		return true;

	}
*/
	NLMISC_CLASS_COMMAND_DECL(dump)
	{
		// call the base class dump
//		NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

		log.displayNL("--------------------------------");
		log.displayNL("Dumping RingSessionManager state");
		log.displayNL("--------------------------------");
/*
		// dumping Known Welcome service
		{
			log.displayNL("Dumping %u known Welcome Service module :", _WelcomeServices.size());
			TWelcomeServices::iterator first(_WelcomeServices.begin()), last(_WelcomeServices.end());
			for (; first != last; ++first)
			{
				uint32 shardId = first->first;
				IModuleProxy *proxy = first->second;
				log.displayNL(" + Welcome service '%s' for shard %u", proxy->getModuleName().c_str(), shardId);
			}

		}

		// dumping known session server
		{
			log.displayNL("Dumping %u known session server :", _SessionServers.size());

			TSessionServers::iterator first(_SessionServers.begin()), last(_SessionServers.end());
			for( ; first != last; ++first)
			{
				IModuleProxy *server = first->first;
				TSessionServerInfo &ssi = first->second;
				
				log.displayNL(" + Session server '%s' :", server->getModuleName().c_str());
				log.displayNL("              Shard ID = %u", ssi.ShardId);
				log.displayNL("           Server Type = %s", ssi.ServerType.toString().c_str());
				log.displayNL("          Total player = %u", ssi.NbTotalPlayingChars);
				log.displayNL("   Nb running sessions = %u", ssi.HostedSessions.size());

				// dumping the sessions
				{
					TSessionServerInfo::THostedSessions::iterator first(ssi.HostedSessions.begin()), last(ssi.HostedSessions.end());
					for (; first != last; ++first)
					{
						TSessionId sessionId = first->first;
						TRunningSessionInfo &rsi = first->second;
						log.displayNL("    + Session %u :", sessionId);
						log.displayNL("       Number of player = %u", rsi.NbPlayingChars);
						log.displayNL("             InstanceId = %u", rsi.InstanceId);
					}
				}
			}
		}
*/
		return true;
	}		

	NLNET_REGISTER_MODULE_FACTORY(CRingSessionManager, RingSessionManagerClassName);

} // namespace RSMGR

