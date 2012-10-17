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


#include "stdpch.h"

#include "server_edition_module.h"

#include "dms.h"

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
#include "game_share/backup_service_interface.h"
#include "game_share/task_list.h"
#include "game_share/ring_session_manager_itf.h"

//#include "game_share/module_security.h"
//nclude "game_share/module_security.h"

#include "game_share/object.h"
#include "game_share/scenario.h"
#include "game_share/r2_messages.h"
#include "game_share/scenario_entry_points.h"
#include "game_share/ring_access.h"

#include "game_share/ai_wrapper.h"

#include "r2_modules_itf.h"

//#include "server_admin_module.h"


using namespace std;
using namespace NLMISC;
using namespace NLLIGO;
using namespace NLNET;



CVariable<uint>	MaxNpcs( "DSS", "MaxNpcs", "Maximum number of Npc's allowed in an act", 100, 0, true );
CVariable<uint>	MaxStaticObjects( "DSS", "MaxStaticObjects", "Maximum number of Static Object allowed in an act", 100, 0, true );

CVariable<bool>	AutoCreateSession( "DSS", "AutoCreateSession", "Create a session for each Client Edition Module and add char as an editor", false, 0, true );
CVariable<bool>	AutoCreateAnimationSession( "DSS", "AutoCreateAnimationSession", "Create an animation session", false, 0, true );
CVariable<bool>	AutoCreateAddCharacter ( "DSS", "AutoCreateAddCharacter", "Add a character to a session witout creating it", false, 0, true );
CVariable<string>	AutoCreateRole( "DSS", "AutoCreateRole", "Set the Role of default connected ur_player | ur_editor | ur_animator", "ur_editor", 0, true );
CVariable<bool> AutoCreateSessionMultiChar("DSS", "AutoCreateSessionMultiChar", "Allow a Cooperative edition with local server", false, 0, true);
CVariable<uint32> AutoCreateSessionMultiCharSessionId("DSS", "AutoCreateSessionMultiCharSessionId", "Session Id of a AutoCreateSession", 1, 0, true);

// 30 secondes
CVariable<uint32> TimeBeforeDisconnectionAfterKick("DSS", "TimeBeforeDisconnectionAfterKick", "Delay in second the client has before quit a session after being kicked from it", 30, 0, true);

// If true load hibernating session and saved session at startup
CVariable<bool> DssUseBs("DSS", "DssUseBs", "If activate the resume/hibernation system is active", true, 0, true);
// 1 minutes
CVariable<uint32> TimeBeforeAutoSaveTimeEditionSession("DSS", "TimeBeforeAutoSaveTimeEditionSession", "Delay in seconde before empty edition is saved (save occurs only if scenario has changed)", 60, 0, true);
// 5 minutes
CVariable<uint32> TimeBeforeStopTestInEditionSessionWithNoPlayer("DSS", "TimeBeforeStopTestInEditionSessionWithNoPlayer", "Delay in secondes before test in empty edition is stopped", 5*60, 0, true);
// 15 minutes
CVariable<uint32> TimeBeforeAutoHibernateEditionSessionWithNoPlayer ("DSS", "TimeBeforeAutoHibernateEditionSessionWithNoPlayer ", "Delay in secondes before an empty edition is hibernating", 15*60, 0, true);
// 7 days
CVariable<uint32> TimeBeforeCloseHibernatingEditionSession("DSS", "TimeBeforeCloseHibernatingEditionSession", "Delay in secondes before a empty hibernating edition is remove", 7*24*60, 0, true);

// 15 minutes
CVariable<uint32> TimeBeforeAutoCloseAnimationSessionWithNoPlayer("DSS", "TimeBeforeAutoCloseAnimationSessionWithNoPlayer", "Delay in secondes before a empty animation session is closed", 15*60, 0, true);

CVariable<std::string>	DefaultCharRingAccess( "DSS", "DefaultCharRingAccess", "Default Access Ring for player eg 'f1:j1:l1:d1:p1:g1:a1:'", "a1", 0, true );
CVariable<bool>	DontUseSU( "DSS", "DontUseSU", "If 1, allow running a shard with no Shard Unifier Service", false, 0, true );

CVariable<bool> AcceptScenarioWithoutHeader("DSS", "AcceptScenarioWithoutHeader", "AcceptScenarioWithoutHeader", false, 0, true);
CVariable<bool> MustVerifyRingAccessWhileLoadingAnimation("DSS", "MustVerifyRingAccessWhileLoadingAnimation", "MustVerifyRingAccessWhileLoadingAnimation", false, 0, true);
CVariable<int> OverrideRoSValue("DSS", "OverrideRoSValue", "OverrideRoSValue", -1, 0, true);


// anonymous namespace
namespace
{
	static inline void debugInfo(bool ok, const std::string &msg)
	{
		if (ok) { nlinfo("Ed: %s", msg.c_str()); } else { nlwarning("Ed: %s", msg.c_str()); }
	}
}

namespace R2
{
	// Recycle aiInstanceId or CharacterEditingSlotId
	class CIdRecycle
	{
		std::set<uint32> _RecycledId;
		std::set<uint32> _UsedId;

	public:
		// no const (give an new id or a recyled id)
		uint32 newId();
		void releaseId(uint32 id);
		CIdRecycle(uint32 initialValue):_InitialValue(initialValue){}
	public:
		uint32 _InitialValue;
	};

	/*! Identify an user
	* Identify a user (it's editor id / characterId / playerId type of session (edition / animation ....)
	*/
	class CCurrentChar
	{
	public:
		typedef uint32 TCharId;
	public:

		CCurrentChar(TCharId charId, uint32 editSlotId, const R2::TUserRole& userRole = R2::TUserRole::ur_editor)
			:_EditSlotId(editSlotId), _CharId(charId), _UserRole(userRole){}

		void setUserRole(const R2::TUserRole& userRole)
		{
			_UserRole = userRole;
		}
		uint32 getCharId() const		{ return _CharId;}
	//	uint32 getUserId() const		{ return _UserId;}
		uint32 getEditSlotId() const	{ return _EditSlotId;}
		TUserRole getRole() const		{ return _UserRole;}

	private:
	//	uint32 _UserId;
		uint32		_EditSlotId;
		TCharId		_CharId;
		TUserRole	_UserRole;
		//eid
		//row???
	};


	// Use the store info about an edition session
	// Hold the sessionId / AiInstanec of the player
	// Hold the character scenario
	// save update / close / hibernate scenario

	class CEditionSession
	{
	public:
		typedef uint32 TCharId;
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TAiInstanceId;
		typedef std::list<CCurrentChar*> TCurrentChars;
	public:

		CEditionSession(RSMGR::TSessionType type = RSMGR::TSessionType::invalid_val, TSessionId sessionId = TSessionId(0), TCharId ownerCharId = 0, TAiInstanceId aiInstanceId = 0, CScenario* scenario = 0);
		// This session shared data with Scenario
		CEditionSession(RSMGR::TSessionType type, TSessionId sessionId, CEditionSession* editionSession);

		CScenario* getScenario() const;
		bool isNoCharConnected() const;
		~CEditionSession();

		//Session Type
		RSMGR::TSessionType getSessionType() const;
		TScenarioSessionType getScenarioSessionType() const;
		void setSessionType(RSMGR::TSessionType sessionType) { _SessionType = sessionType; }

		// Session Tasks
		void serial(NLMISC::IStream& stream);
		bool mustClose(uint32 currentTime) const;
		bool mustSave(uint32 currentTime) const;
		void saved(uint32  currentTime);
		bool mustStopAnimation(uint32  currentTime) const;
		bool mustHibernate(uint32 currentTime) const;
		void update(uint32 currentTime);
		uint32 getLastSaveTime() const;
		void touch(); // simulate sombody change the content of the session.


		void verifyRingAcces(TCharId charId);
		//AiInstance is the aiInstanceId of the player
		//The sessionId is a number given by de sql database
		TAiInstanceId getAiInstanceId() const;
		void setAiInstanceId(TAiInstanceId aiInstanceId);
		// ! only swap _scenario
		void swap(CEditionSession& other);

		uint32 connectChar(TCharId charId, TUserRole connectedAs);
		void disconnectChar(TCharId charId);

		uint32 getEditSlotId(TCharId charId) const;

		TUserRole getCharRole(TCharId charId) const;

		const TCurrentChars& getCurrentChars() const { return _CurrentChars; }
		TCharId getOwnerCharId() const { return _OwnerCharId; }

		void setStartParams(sint32 x, sint32 y, uint8 season){ InitialX = x; InitialY = y; InitialSeason = season;}

		void getStartParamsWithDelay(CServerEditionModule* module, uint32 charId, TSessionId lastStoredSessionId);

		bool isLoaded() const;
		void setLoaded(bool value);

	public:
		NLMISC::CSmartPtr<CScenario> _Scenario;
		TSessionId SessionId;

		uint32 DateSinceNoPlayer;
		std::string RingAccess;
		bool IsAnimationStopped;
		sint32 InitialX, InitialY;
		uint8 InitialSeason;
		std::set<TCharId> CharsToVerify;


	private:
		uint32 _LastSaveTime;
		RSMGR::TSessionType _SessionType;
		TAiInstanceId _AiInstance;
		TCurrentChars _CurrentChars;
		CIdRecycle _EditSlotIdMaker;
		TCharId _OwnerCharId;
		CTaskList<NLMISC::TTime> _WaitingForConnectionTasks; // StartPos
		bool _Loaded;


	};


	class CTaskUpdateScenarioVision: public CTask<NLMISC::TTime>
	{
	public:
		CTaskUpdateScenarioVision(NLMISC::TTime now, CServerEditionModule* module, TSessionId sessionId)
			:CTask<NLMISC::TTime>(now), _SessionId(sessionId), _Module(module){}
		virtual void doOperation()
		{
			_Module->updateScenarioVision(_SessionId);
		}
	private:
		TSessionId _SessionId;
		CServerEditionModule* _Module;
	};


	class CTaskGetStartParamsWhenLoaded: public CTask<NLMISC::TTime>
	{
	public:
		typedef uint32 TCharId;
	public:
		CTaskGetStartParamsWhenLoaded(CServerEditionModule* module, TCharId charId, TSessionId sessionId)
			:_Module(module), _CharId(charId), _SessionId(sessionId){}

		virtual void doOperation()
		{
			_Module->getStartParamsImpl(_CharId, _SessionId);
		}
	private:
		CServerEditionModule* _Module;
		TCharId	_CharId;
		TSessionId _SessionId;
	};
	/*
	class CTaskConnectPlayer: public CTask<NLMISC::TTime>
	{
	public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;

	public:

		CTaskConnectPlayer(CServerEditionModule* serverEditionModule, TSessionId sessionId, TCharId charId, TUserRole userRole, const std::string& ringAccess, bool newcomer)
			:CTask<NLMISC::TTime>(0)
			,_ServerEditionModule(serverEditionModule), _SessionId(sessionId), _CharId(charId), _UserRole(userRole), _RingAccess(ringAccess), _Newcomer(newcomer){}

		virtual void doOperation()
		{
			_ServerEditionModule->connectChar(_SessionId, _CharId, _UserRole, _RingAccess, _Newcomer);
		}
	private:
		CServerEditionModule* _ServerEditionModule;
		uint32 _CharId;
		TSessionId _SessionId;
		std::string _RingAccess;
		R2::TUserRole _UserRole;
		bool _Newcomer;
	};

*/
	class CTaskTryConnectPlayer: public  CTask<NLMISC::TTime>
	{
	public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;

	public:
		CTaskTryConnectPlayer(CServerEditionModule* serverEditionModule, TCharId charId)
			:CTask<NLMISC::TTime>(0)
			,_ServerEditionModule(serverEditionModule), _CharId(charId){}

		virtual void doOperation()
		{
			_ServerEditionModule->tryCharConnection(_CharId);
		}

	private:
		CServerEditionModule* _ServerEditionModule;
		uint32 _CharId;
	};

	class CKickPlayerIfStillConnected: public  CTask<NLMISC::TTime>
	{
	public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;

	public:
		CKickPlayerIfStillConnected(NLMISC::TTime taskDate, CServerEditionModule* serverEditionModule, TSessionId sessionId, TCharId charId)
			: CTask<NLMISC::TTime>(taskDate),_ServerEditionModule(serverEditionModule), _CharId(charId), _SessionId(sessionId)
		{
		}

		virtual void doOperation()
		{
			// The player has return to main land (or quit) So do not disconnect him
			if ( _ServerEditionModule->getSessionIdByCharId(_CharId) == _SessionId)
			{

				std::string msg;
				bool ok = _ServerEditionModule->removeCharacterFromSessionImpl(_SessionId, _CharId, msg);
				debugInfo(ok, msg);
			}

		}
		uint32 getCharId() const {return _CharId; }
		TSessionId getSessionId() const { return _SessionId;}
	private:
		CServerEditionModule* _ServerEditionModule;
		uint32 _CharId;
		TSessionId _SessionId;

	};



	class CVerifyRingAccess: public  CTask<NLMISC::TTime>
	{
	public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;
	public:
		CVerifyRingAccess(NLMISC::TTime taskDate, CServerEditionModule* serverEditionModule, TSessionId sessionId, TCharId charId)
			:CTask<NLMISC::TTime>(taskDate),_ServerEditionModule(serverEditionModule), _CharId(charId), _SessionId(sessionId)
		{
		}

		virtual void doOperation()
		{
			CEditionSession* session = _ServerEditionModule->getSession(_SessionId);
			if (session)
			{
				std::set<TCharId>::iterator found = session->CharsToVerify.find(_CharId);
				if (found != session->CharsToVerify.end())
				{
					session->CharsToVerify.erase(found);
				}
			}

			// The player has return to main land (or quit) So do not disconnect him
			if ( _ServerEditionModule->getSessionIdByCharId(_CharId) == _SessionId)
			{
				_ServerEditionModule->verifyRingAccess(_SessionId, _CharId);
			}

		}

	private:
		CServerEditionModule* _ServerEditionModule;
		uint32 _CharId;
		TSessionId _SessionId;

	};


	class CTestIfMustWakeUpSession : public IBackupFileClassReceiveCallback
	{
	public:
		CTestIfMustWakeUpSession(CServerEditionModule* module, TSessionId sessionId, TCharId ownerId)
			:_Module(module), _SessionId(sessionId), _OwnerId(ownerId){}

		void	callback(const CFileDescriptionContainer& fileList)
		{
			if (fileList.empty())
			{
				_Module->setLoaded(_SessionId);
			}
			else
			{
				std::string msg;
				bool ok = _Module->wakeUpSession(_SessionId, _OwnerId, msg);
				debugInfo(ok, msg);

			}

		}

	private:
		CServerEditionModule* _Module;
		TSessionId _SessionId;
		TCharId _OwnerId;
	};


	class CKeysHolder
	{


		public:

			enum TPolicy { Accept, Refuse, Test};

			typedef std::string TKeyName;

			class CKeyValue
			{
			public:
				CKeyValue(){}

				CKeyValue(const std::string & privateKeyValue, TPolicy policy)
					:PrivateKeyValue(privateKeyValue), Policy(policy){}
			public:
				std::string PrivateKeyValue;

				TPolicy Policy;
			};
			typedef std::map<std::string, CKeyValue> TKeys;

		public:

			void resetPolicies();

			void addKeyPolicy(const std::string& key, const std::string& value, TPolicy policy);

			void setDefaultKey(const std::string& defaultKey);

			std::string getCurrentSignature(const std::string& md5) const;

			std::string getSignature(const std::string& md5, const std::string& key) const;

			bool verifyKey(const std::string& md5, const std::string& signature);
		public:
			TKeys _Keys;
			std::string	_CurrentKey;

	};

}



using namespace R2;


//----------------- <CKeysHolder> --------------------------------------------
void CKeysHolder::resetPolicies()
{
	_CurrentKey = "";
	_Keys.clear();
}

void CKeysHolder::addKeyPolicy(const std::string& key, const std::string& value, TPolicy policy)
{
	_Keys[key] = CKeyValue(value, policy);
}

void CKeysHolder::setDefaultKey(const std::string& defaultKey)
{
	_CurrentKey = defaultKey;
}

std::string CKeysHolder::getCurrentSignature(const std::string& md5) const
{
	return getSignature(md5, _CurrentKey);
}

std::string CKeysHolder::getSignature(const std::string& md5, const std::string& key) const
{
	if (md5.empty() && key.empty()) { return "";}

	TKeys::const_iterator found(_Keys.find(key));
	if (found == _Keys.end())
	{
		return "";
	}
	std::string tmp = md5;
	tmp += found->second.PrivateKeyValue;
	std::string wantedSignature = NLMISC::CHashKeyMD5(NLMISC::getMD5((uint8*)tmp.data(), (uint32)tmp.size())).toString();
	return key + ":" + wantedSignature;

}

bool CKeysHolder::verifyKey(const std::string& md5, const std::string& signature)
{

	std::string key;

	if ( !signature.empty() )
	{
		std::string::size_type pos = signature.find(":");
		if (pos > 0 && pos != std::string::npos)
		{
			key = signature.substr(0, pos);
		}
	}
	TKeys::const_iterator found(_Keys.find(key));

	if (found == _Keys.end())
	{
		found = _Keys.find("DEFAULT");
	}
	// Look default policy

	if (found == _Keys.end())
	{
		return false;
	}

	switch(found->second.Policy)
	{
		case Accept: return true;
		case Refuse: return false;
		case Test:
			{
				std::string wantedSignature = getSignature(md5, key);
				return wantedSignature == signature;
			}
		default:
			return false;
	}

	return false;
}


//----------------------------------------------- <CIdRecycle> -----------------------------------------------------------------------------------
uint32 CIdRecycle::newId()
{
	uint32 ret;
	// recycle instance
	if (!_RecycledId.empty())
	{
		// use *New* Ai

		std::set<uint32>::iterator it = _RecycledId.begin();
		ret = (*it);
		nldebug("Use *recyled* AI Instance: %u", ret );
		_RecycledId.erase(it);
	}
	// used maxUsed Instance + 1
	else if (!_UsedId.empty())
	{
		std::set<uint32>::iterator it = _UsedId.end();
		--it;
		ret = (*it) + 1;
		nldebug("Use new AI Instance: %u", ret );
	}
	else // first Instances are static instance don't use them;
	{
		ret = _InitialValue;
	}
	_UsedId.insert(ret);
	return ret;
}

void CIdRecycle::releaseId(uint32 id)
{

	_RecycledId.insert(id);

	std::set<uint32>::iterator used = _UsedId.find(id);
	if (used == _UsedId.end())
	{
		nlwarning("Error id released 2 time?");
		nlassert(0 && "Error id released 2 time?");
		return;
	}

	_UsedId.erase(  used );
	// If Recycle id are bigger thant used id then remove it (int order to decrese the size of container)
	// eg; Used = { 2, 6} Recyled = {1,3, 4, 5} => if 6 removed => Used {2}, Recycled = {1}
	//
	uint32 maxUsed = 0;
	if (!_UsedId.empty())
	{
		std::set<uint32>::iterator lastUsed = _UsedId.end();
		--lastUsed;
		maxUsed = *lastUsed;
	}
	std::set<uint32>::iterator  upper( _RecycledId.upper_bound(maxUsed)), last(_RecycledId.end());

	if (upper != last)
	{
		_RecycledId.erase(upper, last);
	}
}



//------------------------------------- <CEditionSession> ---------------------------------------------------------------------------------------
void CEditionSession::touch()
{
	DateSinceNoPlayer = 0;
}

bool CEditionSession::isLoaded() const
{

	return  _SessionType==RSMGR::TSessionType::st_anim ||  (_SessionType==RSMGR::TSessionType::st_edit && _Loaded);
}

void CEditionSession::setLoaded(bool value)
{
	nlassert(_SessionType==RSMGR::TSessionType::st_edit);
	nlassert(value == true);
	_Loaded = true;
	_WaitingForConnectionTasks.execute(0);
}

void CEditionSession::getStartParamsWithDelay(CServerEditionModule* module, uint32 charId, TSessionId lastStoredSessionId)
{
	_WaitingForConnectionTasks.addTask( new CTaskGetStartParamsWhenLoaded(module, charId, lastStoredSessionId));
}

void CEditionSession::disconnectChar(TCharId charId)
{
	TCurrentChars::iterator first(_CurrentChars.begin());
	TCurrentChars::iterator last(_CurrentChars.end());
	for(;first!=last;first++)
	{
		if( (*first)->getCharId() == charId )
		{

			CCurrentChar* cc = *first;
			_EditSlotIdMaker.releaseId(cc->getEditSlotId());
			_CurrentChars.erase(first);
			delete cc;
			break;
		}
	}
}



uint32 CEditionSession::connectChar(TCharId charId, TUserRole connectedAs)
{
	TCurrentChars::const_iterator first(_CurrentChars.begin()), last(_CurrentChars.end());
	for (; first != last; ++first)
	{
		if ((*first)->getCharId() == charId)
		{
			return (*first)->getEditSlotId();
		}
	}
	uint32 editSlotId = _EditSlotIdMaker.newId();
	_CurrentChars.push_back(new CCurrentChar(charId, editSlotId, connectedAs));
	return editSlotId;
}


uint32 CEditionSession::getEditSlotId(TCharId charId) const
{

	TCurrentChars::const_iterator first(_CurrentChars.begin()), last(_CurrentChars.end());
	for (; first != last && (*first)->getCharId() != charId ; ++first) {}
	if (first == last) return 0;

	R2::CCurrentChar* currentUser = (*first);
	return currentUser->getEditSlotId();
}


R2::TUserRole CEditionSession::getCharRole(TCharId charId) const
{

	TCurrentChars::const_iterator first(_CurrentChars.begin()), last(_CurrentChars.end());
	for (; first != last && (*first)->getCharId() != charId ; ++first) {}
	if (first == last) return TUserRole::invalid;

	R2::CCurrentChar* currentUser = (*first);
	return currentUser->getRole();
}

void CEditionSession::swap(CEditionSession& other)
{
	nldebug("pre  CEditionSession::swap() - ownerCharId=%u, aiInstanceId=%u, sessionId=%u", _OwnerCharId, _AiInstance, SessionId.asInt());

	NLMISC::CSmartPtr<CScenario> scenarioOther = other._Scenario;
	other._Scenario = _Scenario;
	_Scenario = scenarioOther;

	std::swap(_OwnerCharId, other._OwnerCharId);

	nldebug("post CEditionSession::swap() - ownerCharId=%u, aiInstanceId=%u, sessionId=%u", _OwnerCharId, _AiInstance, SessionId.asInt());
}

void CEditionSession::update(uint32 currentTime)
{

	if (getCurrentChars().size() == 0 && DateSinceNoPlayer == 0 )
	{
		DateSinceNoPlayer = currentTime;
	}

	if (getCurrentChars().size() != 0 && DateSinceNoPlayer != 0 )
	{
		DateSinceNoPlayer = 0;
	}

}


CEditionSession::CEditionSession(RSMGR::TSessionType sessionType, TSessionId sessionId, TCharId ownerCharId, TAiInstanceId aiInstanceId, CScenario* scenario ):
_Scenario(scenario), SessionId(sessionId), _AiInstance(aiInstanceId),_EditSlotIdMaker(1)
	{
	if (_Scenario.isNull())
	{
		_Scenario = new CScenario(0);
	}

	IsAnimationStopped = false;
	DateSinceNoPlayer = 0;
	_LastSaveTime = 0;
	_SessionType = sessionType;
	_OwnerCharId = ownerCharId;
	InitialX = InitialY = 0;
	InitialSeason = 0;
	_Loaded = false;
	nldebug("Creating new CEditionSession - ownerCharId=%u, aiInstanceId=%u, sessionId=%u", ownerCharId, aiInstanceId, sessionId.asInt());
}


