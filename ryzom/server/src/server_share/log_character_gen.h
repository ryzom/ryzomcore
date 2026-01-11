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



#ifndef _LOG_GEN_CHARACTER_H
#define _LOG_GEN_CHARACTER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"


struct TLogContext_Character_BuyRolemasterPhrase
{
	/// The constructor push a log context in the logger system
	TLogContext_Character_BuyRolemasterPhrase(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Character_BuyRolemasterPhrase();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Character_AdminCommand
{
	/// The constructor push a log context in the logger system
	TLogContext_Character_AdminCommand(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Character_AdminCommand();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Character_SkillProgress
{
	/// The constructor push a log context in the logger system
	TLogContext_Character_SkillProgress(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Character_SkillProgress();

private:
	/// The name of the context
	static const std::string _ContextName;


};

struct TLogContext_Character_MissionRecvXp
{
	/// The constructor push a log context in the logger system
	TLogContext_Character_MissionRecvXp(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Character_MissionRecvXp();

private:
	/// The name of the context
	static const std::string _ContextName;


};


/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Character
{
	TLogNoContext_Character();
	~TLogNoContext_Character();
};



void _log_Character_Create(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_);
#define log_Character_Create(userId, charId, charName) \
	_log_Character_Create(userId, charId, charName, __FILE__, __LINE__)

void _log_Character_Delete(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_);
#define log_Character_Delete(userId, charId, charName) \
	_log_Character_Delete(userId, charId, charName, __FILE__, __LINE__)

void _log_Character_Select(uint32 userId, const NLMISC::CEntityId &charId, const std::string &charName, const char *_filename_, uint _lineNo_);
#define log_Character_Select(userId, charId, charName) \
	_log_Character_Select(userId, charId, charName, __FILE__, __LINE__)

void _log_Character_LevelUp(const NLMISC::CEntityId &charId, const std::string &skillName, uint32 level, const char *_filename_, uint _lineNo_);
#define log_Character_LevelUp(charId, skillName, level) \
	_log_Character_LevelUp(charId, skillName, level, __FILE__, __LINE__)

void _log_Character_UpdateSP(const std::string &spName, float spBefore, float spAfter, const char *_filename_, uint _lineNo_);
#define log_Character_UpdateSP(spName, spBefore, spAfter) \
	_log_Character_UpdateSP(spName, spBefore, spAfter, __FILE__, __LINE__)

void _log_Character_LearnPhrase(const NLMISC::CSheetId &phraseId, const char *_filename_, uint _lineNo_);
#define log_Character_LearnPhrase(phraseId) \
	_log_Character_LearnPhrase(phraseId, __FILE__, __LINE__)

void _log_Character_AddKnownBrick(const NLMISC::CSheetId &brickId, const char *_filename_, uint _lineNo_);
#define log_Character_AddKnownBrick(brickId) \
	_log_Character_AddKnownBrick(brickId, __FILE__, __LINE__)

void _log_Character_RemoveKnownBrick(const NLMISC::CSheetId &brickId, const char *_filename_, uint _lineNo_);
#define log_Character_RemoveKnownBrick(brickId) \
	_log_Character_RemoveKnownBrick(brickId, __FILE__, __LINE__)

#endif

