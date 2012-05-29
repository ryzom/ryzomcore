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
#include "log_character_gen.h"


#include "logger_service_itf.h"
#include "logger_service_client.h"

// A function fo force linking of this code module
void forceLink_Character(){}




class CCharacterDesc
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
	CCharacterDesc()
		:	_NoContextCount(0)
	{
		_LogDefs.resize(11);
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[0];
			
			logDef.setLogName("Character_BuyRolemasterPhrase");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[1];
			
			logDef.setLogName("Character_AdminCommand");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[2];
			
			logDef.setLogName("Character_SkillProgress");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[3];
			
			logDef.setLogName("Character_MissionRecvXp");
			
			logDef.setContext(true);

			
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[4];
			
			

			logDef.setLogName("Character_Create");
			logDef.setLogText("Character Created");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("userId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("charId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("charName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[5];
			
			

			logDef.setLogName("Character_Delete");
			logDef.setLogText("Character deleted");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("userId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("charId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("charName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[6];
			
			

			logDef.setLogName("Character_Select");
			logDef.setLogText("A character has been selected to play");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("userId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("charId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("charName");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[7];
			
			

			logDef.setLogName("Character_LevelUp");
			logDef.setLogText("A character has gained a level");

			logDef.getParams().resize(3);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
			logDef.getParams()[0].setList(false);
				
			logDef.getParams()[1].setName("skillName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("level");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_uint32);
			logDef.getParams()[2].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[8];
			
			

			logDef.setLogName("Character_UpdateSP");
			logDef.setLogText("A character Skill Point is updated");

			logDef.getParams().resize(4);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("spName");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_string);
			logDef.getParams()[1].setList(false);
				
			logDef.getParams()[2].setName("spBefore");
			logDef.getParams()[2].setType(LGS::TSupportedParamType::spt_float);
			logDef.getParams()[2].setList(false);
				
			logDef.getParams()[3].setName("spAfter");
			logDef.getParams()[3].setType(LGS::TSupportedParamType::spt_float);
			logDef.getParams()[3].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[9];
			
			

			logDef.setLogName("Character_LearnPhrase");
			logDef.setLogText("A character learn a new rolemaster phrase");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("phraseId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_sheetId);
			logDef.getParams()[1].setList(false);
				
		}
		
		{
			LGS::TLogDefinition  &logDef = _LogDefs[10];
			
			

			logDef.setLogName("Character_AddKnownBrick");
			logDef.setLogText("A character receive a new brick");

			logDef.getParams().resize(2);
			logDef.getListParams().resize(0);

			
			logDef.getParams()[0].setName("charId");
			logDef.getParams()[0].setType(LGS::TSupportedParamType::spt_entityId);
				
			logDef.getParams()[1].setName("brickId");
			logDef.getParams()[1].setType(LGS::TSupportedParamType::spt_sheetId);
			logDef.getParams()[1].setList(false);
				
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
CCharacterDesc	CharacterDesc;



const std::string TLogContext_Character_BuyRolemasterPhrase::_ContextName("Character_BuyRolemasterPhrase");
/// The constructor push a log context in the logger system
TLogContext_Character_BuyRolemasterPhrase::TLogContext_Character_BuyRolemasterPhrase(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	CharacterDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Character_BuyRolemasterPhrase::~TLogContext_Character_BuyRolemasterPhrase()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	CharacterDesc.popContextVar_charId();
	
}

const std::string TLogContext_Character_AdminCommand::_ContextName("Character_AdminCommand");
/// The constructor push a log context in the logger system
TLogContext_Character_AdminCommand::TLogContext_Character_AdminCommand(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	CharacterDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Character_AdminCommand::~TLogContext_Character_AdminCommand()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	CharacterDesc.popContextVar_charId();
	
}

const std::string TLogContext_Character_SkillProgress::_ContextName("Character_SkillProgress");
/// The constructor push a log context in the logger system
TLogContext_Character_SkillProgress::TLogContext_Character_SkillProgress(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	CharacterDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Character_SkillProgress::~TLogContext_Character_SkillProgress()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	CharacterDesc.popContextVar_charId();
	
}

const std::string TLogContext_Character_MissionRecvXp::_ContextName("Character_MissionRecvXp");
/// The constructor push a log context in the logger system
TLogContext_Character_MissionRecvXp::TLogContext_Character_MissionRecvXp(const NLMISC::CEntityId &charId)
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->pushLogContext(_ContextName);

	// stack the context param in the context class object
	CharacterDesc.pushContextVar_charId(charId);
	
}

/// The destructor pop a context in the logger system
TLogContext_Character_MissionRecvXp::~TLogContext_Character_MissionRecvXp()
{
	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->popLogContext(_ContextName);

	// pop the context param in the context class object
	CharacterDesc.popContextVar_charId();
	
}


/// No context context. Use this to disable any contextual log underneath
TLogNoContext_Character::TLogNoContext_Character()
{
	CharacterDesc.pushNoContext();
}

TLogNoContext_Character::~TLogNoContext_Character()
{
	CharacterDesc.popNoContext();
}



void _log_Character_Create(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_Create");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(userId);
		
	logInfo.getParams()[1] = LGS::TParamValue(charId);
		
	logInfo.getParams()[2] = LGS::TParamValue(charName);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_Delete(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_Delete");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(userId);
		
	logInfo.getParams()[1] = LGS::TParamValue(charId);
		
	logInfo.getParams()[2] = LGS::TParamValue(charName);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_Select(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_Select");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(userId);
		
	logInfo.getParams()[1] = LGS::TParamValue(charId);
		
	logInfo.getParams()[2] = LGS::TParamValue(charName);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_LevelUp(const NLMISC::CEntityId &charId, const std::string &skillName, uint32 level, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_LevelUp");
		logInfo.getParams().resize(3);
		logInfo.getListParams().resize(0);
	}

	
	logInfo.getParams()[0] = LGS::TParamValue(charId);
		
	logInfo.getParams()[1] = LGS::TParamValue(skillName);
		
	logInfo.getParams()[2] = LGS::TParamValue(level);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_UpdateSP(const std::string &spName, float spBefore, float spAfter, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_UpdateSP");
		logInfo.getParams().resize(4);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CharacterDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CharacterDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Character'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(spName);
		
	logInfo.getParams()[2] = LGS::TParamValue(spBefore);
		
	logInfo.getParams()[3] = LGS::TParamValue(spAfter);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_LearnPhrase(const NLMISC::CSheetId &phraseId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_LearnPhrase");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CharacterDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CharacterDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Character'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(phraseId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_AddKnownBrick(const NLMISC::CSheetId &brickId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_AddKnownBrick");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CharacterDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CharacterDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Character'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(brickId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}

void _log_Character_RemoveKnownBrick(const NLMISC::CSheetId &brickId, const char *_filename_, uint _lineNo_)
{
	static LGS::TLogInfo logInfo;
	static bool init = false;
	if (!init)
	{
		logInfo.setLogName("Character_RemoveKnownBrick");
		logInfo.getParams().resize(2);
		logInfo.getListParams().resize(0);
	}

	
	// Context parameter
		NLMISC::CEntityId	charId;
	if (!CharacterDesc.getContextVar_charId(charId))
	{
		// If this bomb is thrown, you need to add a log context (or eventualy a 'noContext').
		STOP_IF(CharacterDesc.getNoContextCount() == 0, _filename_<<"("<<_lineNo_<<") : Missing log context for log 'Character'");
		return;
	}

			
	logInfo.getParams()[0] = LGS::TParamValue(charId);
			
	logInfo.getParams()[1] = LGS::TParamValue(brickId);
		

	logInfo.setTimeStamp(NLMISC::CTime::getSecondsSince1970());

	if (LGS::ILoggerServiceClient::isInitialized())
		LGS::ILoggerServiceClient::getInstance()->sendLog(logInfo);
}