CEditionSession::CEditionSession(RSMGR::TSessionType sessionType, TSessionId sessionId, CEditionSession* edit)
: SessionId(sessionId), _EditSlotIdMaker(1)
{
	IsAnimationStopped = false;
	DateSinceNoPlayer = 0;
	_LastSaveTime = 0;
	_AiInstance = edit->_AiInstance;
	_Scenario = edit->_Scenario;
	_SessionType = sessionType;
	_OwnerCharId = edit->_OwnerCharId;
	InitialX = edit->InitialX;
	InitialY = edit->InitialY;
	InitialSeason = edit->InitialSeason;
	_Loaded = edit->_Loaded;
	nldebug("Creating new CEditionSession - ownerCharId=%u, aiInstanceId=%u, sessionId=%u", _OwnerCharId, _AiInstance, sessionId.asInt());
}


CEditionSession::~CEditionSession()
{
	TCurrentChars::iterator first(_CurrentChars.begin()), last(_CurrentChars.end());
	for (;first != last ; ++first)
	{
		delete *first;
		*first = 0;
	}
	_CurrentChars.clear();
}


CScenario* CEditionSession::getScenario() const
{
	return _Scenario.getPtr();
}


bool CEditionSession::isNoCharConnected() const
{
	return getCurrentChars().empty();
	}


RSMGR::TSessionType CEditionSession::getSessionType() const
{

	return _SessionType;
}


TScenarioSessionType CEditionSession::getScenarioSessionType() const
{

	switch(_SessionType.getValue())
	{
		case RSMGR::TSessionType::st_edit: return st_edit;
		case RSMGR::TSessionType::st_anim: return st_anim;
		default:
			nlassert(0 && "Type of session not handle");
		return invalid;
	}
}


bool CEditionSession::mustClose(uint32 currentTime) const
{
	if (TimeBeforeAutoCloseAnimationSessionWithNoPlayer == 0) { return false; }

	if (DateSinceNoPlayer == 0) { return false; } //There is still player connected

	if (!_Scenario.getPtr()) { return false; } //Must not arrived

	if (getSessionType() != RSMGR::TSessionType::st_anim) { return false; } // close only animation session


	if ( currentTime > DateSinceNoPlayer + TimeBeforeAutoCloseAnimationSessionWithNoPlayer )
	{
		return true;
	}
	return false;
}


bool CEditionSession::mustSave(uint32 currentTime) const
{
	if (!DssUseBs) { return false; }

	if (TimeBeforeAutoSaveTimeEditionSession == 0) { return false; }

	if (!_Scenario.getPtr()) { return false; } //Must not arrived

	if (_SessionType != RSMGR::TSessionType::st_edit) { return false; } // backup only editing session

	if ( !isLoaded() ){ return false; }

	if (_LastSaveTime == 0) { return true; }

	if ( currentTime >_LastSaveTime + TimeBeforeAutoSaveTimeEditionSession  && !_Scenario->getClean())
	{
		return true;
	}
	return false;
}


void CEditionSession::saved(uint32 currentTime)
{
	_LastSaveTime = currentTime;
	if (_Scenario.getPtr()) { _Scenario->setClean(true); }
}

void CEditionSession::setAiInstanceId(TAiInstanceId aiInstanceId)
{
	this->_AiInstance = aiInstanceId;
}

CEditionSession::TAiInstanceId CEditionSession::getAiInstanceId() const
{
	return _AiInstance;
}

bool CEditionSession::mustStopAnimation(uint32 currentTime) const
{
	if (TimeBeforeStopTestInEditionSessionWithNoPlayer == 0) { return false; }

	if (DateSinceNoPlayer == 0) { return false; } //There is still player connected

	if (!_Scenario.getPtr()) { return false; } //Must not arrived

	if (_SessionType != RSMGR::TSessionType::st_edit || _Scenario->getMode() != 2) { return false; } // close only editing session in test mode

	if ( currentTime > DateSinceNoPlayer + TimeBeforeStopTestInEditionSessionWithNoPlayer)
	{
		return true;
	}
	return false;
}


bool CEditionSession::mustHibernate(uint32 currentTime) const
{
	if (!DssUseBs) { return false; }
	if (TimeBeforeAutoHibernateEditionSessionWithNoPlayer == 0)
	{
		return false;
	}

	if (DateSinceNoPlayer == 0) { return false; } //There is still player connected

	if (!_Scenario.getPtr()) { return false; } //Must not arrived

	if (_SessionType != RSMGR::TSessionType::st_edit ) { return false; } // hibernate only editing session


	if ( currentTime > DateSinceNoPlayer + TimeBeforeAutoHibernateEditionSessionWithNoPlayer )
	{
		return true;
	}
	return false;
}


uint32 CEditionSession::getLastSaveTime() const { return _LastSaveTime;}




using namespace R2;
NLNET_REGISTER_MODULE_FACTORY(CServerEditionModule, "ServerEditionModule");

static const uint32 FileVersion = 3;


void CEditionSession::serial(NLMISC::IStream& stream)
{

	uint32 version = FileVersion;
	stream.serial(version);
	stream.serial(SessionId);
	if (version >= 3)
	{
		stream.serial(_OwnerCharId);
	}

	if (!stream.isReading()) //Writing
	{

		CScenario* scenario = _Scenario.getPtr();
		nlassert(scenario);
		scenario->serial(stream);

	}
	else
	{

		if ( version >= 2 ) //There was a "Cost" in needed prop of CNpcGrpFeatureSerializer
		{
			_Scenario->serial(stream);
		}
		_Scenario->setMode(1);//go back in edition
	}

	nldebug("CEditionSession::serial(%s) - ownerCharId=%u, aiInstanceId=%u, sessionId=%u", (stream.isReading()?" IN":"OUT"), _OwnerCharId, _AiInstance, SessionId.asInt());
}


//------------------------------------- <ServerEditionModule::CPioneerInfo> ---------------------------------------------------------------------------------------
CServerEditionModule::CPioneerInfo::CPioneerInfo()
{
	WaitingTp = false;
	EgsReloadPos = false;
	Mode = TCharMode::Invalid;
	Newcomer = false;
	WaitingMsg = 0;
}

CServerEditionModule::CPioneerInfo::~CPioneerInfo()
{
	delete WaitingMsg;
}
//----------------------------------------------<CServerEditionModule>---------------------------------------------------------------
void CServerEditionModule::setLoaded(TSessionId sessionId)
{
	CEditionSession* session = getSession(sessionId);
	if (session)
	{
		session->setLoaded(true);
	}
}


bool CServerEditionModule::isEditingSession(TSessionId sessionId) const
{
	RSMGR::TSessionType sessionType;
	CEditionSession* session = getSession(sessionId);
	if (!session) { return false; }
	sessionType =  session->getSessionType();
	if (sessionType == RSMGR::TSessionType::st_edit)
	{
		return true;
	}
	return false;
}




CServerEditionModule::CServerEditionModule()
{
	CRingSessionManagerClientSkel::init(this);
	CShareServerEditionItfSkel::init(this);
}

CServerEditionModule::~CServerEditionModule()
{

	release();//

	TSessions::iterator first(_Sessions.begin()), last(_Sessions.end());
	for ( ;first != last ; ++first)
	{
		CEditionSession* session = first->second;
		//_IdRecycle is already deleted so don't do _IdRecycle->release(aiInstanceId);
		delete session;
	}
	_Sessions.clear();
	CRingAccess::releaseInstance();
}


CObjectFactory Factory("Server");



class COverrideRingAccessCallback : public IBackupFileReceiveCallback
{

public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;
public:
	COverrideRingAccessCallback(CServerEditionModule* editionModule) :_EditionModule(editionModule){}

	// call back for bs file asynchronous read
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		nldebug("BSIcb COverrideRingAccessCallback::callback() for file: %s",fileDescription.FileName.c_str());
		CServerEditionModule::TOverrideRingAccess access;
		parseStream(fileDescription, dataStream, access);
		_EditionModule->swapOverrideRingAccess(access);
	}

	void parseStream(const CFileDescription& fileDescription, NLMISC::IStream& dataStream, CServerEditionModule::TOverrideRingAccess & access)
	{
		CMemStream * memstream = dynamic_cast<CMemStream *>(&dataStream);
		if (!memstream) { return; }

		sint32 len = memstream->size();

		std::vector<std::string> entries;
		std::vector<std::string> tokens;

		uint8* buf = new uint8[len +1];
		dataStream.serialBuffer(buf, len);
		buf[len] = '\0';

		std::string data((const char*)buf);
		delete [] buf;

		NLMISC::splitString(data, "\n", entries);
		std::vector<std::string>::iterator first(entries.begin()), last(entries.end());

		bool firstLoop = true;
		for (; first != last ; ++first)
		{


			NLMISC::splitString(*first, "\t", tokens);

			if (firstLoop)
			{
				firstLoop = false;

				uint32 fileVersion;
				fromString(tokens[1], fileVersion);

				if (tokens.size() != 2 || tokens[0] != "Version" || fileVersion != FileVersion)
				{
					nlwarning("Obsolete File Discard '%s'", fileDescription.FileName.c_str());
					return;

				}

			}
			else
			{
				if (tokens.size() == 0)
				{

				}
				else if (tokens.size() == 2)
				{
					TCharId charId;
					fromString(tokens[0], charId);
					std::string overrideAccess = tokens[1];

					access[charId] = overrideAccess;
				}
				else
				{
					nlstop;
				}

			}


		}
	}

	CServerEditionModule* _EditionModule;
};
void CServerEditionModule::init(NLNET::IModuleSocket* gateway, CDynamicMapService* server)
{
	_BsGoingUp = true;
	_Server = server;
	_WaitingForBS = false;
	_MustUpdateHibernatingFileList = false;
	_IdRecycle.reset( new CIdRecycle(100) );
	this->plugModule(gateway);

	std::string filename = "version_name.txt";
	if ( NLMISC::CFile ::fileExists(filename) )
	{
		_VersionName.readFromFile(filename);
	}


	CObjectSerializerClient::setClientObjectFactory(&Factory);

	_KeysHolder = new CKeysHolder();
	_MustUpdateOverrideRingAcess = false;

	// Default dev policy: accept all scenario
	resetKeyPolicies(0);
	setDefaultKey(0, "DEV");
	addKeyPolicy(0, "DEFAULT", "EMPTY", "ACCEPT");  //Accept could come from live
	addKeyPolicy(0, "EMPTY", "EMPTY", AcceptScenarioWithoutHeader.get()?"ACCEPT":"REFUSE"); // Deprecated (scenario older than 22 juin 2006)
	addKeyPolicy(0, "DEV", "DEV:26289fe9502f4db1fa3229e4fb2d8803", "TEST");

	/* eg prod conf(value generated by cat /dev/random > toto; md5sum toto)
	setDefaultKey(0, "PROD");
	addKeyPolicy(0, "DEFAULT", "EMPTY", "REFUSE");
	addKeyPolicy(0, "EMPTY", "EMPTY", "REFUSE"); // Deprecated (scenario older than 22 juin 2006)
	addKeyPolicy(0, "PROD", "PROD:098085b10aa27157d57587464e3b2390 ", "TEST");
	addKeyPolicy(0, "DEV", "DEV:26289fe9502f4db1fa3229e4fb2d8803", "TEST"); // accept current dev key
	addKeyPolicy(0, "DEVOBSOLETE", "DEVOBSOLETE:26289fe9502f4db1fa3229e4fb2d8803", "REFUSE");  // refuse obsolete dev key
	*/




}


void CServerEditionModule::updateRSMGR()
{
	nlassert(!_SessionManager.isNull());
	nlassert(!_WaitingForBS);

	RSMGR::CRingSessionManagerProxy rsm(_SessionManager);

	vector<RSMGR::TRunningSessionInfo> runningSessions;
	runningSessions.resize(_Sessions.size());

	vector<RSMGR::TRunningSessionInfo>::iterator runningSession( runningSessions.begin() );

	{
		TSessions::const_iterator first(_Sessions.begin()), last(_Sessions.end());
		for (; first != last; ++first, ++runningSession)
		{
			runningSession->setSessionId(first->first);
			runningSession->setInstanceId(first->second->getAiInstanceId());
			runningSession->setNbPlayingChars((uint32)first->second->getCurrentChars().size());
			runningSession->setSessionType(first->second->getSessionType());
		}

	}

	rsm.registerDSS(this, IService::getInstance()->getShardId(), runningSessions);
}




void CServerEditionModule::simulateSU(CEntityId clientEid)
{


	if (!CDynamicMapService::getInstance()->useNetwork())
	{
		TCharId charId = static_cast<TCharId>(clientEid.getShortId());
		createSessionWithoutSu(charId, clientEid);
		return;
	}

	if (DontUseSU)
	{
		TCharId charId = static_cast<TCharId>(clientEid.getShortId());
		CEditionSession* editionSession = getSessionByCharId(charId);
		if (!editionSession) { return; }
		CMessage msgout("SET_CHAR_AIINSTANCE");
		msgout.serial(clientEid);
		TAiInstanceId aiInstanceId = editionSession->getAiInstanceId();
		msgout.serial(aiInstanceId);
		CUnifiedNetwork::getInstance()->send("EGS",msgout);
	}

}


void CServerEditionModule::onModuleUp(NLNET::IModuleProxy *senderModuleProxy)
{
	const std::string &moduleName = senderModuleProxy->getModuleClassName();
	if (moduleName == "RingSessionManager")
	{
		nlassert(_SessionManager == NULL);
		_SessionManager = senderModuleProxy;
		if (!_WaitingForBS){ updateRSMGR(); }
	}
	else if ( moduleName == "CharacterControl")
	{
		_CharacterControlProxy = senderModuleProxy;
		CCharacterControlItfProxy proxy(_CharacterControlProxy);
		// add all editor and animator into the universe chat
		TSessions::iterator first(_Sessions.begin()), last(_Sessions.end());
		for (; first != last; ++first)
		{
			CEditionSession *session = first->second;

			CEditionSession::TCurrentChars::const_iterator f2(session->getCurrentChars().begin()), e2(session->getCurrentChars().end());
			for (; f2 != e2; ++f2)
			{
				const CCurrentChar *cc = *f2;
				if (cc->getRole() == TUserRole::ur_animator
					|| cc->getRole() == TUserRole::ur_editor
					|| cc->getRole() == TUserRole::ur_outland_owner)
				{
					// add this guy in the universe chat
					proxy.subscribeCharacterInRingUniverse(this, cc->getCharId());
				}
			}
		}
	}
	else if (moduleName == "ServerAnimationModule")
	{
		_ServerAnimationProxy = senderModuleProxy;
	}
	else if (moduleName == "ServerAdminModule")
	{
		_ServerAdminProxy = senderModuleProxy;
	}
	else if (moduleName == "SessionBackupModule")
	{
		_R2SessionBackupModule = senderModuleProxy;
		CR2SessionBackupModuleItfProxy proxy(_R2SessionBackupModule);
		proxy.registerDss(this, IService::getInstance()->getShardId());
	}
	else if (moduleName == "IOSRingModule")
	{
		_IOSRingProxy = senderModuleProxy;
	}
	else if (moduleName == "ClientEditionModule")
	{
		// send back a message to the client to open the firewall
		TCharId charId;
		CEntityId clientEid;
		std::string userPriv;
		std::string extendedPriv;


		bool ok = getCharInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);


		simulateSU(clientEid);


		nldebug("R2Ed: onModuleUp( ClientEditionModule %u '%s' '%s'", charId, clientEid.toString().c_str(), userPriv.c_str());

		if (!ok)
		{
			nlwarning("R2Ed: client module '%s' try to connect but has Invalid Security Info", senderModuleProxy->getModuleName().c_str());
			// we won't kick him because we don't have its charId
			return;
		}

		if (_ClientsEditionModule.getA(senderModuleProxy) != NULL)
		{
			nlwarning("R2Ed: The client edition module '%s' just got up but was already identified", senderModuleProxy->getModuleName().c_str());
			_ClientsEditionModule.removeWithB(senderModuleProxy);
		}

		if (_ClientsEditionModule.getB(charId) != NULL)
		{
			nlwarning("R2Ed: The client edition module '%s' just got up but was already there. It means that one charId %u has 2 Modules... We can not tolerate this: so we kick him.", senderModuleProxy->getModuleName().c_str(), charId);

			disconnectChar(charId);

			if (_ClientsEditionModule.getB(charId) != NULL)
			{
				_ClientsEditionModule.removeWithA(charId);
			}

			return;
		}



		_ClientsEditionModule.add(charId, senderModuleProxy);
		_PioneersInfo[charId].EntityId = clientEid;

		_Tasks.addTask( new CTaskTryConnectPlayer(this, charId) );

	}
}


void CServerEditionModule::tryCharConnection(TCharId charId)
{
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	const NLNET::TModuleProxyPtr*senderModuleProxyPtr = getClientProxyPtr(charId);
	if (!senderModuleProxyPtr){ return;} // player disconnect before BS an EGS Up
	NLNET::IModuleProxy *senderModuleProxy = *senderModuleProxyPtr;
	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }
	// Simulate SU if needed


	TPioneersSessionsAlloweds::iterator found(_PioneersSessionsAllowed.find(charId));

	if (found == _PioneersSessionsAllowed.end() )
	{
		nlwarning("R2Ed: The char %u is trying to connect but is not allowed so we was kicked", charId);
		disconnectChar(charId); // kick the player out of dss0
		returnToPreviousSession(charId);
		return;
	}

	TSessionId sessionId = found->second.SessionId;
	TUserRole role = found->second.Role;
	std::string ringAccess = found->second.RingAccess;
	bool egsReloadPos = found->second.EgsReloadPos;
	bool newcomer = found->second.Newcomer;
	_PioneersSessionsAllowed.erase(found);

	_PioneersInfo[charId].EgsReloadPos = egsReloadPos;



	if ( !getSession(sessionId))
	{
		nlwarning("R2Ed: The char %u is trying to connect to session %u but this session does not exist (he was kicked)", charId, sessionId.asInt() );
		disconnectChar(charId); // kick the player out of dss0
		returnToPreviousSession(charId);
		return;
	}
	connectChar(sessionId, charId, role, ringAccess, newcomer);
}



void CServerEditionModule::connectChar(TSessionId sessionId, TCharId charId, TUserRole userRole, const std::string& ringAccess, bool newcomer)
{
	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
	if (!clientEditionProxyPtr) { return; } // client is down
	NLNET::TModuleProxyPtr clientEditionProxy = (*clientEditionProxyPtr);
	// Open Firewall
	{

		CMessage msg("HELLO");
		clientEditionProxy->sendModuleMessage(this, msg);
	}

	{


		CMessage msg("HELLO");
		clientEditionProxy->sendModuleMessage(this->getAnimationModule()->getModule(), msg);
	}
	_PioneersInfo[charId].RingAccess = CSString(ringAccess).toLower();
	_PioneersInfo[charId].Newcomer = newcomer;

	if (_SessionManager)
	{
	// Signal client connection to SU
		RSMGR::CRingSessionManagerProxy rsm(_SessionManager);
		rsm.reportSessionEvent(this, RSMGR::TSessionEvent::se_char_enter, sessionId, charId);
	}
	// Upload to client map, data ...

	/*
	Remap session
	*/




	// If its a dev he has specials ring Acess
	{
		TCharId charId;
		CEntityId clientEid;
		std::string userPriv;
		std::string extendedPriv;

		// Do not useeckSecurityInfo because we put the client in authorized list here.
		bool ok = getCharInfo(clientEditionProxy, charId, clientEid, userPriv, extendedPriv);
		if (ok)
		{
			std::vector<std::string> values;
			splitString(extendedPriv, ":", values);
			static const std::string r2a("R2A_");
			static const std::string::size_type length = r2a.size();

			std::vector<std::string>::const_iterator first( values.begin()), last(values.end());
			for ( ;first != last; ++first)
			{
				if ( first->size() >= length && first->substr(0, length) == r2a)
				{

					std::string ra = CSString(first->substr(length) ).replace("_", ":").toLower();
					_PioneersInfo[charId].RingAccess = ra;
					break;
				}
			}

		}
	}

	onMapConnectionAsked(clientEditionProxy, sessionId, true, true, userRole);


}


void CServerEditionModule::disconnectChar(TCharId charId)
{
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();
	bool alreadyKicked  = !_Kicked.insert(std::make_pair(charId, now)).second;

	if (alreadyKicked)
	{
		return;
	}
	// No double kick



	nldebug("Disconnect/Kick  char %u", charId);

	// manually disconnect client from edition module
	const NLNET::TModuleProxyPtr* client = getClientProxyPtr(charId);
	if (client)
	{
		// Last world
		CShareClientEditionItfProxy proxy(*client);
		proxy.onDisconnected(this);
		onModuleDown(*client); // manually disconnect client
	}
	if (getAnimationModule())
	{
		// manually disconnect client from animation module
		getAnimationModule()->disconnectChar(charId);
	}

	// Will he receive the message?
	DROP_IF(_CharacterControlProxy.isNull(), "Try to disconnect a player but no EGS?", return);
	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.returnToPreviousSession(this, charId);
	//proxy.disconnectChar(this, charId); // removed because prevents the client from completing its FarTP to the previous session
	// Kick the client out of game

}

void CServerEditionModule::createSessionWithoutSu(TCharId charId, NLMISC::CEntityId clientEid)
{
			// Simulate SU : createSession, put char in session, set the AIInstance of the player
	bool mustCreateSession = AutoCreateSessionMultiChar || AutoCreateSession || !CDynamicMapService::getInstance()->useNetwork();
	TSessionId sessionId  = TSessionId(0);
	if (mustCreateSession || AutoCreateAddCharacter)
	{

		nlwarning("Creating session without SU - this code is dangerous when running with SU!!!");
		sessionId = TSessionId(charId);
		if (AutoCreateSessionMultiChar)
		{
			sessionId = TSessionId(AutoCreateSessionMultiCharSessionId);
		}

		if (mustCreateSession)
		{
			TSessions::iterator scenarioIt = _Sessions.find(sessionId);
			if (scenarioIt == _Sessions.end())
			{
				createSession(_SessionManager, charId, sessionId, AutoCreateAnimationSession ? RSMGR::TSessionType::st_anim: RSMGR::TSessionType::st_edit);
			}
		}

		CEditionSession* editionSession = getSession(sessionId);

		if (!editionSession)
		{
			return;
		}

		addCharacterInSession(_SessionManager, sessionId, charId, WS::TUserRole(AutoCreateRole), "", false);


	}


}


void CServerEditionModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	std::string moduleName = moduleProxy->getModuleClassName();

	if (moduleName == "RingSessionManager")
	{
		if (_SessionManager == moduleProxy)
			_SessionManager = NULL;
	}
	else if (moduleName == "ServerAnimationModule")
	{
		_ServerAnimationProxy = NULL;
	}
	else if (moduleName== "ServerAdminModule")
	{
		_ServerAdminProxy = NULL;
	}
	else if (moduleName == "CharacterControl")
	{
		_CharacterControlProxy = NULL;
	}
	else if (moduleName == "SessionBackupModule")
	{
		_R2SessionBackupModule = NULL;
	}
	else if (moduleProxy == _IOSRingProxy)
	{
		_IOSRingProxy = NULL;
	}
	else if (moduleName == "ClientEditionModule")
	{
		// we lost a client editor
		if (_ClientsEditionModule.getA(moduleProxy) != NULL)
		{
			_ClientsEditionModule.removeWithB(moduleProxy);

			TCharId charId;
			CEntityId clientEid;
			std::string userPriv;
			std::string extendedPriv;

			// Do not useeckSecurityInfo because we put the client in authorized list here.
			bool ok = getCharInfo(moduleProxy, charId, clientEid, userPriv, extendedPriv);


			BOMB_IF(!ok, "R2Ed: User Module '"<<moduleProxy->getModuleName()<<"' is disconnecting but it does not have any security info", return);
			TSessionId sessionId = getSessionIdByCharId(charId);
			CEditionSession* session = getSession(sessionId);
			if (session)
			{
				session->disconnectChar(uint32(clientEid.getShortId()));

				// Signal client disconnection to SU
				if (!_SessionManager.isNull())
				{
					RSMGR::CRingSessionManagerProxy rsm(_SessionManager);
					rsm.reportSessionEvent(this, RSMGR::TSessionEvent::se_char_leave, session->SessionId.asInt(), charId);
				}
				if (session->getSessionType() == RSMGR::TSessionType::st_anim && _CharacterControlProxy)
				{
					CCharacterControlItfProxy proxy(_CharacterControlProxy);
					proxy.charLeaveAnimSession(this, charId, sessionId);
				}
				// always send a remove DM to the IOS (even if the char is not DM now)
				// ok, he owns
				if (_IOSRingProxy != NULL)
				{
					CCharacterControlItfProxy proxy(_CharacterControlProxy);
					proxy.unsubscribeCharacterInRingUniverse(this, charId);
				}
			}

//			// Don't remove from _PioneersSessionsAllowed otherwise joining a distinct session
//			// on the same DSS will fail.
//			TPioneersSessionsAlloweds::iterator foundPioneersSessionsAllowed(_PioneersSessionsAllowed.find(userId));
//			if (foundPioneersSessionsAllowed != _PioneersSessionsAllowed.end() )
//			{
//				_PioneersSessionsAllowed.erase(foundPioneersSessionsAllowed);
//			}

			TPioneersSessions::iterator foundPioneersSessions(_PioneersSessions.find(charId));
			if (foundPioneersSessions != _PioneersSessions.end() )
			{
				_PioneersSessions.erase(foundPioneersSessions);
			}


			TPionnerInfos::iterator foundPioneersEId= _PioneersInfo.find(charId);
			if (foundPioneersEId != _PioneersInfo.end())
			{
				_PioneersInfo.erase(foundPioneersEId);
			}

		}
	}
}


void CServerEditionModule::rtScenarioUpdateRequested(NLNET::IModuleProxy *senderModuleProxy, uint32 paramCharId, CObject* rtScenario)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	CScenario* scenario = getScenarioByCharId(charId);
	if (!scenario)
	{
		nlwarning("R2Ed: char %u is trying to act on an invalid scenario", charId);
		return;
	}
	CObject* value2 = rtScenario->clone();
	scenario->setRtData(value2);
}


void CServerEditionModule::onMessageReceivedCreatePrimitives(NLNET::IModuleProxy *senderModuleProxy, TCharId charId)
{

	CScenario* scenario = getScenarioByCharId(charId);
	if (!scenario)
	{
		nlwarning("R2Ed: char %u is trying to act on an invalid scenario", charId);
		return;
	}

	CObject* rtData = scenario->getRtData();
	if (!rtData)
	{
		nlwarning("R2Ed: char %u is trying to create primitives but there is no Rt Data", charId);
		return;
	}

	TSessionId sessionId = getSessionIdByCharId(charId);

	CMessage message("DBG_CREATE_PRIMITIVES");
	message.serial(sessionId);
	CObjectSerializerServer obj(rtData);
	message.serial(obj);
	_ServerAnimationProxy->sendModuleMessage(this, message);
}


void CServerEditionModule::stopTest(TSessionId sessionId)
{

	CEditionSession* session = getSession(sessionId);
	if (!session)
	{
		nlwarning("R2Ed: trying to stop a invalid session %u", sessionId.asInt());
		return;
	}

	CScenario* scenario = session->getScenario();
	session->getScenario()->setMode(1);

	uint32 lastCurrentActIndex = scenario->getInitialActIndex();
	bool ok = getAnimationModule()->stopTest(sessionId, lastCurrentActIndex);

	if ( ! ok )
	{
		nlinfo("INF: The destruction of an CAnimationSession in the session %u was (delayed) ", sessionId.asInt() );
	}

	scenario->setInitialActIndex(lastCurrentActIndex);

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVector(session, broadcastList);
	CShareClientEditionItfProxy::broadcast_onTestModeDisconnected(broadcastList.begin(), broadcastList.end(), this,
		sessionId, lastCurrentActIndex, session->getScenarioSessionType() );

}

void CServerEditionModule::stopTestRequested(NLNET::IModuleProxy *senderModuleProxy, TCharId charId)
{

	TSessionId sessionId = getSessionIdByCharId(charId);
	stopTest(sessionId);
}


void CServerEditionModule::onMapConnectionAsked( NLNET::IModuleProxy * clientEditionProxy,
	TSessionId sessionId, bool updateHighLevel, bool mustTp, TUserRole role)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;
	bool mustForceVisionUpdate = true;

	bool ok = checkSecurityInfo(clientEditionProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }


	CEditionSession* session = getSession(sessionId);
	CScenario* scenario = session ? session->getScenario() : 0;

	TSessionId previousSessionId = TSessionId(0);
	if (!session ||!scenario )
	{
		return;
	}

	TPioneersSessions::iterator previousSessionIdIt = _PioneersSessions.find(charId);
	if (previousSessionIdIt != _PioneersSessions.end())
	{
		previousSessionId = previousSessionIdIt->second;

		if ( previousSessionId != sessionId)
		{
			CEditionSession* previousSession= getSession(previousSessionId);
			previousSession->disconnectChar(charId);
			previousSessionIdIt->second = sessionId;
			nldebug("R2Ed: char %u that was connected to %u is connecting to %u", charId, previousSessionId.asInt(), sessionId.asInt());
			_PioneersSessions[charId] = sessionId;
			if ( scenario == previousSession->getScenario())
			{// Same HL data between an edition Session and its linked animation session
				mustTp = false;
				updateHighLevel = false;
			}

		}
	}
	else
	{
		_PioneersSessions[charId] = sessionId;
	}




	if (clientEditionProxy)
	{
		CMessage message("ADV_CONN");
		CClientMessageAdventureUserConnection clientConnectBody;

		//uint32 ConnectionState;
		clientConnectBody.RingAccess = _PioneersInfo[charId].RingAccess;
		clientConnectBody.InCache = ! updateHighLevel;
		if (updateHighLevel)
		{
			clientConnectBody.HighLevel.setData( scenario->getHighLevel() );
		}

		uint32 editSlotId = 0;
		bool isInPlay = true;

		bool isSessionOwner = charId == session->getOwnerCharId();

		RSMGR::TSessionType sessionType = session->getSessionType();

		if (sessionType == RSMGR::TSessionType::st_anim)
		{
			if (!scenario->getHighLevel() ) //Session Active
			{

				if (role == TUserRole::ur_animator)
				{
				}
				else if (role == TUserRole::ur_player)
				{

				}
				else
				{
					nlwarning("Wrong role %u", role.getValue());
					role = TUserRole::ur_animator;
				}


				if (isSessionOwner)
				{
					clientConnectBody.Mode = 0;
				}
				else
				{
					clientConnectBody.Mode = 1;
				}
			}
			else // //Session not active
			{

				if (role == TUserRole::ur_animator)
				{
					clientConnectBody.Mode = 2;
				}
				else if (role == TUserRole::ur_player)
				{
					clientConnectBody.Mode = 3;
				}
				else
				{
					nlwarning("Wrong role %u", role.getValue());
					clientConnectBody.Mode = 2;
					role = TUserRole::ur_animator;
				}
				mustTp = true;

			}
			editSlotId = session->connectChar(charId, role);
			//clientConnectBody.InitialActIndex = scenario->getInitialActIndex();
			uint32 actId = getAnimationModule()->getCurrentAct(sessionId);
			clientConnectBody.InitialActIndex = actId;
			clientConnectBody.EditSessionLink = getLinkedSession(session);
		}
		else if (sessionType == RSMGR::TSessionType::st_edit)
		{
			bool animationSessionExist = false;

			// If player reconnect after test mode the test mode is stopped
			// Add charId connected in animation Module
			{
				std::vector<TCharId> connectedChars;
				animationSessionExist = getAnimationModule()->getConnectedChars(sessionId, connectedChars);
			}

			//(*) Remove animation Session and kill AI Instance
			if (animationSessionExist)
			{

			//	stopTest(sessionId);
				scenario->setMode(1);
			}
			else
			{
				scenario->setMode(0);
			}


			editSlotId = session->connectChar(charId, TUserRole::ur_editor);
			clientConnectBody.Mode = scenario->getMode();
			clientConnectBody.InitialActIndex = scenario->getInitialActIndex();
			clientConnectBody.EditSessionLink = TSessionId(0);

			//	if ( clientConnectBody.Mode == 1) { isInPlay = false; }
		}


		nlinfo("R2Ed: char %u is identify as %u", charId, editSlotId);

		clientConnectBody.EditSlotId = editSlotId;
		clientConnectBody.SessionId= sessionId;
		clientConnectBody.SessionType = session->getScenarioSessionType();
		clientConnectBody.VersionName = _VersionName;

		clientConnectBody.HighLevel.compress();
		clientConnectBody.MustTp = mustTp;
		clientConnectBody.AiInstance = session->getAiInstanceId();
		clientConnectBody.IsSessionOwner = isSessionOwner;


		// TP only if data

		if ( !_PioneersInfo[charId].EgsReloadPos )
		{
			if (mustTp && clientConnectBody.HighLevel.getData() )
			{
				tpToEntryPoint(clientEditionProxy, clientConnectBody.InitialActIndex);
				mustForceVisionUpdate = false;
			}
		}
		else
		{
	//		_PioneersInfo[charId].EgsReloadPos = true;

		}



		message.serial(clientConnectBody);
		clientEditionProxy->sendModuleMessage(this, message);


		// wait ack from client to launch tp
		_PioneersInfo[charId].WaitingTp = mustTp;

		//send a message to indicates some values like maximum of npcs
		CShareClientEditionItfProxy proxy(clientEditionProxy);
		proxy.onQuotaUpdated(this, MaxNpcs, MaxStaticObjects);



		// Special bugged case
		if (role == TUserRole::ur_animator)
		{
			onCharModeUpdateAsked(clientEditionProxy,  TCharMode::Dm);
		}
		else if (role == TUserRole::ur_editor)
		{
			onCharModeUpdateAsked(clientEditionProxy,  TCharMode::Editer );
			proxy.onCharModeUpdated(this, TCharMode::Editer );
			mustForceVisionUpdate = false;
		}
		else if (role == TUserRole::ur_player)
		{
			onCharModeUpdateAsked(clientEditionProxy, TCharMode::Player);
		}

		if (isInPlay)
		{
			getAnimationModule()->addPioneer( sessionId, charId);
		}

		// Only happends in test when we are doing edition->test
		if (_CharacterControlProxy && previousSessionId.asInt() != 0 && previousSessionId != sessionId)
		{
			double x, y, orient;
			uint8 season;
			_Server->getAdminModule()->getPosition(sessionId, x ,y, orient, season, clientConnectBody.InitialActIndex);
			CFarPosition pos;
			pos.SessionId = sessionId;
			pos.PosState.X = (sint32)(x * 1000.0);
			pos.PosState.Y = (sint32)(y * 1000.0);
			pos.PosState.Z = 0;
			pos.PosState.Heading = 0;

			CCharacterControlItfProxy proxy(_CharacterControlProxy);
			proxy.setUserCharCurrentSession(this, charId, previousSessionId, pos, role);
		}


		if (mustForceVisionUpdate)
		{
			CMessage msgout("R2_VISION_REFRESH");
			msgout.serial(clientEid);
			CUnifiedNetwork::getInstance()->send("GPMS",msgout);
		}

	}




}


void CServerEditionModule::advConnACK(NLNET::IModuleProxy *senderModuleProxy)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	nlassert(_PioneersSessions.count(charId));
	nlassert(_PioneersInfo.count(charId));

	if (!_PioneersInfo[charId].WaitingTp) return;
	_PioneersInfo[charId].WaitingTp = false;

}


void CServerEditionModule::broadcastToCharConnected(const NLNET::CMessage & msg, CEditionSession* session)
{
	nlassert(session);

	// Warning : do not set this vector 'static' because it will hold smart pointer on module
	vector <NLNET::TModuleProxyPtr> broadcastList;

	fillBroadcastVector(session, broadcastList);

	vector <NLNET::TModuleProxyPtr>::iterator first(broadcastList.begin()), last(broadcastList.end());
	for (;first != last ; ++first)
	{
		NLNET::TModuleProxyPtr proxy = *first;
		proxy->sendModuleMessage(this, msg);
	}
}


void CServerEditionModule::fillBroadcastVector(CEditionSession * session, std::vector<NLNET::TModuleProxyPtr> &broadcastList)
{
	nlassert(session);

	// cleanup the list
	broadcastList.clear();

	CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
	for (;first != last ; ++first)
	{
		TCharId charId = (*first)->getCharId();
		const NLNET::TModuleProxyPtr* ptrClientEditionProxyPtr = getClientProxyPtr(charId);
		if (!ptrClientEditionProxyPtr)
		{
			nlwarning("R2Ed: connection broken for char %u", charId );
			continue;
		}
		TModuleProxyPtr clientEditionProxy = *ptrClientEditionProxyPtr;
		broadcastList.push_back(clientEditionProxy);
	}
}

void CServerEditionModule::fillBroadcastVectorNoSender(CEditionSession * session, std::vector<NLNET::TModuleProxyPtr> &broadcastList, NLNET::IModuleProxy *senderModuleProxy)
{
	nlassert(session);

	// cleanup the list
	broadcastList.clear();

	CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
	for (;first != last ; ++first)
	{
		TCharId charId = (*first)->getCharId();
		const NLNET::TModuleProxyPtr* ptrClientEditionProxyPtr = getClientProxyPtr(charId);
		if (!ptrClientEditionProxyPtr)
		{
			nlwarning("R2Ed: connection broken for char %u", charId );
			continue;
		}
		TModuleProxyPtr clientEditionProxy = *ptrClientEditionProxyPtr;
		if (! senderModuleProxy || senderModuleProxy->getModuleProxyId() != clientEditionProxy->getModuleProxyId())
		{
			broadcastList.push_back(clientEditionProxy);
		}

	}
}



void CServerEditionModule::replyToAll(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage & msg)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	CEditionSession* session = getSessionByCharId(charId);
	if (!session)
	{
		nlwarning("R2Ed: error char %u not connected to a session", charId);
		return;
	}

	broadcastToCharConnected(msg, session);
}


std::vector<NLNET::TModuleProxyPtr> CServerEditionModule::removeSender(const std::vector<NLNET::TModuleProxyPtr>& allModule, NLNET::IModuleProxy *senderModuleProxy )
{
	std::vector<NLNET::TModuleProxyPtr> ret;
	vector<TModuleProxyPtr>::const_iterator first(allModule.begin()), last(allModule.end());
	for( ;first != last; ++first)
	{
		if (!senderModuleProxy || (*first)->getModuleProxyId() != senderModuleProxy->getModuleProxyId())
		{
			ret.push_back(*first);
		}
	}
	return ret;
}


void CServerEditionModule::onScenarioUploadAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 msgId,
	const CObjectSerializerServer& hlScenario, bool mustBroadCast)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }
	CShareClientEditionItfProxy sender(senderModuleProxy);


	CScenario* scenario = getScenarioByCharId(charId);
	if (!scenario)
	{
		nlwarning("R2Ed: char %u trying to act on an invalid scenario", charId);
		sender.ackMsg(this, msgId, false);
		return;
	}

	CObject* scenarioHl = hlScenario.getData();
	if (!scenarioHl)
	{
		nlwarning("R2Ed: char %u requested to upload an invalid scenario", charId);
		sender.ackMsg(this, msgId, false);
		return;
	}

	TPioneersSessions::const_iterator found(_PioneersSessions.find(charId));
	if (found == _PioneersSessions.end())
	{
		nlwarning("R2Ed: char %u is trying to create a scenario but he is not in an instance", charId);
		sender.ackMsg(this, msgId, false);
		return;
	}

	TSessionId sessionId = found->second;

	scenario->setHighLevel( scenarioHl );
	getSession(sessionId)->RingAccess = "";
	scenario->setInitialActIndex(1);
	if (mustBroadCast)
	{
		vector<TModuleProxyPtr> broadcastList;
		fillBroadcastVector(getSession(sessionId), broadcastList);

		// no need  to send the same msg that the sender has sent. Just send an ack OK.
		sender.ackMsg(this, msgId, true);

		// Send the update message to all client except the one that send intial message
		vector<TModuleProxyPtr> otherThantSender = removeSender(broadcastList, senderModuleProxy);

		// Warning : Very BAD reinterpret cast due to necessity to convert a server to client object serial.
		// This should not harm because the two objects are completely similar.
		CShareClientEditionItfProxy::broadcast_onScenarioUploaded(otherThantSender.begin(), otherThantSender.end(),
			this,
			reinterpret_cast<const CObjectSerializerClient&>(hlScenario));

		vector<TModuleProxyPtr>::const_iterator first(broadcastList.begin()), last(broadcastList.end());
		for( ;first != last; ++first)
		{
			tpToEntryPoint(*first, scenario->getInitialActIndex());
		}

	}


}


void CServerEditionModule::onNodeInsertAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
	const std::string& instanceId, const std::string & attrName, sint32 position,
	const std::string& key, const CObjectSerializerServer& value2)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	value2.uncompress();
	CObject* value = value2.getData();

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId) ;
	CShareClientEditionItfProxy sender(senderModuleProxy);
	if (!session)
	{
		nlwarning("R2Ed: char %u insert node an empty scenario %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}

	ok = session->getScenario()->insertNode(instanceId, attrName, position, key, value);
	if (!ok)
	{
		nlwarning("RE2D: node insertion refused");
		sender.ackMsg(this, messageId, false);
		return;
	}

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVectorNoSender(session, broadcastList, senderModuleProxy);
	sender.ackMsg(this, messageId, true);
	// Warning : Very BAD reinterpret cast due to necessity to convert a server to client object serial.
	// This should not harm because the two objects are completely similar.
	CShareClientEditionItfProxy::broadcast_onNodeInserted(broadcastList.begin(), broadcastList.end(),
		this,
		instanceId, attrName, position, key, reinterpret_cast<const CObjectSerializerClient&>(value2));
}


void CServerEditionModule::onNodeSetAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
	const std::string& instanceId, const std::string & attrName, const CObjectSerializerServer& value2)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	value2.uncompress();
	CObject* value = value2.getData();
	std::auto_ptr<CObject> autoDelete(value);


	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);

	CEditionSession* session = getSession(sessionId);
	CScenario* scenario = session ? session->getScenario() : 0;
	CShareClientEditionItfProxy sender(senderModuleProxy);
	if (!session)
	{
		nlwarning("R2Ed: char %u insert node an empty session %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}
	// don't take ownership
	ok = scenario->setNode(instanceId, attrName, value);
	if (!ok)
	{
		nlwarning("RE2D: 'setNode' refused char %u, session %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}

	vector<TModuleProxyPtr> broadcastList;
	sender.ackMsg(this, messageId, true);
	fillBroadcastVectorNoSender(session, broadcastList, senderModuleProxy);
	CShareClientEditionItfProxy::broadcast_onNodeSet(broadcastList.begin(), broadcastList.end(),
		this,
		instanceId, attrName, reinterpret_cast<const CObjectSerializerClient&>(value2));
}


void CServerEditionModule::onNodeEraseAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
	const std::string& instanceId, const std::string& attrName, sint32 position)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }


	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId);

	CShareClientEditionItfProxy sender(senderModuleProxy);
	if (!session)
	{
		nlwarning("R2Ed: char %u insert node an empty scenario %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}
	CScenario* scenario = session->getScenario();
	ok = scenario->eraseNode(instanceId, attrName, position);
	if (!ok)
	{
		nlwarning("RE2D: 'eraseNode' refused char %u, session %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVectorNoSender(session, broadcastList, senderModuleProxy);
	sender.ackMsg(this, messageId, true);
	CShareClientEditionItfProxy::broadcast_onNodeErased(broadcastList.begin(), broadcastList.end(),
		this,
		instanceId, attrName, position);
}


void CServerEditionModule::onNodeMoveAsked(NLNET::IModuleProxy *senderModuleProxy, uint32 messageId,
	const std::string& instanceId1, const std::string& attrName1, sint32 position1,
	const std::string& instanceId2, const std::string& attrName2, sint32 position2)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;


	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId);
	CShareClientEditionItfProxy sender(senderModuleProxy);
	if (!session)
	{
		nlwarning("R2Ed: char %u move node an empty scenario %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}
	CScenario* scenario = session->getScenario();

	ok = scenario->moveNode(
		instanceId1, attrName1, position1,
		instanceId2, attrName2, position2);

	if (!ok)
	{
		nlwarning("RE2D: 'moveNode' refused char %u, session %u", charId, sessionId.asInt());
		sender.ackMsg(this, messageId, false);
		return;
	}

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVectorNoSender(session, broadcastList, senderModuleProxy);
	sender.ackMsg(this, messageId, true);
	CShareClientEditionItfProxy::broadcast_onNodeMoved(broadcastList.begin(), broadcastList.end(),
		this,
		instanceId1, attrName1, position1,
		instanceId2, attrName2, position2);
}


void CServerEditionModule::registerSession(TSessionId sessionId, CEditionSession* session)
{

	bool inserted = _Sessions.insert( std::make_pair(sessionId, session) ).second;
	if (!inserted)
	{
		_IdRecycle->releaseId(session->getAiInstanceId());
		delete session;
		nlwarning("R2Ed: Error Session %u registered two times", sessionId.asInt());
		nlassert(0 && "Same session register Twice.");

	}

}

void CServerEditionModule::onUserComponentUploaded(NLNET::IModuleProxy *senderModuleProxy, CUserComponent* component)
{

	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId);
	if (!session)
	{
		delete component;
		nlwarning("R2Ed: char %u mupload component on an empty session %u", charId, sessionId.asInt());
		return;
	}
	CScenario* scenario = session->getScenario();

	CUserComponentRefCounter* refCounter = 0;
	TUserComponentRefCounter::const_iterator found = _UserComponentRefCounter.find(component->Md5);
	if (found == _UserComponentRefCounter.end())
	{
		refCounter = new CUserComponentRefCounter();
		_UserComponentRefCounter[component->Md5] = refCounter;
	}
	else
	{
		refCounter = found->second;
	}

	refCounter->updateComponent(component);
	refCounter->registerSession(sessionId);
	scenario->registerUserComponent(component->Md5);

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVector(session, broadcastList);
	CShareClientEditionItfProxy::broadcast_onUserComponentRegistered(broadcastList.begin(), broadcastList.end(), this, component->Md5);
}

CUserComponent* CServerEditionModule::getUserComponent( const NLMISC::CHashKeyMD5& md5) const
{

	TUserComponentRefCounter::const_iterator found = _UserComponentRefCounter.find(md5);
	if (found != _UserComponentRefCounter.end())
	{
		return found->second->get();

	}
	return 0;
}

void CServerEditionModule::onUserComponentDownloading(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CHashKeyMD5& md5)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	CUserComponent* component = getUserComponent(md5);
	if (component)
	{
		NLNET::CMessage message;
		message.setType("CUCD"); //onUserComponentDownloaded
		component->serial(message);
		senderModuleProxy->sendModuleMessage(this, message);
	}

}

void CServerEditionModule::onUserComponentRegistered(NLNET::IModuleProxy *senderModuleProxy, const NLMISC::CHashKeyMD5& md5 )
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	TUserComponentRefCounter::const_iterator found=_UserComponentRefCounter.find(md5);
	if (found == _UserComponentRefCounter.end())
	{
		CShareClientEditionItfProxy proxy(senderModuleProxy);
		proxy.onUserComponentUploading(this, md5);
		return;
	}

	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId) ;

	if (!session)
	{
		nlwarning("R2Ed: char %u try to register user component on a empty session %u", charId, sessionId.asInt());
		return;
	}

	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVector(session, broadcastList);
	CShareClientEditionItfProxy::broadcast_onUserComponentRegistered(broadcastList.begin(), broadcastList.end(), this, md5);
}

