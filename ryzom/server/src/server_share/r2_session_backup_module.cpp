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


#include "../dynamic_scenario_service/dynamic_scenario_service.h"
#include "nel/misc/types_nl.h"
#include "nel/net/module.h"
#include "nel/net/module_common.h"

#include "nel/net/module_builder_parts.h"

#include "game_share/persistent_data.h"

#include "game_share/task_list.h"
#include "game_share/ring_session_manager_itf.h" //For RSMGR
#include "game_share/r2_modules_itf.h" 
#include "game_share/r2_basic_types.h"

#include <string>
#include <map>

namespace R2
{
// TO GameShare
struct THibernatingSession
{
	THibernatingSession():HibernationDate(0){PositionX = 0; PositionY = 0; Orient = 0; Season=0;}
	
	THibernatingSession(uint32 hibernationDate, RSMGR::TSessionType sessionType, double x, double y, double orient, uint8 season )
		:HibernationDate(hibernationDate), SessionType(sessionType) { PositionX = x; PositionY = y; Orient = orient; Season = season; }

	~THibernatingSession()
	{
						
	}
public:
	uint32 HibernationDate; //secondes since 1970
	RSMGR::TSessionType SessionType;
	CTaskList<NLMISC::TTime> Tasks;
	// In order that getStartPos works without 
	double PositionX,PositionY, Orient; 
	uint8 Season;
};


/*

  Handle the save / Wakup of hiberning Sessions.
  The persistency of index is done via file. It is perhaps unecessary. Because we have a Database.
  -> Save / Load index of file
  -> Send a message to Dss to tell which sesion must be restored.


	CR2SessionBackupModule					 DSS1
	|											|
	|			--- notifyIfSavedFile   -->		|	// Add entry to Dss Hibernating list
	|	        --- ( SU) -> createSession-->		|	// Wake-up Session
	|											|
	|			<-- reportSavedSessions --	    |	// updated every 30 seconds (DSS save sessions)
	|           <-- reportHibernatedSessions--  |	// updated when necessary (a session has hibernated (after X minutes of wait) 
												|	// Remove entries from Dss NotifyHibernating list
	--------------------------------------------------------------------------------------------
	CR2SessionBackupModule						DSS2
	|			--- notifyIfSavedFile   -->		|	// Add entry to Dss Hibernating list
	|	        --- SU -> createSession -->		|   // Wake-up Session
	|			<-- reportSavedSessions --	    |	// updated every 30 seconds (DSS save sessions)
	|			--- SU -> closeSession  -->		|   // Add entry to DSS closing list
	|			<-- reportClosedSessions --		|	// (DSS delete sessions)

*/

class CR2SessionBackupModule :public CR2SessionBackupModuleItfSkel,
	public NLNET::CEmptyModuleServiceBehav<NLNET::CEmptyModuleCommBehav<NLNET::CEmptySocketBehav <NLNET::CModuleBase> > >
{
	
public:
	struct TSessionInfo
	{
		uint32 LastDateUsed;
	};
	typedef uint32 TCharId;
	typedef uint32 TShardId;
	typedef std::map<TSessionId, TSessionInfo> TSessionInfos;	
	
	typedef std::map<TCharId, std::string> TOverrideRingAccess;
	typedef std::map<TShardId, TSessionInfos> TActiveShards;

public:
	CR2SessionBackupModule();
	
	~CR2SessionBackupModule();

	virtual void onServiceUp(const std::string &serviceName, uint16 serviceId);
	
	virtual void onModuleUp(NLNET::IModuleProxy *moduleProxy);
	
	virtual void onModuleDown(NLNET::IModuleProxy *moduleProxy);

	virtual void onModuleUpdate();
	
	virtual void registerDss(NLNET::IModuleProxy *moduleProxy, TShardId shardId);

	virtual bool onProcessModuleMessage(NLNET::IModuleProxy *senderModuleProxy, const NLNET::CMessage &message);

	//virtual void onModuleSecurityChange(NLNET::IModuleProxy *moduleProxy);

	// Load all indexs Up
	// Save all indexs Down
	/* Load / Save hibernating session */
	// From Su to Dss
	virtual void notifyIfSavedFile(NLNET::IModuleProxy *moduleProxy, TShardId shardId, TSessionId sessionId);
	
	// From Su
	// The DSS delete the session_*, the DBM remove entries from index
	virtual void reportDeletedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector<TSessionId>& sessionId);
	// Same as reportSavedSessions but DSS remove local map
	virtual void reportHibernatedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector<TSessionId>& session);
	// Update local index
	virtual void reportSavedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector< TR2SbmSessionInfo > &sessionInfos);

	//implement private Callback CHibernatingSessionListCallback
	void resetHibernatingSessionList(TSessionInfos&  sessions);
	// Save file to Disc every 30 seconds.
	void updateSessionListFile();
	// 
	void addSavedSessionList(TSessionInfos&  sessions);

	/*RingAccessOverrid*/
	//implement private Callback COverrideRingAccessCallback		
	void swapOverrideRingAccess(TOverrideRingAccess& access);

