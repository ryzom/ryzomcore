
#ifndef CHAR_NAME_MAPPER_CLIENT_H
#define CHAR_NAME_MAPPER_CLIENT_H

#include "nel/misc/singleton.h"


/** Interface to the char name mapper client singleton */
class ICharNameMapperClient : public NLMISC::CManualSingleton<ICharNameMapperClient>
{
public:

	virtual void mapCharacterName(const NLMISC::CEntityId &charEid, const ucstring &charName) = 0;
};

#endif // CHAR_NAME_MAPPER_CLIENT_H