bool CServerEditionModule::onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message)
{
	std::string operationName = message.getName();


	if (senderModuleProxy->getModuleClassName() == "ClientEditionModule")
	{
		TCharId charId;
		CEntityId clientEid;
		std::string userPriv;
		std::string extendedPriv;

		bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
		if (!ok) { return true; }

		uint32 length = message.length();
		nldebug("R2Ed: MessageSize from char %u length=%u", charId, length);

		if (operationName == "SUCU")
		{

			CUserComponent* component = new CUserComponent();
			//message.serial(const_cast<CUserComponent&>(*component));
			component->serial( const_cast<CMessage&>(message));
			onUserComponentUploaded(senderModuleProxy, component);
			return true;
		}

		if (operationName == "DBG_CREATE_PRIMITIVES")
		{
			onMessageReceivedCreatePrimitives(senderModuleProxy, charId );
			return true;
		}

		if (operationName == "STOP_TEST")
		{
			stopTestRequested(senderModuleProxy, charId);
			return true;
		}

		if (operationName == "requestUpdateRtScenario")
		{
			CObjectSerializerServer obj;
			nlRead(message,serial,obj);
			obj.uncompress();

			bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
			if (!ok) { return true; }

			rtScenarioUpdateRequested(senderModuleProxy, charId, obj.getData());

			return true;
		}
	}

	return false;
}


void CServerEditionModule::onModuleSecurityChange(IModuleProxy *moduleProxy)
{
}

CEditionSession* CServerEditionModule::getSession(TSessionId sessionId) const
{

	TSessions::const_iterator found ( _Sessions.find(sessionId));
	if (found == _Sessions.end()) return 0;
	return found->second;
}

CScenario* CServerEditionModule::getScenarioById(TSessionId sessionId) const
{
	CEditionSession* session = getSession(sessionId);
	if (!session) return 0;
	return session->getScenario();
}


TSessionId CServerEditionModule::getSessionIdByCharId(TCharId charId) const
{
	{
		TPioneersSessionsAlloweds::const_iterator found = _PioneersSessionsAllowed.find(charId);
		if (found != _PioneersSessionsAllowed.end()) return found->second.SessionId;
	}
	{
		TPioneersSessions/*std::map<TCharId, TSessionId>*/::const_iterator found = _PioneersSessions.find(charId);
		if (found != _PioneersSessions.end()) return found->second;
	}

	return TSessionId(0);

}

TUserRole CServerEditionModule::getRoleByCharId(TCharId charId) const
{
	TPioneersSessionsAlloweds::const_iterator found = _PioneersSessionsAllowed.find(charId);
	if (found == _PioneersSessionsAllowed.end()) return found->second.Role;

	CEditionSession* session = getSessionByCharId(charId);
	if (session)
	{
		return session->getCharRole( charId);
	}

	return TUserRole::invalid;


}


TPioneersSessionsAllowed * CServerEditionModule::getSessionAllowedForChar(TCharId charId) const
{
	TPioneersSessionsAlloweds::const_iterator found = _PioneersSessionsAllowed.find(charId);
	if (found == _PioneersSessionsAllowed.end()) return NULL;
	return const_cast<TPioneersSessionsAllowed * const>(&(found->second));
}



CEditionSession* CServerEditionModule::getSessionByCharId(TCharId charId) const
{
	TSessionId sessionId = getSessionIdByCharId(charId);
	if (sessionId.asInt()==0) return 0;
	return getSession(sessionId);
}

CScenario* CServerEditionModule::getScenarioByCharId(TCharId charId) const
{
	CEditionSession* session = getSessionByCharId(charId);
	if (!session) return 0;
	return session->getScenario();
}


void CServerEditionModule::saveToDb()
{
}

// Ask the client allow a helper character in the session
NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, createSession)
{
	sint sessionType = 0;

	if (args.size() > 2) fromString(args[2], sessionType);

	if (args.size() != 3 || sessionType < 0 || sessionType > 2)
	{
		log.displayNL("3 Parameters needed: ownerCharId sessionId sessionType");
		return false;
	}

	RSMGR::TSessionType value((RSMGR::TSessionType::TValues)sessionType);
	TCharId ownerCharId;
	fromString(args[0], ownerCharId);
	uint32 id;
	fromString(args[1], id);
	TSessionId sessionId = (TSessionId)id;
	log.displayNL("Creating session %u %s", sessionId.asInt(), value.toString().c_str());
	this->createSession(_SessionManager, ownerCharId, sessionId, value );
	return true;
}


// Ask the client to create a new session modules
void CServerEditionModule::createSession(NLNET::IModuleProxy *sender, TCharId ownerCharId, TSessionId sessionId, const RSMGR::TSessionType &type)
{
	//todo
	// create a new session
	// sender == NULL <=> test the command

	nldebug("R2Ed: createSession char %u, session %u, %u", ownerCharId, sessionId.asInt(), static_cast<uint32>(type.getValue()) );
	// create an aiInstanceId


	if (_Sessions.find(sessionId) != _Sessions.end() )
	{
		nlwarning("R2Ed: trying to create an already created scenario %u (use the previous one)", sessionId.asInt());
		return;
	//	closeSession(sender, sessionId);
	}


	// register an empty scenario

	TAiInstanceId aiInstanceId = 0;
	bool linkedSession = false;


	if (type.getValue() == RSMGR::TSessionType::st_edit)
	{
		aiInstanceId = _IdRecycle->newId();
		nldebug("Session %u create AiInstance %u", sessionId.asInt(), aiInstanceId );
		CAiWrapper::getInstance().startInstance(sessionId, aiInstanceId);

		CScenario* scenario = new CScenario(0, st_edit);
		registerSession(sessionId, new CEditionSession(RSMGR::TSessionType::st_edit, sessionId, ownerCharId, aiInstanceId, scenario ));




		std::vector<CBackupFileClass> classes;
		CBackupFileClass	classOk;
		classOk.Patterns.push_back(getSessionFilename(sessionId, ownerCharId));
		classes.push_back(classOk);

		if ( CDynamicMapService::getInstance()->useNetwork() )
		{
			BsiGlobal.requestFileClass("r2/sessions/", classes, new CTestIfMustWakeUpSession(this, sessionId, ownerCharId));
		}
		else
		{
			setLoaded(sessionId);
		}

		TOwnerInfos::iterator owner = _OwnerInfos.find(ownerCharId);
		if (owner == _OwnerInfos.end()) // launch a animation from the born
		{
			_OwnerInfos.insert( std::make_pair(ownerCharId, TOwnerInfo(sessionId, TSessionId(0), aiInstanceId) ) ); // create an edition Session
		}
		else
		{
			owner->second.EditionSessionId = sessionId;
			owner->second.AiInstanceId = aiInstanceId;
		}

	}
	else if (type.getValue() == RSMGR::TSessionType::st_anim)
	{

		TOwnerInfos::iterator owner = _OwnerInfos.find(ownerCharId);
		if (owner == _OwnerInfos.end()) // launch a animation from the born
		{
			owner = _OwnerInfos.insert( std::make_pair(ownerCharId, TOwnerInfo(TSessionId(0), sessionId, aiInstanceId) ) ).first; // create an edition Session
		}
		else
		{
			aiInstanceId = owner->second.AiInstanceId;
			owner->second.AnimationSessionId = sessionId;
			linkedSession = true;

		}

		CEditionSession* editSession = getSession(owner->second.EditionSessionId);

		if (!editSession)
		{
			aiInstanceId = _IdRecycle->newId();
			nldebug("Session %u create AiInstance %u", sessionId.asInt(), aiInstanceId );
			CAiWrapper::getInstance().startInstance(sessionId, aiInstanceId);

			CScenario* scenario = new CScenario(0, st_anim);
			scenario->setMode(0);
			registerSession(sessionId, new CEditionSession(RSMGR::TSessionType::st_anim, sessionId, ownerCharId, aiInstanceId, scenario));
		}
		else
		{
			registerSession(sessionId, new CEditionSession(RSMGR::TSessionType::st_anim, sessionId, editSession ));
		}

		if (linkedSession)
		{
			TOwnerInfos::iterator owner = _OwnerInfos.find(ownerCharId);
			TSessionId animSessionId = owner->second.AnimationSessionId;
			TSessionId editSessionId = owner->second.EditionSessionId;
			if (editSessionId && _CharacterControlProxy)
			{
				// Give 5 more minute to empty session
				CEditionSession* editSession = getSession(editSessionId);
				if (editSession) { editSession->touch(); }

				CCharacterControlItfProxy proxy(_CharacterControlProxy);
				proxy.reportLinkedSession(this, editSessionId, animSessionId);
			}

		}

	}
	else // outland session are not implemented yet
	{
		nlstop;
	}

	// send MSG to session manager
	if (sender)
	{
		nlassert(sender == _SessionManager);

		// you need to report the creation to session manager :
		RSMGR::CRingSessionManagerProxy rsm(_SessionManager);


		if (type.getValue() == RSMGR::TSessionType::st_edit)
		{
		}
		else if (type.getValue() == RSMGR::TSessionType::st_anim)
		{
		}
		else
		{
			nlstop;
			return;
		}

		RSMGR::TRunningSessionInfo sessionInfo;
		sessionInfo.setInstanceId(aiInstanceId);
		sessionInfo.setSessionId(sessionId);
		sessionInfo.setNbPlayingChars(0);
		sessionInfo.setSessionType(type.getValue());

		rsm.sessionCreated(this, sessionInfo);
	}


	if (type.getValue() == RSMGR::TSessionType::st_anim && linkedSession)
	{
		TScenarioHeaderSerializer::TValueType values;
		bool ok = getAnimationModule()->getHeaderInfo(sessionId, values);
		if (!ok)
		{
			nlwarning("Session%u, the linked session %u does not have any animation data", getLinkedSessionId(sessionId).asInt(), sessionId.asInt());
		}
		else
		{
			TRunningScenarioInfo info;
			TScenarioHeaderSerializer::TValueType::const_iterator first(values.begin()), last(values.end());
			bool nevraxScenario = false;
			bool trialAllowed = false;
			std::string missionTag;


			for ( ; first != last; ++first)
			{
				if (first->first == "BodyMD5" )
				{
					CHashKeyMD5 md5;
					md5.fromString(first->second);
					info.setScenarioKey(md5);
				}

				else if (first->first == "Title" )	{ info.setScenarioTitle(first->second); }
				else if (first->first == "ShortDescription" )	{ info.setScenarioDesc(first->second); }
				else if (first->first == "Level" )
				{
					 TSessionLevel level;
					 uint32 charLevel;
					 fromString(first->second, charLevel);
					 level.fromCharacterLevel(charLevel);
					info.setSessionLevel(level);
				}
				else if (first->first == "CreateBy" )	{ info.setScenarioAuthorName( first->second); }
				else if (first->first == "Rules" ) { info.setDMLess( first->second != "Mastered"); }
				else if (first->first == "NevraxScenario" && first->second == "1") { nevraxScenario = true; }
				else if (first->first == "TrialAllowed" && first->second == "1") { trialAllowed = true; }
				else if (first->first == "MissionTag" && first->second != "") { missionTag = first->second; }
			}
//			info.setMissionTag(missionTag);
			info.setSessionAnimatorCharId( ownerCharId);
			if ( !nevraxScenario )
			{
				missionTag = "";
				trialAllowed = false;
			}




			const NLNET::TModuleProxyPtr * ownerModule = getClientProxyPtr(ownerCharId);

			std::string extendedPriv;
			// The owner must be there because he launches the animation from the born
			if (ownerModule)
			{
				uint32 charId;
				NLMISC::CEntityId clientEid;
				std::string userPriv;
				getCharInfo(*ownerModule, charId, clientEid, userPriv, extendedPriv);
			}

			if (!trialAllowed && getIsCharRoS(ownerCharId) && extendedPriv.find("TRIAL") != std::string::npos)
			{
				nlwarning("ERROR: A client %u(CharId) (RoS) try to launch an animation that is not RoS. That means that the client has not check the RoS Flags or that this user is trying to hack use. So we will close he animation session (%u) so he will be kicked back to edition.", ownerCharId, sessionId.asInt());
				return;
			}

			if(_CharacterControlProxy)
			{
				CCharacterControlItfProxy proxy(_CharacterControlProxy);
				proxy.animSessionStarted(this, sessionId, info);

				CEditionSession* session = getSession(sessionId);
				BOMB_IF(!session, "ERROR: session must exist", return);
				// ie the animator and player that just arrived before the scenario has uploaded
				CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
				for (; first != last; ++first)
				{
					proxy.charJoinAnimSession(this, (*first)->getCharId(), sessionId);
				}

			}

			if (_SessionManager)
			{
				RSMGR::CRingSessionManagerProxy rsm(_SessionManager);
				rsm.scenarioStarted(this, sessionId, info);
				nldebug("Scenario Started %d %s", sessionId.asInt(), info.getScenarioKey().toString().c_str());
			}

		}

	}
}




// Ask the client allow a helper character in the session
NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, addCharacterInSession)
{
	if (args.size() <3  || args.size() > 5)
	{
		log.displayNL("addCharacterInSession.addCharacterInSession <sessionId> <clientId> <roleAsString> <[ringAcess]> <[isnewcomer]>");
		return false;
	}
	uint32 id;
	fromString(args[0], id);
	TSessionId sessionId = (TSessionId)id;

	TCharId charId;
	fromString(args[1], charId);

	WS::TUserRole userRole = 	WS::TUserRole(args[2]);

	if (userRole.getValue() == WS::TUserRole::invalid_val)
	{

		log.displayNL("roleAsString must be ur_editor ur_animator ur_player");
		return false;
	}

	std::string ringAccess;
	if (args.size() == 4 )
	{
		ringAccess = args[3];
	}

	bool newcomer = false;
	if (args.size() == 5 )
	{
		fromString(args[4], newcomer);
	}

	this->addCharacterInSession(0, sessionId, charId, userRole, ringAccess, newcomer );




	return true;
}

void CServerEditionModule::addCharacterInSession(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId, const WS::TUserRole &connectedAs, const std::string &ringAccess, bool newcomer)
{

	nldebug("R2Ed: addCharacterInSession %u %u %s",sessionId.asInt(), charId, connectedAs.toString().c_str() );

	_PioneersSessionsAllowed[charId].SessionId = sessionId;
	_PioneersSessionsAllowed[charId].EgsReloadPos = false;
	_PioneersSessionsAllowed[charId].Newcomer = newcomer;

	TUserRole role;
	switch(connectedAs.getValue()) // WS::TUserRole and R2::TUserRole are distinct because WS is in NeLNS
	{
		case WS::TUserRole::ur_animator: role = R2::TUserRole::ur_animator; break;
		case WS::TUserRole::ur_editor: role = R2::TUserRole::ur_editor; break;
		case WS::TUserRole::ur_player: role = R2::TUserRole::ur_player; break;
		// TODO: Add ur_outland_owner
		default:
		nlassert(0 && "User Role not handled");
	}


	std::string upgradedRingAccess = CRingAccess::getInstance().upgradeRingAccess(CSString(DefaultCharRingAccess.get()).toLower(), CSString(ringAccess).toLower()) ;


	// TODO use security info for overdoing ring access
	_PioneersSessionsAllowed[charId].Role = role;
	_PioneersSessionsAllowed[charId].RingAccess = upgradedRingAccess;

	// give 5 more minutes to the session to live
	CEditionSession* session = getSession(sessionId);
	if (session) { session->touch(); }

	// Append only if pioneer is already in a session (like edition/test) and start an animation
	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
	if (clientEditionProxyPtr)
	{
		connectChar(sessionId, charId, role, upgradedRingAccess, newcomer);
		return;
	}


}


void CServerEditionModule::resetSession(NLNET::IModuleProxy *sender, TSessionId sessionId, bool reconnect)
{

	CEditionSession* session = getSession(sessionId);

	CScenario* scenario = session?session->getScenario():0;
	if (!session || !scenario)
	{
		nlwarning("Try to reset a non existing session %u", sessionId.asInt());
		// can not reset this session
		return;
	}

	session->RingAccess = "";
	if (!reconnect) // reset
	{
		scenario->setHighLevel( 0 );
		getSession(sessionId)->RingAccess = "";

		scenario->setMode(0);


		bool animationSessionExist = false;
		// Add charId connected in animation Module
		{
			std::vector<TCharId> connectedChars;
			animationSessionExist = getAnimationModule()->getConnectedChars(sessionId, connectedChars);
		}

		//(*) Remove animation Session and kill AI Instance
		if (animationSessionExist)
		{

			stopTest(sessionId);
		}
	}

	// Reconnect user
	CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
	for (; first != last; ++first)
	{
		TCharId  charId = (*first)->getCharId();
		const TModuleProxyPtr* proxy = getClientProxyPtr( charId);
		TUserRole role =  (*first)->getRole();
		if (proxy)
		{
			onMapConnectionAsked(*proxy, sessionId, true, true, role);
		}
	}
}


bool CServerEditionModule::closeSessionImpl(TSessionId sessionId,  std::string& msg)
{
	//(*) remove player from connecting queue
	// This very small map contains player that are connecting to dss (allowed by SU to connect but not yet connected because they are loading data)
	// so its ok to iterate





	bool ret = true;
	if ( _Sessions.find(sessionId) != _Sessions.end())
	{
		msg += toString("Closing (running) session %u)", sessionId.asInt());
	}
	else
	{
		msg += toString("No session (running or hiberning) with id %u", sessionId.asInt());
		ret = false;
	}



	TPioneersSessionsAlloweds::iterator first(_PioneersSessionsAllowed.begin()), last(_PioneersSessionsAllowed.end());
	for (; first != last;)
	{
		TPioneersSessionsAlloweds::iterator copy = first++;
		if (copy->second.SessionId == sessionId)
		{
			_PioneersSessionsAllowed.erase(copy);
		}
	}

	//(*) Remove connected Player (It is the SU choice)

	std::set<TCharId> charIds;
	// Add charId connected in edition module

	CEditionSession* session = getSession(sessionId);

	bool mustStopAiInstance = false;
	TCharId ownerCharId = 0;
	TSessionId animationSessionId = 0;
	bool isLinkedAnimSession = false;
	TSessionId linkedSessionId = sessionId;
	// Closing edit session linked to animation close the animation.
	if (session)
	{
		if (session->getSessionType() == RSMGR::TSessionType::st_edit)
		{
			ownerCharId = session->getOwnerCharId();
			TOwnerInfos::iterator it=_OwnerInfos.find(ownerCharId);
			if (it != _OwnerInfos.end())
			{
				animationSessionId= it->second.AnimationSessionId ;
				if (animationSessionId)
				{
					msg += "Closing linked anim session.\n";
					bool ok = closeSessionImpl(animationSessionId, msg);
					if ( ret) { ret = ok; }
				}
			}

		}
		if (session->getSessionType() == RSMGR::TSessionType::st_anim)
		{
			if (_CharacterControlProxy)
			{
				CCharacterControlItfProxy proxy(_CharacterControlProxy);
				uint32 score;
				NLMISC::TTime timeTaken;
				getAnimationModule()->getScore(linkedSessionId, score, timeTaken);

				proxy.animSessionEnded(this, sessionId,score,timeTaken);
			}
		}


		ownerCharId = session->getOwnerCharId();

		TOwnerInfos::iterator it=_OwnerInfos.find(ownerCharId);
		if (it != _OwnerInfos.end())
		{
			if (session->getSessionType() == RSMGR::TSessionType::st_edit)
			{
				it->second.EditionSessionId = TSessionId(0);
				animationSessionId = it->second.AnimationSessionId;
			}
			else if (session->getSessionType() == RSMGR::TSessionType::st_anim)
			{
				TSessionId oldAnimationSession = it->second.AnimationSessionId;
				 it->second.AnimationSessionId = TSessionId(0);
				mustStopAiInstance = true;
				isLinkedAnimSession = true;
				if (it->second.EditionSessionId)
				{
					linkedSessionId = it->second.EditionSessionId;
				}

				if (it->second.EditionSessionId && _CharacterControlProxy)
				{
					CCharacterControlItfProxy proxy(_CharacterControlProxy);
					proxy.reportUnlinkedSession(this, it->second.EditionSessionId, oldAnimationSession);
				}

			}
			if (it->second.EditionSessionId == TSessionId(0) && it->second.AnimationSessionId == TSessionId(0))
			{
				_OwnerInfos.erase(it);
				mustStopAiInstance = true;
			}
		}

		const CEditionSession::TCurrentChars& charIdContainer = session->getCurrentChars();
		CEditionSession::TCurrentChars::const_iterator first(charIdContainer.begin()), last(charIdContainer.end());
		for (; first != last ; ++first)
		{
			charIds.insert( (*first)->getCharId() );
		}
	}



	bool animationSessionExist;
	{
		std::vector<uint32> connectedChars;
		animationSessionExist = getAnimationModule()->getConnectedChars(linkedSessionId, connectedChars);
	}

	// Kick all connected character
	{
		std::set<TCharId>::iterator first(charIds.begin()), last(charIds.end());
		for ( ; first != last; ++first)
		{
			std::string outMsg;
			removeCharacterFromSessionImpl(sessionId, *first, outMsg);
		}
	}

	if (mustStopAiInstance)
	{
			//(*) Remove animation Session and kill AI Instance
		if (animationSessionExist)
		{
			stopTest(linkedSessionId);
		}
	}


	//(*) Remove Edition Session

	{
		TSessions::iterator found = _Sessions.find(sessionId);
		if (found != _Sessions.end())
		{
			CEditionSession* session = found->second;
			if (session && mustStopAiInstance)
			{
				TAiInstanceId aiInstanceId = session->getAiInstanceId();

				if (!isLinkedAnimSession)
				{
					if (aiInstanceId == 0)
					{
						nlwarning("Error use of an unallocated instance");
						nlassert(0 && "Error use of an unallocated instance");
						return false;
					}
					_IdRecycle->releaseId(aiInstanceId);
				}

				nldebug("Session %u destroy AiInstance %u", sessionId.asInt(), aiInstanceId );
				CAiWrapper::getInstance().stopTest(linkedSessionId, aiInstanceId);

			}
			delete session;
			_Sessions.erase(found);
		}

		// No need to do more (because all client are removed)
	}

	//(*) If session is hibernating

	if (DssUseBs)
	{
		_MustUpdateHibernatingFileList = true;
		if (ownerCharId == 0)
		{
			nlwarning("Try to close a session that was not opened %u", sessionId.asInt());
		}
		else
		{
			_ClosedSessions.insert( std::make_pair(sessionId, ownerCharId));
			// force the 'delete' command to BS to be send now
			updateSessionListFile();
		}

	}

	if (_SessionManager)
	{
		//( 5 ) report the session closing event
		RSMGR::CRingSessionManagerProxy rsm(_SessionManager);

		rsm.reportSessionEvent(this, RSMGR::TSessionEvent::se_session_closing, sessionId, 0);
	}




	return ret;
}