private:
	typedef std::map<TShardId, NLNET::TModuleProxyPtr>	TServerEditionProxys;

	typedef std::map<NLNET::TModuleProxyPtr, TShardId>	TShardIds;

private:
	std::string getSavedSessionListFilename() const;

	std::string getHibernatingSessionListFilename() const;
	// Name of the file that contains override ring access (It is better to use DB).
	std::string getOverrideRingAccessFilename() const;
	// Proxy to dss



private:
	bool _BsGoingUp;
	bool _WaitingForBS;

	TOverrideRingAccess _OverrideRingAccess; //Ring access for dev

	// The active shard (Active sessions sorted by shard)
	TActiveShards	_ActiveShards;
		
	// Rarely Saved only if data changed
	TSessionInfos _HibernatingSessions; 
	bool _MustUpdateHibernatingFileList;

	//Proxy to Dss <=> ShardId
	TServerEditionProxys _ServerEditionProxys;
	TShardIds	_ShardIds;
};

	
} // namespace DMS








//---------------------------------------------------------------------------------------------------------------------------------------------


#include "r2_session_backup_module.h"

#include "nel/net/service.h"
#include "nel/net/unified_network.h"
#include "nel/net/module_message.h"
#include "nel/net/module_socket.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_manager.h"
#include "nel/net/module_builder_parts.h"


#include "nel/misc/command.h"
#include "nel/misc/path.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/sstring.h"
#include "nel/misc/variable.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"


#include "game_share/file_description_container.h"
#include "game_share/utils.h"
#include "game_share/backup_service_interface.h"

#include "game_share/r2_modules_itf.h"

#include <string>
#include <sstream>





using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace R2;

//static std::string AdminModuleSavePath;

//NLMISC_VARIABLE( std::string, AdminModuleSavePath, "File where users adventure are stored." );

//const std::string CR2SessionBackupModule::_AdminModuleSaveFilename( "server_admin_module_data.xml");

NLNET_REGISTER_MODULE_FACTORY(CR2SessionBackupModule,"SessionBackupModule");


namespace R2
{
static const uint32 FileVersion = 3;

class COverrideRingAccessCallback : public IBackupFileReceiveCallback
{
	
public:
//		typedef CSessionId TSessionId; //		typedef uint32 TSessionId;
		typedef uint32 TCharId;
public:
	COverrideRingAccessCallback(CR2SessionBackupModule* module) :_Module(module){}

	// call back for bs file asynchronous read
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		nldebug("R2SBM: BSIcb COverrideRingAccessCallback::callback() for file: %s",fileDescription.FileName.c_str());
		CR2SessionBackupModule::TOverrideRingAccess access;
		parseStream(fileDescription, dataStream, access);
		_Module->swapOverrideRingAccess(access);		
	}

	void parseStream(const CFileDescription& fileDescription, NLMISC::IStream& dataStream, CR2SessionBackupModule::TOverrideRingAccess & access)
	{
	/*	CMemStream * memstream = dynamic_cast<CMemStream *>(&dataStream);
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

				uint32 version = 0;
				if (tokens.size() > 1)
					NLMISC::fromString(tokens[1], version)

				if (tokens.size() != 2 || tokens[0] != "Version" || version != FileVersion)
				{
					nlwarning("R2SBM: Obsolete File Discard '%s'", fileDescription.FileName.c_str());	
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
					NLMISC::fromString(tokens[0], charId);
					std::string overrideAccess = tokens[1];
					
					access[charId] = overrideAccess;
				}
				else
				{
					nlstop;
				}			

			}

			
		}	*/	
	}

