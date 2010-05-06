
#ifndef CHAT_UNIFIER_CLIENT_H
#define CHAT_UNIFIER_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"

class IChatUnifierClient : public NLMISC::CManualSingleton<IChatUnifierClient>
{
public:

	virtual void sendFarTell(const NLMISC::CEntityId &senderCharId, bool havePrivilege, const ucstring &destName, const ucstring &text) =0;
	virtual void sendFarGuildChat(const ucstring &senderName, uint32 guildId, const ucstring &text) =0;
	virtual void sendFarGuildChat2(const ucstring &senderName, uint32 guildId, const std::string &phraseName) =0;
	virtual void sendFarGuildChat2Ex(const ucstring &senderName, uint32 guildId, uint32 phraseId) =0;
	virtual void sendUniverseChat(const ucstring &senderName, uint32 homeSessionId, const ucstring &text) = 0;
	virtual void sendUnifiedDynChat(const NLMISC::CEntityId &dynCharId, const ucstring &senderName, const ucstring &text) = 0;
};

#endif // CHAT_UNIFIER_CLIENT_H