void CServerEditionModule::closeSession(NLNET::IModuleProxy *sender, TSessionId sessionId)
{
//#pragma message (NL_LOC_WRN "TODO : Vianney : remove hibernation file for the specified session")
	std::string msg;
	bool ok = closeSessionImpl(sessionId, msg);
	debugInfo(ok, msg);

}

void CServerEditionModule::stopHibernation(NLNET::IModuleProxy *sender, TSessionId sessionId, uint32 charId)
{
//#pragma message (NL_LOC_WRN "TODO : Vianney : remove hibernation file for the specified session")
	BsiGlobal.deleteFile( std::string("r2/sessions/")+getSessionFilename(sessionId, charId), false);
}

void CServerEditionModule::hibernateSession(NLNET::IModuleProxy *sender, TSessionId sessionId)
{

	std::string msg;
	bool ok = hibernateSessionImpl(sessionId, msg);
	debugInfo(ok, msg);

}


std::string CServerEditionModule::getSessionFilename(TSessionId sessionId, TCharId charId) const
{
	return NLMISC::toString("r2_session_%.08u_%08u.dat",  sessionId.asInt(), charId );
}

std::string CServerEditionModule::getOverrideRingAccessFilename() const
{
	return "r2/override_ring_access.txt";
}

//implementation of characterKicked and command
bool CServerEditionModule::unkickPioneerImpl(TSessionId sessionId, TCharId charId, std::string& outMsg)
{

	// The web/dm has asked to session manager to unkick a character from a session,



	outMsg = NLMISC::toString( "Trying to unckick the character %u from session %u", charId, sessionId.asInt());

	TPioneersSessions::iterator foundPioneerSession = _PioneersSessions.find(charId);

	//(if he is connected).
	if (foundPioneerSession != _PioneersSessions.end())
	{
		TSessionId connectedSessionId = foundPioneerSession->second;
		if (connectedSessionId != sessionId)
		{
			outMsg += NLMISC::toString(
				"Refuse to unkick the character %u from session %u because he is connected to another session %u",
				charId, sessionId.asInt(), connectedSessionId.asInt() ) ;
			return false;
		}

		outMsg += NLMISC::toString(
				"Send a message to character %u from session %u to indicates that he is no more banned",
				charId, sessionId.asInt()) ;

		// disconnect the client
		const NLNET::TModuleProxyPtr* clientPtr = getClientProxyPtr(charId);
		if (clientPtr)
		{
			CShareClientEditionItfProxy clientProxy(*clientPtr);
			clientProxy.onKicked(this, 0, false);
		}

		uint32 last = _Tasks.getSize();
		uint32 first = 0;
		for ( ; first != last ; )
		{
			CTask<NLMISC::TTime>* task = _Tasks.getTaskAt(first);
			CKickPlayerIfStillConnected* t = dynamic_cast<CKickPlayerIfStillConnected*>(task);
			if (t && t->getCharId() == charId && t->getSessionId() == sessionId )
			{
				_Tasks.removeTaskAt(first);
				--last;
			}
			else
			{
				++first;
			}
		}

		// here the client is first disconnected, then firewalled again an finally out of the game
		return true;
	}

	else
	{
		outMsg += NLMISC::toString(
			"R2Ed: The character %u was unkick from session %u (but because he is not connected we have nothing to do)",
			charId, sessionId.asInt()) ;
		return true;

	}

	nlassert(0); // can not be here
	return true;
}


//implementation of characterKicked and command
bool CServerEditionModule::kickPioneerImpl(TSessionId sessionId, TCharId charId, std::string& outMsg)
{

	// The web/dm has asked to session manager to kick a character from a session,
	// in response, the session manager as tagged the character participation as
	// 'kicked' and ask the session server (DSS) to kick the character
	// out of the session


	outMsg = NLMISC::toString( "Trying to kick the character %u from session %u", charId, sessionId.asInt());

	TPioneersSessions::iterator foundPioneerSession = _PioneersSessions.find(charId);

	//(if he is connected).
	if (foundPioneerSession != _PioneersSessions.end())
	{
		TSessionId connectedSessionId = foundPioneerSession->second;
		if (connectedSessionId != sessionId)
		{
			outMsg += NLMISC::toString(
				"Refuse to kick the character %u from session %u because he is connected to another session %u",
				charId, sessionId.asInt(), connectedSessionId.asInt() ) ;
			return false;
		}

		outMsg += NLMISC::toString(
				"Send a message to character %u from session %u to ask him to go away or he will be kicked",
				charId, sessionId.asInt()) ;

		// disconnect the client
		const NLNET::TModuleProxyPtr* clientPtr = getClientProxyPtr(charId);
		if (clientPtr)
		{
			CShareClientEditionItfProxy clientProxy(*clientPtr);
			clientProxy.onKicked(this, TimeBeforeDisconnectionAfterKick, true);
		}

		NLMISC::TTime now = NLMISC::CTime::getLocalTime();
		_Tasks.addTask(new CKickPlayerIfStillConnected(now + 1000*TimeBeforeDisconnectionAfterKick , this, sessionId, charId));
		// here the client is first disconnected, then firewalled again an finally out of the game
		return true;
	}

	else
	{
		// If not connected, the player is in a waiting list of allowed player,
		// so we remove him from the this waiting list (the player is
		// currently loading 3d data before entering the session.)
		// Whe he will ask to enter he will be kicked

		// player that are allowed by su to connect but are not yet connected
		TPioneersSessionsAlloweds::iterator allowedChar = _PioneersSessionsAllowed.find(charId);
		if (allowedChar == _PioneersSessionsAllowed.end())
		{
			// no player are in the waiting for connection of this session

			outMsg += NLMISC::toString(
				"Refuse to kick the character %u from session %u because he is offline",
				charId, sessionId.asInt()
				);

			return false;
		}

		if (allowedChar->second.SessionId != sessionId)
		{
			outMsg += NLMISC::toString(
				"R2Ed: Refuse to kick the character %u from session %u because he is connecting to another session %u",
				charId, sessionId.asInt(), allowedChar->second.SessionId.asInt()
				);

			return false;
		}

		_PioneersSessionsAllowed.erase( allowedChar);

		outMsg += NLMISC::toString(
			"R2Ed: The character %u was kick from session %u while he was connecting",
			charId, sessionId.asInt()
			) ;
		return true;

	}

	nlassert(0); // can not be here
	return true;
}



void CServerEditionModule::verifyRingAccess(TSessionId sessionId, TCharId charId)
{
	CEditionSession* session = getSession(sessionId);
	if (!session) { return; }
	if (!session->getScenario()->getHighLevel())
	{
		return;
	}
	TPionnerInfos::const_iterator found = _PioneersInfo.find(charId);

	if (found != _PioneersInfo.end())
	{
		if (! CRingAccess::getInstance().verifyRight(session->RingAccess, found->second.RingAccess))
		{
			 resetSession(0, sessionId, false); //TODO kick if invited, reset if owner
		}
	}
}

bool CServerEditionModule::removeCharacterFromSessionImpl(TSessionId sessionId, TCharId charId, std::string& outMsg)
{

	// The web/dm has asked to session manager to kick a character from a session,
	// in response, the session manager as tagged the character participation as
	// 'kicked' and ask the session server (DSS) to kick the character
	// out of the session



	outMsg = NLMISC::toString("Trying to remove the character %u from session %u", charId, sessionId.asInt()) ;


	TPioneersSessions::iterator foundPioneerSession = _PioneersSessions.find(charId);

	//(if he is connected).
	if (foundPioneerSession != _PioneersSessions.end())
	{
		TSessionId connectedSessionId = foundPioneerSession->second;
		if (connectedSessionId != sessionId)
		{
			outMsg += NLMISC::toString("Refuse to remove the character %u from session %u because he is connected to another session %u",
				charId, sessionId.asInt(), connectedSessionId.asInt()	);
			return false;
		}

		// disconnect the client
		disconnectChar(charId);
		// here the client is first disconnected, then firewaled again an finally out of the game
		return true;
	}

	else
	{
		// If not connected, the player is in a waiting list of allowed player,
		// so we remove him from the this waiting list (the player is
		// currently loading 3d data before entering the session.)
		// Whe he will ask to enter he will be kicked

		// player that are allowed by su to connect but are not yet connected
		TPioneersSessionsAlloweds::iterator allowedChar = _PioneersSessionsAllowed.find(charId);
		if (allowedChar == _PioneersSessionsAllowed.end())
		{
			// no player are in the waiting for connection of this session

			outMsg += NLMISC::toString( "Refuse to remove the character %u from session %u because he is offline",	charId, sessionId.asInt());

			return false;
		}

		if (allowedChar->second.SessionId != sessionId)
		{
			outMsg += NLMISC::toString("R2Ed: Refuse to remove the character %u from session %u because he is connecting to another session %u",
				charId, sessionId.asInt(), allowedChar->second.SessionId.asInt());

			return false;
		}

		_PioneersSessionsAllowed.erase( allowedChar);

		outMsg += NLMISC::toString( "R2Ed: The character %u was remove from session %u while he was connecting",charId, sessionId.asInt()) ;

		return true;

	}

	nlassert(0); // can not be here
	return true;
}


// Session manager report that a character has been kicked by the web
void CServerEditionModule::characterKicked(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId)
{
	// sender is SU

	std::string msg;
	bool ok = kickPioneerImpl(sessionId, charId, msg);
	debugInfo(ok, msg);

}

void CServerEditionModule::characterUnkicked(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId charId)
{
	// sender is SU

	std::string msg;
	bool ok = unkickPioneerImpl(sessionId, charId, msg);
	debugInfo(ok, msg);

}





// ----------------------------------- TELEPORT ----------------------------------------------------
// Session manager report that a character has been kicked by the web
void CServerEditionModule::teleportOneCharacterToAnother(NLNET::IModuleProxy *sender, TSessionId sessionId, TCharId source, TCharId dest)
{
	// sender is SU

	std::string msg;
	bool ok = teleportOneCharacterToAnotherImpl(sessionId, source, dest, msg);
	debugInfo(ok, msg);

}

bool CServerEditionModule::teleportOneCharacterToAnotherImpl(TSessionId sessionId, TCharId source, TCharId dest, std::string&msg)
{
	CEditionSession * session = getSession(sessionId);
	CScenario* scenario = 0;

	if (session) { scenario = session->getScenario(); }
	if (!session || !scenario)
	{
		msg = toString("Try to teleport a character %u to another character %u but in a session  %u that do not exist", source, dest, session);
		return false;
	}

	const NLNET::TModuleProxyPtr * srcClient = getClientProxyPtr(source);
	const NLNET::TModuleProxyPtr * destClient = getClientProxyPtr(dest);
	bool sourceCharOk =  (session->getCharRole(source) != TUserRole::invalid && srcClient);
	bool destCharOk =  (session->getCharRole(dest) != TUserRole::invalid && destClient);

	if (! sourceCharOk || !destCharOk)
	{
		msg = toString("Try to teleport a character %u %sto another character %u %sin the session  %u",
			source,  sourceCharOk?"":"that is not present in the current session ",
			dest,  destCharOk?"":"that is not present in the current session ",
			session);

		if (srcClient)
		{
			CShareClientEditionItfProxy proxy( *srcClient );
			proxy.systemMsg(this, "BC", "", "uiR2EDTryToTeleportToAnInvalidCharacter");
		}
		/*
			Send warning message to client
		*/
		return false;
	}

	if (_CharacterControlProxy.isNull())
	{
		msg="Try to tp to another player but no EGS?";
		return false;
	}
	CCharacterControlItfProxy proxy(_CharacterControlProxy);

	uint8 season=0;
	//TSessionId sessionId = this->getSessionIdByCharId(charId);
	{
		double sx, sy, sorient;
		this->_Server->getAdminModule()->getPosition(sessionId, sx, sy, sorient, season);
	}

	proxy.teleportOneCharacterToAnother(this, source, dest, season);

	msg = NLMISC::toString("teleportCharacter %u %u in session(%u) at season(%u)", source, dest, sessionId.asInt(), season);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, teleportOneCharacterToAnother)
{

	if (args.size() != 3)
	{
		log.displayNL("Wrong parameters CServerEditionModule.teleportPioneer <sessionId> <sourceCharId> <destCharId>");
		return false;
	}

	uint32 sessionId;
	fromString(args[0], sessionId);

	TCharId sourceCharId, destCharId;
	fromString(args[1], sourceCharId);
	fromString(args[2], destCharId);

	std::string msg;
	teleportOneCharacterToAnotherImpl( (TSessionId)sessionId, sourceCharId, destCharId, msg);
	log.displayNL("%s", msg.c_str());
	return true;
}

bool CServerEditionModule::checkScenario(CObject* scenario)
{
	CObject* acts = scenario->findAttr("Acts");

	uint32 maxNpcs = 0;
	uint32 baseActCost = 0;

	uint32 baseActStates = 0;
	uint32 maxStates = 0;

	if(acts&&acts->isTable())
	{
		uint32 max = acts->getSize();
		for(uint32 i = 0;i<max;i++)
		{
			CObject * act = acts->getValue(i);
			CObject * npcs = act->findAttr("Npcs");
			CObject * states = act->findAttr("AiStates");
			if (states && states->isTable())
			{
				uint32 max2 = states->getSize();
				if (i==0)
				{
					baseActStates = max2;
				}
				else
				{
					if(baseActStates + max2 > maxStates)
					{
						maxStates = baseActStates + max2;
					}
				}
			}
			if(npcs && npcs->isTable())
			{
				uint32 max2 = npcs->getSize();
				if(i==0)
				{
					baseActCost = max2;
				}
				else
				{
					// MaxNpcs
					// MaxStaticObject (Static object are handel as Npc for the moment
					// 5 Comptation error (error in a component)
					if (baseActCost+max2 > MaxNpcs + MaxStaticObjects + 5)
					{
						nlwarning("R2Ed: checkScenario: too many npcs: %u",(baseActCost+max2));
						return false;
					}

					if (maxNpcs < baseActCost+max2)
					{
						maxNpcs = baseActCost+max2;
					}

				}
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		return false;
	}
	nldebug("Max number of Npcs : %u",maxNpcs);
	nldebug("Max number of states : %u",maxStates);
	return true;
}

void CServerEditionModule::startingScenario(NLNET::IModuleProxy *senderModuleProxy)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }


	CEditionSession* session = getSessionByCharId(charId);
	if (!session)
	{
		nlwarning("R2Ed: error: char %u not connected to a scenario", charId);
		return;
	}

	// Warning : do not set this vector 'static' because it will hold smart pointer on module
	vector<TModuleProxyPtr> broadcastList;
	fillBroadcastVector(session, broadcastList);
	CShareClientEditionItfProxy::broadcast_startingScenario(broadcastList.begin(), broadcastList.end(), this, charId);

}

void CServerEditionModule::startScenario(NLNET::IModuleProxy *senderModuleProxy, bool ok, const TScenarioHeaderSerializer& header, const CObjectSerializerServer &data, uint32 startingAct)
{
	CObject* rtData = data.getData();
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;
	std::string errorReason;

	if ( !checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv) )
	{
		return;
	}


	if (ok)
	{
		if (!checkScenario(rtData))
		{
			nlwarning("R2Ed: char %u has too many elements in scenario!", charId);
			errorReason = "uiR2EDTooManyElmements";
			delete rtData;
			ok = false;
		}

	}

	// check user has the right to start the scenario
	if (MustVerifyRingAccessWhileLoadingAnimation && ok)
	{
		CVerfiyRightRtScenarioError *err = 0;
		std::string charAccess = _PioneersInfo[charId].RingAccess;
		if (!CRingAccess::getInstance().verifyRtScenario(rtData, charAccess, err))
		{
			nlwarning("R2Ed: char %u try to run a scenario with too high level content. ", charId);
			if (err)
			{
				nlwarning("R2Ed: '%s'", err->toString().c_str());
			}
			delete rtData;
			delete err;
			ok = false;
			errorReason = "uiR2EDTooHighContent";
		}

	}


	if (ok)
	{

		CEditionSession* session = getSessionByCharId(charId);
		CScenario* scenario = session?session->getScenario():0;
		if (!scenario)
		{
			nlwarning("R2Ed: char %u is trying to act on an invalid scenario", charId);
			delete rtData;
			return;
		}

		scenario->setRtData(rtData);


		TPioneersSessions/*std::map<uint32, uint32>*/::const_iterator sessionIdIt = _PioneersSessions.find(charId);

		if (sessionIdIt == _PioneersSessions.end() )
		{
			nlwarning("R2Ed: char %u is not connected", charId);
			return;
		}


		{
			CAnimationMessageAnimationStart body;

			const CEditionSession::TCurrentChars& animatorCharId = session->getCurrentChars();
			CEditionSession::TCurrentChars::const_iterator first(animatorCharId.begin()), last(animatorCharId.end());
			for (; first != last ; ++first)
			{
				body.AnimatorCharId.push_back( (*first)->getCharId() );
			}

			//body.EditSlotId = scenario->getUserSlotId(userId);
			body.SessionId = sessionIdIt->second;
			body.AiInstance = session->getAiInstanceId();

			if ( session->getEditSlotId(charId) == 0)
			{
				nlwarning("R2Ed: char %u is not connected to scenario %u but wants to do a start Test.", charId, body.SessionId.asInt());
				return;
			}

			if (!rtData)
			{
				nlwarning("R2Ed: char %u is connected to scenario %u but wants to do a start Test with no rt Data.", charId, body.SessionId.asInt());
				delete rtData;
				return;
			}
			scenario->setMode(2);
			body.RtData.setData(rtData);
			body.StartingAct = startingAct;
			body.ScenarioHeader = header;

			getAnimationModule()->scheduleStartSession(body);
			bool trialAllowed = false;
			bool nevraxScenario = false;
			std::string missionTag;

			/*Informs EGS that the animation has started*/
			if ( getLinkedSessionId(body.SessionId) == TSessionId(0) && session->getSessionType() == RSMGR::TSessionType::st_anim )
			{
				TRunningScenarioInfo info;
				TScenarioHeaderSerializer::TValueType::const_iterator first(header.Value.begin()), last(header.Value.end());
				for ( ; first != last; ++first)
				{
					if (first->first == "BodyMD5" )
					{
						CHashKeyMD5 md5;
						md5.fromString(first->second);
						info.setScenarioKey(md5);
					}
					else if (first->first == "Title" )	{ info.setScenarioTitle(first->second); }
					else if (first->first == "ShortDescription" )	{ info.setScenarioDesc(first->second); }
					else if (first->first == "Level" )
					{
						 TSessionLevel level;
						 uint32 charLevel;
						 fromString(first->second, charLevel);
						 level.fromCharacterLevel(charLevel);
						info.setSessionLevel(level);
					}
					else if (first->first == "CreateBy" )	{ info.setScenarioAuthorName( first->second); }
					else if (first->first == "Rules" ) { info.setDMLess( first->second != "Mastered"); }
					else if (first->first == "NevraxScenario" && first->second == "1") { nevraxScenario = true; }
					else if (first->first == "TrialAllowed" && first->second == "1") { trialAllowed = true; }
					else if (first->first == "MissionTag" && first->second != "") { missionTag = first->second; }
				}

				info.setSessionAnimatorCharId( charId);
				if ( !nevraxScenario )
				{
					missionTag = "";
					trialAllowed = false;
				}


				bool isCharRoS = getIsCharRoS(charId);

				if  ( isCharRoS )
				{
					if (extendedPriv.find("TRIAL") != std::string::npos && !trialAllowed)
					{
						errorReason = "uiR2EDRoSTryMainland";
						ok = false;
						nlwarning("ERROR: A client %u(CharId) (RoS) try to launch an animation (from the born) that is not RoS. That means that the client has not check the RoS Flags or that this user is trying to hack use. So we will close he animation session (%u) so he will be kicked back to edition.", charId, body.SessionId.asInt());
					}

				}


				if(ok &&_CharacterControlProxy)
				{
					CCharacterControlItfProxy proxy(_CharacterControlProxy);
					proxy.animSessionStarted(this, body.SessionId, info);
					CEditionSession* session = getSession(body.SessionId);
					BOMB_IF(!session, "ERROR: session must exist", return);
					// ie the animator and player that just arrived before the scenario has uploaded
					CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
					for (; first != last; ++first)
					{
						proxy.charJoinAnimSession(this, (*first)->getCharId(), body.SessionId);
					}
				}

				if (_SessionManager)
				{
					RSMGR::CRingSessionManagerProxy rsm(_SessionManager);
					rsm.scenarioStarted(this, body.SessionId, info);
					nldebug("Scenario Started %d %s", body.SessionId.asInt(), info.getScenarioKey().toString().c_str());
				}
			}
		}

	}


	CMessage message;
	replyToAll(senderModuleProxy, CShareClientEditionItfProxy::buildMessageFor_startScenario(message, ok, startingAct, errorReason));


	if (ok)
	{
		_Tasks.addTask(new CTaskUpdateScenarioVision(0, this, getSessionIdByCharId(charId)));
	}
}

bool CServerEditionModule::getIsCharRoS(TCharId charId) const
{

	TPionnerInfos::const_iterator info = _PioneersInfo.find(charId);
	if (info == _PioneersInfo.end() )
	{
		nlwarning("ERROR: the code is looking to see if a character(%u/%u) is a newcomer, but the character is not log yet", charId, (charId >> 4));
		return false;
	}

	bool isCharRoS = info->second.Newcomer;
	if (OverrideRoSValue == 0)
	{
		isCharRoS = false;
	}
	if (OverrideRoSValue == 1)
	{
		isCharRoS = true;
	}
	else // -1
	{
		// keep default value
	}
	return isCharRoS;
}

void CServerEditionModule::updateScenarioVision(TSessionId sessionId)
{

	CEditionSession* session = getSession(sessionId);
	if (!session){ return; }
// Reinitialise the Vision He would be smarter to use an new message where whe serialize the list of co editor
	const CEditionSession::TCurrentChars& charIds = session->getCurrentChars();
	CEditionSession::TCurrentChars::const_iterator first(charIds.begin()), last(charIds.end());
	for (; first != last ; ++first)
	{
		TCharId charId = (*first)->getCharId();
		TPionnerInfos::iterator found(_PioneersInfo.find(charId));
		if (found != _PioneersInfo.end())
		{
			CEntityId eid = found->second.EntityId;
			{
				CMessage msgout("R2_VISION_REFRESH");
				msgout.serial(eid);
				CUnifiedNetwork::getInstance()->send("GPMS",msgout);
			}
		}
	}

}