	CR2SessionBackupModule* _Module;
};



// file callback class
class CHibernatingSessionListCallback : public IBackupFileReceiveCallback
{
public:
	CHibernatingSessionListCallback(CR2SessionBackupModule* module) :_Module(module){}


	void parseStream(const CFileDescription& fileDescription, NLMISC::IStream& dataStream, CR2SessionBackupModule::TSessionInfos & sessions)
	{
	/*	CMemStream * memstream = dynamic_cast<CMemStream *>(&dataStream);
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

				uint version = 0;
				if (tokens.size() > 1)
					NLMISC::fromString(tokens[1], version);

				if (tokens.size() != 2 || tokens[0] != "Version" || version != FileVersion)
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
				else if (tokens.size() == 7)
				{
					uint32 sessionIdTmp;
					NLMISC::fromString(tokens[0], sessionIdTmp);
					TSessionId sessionId = (TSessionId)sessionIdTmp;
					std::string sessionType = tokens[1];
					uint32 timestamp;
					NLMISC::fromString(tokens[2], timestamp);
					double positionX;
					NLMISC::fromString(tokens[3], positionX);
					double positionY;
					NLMISC::fromString(tokens[4], positionY);
					double orient;
					NLMISC::fromString(tokens[5], orient);
					uint8 season;
					NLMISC::fromString(tokens[6], season);

					sessions[sessionId] = TSessionInfos(timestamp, RSMGR::TSessionType(sessionType)
						,positionX, positionY, orient, season);
				}
				else
				{
					nlstop;
				}			

			}

			
		}		*/
	}

	// call back for bs file asynchronous read
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{
		nldebug("R2SBM: BSIcb CHibernatingSessionListCallback::callback() for file: %s",fileDescription.FileName.c_str());
		CR2SessionBackupModule::TSessionInfos sessions;
		parseStream(fileDescription, dataStream, sessions);
		_Module->resetHibernatingSessionList( sessions );
	}
	
protected:
	CR2SessionBackupModule* _Module;
};


class CSaveSessionListCallback : public CHibernatingSessionListCallback
{
public:
	CSaveSessionListCallback(CR2SessionBackupModule* module) :CHibernatingSessionListCallback(module){}

	// call back for bs file asynchronous read
	void callback(const CFileDescription& fileDescription, NLMISC::IStream& dataStream)
	{	
		nldebug("R2SBM: BSIcb CSaveSessionListCallback::callback() for file: %s",fileDescription.FileName.c_str());
		CR2SessionBackupModule::TSessionInfos sessions;
		parseStream(fileDescription, dataStream, sessions);
		_Module->addSavedSessionList( sessions );
	}


};


void CR2SessionBackupModule::registerDss(NLNET::IModuleProxy *moduleProxy, TShardId shardId)
{
	nldebug("R2SBM : receive DSS registration from '%s'",  moduleProxy->getModuleName().c_str());

	bool ok = _ServerEditionProxys.insert( std::make_pair(shardId, moduleProxy) ).second;
	if (!ok)
	{
		nlwarning("R2SBM: a DSS module '%s' has tryed register to the same shardId '%d' than a other dss.", moduleProxy->getModuleName().c_str(), shardId);
		return;
	}
	ok = _ShardIds.insert( std::make_pair(moduleProxy, shardId) ).second;
	if (!ok)
	{
		nlwarning("R2SBM: a DSS module '%s' has tryed to register 2 times.", moduleProxy->getModuleName().c_str());
		return;
	}
}

CR2SessionBackupModule::~CR2SessionBackupModule()
{
	nlassert( CDynamicScenarioService::instance().getR2Sbm()); 
	CDynamicScenarioService::instance().setR2Sbm(NULL);
	
}


CR2SessionBackupModule::CR2SessionBackupModule()
{
	nlassert( !CDynamicScenarioService::instance().getR2Sbm()); 
	CDynamicScenarioService::instance().setR2Sbm(this);

	_BsGoingUp = CDynamicScenarioService::instance().getBsUp();
	_WaitingForBS=  true;
}


