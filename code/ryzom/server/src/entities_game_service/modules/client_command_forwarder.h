
#include "nel/misc/types_nl.h"
#include "nel/misc/singleton.h"


class IClientCommandForwader
:	public NLMISC::CManualSingleton<IClientCommandForwader>
{
public:

	virtual void sendCommand(const std::string &service, const std::string &commandName, const NLMISC::CEntityId &senderEId, bool haveTarget, const NLMISC::CEntityId &targetEId, const std::string &arg) =0;
};