bool CServerEditionModule::getPosition(TSessionId sessionId, double& x, double& y, double& orient, uint8& season, uint32 actIndex)
{
	x=0;
	y=0;
	orient=0;
	season=0;

	CScenario* scenario = getScenarioById(sessionId);


	BOMB_IF(!scenario, "Invalid scenario.", return false);

	CObject* hl = scenario->getHighLevel();


	if (!hl)
	{
		TSessionId linkedId = 0;
		CEditionSession* session = getSession(sessionId);
		if (session && (session->InitialX != 0 || session->InitialY != 0))
		{
			x = session->InitialX;
			y = session->InitialY;
			orient = 0;
			season = session->InitialSeason;
			return true;
		}
		return false;

	} // Map selection (non tp)

	CObject* locations = hl->getAttr("Locations");

	BOMB_IF(!locations || !locations->isTable(), "Invalid location.", return false);

	uint32 locationId = 0;

	if (actIndex == 0)
	{
		actIndex = scenario->getInitialActIndex();
		if (!actIndex) { actIndex = 1; }
	}

	CObject* acts = hl->getAttr("Acts");
	BOMB_IF(!acts || !acts->isTable() || actIndex >= acts->getSize(), "Invalid acts.", return false);

	CObject* selectedAct = acts->getValue(actIndex);
	BOMB_IF(!selectedAct || !selectedAct->isString("LocationId"), "Invalid act.", return false);
	std::string locationInstanceId = selectedAct->toString("LocationId");

	uint32 firstLocationIndex = 0, lastLocationIndex = locations->getSize();
	for (; firstLocationIndex != lastLocationIndex; ++firstLocationIndex)
	{
		CObject* location = locations->getValue(firstLocationIndex);
		if ( location && location->isString("InstanceId") && location->toString("InstanceId") == locationInstanceId )
		{
			locationId = firstLocationIndex;
			break;
		}
	}



	BOMB_IF(locationId >= locations->getSize(), "Invalid location.", return false);



	CObject* firstLocation = locations->getValue(locationId);
	BOMB_IF(!firstLocation || !firstLocation->isTable(), "Invalid location.", return false);

	CObject* entryPointObj = firstLocation->getAttr("EntryPoint");
	CObject* islandName = firstLocation->getAttr("IslandName");
	CObject* seasonObj = firstLocation->getAttr("Season");
	bool locationInfo = !entryPointObj || !islandName || !seasonObj ||
		islandName->isString() || !islandName->isString() || !seasonObj->isString();
	BOMB_IF(!locationInfo, "Invalid Loction infos", return false);

	CScenarioEntryPoints& epManager = CScenarioEntryPoints::getInstance();

	CScenarioEntryPoints::CCompleteIsland * island = epManager.getIslandFromId(islandName->toString());
	BOMB_IF(!island, NLMISC::toString("Session %u No Island '%s'.", sessionId.asInt(), islandName->toString().c_str()), return false);

	CScenarioEntryPoints::CShortEntryPoint *entryPoint = epManager.getEntryPointFromIds(islandName->toString(), entryPointObj->toString());

	if (!entryPoint)
	{
		entryPoint = epManager.getEntryPointFromIds(islandName->toString(), island->EntryPoints[0].Location); //Evil Hack must be removed
	}

	BOMB_IF(!entryPoint, NLMISC::toString("Session %u: No EntryPoint '%s' '%s'.", sessionId.asInt(), islandName->toString().c_str(), entryPointObj->toString().c_str()), return false);

	x = entryPoint->X;
	y = entryPoint->Y;

	if (seasonObj->toString()=="Automatic") { season = 0; }
	else if (seasonObj->toString()=="Spring") { season = 1; }
	else if (seasonObj->toString()=="Summer") { season = 2; }
	else if (seasonObj->toString()=="Autumn" ) { season = 3; }
	else if (seasonObj->toString()=="Winter") { season = 4; }

	return true;

}


TSessionId CServerEditionModule::getLinkedSession(CEditionSession* session) const
{
	TSessionId linkedSessionId = TSessionId(0);
	TCharId ownerCharId = session->getOwnerCharId();
	TOwnerInfos::const_iterator it = _OwnerInfos.find(ownerCharId);
	if (it != _OwnerInfos.end())
	{
		if (it->second.EditionSessionId.asInt() != 0 && session->SessionId != it->second.EditionSessionId )
		{
			linkedSessionId = it->second.EditionSessionId;
		}
		if (it->second.AnimationSessionId.asInt() != 0 && session->SessionId != it->second.AnimationSessionId )
		{
			linkedSessionId = it->second.AnimationSessionId;
		}
	}
	return linkedSessionId;

}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, kickPioneer)
{
	if (args.size() != 2)
	{
		log.displayNL("Wrong usage ServerEditionModule.kickPioneer <sessionId> <charId>");
		return false;
	}

	uint32 sessionId;
	fromString(args[0], sessionId);

	uint32 userId;
	fromString(args[1], userId);

	std::string msg;
	bool success = kickPioneerImpl((TSessionId)sessionId, userId, msg);
	log.displayNL(msg.c_str());

	return success;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, unkickPioneer)
{
	if (args.size() != 2)
	{
		log.displayNL("Wrong usage ServerEditionModule.unkickPioneer <sessionId> <charId>");
		return false;
	}

	uint32 sessionId;
	fromString(args[0], sessionId);

	uint32 userId;
	fromString(args[1], userId);

	std::string msg;
	bool success = unkickPioneerImpl((TSessionId)sessionId, userId, msg);
	log.displayNL(msg.c_str());

	return success;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, listAllowedPioneers)
{
	if (args.size() != 0) return false;

	TPioneersSessionsAlloweds::const_iterator first(_PioneersSessionsAllowed.begin()), last(_PioneersSessionsAllowed.end());
	for ( ; first != last ; ++first)
	{
		log.displayNL("Session %u: chars:%u", first->second.SessionId.asInt(), first->first);
	}
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, listConnectedPioneers)
{
	if (args.size() != 0) return false;


	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, listSessions)
{
	if (args.size() != 0) return false;

	TSessions::const_iterator first(_Sessions.begin()), last(_Sessions.end());
	for ( ; first != last ; ++first)
	{
		TSessionId sessionId = first->first;
		CEditionSession* session = first->second;
		TSessionId linkedSessionId = TSessionId(0);

		TCharId ownerCharId = session->getOwnerCharId();



		TOwnerInfos::const_iterator it = _OwnerInfos.find(ownerCharId);
		if (it != _OwnerInfos.end())
		{
			if (it->second.EditionSessionId.asInt() != 0 && sessionId != it->second.EditionSessionId )
			{
				linkedSessionId = it->second.EditionSessionId;
			}
			if (it->second.AnimationSessionId.asInt() != 0 && sessionId != it->second.AnimationSessionId )
			{
				linkedSessionId = it->second.AnimationSessionId;
			}
		}


		const CEditionSession::TCurrentChars& chars = session->getCurrentChars();
		CEditionSession::TCurrentChars::const_iterator first(chars.begin()), last(chars.end());
		std::string clients;
		for ( ; first != last; ++first)
		{
			if (!clients.empty()) { clients += ", ";}
			clients += toString("%u",(*first)->getCharId());
		}

		log.displayNL("Session %u: type '%s': chars:(%u)[%s], AiInstance:%u Owner:%u %s",
			sessionId.asInt(),
			session->getSessionType().toString().c_str(),
			session->getCurrentChars().size(),
			clients.c_str(),
			session->getAiInstanceId(),
			ownerCharId,
			linkedSessionId.asInt()!=0 ?toString(" Linked to %u", linkedSessionId.asInt()).c_str():"" );

	}
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, closeSession)
{
	if (args.size() < 1)
	{
		log.displayNL("Wrong usage: ServerEditionModule.closeSession <sessionId>");
		return false;
	}
	uint32 sessionId;
	fromString(args[0], sessionId);

	std::string msg;
	closeSessionImpl((TSessionId)sessionId, msg);
	log.displayNL("%s", msg.c_str());
	return true;
}



NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, displaySession)
{
	if (args.size() < 1)
	{
		log.displayNL("Wrong usage: ServerEditionModule.displaySession <sessionId> [<displayLevel>]");
		return false;
	}

	uint32 sId;
	fromString(args[0], sId);

	TSessionId sessionId = (TSessionId)sId;
	uint32 logLevel = 0;
	if (args.size() > 1)
	{
		fromString(args[1], logLevel);
	}


	CEditionSession* session = getSession(sessionId);

	if (session)
	{
		CScenario* scenario = session->getScenario();
		if (! scenario)
		{
			log.displayNL("No scenario in this session");
			return true;
		}


		TCharId ownerCharId = session->getOwnerCharId();
		TSessionId linkedSessionId = getLinkedSession(session);
		log.displayNL("Session %u: chars:%u, AiInstance %u Owner %u %s",
			sessionId.asInt(), session->getCurrentChars().size(), session->getAiInstanceId(),
			ownerCharId, linkedSessionId.asInt()!=0 ?toString(" Linked to %u", linkedSessionId.asInt()).c_str():"" );

		const CEditionSession::TCurrentChars& chars = session->getCurrentChars();
		CEditionSession::TCurrentChars::const_iterator firstChars(chars.begin()), lastChars(chars.end());
		for ( ; firstChars != lastChars; ++firstChars)
		{
			log.displayNL("Chars: %u , EditSlot: %u:%u", (*firstChars)->getCharId(), sessionId.asInt(), (*firstChars)->getEditSlotId());
		}

		CObject* hl = scenario->getHighLevel();
		if (hl)
		{


			if (logLevel != 1)
			{
				log.displayNL("HighLevel data: Use logLevel at 1 for more info");
			}
			else
			{
				//std::stringstream ss;
				std::string ss;
				hl->serialize(ss);
				log.displayNL("HighLevel data:");
				std::vector<std::string> lines;
				NLMISC::splitString(ss, "\n", lines);
				uint first=0, last=(uint)lines.size();
				for (; first != last ; ++first) {	log.displayNL("%s", lines[first].c_str()); }
			}
		}
		else
		{
			log.display("No HighLevel data");
		}

		R2::CObject* rt = scenario->getRtData();
		if (rt)
		{
			if (logLevel != 2)
			{
				log.displayNL("RealTime data: Use logLevel to 2 for more info");
			}
			else
			{
				//std::stringstream ss;
				std::string ss;
				rt->serialize(ss);
				log.displayNL("RealTime data:");

				std::vector<std::string> lines;
				NLMISC::splitString(ss, "\n", lines);
				uint first=0, last=(uint)lines.size();
				for (; first != last ; ++first) {	log.displayNL("%s", lines[first].c_str()); }
			}

		}
		else
		{
			log.displayNL("No Rt Data");
		}

	}

	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, dumpSession)
{
	if (args.size() < 1)
	{
		log.displayNL("Wrong usage: ServerEditionModule.displaySession <sessionId> [<displayLevel>]");
		return false;
	}

	uint32 sId;
	fromString(args[0], sId);

	TSessionId sessionId = (TSessionId)sId;
	uint32 logLevel = 0;
	if (args.size() > 1)
	{
		fromString(args[1], logLevel);
	}


	CEditionSession* session = getSession(sessionId);

	if (session)
	{
		CScenario* scenario = session->getScenario();
		if (! scenario)
		{
			log.displayNL("No scenario in this session");
			return true;
		}


		TCharId ownerCharId = session->getOwnerCharId();
		TSessionId linkedSessionId = getLinkedSession(session);
		log.displayNL("Session %u: chars:%u, AiInstance %u Owner %u %s",
			sessionId.asInt(), session->getCurrentChars().size(), session->getAiInstanceId(),
			ownerCharId, linkedSessionId.asInt()!=0 ?toString(" Linked to %u", linkedSessionId.asInt()).c_str():"" );

		const CEditionSession::TCurrentChars& chars = session->getCurrentChars();
		CEditionSession::TCurrentChars::const_iterator firstChars(chars.begin()), lastChars(chars.end());
		for ( ; firstChars != lastChars; ++firstChars)
		{
			log.displayNL("Chars: %u , EditSlot: %u:%u", (*firstChars)->getCharId(), sessionId.asInt(), (*firstChars)->getEditSlotId());
		}

		CObject* hl = scenario->getHighLevel();
		if (hl)
		{


			if (logLevel != 1)
			{
				log.displayNL("HighLevel data: Use logLevel at 1 for more info");
			}
			else
			{
				//std::stringstream ss;
				std::string ss;
				hl->serialize(ss);
				log.displayNL("HighLevel data:");
				std::vector<std::string> lines;
				NLMISC::splitString(ss, "\n", lines);
				uint first=0, last=(uint)lines.size();
				for (; first != last ; ++first) {	log.displayNL("%s", lines[first].c_str()); }
			}
		}
		else
		{
			log.display("No HighLevel data");
		}

		R2::CObject* rt = scenario->getRtData();
		if (rt)
		{
			if (logLevel != 2)
			{
				log.displayNL("RealTime data: Use logLevel to 2 for more info");
			}
			else
			{
				//std::stringstream ss;
				std::string ss;
				rt->serialize(ss);
				log.displayNL("RealTime data:");

				std::vector<std::string> lines;
				NLMISC::splitString(ss, "\n", lines);
				uint first=0, last=(uint)lines.size();
				for (; first != last ; ++first) {	log.displayNL("%s", lines[first].c_str()); }
			}

		}
		else
		{
			log.displayNL("No Rt Data");
		}

	}

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, listPioneers)
{
	if (args.size() != 0 ) return false;

	TPioneersSessions::const_iterator first(_PioneersSessions.begin()), last(_PioneersSessions.end());

	for ( ; first != last; ++first)
	{
		log.displayNL("Chars: %u , Session: %u", first->first, first->second.asInt());
	}
	return true;

}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, displayPioneer)
{
	if (args.size() != 1) return false;

	TCharId charId;
	fromString(args[0], charId);

	TPioneersSessions::const_iterator found(_PioneersSessions.find(charId));

	if (found == _PioneersSessions.end())
	{
		log.displayNL("No Pioneer with this char id");
		return false;
	}

	TSessionId sessionId = found->second;

	CEditionSession * session = getSession( sessionId );

	if (!session)
	{
		nlwarning("R2Ed: Error a Pioneer(%u) belongs to a Session(%u), but the session seems to not exist", charId, sessionId.asInt());
		return false;
	}

	const CEditionSession::TCurrentChars& chars = session->getCurrentChars();
	CEditionSession::TCurrentChars::const_iterator firstChars(chars.begin()), lastChars(chars.end());

	for ( ; firstChars != lastChars && (*firstChars)->getCharId() != charId; ++firstChars ){}

	if (firstChars == lastChars)
	{
		nlwarning("R2Ed: Error a Pioneer(%u) belongs to a Session(%u), but the session does not hold the Pioneer", found->first, found->second.asInt());
		return false;
	}

	log.displayNL("'%s' '%s'",
		NLMISC::toString("Chars: %u, IsConnected %u", found->first, (getClientProxyPtr(found->first) != 0)).c_str(),
		NLMISC::toString("EditSlot: %u:%u", found->second.asInt(), (*firstChars)->getEditSlotId()).c_str()
	);
	return true;

}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, resetSession)
{
	if (args.size() < 1)
	{
		log.displayNL("Wrong usage: ServerEditionModule.resetSession <sessionId>");
		return false;
	}
	uint32 sessionId;
	fromString(args[0], sessionId);
	this->resetSession(0, (TSessionId)sessionId, false);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, reconnectSession)
{
	if (args.size() < 1)
	{
		log.displayNL("Wrong usage: ServerEditionModule.reconnectSession <sessionId>");
		return false;
	}
	uint32 sessionId;
	fromString(args[0], sessionId);
	this->resetSession(0, (TSessionId)sessionId, true);
	return true;
}



NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, setCharMode)
{
	if (args.size() != 2)
	{
		return false;
	}

	TUserId charId;
	fromString(args[0], charId);
	TCharMode charMode(args[1]);


	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);

 // use Boris Magic TUserId will be TCharId
	if (!clientEditionProxyPtr)
	{
		log.displayNL("No such character %d", charId);
		return false;
	}

	CShareClientEditionItfProxy proxy(*clientEditionProxyPtr);;
	proxy.onCharModeUpdated(this, charMode);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, setCharRingAccess)
{
	if (args.size() != 2)
	{
		log.displayNL("wrong usage ServerEditionModule.setCharRingAccess <charId> <RingAccess>");
		return false;
	}

	TUserId charId;
	fromString(args[0], charId);
	std::string newRingAccess = args[1];

	TPionnerInfos::iterator found( _PioneersInfo.find(charId) );

	if ( found == _PioneersInfo.end())
	{
		log.displayNL("Char not found");
		return false;
	}

	found->second.RingAccess =  newRingAccess;

	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);

 // use Boris Magic TUserId will be TCharId
	if (!clientEditionProxyPtr)
	{
		log.displayNL("No such character %d", charId);
		return false;
	}

	CShareClientEditionItfProxy proxy(*clientEditionProxyPtr);
	proxy.onRingAccessUpdated(this, newRingAccess);
	log.displayNL("Ring Access changed to '%s' for user %u", newRingAccess.c_str(),charId);
	return true;
}

NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, displayCharRingAccess)
{
	if (args.size() != 1)
	{
		log.displayNL("wrong usage ServerEditionModule.displayCharRingAccess <charId>");
		return false;
	}

	TUserId charId;
	fromString(args[0], charId);

	TPionnerInfos::const_iterator found( _PioneersInfo.find(charId) );

	if ( found == _PioneersInfo.end())
	{
		log.displayNL("Char not found");
		return false;
	}

	log.displayNL("Char %u: RingAccess='%s'", charId, found->second.RingAccess.c_str());

 	return true;
}


void CServerEditionModule::onCharModeUpdateAsked(NLNET::IModuleProxy *senderModuleProxy, TCharMode mode)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	nldebug("R2Ed: Char %u is now in mode '%s'", charId, mode.toString().c_str());


	bool isDm = 0;
	if ( mode == TCharMode::Tester || mode == TCharMode::Player)
	{
		isDm = 0;
		CShareClientEditionItfProxy proxy(senderModuleProxy);
		proxy.onCharModeUpdated(this, mode);
		getAnimationModule()->stopControlNpcs(charId);
	}

	if ( mode == TCharMode::Editer || mode == TCharMode::Dm)
	{
		isDm = 1;
		CShareClientEditionItfProxy proxy(senderModuleProxy);
		proxy.onCharModeUpdated(this, mode);
	}

	if (_CharacterControlProxy)
	{
		CCharacterControlItfProxy proxy(_CharacterControlProxy);
		proxy.setPioneerRight(this, charId, isDm);
	}



	TPionnerInfos::iterator found = _PioneersInfo.find(charId);
	if (found == _PioneersInfo.end())
	{
		nlwarning("Try to do an action on a non logged character");
		return;
	}
	found->second.Mode = mode;


}

void CServerEditionModule::updateCharPioneerRight(TCharId charId)
{
	TPionnerInfos::const_iterator found = _PioneersInfo.find(charId);
	if (found == _PioneersInfo.end())
	{
		nlwarning("Try to do an action on a non logged character");
		return;
	}

	TCharMode mode = found->second.Mode;
	if (mode == TCharMode::Player || mode == TCharMode::Tester)
	{
		// Only reseted for DM
	/*	if (_CharacterControlProxy)
		{
			CCharacterControlItfProxy proxy(_CharacterControlProxy);
			proxy.setPioneerRight(this, charId, 0);
		}
	*/
	}
	else if (mode == TCharMode::Editer || mode == TCharMode::Dm)
	{
		if (_CharacterControlProxy)
		{
			CCharacterControlItfProxy proxy(_CharacterControlProxy);
			proxy.setPioneerRight(this, charId, 1);
		}
	}
	else
	{
		nlwarning("ERROR: should not be here");

	}

}

void CServerEditionModule::characterReady(TCharId charId)
{
	// Check for editor or animator character that need to be added in ring universe chat.

	// if character is owner of the session, ask IOS to put it
	// in the ring universe (witch contains all DM, editor and
	// anim launchers)
	CEditionSession *session = getSessionByCharId(charId);
	if (session == NULL)
	{
		nlwarning("CServerEditionModule::characterReady : char %u is not associated to any session", charId);
		return;
	}

	if (session->getOwnerCharId() == charId && _IOSRingProxy != NULL)
	{
		// ok, he owns, add it to universe

		nldebug("CServerEditionModule::characterReady : subscribing the character %u to universe chat with EGS '%s'", charId, _CharacterControlProxy->getModuleName().c_str());
		CCharacterControlItfProxy proxy(_CharacterControlProxy);
		proxy.subscribeCharacterInRingUniverse(this, charId);
	}

	// Indicates the EGS that a player connect in animation session (for reward calculation)
	if (session->getSessionType() == RSMGR::TSessionType::st_anim)
	{
		if (_CharacterControlProxy)
		{
			CCharacterControlItfProxy proxy(_CharacterControlProxy);
			proxy.charJoinAnimSession(this, charId, session->SessionId);
		}
	}

}


IServerAnimationModule* CServerEditionModule::getAnimationModule() const
{
	return _Server->getAnimationModule();
}

bool CServerEditionModule::isInitialized() const
{
	// Local mode (no network)
	if (!CDynamicMapService::getInstance()->useNetwork()){ return true; }

	if (DssUseBs) { return _CharacterControlProxy != 0 && !_WaitingForBS;}


	return _CharacterControlProxy != 0;
}


static std::string getSecondesAsString(uint32 secondes)
{
	uint32 s = secondes % 60;
	secondes /= 60;
	uint32 m = secondes %60;
	secondes /= 60;
	uint32 h = secondes / 24;
	secondes /= 24;
	uint32 d = secondes;

	std::string ret;
	if (d) { ret += NLMISC::toString("%u day%s ", d, d>1?"s":""); }
	if (h) { ret += NLMISC::toString("%u hour%s ", h, h>1?"s":""); }
	if (m) { ret += NLMISC::toString("%u minute%s ", m, m>1?"s":""); }
	if (s) { ret += NLMISC::toString("%u seconde%s ", s, s>1?"s":""); }
	return ret;
}

