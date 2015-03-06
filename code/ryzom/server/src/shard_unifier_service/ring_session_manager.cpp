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

#include "nel/net/module_builder_parts.h"
#include "nel/net/callback_server.h"

#include "game_share/welcome_service_itf.h"
#include "game_share/r2_types.h"


#include "game_share/ryzom_entity_id.h"
#include "game_share/ring_session_manager_itf.h"
#include "game_share/misc_const.h"
#include "game_share/utils.h"
#include "server_share/mysql_wrapper.h"
#include "game_share/shard_names.h"
#include "game_share/security_check.h"

#include "ring_session_manager.h"
#include "database_mapping.h"
#include "nel_database_mapping.h"
#include "character_sync.h"

// defined by MySQL config file
#ifdef access
#undef access
#endif

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace MSW;
using namespace CHARSYNC;
using namespace R2;
//CVariable<uint32>	ForceWelcomerShard( "su", "ForceWelcomerShard", "SessionId of a shard to promote when joining any mainland (0=none)", 0, 0, true );
//CVariable<string>	DomainName("su", "DomainName", "The name of the domain", "ryzom", 0, true);

CVariable<uint32>	ShardAssocTimeout( "su", "ShardAssocTimeout", "Timeout (in s)before a temporary shard assoc is removed after the session is closed", 10*60, 0, true );
CVariable<uint32>	ForceWelcomerShard( "su", "ForceWelcomerShard", "SessionId of a shard to promote when joining any mainland (0=none)", 0, 0, true );
CVariable<string>	DomainName("su", "DomainName", "The name of the domain", "ryzom", 0, true);
CVariable<string>	PrivilegeForSessionAccess("su", "PrivilegeForSessionAccess", "A list of privilege allowed to join any animation session", ":GM:SGM:", 0, true);

namespace RSMGR
{

	class CRingSessionManager :
		public CEmptyModuleServiceBehav<CEmptyModuleCommBehav<CEmptySocketBehav<CModuleBase> > >,
		public CRingSessionManagerSkel,
		public CRingSessionManagerWebItf,
		public WS::CWelcomeServiceClientSkel,
		public ICharacterSyncCb
	{
		typedef uint32	TCharId;
		typedef uint32	TShardId;
//		typedef uint32	TSessionId;

		// mysql ring database connection
		MSW::CConnection _RingDb;
		// mysql nel database connection
		MSW::CConnection _NelDb;


		struct TSessionServerInfo
		{
			/// Shard id
			TShardId			ShardId;
			/// Total number of player in the sessions hosted by this server
			uint32				NbTotalPlayingChars;

			typedef std::map<TSessionId, TRunningSessionInfo>	THostedSessions;
			/// List of session running on this server
			THostedSessions		HostedSessions;

			TSessionServerInfo()
				: NbTotalPlayingChars(0)
			{
			}
		};

		typedef map<TModuleProxyPtr, TSessionServerInfo>	TSessionServers;
		/// known session servers
		TSessionServers			_SessionServers;

		typedef map<TSessionId, TModuleProxyPtr>		TSessionServersIdx;
		// index of sessionId to session server
		TSessionServersIdx		_SessionIndex;

		struct TWelcomeServiceInfo
		{
			TModuleProxyPtr		WSModuleProxy;
			uint32				NbOnlinePlayers;
			uint32				NbPendingPlayers;
			TSessionId			FixedSessionId; // only for mainland shards, otherwise 0
			/// Info stored in database
			CShardPtr			ShardInfo;
		};

		typedef std::map<TShardId, TWelcomeServiceInfo>	TWelcomeServices;
		/// List of known welcome service module for each shard
		TWelcomeServices		_WelcomeServices;

		typedef set<TSessionId>		TSessionSet;
		/// List of open session temporary lock until a session server claim to host them
		/// or the owner close the session.
		TSessionSet				_TemporaryLockedSession;


		struct TPendingSessionCreateInfo
		{
			/// The character that created the session
			uint32		CharId;
			/// the session id
			TSessionId	SessionId;
			/// the web connection that wait the response
			TSockId		From;
		};
		typedef list<TPendingSessionCreateInfo>	TPendingSessions;
		/// The list of session pending create result from DSS
		TPendingSessions	_PendingSessions;


		/// This struct hold data that temporary stuck a character with a ring
		/// shard.
		struct TTemporaryShardAssoc
		{
			/// The last owned session
			TSessionId		SessionId;
			/// The date of closing of the session (0 if not closed)
			uint32			SessionClosedDate;
			/// the shard on witch the session is/was open
			TModuleProxyPtr	DSSProxy;

		};
		typedef map<TCharId, TTemporaryShardAssoc>	TTemporaryShardAssocs;
		/** A table of session owner and the ring shard on witch they are
		 *	currently stuck. This allow the RSM to assign edit and anim session
		 *	of a character to the same DSS to use the DSS session ID masquerading
		 *	that allow reuse of the same session for editing and animating without
		 *	redownloading the scenario data.
		 *	When a session is closed, the record is removed from this table after
		 *	10 mn.
		 */
//		TTemporaryShardAssocs	_TemporaryShardAssocs;

//		// Version when CSecurityCode is big (ex: typedefed to CHashCodeMD5)
//		class COptionalSecurityCode
//		{
//		public:
//			COptionalSecurityCode() : _SecurityCode(NULL) {}
//			~COptionalSecurityCode() { if (_SecurityCode) { delete _SecurityCode; _SecurityCode = NULL; } }
//			void setSecurityCode(const CSecurityCode& sc) { if (! _SecurityCode) _SecurityCode = new CSecurityCode(); *_SecurityCode = sc; }
//			const CSecurityCode * getSecurityCode() const { return _SecurityCode; } // return NULL if not included
//		private:
//			CSecurityCode	*_SecurityCode;
//		};
		// Version when CSecurityCode is small
		class COptionalSecurityCode
		{
		public:
			COptionalSecurityCode() : _Enabled(false) {}
			void setSecurityCode(const CSecurityCode& sc) { _SecurityCode = sc; _Enabled = true; }
			const CSecurityCode * getSecurityCode() const { if (_Enabled) return &_SecurityCode; else return NULL; } // return NULL if not included

		private:
			CSecurityCode _SecurityCode;
			bool _Enabled;
		};

		struct TPendingJoinSession
		{
			/// The user ID
			uint32		UserId;
			/// the session id
			TSessionId	SessionId;
			/// The web connection
			TSockId		From;
			/// The character id
			uint32		CharId;
			/// Optional Security Code
			COptionalSecurityCode OptSecurityCode;
		};

		typedef list<TPendingJoinSession>	TPendingJoins;
		/// The list of character join session pending result from WS
		TPendingJoins	_PendingJoins;


		/// Disable checking of permission
		bool			_DontUsePerm;

		/// Pointer to character sync speaker
		ICharacterSync	*_CharSync;
	public:
		CRingSessionManager()
			:	_DontUsePerm(false),
				_CharSync(NULL)
		{
			CRingSessionManagerSkel::init(this);
			CWelcomeServiceClientSkel::init(this);
		}


		static const std::string &getInitStringHelp()
		{
			static string help("web(port=<port>)");
			return help;
		}

		bool initModule(const TParsedCommandLine &initInfo)
		{
			nldebug("RSM : initModule with '%s'", initInfo.toString().c_str());
			// this module require task message dispatching
			TParsedCommandLine baseInfo(initInfo);

			TParsedCommandLine *base = new TParsedCommandLine;
			base->ParamName = "base";
			TParsedCommandLine *taskDispatch = new TParsedCommandLine;
			taskDispatch->ParamName = "useCoTaskDispatch";
			base->SubParams.push_back(taskDispatch);

			baseInfo.SubParams.push_back(base);

			if (!CModuleBase::initModule(baseInfo))
				return false;

			// init ring db
			const TParsedCommandLine *initRingDb = initInfo.getParam("ring_db");
			if (initRingDb  == NULL)
			{
				nlwarning("RSM : missing ring db connection information");
				return false;
			}

			const TParsedCommandLine *noPerm= initInfo.getParam("noPerm");
			if (noPerm != NULL)
			{
				nlwarning("RSM : set mode to do not check permission table");

				_DontUsePerm = true;
			}

			// connect to the database
			if (!_RingDb.connect(*initRingDb))
			{
				nlwarning("Failed to connect to database using %s", initRingDb->toString().c_str());
				return false;
			}

			// init nel db
			const TParsedCommandLine *initNelDb = initInfo.getParam("nel_db");
			if (initNelDb  == NULL)
			{
				nlwarning("RSM : missing nel db connection information");
				return false;
			}

			// connect to the database
			if (!_NelDb.connect(*initNelDb))
			{
				nlwarning("Failed to connect to database using %s", initNelDb->toString().c_str());
				return false;
			}


			// init web interface
			const TParsedCommandLine *web = initInfo.getParam("web");
			if (web == NULL)
			{
				nlwarning("Missing 'web' param in module init string");
				return false;
			}

			const TParsedCommandLine *portParam = web->getParam("port");
			if (portParam == NULL)
			{
				nlwarning("Missing 'web.port' param in module init string");
				return false;
			}

			// cleanup the session state : for each open and locked session, set it to locked and remember it as previously open
			string query("SELECT session_id FROM sessions WHERE state = 'ss_open' OR state = 'ss_locked'");
			nlverify(_RingDb.query(query));

			list<TSessionId> sessionIds;
			{
				// scope the result to make it destroyed at scope end
				auto_ptr<MSW::CUseResult> result(_RingDb.useResult());

				// for each open session, put it in the locked state
				while (result->fetchRow())
				{
					uint32 sessionNum;
					result->getField(0, sessionNum);
					sessionIds.push_back(TSessionId(sessionNum));
				}
			}

			while (!sessionIds.empty())
			{
				TSessionId sessionId = sessionIds.front();
				sessionIds.pop_front();

				// load the session
				CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
				nlassert(session != NULL);

				_TemporaryLockedSession.insert(sessionId);
				session->setState(TSessionState::ss_locked);
				session->update(_RingDb);
			}

			uint16 port;
			NLMISC::fromString(portParam->ParamValue, port);

			openItf(port);

			return true;
		}

//		void onProcessModuleMessage(IModuleProxy *sender, const CMessage &message)
//		{
//			if (CRingSessionManagerSkel::onDispatchMessage(sender, message))
//				return;
//			else if (CWelcomeServiceClientSkel::onDispatchMessage(sender, message))
//				return;
//
//			nlwarning("CRingSessionManager : Unknown message '%s' received", message.getName().c_str());
//		}


		// the character sync speaker is dead
		void speakerIsDead	(ISpeaker *speaker)
		{
			if (speaker == _CharSync)
			{
				_CharSync = NULL;
			}
		}


		void onModuleUpdate()
		{
			H_AUTO(CRingSessionManager_onModuleUpdate);

			// try to connect to the character sync speaker
			if (_CharSync == NULL)
			{
				_CharSync = ICharacterSync::getInstance();
				if (_CharSync != NULL)
				{
					// register the listener
					_CharSync->registerListener(this);
				}
			}

			// update the php interface
			CRingSessionManagerWebItf::update();

//			// check for temporary assoc expiration
//			uint32 now = CTime::getSecondsSince1970();
//			TTemporaryShardAssocs::iterator first(_TemporaryShardAssocs.begin()), last(_TemporaryShardAssocs.end());
//			while (first != last)
//			{
//				TTemporaryShardAssoc &tsa = first->second;
//				if (tsa.SessionClosedDate != 0 && tsa.SessionClosedDate + ShardAssocTimeout < now)
//				{
//					// time to remove the assoc
//					TTemporaryShardAssocs::iterator tmp = first;
//					++first;
//					_TemporaryShardAssocs.erase(tmp);
//				}
//				else
//					++first;
//			}
		}


		void onModuleDown(IModuleProxy *proxy)
		{
			H_AUTO(SessionManager_onModuleUpdate);
			TSessionServers::iterator it(_SessionServers.find(proxy));

			if (it != _SessionServers.end())
			{
				nldebug("RSM : receive module down for session server '%s'", proxy->getModuleName().c_str());

				// lock any sessions that belong to this module (because we don't now if
				// the session server if just unreachable or really dead)
				TSessionServerInfo &ssi = it->second;
				{
					TSessionServerInfo::THostedSessions::iterator first(ssi.HostedSessions.begin()), last(ssi.HostedSessions.end());
					for (; first != last; ++first)
					{
						TSessionId sessionId = first->first;
						CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
						BOMB_IF(session == NULL, "Failed to load session "<<sessionId<<" from database, but the session is running a some DSS", continue);

						// lock the session
						session->setState(TSessionState::ss_locked);
						session->update(_RingDb);

						// store it in the locked session set
						_TemporaryLockedSession.insert(sessionId);

						{
							// check if we have a web callback pending
							TPendingSessions::iterator it(_PendingSessions.begin());
							while (it != _PendingSessions.end())
							{
								if (it->SessionId == sessionId)
								{
									// session server failure !
									invokeResult(it->From, (it->CharId)>>4, 5, "Session server failure");
									_PendingSessions.erase(it);
									// restart because the iterator is invalid
									it = _PendingSessions.begin();
								}
								else
									++it;
							}

						}

						{
							// check if we have a web callback pending for session join
							TPendingJoins::iterator it(_PendingJoins.begin());
							while (it != _PendingJoins.end())
							{
								TPendingJoinSession &pjs = *it;
								if (pjs.SessionId == sessionId)
								{
									// session server failure !
//									joinSessionResult((NLNET::CBufSock*)NULL, 0, TSessionId(0), 9, "", TSessionPartStatus::invalid_val);

									joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 9, "", TSessionPartStatus::invalid_val);
									_PendingJoins.erase(it);
									// restart because the iterator is invalid
									it = _PendingJoins.begin();
								}
								else
									++it;
							}
						}
					}

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

				// erase the session info record of this session server
				_SessionServers.erase(it);

//				// check temporary session assoc
//				{
//					TTemporaryShardAssocs::iterator first(_TemporaryShardAssocs.begin()), last(_TemporaryShardAssocs.end());
//					while (first != last)
//					{
//						TTemporaryShardAssoc &tsa = first->second;
//
//						if (tsa.DSSProxy == proxy)
//						{
//							// remove this one
//							TTemporaryShardAssocs::iterator tmp = first;
//							++first;
//							_TemporaryShardAssocs.erase(tmp);
//						}
//						else
//							++first;
//					}
//				}
//
			}
			else if (proxy->getModuleClassName() == "WelcomeService")
			{
				nldebug("RSM : receive a module down for Welcome service module '%s'", proxy->getModuleName().c_str());
				// look in the welcome services
				TWelcomeServices::iterator first(_WelcomeServices.begin()), last(_WelcomeServices.end());
				for (; first != last; ++first)
				{
					if (first->second.WSModuleProxy == proxy)
					{
						nlinfo("Removing WelcomeServer '%s' for ShardId %u", first->second.WSModuleProxy->getModuleName().c_str(), first->first);

						TWelcomeServiceInfo &wsi = first->second;

						// update the database
						wsi.ShardInfo->setWSOnline(false);
						wsi.ShardInfo->setMOTD("Shard closed, WS unreachable");
						wsi.ShardInfo->update(_RingDb);

						_WelcomeServices.erase(first);
						break;
					}
				}
			}
		}

		~CRingSessionManager()
		{
		}

		bool closeSession(TSessionId sessionId)
		{
			H_AUTO(SessionManager_closeSession);

			nldebug("RSM : closeSession : closing session %u", sessionId.asInt());
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
				return false;

			// the session exist, close it
			if (session->getState() != TSessionState::ss_open
				&& session->getState() != TSessionState::ss_locked)
			{
				nldebug("RSM : closeSession : the session %u is in state '%s', must be open or locked", sessionId.asInt(), session->getState().toString().c_str());
				return false;
			}

			session->setState(TSessionState::ss_closed);
			session->setFinalDuration(uint32(session->getStartDate() - NLMISC::CTime::getSecondsSince1970()));
			session->update(_RingDb);

//			// update the temporary shard assocs if needed
//			TTemporaryShardAssocs::iterator it(_TemporaryShardAssocs.find(session->getOwnerId()));
//			if (it != _TemporaryShardAssocs.end() && it->second.SessionId.asInt() == session->getObjectId())
//			{
//				// ok, we can set the closing date
//				it->second.SessionClosedDate = CTime::getSecondsSince1970();
//			}
//
			// if this is an edit session, remove all users from the session
			if (session->getSessionType() == TSessionType::st_edit)
			{
				session->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
				for (uint i=0; i<session->getSessionParticipants().size(); ++i)
				{
					uint32 charId = session->getSessionParticipants()[i]->getCharId();
					CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
					if (character == NULL)
					{
						nlwarning("Session %u have character [%u:%u], but character %u is unknown",
							sessionId.asInt(),
							session->getSessionParticipants()[i]->getCharId() >> 4,
							session->getSessionParticipants()[i]->getCharId() & 0xf,
							charId);
					}
					else
					{
						character->setCurrentSession(0);
						character->update(_RingDb);
					}
				}
			}

			return true;
		}


		bool checkAccessRight(const vector<string> &userAccessPriv, const TAccessLevel &accessLevel)
		{
			for (uint i=0; i<userAccessPriv.size(); ++i)
			{
				string access("ds_");
				access += toLower(userAccessPriv[i]);

				if (accessLevel.toString() == access)
					return true;
			}

			return false;
		}


		// check if a user has the needed privilege to access any session
		bool hasSessionAccessPrivilege(uint32 userId)
		{
			// check if the user is a GM and have right to access the session
			CNelUserPtr user = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
			if (user != NULL)
			{
				vector<string> userPriv;
				explode(user->getPrivilege(), string(":"), userPriv, true);
				for (uint i=0; i<userPriv.size(); ++i)
				{
					if (PrivilegeForSessionAccess.toString().find(userPriv[i]) != string::npos)
					{
						// ok, the user have the required access
						return true;
					}
				}
			}

			return false;
		}

		// load a shard object for a given shard ID
//		CShardPtr loadShardInfo(uint32 shardId)
//		{
//			// load the data from DB
//			CSString query;
//			query << "SELECT shard.prim FROM shard, domain";
//			query << " WHERE shard.domain_id = domain.domain_id";
//			query << " AND domain_name = '" << DomainName << "'";
//			query << " AND ShardId = " << shardId;
//
//			BOMB_IF(!_NelDb.query(query), "registerWS : Failed to request into the NeL database", return CNelShardPtr());
//			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_NelDb.storeResult());
//			BOMB_IF(result.get() == NULL, "registerWS : Failed to retrieve request result", return CNelShardPtr());
//
//			if (result->getNumRows() == 0)
//			{
//				return CNelShardPtr();
//			}
//
//			result->fetchRow();
//			// load the data
//			uint32 prim;
//			result->getField(0, prim);
//
//			return CNelShard::load(_NelDb, prim, __FILE__, __LINE__);
//		}

		bool loadUserAccessPrivileges(uint32 userId, vector<string> &userAccessPriv)
		{
			userAccessPriv.clear();
			CSString query;
			query << "SELECT AccessPrivilege FROM permission";
			query << " WHERE UId = " << userId << " AND DomainId = (SELECT domain_id FROM domain WHERE domain_name = '" << DomainName << "')";

			if (!_NelDb.query(query))
			{
				// can't find a permission record
//				joinSessionResult(from, charId>>4, 0, 12, "failed to request for access permission", TSessionPartStatus::invalid_val);
				return false;
			}
			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_NelDb.storeResult());
			if (result->getNumRows() == 0)
			{
				return false;
				// can't find a permission record
//				joinSessionResult(from, charId>>4, 0, 13, "can't find access permission for user and domain", TSessionPartStatus::invalid_val);
//				return;
			}
			result->fetchRow();
			string accessString;
			result->getField(0, accessString);

			NLMISC::explode(accessString, string(","), userAccessPriv, false);

			return true;
		}


