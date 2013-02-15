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
#include "log_command_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Command(){}




class CCommandDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	
	std::vector<NLMISC::CEntityId>	_charId;
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	CCommandDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(4);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			logDef.setLogName("Command_ExecCtx");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			

			logDef.setLogName("Command_Exec");
			logDef.setLogText("A character execute an admin command");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("cmdName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("cmdArg");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[2];
			
			

			logDef.setLogName("Command_ExecOnTarget");
			logDef.setLogText("A character execute an admin command on its target");

			logDef.getParams().resize(4);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("targetId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("cmdName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("cmdArg");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[3].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[3];
			
			

			logDef.setLogName("Command_TPOutsideNewbieland");
			logDef.setLogText("A CSR has tp'ed a player outside of the newbieland");

			logDef.getParams().resize(1);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("movedCharId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[0].setList(false);
				
		}
		

		// Register the log definitions
		LGS::ILoggerServiceClient::addLogDefinitions(_LogDefs);
	}

	// Context var stack accessor
	
	bool getContextVar_charId (NLMISC::CEntityId &value)
	{
		if (_charId.empty())
			return false;

		value = _charId.back();
		return true;
	}

	void pushContextVar_charId (const NLMISC::CEntityId &value)
	{
		_charId.push_back(value);
	}
	void popContextVar_charId ()
	{
		_charId.pop_back();
	}
	

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
CCommandDesc	CommandDesc;



const std::string TLogContext_Command_ExecCtx::_ContextName("Command_ExecCtx");
/// The constructor push a log context in the logger system
TLogContext_Command_ExecCtx::TLogContext_Command_ExecCtx(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	CommandDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Command_ExecCtx::~TLogContext_Command_ExecCtx()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	CommandDesc.popContextVar_charId();
	
}


/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Command::TLogNoContext_Command()
{
	CommandDesc.pushNoContext();
}

TLogNoContext_Command::~TLogNoContext_Command()
{
	CommandDesc.popNoContext();
}



void _log_Command_Exec(const std::string &cmdName, const std::string &cmdArg, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Command_Exec");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CommandDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CommandDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Command'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(cmdName);
		
	logInfo.getParams()[2] = LGS::TParamValue(cmdArg);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Command_ExecOnTarget(const NLMISC::CEntityId &targetId, const std::string &cmdName, const std::string &cmdArg, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Command_ExecOnTarget");
		logInfo.getParams().resize(4);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CommandDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CommandDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Command'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(targetId);
		
	logInfo.getParams()[2] = LGS::TParamValue(cmdName);
		
	logInfo.getParams()[3] = LGS::TParamValue(cmdArg);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Command_TPOutsideNewbieland(const NLMISC::CEntityId &movedCharId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Command_TPOutsideNewbieland");
		logInfo.getParams().resize(1);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(movedCharId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