void CServerEditionModule::onModuleUpdate()
{
	H_AUTO(CServerEditionModule_onModuleUpdate);

	if (DssUseBs && _BsGoingUp)
	{
			nldebug("BSI requesting files: '%s'",getOverrideRingAccessFilename().c_str());
			Bsi.requestFile(getOverrideRingAccessFilename(), new COverrideRingAccessCallback(this));
			_BsGoingUp = false;
	}

	if ( !isInitialized() ) { return; }




	// close, hibernate, stop, save session
	{
		NLMISC::TTime now =  NLMISC::CTime::getLocalTime();


		_Tasks.execute(now);

		static NLMISC::TTime lastCheck = now;
		//
		if ( now - lastCheck > 1000)
		{
			lastCheck = now; //every second

			uint32 nowSeconds = NLMISC::CTime::getSecondsSince1970();
			std::set<TSessionId> sessionIds;
			// We cut branch while we are sitting on it
			// !!!! When we close an edition session, it is possible that the system also close the linked animation session.
			// So we create a temporary list of ids and we iterate through this list

			{
				TSessions::const_iterator first(_Sessions.begin()), last(_Sessions.end());
				for (; first != last ; ++first)
				{
					sessionIds.insert(first->first);
				}
			}

			std::set<TSessionId>::iterator first(sessionIds.begin()), last(sessionIds.end());
			for ( ;first !=  last; ++first )
			{

				TSessionId sessionId = *first;
				CEditionSession* session = getSession(sessionId);
				if (session)
				{

					session->update(nowSeconds);

					TSessionId linkedSessionId = getLinkedSessionId(sessionId);
					CEditionSession* linkedSession = getSession(linkedSessionId);
					if ( linkedSession && !linkedSession->getCurrentChars().empty() )
					{
						session->touch();
					}


					if ( session->mustHibernate(nowSeconds))
					{
						std::string msg;
						nlinfo("Hibernating Edition Session %u (because no player since %s)", sessionId.asInt(), getSecondesAsString(TimeBeforeCloseHibernatingEditionSession).c_str() );
						bool ok = hibernateSessionImpl(sessionId, msg);
						debugInfo(ok, msg);
						// session is remove from _Sessions here
					}
					else if (session->mustClose(nowSeconds) )
					{
						nlinfo("Closing Animation Session %u (because no player since %s)", sessionId.asInt(), getSecondesAsString(TimeBeforeAutoCloseAnimationSessionWithNoPlayer ).c_str() );
						closeSession(0, sessionId);
						// session is remove from _Sessions here
					}
					else if (session->mustStopAnimation(nowSeconds) && getLinkedSessionId(sessionId).asInt() == 0)
					{

						nlinfo("Stop test in Edition Session %u (because no player since %s)", sessionId.asInt(), getSecondesAsString(TimeBeforeStopTestInEditionSessionWithNoPlayer ).c_str() );
						stopTest(sessionId);
					}
					else if (session->mustSave(nowSeconds))
					{
						std::string msg;
						nlinfo("Save edition Session %u (because scenario changed and was not save since %s)", sessionId.asInt(), getSecondesAsString(TimeBeforeAutoSaveTimeEditionSession ).c_str() );
						bool ok = saveSessionImpl(sessionId, msg);
						debugInfo(ok, msg);
					}
				}
			}
		}
	}

	{
		NLMISC::TTime now = NLMISC::CTime::getLocalTime();
		static NLMISC::TTime lastCheck = now;
		if (lastCheck + 30*1000 < now) // sync with file every 30 secondes
		{
			lastCheck = now;
			updateSessionListFile(); // update session file list if necessary
		}
	}


	{
		NLMISC::TTime now = NLMISC::CTime::getLocalTime();
		static NLMISC::TTime lastCheck = now;
		if (lastCheck + 1*1000 < now) //Update kick list every second
		{
			lastCheck = now;

			TKicked::iterator first(_Kicked.begin()), last(_Kicked.end());
			for ( ;first != last; )
			{
				if (now  - first->second > 30*1000) // remove from kick list 30 secondes
				{

					TKicked::iterator dead = first;
					++first;
					_Kicked.erase(dead);
				}
				else
				{
					++first;
				}


			}
		}
	}


}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, hibernateSession)
{
	if (args.size() != 1)
	{
		log.displayNL("Wrong parameters: 1 parameter needed : the Session to hibernate");
		return false;
	}

	uint32 sessionId;
	fromString(args[0], sessionId);
	std::string msg;
	hibernateSessionImpl((TSessionId)sessionId, msg);
	log.displayNL("%s", msg.c_str());
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, wakeUpSession)
{
	if (args.size() != 2)
	{
		log.displayNL("Wrong parameters: 1 parameter needed: the Session to hibernate");
		return false;
	}

	uint32 sessionId;
	fromString(args[0], sessionId);
	TCharId charId;
	fromString(args[1], charId);
	std::string msg;
	wakeUpSession((TSessionId)sessionId, charId, msg);
	log.displayNL("%s", msg.c_str());
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, listCharOverrideRingAccess)
{

	if (args.size() != 0)
	{
		log.displayNL("Wrong parameters");
		return false;
	}

	log.displayNL("There is %u  char that have overridden ring access session", _OverrideRingAccess.size());
	TOverrideRingAccess::const_iterator first(_OverrideRingAccess.begin()), last(_OverrideRingAccess.end());
	for (; first != last ; ++first)
	{
		log.displayNL("Overridden ring access charId %u, access='%s'", first->first, first->second.c_str());
	}
	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, displayCharOverrideRingAccess)
{

	if (args.size() != 1)
	{
		log.displayNL("Wrong parameters CServerEditionModule.displayCharOverrideRingAccess <charId>");
		return false;
	}

	TCharId charId;
	fromString(args[0], charId);

	TOverrideRingAccess::const_iterator found = _OverrideRingAccess.find(charId);
	if ( found != _OverrideRingAccess.end())
	{
		log.displayNL("Overridden ring access charId %u, access='%s'", found->first, found->second.c_str());
	}
	else
	{
		log.displayNL("The char %u has no overridden ring access", charId);
	}

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, setCharOverrideRingAccess)
{

	if (args.size() != 2)
	{
		log.displayNL("Wrong parameters CServerEditionModule.removeCharOverrideRingAccess <charId> <newValue>");
		return false;
	}

	TCharId charId;
	fromString(args[0], charId);

	std::string newRingAccess(args[1]);

	{
		TOverrideRingAccess::iterator found = _OverrideRingAccess.find(charId);
		if ( found != _OverrideRingAccess.end())
		{
			log.displayNL("The Overridden ring access charId %u, access='%s' has been been change to '%s'", found->first, found->second.c_str(), newRingAccess.c_str());
			found->second = newRingAccess;
		}
		else
		{
			_OverrideRingAccess.insert( std::make_pair(charId, newRingAccess));
			log.displayNL("The char %u has now a overridden ring access '%s'", charId, newRingAccess.c_str());
		}
	}

	{
		TPionnerInfos::iterator found( _PioneersInfo.find(charId) );
		if ( found != _PioneersInfo.end())
		{
			found->second.RingAccess =  newRingAccess;
		}

	}

	{
		const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
		if (clientEditionProxyPtr)
		{
			CShareClientEditionItfProxy proxy(*clientEditionProxyPtr);
			proxy.onRingAccessUpdated(this, newRingAccess);
		}
	}
	_MustUpdateOverrideRingAcess = true;

	return true;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, removeCharOverrideRingAccess)
{

	if (args.size() != 1)
	{
		log.displayNL("Wrong parameters CServerEditionModule.removeCharOverrideRingAccess <charId>");
		return false;
	}

	TCharId charId;
	fromString(args[0], charId);

	TOverrideRingAccess::iterator found = _OverrideRingAccess.find(charId);
	if ( found != _OverrideRingAccess.end())
	{
		log.displayNL("The Overridden ring access charId %u, access='%s' has been removed", found->first, found->second.c_str());
		_OverrideRingAccess.erase(found);

	}
	else
	{
		log.displayNL("The char %u has no overridden ring access", charId);
	}

	_MustUpdateOverrideRingAcess = true;
	return true;
}



class CWakeUpHibernatingSessionCallback : public IBackupFileReceiveCallback
{
public:
	CWakeUpHibernatingSessionCallback(CServerEditionModule* editionModule) :_EditionModule(editionModule){}

	// call back for bs file asynchronous read
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		nldebug("BSIcb CWakeUpHibernatingSessionCallback::callback() for file: %s",fileDescription.FileName.c_str());

		if ( fileDescription.FileSize == 0)
		{
			nlwarning("ERROR: try to wakeup a session that does not exit or has been deleted" );
			return;
		}
		// Serial the Edition Session
		CEditionSession* sessionPtr = new CEditionSession() ;
		sessionPtr->serial(dataStream); // danger AIIntanceisNotYetSet
		//AiInstance Is not yet set
  		_EditionModule->wakeUpSessionImpl(sessionPtr);
	}

	CServerEditionModule* _EditionModule;
};


bool CServerEditionModule::wakeUpSession(TSessionId sessionId, TCharId ownerCharId, std::string& msg)
{
	if (!DssUseBs)
	{
		msg = "DssUseBs == false so we do not wake up sessions";
		return false;
	}


	msg = NLMISC::toString("waking up the session %u", sessionId.asInt());

	if (!CDynamicMapService::getInstance()->useNetwork()){ return true; }
	std::string filename = getSessionFilename(sessionId, ownerCharId);
	nldebug("WAKING session %u - BSI requesting file: %s",sessionId.asInt(),filename.c_str());
	BsiGlobal.requestFile(std::string("r2/sessions/")+filename, new CWakeUpHibernatingSessionCallback(this));

	return true;
}


void CServerEditionModule::wakeUpSessionImpl(CEditionSession* session)
{
	std::auto_ptr<CEditionSession> sessionPtr(session);
	TSessionId sessionId = sessionPtr->SessionId;

	TSessions::iterator found = _Sessions.find(sessionId);

	if (found != _Sessions.end())
	{

		CEditionSession* previous = found->second ;
		nlassert( session != previous);
		sessionPtr->swap( *previous);
		session = previous;
	}
	else
	{
		// new Session
		TAiInstanceId aiInstanceId = _IdRecycle->newId();
		session->setAiInstanceId(aiInstanceId);
		bool ok = _Sessions.insert( std::make_pair( sessionId, sessionPtr.release() ) ).second;
		nlassert(ok && "The insertion must have success");

		TCharId ownerCharId = session->getOwnerCharId();
		TOwnerInfos::iterator owner = _OwnerInfos.find(ownerCharId);
		if (owner == _OwnerInfos.end()) // launch a animation from the born
		{
			owner = _OwnerInfos.insert( std::make_pair(ownerCharId, TOwnerInfo(sessionId,TSessionId(0), aiInstanceId) ) ).first; // create an edition Session
		}
		else
		{
			owner->second.AiInstanceId = aiInstanceId;
			owner->second.EditionSessionId = sessionId;
		}

	}
	session->setSessionType(RSMGR::TSessionType::st_edit);
	session->setLoaded(true);

}


bool CServerEditionModule::saveSessionImpl(TSessionId sessionId, std::string& msg)
{

	//
	// The session must be empty



	TSessions::iterator found = _Sessions.find(sessionId);

	if ( found == _Sessions.end())
	{
		msg = NLMISC::toString("Try to hibernate a non existing session %u", sessionId.asInt());
		return false;
	}

	CEditionSession* session = found->second;
	nlassert(session);

//	uint32 hibernationDate = NLMISC::CTime::getSecondsSince1970();
	RSMGR::TSessionType rsmSessionType = session->getSessionType();
	uint32 savedTime = NLMISC::CTime::getSecondsSince1970();
	if (rsmSessionType != RSMGR::TSessionType::st_edit)
	{
		msg = "Can only save edition Session";
		return false;
	}
	session->saved(savedTime);

	_MustUpdateHibernatingFileList = true;


	// There must not be any animation
	if (!DssUseBs)
	{
		msg = "DssUseBs == false so we do not save session";
		return false;
	}
	uint32 owner = session->getOwnerCharId();
	{
		nldebug("SAVING session %u - BSI send file: %s",sessionId.asInt(), getSessionFilename(sessionId, owner).c_str());
		CBackupMsgSaveFile bsMsg( std::string("r2/sessions/")+ getSessionFilename(sessionId, owner), CBackupMsgSaveFile::SaveFileCheck, BsiGlobal );
		bsMsg.DataMsg.serial(*session);
		BsiGlobal.sendFile( bsMsg );
	}

	msg = NLMISC::toString("Save session %u", sessionId.asInt());
	return true;
}


bool CServerEditionModule::hibernateSessionImpl  (TSessionId sessionId, std::string& msg)
{
	bool ret = true;
	//
	// The session must be empty
	// There must not be any animation
	if (!DssUseBs)
	{
		msg = "DssUseBs == false so we do not hibernate session";
		return false;
	}


	TSessions::iterator found = _Sessions.find(sessionId);

	CEditionSession* session = found->second;
	nlassert(session);
	TAiInstanceId aiInstanceId = session->getAiInstanceId();
	TSessionId animationSessionId=0;
	if (session && session->getSessionType() == RSMGR::TSessionType::st_edit)
	{
		TCharId ownerCharId = session->getOwnerCharId();
		TOwnerInfos::iterator it=_OwnerInfos.find(ownerCharId);
		if (it != _OwnerInfos.end())
		{
			animationSessionId = it->second.AnimationSessionId ;
			if (animationSessionId)
			{
				msg += "Closing linked anim session.\n";
				bool ok = closeSessionImpl(animationSessionId, msg);
				if ( ret) { ret = ok; }
			}
		}

	}

	// Suppress the link to the edition/anim session otherwise owner will return to edition instead of previous
	if (session)
	{
		uint32 ownerCharId = session->getOwnerCharId();
		TOwnerInfos::iterator it=_OwnerInfos.find(ownerCharId);
		if (it != _OwnerInfos.end())
		{
			if (session->getSessionType() == RSMGR::TSessionType::st_edit)
			{
				it->second.EditionSessionId = TSessionId(0);
			}
			else if (session->getSessionType() == RSMGR::TSessionType::st_anim)
			{
				it->second.AnimationSessionId = TSessionId(0);
			}
			if (it->second.EditionSessionId == TSessionId(0) && it->second.AnimationSessionId == TSessionId(0))
			{
				_OwnerInfos.erase(it);
			}
		}
	}




	//(*) remove player from connecting queue
	// This very small map contains player that are connecting to dss (allowed by SU to connect but not yet connected because they are loading data)
	TPioneersSessionsAlloweds::iterator first(_PioneersSessionsAllowed.begin()), last(_PioneersSessionsAllowed.end());
	for (; first != last;)
	{
		TPioneersSessionsAlloweds::iterator copy = first++;
		if (copy->second.SessionId == sessionId)
		{
			_PioneersSessionsAllowed.erase(copy);
		}
	}

	//(*) Remove connected Player (It is the SU choice)

	std::set<TCharId> charIds;
	// Add charId connected in edition module
	{
		const CEditionSession::TCurrentChars& charIdContainer = session->getCurrentChars();
		CEditionSession::TCurrentChars::const_iterator first(charIdContainer.begin()), last(charIdContainer.end());
		for (; first != last ; ++first)
		{
			charIds.insert( (*first)->getCharId() );
		}
	}

	bool animationSessionExist = false;
	// Add charId connected in animation Module
	{
		std::vector<TCharId> connectedChars;
		animationSessionExist = getAnimationModule()->getConnectedChars(sessionId, connectedChars);
		std::vector<TCharId>::const_iterator first( connectedChars.begin()), last(connectedChars.end());
		for ( ; first != last ; ++first)
		{
			charIds.insert(*first);
		}
	}

	// Kick all connected character
	{

		std::set<TCharId>::iterator first(charIds.begin()), last(charIds.end());
		for ( ; first != last; ++first)
		{
			std::string outMsg;
			bool ok = removeCharacterFromSessionImpl(sessionId, *first, outMsg);
			debugInfo(ok, outMsg);
		}
	}

	//(*) Remove animation Session and kill AI Instance
	if (animationSessionExist)
	{

		stopTest(sessionId);
	}

	uint32 hibernationDate = NLMISC::CTime::getSecondsSince1970();
	RSMGR::TSessionType rsmSessionType = session->getSessionType();

	_MustUpdateHibernatingFileList = true;

	double positionX, positionY, orient;
	uint8 season;
	uint32 actId = session->getScenario()->getInitialActIndex();
	uint32 ownerCharId = session->getOwnerCharId();
	getPosition(sessionId, positionX, positionY, orient, season, actId);
	nldebug("ADD HIBERNATE SESSION: CServerEditionModule::hibernateSessionImpl(): sessionId=%u ",sessionId.asInt());
	_HibernatingSessions[sessionId] = THibernatingSession( hibernationDate, rsmSessionType, positionX, positionY, orient, season );

	{
		nldebug("HIBERNATE - SAVING session %u - BSI send file: %s",sessionId.asInt(),getSessionFilename(sessionId, ownerCharId).c_str());
		CBackupMsgSaveFile bsMsg( std::string("r2/sessions/")+getSessionFilename(sessionId, ownerCharId), CBackupMsgSaveFile::SaveFileCheck, BsiGlobal );
		bsMsg.DataMsg.serial(*session);
		BsiGlobal.sendFile( bsMsg );
	}

	msg = NLMISC::toString("Save session %u", sessionId.asInt());


	delete session;

	_Sessions.erase(found);
	_IdRecycle->releaseId( aiInstanceId);
	nldebug("Session %u destroy AiInstance %u", sessionId.asInt(), aiInstanceId );
	CAiWrapper::getInstance().stopTest(sessionId, aiInstanceId);
	msg = NLMISC::toString("Sucess: Session %u is now hibernating", sessionId.asInt());

	nldebug("Send to RSMG that session is hibernating %u", sessionId.asInt());

	if (_SessionManager)
	{
		//( 5 ) report the session closing event
		RSMGR::CRingSessionManagerProxy rsm(_SessionManager);
		rsm.reportSessionEvent(this, RSMGR::TSessionEvent::se_session_closing, sessionId, 0);
	}
	return ret;
}


// Update list of hibernating Session
void CServerEditionModule::release()
{
	if ( !CDynamicMapService::getInstance()->useNetwork() ) { return; }

	if (DssUseBs)
	{
		// don't erase current file if server restart before bs update data
		if (!_WaitingForBS )
		{
			std::set<TSessionId> sessions;
			{
				TSessions::iterator first(_Sessions.begin()), last(_Sessions.end());
				for (; first != last; ++first)
				{
					// do not hibernate if animation session
					if (first->second->getSessionType() == RSMGR::TSessionType::st_edit)
					{
						sessions.insert(first->first);
					}
				}
			}

			// don't destroy the tree we are walking through
			{
				std::set<TSessionId>::iterator first(sessions.begin()), last(sessions.end());
				for (; first != last; ++first)
				{
					std::string msg;
					bool ok = hibernateSessionImpl(*first, msg);
					debugInfo(ok, msg);

				}
			}
			_MustUpdateHibernatingFileList = true;
			// update the session_list.txt
			updateSessionListFile();
		}

	}

	_IdRecycle.release();

	delete _KeysHolder;
	_KeysHolder = 0;
}


void CServerEditionModule::updateSessionListFile()
{
	if (!DssUseBs) { return; }

	if (_MustUpdateHibernatingFileList)
	{

		if (_R2SessionBackupModule)
		{
			std::vector<TSessionId> sessionIds;
			THibernatingSessions::const_iterator first( _HibernatingSessions.begin() ), last( _HibernatingSessions.end() );
			for (; first != last; ++first) { sessionIds.push_back(first->first); }

			CR2SessionBackupModuleItfProxy proxy(_R2SessionBackupModule);
			proxy.reportHibernatedSessions(this, sessionIds);
		}
		_HibernatingSessions.clear();
		_MustUpdateHibernatingFileList = false;
	}


	if (_R2SessionBackupModule)
	{
		TSessions::const_iterator first( _Sessions.begin() ), last( _Sessions.end() );
		std::vector<TR2SbmSessionInfo> infos;
		for (; first != last; ++first)
		{
			uint32 lastSave = first->second->getLastSaveTime();
			if (lastSave > 0)
			{
				TR2SbmSessionInfo info;
				info.setSessionId(first->first);
				info.setDateEmpty(lastSave);
				infos.push_back(info);
			}
		}
		CR2SessionBackupModuleItfProxy proxy(_R2SessionBackupModule);
		proxy.reportSavedSessions(this, infos);
	}

	if (!_ClosedSessions.empty())
	{
		std::vector<TSessionId> sessionIds;
		TClosedSessions::iterator first(_ClosedSessions.begin()), last(_ClosedSessions.end());
		for (; first != last; ++first)
		{
			TSessionId sessionId = first->first;
			TCharId ownerCharId = first->second;
			nldebug("CLEANING closed session - BSI delete file: %s",getSessionFilename(sessionId, ownerCharId).c_str());
			BsiGlobal.deleteFile( std::string("r2/sessions/")+getSessionFilename(sessionId, ownerCharId), false);
			sessionIds.push_back(sessionId);
		}

		if (_R2SessionBackupModule)
		{
			CR2SessionBackupModuleItfProxy proxy(_R2SessionBackupModule);
			proxy.reportDeletedSessions(this, sessionIds);
		}
		_ClosedSessions.clear();
	}

	if (_MustUpdateOverrideRingAcess)
	{
		TOverrideRingAccess::const_iterator first( _OverrideRingAccess.begin() ), last( _OverrideRingAccess.end() );

//		std::stringstream ss;
//		ss <<  toString("Version\t%u\n", FileVersion );
//		for (; first != last; ++first)
//		{
//			std::string tmp = NLMISC::toString("%d\t%s\n", first->first, first->second.c_str());
//			ss << tmp;
//
//		}
//		std::string copy = ss.str();


		std::string copy;
		copy += toString("Version\t%u\n", FileVersion );
		for (; first != last; ++first)
			copy += NLMISC::toString("%d\t%s\n", first->first, first->second.c_str());

		nldebug("UPDATING ring access override file list - BSI send file: %s",getOverrideRingAccessFilename().c_str());
		CBackupMsgSaveFile msg( getOverrideRingAccessFilename(), CBackupMsgSaveFile::SaveFileCheck, Bsi );
		msg.DataMsg.serialBuffer((uint8*)copy.c_str(), (uint)copy.size());
		Bsi.sendFile( msg );
		_MustUpdateOverrideRingAcess = false;
	}
}


void CServerEditionModule::onServiceDown(const std::string &serviceName, NLNET::TServiceId serviceId)
{

	if (serviceName == "AIS")
	{
		nlinfo("AIS is down so close all animation session and stop all Test");
		TSessions::iterator first(_Sessions.begin()), last(_Sessions.end());
		std::set<TSessionId> toClose;
		std::set<TSessionId> toStop;
		// DO NOT close session while going through session list.
		for ( ;first != last ; ++first)
		{
			if (first->second->getSessionType() == RSMGR::TSessionType::st_edit)
			{
				toStop.insert(first->first);
			}
			else if (first->second->getSessionType() == RSMGR::TSessionType::st_anim)
			{
				toClose.insert(first->first);
			}
			else
			{
				nlassert(0 && "outland sessions not handle yet");
			}
		}

		nlinfo("AIS is down so close all animation session");
		{
			std::set<TSessionId>::iterator first(toClose.begin()), last(toClose.end());
			for (; first != last; ++first)
			{
				std::string msg;
				bool ok = closeSessionImpl(*first, msg);
				debugInfo(ok, msg);
			}
		}

		nlinfo("AIS is down so close all edit session");
		{

			std::set<TSessionId>::const_iterator first(toStop.begin()), last(toStop.end());
			for (; first != last; ++first)
			{
				stopTest(*first);
			}
		}

		// close all anim_session
		// stopTest all edit_session
	}

}


void CServerEditionModule::onServiceUp(const std::string &serviceName, NLNET::TServiceId serviceId)
{
	if (serviceName == "BS")
	{
		this->_BsGoingUp = true;
	}
}


bool CServerEditionModule::isClientAuthorized(TCharId charId) const
{
	return getClientProxyPtr(charId) != NULL;
}


const NLNET::TModuleProxyPtr * CServerEditionModule::getClientProxyPtr(TCharId charId) const
{
	return _ClientsEditionModule.getB(charId);
}


void CServerEditionModule::getTpContext(TCharId charId, std::string& tpCancelTextId, R2::TTeleportContext& tpContext)
{
	tpCancelTextId = "";
	tpContext = R2::TPContext_Unknown;

	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session =getSession(sessionId);
	if (!session) { return; }

	// If player reconnect after test mode the test mode is stopped
	// Add charId connected in animation Module
	bool animationSessionExist = false;
	{
		std::vector<TCharId> connectedChars;
		animationSessionExist = getAnimationModule()->getConnectedChars(sessionId, connectedChars);
	}

	bool isLive = getLinkedSessionId(sessionId) != 0 || session->getSessionType() == RSMGR::TSessionType::st_anim;

	bool isOwner = session->getOwnerCharId() == charId;


	if (animationSessionExist)
	{
		if(isLive)
		{
			if(isOwner)
			{
				tpCancelTextId = "uiR2EDTPContextCancelIslandOwner";
				tpContext = R2::TPContext_IslandOwner;
			}
			else
			{
				tpCancelTextId = "uiR2EDTPContextCancelMainland";
				tpContext = R2::TPContext_Mainland;
			}
		}
		else
		{
			tpCancelTextId = "uiR2EDTPContextCancelEdit";
			tpContext = R2::TPContext_Edit;
		}
	}
}

void CServerEditionModule::onTpPositionAsked( NLNET::IModuleProxy *senderModuleProxy, float x, float y, float z)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }

	uint8 season;
	TSessionId sessionId = this->_Server->getEditionModule()->getSessionIdByCharId(charId);
	//TSessionId sessionId = this->getSessionIdByCharId(charId);
	{
		double sx, sy, sorient;
		this->_Server->getAdminModule()->getPosition(sessionId, sx, sy, sorient, season);
	}
	R2::TR2TpInfos tpInfos;
	tpInfos.UseTpMessage = true;
	tpInfos.TpReasonId = "uiR2EDTPContextTeleport";
	getTpContext(charId, tpInfos.TpCancelTextId, tpInfos.TpContext );

	this->tpPosition( senderModuleProxy, clientEid, x, y, z, season, tpInfos);

}