		/////////////////////////////////////////////////////////////
		//// CRingSessionManager interface impl
		/////////////////////////////////////////////////////////////

		// A edition or animation server module register in the session manager
		virtual void registerDSS(NLNET::IModuleProxy *sender, uint32 shardId, const std::vector < TRunningSessionInfo > &runningSessions)
		{
			nldebug("RSM : receive DSS registration from '%s'",
				sender->getModuleName().c_str());

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

				// check that we know about this session
				CSessionPtr session = CSession::load(_RingDb, rsi.getSessionId(), __FILE__, __LINE__);
				if (session == NULL )
				{
					nlwarning("registerDSS : can't load the session %u hosted by '%s, closing it",
						static_cast<uint32>(rsi.getSessionId()),
						sender->getModuleName().c_str());

					CRingSessionManagerClientProxy smc(sender);
					smc.closeSession(this, TSessionId(rsi.getSessionId()));

					// advance to next session
					continue;
				}

				// ok, the session is known

				// check that the session is open or locked
				if (session->getState() != TSessionState::ss_open && session->getState() != TSessionState::ss_locked)
				{
					nlwarning("registerDSS : The session %u open hosted by '%s' is '%s', asking DSS to close it",
						 static_cast<uint32>(rsi.getSessionId()),
						 sender->getModuleName().c_str(),
						 session->getState().toString().c_str());

					CRingSessionManagerClientProxy smc(sender);
					smc.closeSession(this, TSessionId(rsi.getSessionId()));

					// advance to next session
					continue;
				}

				ssi.HostedSessions.insert(make_pair(rsi.getSessionId(), rsi));
				ssi.NbTotalPlayingChars += rsi.getNbPlayingChars();

				_SessionIndex.insert(make_pair(rsi.getSessionId(), sender));

				// reopen temporary locked session
				if (_TemporaryLockedSession.find(rsi.getSessionId()) != _TemporaryLockedSession.end())
				{
//					CSessionPtr session = CSession::load(_RingDb, rsi.getSessionId(), __FILE__, __LINE__);
//					if (session == NULL)
//					{
//						nlwarning("registerDSS : can't load the session %u hosted by '%s'", rsi.getSessionId(), sender->getModuleName().c_str());
//					}
//					else
					{
						session->setState(TSessionState::ss_open);
						session->update(_RingDb);
					}

					_TemporaryLockedSession.erase(rsi.getSessionId());
				}

//				// update the temporary shard assoc
//				TTemporaryShardAssoc tsa;
//				tsa.SessionId = TSessionId(session->getObjectId());
//				tsa.SessionClosedDate = 0;
//				tsa.DSSProxy = sender;
//				_TemporaryShardAssocs[session->getOwnerId()] = tsa;
			}

				// close any session associated to this shard/type of session and not open on the server
			{
restartLoop:
				list<TSessionId>	sessionToClose;

				// first; look for sessions to close
				TSessionSet::iterator first(_TemporaryLockedSession.begin()), last(_TemporaryLockedSession.end());
				for (; first != last; ++first)
				{
					TSessionId sessionId = *first;
					CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
					BOMB_IF(session == NULL, "RSM: registerDSS : failed to load session "<<sessionId<<" from database", _TemporaryLockedSession.erase(first); goto restartLoop);

					if (session->getHostShardId() == shardId)
					{
						// this session is no more alive, we need to close it
						sessionToClose.push_back(sessionId);
					}
				}

				// second, close the sessions
				while (!sessionToClose.empty())
				{
					TSessionId sessionId = sessionToClose.front();

					nlinfo("RSM:registerDSS : closing dead session %u", sessionId.asInt());

					closeSession(sessionId);
					_TemporaryLockedSession.erase(sessionId);
					sessionToClose.pop_front();
				}

			}

			_SessionServers[sender] = ssi;
		}

		// The session server report a session creation.
		virtual void sessionCreated(NLNET::IModuleProxy *sender, const RSMGR::TRunningSessionInfo &sessionInfo)
		{
			H_AUTO(SessionManager_sessionCreated);

			nldebug("RSM : DSS '%s' report session %u created", sender->getModuleName().c_str(), static_cast<uint32>(sessionInfo.getSessionId()));
			TSockId from = NULL;
			// retrieve the pending session creation
			TPendingSessions::iterator it3=_PendingSessions.begin();
			while (it3 != _PendingSessions.end() && it3->SessionId.asInt() != sessionInfo.getSessionId())
				++it3;
			if( it3 != _PendingSessions.end())
			{
				from = it3->From;
			}
			else
			{
				nlwarning("reportSessionEvent : can't find the pending web session for session %u",
					static_cast<uint32>(sessionInfo.getSessionId()));
			}



			TSessionServers::iterator it(_SessionServers.find(sender));
			if (it == _SessionServers.end())
			{
				nlwarning("Module '%s' send sessionCreated but is unknown here",
					sender->getModuleName().c_str());

				if (from != NULL)
					invokeResult(from, (it3->CharId) >> 4, 5, "Session server failure");
				return;
			}

			TSessionServerInfo &ssi = it->second;
			TSessionServerInfo::THostedSessions::iterator it2(ssi.HostedSessions.find(sessionInfo.getSessionId()));

			if (it2 != ssi.HostedSessions.end())
			{
				nlwarning("Server '%s' send sessionCreated for session %u that I already set on it",
					sender->getModuleName().c_str(),
					static_cast<uint32>(sessionInfo.getSessionId()));
				if (from != NULL)
					invokeResult(from, (it3->CharId) >> 4, 5, "Session server failure");
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionInfo.getSessionId(), __FILE__, __LINE__);
			if (session == NULL)
			{
				nlwarning("Module %s send sessionCreated for session %u, but session is unknown in db",
					sender->getModuleName().c_str(),
					static_cast<uint32>(sessionInfo.getSessionId()));
				if (from != NULL)
					invokeResult(from, (it3->CharId) >> 4, 1, "Session not found");
				return;
			}

			// set session info
			ssi.HostedSessions.insert(make_pair(sessionInfo.getSessionId(), sessionInfo));
			// update the index
			pair<TSessionServersIdx::iterator, bool> ret = _SessionIndex.insert(make_pair(sessionInfo.getSessionId(), sender));
			nlassert(ret.second);

			// update session database
			session->setState(TSessionState::ss_open);
			session->setHostShardId(ssi.ShardId);
			session->setStartDate(NLMISC::CTime::getSecondsSince1970());
			// save the changes
			session->update(_RingDb);

			if (from != NULL)
			{
				/// callback the web that is waiting for session startup
				// ok, the session is created
				invokeResult(from, (it3->CharId)>>4, 0, "");

				_PendingSessions.erase(it3);
			}

			// that all
		}

		// The session report an event.
		// char id is used only when the event is about a character.
		virtual void reportSessionEvent(NLNET::IModuleProxy *sender, RSMGR::TSessionEvent event, TSessionId sessionId, uint32 charId)
		{
			H_AUTO(SessionManager_reportSessionEvent);

			nldebug("RSM : DSS '%s' report session event %s for session %u, char %u",
				sender->getModuleName().c_str(),
				event.toString().c_str(),
				sessionId.asInt(),
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
					sessionId.asInt());
				return;
			}
			TRunningSessionInfo &rsi = it2->second;

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				nlwarning("Module %s report event for session %u, but session is unknown in db",
					sender->getModuleName().c_str(),
					sessionId.asInt());
				return;
			}

			if (event != TSessionEvent::se_session_closing)
			{
				// the char must be valid
				CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
				if (character == NULL)
				{
					nlwarning("Module %s report event for session %u and character %u, but character is unknown in db",
						sender->getModuleName().c_str(),
						sessionId.asInt(),
						charId);
					return;
				}
			}

			switch(event.getValue())
			{
			case TSessionEvent::se_session_closing:
				{
					// update the DSS client count
					if (ssi.NbTotalPlayingChars > rsi.getNbPlayingChars())
						ssi.NbTotalPlayingChars -= rsi.getNbPlayingChars();
					else
						ssi.NbTotalPlayingChars = 0;

					closeSession(sessionId);

					/// Remove the session from the index
					_SessionIndex.erase(sessionId);
					/// remove the running session
					ssi.HostedSessions.erase(sessionId);
				}
				break;
			case TSessionEvent::se_char_enter:
				{
					++ssi.NbTotalPlayingChars;
					rsi.setNbPlayingChars(rsi.getNbPlayingChars()+1);
				}
				break;
			case TSessionEvent::se_char_leave:
				{
					if (ssi.NbTotalPlayingChars > 0)
						--ssi.NbTotalPlayingChars;
					rsi.setNbPlayingChars(rsi.getNbPlayingChars()-1);
				}
				break;
			}
		}

		// The DSS report that an animation scenario has just started
		// this allow SU to create the session log and scenario info record if needed.
		virtual void scenarioStarted(NLNET::IModuleProxy *sender, TSessionId sessionId, const R2::TRunningScenarioInfo &scenarioInfo)
		{
			H_AUTO(SessionManager_scenarioStarted);

			// load the session info
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "Failed to load session "<<sessionId<<" reported by DSS with 'scenarioStarted'", return);

			// retrieve the author id
			TSessionId authorHomeSessionId;
			string authorShortName;
			uint32 authorCharId = 0;
			CShardNames::getInstance().parseRelativeName(TSessionId(0), scenarioInfo.getScenarioAuthorName(), authorShortName, authorHomeSessionId);
			if (ICharacterSync::getInstance())
				authorCharId = ICharacterSync::getInstance()->findCharId(authorShortName, authorHomeSessionId);
			if (authorCharId == 0)
			{
				// try to found the character with 'best effort', ie. we don't know the session, look for a unique character
				authorCharId = ICharacterSync::getInstance()->findCharId(authorShortName);
			}

			// load the author
			CCharacterPtr author;
			if (authorCharId != 0)
			{
				author = CCharacter::load(_RingDb, authorCharId, __FILE__, __LINE__);
				BOMB_IF(author == NULL, "Failed to load the scenario author character "<<authorCharId, return;)
			}

			// load the animator
			CCharacterPtr animator = CCharacter::load(_RingDb, session->getOwnerId(), __FILE__, __LINE__);
			BOMB_IF(animator == NULL, "Failed to load the scenario animator character "<<session->getOwnerId(), return;)

			// try to load an existing scenario record
			CScenarioPtr scenario;
			CSString query;
			query << "SELECT id FROM scenario WHERE md5 = '"<<scenarioInfo.getScenarioKey().toString()<<"'";
			BOMB_IF(!_RingDb.query(query), "Failed to request in ring database", return;);
			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());
			if (result->getNumRows() != 0)
			{
				result->fetchRow();
				uint32 scenarioId;
				result->getField(0, scenarioId);

				scenario = CScenario::load(_RingDb, scenarioId, __FILE__, __LINE__);
				BOMB_IF(scenario == NULL, "Failed to load the scenario "<<scenarioId, return);
			}
			else
			{
				// create a new entry for the scenario
				scenario = CScenario::createTransient(__FILE__, __LINE__);
				scenario->setAnimMode(session->getAnimMode());
				scenario->setAuthor(scenarioInfo.getScenarioAuthorName());
				scenario->setDescription(session->getDescription());
				scenario->setLanguage(session->getLang());
				scenario->setLevel(session->getLevel());
				scenario->setMD5(scenarioInfo.getScenarioKey());
				scenario->setOrientation(session->getOrientation());
				scenario->setRRPTotal(0);
				scenario->setTitle(session->getTitle());

				scenario->create(_RingDb);
			}

			scenario->update(_RingDb);

			// update the author
			// create the session log record
			CSessionLogPtr sessionLog = CSessionLog::createTransient(__FILE__, __LINE__);
			sessionLog->setObjectId(sessionId.asInt());
			sessionLog->setScenarioId(scenario->getObjectId());
			sessionLog->setLaunchDate(session->getStartDate());
			sessionLog->setOwner(CShardNames::getInstance().makeFullName(animator->getCharName(), animator->getHomeMainlandSessionId()));
			sessionLog->setParticipants("");
			sessionLog->setRRPScored(0);
			sessionLog->setScenarioPointScored(0);
			sessionLog->setTimeTaken(0);
			sessionLog->setGuildName("");

			sessionLog->create(_RingDb);
		}


		void reportCharacterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId)
		{
			H_AUTO(SessionManager_reportCharacterKicked);

			nldebug("RSM : reportCharacterKicked : session server '%s' report character %u has been kicked from session %u",
				sender->getModuleName().c_str(),
				sessionId.asInt(),
				charId);

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			DROP_IF(session == NULL, ("RSM : reportCharacterKicked : failed to load the session "+toString(sessionId)+" from database"), return);
			DROP_IF(!session->loadSessionParticipants(_RingDb, __FILE__, __LINE__), ("RSM : reportCharacterKicked : failed to load particpants of session "+toString(sessionId)), return);

			// a little survival check
			DROP_IF(session->getOwnerId() == (charId >> 4), ("RSM : reportCharacterKicked : session server "+sender->getModuleName()+" try to kick a character of the session owner, forbidden"), return);

			// iterate over the participation, looking for the kicked character
			const std::vector<CSessionParticipantPtr> &parts = session->getSessionParticipants();
			for (uint i=0; i<parts.size(); ++i)
			{
				CSessionParticipantPtr part = parts[i];

				if (part->getCharId() == charId)
				{
					// we found it, update the record
					part->setKicked(true);

					part->update(_RingDb);

					return;
				}
			}

			nlwarning("RSM : reportCharacterKicked : failed to found participation for kicked character %u in session %u", charId, sessionId.asInt());
		}

		// The DSS report the end of an animation session and
		// provides a bunch of data about the session life.
		// The DSS report the end of an animation session and
		// provides a bunch of data about the session life.
		virtual void scenarioEnded(NLNET::IModuleProxy *sender,
			TSessionId sessionId,
			const R2::TRunningScenarioInfo &scenarioInfo,
			uint32 rrpScored,
			uint32 scenarioPointScored,
			uint32 timeTaken,
			const std::vector < uint32 > &participants)
		{
			H_AUTO(SessionManager_scenarioEnded);

			// load the session info
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "Failed to load session "<<sessionId<<" reported by DSS with 'scenarioEnded'", return);

			// retrieve the author id
			TSessionId authorHomeSessionId;
			string authorShortName;
			uint32 authorCharId = 0;
			CShardNames::getInstance().parseRelativeName(TSessionId(0), scenarioInfo.getScenarioAuthorName(), authorShortName, authorHomeSessionId);
			if (ICharacterSync::getInstance())
				authorCharId = ICharacterSync::getInstance()->findCharId(authorShortName, authorHomeSessionId);
			if (authorCharId == 0)
			{
				// try to found the character with 'best effort', ie. we don't know the session, look for a unique character
				authorCharId = ICharacterSync::getInstance()->findCharId(authorShortName);
			}

			// load the author
			CCharacterPtr author;
			if (authorCharId != 0)
			{
				author = CCharacter::load(_RingDb, authorCharId, __FILE__, __LINE__);
				BOMB_IF(author == NULL, "Failed to load the scenario author character "<<authorCharId, return;)
			}

			// load the animator
			CCharacterPtr animator = CCharacter::load(_RingDb, session->getOwnerId(), __FILE__, __LINE__);
			BOMB_IF(animator == NULL, "Failed to load the scenario animator character "<<session->getOwnerId(), return;)

			// try to load an existing scenario record
			CScenarioPtr scenario;
			CSString query;
			query << "SELECT id FROM scenario WHERE md5 = '"<<scenarioInfo.getScenarioKey().toString()<<"'";
			BOMB_IF(!_RingDb.query(query), "Failed to request in ring database", return;);
			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());
			if (result->getNumRows() != 0)
			{
				result->fetchRow();
				uint32 scenarioId;
				result->getField(0, scenarioId);

				scenario = CScenario::load(_RingDb, scenarioId, __FILE__, __LINE__);
				BOMB_IF(scenario == NULL, "Failed to load the scenario "<<scenarioId, return);
			}
			else
			{
				// create a new entry for the scenario
				scenario = CScenario::createTransient(__FILE__, __LINE__);
				scenario->setAnimMode(session->getAnimMode());
				scenario->setAuthor(scenarioInfo.getScenarioAuthorName());
				scenario->setDescription(session->getDescription());
				scenario->setLanguage(session->getLang());
				scenario->setLevel(session->getLevel());
				scenario->setMD5(scenarioInfo.getScenarioKey());
				scenario->setOrientation(session->getOrientation());
				scenario->setRRPTotal(0);
				scenario->setTitle(session->getTitle());

				scenario->create(_RingDb);
			}

			CSString participantStr;
			uint32 guildId = 0xffffffff;
			// build the participants string
			for (uint i=0; i<participants.size(); ++i)
			{
				// load the character
				CCharacterPtr character = CCharacter::load(_RingDb, participants[i], __FILE__, __LINE__);
				if (character != NULL)
				{
					// add the participant name
					participantStr << CShardNames::getInstance().makeFullName(character->getCharName(), character->getHomeMainlandSessionId());

					// check for guild scoring
					if (guildId == 0xffffffff)
						// first char, get it's guild
						guildId = character->getGuildId();
					else if (character->getGuildId() != guildId)
						// this char is from a different guild, no guild will be scored
						guildId = 0;
				}
			}

			// try to get the guild name (if any)
			string guildName;
			if (guildId != 0 && guildId != 0xffffffff)
			{
				CGuildPtr guild = CGuild::load(_RingDb, guildId, __FILE__, __LINE__);
				if (guild != NULL)
				{
					guildName = guild->getGuildName();
				}
			}

			// update the scenario info
			scenario->setRRPTotal(scenario->getRRPTotal() + rrpScored);
			scenario->update(_RingDb);


			// update the author
			if (author != NULL)
			{
				author->setRRPAuthor(author->getRRPAuthor() + rrpScored);
				author->update(_RingDb);
			}

			// update the animator
			if (scenarioInfo.getDMLess())
				animator->setRRPMasterless(animator->getRRPMasterless() + rrpScored);
			else
				animator->setRRPAM(animator->getRRPAM() + rrpScored);
			animator->update(_RingDb);

			// try to load the session log
			CSessionLogPtr sessionLog = CSessionLog::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (sessionLog == NULL)
			{
				// no session log record, create a new one
				sessionLog.assign(CSessionLog::createTransient(__FILE__, __LINE__), __FILE__, __LINE__);
				sessionLog->setObjectId(sessionId.asInt());
			}

			// update the session log
			sessionLog->setScenarioId(scenario->getObjectId());
			sessionLog->setLaunchDate(session->getStartDate());
			sessionLog->setOwner(CShardNames::getInstance().makeFullName(animator->getCharName(), animator->getHomeMainlandSessionId()));
			sessionLog->setParticipants(participantStr);
			sessionLog->setRRPScored(rrpScored);
			sessionLog->setScenarioPointScored(scenarioPointScored);
			sessionLog->setTimeTaken(timeTaken);
			sessionLog->setGuildName(guildName);

			if (sessionLog->getPersistentState() == NOPE::os_transient)
			{
				// new object, persist it
				sessionLog->create(_RingDb);
			}
			else
			{
				// update the db
				sessionLog->update(_RingDb);
			}
		}

		/////////////////////////////////////////////////////////////
		//// Welcome service client messages
		/////////////////////////////////////////////////////////////

		// Register the welcome service in the ring session manager
		virtual void registerWS(NLNET::IModuleProxy *sender, uint32 shardId, uint32 fixedSessionId, bool isOnline)
		{
			nlinfo("Adding WelcomeServer '%s' for ShardId %u", sender->getModuleName().c_str(), shardId);
			// store the module proxy pointer
			TWelcomeServiceInfo wsInfo;
			wsInfo.WSModuleProxy = sender;
			wsInfo.NbOnlinePlayers = 0;
			wsInfo.NbPendingPlayers = 0;
			wsInfo.FixedSessionId = TSessionId(fixedSessionId);

			// load the data from DB
			CShardPtr shard = CShard::load(_RingDb, shardId, __FILE__, __LINE__);
			if (shard == NULL)
			{
				// no record for this shard, create a new one
				wsInfo.ShardInfo = CShard::createTransient(__FILE__, __LINE__);
				wsInfo.ShardInfo->setObjectId(shardId);
				// new WS are always inserted in restricted mode
				wsInfo.ShardInfo->setWSOnline(false);
				wsInfo.ShardInfo->setRequiredState(TAccessLevel::ds_restricted);
				wsInfo.ShardInfo->setMOTD("Shard up, access restricted");

				// create the record in database
				wsInfo.ShardInfo->create(_RingDb);
			}
			else
			{
				wsInfo.ShardInfo = shard;
			}

			if (isOnline && wsInfo.ShardInfo->getRequiredState() != TAccessLevel::ds_close)
			{
				// the WS is online, check the previous known state of the
				// WS and put it in restricted if it was previously close.
				if (!wsInfo.ShardInfo->getWSOnline())
				{
					// the WS was previously known in closed state, put it in
					// the required mode
					wsInfo.ShardInfo->setMOTD("Shard up");
				}
				else
				{
					// we don't change previous the WS state and MOTD
				}
			}
			else
			{
				// The WS is closed or required to be closed, remember this in the ring DB
				wsInfo.ShardInfo->setMOTD("Shard down, access closed");
			}
			wsInfo.ShardInfo->setWSOnline(isOnline);
			wsInfo.ShardInfo->update(_RingDb);

			// Store this WS in the table
			_WelcomeServices.insert(std::make_pair(shardId, wsInfo));
		}

		// WS report it's current open state
		virtual void reportWSOpenState(NLNET::IModuleProxy *sender, bool isOnline)
		{
			nlinfo("Welcome service '%s' report online status '%s'", sender->getModuleName().c_str(), isOnline ? "true" : "false");

			for ( TWelcomeServices::iterator itw=_WelcomeServices.begin(); itw!=_WelcomeServices.end(); ++itw )
			{
				if (itw->second.WSModuleProxy == sender)
				{
					TWelcomeServiceInfo &wsi = itw->second;

					if (isOnline
						&& wsi.ShardInfo->getRequiredState() != TAccessLevel::ds_close
						&& !wsi.ShardInfo->getWSOnline())
					{
						// unclose this shard
						wsi.ShardInfo->setMOTD("Shard up");

						nldebug("Shard %u is now online", wsi.FixedSessionId.asInt());
					}
					else if (!isOnline && wsi.ShardInfo->getWSOnline())
					{
						// close this shard
						wsi.ShardInfo->setMOTD("Shard down, access closed");
						nldebug("Shard %u is now offline and closed", wsi.FixedSessionId.asInt());
					}
					// store the new WS state
					wsi.ShardInfo->setWSOnline(isOnline);

					wsi.ShardInfo->update(_RingDb);
					break;
				}
			}
		}

		// return for welcome user
		virtual void welcomeUserResult(NLNET::IModuleProxy *sender, uint32 userId, bool ok, const std::string &shardAddr, const std::string &errorMsg)
		{
			H_AUTO(SessionManager_welcomeUserResult);

			nldebug("RSM : WS '%s' send welcome result for user %u : %s (address = '%s', errorMsg = '%s')",
				sender->getModuleName().c_str(),
				userId,
				ok ? "OK" : "NOT OK",
				shardAddr.c_str(),
				errorMsg.c_str());

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
						// Is this to join a session or a shard?
						if (pjs.SessionId.asInt() != 0)
						{
							// joining a session => retrieve character data
							CCharacterPtr character = CCharacter::load(_RingDb, pjs.CharId, __FILE__, __LINE__);
							if (character == NULL)
							{
								nlwarning("welcomeUserResult : character %u is not found", pjs.CharId);

								// char not found
								joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 1, "Character not found", TSessionPartStatus::invalid_val);
							}
							else
							{
								// load the session info
								CSessionPtr session = CSession::load(_RingDb, pjs.SessionId, __FILE__, __LINE__);
								BOMB_IF(session == NULL, "welcomeUserResult : the session "<<pjs.SessionId<<" cannot be loaded from database", joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 11, "Failed to load the session", TSessionPartStatus::invalid_val); goto endOfWelcomeUserResult;);
								// retrieve the session participant
								character->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
								TSessionPartStatus status;

	//							vector<CSessionParticipantPtr>::iterator first(character->getSessionParticipants().begin()), last(character->getSessionParticipants().end());
	//							for (; first != last; ++first)
								uint i;
								for (i=0; i<character->getSessionParticipants().size(); ++i)
								{
									CSessionParticipantPtr &sp = character->getSessionParticipantsByIndex(i);
									if (sp->getSessionId() == pjs.SessionId.asInt())
									{
										status = sp->getStatus();
										break;
									}
								}
								if (i == character->getSessionParticipants().size())
								{
									// check if the character is a GM
									bool allowGm = hasSessionAccessPrivilege(userId);

									if (session->getSessionType() == TSessionType::st_edit)
										status = TSessionPartStatus::sps_edit_invited;
									else
										status = TSessionPartStatus::sps_play_invited;

									if (!allowGm)
									{
										ok = false;
										// session participant not found
										joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 3, "No participation found", TSessionPartStatus::invalid_val);
										goto endOfWelcomeUserResult;
									}
								}
								// update the ring user with the active session