void CR2SessionBackupModule::onModuleUp(NLNET::IModuleProxy *moduleProxy)
{

}

	
void CR2SessionBackupModule::onModuleDown(NLNET::IModuleProxy *moduleProxy)
{
	const std::string &moduleName = moduleProxy->getModuleClassName();
	if (moduleName == "ServerEditionModule")
	{
		TShardIds::iterator found = _ShardIds.find(moduleProxy);
		if ( found != _ShardIds.end())
		{
			nldebug("R2SBM: Looks like a module '%s' was down before registering. The dss must have crash at startup?", moduleProxy->getModuleName().c_str() );
			return;
		}		

		TServerEditionProxys::iterator it = _ServerEditionProxys.find(found->second);
		nlassert( it != _ServerEditionProxys.end()); // first check has been done in registerDss
		_ServerEditionProxys.erase( it );
		_ShardIds.erase( found );		
	}
}

	

bool CR2SessionBackupModule::onProcessModuleMessage(IModuleProxy *senderModuleProxy, const CMessage &msgin)
{
	std::string operationName = msgin.getName();
	
	nlwarning("R2SBM: Invalid Operation Name '%s'", operationName.c_str() );

	return false;
}


void CR2SessionBackupModule::onServiceUp(const std::string &serviceName, uint16 serviceId)
{
	if (serviceName == "BS")
	{	
		this->_BsGoingUp = true;			
	}
}

void CR2SessionBackupModule::onModuleUpdate()
{
	H_AUTO(CR2SessionBackupModule_onModuleUpdate);
	
	if (_BsGoingUp)
	{		
		nldebug("BSI requesting files: '%s', '%s', '%s'",getOverrideRingAccessFilename().c_str(),getHibernatingSessionListFilename().c_str(),getSavedSessionListFilename().c_str());
			
		std::vector<CBackupFileClass> classes;
		// Obsolet system of right
		BsiGlobal.requestFile(getOverrideRingAccessFilename(), new COverrideRingAccessCallback(this));		
		BsiGlobal.requestFile(getHibernatingSessionListFilename(), new CHibernatingSessionListCallback(this));		
		BsiGlobal.requestFile(getSavedSessionListFilename(), new CSaveSessionListCallback(this));	

		_BsGoingUp = false;
	}	

	// Do nothing until we have read data from BS
	if (_WaitingForBS)
	{
		return;
	}
}

void CR2SessionBackupModule::updateSessionListFile()
{
/*
	// Send list only if changed
	if (_MustUpdateHibernatingFileList)
	{	

		TSessionInfos::const_iterator first( _HibernatingSessions.begin() ), last( _HibernatingSessions.end() );

		std::stringstream ss;
		
		ss <<  toString("Version\t%u\n", FileVersion );
		for (; first != last; ++first)
		{
			std::string tmp = NLMISC::toString("%u\t%s\t%u\t%lf\t%lf\t%lf\t%u\n", first->first.asInt(), first->second.SessionType.toString().c_str(), first->second.HibernationDate,
				first->second.PositionX, first->second.PositionY, first->second.Orient, first->second.Season);
			ss << tmp;
		}
		std::string copy = ss.str();

		nldebug("UPDATING hibernating file list - BSI send file: %s",getHibernatingSessionListFilename().c_str());
		CBackupMsgSaveFile msg( getHibernatingSessionListFilename(), CBackupMsgSaveFile::SaveFileCheck, Bsi );
		msg.DataMsg.serialBuffer((uint8*)copy.c_str(), copy.size());
		Bsi.sendFile( msg );
		_MustUpdateHibernatingFileList = false;
	}

	
	{
		TSessions::const_iterator first( _Sessions.begin() ), last( _Sessions.end() );

		std::stringstream ss;
		ss <<  toString("Version\t%u\n", FileVersion );
		for (; first != last; ++first)
		{
			if (first->second->getLastSaveTime())
			{
				double x,y,orient;
				uint8 season;
				getPosition(first->first, x,y,orient, season, 1);
				std::string basic = NLMISC::toString("%u\t%s\t%u\t", first->first.asInt(), first->second->getSessionType().toString().c_str(), first->second->getLastSaveTime());
				std::string position = NLMISC::toString("%lf\t%lf\t%lf\t%u\n", x, y, orient, season);
				std::string tmp = toString("%s%s", basic.c_str(), position.c_str());
					
				ss << tmp;
			}					
		}
		std::string copy = ss.str();

		nldebug("UPDATING saved session file list - BSI send file: %s",getSavedSessionListFilename().c_str());
		CBackupMsgSaveFile msg( getSavedSessionListFilename(), CBackupMsgSaveFile::SaveFileCheck, Bsi );
		msg.DataMsg.serialBuffer((uint8*)copy.c_str(), copy.size());
		Bsi.sendFile( msg );
	}


	{ 
		TClosedSessions::iterator first(_ClosedSessions.begin()), last(_ClosedSessions.end());
		for (; first != last; ++first)
		{
			nldebug("CLEANING closed session - BSI delete file: %s",getSessionFilename(*first).c_str());
			Bsi.deleteFile( getSessionFilename(*first), false);
		}
		_ClosedSessions.clear();
	} 


	{
		TOverrideRingAccess::const_iterator first( _OverrideRingAccess.begin() ), last( _OverrideRingAccess.end() );

		std::stringstream ss;
		ss <<  toString("Version\t%u\n", FileVersion );
		for (; first != last; ++first)
		{
			std::string tmp = NLMISC::toString("%d\t%s\n", first->first, first->second.c_str());
			ss << tmp;
		
		}
		std::string copy = ss.str();

		nldebug("UPDATING ring access override file list - BSI send file: %s",getOverrideRingAccessFilename().c_str());
		CBackupMsgSaveFile msg( getOverrideRingAccessFilename(), CBackupMsgSaveFile::SaveFileCheck, Bsi );
		msg.DataMsg.serialBuffer((uint8*)copy.c_str(), copy.size());
		Bsi.sendFile( msg );
	}
*/
}

