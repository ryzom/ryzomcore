


#ifndef _LOG_GEN_CHAT_H
#define _LOG_GEN_CHAT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Chat
{
	TLogNoContext_Chat();
	~TLogNoContext_Chat();
};



void _log_Chat_Chat(std::string channel, const NLMISC::CEntityId &senderCharId, const std::string &chatText, const std::list< NLMISC::CEntityId > &receiverCharIds, const char *_filename_, uint _lineNo_);
#define log_Chat_Chat(channel, senderCharId, chatText, receiverCharIds) \
	_log_Chat_Chat(channel, senderCharId, chatText, receiverCharIds, __FILE__, __LINE__)

void _log_Chat_Tell(const NLMISC::CEntityId &senderCharId, const NLMISC::CEntityId &receiverCharId, const std::string &chatText, const char *_filename_, uint _lineNo_);
#define log_Chat_Tell(senderCharId, receiverCharId, chatText) \
	_log_Chat_Tell(senderCharId, receiverCharId, chatText, __FILE__, __LINE__)


#endif