//									CRingUserPtr ru = CRingUser::load(_RingDb, character->getUserId(), __FILE__, __LINE__);
//									BOMB_IF(ru == NULL, "RSM:welcomeUserResult failed to load ring user "<<character->getUserId()<<" from the database", joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 7, "Ring user not found", TSessionPartStatus::invalid_val); goto endOfWelcomeUserResult;);

								character->setCurrentSession(pjs.SessionId.asInt());
								character->update(_RingDb);


//								// clear the 'session rated' flag in the participation
//								if (i != character->getSessionParticipants().size())
//								{
//									CSessionParticipantPtr &sp = character->getSessionParticipantsByIndex(i);
//
//									sp->setSessionRated(false);
//									sp->update(_RingDb);
//								}

								// return info to the web
								if (pjs.OptSecurityCode.getSecurityCode() != NULL)
									joinSessionResultExt(pjs.From, pjs.UserId, pjs.SessionId, 0, shardAddr, status, *(pjs.OptSecurityCode.getSecurityCode()));
								else
									joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 0, shardAddr, status);
							}
						}
						else
						{
							// mainland

							// update the ring user with the active session
							CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
							BOMB_IF(ru == NULL, "RSM:welcomeUserResult failed to load ring user "<<userId<<" from the database", joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 7, "Ring user not found", TSessionPartStatus::invalid_val); ok = false; goto endOfWelcomeUserResult;);
							CCharacterPtr character = CCharacter::load(_RingDb, pjs.CharId, __FILE__, __LINE__);
//							BOMB_IF(character == NULL, "RSM:welcomeUserResult failed to load character "<<pjs.CharId<<" from the database", joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 1, "Character not found", TSessionPartStatus::invalid_val); goto endOfWelcomeUserResult;);

							TWelcomeServices::iterator itw;
							for (itw=_WelcomeServices.begin(); itw!=_WelcomeServices.end(); ++itw)
							{
								if ((*itw).second.WSModuleProxy == sender)
								{
									if (character != NULL)
									{
										character->setCurrentSession((*itw).second.FixedSessionId.asInt());
										character->update(_RingDb);
									}

									// joining a shard => return info to the web
									// TODOliv: which playing status?
									joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 0, shardAddr, TSessionPartStatus(TSessionPartStatus::sps_playing).toString());
									break;
								}
							}
							BOMB_IF(itw == _WelcomeServices.end(), "RSM:welcomeUserResult: WSModule sender not found", joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 11, "Unknown welcome service responded", TSessionPartStatus::invalid_val); goto endOfWelcomeUserResult;);
						}
					}
					else
					{
						// conn refused
						joinSessionResult(pjs.From, pjs.UserId, pjs.SessionId, 8, string("Welcome refused entry in shard with error : '")+errorMsg+"'", TSessionPartStatus::invalid_val);
					}

