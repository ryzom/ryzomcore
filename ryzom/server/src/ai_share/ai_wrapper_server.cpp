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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "stdpch.h"

#include "game_share/ai_wrapper.h"

#include "../ai_share/ai_share.h"

#include "game_share/persistent_data.h"

#include "ai_primitive_parser.h"

using namespace R2;
using namespace NLLIGO;
using namespace NLNET;
using namespace NLMISC;

namespace R2
{
	NLLIGO::CLigoConfig * LigoConfigPtr;
}

void CAiWrapperServer::init(NLLIGO::CLigoConfig *  ligoConfig)
{
	const char* CLASS_FILE_NAME="world_editor_classes.xml";

	if ( ! CPrimitiveContext::instance().CurrentLigoConfig )
	{

		CPrimitiveContext::instance().CurrentLigoConfig = R2::LigoConfigPtr;
		// Read the ligo primitive class file
	//	NLLIGO::Register();

	}

	AI_SHARE::init(R2::LigoConfigPtr);
}

void CAiWrapperServer::streamToPdr(NLMISC::IStream& stream, const std::string& primName, CPersistentDataRecord& pdr)
{
	H_AUTO_INST( streamToPdr );
	CAIPrimitiveParser::init(&pdr);
	AI_SHARE::parsePrimStream(stream, primName.c_str());
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::primsToPdr(NLLIGO::CPrimitives *prims, const std::string& primName, CPersistentDataRecord& pdr)
{
	H_AUTO_INST( primsToPdr );
	CAIPrimitiveParser::init(&pdr);
	AI_SHARE::parsePrimNoStream(prims, primName.c_str());
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::pdrToFile(CPersistentDataRecord& pdr, const std::string& pdrName)
{
	CAIPrimitiveParser::init(&pdr);
	CAIPrimitiveParser::getInstance().writeFile(pdrName.c_str());
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::fileToPdr(const std::string& pdrName, CPersistentDataRecord& pdr)
{
	CAIPrimitiveParser::init(&pdr);
	CAIPrimitiveParser::getInstance().readFile(pdrName.c_str());
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::displayPdr( CPersistentDataRecord& pdr)
{
	CAIPrimitiveParser::init(&pdr);
	CAIPrimitiveParser::getInstance().display();
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::clearPdr( CPersistentDataRecord& pdr)
{
	CAIPrimitiveParser::init(&pdr);
	CAIPrimitiveParser::getInstance().clear();
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::primitiveFileToPdr(const std::string& primitiveName, CPersistentDataRecord& pdr)
{
	pdr.clear();
	CAIPrimitiveParser::init(&pdr);
	AI_SHARE::parsePrimFile(primitiveName.c_str());
	CAIPrimitiveParser::release();
}

void CAiWrapperServer::stopTest(TSessionId sessionId, uint32 aiInstanceId)
{
	nldebug("Stop Test in session %u (aiInstance %u)", sessionId.asInt(), aiInstanceId);
	CMessage msgout("R2_STOPLIVE");
	bool isBase = true;

	msgout.serial(aiInstanceId);
	msgout.serial(isBase);
	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::startTest(TSessionId sessionId, uint32 aiInstanceId,  CPersistentDataRecord& pdr)
{

	nldebug("Start Test in session %u (aiInstance %u)", sessionId.asInt(), aiInstanceId);
	uint32 totalDataSize = pdr.totalDataSize();
	char* dest = new char[totalDataSize];
	if (!pdr.toBuffer(dest, totalDataSize))
	{
		nlwarning("can't serialise data");
		return;
	}

	{
		CMessage msgout("R2_GOLIVE");
		bool isBase = true;
		msgout.serial(sessionId);
		msgout.serial(aiInstanceId);
		msgout.serial(isBase);
		msgout.serial(totalDataSize);
		msgout.serialBuffer (reinterpret_cast<uint8*>(dest), totalDataSize);
		CUnifiedNetwork::getInstance()->send("AIS",msgout);
	}
}

void CAiWrapperServer::startInstance(TSessionId sessionId, uint32 aiInstanceId)
{
	nldebug("Start Instance in session %u (aiInstance %u)", sessionId.asInt(), aiInstanceId);
	nlinfo("R2An: startInstance %u", aiInstanceId);
	CMessage msgout("R2_STARTINSTANCE");
	msgout.serial(aiInstanceId);
	CUnifiedNetwork::getInstance()->send("AIS",msgout);

}

void CAiWrapperServer::stopAct(TSessionId sessionId, uint32 aiInstanceId)
{
	nldebug("Stop Act in session %u (aiInstance %u)", sessionId.asInt(), aiInstanceId);
	CMessage msgout("R2_STOPLIVE");
	msgout.serial(aiInstanceId);
	bool isBase = false;
	msgout.serial(isBase);
	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::startAct(TSessionId sessionId, uint32 aiInstanceId, CPersistentDataRecord& pdr)
{
	nldebug("Start Act in session %u (aiInstance %u)", sessionId.asInt(), aiInstanceId);
	uint32 totalDataSize = pdr.totalDataSize();
	char* dest = new char[totalDataSize];
	if (!pdr.toBuffer(dest, totalDataSize))
	{
		nlwarning("can't serialise data");
		return;
	}

	{
		CMessage msgout("R2_GOLIVE");
		bool isBase = false;
		msgout.serial(sessionId);
		msgout.serial(aiInstanceId);
		msgout.serial(isBase);
		msgout.serial(totalDataSize);
		msgout.serialBuffer (reinterpret_cast<uint8*>(dest), totalDataSize);
		CUnifiedNetwork::getInstance()->send("AIS",msgout);
	}
}

void CAiWrapperServer::setAggroRange(NLMISC::CEntityId entityId, float range)
{
	CMessage msgout("EXEC_COMMAND");
	std::string command = NLMISC::toString("eventSetNpcGroupAggroRange %s %f", entityId.toString().c_str(), range);
	msgout.serial(command);
	CUnifiedNetwork::getInstance()->send("EGS",msgout);
}

namespace
{
	static std::string aliasToString(uint32 alias)
	{
		uint32 staticPart = alias >> 20;
		uint32 dynPart = alias  &  ((1 << 21)-1);
		return NLMISC::toString("(A:%u:%u)", staticPart, dynPart);
	}
}

void CAiWrapperServer::despawnEntity(NLMISC::CEntityId entityId, uint32 alias)
{
	uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  entityId.toString();

	std::string str = NLMISC::toString("()despawnBotByAlias(\"%s\");", aliasToString(alias).c_str());

	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(str);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::setGrpHPLevel(NLMISC::CEntityId entityId, uint32 alias,  float hp)
{
		uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  entityId.toString();

	std::string hpstr = NLMISC::toString("()setHPScale(%f);", hp);

	//CMessage msgout("EVENT_NPC_GROUP_SCRIPT");
	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(hpstr);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::setHPLevel(NLMISC::CEntityId entityId, uint32 alias, float hp)
{

	uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  entityId.toString();

	std::string hpstr = NLMISC::toString("()setBotHPScaleByAlias(%f, \"%s\");", hp, aliasToString(alias).c_str());

	//CMessage msgout("EVENT_NPC_GROUP_SCRIPT");
	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(hpstr);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}


void CAiWrapperServer::triggerGrpEvent(NLMISC::CEntityId entityId, float eventId)
{

	uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  entityId.toString();
	std::string str = NLMISC::toString("()setEvent(%f);", eventId);
	eventId = (float)(int) eventId;
	if (eventId < 0 || eventId > 10) return;

	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(str);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);

}

void CAiWrapperServer::controlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc)
{

	uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  npc.toString();
	std::string str = NLMISC::toString("()setPlayerController(\"%s\",\"%s\");",  npc.toString().c_str(), clientId.toString().c_str());

	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(str);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::stopControlNpc(NLMISC::CEntityId clientId, NLMISC::CEntityId npc)
{

	uint32 messageVersion = 1;
	uint32 nbString=2;
	std::string eid  =  npc.toString();
	std::string str = NLMISC::toString("()clearPlayerController(\"%s\");", npc.toString().c_str());

	CMessage msgout("R2_NPC_BOT_SCRIPT_BY_ID");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(eid);
	msgout.serial(str);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

void CAiWrapperServer::triggerUserTrigger(const std::string& groupName, uint32 triggerId)
{
/*
	uint32 messageVersion = 1;
	uint32 nbString=2;

	std::string script = NLMISC::toString("()setEvent(%d);", triggerId);


	CMessage msgout("R2_NPC_GROUP_SCRIPT_BY_NAME");
	msgout.serial(messageVersion);
	msgout.serial(nbString);
	msgout.serial(const_cast<std::string&>(groupName));
	msgout.serial(script);

	CUnifiedNetwork::getInstance()->send("AIS",msgout);

*/



	{
		std::string script = NLMISC::toString("\"()setEvent(%d);\"", triggerId);
		CMessage msgout("EXEC_COMMAND");
		std::string command = NLMISC::toString("script %s %s", groupName.c_str(), script.c_str());
		msgout.serial(command);
		CUnifiedNetwork::getInstance()->send("AIS",msgout);
	}

}

void CAiWrapperServer::setPioneerRight(NLMISC::CEntityId entityId, const R2::TPioneerRight& right)
{

	if (right == R2::TPioneerRight::DM)
	{
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("God %s %d", entityId.toString().c_str(), 1);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("Invisible %s %d", entityId.toString().c_str(), 1);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("Aggro %s %d", entityId.toString().c_str(), 0);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
	}
	else  if (right == R2::TPioneerRight::Tester)
	{
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("God %s %d", entityId.toString().c_str(), 0);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("Invisible %s %d", entityId.toString().c_str(), 0);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
		{
			CMessage msgout("EXEC_COMMAND");
			std::string command = NLMISC::toString("Aggro %s %d", entityId.toString().c_str(), 1);
			msgout.serial(command);
			CUnifiedNetwork::getInstance()->send("EGS",msgout);
		}
	}
	else
	{
	}
}

void CAiWrapperServer::askBotDespawnNotification(NLMISC::CEntityId creatureId, TAIAlias alias)
{
	NLNET::CMessage msgout("ASK_BOT_DESPAWN_NOTIFICATION");
	uint32 messageVersion = 1;
	msgout.serial(messageVersion, alias, creatureId);
	CUnifiedNetwork::getInstance()->send("AIS",msgout);
}

