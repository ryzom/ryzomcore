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
#include "log_chat_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Chat(){}




class CChatDesc
{
	friend class CLoggerClient;

	/// The list of log definition for this log class
	std::vector<LGS::TLogDefinition>	_LogDefs;

	/// Stack of context variable
	

	/// Counter of 'no context' object stacked.
	uint32	_NoContextCount;

public:
	/// constructor
	CChatDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(2);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			

			logDef.setLogName("Chat_Chat");
			logDef.setLogText("A Character send a chat to a channel");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(1);

			
			logDef.getParams()[0].setName("channel");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("senderCharId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("chatText");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
			logDef.getListParams()[0].setName("receiverCharIds");
			logDef.getListParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getListParams()[0].setList(true);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			

			logDef.setLogName("Chat_Tell");
			logDef.setLogText("A Character send a tell to another char");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("senderCharId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("receiverCharId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("chatText");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
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
CChatDesc	ChatDesc;




/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Chat::TLogNoContext_Chat()
{
	ChatDesc.pushNoContext();
}

TLogNoContext_Chat::~TLogNoContext_Chat()
{
	ChatDesc.popNoContext();
}



void _log_Chat_Chat(std::string channel, const NLMISC::CEntityId &senderCharId, const std::string &chatText, const std::list< NLMISC::CEntityId > &receiverCharIds, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Chat_Chat");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(1);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(channel);
		
	logInfo.getParams()[1] = LGS::TParamValue(senderCharId);
		
	logInfo.getParams()[2] = LGS::TParamValue(chatText);
		
	LGS::TListParamValues &receiverCharIds_list = logInfo.getListParams()[0];
	receiverCharIds_list.getParams().clear();
	std::list<NLMISC::CEntityId>::const_iterator first(receiverCharIds.begin()), last(receiverCharIds.end());
	for (; first != last; ++first)
	{
			receiverCharIds_list.getParams().push_back(LGS::TParamValue(*first));
			
	}

		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Chat_Tell(const NLMISC::CEntityId &senderCharId, const NLMISC::CEntityId &receiverCharId, const std::string &chatText, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Chat_Tell");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(senderCharId);
		
	logInfo.getParams()[1] = LGS::TParamValue(receiverCharId);
		
	logInfo.getParams()[2] = LGS::TParamValue(chatText);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