endOfWelcomeUserResult:
					if (ok)
					{
						// update the 'ring_user' table to signal which session we're entering and with which character
						CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
						ru->setCurrentSession(pjs.SessionId);
						ru->setCurrentCharacter(pjs.CharId);
						ru->update(_RingDb);
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

		// transmits the current player counts
		virtual void updateConnectedPlayerCount(NLNET::IModuleProxy *sender, uint32 nbOnlinePlayers, uint32 nbPendingPlayers)
		{
			H_AUTO(SessionManager_updateConnectedPlayerCount);

			for ( TWelcomeServices::iterator itw=_WelcomeServices.begin(); itw!=_WelcomeServices.end(); ++itw )
			{
				if (itw->second.WSModuleProxy == sender)
				{
					nldebug("RSM : updating connected player count for shard %u", itw->first);
					itw->second.NbOnlinePlayers = nbOnlinePlayers;
					itw->second.NbPendingPlayers = nbPendingPlayers;
					break;
				}
			}
		}

		/////////////////////////////////////////////////////////////
		//// Web callback implementation
		/////////////////////////////////////////////////////////////

		/// Connection callback : a new interface client connect
		virtual void on_CRingSessionManagerWeb_Connection(NLNET::TSockId from)
		{
			nldebug("RSM : connection on web interface from %s", from->getTcpSock()->remoteAddr().asString().c_str());
		}

		virtual void on_CRingSessionManagerWeb_Disconnection(NLNET::TSockId from)
		{
			nldebug("RSM : disconnection on web interface from %s", from->getTcpSock()->remoteAddr().asString().c_str());
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

		virtual void on_setSessionStartParams(NLNET::TSockId from,
			uint32 charId, TSessionId sessionId, const std::string &initialIslandLocation, const std::string &initialEntryPointLocation, const std::string &initialSeason)
		{
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);

			if (session == NULL) { return; }
			if (session->getSessionType() == TSessionType::st_mainland)	 { return;}

			TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
			if (it == _SessionIndex.end()) { return; }

			CRingSessionManagerClientProxy ss(it->second);
			ss.setSessionStartParams(this, charId, sessionId, initialIslandLocation, initialEntryPointLocation, initialSeason);

		}


		virtual void on_scheduleSession(NLNET::TSockId from,
			uint32 charId,
			const TSessionType &sessionType,
			const std::string &sessionTitle,
			const std::string &sessionDesc,
			const TSessionLevel &sessionLevel,
//			const TAccessType &accessType,
			const TRuleType &ruleType,
			const TEstimatedDuration &estimatedDuration,
			uint32 subscriptionSlot,
			const TAnimMode &animMode,
			const TRaceFilter &raceFilter,
			const TReligionFilter &religionFilter,
			const TGuildFilter &guildFilter,
			const TShardFilter &shardFilter,
			const TLevelFilter &levelFilter,
			const std::string &language,
			const TSessionOrientation &orientation,
			bool subscriptionClosed,
			bool autoInvite)
		{
			nldebug("RSM : web schedule a new '%s' session for char %u", sessionType.toString().c_str(), charId);

			uint32 userId = charId >> 4;

			TSessionType st(sessionType);
			if (st == TSessionType::invalid_val)
			{
				// failed !, invalid session type
				scheduleSessionResult(from, charId, TSessionId(0), 1, "Invalid session type");
			}
			TSessionLevel sl(sessionLevel);
			if (sl == TSessionLevel::invalid_val)
			{
				// failed !, invalid level
				scheduleSessionResult(from, charId, TSessionId(0), 2, "Invalid level");
			}
//			TAccessType at(accessType);
//			if (at == TAccessType::invalid_val)
//			{
//				// failed !, invalid access type
//				scheduleSessionResult(from, charId, TSessionId(0), 5, "Invalid access type");
//			}
			TRuleType rt(ruleType);
			if (rt == TRuleType::invalid_val)
			{
				// failed !, invalid rule type
				scheduleSessionResult(from, charId, TSessionId(0), 6, "Invalid rule type");
			}
			TEstimatedDuration ed(estimatedDuration);
			if (ed == TEstimatedDuration::invalid_val)
			{
				// failed !, invalid estimated duration
				scheduleSessionResult(from, charId, TSessionId(0), 7, "Invalid estimated duration");
			}

			// first, check for session already in hands of this character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				// failed !, invalid character
				scheduleSessionResult(from, charId, TSessionId(0), 3, "Invalid character");
				return;
			}

			// for anim session, check that the user is not a free trial or banned
			if (sessionType == TSessionType::st_anim)
			{
				CNelUserPtr nelUser = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
				BOMB_IF(nelUser == NULL, "Failed to load nel user "<<userId<<" from the database", scheduleSessionResult(from, charId, TSessionId(0), 8, "Invalid user"); return;);

				// Commented because a free trial client is allowed to launch some nevrax scenarios
				/*
				if (nelUser->getExtendedPrivilege().find(":TRIAL:") != string::npos)
				{
					// this account is free trial, no anim session allowed
					scheduleSessionResult(from, charId, TSessionId(0), 9, "Forbidden for free trial");
					return;
				}
				*/

				if (nelUser->getExtendedPrivilege().find(":R2_BAN:") != string::npos)
				{
					// this guy is banned from ring anim session
					scheduleSessionResult(from, charId, TSessionId(0), 10, "User is banned from ring animation session");
					return;
				}
			}

			// Now the access type depend of the added privilege of the ring user
			CRingUserPtr ringUser = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			BOMB_IF(ringUser == NULL, "Failed to load ring user "<<userId<<" from the database", scheduleSessionResult(from, charId, TSessionId(0), 8, "Invalid user"); return;);

			TAccessType at= (autoInvite /*|| ringUser->getAddedPrivileges().find(":PIONEER:") != string::npos*/)? TAccessType::at_public: TAccessType::at_private;

			// ok, we can create a session
			CSessionPtr session = CSession::createTransient(__FILE__, __LINE__);
			session->setOwnerId(charId);
			session->setSessionType(st);
			session->setTitle(sessionTitle);
//			session->setPlanDate(plannedDate);
			session->setPlanDate(0);	// not used now
			session->setStartDate(0); // mean not started
			session->setDescription(sessionDesc);
			session->setLevel(sl);
			session->setRuleType(rt);
			session->setAccessType(at);
			session->setState(TSessionState::ss_planned);
			session->setHostShardId(0);
			session->setSubscriptionSlots(subscriptionSlot);
			session->setReservedSlots(0);
			session->setEstimatedDuration(ed);
			session->setFinalDuration(0);
			session->setFolderId(0);
			session->setAnimMode(animMode);
			session->setRaceFilter(raceFilter);
			session->setReligionFilter(religionFilter);
			session->setGuildFilter(guildFilter);
			session->setShardFilter(shardFilter);
			session->setLevelFilter(levelFilter);
			session->setSubscriptionClosed(false);
			session->setLang(language);
			session->setOrientation(orientation);
			session->setSubscriptionClosed(subscriptionClosed);
			session->setNewcomer(character->getNewcomer());

			// store the session
			session->create(_RingDb);

			// ok, return the result to web
			scheduleSessionResult(from, charId, TSessionId(session->getObjectId()), 0, "");
		}

		virtual void on_getSessionInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : getSessionInfo for session %u with char %u", sessionId.asInt(), charId);

			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				// unknown character
				sessionInfoResult(from, 0, TSessionId(0), TRaceFilter(), TReligionFilter(), TGuildFilter(), TShardFilter(), TLevelFilter(), false, false, std::string(), TSessionOrientation(), std::string());
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				// unknown session
				sessionInfoResult(from, 0, TSessionId(0), TRaceFilter(), TReligionFilter(), TGuildFilter(), TShardFilter(), TLevelFilter(), false, false, std::string(), TSessionOrientation(), std::string());
				return;
			}

			// ok, return the result to web
			sessionInfoResult(from, charId, TSessionId(session->getObjectId()),
				session->getRaceFilter(), session->getReligionFilter(), session->getGuildFilter(),
				session->getShardFilter(), session->getLevelFilter(), session->getSubscriptionClosed(),
				(session->getAccessType()==TAccessType::at_public), session->getLang(), session->getOrientation(), session->getDescription());
		}

		// Update the information of a planned or running session
		// Return 'invokeResult' : 0 : ok, session updated
		//                         1 : unknown character
		//                         2 : unknown session
		//                         3 : char don't own the session
		//                         4 : session is closed, no update allowed
		//                         5 : invalid parameter
		virtual void on_updateSessionInfo(NLNET::TSockId from,
			uint32 charId,
			TSessionId sessionId,
			const std::string &sessionTitle,
			uint32 plannedDate,
			const std::string &sessionDesc,
			const TSessionLevel &sessionLevel,
//			const TAccessType &accessType,
			const TEstimatedDuration &estimatedDuration,
			uint32 subscriptionSlot,
			const TRaceFilter &raceFilter,
			const TReligionFilter &religionFilter,
			const TGuildFilter &guildFilter,
			const TShardFilter &shardFilter,
			const TLevelFilter &levelFilter,
			bool subscriptionClosed,
			bool autoInvite,
			const std::string &language,
			const TSessionOrientation &orientation)
		{
			nldebug("RSM : updateSessionInfo for session %u with char %u", sessionId.asInt(), charId);

			// validate parameters
			TSessionLevel level(sessionLevel);
//			TAccessType	access(accessType);
			TEstimatedDuration duration(estimatedDuration);
			if (level == TSessionLevel::invalid_val
//				|| access == TAccessType::invalid_val
				|| duration == TEstimatedDuration::invalid_val)
			{
				// invalid parameters
				invokeResult(from, charId>>4, 5, "Invalid parameters");
				return;
			}

			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				// unknown character
				invokeResult(from, charId>>4, 1, "Character not found");
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				// unknown session
				invokeResult(from, charId>>4, 2, "Session not found");
				return;
			}

			// check owner
			if (session->getOwnerId() != charId)
			{
				// unknown session
				invokeResult(from, charId>>4, 3, "Character don't own the session");
				return;
			}

			// check state
			if (session->getState() == TSessionState::ss_closed)
			{
				// session closed
				invokeResult(from, charId>>4, 4, "The session is closed, no update allowed");
				return;
			}

			// ok, update the session content
			session->setTitle(sessionTitle);
			session->setDescription(sessionDesc);
			session->setLevel(level);
			session->setEstimatedDuration(duration);
			session->setSubscriptionSlots(subscriptionSlot);
			session->setRaceFilter(raceFilter);
			session->setReligionFilter(religionFilter);
			session->setGuildFilter(guildFilter);
			session->setShardFilter(shardFilter);
			session->setLevelFilter(levelFilter);
			session->setSubscriptionClosed(subscriptionClosed);
			session->setAccessType(autoInvite? TAccessType::at_public: TAccessType::at_private);
			session->setLang(language);
			session->setOrientation(orientation);

			// save the changes
			session->update(_RingDb);

			// result ok
			invokeResult(from, charId>>4, 0, "");

		}

		// Cancel a planned session
		// Return 'invokeResult' : 0 : ok, session canceled
		//                         1 : unknown char
		//                         2 : unknown session
		//						   3 : char don't own the session
		//						   4 : session not in planned state
		virtual void on_cancelSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : web cancel planned session %u with char %u", sessionId.asInt(), charId);

			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				// unknown user
				invokeResult(from, charId>>4, 1, "Character not found");
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				// unknown session
				invokeResult(from, charId>>4, 2, "Session not found");
				return;
			}

			if (session->getOwnerId() != charId)
			{
				// not owner !
				invokeResult(from, charId>>4, 3, "Character not owner of session");
				return;
			}

			if (session->getState() != TSessionState::ss_planned
				&& session->getState() != TSessionState::ss_locked)
			{
				// not planned
				invokeResult(from, charId>>4, 4, "Session not in planned state");
				return;
			}

			// remove the session from database
			session->remove(_RingDb);

			// return result ok
			invokeResult(from, charId>>4, 0, "");
		}

		// start a planned session
		// Return 'invokeResult' : 0 : ok, session started
		//                         1 : char not found
		//                         2 : session not found
		//                         3 : session not owned by char
		//                         4 : user already has a runnning session of this type
		//                         5 : database/internal failure
		//                         6 : nel user not found
		virtual void on_startSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : web start session %u with char %u", sessionId.asInt(), charId);

			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, charId>>4, 1, "Character not found");
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, charId>>4, 2, "Session not found");
				return;
			}

			if (session->getOwnerId() != charId)
			{
				invokeResult(from, charId>>4, 3, "Character not owner of session");
				return;
			}

			// check that the user don't run more one session of the same type
			CSString query;
			query << "SELECT COUNT(*) FROM sessions WHERE owner = "<<charId<<" AND state = '"<<TSessionState::toString(TSessionState::ss_open)<<"' AND session_type = '"<<session->getSessionType().toString()<<"'";
			BOMB_IF(!_RingDb.query(query), "Failed to count number of edit session for character "<<charId, invokeResult(from, charId>>4, 5, "Database failure"); return);
			auto_ptr<MSW::CStoreResult> result = _RingDb.storeResult();

			result->fetchRow();
			uint32 nbSession;
			result->getField(0, nbSession);

			DROP_IF(nbSession > 0, "Character "<<charId<<" already has a session of type '"<<session->getSessionType().toString()<<"' running", invokeResult(from, charId>>4, 4, "User already have a running session of this type"); return;);

			// look for the best server
			TSessionServers::iterator best(_SessionServers.end());


			// check if the user has a open edit session, if so, lauch the session on the same shard
			if (session->getSessionType() == TSessionType::st_anim)
			{
				// load the sessions
				character->loadSessions(_RingDb, __FILE__, __LINE__);
				for (uint i=0; i<character->getSessions().size(); ++i)
				{
					const CSessionPtr &s = character->getSessions()[i];
					if (s->getSessionType() == TSessionType::st_edit && s->getState() == TSessionState::ss_open)
					{
						// we got an open anim session, choose the same session server
						TSessionServersIdx::iterator it(_SessionIndex.find(s->getObjectId()));
						if (it != _SessionIndex.end())
							best = _SessionServers.find(it->second);

						break;
					}
				}
			}
			// if the character has an active temporary assoc with a shard, select it,
//			TTemporaryShardAssocs::iterator it(_TemporaryShardAssocs.find(charId));
//			if (it != _TemporaryShardAssocs.end())
//			{
//				best = _SessionServers.find(it->second.DSSProxy);
//				nlassert(best != _SessionServers.end());
//			}
//			else
			if (best == _SessionServers.end())
			{
				// still no server choose, choose the best one
				vector<string>	userAccessPriv;
				// load the nel user for permissions
				CNelUserPtr nelUser = CNelUser::load(_NelDb, character->getUserId(), __FILE__, __LINE__);
				BOMB_IF(nelUser == NULL, "RSM:on_StartSession: failed to load ring user "<<character->getUserId()<<"from the database", invokeResult(from, charId>>4, 6, "Nel user not found"); return);
				BOMB_IF(!loadUserAccessPrivileges(charId >> 4, userAccessPriv), "RSM:on_startSession : failed to load privileges for user "<<(charId >> 4), invokeResult(from, charId>>4, 6, "Nel user not found"); return);

				// otherwise, look for the least loaded one
				TSessionServers::iterator first(_SessionServers.begin()), last(_SessionServers.end());

				for (; first != last; ++first)
				{
					TSessionServerInfo &ssi = first->second;
					if (best == _SessionServers.end() || best->second.NbTotalPlayingChars > ssi.NbTotalPlayingChars)
					{
						// check shard state and user permission matches
						CShardPtr shard = CShard::load(_RingDb, ssi.ShardId, __FILE__, __LINE__);
						if (shard == NULL)
							continue;
						if (shard->getWSOnline() && shard->getRequiredState() != TAccessLevel::ds_close)
						{
							// check the permission of the player
							for (uint i=0; i<userAccessPriv.size(); ++i)
							{
								if (_DontUsePerm || string("ds_")+toLower(userAccessPriv[i]) == shard->getRequiredState().toString())
								{
									// ok, this one is accessible
									// this server is better (i.e had less player in it)
									best = first;

//									// Store the temporary assoc between character and shard
//									TTemporaryShardAssoc tsa;
//									tsa.SessionId = sessionId.asInt();
//									tsa.SessionClosedDate = 0;
//									tsa.DSSProxy = best->first;
//									_TemporaryShardAssocs[charId] = tsa;
//
									// stop iterating over user access
									break;
								}
							}
						}
					}
				}
			}
			if (best == _SessionServers.end())
			{
				// oups! no server to host this session !
				invokeResult(from, charId>>4, 5, "No server to host the session");
				return;
			}

			// Commented: free trial player, are allowed to launch some free trial scenarios. (the servers will refuse to invite people after the scenario has started
			/*
			// for anim session, check that the user is not a free trial
			if (session->getSessionType() == TSessionType::st_anim)
			{
				CNelUserPtr nelUser = CNelUser::load(_NelDb, charId>>4, __FILE__, __LINE__);
				BOMB_IF(nelUser == NULL, "Failed to load nel user "<<(charId>>4)<<" from the database", invokeResult(from, charId>4, 7, "Nel user not found"); return;);

				if (nelUser->getExtendedPrivilege().find(":TRIAL:") != string::npos)
				{
					// this account is free trial, no anim session allowed
					invokeResult(from, charId>>4, 6, "forbidden for free trial account");
					return;
				}
			}
			*/


			IModuleProxy *server = best->first;

			// insert pending session info
			TPendingSessionCreateInfo psci;
			psci.CharId = charId;
			psci.From = from;
			psci.SessionId = sessionId;
			_PendingSessions.push_back(psci);

			// ask the session server to open the session
			CRingSessionManagerClientProxy rsmc(server);
			rsmc.createSession(this, charId, TSessionId(session->getObjectId()), session->getSessionType());

			// the return to web site will be done when DSS respond
		}

		// Close a running session
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : session not found
		//                         2 : char don't own the session
		//						   3 : session not open
		//						   4 : failed to close the session, internal error
		virtual void on_closeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : web close session %u with char %u", sessionId.asInt(), charId);

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, charId>>4, 1, "session not found");
				return;
			}

			if (session->getOwnerId() != charId)
			{
				invokeResult(from, charId>>4, 2, "character not own the session");
				return;
			}

			if (session->getState() == TSessionState::ss_closed
				||session->getState() == TSessionState::ss_planned)
			{
				invokeResult(from, charId>>4, 3, "session not open");
				return;
			}

			// ask the session server to close this session
			TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
			if (it == _SessionIndex.end())
			{
				nlwarning("RSM:closeSession : could not find server for opened session %u, closing the session", sessionId.asInt());

				closeSession(sessionId);
			}
			else
			{
				TModuleProxyPtr proxy = it->second;

				CRingSessionManagerClientProxy ss(proxy);

				ss.closeSession(this, sessionId);
			}

			// ok, all is good
			invokeResult(from, charId>>4, 0, "");
		}


		// Close the current edit session of the character
		// Return 'invokeResult' : 0 : ok, session closed (or about to close)
		//                         1 : char not found
		//                         2 : failed to close the session, internal error
		virtual void on_closeEditSession(NLNET::TSockId from, uint32 charId)
		{
			nldebug("RSM : web close edit session of char %u", charId);

			uint32 userId = charId >> 4;

			// we need to request the database in order to find the edit session
			// of the character
			CSString query;
			query << "SELECT session_id FROM sessions WHERE owner = "<<charId;
			query << " AND session_type = 'st_edit'";
			BOMB_IF(!_RingDb.query(query), "Error in database request", invokeResult(from, userId, 2, "Error in database request"); return);

			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());

			bool sessionClosed = false;
			if (result->getNumRows() == 0)
			{
				// oups, no edit session found !
				invokeResult(from, userId, 4, "No edit session found");
				return;
			}
			// get the row
			result->fetchRow();
			uint32 sessionId;
			result->getField(0, sessionId);

			// ask the DSS to close the session
			CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
			BOMB_IF(session == NULL, "Session object not found", invokeResult(from, userId, 5, "Session object not found"); return;);

			if (session->getState() != TSessionState::ss_closed
				&& session->getState() != TSessionState::ss_planned)
			{
				// ask the session server to close this session
				TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
				if (it != _SessionIndex.end())
				{
					TModuleProxyPtr proxy = it->second;

					CRingSessionManagerClientProxy ss(proxy);

					ss.closeSession(this, sessionId);

					// ok, we have closed the session
					sessionClosed = true;
				}
			}
			else
			{
				// the session is closed or planned, just ask to remove the hibernation file
				if (_SessionServers.empty())
				{
					// oups!
					invokeResult(from, userId, 3, "No session server to close the session");
					return;
				}

				// use the first server in the list
				TModuleProxyPtr proxy = _SessionServers.begin()->first;
				CRingSessionManagerClientProxy ss(proxy);

				ss.stopHibernation(this, TSessionId(sessionId), charId);
			}

			// ok, all is good
			invokeResult(from, userId, 0, "");
		}

		// Close a running session
		// Return 'invokeResult' : 0 : ok,
		//                         1 : not ok
		//:TODO: change the name to Cloase or hibernate previous sessions
		virtual void on_hibernateEditSession(NLNET::TSockId from, uint32 charId)
		{

			nldebug("RSM: client %s ask to hibernate the edit session for char %u",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				charId);

			// 1 : check if the char already have an edit session
			// load the character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, charId>>4, 1, "Character not found");
				return;
			}
			// Hiberning edit sessions
			{

				CSString query;
				query << "SELECT session_id FROM sessions";
				query << " WHERE owner = "<<charId<<" AND session_type ='st_edit'";
				BOMB_IF(!_RingDb.query(query), "on_hibernateEditSession : Failed to request in database", invokeResult(from, charId>>4, 1, "Database error"); return);
				auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());

				// 1.1 : if no session so no need to hibernate (not an error)
				if (result->getNumRows() != 0)
				{


					if (result->getNumRows() > 1)
					{
						nlwarning("RSM : on_hibernateEditSession : character %u has %u edit session in DB, should only have 1",
							charId, result->getNumRows());
					}

					// get the session id
					result->fetchRow();
					uint32 sessionNum;
					result->getField(0, sessionNum);
					TSessionId sessionId(sessionNum);

					CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
					if (session != NULL)
					{
						if (session->getState() != TSessionState::ss_closed
							&& session->getState() != TSessionState::ss_planned)
						{
							// ask the session server to close this session
							TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
							if (it != _SessionIndex.end())
							{
								TModuleProxyPtr proxy = it->second;

								CRingSessionManagerClientProxy ss(proxy);

								ss.hibernateSession(this, sessionId);
							}
						}
					}
				}
			}

			// Close anim sessions
			{

				CSString query;
				query << "SELECT session_id FROM sessions";
				query << " WHERE owner = "<<charId<<" AND session_type ='st_anim'";
				BOMB_IF(!_RingDb.query(query), "on_hibernateEditSession : Failed to request in database", invokeResult(from, charId>>4, 1, "Database error"); return);
				auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());

				// 1.1 : if no session so no need to hibernate (not an error)
				if (result->getNumRows() > 1)
				{
					nlwarning("RSM : on_hibernateEditSession : character %u has %u anim session in DB, should only have 1",
						charId, result->getNumRows());
				}


				unsigned int firstSession = 0;
				unsigned int lastSession = result->getNumRows();


				for (;firstSession != lastSession; ++firstSession)
				{
					result->fetchRow();
					// get the session id

					uint32 sessionNum;
					result->getField(0, sessionNum);
					TSessionId sessionId(sessionNum);

					CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
					if (session != NULL)
					{

						if ( session->getState() != TSessionState::ss_closed && session->getState() != TSessionState::ss_planned)
						{
							// ask the session server to close this session
							TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
							if (it == _SessionIndex.end())
							{
								nlwarning("RSM:closeSession : could not find server for opened session %u, closing the session", sessionId.asInt());

								closeSession(sessionId);
							}
							else
							{
								TModuleProxyPtr proxy = it->second;

								CRingSessionManagerClientProxy ss(proxy);

								ss.closeSession(this, sessionId);
							}

						}
					}
				}
			}
			// ok, all is good
			invokeResult(from, charId>>4, 0, "");
		}

		// Add a character in a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already friend
		//                         4 : internal error
		virtual void on_addFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
		{
			nldebug("RSM : addFriendCharacter add char %u as friend of user %u", friendCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, friendCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 friendUser = friendCharId>>4;

			// check if this character is not already in the list as a banned or friend
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == friendCharId
					&& (ku->getRelation() == TKnownUserRelation::rt_friend || ku->getRelation() == TKnownUserRelation::rt_banned))
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// we need to add this char as friend
				CKnownUserPtr ku = CKnownUser::createTransient(__FILE__, __LINE__);

				ku->setOwnerId(userId);
				ku->setTargetUser(friendUser);
				ku->setTargetCharacter(friendCharId);
				ku->setRelation(TKnownUserRelation::rt_friend);

				// store a new row in the database
				ku->create(_RingDb);
			}
			else
			{
				// the character is already in the list, update it's status
				CKnownUserPtr ku = ru->getKnownUsersByIndex(i);

				if (ku->getRelation() == TKnownUserRelation::rt_friend)
				{
					invokeResult(from, userId, 3, "character already friend");
					return;
				}

				ku->setRelation(TKnownUserRelation::rt_friend);

				// store the change to disk
				ku->update(_RingDb);
			}

			// result ok
			invokeResult(from, userId, 0, "");
		}

		// Remove a character from a user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend
		//						   4 : internal error
		virtual void on_removeFriendCharacter(NLNET::TSockId from, uint32 userId, uint32 friendCharId)
		{
			nldebug("RSM : removeFriendCharacter remove char %u as friend of user %u", friendCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, friendCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 friendUser = friendCharId>>4;
			// check if this character is not already in the list
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == friendCharId && ku->getRelation() == TKnownUserRelation::rt_friend)
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// not found
				invokeResult(from, userId, 3, "character not in friend list");
				return;
			}

			CKnownUserPtr ku = ru->getKnownUsersByIndex(i);

			// ok, remove the know user record
			ku->remove(_RingDb);

			// result ok
			invokeResult(from, userId, 0, "");
		}

		// Add a character to a user ban list. This ban the user that own the character
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char already banned by user
		virtual void on_addBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
		{
			nldebug("RSM : addBannedCharacter add char %u as banned of user %u", bannedCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, bannedCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 bannedUser = bannedCharId>>4;

			// check if this character is not already in the list
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == bannedCharId && ku->getRelation() == TKnownUserRelation::rt_banned)
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// we need to add this char as banned
				CKnownUserPtr ku = CKnownUser::createTransient(__FILE__, __LINE__);

				ku->setOwnerId(userId);
				ku->setTargetUser(bannedUser);
				ku->setTargetCharacter(bannedCharId);
				ku->setRelation(TKnownUserRelation::rt_banned);

				// store a new row in the database
				ku->create(_RingDb);
			}
			else
			{
				// the character is already banned
			}

			vector<CKnownUserPtr>	removeList;
			// remove any friend or DM friend record
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == bannedCharId && ku->getRelation() != TKnownUserRelation::rt_banned)
					removeList.push_back(ku);
			}

			while (!removeList.empty())
			{
				removeList.back()->remove(_RingDb);
				removeList.pop_back();
			}

			// result ok
			invokeResult(from, userId, 0, "");
		}

		// Remove a character from a user ban list.
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : banned char not found
		//                         3 : char not banned by user
		virtual void on_removeBannedCharacter(NLNET::TSockId from, uint32 userId, uint32 bannedCharId)
		{
			nldebug("RSM : removeBannedCharacter remove char %u as friend of user %u", bannedCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, bannedCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 bannedUser = bannedCharId>>4;
			// check if this character is not already in the list
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == bannedCharId && ku->getRelation() == TKnownUserRelation::rt_banned)
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// not found
				invokeResult(from, userId, 3, "character not in ban list");
				return;
			}

			CKnownUserPtr ku = ru->getKnownUsersByIndex(i);
