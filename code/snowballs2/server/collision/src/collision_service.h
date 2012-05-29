// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


#ifndef SERVICE_EMPTY_SERVICE_H
#define SERVICE_EMPTY_SERVICE_H

// This include is mandatory to use NeL. It include NeL types.
#include <nel/misc/types_nl.h>

// Some other NeL structures that can be used.
#include <nel/misc/vector.h>
#include <nel/misc/vectord.h>
#include <nel/misc/time_nl.h>

// The stuff from PACS
#include <nel/pacs/u_retriever_bank.h>
#include <nel/pacs/u_global_retriever.h>
#include <nel/pacs/u_move_container.h>
#include <nel/pacs/u_move_primitive.h>

// And we're also using the NeL Service framework, layer 5.
#include <nel/net/service.h>

namespace SBSERVICE {

class CCollisionService : public NLNET::IService
{
protected:
	struct CEntity
	{
		CEntity() : OldClientPosition(), NewClientPosition(), MovePrimitive(NULL), Distance(0.0f), Moving(false), Retry(false) { }
		NLMISC::CVector OldClientPosition;
		NLMISC::CVector NewClientPosition;
		NLPACS::UMovePrimitive *MovePrimitive; // ServerPosition
		float Distance;
		bool Moving;
		bool Retry;
	};
	typedef std::map<uint32, CEntity> CEntityMap;
	typedef std::map<uint16, CEntityMap> CClientMap;
	typedef std::vector<NLPACS::UMovePrimitive *> CMovePrimitiveVector;

private:
	// The list of registered services, their entities, and their move primitive
	static CClientMap _Clients;
	// The retriever bank used in the world
	static NLPACS::URetrieverBank *_RetrieverBank;
	// The global retriever used for pacs
	static NLPACS::UGlobalRetriever *_GlobalRetriever;
	// The move container used for dynamic collisions
	static NLPACS::UMoveContainer *_MoveContainer;
	// The collision primitive for the static entities
	static CMovePrimitiveVector _StaticMovePrimitives;
	// Time
	static NLMISC::TTime _LastTime, _NewTime, _DiffTime;
	static double _DiffTimeSeconds;
	static float _DiffTimeFloat;
	
public:
	virtual void commandStart();
	virtual void init();
	virtual bool update();
	virtual void release();

	static void sendMessage(NLNET::CMessage &msgout);

	static void msgUpdate();
	static void msgPosition(uint32 id, NLMISC::CVector position);

	static void cbAdd(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);
	static void cbMove(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);
	static void cbRemove(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);
	static void cbRegister(NLNET::CMessage &msgin, const std::string &serviceName, NLNET::TServiceId sid);

	static void cbDown(const std::string &serviceName, NLNET::TServiceId sid, void *arg);
};

}

#endif /* SERVICE_EMPTY_SERVICE_H */

/* end of file */
