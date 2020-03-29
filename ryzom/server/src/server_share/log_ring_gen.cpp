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
#include "game_share/utils.h"
#include "log_ring_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Ring(){}




class CRingDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	CRingDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(2);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			

			logDef.setLogName("Ring_EnterSession");
			logDef.setLogText("A character enter a ring session");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("sessionId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[1].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			

			logDef.setLogName("Ring_LeaveSession");
			logDef.setLogText("A character leave a ring session");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("sessionId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[1].setList(false);
				
		}
		

		// Register the log definitions
		LGS::ILoggerServiceClient::addLogDefinitions(_LogDefs);
	}

	// Context var stack accessor
	

	void pushNoContext()
	{
		++_NoContextCount;
	}
	void popNoContext()
	{
		nlassert(_NoContextCount > 0);
		--_NoContextCount;
	}

	uint32 getNoContextCount()
	{
		return _NoContextCount;
	}

};
// Instantiate the descriptor class
CRingDesc	RingDesc;




/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Ring::TLogNoContext_Ring()
{
	RingDesc.pushNoContext();
}

TLogNoContext_Ring::~TLogNoContext_Ring()
{
	RingDesc.popNoContext();
}



void _log_Ring_EnterSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Ring_EnterSession");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(charId);
		
	logInfo.getParams()[1] = LGS::TParamValue(sessionId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Ring_LeaveSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Ring_LeaveSession");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(charId);
		
	logInfo.getParams()[1] = LGS::TParamValue(sessionId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