//			if (ku->getRelation() != TKnownUserRelation::rt_banned)
//			{
//				invokeResult(from, userId, 3, "character not flagged as banned");
//				return;
//			}

			// ok, remove the know user record
			ku->remove(_RingDb);

			// result ok
			invokeResult(from, userId, 0, "");
		}
		// Add a character in a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char already DM friend
		void on_addFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId)
		{
			nldebug("RSM : addFriendDMCharacter add char %u as friend DM of user %u", friendDMCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, friendDMCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 friendUser = friendDMCharId>>4;

			// check if this character is not already in the list as friend DM or banned
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == friendDMCharId
					&& (ku->getRelation() == TKnownUserRelation::rt_friend_dm || ku->getRelation() == TKnownUserRelation::rt_banned))
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// we need to add this char as friend
				CKnownUserPtr ku = CKnownUser::createTransient(__FILE__, __LINE__);

				ku->setOwnerId(userId);
				ku->setTargetUser(friendUser);
				ku->setTargetCharacter(friendDMCharId);
				ku->setRelation(TKnownUserRelation::rt_friend_dm);

				// store a new row in the database
				ku->create(_RingDb);
			}
			else
			{
				// the character is already in the list, just update it
				CKnownUserPtr ku = ru->getKnownUsersByIndex(i);

				if (ku->getRelation() == TKnownUserRelation::rt_friend_dm)
				{
					invokeResult(from, userId, 3, "character already friend DM");
					return;
				}

				ku->setRelation(TKnownUserRelation::rt_friend_dm);

				// update the database
				ku->update(_RingDb);
			}

			// result ok
			invokeResult(from, userId, 0, "");
		}

		// Remove a character from a DM user friend list
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : friend char not found
		//                         3 : char not friend
		void on_removeFriendDMCharacter(NLNET::TSockId from, uint32 userId, uint32 friendDMCharId)
		{
			nldebug("RSM : removeFriendDMCharacter remove char %u as friend DM of user %u", friendDMCharId, userId);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, userId, 1, "user not found");
				return;
			}

			// load the known user records
			if (!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__))
			{
				invokeResult(from, userId, 4, "error while loading known user");
				return;
			}

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, friendDMCharId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, userId, 2, "error while loading target character");
				return;
			}

			uint32 friendUser = friendDMCharId>>4;
			// check if this character is not already in the list
			uint i;
			for (i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsers()[i];
				if (ku->getTargetCharacter() == friendDMCharId && ku->getRelation() == TKnownUserRelation::rt_friend_dm)
					break;
			}
			if (i == ru->getKnownUsers().size())
			{
				// not found
				invokeResult(from, userId, 3, "character not in dm friend list");
				return;
			}

			CKnownUserPtr ku = ru->getKnownUsersByIndex(i);
//			if (ku->getRelation() != TKnownUserRelation::rt_friend)
//			{
//				invokeResult(from, userId, 3, "character not flagged as friend");
//				return;
//			}

			// ok, remove the know user record
			ku->remove(_RingDb);

			// result ok
			invokeResult(from, userId, 0, "");
		}

		// Set the comment associated to a known character entry
		// Return 'invokeResult' : 0 : ok
		//                         1 : user not found
		//                         2 : known character entry not found
		//                         3 : character relation don't match the set comments relation
		//                         4 : internal error
		//                         5 : invalid param
		virtual void on_setKnownCharacterComments(NLNET::TSockId from, uint32 userId, uint32 charId, const std::string &relation, const std::string &comments)
		{
			nldebug("RSM : setKnownCharacterComments set comment '%s' on char %u knowns as '%s' of user %u",
				comments.c_str(),
				charId,
				relation.c_str(),
				userId);

			TKnownUserRelation rel(relation);
			BOMB_IF(rel == TKnownUserRelation::invalid_val, "RSM : setKnownCharacterComments : '"<<relation<<"' is an invalid relation", invokeResult(from, userId, 5, "invalid relation"); return);

			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			BOMB_IF(ru == NULL, "RSM : setKnownCharacterComments : failed to load user "<<userId, invokeResult(from, userId, 1, "user not found"); return);

			// load the known user records
			BOMB_IF(!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__), "RSM : setKnownCharacterComments : failed to load known user of user "<<userId, invokeResult(from, userId, 4, "error while loading known user"); return);

			// load the target character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "RSM : setKnownCharacterComments : failed to load character "<<charId, invokeResult(from, userId, 2, "error while loading target character"); return);

			uint32 knownUser = charId>>4;
			// check that this character is already in the list
			bool foundARelation = false;
			for (uint i=0; i<ru->getKnownUsers().size(); ++i)
			{
				CKnownUserPtr &ku = ru->getKnownUsersByIndex(i);
				if (ku->getTargetCharacter() == charId && ku->getRelation() == rel)
				{
					// we found the relation, update the comment
					ku->setComments(comments);
					ku->update(_RingDb);

					// result ok
					invokeResult(from, userId, 0, "");

					return;
				}
				else if (ku->getTargetCharacter() == charId)
				{
					foundARelation = true;
				}
			}

			if (foundARelation)
			{
				// bad relation specified
				invokeResult(from, userId, 3, "invalid relation specified");
			}
			else
			{
				// failed to found the known user record
				invokeResult(from, userId, 2, "known user not found");
			}
		}

		// A user invite a character to help or play in his session
		// charRole is from enum TSessionPartStatus
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : invited char not found
		//                4 : char not own the session
		//                5 : char already invited
		//                6 : char role and session type don't match (edit/editor, anim/animator)
		//                7 : charRole is invalid (must be sps_play_invited, sps_edit_invited or sps_anim_invited)
		//                8 : database failure
		//				 11 : owner char is not animator in the session
		//				 12 : newcomer flag missmatch
		//				 13 : scenarion not started, can't validate invitation now
		virtual void on_inviteCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 invitedCharId, const TSessionPartStatus  &charRole)
		{
			nldebug("RSM : web invite character %u as %s in session %u with char %u",
				invitedCharId, charRole.toString().c_str(), sessionId.asInt(), ownerCharId);

			TSessionPartStatus	role(charRole);
			if (role == TSessionPartStatus::invalid_val
				|| (	role != TSessionPartStatus::sps_play_invited
					&& role != TSessionPartStatus::sps_edit_invited
					&& role != TSessionPartStatus::sps_anim_invited
					)
				)
			{
				invokeResult(from, ownerCharId>>4, 7, "Invalid participation status");
				return;
			}

			CCharacterPtr ownerChar= CCharacter::load(_RingDb, ownerCharId, __FILE__, __LINE__);
			if (ownerChar == NULL)
			{
				invokeResult(from, ownerCharId>>4, 1, "Owner char not found");
				return;
			}

			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, ownerCharId>>4, 2, "Session not found");
				return;
			}


			CCharacterPtr invitedCharacter = CCharacter::load(_RingDb, invitedCharId, __FILE__, __LINE__);
			if (invitedCharacter == NULL)
			{
				invokeResult(from, ownerCharId>>4, 3, "Invited character not found");
				return;
			}

			// check the newcomer flag match
			if (ownerChar->getNewcomer() != invitedCharacter->getNewcomer())
			{
				invokeResult(from, ownerCharId>>4, 12, "Newcomer flag mismatch");
				return;
			}

			// if the invited character is not the session owner
			if (invitedCharId != ownerCharId)
			{
				// check that free trial account can't access non authorised scenarios
				CNelUserPtr invitedNelUser = CNelUser::load(_NelDb, invitedCharId>>4, __FILE__, __LINE__);
				BOMB_IF(invitedNelUser == NULL, "Failed to load nel user "<<(ownerCharId>>4), invokeResult(from, ownerCharId>>4, 8, "Can not load Nel user of invited character"); return);
				CSessionLogPtr sessionLog = CSessionLog::load(_RingDb, sessionId, __FILE__, __LINE__);
				if (sessionLog == NULL)
				{
					// the scenario is not started, can't accept invitation now
					invokeResult(from, ownerCharId>>4, 13, "scenario not started, can't validate invitation now");
					return;
				}

				CScenarioPtr scenario = CScenario::load(_RingDb, sessionLog->getScenarioId(), __FILE__, __LINE__);
				BOMB_IF(invitedNelUser == NULL, "Failed to load scenario for id "<<sessionLog->getScenarioId()<<" for session "<<sessionId.asInt(), invokeResult(from, ownerCharId>>4, 8, "Can not load scenario associated with session log"); return);

				if (invitedNelUser->getExtendedPrivilege().find(":TRIAL:") != string::npos
					&& !scenario->getAllowFreeTrial())
				{
					// free trial are not allowed in this scenario
					invokeResult(from, ownerCharId>>4, 14, "free trial character are not allowed in user scenario");
					return;

				}

				// check that the character is not already participating
				invitedCharacter->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
				for (uint i=0; i<invitedCharacter->getSessionParticipants().size(); ++i)
				{
					if (invitedCharacter->getSessionParticipants()[i]->getSessionId() == sessionId.asInt())
					{
						invokeResult(from, ownerCharId>>4, 5, "Participation already exist in session");
						return;
					}
				}


				if (session->getSessionType() == TSessionType::st_edit)
				{
					invokeResult(from, ownerCharId>>4, 7, "It is not allowed to invite someone else in edit session");
					return;
				}
				else if (role != TSessionPartStatus::sps_play_invited)
				{
					invokeResult(from, ownerCharId>>4, 4, "Animator can't invite other animator (only session owner can)");
					return;
				}
				else if (session->getOwnerId() != ownerCharId)
				{
					// the character that request the invitation is not the session owner,
					// check that he is animator in the session/

					BOMB_IF(!ownerChar->loadSessionParticipants(_RingDb, __FILE__, __LINE__), "Failed to load participation for host character "<<ownerCharId, invokeResult(from, ownerCharId>>4, 8, "Failed to load participation for host character"); return);

					uint i=0;
					for (; i<ownerChar->getSessionParticipants().size(); ++i)
					{
						CSessionParticipantPtr sp = ownerChar->getSessionParticipantsByIndex(i);

						if (sp->getSessionId() == sessionId.asInt())
						{
							// we found it
							if (sp->getStatus() != TSessionPartStatus::sps_anim_invited
								&& sp->getStatus() != TSessionPartStatus::sps_animating)
							{
								// the host char is not animator in the session !
								invokeResult(from, ownerCharId>>4, 11, "Invitation requester char is not animator in the session");
								return;
							}

							// ok, stop the loop
							break;
						}
					}

					if (i == ownerChar->getSessionParticipants().size())
					{
						// no participation found !
						invokeResult(from, ownerCharId>>4, 11, "Requester character is not invited in session");
						return;
					}
				}
			}

			// TODO : check for role and session type coherency

