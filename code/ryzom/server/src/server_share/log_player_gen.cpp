
#include "stdpch.h"
#include "game_share/utils.h"
#include "log_player_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Player(){}




class CPlayerDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	CPlayerDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(2);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			

			logDef.setLogName("Player_Connect");
			logDef.setLogText("The player has connected");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("userId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("fromAddr");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			

			logDef.setLogName("Player_Disconnect");
			logDef.setLogText("The player has disconnected");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("userId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("crashed");
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
CPlayerDesc	PlayerDesc;




/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Player::TLogNoContext_Player()
{
	PlayerDesc.pushNoContext();
}

TLogNoContext_Player::~TLogNoContext_Player()
{
	PlayerDesc.popNoContext();
}



void _log_Player_Connect(uint32 userId, const std::string &fromAddr, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Player_Connect");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(userId);
		
	logInfo.getParams()[1] = LGS::TParamValue(fromAddr);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Player_Disconnect(uint32 userId, bool crashed, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Player_Disconnect");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(userId);
		
	logInfo.getParams()[1] = LGS::TParamValue(crashed);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
