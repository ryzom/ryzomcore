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
#include "log_outpost_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Outpost(){}




class COutpostDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	COutpostDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(4);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			

			logDef.setLogName("Outpost_Challenge");
			logDef.setLogText("A guild challenge an outpost");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("outpostName");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("ownerGuildName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("challengerGuildName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			

			logDef.setLogName("Outpost_ChallengeWin");
			logDef.setLogText("The guild challenging the outpost has win");

			logDef.getParams().resize(4);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("outpostName");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("oldOwnerGuildName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("newOwnerGuildName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("winLevel");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[3].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[2];
			
			

			logDef.setLogName("Outpost_ChallengeLost");
			logDef.setLogText("The guild challenging the outpost has lost");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("outpostName");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("ownerGuildName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("challengerGuildName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[3];
			
			

			logDef.setLogName("Outpost_BuyOption");
			logDef.setLogText("The guild bought an option for an outpost");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("outpostName");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("ownerGuildName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("buildingSheet");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_sheetId);
			logDef.getParams()[2].setList(false);
				
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
COutpostDesc	OutpostDesc;




/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Outpost::TLogNoContext_Outpost()
{
	OutpostDesc.pushNoContext();
}

TLogNoContext_Outpost::~TLogNoContext_Outpost()
{
	OutpostDesc.popNoContext();
}



void _log_Outpost_Challenge(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Outpost_Challenge");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(outpostName);
		
	logInfo.getParams()[1] = LGS::TParamValue(ownerGuildName);
		
	logInfo.getParams()[2] = LGS::TParamValue(challengerGuildName);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Outpost_ChallengeWin(const std::string &outpostName, const std::string &oldOwnerGuildName, const std::string &newOwnerGuildName, uint32 winLevel, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Outpost_ChallengeWin");
		logInfo.getParams().resize(4);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(outpostName);
		
	logInfo.getParams()[1] = LGS::TParamValue(oldOwnerGuildName);
		
	logInfo.getParams()[2] = LGS::TParamValue(newOwnerGuildName);
		
	logInfo.getParams()[3] = LGS::TParamValue(winLevel);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Outpost_ChallengeLost(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Outpost_ChallengeLost");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(outpostName);
		
	logInfo.getParams()[1] = LGS::TParamValue(ownerGuildName);
		
	logInfo.getParams()[2] = LGS::TParamValue(challengerGuildName);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Outpost_BuyOption(const std::string &outpostName, const std::string &ownerGuildName, NLMISC::CSheetId buildingSheet, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Outpost_BuyOption");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(outpostName);
		
	logInfo.getParams()[1] = LGS::TParamValue(ownerGuildName);
		
	logInfo.getParams()[2] = LGS::TParamValue(buildingSheet);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
