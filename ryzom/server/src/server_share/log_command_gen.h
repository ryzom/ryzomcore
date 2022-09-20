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



#ifndef _LOG_GEN_COMMAND_H
#define _LOG_GEN_COMMAND_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"


struct TLogContext_Command_ExecCtx
{
	/// The constructor push a log context in the logger system
	TLogContext_Command_ExecCtx(const NLMISC::CEntityId &charId);

	/// The desstructor pop a context in the logger system
	~TLogContext_Command_ExecCtx();

private:
	/// The name of the context
	static const std::string _ContextName;


};


/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Command
{
	TLogNoContext_Command();
	~TLogNoContext_Command();
};



void _log_Command_Exec(const std::string &cmdName, const std::string &cmdArg, const char *_filename_, uint _lineNo_);
#define log_Command_Exec(cmdName, cmdArg) \
	_log_Command_Exec(cmdName, cmdArg, __FILE__, __LINE__)

void _log_Command_ExecOnTarget(const NLMISC::CEntityId &targetId, const std::string &cmdName, const std::string &cmdArg, const char *_filename_, uint _lineNo_);
#define log_Command_ExecOnTarget(targetId, cmdName, cmdArg) \
	_log_Command_ExecOnTarget(targetId, cmdName, cmdArg, __FILE__, __LINE__)

void _log_Command_TPOutsideNewbieland(const NLMISC::CEntityId &movedCharId, const char *_filename_, uint _lineNo_);
#define log_Command_TPOutsideNewbieland(movedCharId) \
	_log_Command_TPOutsideNewbieland(movedCharId, __FILE__, __LINE__)


#endif