void CR2SessionBackupModule::resetHibernatingSessionList(TSessionInfos& sessions)
{
	_HibernatingSessions.swap(sessions);
}


void CR2SessionBackupModule::addSavedSessionList(TSessionInfos& sessions)
{
	//	_HibernatingSessions.swap(sessions);
	if (!sessions.empty())
	{
		nlwarning("R2SBM: Warning: The server must have crashed. Because some saved file have not been synchronized. So we will define has hibernationg %u session", sessions.size());
	}	
	uint32 now = NLMISC::CTime::getSecondsSince1970();
	TSessionInfos::iterator first(sessions.begin()), last(sessions.end());
	for (; first != last ; ++first)
	{		
		first->second.LastDateUsed = now;
		nldebug("R2SBM: ADD HIBERNATE SESSION: CServerEditionModule::addSavedSessionList(): sessionId=%u ",first->first.asInt());
		_HibernatingSessions[first->first] = first->second;
	}

	_WaitingForBS = false;
}

//The module send a message to a dss saying that the dss must wakeup a session from hibernation
void CR2SessionBackupModule::notifyIfSavedFile(NLNET::IModuleProxy *moduleProxy, TShardId shardId, TSessionId sessionId)
{
	if (_WaitingForBS) 
	{
		nlwarning("R2SBM: try to send message to dss before data where loding from BS");
		return;
	}

	TServerEditionProxys::const_iterator found(_ServerEditionProxys.find(shardId));
	if ( found == _ServerEditionProxys.end())
	{
		nlwarning("R2SBM: try to create a session on a DSS that do not exist yet...");
		return;
	}
	
	//<sql>
	TSessionInfos::const_iterator sessionFound = _HibernatingSessions.find(sessionId);
	bool wasSessionFound = sessionFound != _HibernatingSessions.end();
	//</sql>

	if (wasSessionFound)
	{
		CServerEditionItfProxy dss(found->second);
	//	dss.wakupSession(this, sessionId);
	}
	
	
}


