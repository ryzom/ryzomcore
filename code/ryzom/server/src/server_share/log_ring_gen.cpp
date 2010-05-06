
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