void CServerEditionModule::tpPosition(NLNET::IModuleProxy *sender, const NLMISC::CEntityId & eid, float x, float y, float z, uint8 season,  const R2::TR2TpInfos& tpInfos)
{
	if ( !_CharacterControlProxy.isNull())
	{
		CCharacterControlItfProxy ccip( _CharacterControlProxy );
		ccip.onTpPositionAsked(this, eid, x, y, z, season, tpInfos);
		return;
	}

	bool isLocal = !_Server->useNetwork();

	if (isLocal && sender)
	{

		TCharId charId = static_cast<uint32>( eid.getShortId() );
		const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
		if (clientEditionProxyPtr)
		{
			CShareClientEditionItfProxy client(*clientEditionProxyPtr);
			TSessionId sessionId = getSessionIdByCharId(charId);
			client.onTpPositionSimulated(this, sessionId, eid.getRawId(), sint32(x), sint32(y), sint32(z), season);
		}

		// send message to client in order to TP ihm event if he is not connected
	return;

	}

}


void CServerEditionModule::setStartingAct(NLNET::IModuleProxy *senderModuleProxy, uint32 actId)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }
	TSessionId sessionId = getSessionIdByCharId(charId);
	if (sessionId.asInt()==0) { return ;}
	CScenario* scenario = getScenarioById(sessionId);
	scenario->setInitialActIndex(actId);
}


void CServerEditionModule::tpToEntryPoint(NLNET::IModuleProxy *senderModuleProxy, uint32 actId)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }


	double x,y, orient;
	uint8 season;


	TSessionId sessionId = _Server->getAdminModule()->getSessionIdByCharId(charId);
	uint32 slot = 0;

	CEditionSession* session = getSession(sessionId);
	if (session)
	{
		slot = session->getEditSlotId(charId);
	}

	static  double dx[] = { 0, 1, -1,  0,  1, -1, 1, -1};
	static  double dy[] = { 1, 0,  0, -1, -1, -1, 1,  1};



	ok = _Server->getAdminModule()->getPosition(sessionId, x ,y, orient, season, actId);
	if (!ok)
	{
		return;
	}

	x += dx[slot%8]*(slot/8);
	y += dy[slot%8]*(slot/8);

	R2::TR2TpInfos tpInfos;
	tpInfos.UseTpMessage = true;
	tpInfos.TpReasonId = "uiR2EDTPContextTeleportToAct";
	tpInfos.TpReasonParams.push_back(toString("%u", actId));
	getTpContext(charId, tpInfos.TpCancelTextId, tpInfos.TpContext);

	this->tpPosition( senderModuleProxy, clientEid, (float) x, (float) y, 0, season, tpInfos);

}


void CServerEditionModule::onScenarioRingAccessUpdated(NLNET::IModuleProxy *senderModuleProxy, bool ok,const std::string & ringAccess, const std::string& errMsg)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	if (! checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv) ) { return; }


	TSessionId sessionId = getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId);

	if (!session)
	{
		return;
	}

	session->RingAccess = ringAccess;

	if (!ok)
	{
		std::set<uint32>::const_iterator found = session->CharsToVerify.find(charId);
		if (found != session->CharsToVerify.end())
		{
			return;
		}
		session->CharsToVerify.insert(charId);

		CShareClientEditionItfProxy client(senderModuleProxy);
		client.systemMsg(this, "ERR", "", errMsg);

		NLMISC::TTime now = NLMISC::CTime::getLocalTime();
		_Tasks.addTask(	new CVerifyRingAccess(now + 1000*TimeBeforeDisconnectionAfterKick, this, sessionId, charId));



	}

}


void CServerEditionModule::swapOverrideRingAccess(TOverrideRingAccess& access)
{
	_OverrideRingAccess.swap(access);
	_MustUpdateOverrideRingAcess = true;
}


bool CServerEditionModule::isSessionHibernating(TSessionId sessionId, RSMGR::TSessionType& sessionType, double& x, double& y, double& orient, uint8& season)
{

	THibernatingSessions::const_iterator session = _HibernatingSessions.find(sessionId);
	if ( session != _HibernatingSessions.end())
	{
		sessionType = session->second.SessionType;
		x= session->second.PositionX;
		y = session->second.PositionY;
		orient = session->second.Orient;
		season = session->second.Season;
		return true;
	}
	return false;
}


// a message to validate a file waiting to be saved
void CServerEditionModule::saveScenarioFile(NLNET::IModuleProxy *sender, const std::string &md5, const TScenarioHeaderSerializer &header)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	if (! checkSecurityInfo(sender, charId, clientEid, userPriv, extendedPriv) ) { return; }

	std::string signature = _KeysHolder->getCurrentSignature(md5);
	bool ok = true;
	TScenarioHeaderSerializer::TValueType value = header.Value;
	for (uint32 i = 0; i < value.size() ; ++i)
	{
		volatile static bool override = false;

 		if (value[i].first == "NevraxScenario" && value[i].second == "1" && userPriv.find("DEV") == std::string::npos && !override)
		{
			nlwarning("The client %u(CharId) try to save a scenario that is RoSOnly so it means that the gui has let the user the possibility to try or that the user is trying to hack us", charId);
			ok = false;
			break;
		}
	}

	CShareClientEditionItfProxy client(sender);
	if (ok)
	{
		client.saveScenarioFileAccepted(this, md5, signature, true);
	}
	else
	{
		client.saveScenarioFileAccepted(this, md5, "", false);
	}
}


// a message to validate a file waiting to be loaded
void CServerEditionModule::loadScenarioFile(NLNET::IModuleProxy *senderModuleProxy, const std::string &md5, const std::string &signature)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;



	if (! checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv) ) { return; }

	bool ok = _KeysHolder->verifyKey(md5, signature);
	CShareClientEditionItfProxy client(senderModuleProxy);
	client.loadScenarioFileAccepted(this, md5, ok);
}

// a message to validate a user component file waiting to be saved
void CServerEditionModule::saveUserComponentFile(NLNET::IModuleProxy *sender, const std::string &md5, const TScenarioHeaderSerializer &header)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	if (! checkSecurityInfo(sender, charId, clientEid, userPriv, extendedPriv) ) { return; }

	std::string signature = _KeysHolder->getCurrentSignature(md5);

	CShareClientEditionItfProxy client(sender);
	client.saveUserComponentFileAccepted(this, md5, signature, true);
}

// a message to validate a file waiting to be loaded
void CServerEditionModule::loadUserComponentFile(NLNET::IModuleProxy *senderModuleProxy, const std::string &md5, const std::string &signature)
{
	TCharId charId;
	NLMISC::CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;



	if (! checkSecurityInfo(senderModuleProxy, charId, clientEid, userPriv, extendedPriv) ) { return; }

	bool ok = _KeysHolder->verifyKey(md5, signature);
	CShareClientEditionItfProxy client(senderModuleProxy);
	client.loadUserComponentFileAccepted(this, md5, ok);
}


void CServerEditionModule::resetKeyPolicies(NLNET::IModuleProxy *senderModuleProxy)
{

	nlassert(_KeysHolder);
	_KeysHolder->resetPolicies();
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, resetKeyPolicies)
{

	if (args.size() != 0)
	{
		log.displayNL("Wrong parameters CServerEditionModule.resetKeyPolicies");
		return false;
	}
	resetKeyPolicies(0);
	return true;
}


void CServerEditionModule::setDefaultKey(NLNET::IModuleProxy *sender, const std::string & defaultKeyName)
{
	nlassert(_KeysHolder);
	_KeysHolder->setDefaultKey(defaultKeyName);

}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, setDefaultKey)
{

	if (args.size() != 1)
	{
		log.displayNL("Wrong parameters CServerEditionModule.setDefaultKey <defaultKey>");
		return false;
	}
	setDefaultKey(0, args[0]);
	return true;
}


void CServerEditionModule::addKeyPolicy(NLNET::IModuleProxy *sender, const std::string & keyName, const std::string & privateKeyValue, const std::string& policy)
{
	CKeysHolder::TPolicy p = CKeysHolder::Refuse;

	if (policy == "REFUSE")
	{
		p = CKeysHolder::Refuse;
	}
	else if (policy == "ACCEPT")
	{
		p =  CKeysHolder::Accept;
	}
	else if (policy == "TEST")
	{
		p =  CKeysHolder::Test;
	}
	else
	{
		nlwarning("Policy not implemented %s", policy.c_str());
	}

	std::string key = keyName;
	if (key == "EMPTY")
	{
		key = "";
	}

	std::string privateKey = privateKeyValue;
	if (privateKey == "EMPTY")
	{
		privateKey = "";
	}

	_KeysHolder->addKeyPolicy(key, privateKey, p);
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, addKeyPolicy)
{

	if (args.size() != 3)
	{
		log.displayNL("Wrong parameters CServerEditionModule.addKeyPolicy <key|EMPTY|DEFAULT> <privatekey|EMPTY> <ACCEPT|REFUSE|TEST>");
		return false;
	}
	addKeyPolicy(0, args[0], args[1], args[2]);
	return true;
}


TSessionId CServerEditionModule::getLinkedSessionId(TSessionId sessionId) const
{
	TSessions::const_iterator found = _Sessions.find(sessionId);
	if ( found == _Sessions.end()) { return TSessionId(0); }
	TCharId ownerId = found->second->getOwnerCharId();
	TOwnerInfos::const_iterator ownerIt = _OwnerInfos.find(ownerId);
	if (ownerIt == _OwnerInfos.end()) { return TSessionId(0);}
	if (ownerIt->second.EditionSessionId.asInt() == 0 ||  ownerIt->second.AnimationSessionId.asInt() == 0) { return TSessionId(0); }

	return ownerIt->second.EditionSessionId == sessionId
		? ownerIt->second.AnimationSessionId
		: ownerIt->second.EditionSessionId;
}


NLMISC_CLASS_COMMAND_IMPL(CServerEditionModule, setSessionStartParams)
{

	if (args.size() != 5)
	{
		log.displayNL("Wrong parameters CServerEditionModule.setSessionStartParams charId, sessionId, initialIslandLocation, initialEntryPoint, initialSeason");
		return false;
	}

	TCharId charId;
	fromString(args[0], charId);

	uint32 sessionId;
	fromString(args[1], sessionId);

	std::string initialIslandLocation(args[2]);
	std::string initialEntryPoint(args[3]);
	std::string initialSeason(args[4]);
	setSessionStartParams(0, charId, (TSessionId)sessionId, initialIslandLocation, initialEntryPoint, initialSeason);
	return true;
}


void CServerEditionModule::setSessionStartParams(NLNET::IModuleProxy *sender, TCharId charId, TSessionId sessionId, const std::string& initialIslandLocation, const std::string& initialEntryPoint, const std::string& initialSeason)
{

	CScenarioEntryPoints& epManager = CScenarioEntryPoints::getInstance();

	CScenarioEntryPoints::CCompleteIsland * island = epManager.getIslandFromId(initialIslandLocation);
	BOMB_IF(!island, NLMISC::toString("Session %u No Island '%s'.", sessionId.asInt(), initialIslandLocation.c_str()), return);

	CScenarioEntryPoints::CShortEntryPoint *entryPoint = epManager.getEntryPointFromIds(initialIslandLocation, initialEntryPoint);


	BOMB_IF(!entryPoint, NLMISC::toString("Session %u: No EntryPoint '%s' '%s'.", sessionId.asInt(), initialIslandLocation.c_str(), initialEntryPoint.c_str()), return);

	sint32 x = entryPoint->X;
	sint32 y = entryPoint->Y;
	uint8 season = 0;

	if (initialSeason=="Automatic") { season = 0; }
	else if (initialSeason=="Spring") { season = 1; }
	else if (initialSeason=="Summer") { season = 2; }
	else if (initialSeason=="Autumn" ) { season = 3; }
	else if (initialSeason=="Winter") { season = 4; }

	setSessionStartParams(sessionId, x, y, season);


}

void CServerEditionModule::setSessionStartParams(TSessionId sessionId, sint32 x, sint32 y, uint8 season)
{
	CEditionSession* session = getSession(sessionId);
	if (session)
	{
		session->setStartParams(x, y, season);
		getAnimationModule()->setSessionStartParams(sessionId, x, y, season);
	}

	TSessionId linkedId = getLinkedSessionId(sessionId);
	CEditionSession* session2 = getSession(linkedId);
	if (session2)
	{
		if (session2) { session2->setStartParams(x, y, season); }
		getAnimationModule()->setSessionStartParams(linkedId, x, y, season);
	}


}

void CServerEditionModule::teleportWhileUploadingScenario(NLNET::IModuleProxy *sender, const std::string& initialIslandLocation, const std::string& initialEntryPoint, const std::string& initialSeason)
{
	TCharId charId;
	CEntityId clientEid;
	std::string userPriv;
	std::string extendedPriv;

	bool ok = checkSecurityInfo(sender, charId, clientEid, userPriv, extendedPriv);
	if (!ok) { return; }


	TSessionId  sessionId =  getSessionIdByCharId(charId);
	CEditionSession* session = getSession(sessionId);
	// :TODO:
	CScenarioEntryPoints& epManager = CScenarioEntryPoints::getInstance();

	CScenarioEntryPoints::CCompleteIsland * island = epManager.getIslandFromId(initialIslandLocation);
	BOMB_IF(!island, NLMISC::toString("Session %u No Island '%s'.", sessionId.asInt(), initialIslandLocation.c_str()), return);

	CScenarioEntryPoints::CShortEntryPoint *entryPoint = epManager.getEntryPointFromIds(initialIslandLocation, initialEntryPoint);


	BOMB_IF(!entryPoint, NLMISC::toString("Session %u: No EntryPoint '%s' '%s'.", sessionId.asInt(), initialIslandLocation.c_str(), initialEntryPoint.c_str()), return);

	sint32 x = entryPoint->X;
	sint32 y = entryPoint->Y;
	uint8 season = 0;

	if (initialSeason=="Automatic") { season = 0; }
	else if (initialSeason=="Spring") { season = 1; }
	else if (initialSeason=="Summer") { season = 2; }
	else if (initialSeason=="Autumn" ) { season = 3; }
	else if (initialSeason=="Winter") { season = 4; }


	if (!session)
	{
	//	nlwarning("R2Ed: error char %u not connected to a session", charId);
		return;
	}

	CEditionSession::TCurrentChars::const_iterator first(session->getCurrentChars().begin()), last(session->getCurrentChars().end());
	for (;first != last ; ++first)
	{
		TCharId charId = (*first)->getCharId();

		TPionnerInfos::iterator foundPioneersEId= _PioneersInfo.find(charId);
		if (foundPioneersEId != _PioneersInfo.end())
		{
			R2::TR2TpInfos tpInfos;
			tpInfos.UseTpMessage = true;
			tpInfos.TpReasonId = "uiR2EDTPContextStartScenario";
			getTpContext(charId, tpInfos.TpCancelTextId, tpInfos.TpContext );
			tpPosition(0, foundPioneersEId->second.EntityId, float(x), float(y), 0, season, tpInfos);
		}
	}
}


void CServerEditionModule::returnToPreviousSession(TCharId charId)
{
	DROP_IF(_CharacterControlProxy.isNull(), "Try to disconnect a player but no EGS?", return);
	nlinfo("The charId '%u' was send back to its previous session (maybe the session he was invited was closed)", charId);
	CCharacterControlItfProxy proxy(_CharacterControlProxy);
	proxy.returnToPreviousSession(this, charId);
}

void CServerEditionModule::getStartParams(uint32 charId, TSessionId lastStoredSessionId)
{
	nldebug( "R2An: Setting Char %u start position", charId);
	{
		CEntityId clientEid = CEntityId::Unknown;
		clientEid.setShortId(charId);
		clientEid.setDynamicId(0x86); //FrontendId
		clientEid.setType(RYZOMID::player);
		createSessionWithoutSu(charId, clientEid);
	}

	TPioneersSessionsAllowed *psa = getSessionAllowedForChar( charId ); // the client is not ready yet
	if ( !psa )
	{
		nlwarning( "Char %u not allowed in any session", charId );
		returnToPreviousSession(charId);
		return;
	}

	TSessionId sessionId = psa->SessionId;
	CEditionSession* session = getSession(sessionId);
	if (!session)
	{
		nlwarning( "Char %u try to connect to an empty session %u", charId, sessionId.asInt() );
		returnToPreviousSession(charId);
		return;
	}
	// True if (the session is up and this is an  edition session)
	// True if (the session is down and this is an hiberting edition session)



	if (session->isLoaded())
	{
		nldebug("Getting start position from session %u", sessionId.asInt());
		getStartParamsImpl(charId, lastStoredSessionId);
	}
	else
	{
		nldebug("Getting start position from session %u (that is hibernating)", sessionId.asInt());
		session->getStartParamsWithDelay(this, charId, lastStoredSessionId);
	}

}

void CServerEditionModule::getStartParamsImpl(uint32 charId, TSessionId lastStoredSessionId)
{


	// Get the session in which the character is connecting



	double x, y, orient;
	uint8 season=0;
	RSMGR::TSessionType sessionType;

	TPioneersSessionsAllowed *psa = getSessionAllowedForChar( charId ); // the client is not ready yet
	if ( !psa )
	{
		nlwarning( "Char %u not allowed in any session", charId );
		returnToPreviousSession(charId);
		return;
	}

	// Find out if the character has already visited this session
	TSessionId sessionId = psa->SessionId;
	bool reloadPos = (sessionId == lastStoredSessionId) && (psa->Role != R2::TUserRole::ur_editor);
	if (reloadPos)
	{
		reloadPos = getAnimationModule()->mustReloadPosition(sessionId, charId);
	}

	// Get the starting pos for the current act of the animation session (at least for respawn point)
	CFarPosition actStartPosition;
	actStartPosition.SessionId = sessionId;



	bool ok = _Server->getAdminModule()->getPosition(sessionId, x, y, orient, season);

	// Do not appers each time at the same location. (at 10 m arround)
	sint32 dx = rand() % 5000 - 2500;
	sint32 dy = rand() % 5000 - 2500;

	if ( ok )
	{
		actStartPosition.PosState.X = (sint32)(x*1000.0 + dx);
		actStartPosition.PosState.Y = (sint32)(y*1000.0 + dy);
		actStartPosition.PosState.Z = 0; // ??
		actStartPosition.PosState.Heading = 0; // ??
	}
	else
	{
		x = y = 0;
		actStartPosition.PosState.X =  (sint32)(x*1000.0); //0;
		actStartPosition.PosState.Y = (sint32)(y*1000.0); //0
		actStartPosition.PosState.Z = 0;
		actStartPosition.PosState.Heading = 0;
	}

	CScenario * scenario = getScenarioById(sessionId);
	if (!scenario || scenario->getHighLevel() == 0) { reloadPos = false; }



	nldebug( "R2An: Char %u will start at position (%f, %f) when he will arrive in session %u", charId, x, y, sessionId.asInt() );


	// Send the answer
	CCharacterControlItfProxy ccip( _CharacterControlProxy );
	ccip.setUserCharStartParams( this, charId, actStartPosition, reloadPos, season, psa->Role );
	if (reloadPos)
	{
		psa->EgsReloadPos = true;
	}
	bool isEd =  isEditingSession(psa->SessionId);
	nldebug( "R2An: Char %u will %s %s session %u", charId, (reloadPos ? "resume" : "join"),  (isEd? "edition":"animation") , sessionId.asInt() );
}


CServerEditionModule::CPioneerInfo* CServerEditionModule::getPioneerInfo(TCharId charId)
{
	TPionnerInfos::iterator found = _PioneersInfo.find(charId);
	if (found != _PioneersInfo.end())
	{
		return &found->second;
	}
	return 0;
}


void CServerEditionModule::multiPartMsgHead(NLNET::IModuleProxy *sbs, uint32 charId, const std::string &msgName, uint32 nbPacket, uint32 size)
{
	nldebug(" Received a multi-part message %s (Start, part=%u, size=%u)", msgName.c_str(), nbPacket, size);
	CPioneerInfo* info = getPioneerInfo(charId);
	if (!info) { nlwarning("Error: received a multipartMsgHead from a unknown char %u", charId); return;}
	if (info->WaitingMsg)
	{
		nlwarning("Error: Char %u was ask to receive a multi-part message but the previous one seems to have be lost.", charId);
		delete info->WaitingMsg;
		info->WaitingMsg = 0;
	}

	info->WaitingMsg = new NLNET::CMessage(); //Message name is directly in the data
	//info->WaitingMsgCurrentSize = 0;
	//info->WaitingMsgSize = size;
	//info->WaitingMsgName = msgName;

	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
	if (clientEditionProxyPtr)
	{
		NLNET::TModuleProxyPtr clientEditionProxy = (*clientEditionProxyPtr);
		CShareClientEditionItfProxy proxy( *clientEditionProxyPtr );
		proxy.multiPartMsgHead(this, msgName, nbPacket, size);
	}
}


void CServerEditionModule::multiPartMsgBody(NLNET::IModuleProxy *sbs, uint32 charId, uint32 partId, const std::vector<uint8> &data)
{
	nldebug("Received a multi-part message from %u (Body, part=%u, size=%u)", charId, partId, data.size());
	CPioneerInfo* info = getPioneerInfo(charId);
	if (!info) { nlwarning("Error: received a multipartMsgHead from a unknown char %u", charId); return;}

	if (!info->WaitingMsg) { nlwarning("Error: Char %u was ask to receive a multi-part message  part but the first part seems to have been lost.", charId) ;return;	}

	info->WaitingMsg->serialBuffer(const_cast<uint8*>(&data[0]), (uint)data.size());
	//info->WaitingMsgCurrentSize += data.size();

	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
	if (clientEditionProxyPtr)
	{
		NLNET::TModuleProxyPtr clientEditionProxy = (*clientEditionProxyPtr);
		CShareClientEditionItfProxy proxy( *clientEditionProxyPtr );
		proxy.multiPartMsgBody(this, partId, (uint32)data.size());
	}
}


void CServerEditionModule::multiPartMsgFoot(NLNET::IModuleProxy *sbs, uint32 charId)
{
	nldebug("Received a multi-part message from  %u (End)", charId);
	CPioneerInfo* info = getPioneerInfo(charId);
	if (!info) { nlwarning("Error: received a multipartMsgHead from a unknown char %u", charId); return;}

	if (!info->WaitingMsg) { nlwarning("Error: Char %u was ask to receive a multi-part message end but the first part seems to have been lost.", charId) ;return;	}


	info->WaitingMsg->invert();

	const NLNET::TModuleProxyPtr* clientEditionProxyPtr = getClientProxyPtr(charId);
	if (clientEditionProxyPtr)
	{
		CShareClientEditionItfProxy proxy( *clientEditionProxyPtr);
		proxy.multiPartMsgFoot(this);
		onReceiveModuleMessage(*clientEditionProxyPtr, *info->WaitingMsg);
	}
	else
	{
		nlwarning("Error: Char %u was ask to receive the last part of  a multi-part message  part but the player seem to not be connected.", charId) ;
	}

	delete info->WaitingMsg;
	info->WaitingMsg = 0;

}
// Simulate the dss
void CServerEditionModule::forwardToDss(NLNET::IModuleProxy *senderModuleProxy, uint32 charId, const NLNET::CMessage& msg)
{
	NLNET::CMessage message(msg);
	if (!message.isReading())
	{
		message.invert();
	}
	onReceiveModuleMessage(senderModuleProxy, message);
}