//			// check that character is the pioneer character in case of DM'ing
//			if (role == TSessionPartStatus::sps_edit_invited || role == TSessionPartStatus::sps_anim_invited)
//			{
//				// load the invited user
//				CRingUserPtr invitedUser = CRingUser::load(_RingDb, invitedCharacter->getUserId(), __FILE__, __LINE__);
//				if (invitedUser == NULL)
//				{
//					invokeResult(from, ownerCharId>>4, 8, "Failed to load ring user of the invited character");
//					return;
//				}
//
//				if (invitedUser->getPioneerCharId() != invitedCharId)
//				{
//					invokeResult(from, ownerCharId>>4, 9, "The character invited for DM'ing is not a pioneer character");
//					return;
//				}
//			}

			// ok, all is fine, we can create in invitation entry for the character
			CSessionParticipantPtr ss = CSessionParticipant::createTransient(__FILE__, __LINE__);

			ss->setCharId(invitedCharId);
			ss->setSessionId(sessionId.asInt());
			ss->setStatus(role);

			if (!ss->create(_RingDb))
			{
				invokeResult(from, ownerCharId>>4, 8, "Failed to create the participation entry");
				return;
			}

			// finally, all was good, return 0
			invokeResult(from, ownerCharId>>4, 0, "");
		}

		// A user remove an invitation in a session
		// invokeReturn : 0 : ok, character invited
		//                1 : char not found
		//                2 : session not found
		//                3 : character already entered in session
		//                4 : invitation not found
		//                5 : session not owned by user
		virtual void on_removeInvitedCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 removedCharId)
		{
			nldebug("RSM : web revoke invitation for character %u in session %u with char %u",
				removedCharId, sessionId.asInt(), ownerCharId);

			CCharacterPtr ownerChar = CCharacter::load(_RingDb, ownerCharId, __FILE__, __LINE__);
			if (ownerChar == NULL)
			{
				invokeResult(from, ownerCharId>>4, 1, "User not found");
				return;
			}

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, ownerCharId>>4, 2, "Session not found");
				return;
			}

			// check the session owner
			if (session->getOwnerId() != ownerCharId)
			{
				invokeResult(from, ownerCharId>>4, 5, "Owner character is not owner of the session");
				return;
			}

			// load the participation for the session
			session->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
			uint i;
			for (i=0; i<session->getSessionParticipants().size(); ++i)
			{
				CSessionParticipantPtr sp = session->getSessionParticipantsByIndex(i);

				if (sp->getCharId() == removedCharId)
				{
					// ok, we have found it !
					break;
				}
			}
			if (i == session->getSessionParticipants().size())
			{
				invokeResult(from, ownerCharId>>4, 4, "Invitation not found");
				return;
			}

			CSessionParticipantPtr sp = session->getSessionParticipantsByIndex(i);

			// check that character not already entered
			if (!(sp->getStatus() < TSessionPartStatus::sps_playing))
			{
				invokeResult(from,ownerCharId>>4,  3, "Character already entered in the session");
				return;
			}

			// remove the invitation
			sp->remove(_RingDb);

			// result ok
			invokeResult(from, ownerCharId>>4, 0, "");
		}

		// A character subscribe to a public animation session
		// invokeReturn : 0 : ok, subscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character already subscribed to or invited in the session
		//                4 : session not public
		//                5 : character banned
		//				  6 : no place left, session is full
		//				  7 : session owner not found
		//				  8 : internal error
		void on_subscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : web ask char %u to subscribe in the session %u", charId, sessionId.asInt());

			// load the character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "RSM:on_subscribeSession : failed to load character "<<charId<<" from database", invokeResult(from, charId>>4, 1, "character not found"); return);

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "RSM:on_subscribeSession : failed to load session "<<sessionId<<" from database", invokeResult(from, charId>>4, 2, "session not found"); return);

			// check for free slot before more time consuming processing
			if (session->getReservedSlots() >= session->getSubscriptionSlots() )
			{
				invokeResult(from, charId>>4, 6, "no place left in the session");
				return;
			}

			// load the session owner and it's known users sub items
			CRingUserPtr ru = CRingUser::load(_RingDb, session->getOwnerId()>>4, __FILE__, __LINE__);
			BOMB_IF(ru == NULL, "RSM:on_subscribeSession : failed to load user "<<session->getOwnerId()<<" owner of the session", invokeResult(from, charId>>4, 7, "session owner not found"); return);
			BOMB_IF(!ru->loadKnownUsers(_RingDb, __FILE__, __LINE__), "RSM:on_subscribeSession : failed to load known users of user "<<ru->getObjectId(), invokeResult(from, charId>>4, 8, "failed to load known user of session owner"); return);

			// ok, look in the known users if the character is banned
			for (uint i=0; i<ru->getKnownUsers().size(); ++i)
			{
				const CKnownUserPtr &ku = ru->getKnownUsersByIndex(i);

				if (ku->getRelation() == TKnownUserRelation::rt_banned
					&& ku->getTargetUser() == character->getUserId())
				{
					// this character is banned !
					invokeResult(from, charId>>4, 5, "character banned");
					return;
				}
			}

			// ok, the character is allowed, create a session participants
			CSessionParticipantPtr part = CSessionParticipant::createTransient(__FILE__, __LINE__);

			part->setSessionId(sessionId.asInt());
			part->setCharId(charId);
			part->setStatus(TSessionPartStatus::sps_play_subscribed);

			if (part->create(_RingDb))
			{
				// update the free slots counter
				session->setReservedSlots(session->getReservedSlots()+1);
				session->update(_RingDb);

				// return ok
				invokeResult(from, charId>>4, 0, "");
			}
			else
			{
				nlwarning("RSM:on_subscribeSession : failed to create a participation record in database for char %u in session %u", charId, sessionId.asInt());

				invokeResult(from, charId>>4, 8, "Failed to create a participation record in database");
			}
		}

		// A character unsubscribe to a public animation session
		// The character must not join the session in order to unsubscribe
		// invokeReturn : 0 : ok, unsubscription accepted
		//                1 : char not found
		//                2 : session not found
		//                3 : character has not subscribed in the session
		//                4 : internal error
		//				 17 : Newcomer flag missmatch
		void on_unsubscribeSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId)
		{
			nldebug("RSM : web ask to remove subscription for char %u in session %u", charId, sessionId.asInt());

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "RSM:on_unsubscribeSession : failed to load session "<<sessionId<<" from the database", invokeResult(from, charId>>4, 2, "session not found"); return);

			// load the session participants
			BOMB_IF(!session->loadSessionParticipants(_RingDb, __FILE__, __LINE__), "RSM:on_unsubscribeSession : failed to load participations for session "<<sessionId, invokeResult(from, charId>>4, 4, "failed to load session participants"); return);

			// iterate over the participation, looking for the character
			for (uint i=0; i<session->getSessionParticipants().size(); ++i)
			{
				const CSessionParticipantPtr &part = session->getSessionParticipantsByIndex(i);
				BOMB_IF(part == NULL, "RSM:on_unsubsribeSession : error accessing participants at index "<<i<<", part is NULL", invokeResult(from, charId>>4, 4, "failed to accesss participants record"); return)

				if (part->getCharId() == charId
					&& part->getStatus() == TSessionPartStatus::sps_play_subscribed)
				{
					// ok, we found it, we can erase it
					if (CSessionParticipant::removeById(_RingDb, part->getObjectId()))
					{
						// the record has been removed, update the reserved slot counter
						if (session->getReservedSlots() > 0)
						{
							session->setReservedSlots(session->getReservedSlots()-1);
							session->update(_RingDb);
						}
						// result ok
						invokeResult(from, charId>>4, 0, "");
						return;
					}
					else
					{
						invokeResult(from, charId>>4, 4, "Failed to remove the subscription from database");

					}
				}
			}

			// if we are there, then the subscription has not been found
			invokeResult(from, charId>>4, 3, "the character subscription has not been found in the session");
		}

		// A character ask to join a session.
		virtual void on_joinSession(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &clientApplication)
		{
			_joinSessionCommon(from, charId, sessionId, clientApplication,false);
		}

		void _joinSessionCommon(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &clientApplication,bool isEditSessionRequested)
		{
			nldebug("RSM : web asks char %u to join the session %u", charId, sessionId.asInt());

			// load the user
			uint32 userId = charId >> 4;
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				joinSessionResult(from, charId>>4, sessionId, 7, "User not found", TSessionPartStatus::invalid_val);
				return;
			}

			// load access privilege for the user
			vector<string> userAccessPriv;
			if (!_DontUsePerm)
			{
				BOMB_IF (!loadUserAccessPrivileges(userId, userAccessPriv), "RSM:on_joinSession : failed to load privileges for user "<<userId, joinSessionResult(from, userId, sessionId, 12, "failed to request for access permission", TSessionPartStatus::invalid_val); return;);
			}

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				joinSessionResult(from, charId>>4, sessionId, 2, "Session not found", TSessionPartStatus::invalid_val);
				return;
			}

			// Make sure that we don't enter an edit sessino when we wanted to go to an anim / mainland and vice versa
			if ( (session->getSessionType()==TSessionType::st_edit) != isEditSessionRequested )
			{
				STOP("We have a BUG - character trying to go to a session of the wrong type "<<charId);
				joinSessionResult(from, charId>>4, sessionId, 2, "Session type error", TSessionPartStatus::invalid_val);
				return;
			}

			bool mustGrantFastDisconnection = false;

			// For a mainland shard, there is no animation module (aka session server or DSS)
			WS::CWelcomeServiceProxy *wspSwitch = NULL;
			uint32 storedSessionId = sessionId.asInt();
			WS::TUserRole enterAs = WS::TUserRole::invalid_val;
			uint32 instanceId = INVALID_AI_INSTANCE;
			if (session->getSessionType() != TSessionType::st_mainland)
			{
				// This is a Ring session

				// load the character
				CCharacterPtr charac = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
				if (charac == NULL)
				{
					joinSessionResult(from, charId >> 4, sessionId, 1, "Character not found", TSessionPartStatus::invalid_val);
					return;
				}

				// check free trial access or banned character for anim session
				if (session->getSessionType() == TSessionType::st_anim)
				{
					// check newcomer flag
					if (charac->getNewcomer() != session->getNewcomer())
					{
						if (!hasSessionAccessPrivilege(userId))
						{
							nlwarning("Character %u try to join session %u, but newcomer flags differ", charId, sessionId.asInt());
							joinSessionResult(from, charId>>4, sessionId, 17, "Newcomer flag missmatch", TSessionPartStatus::invalid_val);
							return;
						}
					}

					CNelUserPtr nelUser = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
					BOMB_IF(nelUser == NULL, "Failed to load nel user "<<userId, joinSessionResult(from, charId>>4, sessionId, 7, "Nel user not found", TSessionPartStatus::invalid_val); return;);
					if (nelUser->getExtendedPrivilege().find(":R2_BAN:") != string::npos)
					{
						// this guy is banned from ring anim session
						joinSessionResult(from, charId>>4, sessionId, 16, "User is banned from ring", TSessionPartStatus::invalid_val);
						return;
					}

					if (session->getOwnerId() != charId
						&& nelUser->getExtendedPrivilege().find(":TRIAL:") != string::npos)
					{
						// check that the scenario launched allow free trials players
						CSessionLogPtr sessionLog = CSessionLog::load(_RingDb, sessionId, __FILE__, __LINE__);
						if (sessionLog == NULL)
						{
							joinSessionResult(from, charId>>4, sessionId, 18, "Can't find session log to validate sesison access", TSessionPartStatus::invalid_val);
							return;
						}
						CScenarioPtr scenario = CScenario::load(_RingDb, sessionLog->getScenarioId(), __FILE__, __LINE__);
						if (scenario == NULL)
						{
							joinSessionResult(from, charId>>4, sessionId, 19, "Can't find scenario info to validate session access", TSessionPartStatus::invalid_val);
							return;
						}
						if (!scenario->getAllowFreeTrial())
						{
							// this guy is FREE TRIAL and the scenario didn't allow free trial players
							joinSessionResult(from, charId>>4, sessionId, 20, "Scenario is not allowed to free trial players", TSessionPartStatus::invalid_val);
							return;
						}
					}
				}

				// load the session participant from the character
				charac->loadSessionParticipants(_RingDb, __FILE__, __LINE__);

				CSessionParticipantPtr sessionPart;
				// look for the entry pertaining to the session
				for (uint i=0; i<charac->getSessionParticipants().size(); ++i)
				{
					if (charac->getSessionParticipants()[i]->getSessionId() == sessionId.asInt())
					{
						// we found it
						sessionPart.assign(charac->getSessionParticipants()[i], __FILE__, __LINE__);
						break;
					}
				}
				if (sessionPart == NULL)
				{
					// if the session is public, automatically create an invitation as player
					if (session->getAccessType() == TAccessType::at_public)
					{
						sessionPart = CSessionParticipant::createTransient(__FILE__, __LINE__);
						sessionPart->setCharId(charId);
						sessionPart->setSessionId(session->getObjectId());
						sessionPart->setStatus(TSessionPartStatus::sps_play_invited);

						// store the participation
						sessionPart->create(_RingDb);
					}
					else
					{
						// check if the user is a GM and have right to access the session
						bool allowGm = hasSessionAccessPrivilege(userId);

						if (!allowGm)
						{
							// no participation and not a GM
							joinSessionResult(from, charId>>4, sessionId, 3, "Participation not found", TSessionPartStatus::invalid_val);
							return;
						}
					}
				}

				// retrieve the session server
				TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
				if (it == _SessionIndex.end())
				{
					// can't find server for this session !
					joinSessionResult(from, charId>>4, sessionId, 4, "No server hosting the session", TSessionPartStatus::invalid_val);
					return;
				}

				CRingSessionManagerClientProxy ss(it->second);
				TSessionServerInfo &ssi = _SessionServers[it->second];

				// retrieve the welcome service
				TWelcomeServices::iterator it3(_WelcomeServices.find(ssi.ShardId));
				if (it3 == _WelcomeServices.end())
				{
					// can't find a welcome service for this shard
					joinSessionResult(from, charId>>4, sessionId, 5, "No welcome service to connect to shard hosting the session", TSessionPartStatus::invalid_val);
					return;
				}

				TWelcomeServiceInfo &wsi = it3->second;

				// match the access priv with ws open state
				if (!wsi.ShardInfo->getWSOnline() || (!_DontUsePerm && !checkAccessRight(userAccessPriv, wsi.ShardInfo->getRequiredState())))
				{
					// can't find a welcome service for this shard
					joinSessionResult(from, charId >>4, sessionId, 14, string("Welcome service closed : ")+wsi.ShardInfo->getMOTD(), TSessionPartStatus::invalid_val);
					return;
				}

				wspSwitch = new WS::CWelcomeServiceProxy( it3->second.WSModuleProxy );

				// retrieve the session info
				TSessionServerInfo::THostedSessions::iterator it2(ssi.HostedSessions.find(sessionId));
				nlassert(it2 != ssi.HostedSessions.end());

				TRunningSessionInfo &rsi = it2->second;
				instanceId = rsi.getInstanceId();

				if (sessionPart != NULL)
				{
					// normal user with participation
					if (	sessionPart->getStatus() == TSessionPartStatus::sps_edit_invited
						||	sessionPart->getStatus() == TSessionPartStatus::sps_editing)
					{
						// warn the session server that an editor come in
						enterAs = WS::TUserRole::ur_editor;
						ss.addCharacterInSession(this, sessionId, charId, enterAs, charac->getRingAccess(), charac->getNewcomer());


						// update the participant status
						sessionPart->setStatus(TSessionPartStatus::sps_editing);
					}
					else if (	sessionPart->getStatus() == TSessionPartStatus::sps_anim_invited
						||	sessionPart->getStatus() == TSessionPartStatus::sps_animating)
					{
						// warn the session server that an animator come in
						enterAs = WS::TUserRole::ur_animator;
						ss.addCharacterInSession(this, sessionId, charId, enterAs, charac->getRingAccess(), charac->getNewcomer());
						// update the participant status
						sessionPart->setStatus(TSessionPartStatus::sps_animating);
						mustGrantFastDisconnection = true;
					}
					else
					{
						enterAs = WS::TUserRole::ur_player;
						ss.addCharacterInSession(this, sessionId, charId, enterAs, charac->getRingAccess(), charac->getNewcomer());
						// update the participant status
						sessionPart->setStatus(TSessionPartStatus::sps_playing);
						mustGrantFastDisconnection = true;
					}
					// save the participant status
					sessionPart->update(_RingDb);
				}
				else
				{
					// this is for a GM without participation
					if (session->getSessionType() == TSessionType::st_edit)
						enterAs = WS::TUserRole::ur_editor;
					else
						enterAs = WS::TUserRole::ur_player;
					// allow connection on the DSS
					ss.addCharacterInSession(this, sessionId, charId, enterAs, charac->getRingAccess(), charac->getNewcomer());
					mustGrantFastDisconnection = true;
				}
			}
			else
			{
				// join a mainland session
				// retrieve the welcome service (TODOliv: optim)
				for (TWelcomeServices::iterator itws=_WelcomeServices.begin(); itws!=_WelcomeServices.end(); ++itws)
				{
					TWelcomeServiceInfo &wsi = itws->second;
					if (wsi.FixedSessionId == sessionId) // found the mainland WS
					{
						// match the access priv with ws open state
						if (!wsi.ShardInfo->getWSOnline() || !checkAccessRight(userAccessPriv, wsi.ShardInfo->getRequiredState()))
						{
							// can't find a welcome service for this shard
							joinSessionResult(from, charId >>4, sessionId, 14, string("Welcome service closed : ")+wsi.ShardInfo->getMOTD(), TSessionPartStatus::invalid_val);
							return;
						}

						wspSwitch = new WS::CWelcomeServiceProxy( wsi.WSModuleProxy );
						break;
					}
				}
				if (wspSwitch == NULL)
				{
					// can't find a welcome service for this shard

					// try to read a shard status in the database
					CShardPtr shard = CShard::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
					if (shard != NULL)
					{
						joinSessionResult(from, charId>>4, sessionId, 5, string("Shard closed : ")+shard->getMOTD(), TSessionPartStatus::invalid_val);
					}
					else
					{
						joinSessionResult(from, charId>>4, sessionId, 5, "No welcome service to connect to the selected shard", TSessionPartStatus::invalid_val);
					}

					return;
				}
				storedSessionId = 0;
				enterAs = WS::TUserRole::ur_player;
			}

			WS::CWelcomeServiceProxy wsp( *wspSwitch );
			delete wspSwitch;

			// load Nel user information
			CNelUserPtr nelUser = CNelUser::load(_NelDb, userId, __FILE__, __LINE__);
			if (nelUser == NULL)
			{
				joinSessionResult(from, charId>>4, sessionId, 6, "Billing account not found", TSessionPartStatus::invalid_val);
				return;
			}

			CLoginCookie cookie;
			cookie.setFromString(ru->getCookie());

			/// store info for waiting WS response (before calling WS in case of collocation)
			TPendingJoinSession pjs;
			pjs.UserId = userId;
			pjs.CharId = charId;
			pjs.From = from;
			pjs.SessionId = TSessionId(storedSessionId);

			if (mustGrantFastDisconnection)
			{
				CSecurityCheckForFastDisconnection securityCheck;
				securityCheck.setSessionId(sessionId);
				securityCheck.setCookie(cookie); // must not be changed by joinSession(), because the client sends the one he knows before Far TPing
				pjs.OptSecurityCode.setSecurityCode(securityCheck.encode(""));
			}
			_PendingJoins.push_back(pjs);

			// reserve an entry place on the welcome service of the shard and wait for the response
			wsp.welcomeUser(this,
				charId,
				nelUser->getLoginName(),
				cookie,
				nelUser->getPrivilege(),
				nelUser->getExtendedPrivilege(),
				enterAs,
				instanceId);

			// the rest of the code is done in 'welcomeUserResult' returned by the WS.
		}

		// A character asks to join a shard.
		// The actual shard will be chosen according to current load.
		virtual void on_joinMainland(NLNET::TSockId from, uint32 charId, const std::string &clientApplication)
		{
			nldebug("RSM : web asks char %u to join mainland", charId);

			uint32 userId = charId >> 4;
			// load access privilege for the user
			vector<string> userAccessPriv;
			BOMB_IF (!loadUserAccessPrivileges(userId, userAccessPriv), "RSM:on_joinMainland : failed to load privileges for user "<<userId, joinSessionResult(from, userId, TSessionId(0), 12, "failed to request for access permission", TSessionPartStatus::invalid_val); return;);

			multimap<uint32, TSessionId> mainlandShardsByAscLoad;
			for ( TWelcomeServices::iterator itw=_WelcomeServices.begin(); itw!=_WelcomeServices.end(); ++itw )
			{
				TWelcomeServiceInfo &wsi = itw->second;
				if (wsi.FixedSessionId.asInt() != 0)
				{
					// Include only shards for which there is a known mainland session in the database
					CSessionPtr session = CSession::load(_RingDb, wsi.FixedSessionId.asInt(), __FILE__, __LINE__);
					if ((session != NULL) && (session->getSessionType() == TSessionType::st_mainland))
					{
						// this welcome is a candidate for connection

						// check open state agains user access
						if (wsi.ShardInfo->getWSOnline() && checkAccessRight(userAccessPriv, wsi.ShardInfo->getRequiredState()))
						{
							if ( ForceWelcomerShard.get() != 0 )
							{
								// If ForceWelcomeShard set and found online, bypass remaining shards and select this one
								if ( wsi.FixedSessionId.asInt() == ForceWelcomerShard.get() )
								{
									nlinfo( "Selecting mainland %u (ForceWelcomerShard)", ForceWelcomerShard.get() );
									_joinSessionCommon(from, charId, wsi.FixedSessionId, clientApplication, false );
									return;
								}
							}

							// ok, the user is granted access
							mainlandShardsByAscLoad.insert(make_pair(wsi.NbOnlinePlayers+wsi.NbPendingPlayers, wsi.FixedSessionId));
						}
					}
					else
					{
						nlwarning( "Ignoring mainland session %u (not registered in db)", wsi.FixedSessionId.asInt() );
					}
				}
			}
			if (mainlandShardsByAscLoad.empty())
			{
				joinSessionResult(from, charId>>4, TSessionId(0), 10, "No mainland shard available", TSessionPartStatus::invalid_val);
				return;
			}
			_joinSessionCommon(from, charId, (*mainlandShardsByAscLoad.begin()).second, clientApplication, false);
		}

		// Ask to join the edit session for the specified character.
		// If the edit session do not exist, then the SBS
		// create it before sending the join session request to
		// the SU.
		// Return joinSessionResult forwarded from the SU.
		virtual void on_joinEditSession(NLNET::TSockId from, uint32 charId, const std::string &clientApplication)
		{
			nldebug("RSM: client %s ask to join the edit session for char %u",
				from->getTcpSock()->remoteAddr().asString().c_str(),
				charId);

			// 1 : check if the char already have an edit session
			// load the character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				joinSessionResult(from, charId>>4, TSessionId(0), 1, "Character not found", TSessionPartStatus::invalid_val);
				return;
			}
			// look in the session for an edit session for this character
			CSString query;
			query << "SELECT session_id FROM sessions";
			query << " WHERE owner = "<<charId<<" AND session_type ='st_edit'";
			BOMB_IF(!_RingDb.query(query), "on_joinEditSession : Failed to request in database", joinSessionResult(from, charId>>4, TSessionId(0), 11, "Database error", TSessionPartStatus::invalid_val); return);
			auto_ptr<CStoreResult> result = auto_ptr<CStoreResult>(_RingDb.storeResult());

			// 1.1 : if no session found, return an error to the client
			if (result->getNumRows() == 0)
			{
				joinSessionResult(from, charId>>4, TSessionId(0), 2, "Session not found", TSessionPartStatus::invalid_val);
				return;
			}

			if (result->getNumRows() > 1)
			{
				nlwarning("RSM : on_joinEditSession : character %u has %u edit session in DB, should only have 1",
					charId, result->getNumRows());
			}

			// get the session id
			result->fetchRow();
			uint32 sessionNum;
			result->getField(0, sessionNum);
			TSessionId sessionId(sessionNum);

			// 2 : check that the character is invited in his session
			BOMB_IF(!character->loadSessionParticipants(_RingDb, __FILE__, __LINE__), "on_joinEditSession : failed to load session participants for char "<<charId, joinSessionResult(from, charId>>4, TSessionId(0), 3, "Error loading partipation", TSessionPartStatus::invalid_val); return);
			uint i;
			for (i=0; i<character->getSessionParticipants().size(); ++i)
			{
				if (character->getSessionParticipantsByIndex(i)->getSessionId() == sessionId.asInt())
				{
					break;
				}
			}
			// 2.1 : create the invitation if needed
			if (i == character->getSessionParticipants().size())
			{
				CSessionParticipantPtr part = CSessionParticipant::createTransient(__FILE__, __LINE__);
				part->setCharId(charId);
				part->setKicked(false);
				part->setSessionId(sessionId.asInt());
//				part->setSessionRated(false);
				part->setStatus(TSessionPartStatus::sps_edit_invited);

				// store the participation
				part->create(_RingDb);
			}

			// if the session is closed, just change it to planned, and return an error to the client
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			nlassert(session != NULL);
			if (session->getState() != TSessionState::ss_open)
			{
				if (session->getState() == TSessionState::ss_locked)
				{
					// remove the session from the temporary locked one's
					_TemporaryLockedSession.erase(session->getObjectId());
				}

				session->setState(TSessionState::ss_planned);
				session->update(_RingDb);

				joinSessionResult(from, charId>>4, sessionId, 15, "Session is not open", TSessionPartStatus::invalid_val);
				return;
			}

			// ok, now call the normal join session
			_joinSessionCommon(from, charId, sessionId, clientApplication, true);
		}

		// Request to have the list of accessible shards with their attributes.
		virtual void on_getShards(NLNET::TSockId from, uint32 charId)
		{
			nldebug("RSM : web requests shard list for char %u", charId);

			// list all the online welcome services of the current domain
			string result;
			for ( TWelcomeServices::iterator itw=_WelcomeServices.begin(); itw!=_WelcomeServices.end(); ++itw )
			{
				// Old one: "ShardId NbOnlinePlayers NbPendingPlayers"
				//result += toString( "%u %u %u;", (*itw).first, (*itw).second.NbOnlinePlayers, (*itw).second.NbPendingPlayers );

				// New one: "SessionId NbOnlinePlayers NbPendingPlayers"
				if ((*itw).second.FixedSessionId.asInt() != 0)
				{
					result += toString( "%u,Mainland %u %u;", (*itw).second.FixedSessionId.asInt(), (*itw).second.NbOnlinePlayers, (*itw).second.NbPendingPlayers );
				}
				else
				{
					for (TSessionServers::const_iterator iss=_SessionServers.begin(); iss!=_SessionServers.end(); ++iss)
					{
						if ((*itw).first == (*iss).second.ShardId)
						{
							const TSessionServerInfo::THostedSessions& hostedSessions = (*iss).second.HostedSessions;
							for (TSessionServerInfo::THostedSessions::const_iterator ihs=hostedSessions.begin(); ihs!=hostedSessions.end(); ++ihs)
							{
								result += toString( "%u,Ring %u %u;", (*ihs).first.asInt(), (*itw).second.NbOnlinePlayers, (*itw).second.NbPendingPlayers );
							}
							break;
						}
					}
				}

			}
			getShardsResult(from, charId>>4, result);
		}

		// Kick a character from a session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : character has no participation in the session
		//                4 : internal error
		//				  5 : owner char don't own the session
		void on_kickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 charId)
		{
			nldebug("RSM : on_kickCharacter : web ask to kick character %u from session %u",
				charId,
				sessionId.asInt());

			uint32 userId = ownerCharId >> 4;
			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "RSM : on_kickCharacter : failed to load session "<<sessionId, invokeResult(from, ownerCharId>>4, 2, "Failed to load session"); return);
			if (session->getOwnerId() != ownerCharId)
			{
				// check if the char has the required rights
				if (!hasSessionAccessPrivilege(userId))
				{
					STOP("RSM : on_kickCharacter : char "<<ownerCharId<<" don't own the session "<<sessionId);
					invokeResult(from, ownerCharId>>4, 5, "Owner char don't own the session");
					return;
				}
			}

			// load the participants
			BOMB_IF(!session->loadSessionParticipants(_RingDb, __FILE__, __LINE__), "RSM : on_kickCharacter : failed to load session participants for session "<<sessionId, invokeResult(from, ownerCharId>>4, 4, "Failed to load session participants"); return);

			// look for the participation of the character
			for (uint i=0; i<session->getSessionParticipants().size(); ++i)
			{
				if (session->getSessionParticipantsByIndex(i)->getCharId() == charId)
				{
					// we found it, mark it as kicked
					session->getSessionParticipantsByIndex(i)->setKicked(true);
					session->getSessionParticipantsByIndex(i)->update(_RingDb);

					// warn the session server to remove this character
					TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
					if (it != _SessionIndex.end())
					{
						CRingSessionManagerClientProxy rsmc(it->second);

						rsmc.characterKicked(this, sessionId, charId);
					}

					// result ok
					invokeResult(from, ownerCharId>>4, 0, "");

					return;
				}
			}

			// participation not found
			invokeResult(from, ownerCharId>>4, 3, "Participation not found");
		}

		// Unkick a character from a session
		// invokeReturn : 0 : ok, character kicked
		//                1 : char not found
		//                2 : session not found
		//                3 : character has no participation in the session
		//                4 : internal error
		//				  5 : owner char don't own the session
		void on_unkickCharacter(NLNET::TSockId from, uint32 ownerCharId, TSessionId sessionId, uint32 charId)
		{
			nldebug("RSM : on_unkickCharacter : web ask to unkick character %u from session %u",
				charId,
				sessionId.asInt());

			uint32 userId = ownerCharId >> 4;
			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "RSM : on_unkickCharacter : failed to load session "<<sessionId, invokeResult(from, ownerCharId>>4, 2, "Failed to load session"); return);
			if (session->getOwnerId() != ownerCharId)
			{
				// check if the char has the required rights
				if (!hasSessionAccessPrivilege(userId))
				{
					STOP("RSM : on_unkickCharacter : char "<<ownerCharId<<" don't own the session "<<sessionId);
					invokeResult(from, ownerCharId>>4, 5, "Owner char don't own the session");
					return;
				}
			}
			// load the participants
			BOMB_IF(!session->loadSessionParticipants(_RingDb, __FILE__, __LINE__), "RSM : on_kickCharacter : failed to load session participants for session "<<sessionId, invokeResult(from, ownerCharId>>4, 4, "Failed to load session participants"); return);

			// look for the participation of the character
			for (uint i=0; i<session->getSessionParticipants().size(); ++i)
			{
				if (session->getSessionParticipantsByIndex(i)->getCharId() == charId)
				{
					// we found it, mark it as unkicked
					session->getSessionParticipantsByIndex(i)->setKicked(false);
					session->getSessionParticipantsByIndex(i)->update(_RingDb);

					TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
					if (it != _SessionIndex.end())
					{
						CRingSessionManagerClientProxy rsmc(it->second);

						rsmc.characterUnkicked(this, sessionId, charId);
					}
					// result ok
					invokeResult(from, ownerCharId>>4, 0, "");
					return;
				}
			}

			// participation not found
			invokeResult(from, ownerCharId>>4, 3, "Participation not found");
		}

		// A user invite a guild to play in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild already invited
		//                5 : char don't own the session
		virtual void on_inviteGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId)
		{
			nldebug("RSM : onInviteGuild char %u invite guild %u in session %u",
						charId,
						guildId,
						sessionId.asInt());

			// load the user
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, charId>>4, 2, "Char not found");
				return;
			}

			// load the guild
			CGuildPtr guild = CGuild::load(_RingDb, guildId, __FILE__, __LINE__);
			if (guild == NULL)
			{
				invokeResult(from, charId>>4, 1, "Guild not found");
				return;
			}

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, charId>>4, 3, "Session not found");
				return;
			}

			// check the session owner
			if (session->getOwnerId() != charId)
			{
				invokeResult(from, charId>>4, 5, "Character don't own the session");
				return;
			}

			// check that the session is not already invited
			session->loadGuildInvites(_RingDb, __FILE__, __LINE__);
			uint i;
			for (i=0; i<session->getGuildInvites().size(); ++i)
			{
				CGuildInvitePtr invite = session->getGuildInvitesByIndex(i);

				if (invite->getGuildId() == guildId)
				{
					invokeResult(from, charId>>4, 4, "The guild is already invited");
					return;
				}
			}

			// ok, we can create a new invitaion
			CGuildInvitePtr invite = CGuildInvite::createTransient(__FILE__, __LINE__);

			invite->setGuildId(guildId);
			invite->setSessionId(sessionId.asInt());

			// store the new invite
			invite->create(_RingDb);

			// result ok
			invokeResult(from, charId>>4, 0, "");
		}

		// Remove a guild invitation in a session
		// invokeReturn : 0 : ok, guild invited
		//                1 : guild not found
		//                2 : char not found
		//                3 : session not found
		//                4 : guild not invited
		//                5 : char don't own the session
		virtual void on_removeInvitedGuild(NLNET::TSockId from, uint32 charId, TSessionId sessionId, uint32 guildId)
		{
			nldebug("RSM : removeInvitedGuild char %u revoke invitattion for guild %u in session %u",
						charId,
						guildId,
						sessionId.asInt());

			// load the user
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, charId>>4, 2, "Character not found");
				return;
			}

			// load the guild
			CGuildPtr guild = CGuild::load(_RingDb, guildId, __FILE__, __LINE__);
			if (guild == NULL)
			{
				invokeResult(from, charId>>4, 1, "Guild not found");
				return;
			}

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, charId>>4, 3, "Session not found");
				return;
			}

			// check session ownership
			if (session->getOwnerId() != charId)
			{
				invokeResult(from, charId>>4, 5, "User don't own the session");
				return;
			}

			// load the invitations
			session->loadGuildInvites(_RingDb, __FILE__, __LINE__);

			// search the invitation
			uint i;
			for (i=0; i<session->getGuildInvites().size(); ++i)
			{
				CGuildInvitePtr &invite = session->getGuildInvitesByIndex(i);
				if (invite->getGuildId() == guildId)
				{
					// we found it !
					break;
				}
			}
			if (i == session->getGuildInvites().size())
			{
				// invitation not found
				invokeResult(from, charId>>4, 4, "Guild not invited");
				return;
			}

			// ok, we can remove then invitation
			CGuildInvitePtr invite = session->getGuildInvitesByIndex(i);
			invite->remove(_RingDb);

			// result ok
			invokeResult(from, charId>>4, 0, "");
		}

		// Set the additional scenario info
		// playType is the enumerated type TPlayType
		// invokeReturn : 0 : ok, info setted
		//                1 : scenario not found
		//                2 : user not owner of session
		//                3 : user not found
		//                4 : session not found
