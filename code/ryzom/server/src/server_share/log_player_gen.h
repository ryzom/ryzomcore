


#ifndef _LOG_GEN_PLAYER_H
#define _LOG_GEN_PLAYER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Player
{
	TLogNoContext_Player();
	~TLogNoContext_Player();
};



void _log_Player_Connect(uint32 userId, const std::string &fromAddr, const char *_filename_, uint _lineNo_);
#define log_Player_Connect(userId, fromAddr) \
	_log_Player_Connect(userId, fromAddr, __FILE__, __LINE__)

void _log_Player_Disconnect(uint32 userId, bool crashed, const char *_filename_, uint _lineNo_);
#define log_Player_Disconnect(userId, crashed) \
	_log_Player_Disconnect(userId, crashed, __FILE__, __LINE__)


#endif

