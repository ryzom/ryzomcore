


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