//		virtual void on_setScenarioInfo(NLNET::TSockId from,
//			uint32 userId,
//			TSessionId sessionId,
//			const std::string &title,
//			const std::string &journal,
//			const std::string &credits,
//			uint32 numPlayer,
//			const std::string &playType)
		virtual void on_setScenarioInfo(NLNET::TSockId from, uint32 charId, TSessionId sessionId, const std::string &title, uint32 numPlayer, const std::string &playType)
		{
			nldebug("RSM : setScenarioInfo char %u set scenario info for session %u", charId, sessionId.asInt());

			nlstopex(("Deprecated"));

//			// load the user
//			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
//			if (character == NULL)
//			{
//				invokeResult(from, charId>>4, 3, "Character not found");
//				return;
//			}
//
//			// load the session
//			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
//			if (session == NULL)
//			{
//				invokeResult(from, charId>>4, 4, "Session not found");
//				return;
//			}
//
//			// check session ownership
//			if (session->getOwnerId() != charId)
//			{
//				invokeResult(from, charId>>4, 2, "Character don't own the session");
//				return;
//			}
//
//			// load the scenario desc
//			CScenarioDescPtr scenario = CScenarioDesc::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
//			if (scenario == NULL)
//			{
//				invokeResult(from, charId>>4, 1, "ScenarioDesc not found");
//				return;
//			}
//
//			// ok, we can update the scenario desc
//			scenario->setTitle(title);
//			//oups, data do not matchs
//			nlstop;
//
			invokeResult(from, charId>>4, 100, "Deprecatted");
		};


		// Add an entry in the session journal
		// invokeReturn : 0 : ok, entry added
		//                1 : scenario not found
		//                2 : user can't post in this journal
		//                3 : user not found
		//                4 : session not found
		//                5 : user participation not found
		//                6 : character not found
		//                7 : invalid params
		virtual void on_addJournalEntry(NLNET::TSockId from,
			uint32 charId,
			TSessionId sessionId,
			const std::string &entryType,
			const std::string &text)
		{
			nldebug("RSM : addJournalEntry char %u add an entry in journal of session %u", charId, sessionId.asInt());
			// check the enum params
			TJournalEntryType type(entryType);
			if (type == TJournalEntryType::invalid_val)
			{
				invokeResult(from, charId>>4, 7, "Invalid entryType");
				return;
			}

			uint32 userId = charId>>4;
			// load the user
			CRingUserPtr ru = CRingUser::load(_RingDb, userId, __FILE__, __LINE__);
			if (ru == NULL)
			{
				invokeResult(from, charId>>4, 3, "User not found");
				return;
			}

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			if (session == NULL)
			{
				invokeResult(from, charId>>4, 4, "Session not found");
				return;
			}

			// load the character
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			if (character == NULL)
			{
				invokeResult(from, charId>>4, 6, "Character not found");
				return;
			}

			// check that this char is animator in the session
			session->loadSessionParticipants(_RingDb, __FILE__, __LINE__);
			uint i;
			for (i=0; i<session->getSessionParticipants().size(); ++i)
			{
				CSessionParticipantPtr &cp = session->getSessionParticipantsByIndex(i);
				if (cp->getCharId() == charId)
				{
					if (cp->getStatus() == TSessionPartStatus::sps_animating)
					{
						// ok, this guys is allowed to post in journal
						break;
					}
					else
					{
						// this guy can't post
						invokeResult(from, charId>>4, 2, "The user is not authorized to post in the session journal");
						return;
					}
				}
			}
			if (i == session->getSessionParticipants().size())
			{
				invokeResult(from, charId>>4, 5, "Participation not found for user in the session");
				return;
			}

			// ok, the user can post in the journal
			CJournalEntryPtr journal = CJournalEntry::createTransient(__FILE__, __LINE__);

			journal->setSessionId(sessionId.asInt());
			journal->setAuthor(charId);
			journal->setType(type);
			journal->setText(text);
			journal->setTimeStamp(NLMISC::CTime::getSecondsSince1970());

			// store the journal entry
			journal->create(_RingDb);

			// result ok
			invokeResult(from, charId>>4, 0, "");
		}

		// Set the rating for a scenario
		// invokeReturn : 0 : ok, rating set
		//                1 : scenario not found
		//                2 : char is not found
		//                3 : char is not a participant of session
		//                4 : no session log for the session
		//                5 : char is banned from session
		//                6 : session not found
		//                7 : scenario not found
		//                8 : internal error
		virtual void on_setPlayerRating(NLNET::TSockId from, uint32 charId, TSessionId sessionId,
			uint32 rateFun,
			uint32 rateDifficulty,
			uint32 rateAccessibility,
			uint32 rateOriginality,
			uint32 rateDirection)
		{
			nldebug("Web set player rating from char %u to session %u as %u,%u,%u,%u, %u",
				charId, sessionId.asInt(),
				rateFun, rateDifficulty, rateAccessibility, rateOriginality, rateDirection);

			// get the session
			CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(session == NULL, "Invalid session "<<sessionId.asInt(), invokeResult(from, charId>>4, 6, "Session not found"); return );

			// check that the char has an active participation in the session (and is not ban)
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "Invalid character "<<charId, invokeResult(from, charId>>4, 2, "Character not found"); return );

			BOMB_IF(!character->loadSessionParticipants(_RingDb, __FILE__, __LINE__),
				"Failed to load session participation for char "<<charId, invokeResult(from, charId>>4, 3, "Failed to load participation"); return);
			uint i=0;
			for (i=0; i<character->getSessionParticipants().size(); ++i)
			{
				const CSessionParticipantPtr &sp = character->getSessionParticipants()[i];
				if (sp->getSessionId() == sessionId)
				{
					// we have found the participation

					if (sp->getKicked() == true)
					{
						invokeResult(from, charId>>4, 5, "Char is banned from the session");
						return;
					}

					break;
				}
			}
			BOMB_IF(i == character->getSessionParticipants().size(),
				"No participation found for char "<<charId<<" in session "<<sessionId.asInt(), invokeResult(from, charId>>4, 3, "No participation found in session"); return);

			// retrieve the scenario data
			CSessionLogPtr sessionLog = CSessionLog::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);
			BOMB_IF(sessionLog == NULL, "Failed to load the session log for session "<<sessionId.asInt(), invokeResult(from, charId>>4, 4, "No session log found for the session"); return);

			// load the scenario data
			CScenarioPtr scenario = CScenario::load(_RingDb, sessionLog->getScenarioId(), __FILE__, __LINE__);
			BOMB_IF(sessionLog == NULL, "Failed to load the scenario "<<sessionLog->getScenarioId(), invokeResult(from, charId>>4, 7, "No scenario info found for the session"); return);

			// lookup for an existing rating to update
			CPlayerRatingPtr playerRating;

			character->loadPlayerRatings(_RingDb, __FILE__, __LINE__);
			for (uint i=0; i<character->getPlayerRatings().size(); ++i)
			{
				const CPlayerRatingPtr &existingPR = character->getPlayerRatings()[i];
				if (existingPR->getScenarioId() == sessionLog->getScenarioId())
				{
					// yea, rating exist
					playerRating.assign(existingPR, __FILE__, __LINE__);
					break;
				}
			}
			if (playerRating == NULL)
			{
				// create a new player rating
				playerRating = CPlayerRating::createTransient(__FILE__, __LINE__);
				playerRating->setAuthor(charId);
				playerRating->setScenarioId(sessionLog->getScenarioId());
			}

			// set the rating values
			playerRating->setRateFun(rateFun);
			playerRating->setRateDifficulty(rateDifficulty);
			playerRating->setRateAccessibility(rateAccessibility);
			playerRating->setRateOriginality(rateOriginality);
			playerRating->setRateDirection(rateDirection);

			if (playerRating->getPersistentState() == NOPE::os_transient)
			{
				// this is a new transient instance, create the new object in DB
				BOMB_IF(!playerRating->create(_RingDb), "Failed to create new player rating", invokeResult(from, charId>>4, 8, "Internal database error"); return);
			}
			else
			{
				// just update the database
				playerRating->update(_RingDb);
			}

			// ok, all done
			invokeResult(from, charId>>4, 0, "");
		}

		///////////// ICharacterSyncCb /////////////////////
		/** Callback called when the name of a character have been changed */
		void onCharacterNameUpdated(uint32 charId, const std::string &oldName, const std::string &newName)
		{
			// nothing
		}

		/** Callback called when a character is deleted/removed */
		virtual void onBeforeCharacterDelete(uint32 charId)
		{
			nldebug("RSM : closing session for deleted char %u", charId);

			// close all the sessions and stop hibernation of the editing session
			CCharacterPtr character = CCharacter::load(_RingDb, charId, __FILE__, __LINE__);
			BOMB_IF(character == NULL, "A character to begin deleted can't be loaded !", return);

			// load the sessions of this character
			BOMB_IF(!character->loadSessions(_RingDb, __FILE__, __LINE__), "Failed to load session of a character", return);

			// for each session
			for (uint i=0; i<character->getSessions().size(); ++i)
			{
				CSessionPtr &session = character->getSessionsByIndex(i);
				if (session->getState() == TSessionState::ss_open)
				{
					// try to close the session
					TSessionServersIdx::iterator it(_SessionIndex.find(session->getObjectId()));
					if (it != _SessionIndex.end())
					{
						// ok, we have info on it
						CRingSessionManagerClientProxy rsmc(it->second);
						rsmc.closeSession(this, session->getObjectId());
					}
				}
				else if (session->getSessionType() == TSessionType::st_edit)
				{
					// for edit session, ask DSS to stop hibernation
					if (!_SessionServers.empty())
					{
						// use the first available one
						CRingSessionManagerClientProxy rsmc(_SessionServers.begin()->first);
						rsmc.stopHibernation(this, TSessionId(session->getObjectId()), charId);
					}
				}
			}
		}



		NLMISC_COMMAND_HANDLER_TABLE_EXTEND_BEGIN(CRingSessionManager, CModuleBase)
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, dump, "dump the session manager internal state", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, forceSessionCleanup, "force a database synchronisation with current running session", "no param");
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, setWSState, "Set the access state of a welcome service", "<shardId|shardName> OPEN|DEV|RESTRICTED|CLOSED <messageOfTheDay>");
			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, banCharFromSession, "Ban a character from a session", "<charId> <sessionId>");
