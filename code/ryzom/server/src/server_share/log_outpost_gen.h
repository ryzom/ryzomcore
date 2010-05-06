


#ifndef _LOG_GEN_OUTPOST_H
#define _LOG_GEN_OUTPOST_H

#include "nel/misc/types_nl.h"
#include "nel/misc/entity_id.h"
#include "nel/misc/sheet_id.h"
#include "game_share/inventories.h"



/// No context context. Use this to disable any contextual log underneath
struct TLogNoContext_Outpost
{
	TLogNoContext_Outpost();
	~TLogNoContext_Outpost();
};



void _log_Outpost_Challenge(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_);
#define log_Outpost_Challenge(outpostName, ownerGuildName, challengerGuildName) \
	_log_Outpost_Challenge(outpostName, ownerGuildName, challengerGuildName, __FILE__, __LINE__)

void _log_Outpost_ChallengeWin(const std::string &outpostName, const std::string &oldOwnerGuildName, const std::string &newOwnerGuildName, uint32 winLevel, const char *_filename_, uint _lineNo_);
#define log_Outpost_ChallengeWin(outpostName, oldOwnerGuildName, newOwnerGuildName, winLevel) \
	_log_Outpost_ChallengeWin(outpostName, oldOwnerGuildName, newOwnerGuildName, winLevel, __FILE__, __LINE__)

void _log_Outpost_ChallengeLost(const std::string &outpostName, const std::string &ownerGuildName, const std::string &challengerGuildName, const char *_filename_, uint _lineNo_);
#define log_Outpost_ChallengeLost(outpostName, ownerGuildName, challengerGuildName) \
	_log_Outpost_ChallengeLost(outpostName, ownerGuildName, challengerGuildName, __FILE__, __LINE__)

void _log_Outpost_BuyOption(const std::string &outpostName, const std::string &ownerGuildName, NLMISC::CSheetId buildingSheet, const char *_filename_, uint _lineNo_);
#define log_Outpost_BuyOption(outpostName, ownerGuildName, buildingSheet) \
	_log_Outpost_BuyOption(outpostName, ownerGuildName, buildingSheet, __FILE__, __LINE__)


#endif