// From Su
// The DSS delete the session_*, the DBM remove entries from index
void CR2SessionBackupModule::reportDeletedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector<TSessionId>& sessionIds)
{
	if (_WaitingForBS) 
	{
		nlwarning("R2SBM: try to send message to dss before data where loding from BS");
		return;
	}

	TShardIds::const_iterator foundShard(_ShardIds.find(moduleProxy));
	if ( foundShard == _ShardIds.end())
	{
		nlwarning("R2SBM: Message from an unregistered proxy '%s', moduleProxy->getModuleName().c_str()");
		return;
	}

	//<sql>
	// Remove from active shard and add from hibernating files
	TActiveShards::iterator foundActiveShard = _ActiveShards.find(foundShard->second);
	std::vector<TSessionId>::const_iterator first(sessionIds.begin()), last(sessionIds.end());
	for (; first != last; ++first)
	{
		TSessionId sessionId(*first);
		TSessionInfos::iterator session = foundActiveShard->second.find(*first);
		if (session !=  foundActiveShard->second.end())
		{
			foundActiveShard->second.erase(session);
		}
		TSessionInfos::iterator hibernatingSession = _HibernatingSessions.find(sessionId);
		if (hibernatingSession != _HibernatingSessions.end())
		{
			_HibernatingSessions.erase(hibernatingSession);
		}
	}
	//</sql>
		
}


// Same as reportSavedSessions but DSS remove local map
void CR2SessionBackupModule::reportHibernatedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector<TSessionId>& sessionIds)
{
	if (_WaitingForBS) 
	{
		nlwarning("R2SBM: try to send message to dss before data where loding from BS");
		return;
	}

	TShardIds::const_iterator foundShard(_ShardIds.find(moduleProxy));
	if ( foundShard == _ShardIds.end())
	{
		nlwarning("R2SBM: Message from an unregistered proxy '%s', moduleProxy->getModuleName().c_str()");
		return;
	}

	//<sql>
	// Remove from active shard and add to hibernating files
	TActiveShards::iterator foundActiveShard(_ActiveShards.find(foundShard->second));
	std::vector<TSessionId>::const_iterator first(sessionIds.begin()), last(sessionIds.end());
	for (; first != last; ++first)
	{
		// remove session from active session
		TSessionId sessionId(*first);
		TSessionInfos::iterator session = foundActiveShard->second.find(*first);
		if (session !=  foundActiveShard->second.end())
		{
			foundActiveShard->second.erase(session);
		}
		// Add to hibernating session 
		uint32 now = NLMISC::CTime::getSecondsSince1970();
		_HibernatingSessions[sessionId].LastDateUsed = now; // Update or create entry			
	}
	//</sql>
}

// Update local index
void CR2SessionBackupModule::reportSavedSessions(NLNET::IModuleProxy *moduleProxy, const std::vector< TR2SbmSessionInfo > &sessionInfos)
{
	if (_WaitingForBS) 
	{
		nlwarning("R2SBM: try to send message to dss before data where loding from BS");
		return;
	}
	
	TShardIds::const_iterator foundShard(_ShardIds.find(moduleProxy));
	if ( foundShard == _ShardIds.end())
	{
		nlwarning("R2SBM: Message from an unregistered proxy '%s', moduleProxy->getModuleName().c_str()");
		return;
	}
/*
	//<sql>
	// Update active session start time
	TActiveShards::iterator foundActiveShard(_ActiveShards.find(foundShard->second));
	
	std::vector<std::pair<TSessionId, TSessionInfo> >::const_iterator first(sessionInfos.begin()), last(sessionInfos.end());
	foundActiveShard->second.clear();
	foundActiveShard->second.insert(first, last);

	for (; first != last ; ++first)
	{
		TSessionInfos::iterator found(_HibernatingSessions.find(first->first));
		if (found != _HibernatingSessions.end())
		{
			_HibernatingSessions.erase(found);
		}		
	}
	//</sql>
*/

}

static const string _SubRep("r2");
std::string CR2SessionBackupModule::getSavedSessionListFilename() const
{
	return NLMISC::toString("%sr2_session_save_list.txt", _SubRep.c_str());
}


std::string CR2SessionBackupModule::getHibernatingSessionListFilename() const
{
	return NLMISC::toString("%sr2_session_hibernating_list.txt", _SubRep.c_str());
}


std::string CR2SessionBackupModule::getOverrideRingAccessFilename() const
{
	return NLMISC::toString("%soverride_ring_access.txt", _SubRep.c_str());
}

void CR2SessionBackupModule::swapOverrideRingAccess(TOverrideRingAccess& access)
{
	_OverrideRingAccess.swap(access);
}


void CR2SessionBackupModule_WantToBeLinked()
{
	
}


} // </R2>