//			NLMISC_COMMAND_HANDLER_ADD(CRingSessionManager, listTemporaryAssoc, "List the temporary assoc of character to ring shard", "no param");
		NLMISC_COMMAND_HANDLER_TABLE_END

		NLMISC_CLASS_COMMAND_DECL(banCharFromSession)
		{
			if (args.size() != 2)
				return false;

			uint32 charId;
			NLMISC::fromString(args[0], charId);

			uint32 sessionId;
			NLMISC::fromString(args[1], sessionId);

			log.displayNL("Banning char %u from session %u", charId, sessionId);

			// load the session
			CSessionPtr session = CSession::load(_RingDb, sessionId, __FILE__, __LINE__);
			if (session == NULL)
			{
				log.displayNL("failed to load session %u", sessionId);
				return true;
			}

			// load the participants
			if (!session->loadSessionParticipants(_RingDb, __FILE__, __LINE__))
			{
				log.displayNL("failed to load session participants for session %u", sessionId);
				return true;
			}

			// look for the participation of the character
			for (uint i=0; i<session->getSessionParticipants().size(); ++i)
			{
				if (session->getSessionParticipantsByIndex(i)->getCharId() == charId)
				{
					// we found it, mark it as kicked
					session->getSessionParticipantsByIndex(i)->setKicked(true);
					session->getSessionParticipantsByIndex(i)->update(_RingDb);

					// warn the session server to remove this character
					TSessionServersIdx::iterator it(_SessionIndex.find(sessionId));
					if (it != _SessionIndex.end())
					{
						CRingSessionManagerClientProxy rsmc(it->second);

						rsmc.characterKicked(this, sessionId, charId);
					}

					// result ok
					log.displayNL("Character %u has been kicked and ban from session %u", charId, sessionId);

					return true;
				}
			}

			log.displayNL("No participation found for character %u in session %u, can not ban", charId, sessionId);

			return true;
		}

//		NLMISC_CLASS_COMMAND_DECL(listTemporaryAssoc)
//		{
//			if (args.size() != 0)
//				return false;
//
//			log.displayNL("Listing %u temporary shard assoc :", _TemporaryShardAssocs.size());
//
//			TTemporaryShardAssocs::const_iterator first(_TemporaryShardAssocs.begin()), last(_TemporaryShardAssocs.end());
//			for (; first != last; ++first)
//			{
//				const TTemporaryShardAssoc &tsa = first->second;
//				log.displayNL("  The character %u is stuck to DSS %s with a %s session %u",
//					first->first,
//					tsa.DSSProxy->getModuleName().c_str(),
//					tsa.SessionClosedDate != 0 ? "closed" : "open or planned",
//					tsa.SessionId.asInt());
//			}
//
//			return true;
//		}

		NLMISC_CLASS_COMMAND_DECL(setWSState)
		{
			if (args.size() > 3 || args.size() < 2)
				return false;

			uint32 shardId;
			NLMISC::fromString(args[0], shardId);

			if (shardId == 0)
			{
				// try with the shard name
				shardId = CShardNames::getInstance().getShardId(args[0]).asInt();
				if (shardId == 0)
				{
					log.displayNL("Invalid shard id or name '%s'", args[0].c_str());
					return true;
				}
			}

			TAccessLevel al = string("ds_")+toLower(args[1]);
			if (al == TAccessLevel::invalid_val)
			{
				log.displayNL("Invalid access state '%s'", args[1].c_str());
				return true;
			}

			// load the shard info
			CShardPtr shard = CShard::load(_RingDb, shardId, __FILE__, __LINE__);
			if (shard == NULL)
			{
				log.displayNL("Can't find shard info for shard %u in nel DB", shardId);
				return true;
			}

//			// check that one don't try to close or unclose manually a WS
//			if (shard->getCurrentState() == TAccessLevel::ds_close && al != TAccessLevel::ds_close)
//			{
//				log.displayNL("You are not allowed to unclose a closed WS, you need to wait for the WS to open itself (check for missing services on the shard)");;
//				return true;
//			}
//			if (shard->getCurrentState() != TAccessLevel::ds_close && al == TAccessLevel::ds_close)
//			{
//				log.displayNL("You are not allowed to close a WS, a WS is closed only if it mess some needed services");;
//				return true;
//			}
//
			// update the shard required state
			shard->setRequiredState(al);
			if (args.size() > 2)
			{
				shard->setMOTD(args[2]);
				log.displayNL("Shard %u set required state to '%s' with message '%s'",
					shardId,
					al.toString().c_str(),
					args[2].c_str());
			}
			else
			{
				log.displayNL("Shard %u set required state to '%s'",
					shardId,
					al.toString().c_str());
				shard->setMOTD("");
			}

			// update WSState if possible


			shard->update(_RingDb);


			return true;
		}

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
				uint32 sessionNum;
				result->getField(0, sessionNum);
				TSessionId sessionId(sessionNum);

				if (_SessionIndex.find(sessionId) == _SessionIndex.end())
				{
					// this session is not running !
					sessionToClose.insert(sessionId.asInt());
				}
			}

			while (!sessionToClose.empty())
			{
				TSessionId sessionId(*sessionToClose.begin());
				log.displayNL("Closing session %u", sessionId.asInt());
				CSessionPtr session = CSession::load(_RingDb, sessionId.asInt(), __FILE__, __LINE__);

				session->setState(TSessionState::ss_closed);
				session->update(_RingDb);

				sessionToClose.erase(sessionToClose.begin());
			}

			return true;

		}

		NLMISC_CLASS_COMMAND_DECL(dump)
		{
			// call the base class dump
			NLMISC_CLASS_COMMAND_CALL_BASE(CModuleBase, dump);

			log.displayNL("--------------------------------");
			log.displayNL("Dumping RingSessionManager state");
			log.displayNL("--------------------------------");
			// dumping Known Welcome service
			{
				log.displayNL("Dumping %u known Welcome Service module :", _WelcomeServices.size());
				TWelcomeServices::iterator first(_WelcomeServices.begin()), last(_WelcomeServices.end());
				for (; first != last; ++first)
				{
					uint32 shardId = first->first;
					TWelcomeServiceInfo& wsInfo = first->second;
					log.displayNL(" + Welcome service '%s' for shard %u", wsInfo.WSModuleProxy->getModuleName().c_str(), shardId);
					if (wsInfo.FixedSessionId.asInt() != 0)
						log.displayNL("     Mainland sessionId: %u", wsInfo.FixedSessionId.asInt());
					log.displayNL("     Access State = '%s', online = %s, message = '%s'",
						wsInfo.ShardInfo->getRequiredState().toString().c_str(),
						wsInfo.ShardInfo->getWSOnline() ? "true" : "false",
						wsInfo.ShardInfo->getMOTD().c_str());
					log.displayNL("     %u online players", wsInfo.NbOnlinePlayers);
					log.displayNL("     %u pending players", wsInfo.NbPendingPlayers);
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
					log.displayNL("          Total player = %u", ssi.NbTotalPlayingChars);
					log.displayNL("   Nb running sessions = %u", ssi.HostedSessions.size());

					// dumping the sessions
					{
						TSessionServerInfo::THostedSessions::iterator first(ssi.HostedSessions.begin()), last(ssi.HostedSessions.end());
						for (; first != last; ++first)
						{
							TSessionId sessionId = first->first;
							TRunningSessionInfo &rsi = first->second;
							log.displayNL("    + Session %u :", sessionId.asInt());
							log.displayNL("           Session Type = %s", rsi.getSessionType().toString().c_str());
							log.displayNL("       Number of player = %u", rsi.getNbPlayingChars());
							log.displayNL("             InstanceId = %u", rsi.getInstanceId());
						}
					}
				}
			}

			return true;
		}


	};
	NLNET_REGISTER_MODULE_FACTORY(CRingSessionManager, RingSessionManagerClassName);


} // namespace RSMGR


