


#ifndef _LOG_GEN_RING_H
#define _LOG_GEN_RING_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"


#include "game_share/ring_session_manager_itf.h"


/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Ring
{
	TLogNoContext_Ring();
	~TLogNoContext_Ring();
};



void _log_Ring_EnterSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_);
#define log_Ring_EnterSession(charId, sessionId) \
	_log_Ring_EnterSession(charId, sessionId, __FILE__, __LINE__)

void _log_Ring_LeaveSession(const NLMISC::CEntityId &charId, uint32 sessionId, const char *_filename_, uint _lineNo_);
#define log_Ring_LeaveSession(charId, sessionId) \
	_log_Ring_LeaveSession(charId, sessionId, __FILE__, __LINE__)


#endif

