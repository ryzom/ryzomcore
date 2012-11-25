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

#ifndef R2_AI_WRAPPER_H
#define R2_AI_WRAPPER_H

#include <string>
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/singleton.h"
#include "nel/misc/entity_id.h"
#include "game_share/r2_share_itf.h"
#include "game_share/misc_const.h"

class CPersistentDataRecord;

namespace NLLIGO
{
	class CLigoConfig;
	class CPrimitives;
}

namespace NLMISC
{
	class IStream;
}

namespace R2
{
// This is a virtual that is referenced in both client and server apps
// - in the client the default empty implementations of the methods are used
// - in the server the CAiWrapperServer specialization is used

class IAiWrapper
{
public:
	virtual ~IAiWrapper()
	{
	}

	virtual void init(NLLIGO::CLigoConfig *             ligoConfig)
	{
	}

	virtual void streamToPdr(NLMISC::IStream& stream, const std::string& primName, CPersistentDataRecord& pdr)
	{
	}

	virtual void primsToPdr(NLLIGO::CPrimitives *prims, const std::string& primName, CPersistentDataRecord& pdr)
	{
	}

	virtual void pdrToFile(CPersistentDataRecord& pdr, const std::string& pdrName)
	{
	}

	virtual void fileToPdr(const std::string& pdrName, CPersistentDataRecord& pdr)
	{
	}

	virtual void displayPdr( CPersistentDataRecord& pdr)
	{
	}

	virtual void clearPdr( CPersistentDataRecord& pdr)
	{
	}

	virtual void primitiveFileToPdr(const std::string& primitiveName, CPersistentDataRecord& pdr)
	{
	}

	virtual void stopTest(TSessionId sessionId, uint32 aiInstance)
	{
	}

	virtual void startTest(TSessionId sessionId, uint32 aiInstance, CPersistentDataRecord& pdr)
	{
	}

	virtual void stopAct(TSessionId sessionId, uint32 aiInstance)
	{
	}

	virtual void startAct(TSessionId sessionId, uint32 aiInstance, CPersistentDataRecord& pdr)
	{
	}

	virtual void despawnEntity(NLMISC::CEntityId entityId, TAIAlias alias)
	{
	}

	virtual void setAggroRange(NLMISC::CEntityId entityId, float range)
	{
	}

	virtual void setHPLevel(NLMISC::CEntityId entityId, uint32 alias,  float value)
	{
	}

	virtual void setGrpHPLevel(NLMISC::CEntityId entityId, uint32 alias,  float value)
	{
	}

	virtual		void startInstance(TSessionId sessionId, uint32 aiInstance)
	{
	}
	virtual void triggerGrpEvent(NLMISC::CEntityId entityId, float eventId)
	{
	}
	virtual void setPioneerRight(NLMISC::CEntityId clientId, const R2::TPioneerRight& right)
	{
	}

	virtual void controlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc)
	{
	}

	virtual void stopControlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc)
	{
	}

	virtual void triggerUserTrigger(const std::string& groupName, uint32 triggerId)
	{
	}

	virtual void askBotDespawnNotification(NLMISC::CEntityId creatureId, TAIAlias alias)
	{
	}
};


// This specialization of IAiWrapper is the true implementation of CAiWrapper for use by server apps

class CAiWrapperServer: public IAiWrapper
{
public:
	void init(NLLIGO::CLigoConfig *             ligoConfig);

	virtual void streamToPdr(NLMISC::IStream& stream, const std::string& primName, CPersistentDataRecord& pdr);

	virtual void primsToPdr(NLLIGO::CPrimitives *prims, const std::string& primName, CPersistentDataRecord& pdr);

	virtual void pdrToFile(CPersistentDataRecord& pdr, const std::string& pdrName);

	virtual void fileToPdr(const std::string& pdrName, CPersistentDataRecord& pdr);

	virtual void displayPdr( CPersistentDataRecord& pdr);

	virtual void clearPdr( CPersistentDataRecord& pdr);

	virtual void primitiveFileToPdr(const std::string& primitiveName, CPersistentDataRecord& pdr);

	virtual void stopTest(TSessionId sessionId, uint32 aiInstance);

	virtual void startTest(TSessionId sessionId, uint32 aiInstance, CPersistentDataRecord& pdr);

	virtual void stopAct(TSessionId sessionId, uint32 aiInstance);

	virtual void startAct(TSessionId sessionId, uint32 aiInstance, CPersistentDataRecord& pdr);

	virtual void despawnEntity(NLMISC::CEntityId entityId, TAIAlias alias);

	virtual void setAggroRange(NLMISC::CEntityId entityId, float range);

	virtual void setHPLevel(NLMISC::CEntityId entityId, uint32 alias, float value);
	virtual void setGrpHPLevel(NLMISC::CEntityId entityId, uint32 alias, float value);


	virtual void triggerGrpEvent(NLMISC::CEntityId entityId, float eventId);

	virtual void startInstance(TSessionId sessionId, uint32 aiInstance);

	virtual void setPioneerRight(NLMISC::CEntityId clientId, const R2::TPioneerRight& right);

	virtual void controlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc);

	virtual void stopControlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc);

	virtual void triggerUserTrigger(const std::string& groupName, uint32 triggerId);

	virtual void askBotDespawnNotification(NLMISC::CEntityId creatureId, TAIAlias alias);


};

class CAiWrapper
{
public:
	static IAiWrapper& getInstance()
	{
		return *_instance();
	}

	static void setInstance(IAiWrapper* theInstance)
	{
		IAiWrapper*& inst= _instance();
		delete inst;
		inst= theInstance;
	}

private:
	// implementation of a singleton variable
	static IAiWrapper*& _instance()
	{
		static IAiWrapper* inst= NULL;
		if (inst==NULL)
			inst= new IAiWrapper;
		return inst;
	}
};

}	// namespace R2
#endif
