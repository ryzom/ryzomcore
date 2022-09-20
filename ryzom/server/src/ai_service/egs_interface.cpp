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



#include "stdpch.h"
#include "egs_interface.h"
#include "ai.h"
#include "ai_mgr.h"
#include "server_share/msg_ai_service.h"
#include "ai_bot_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "nel/misc/path.h"

extern bool simulateBug(int bugId);

using namespace NLMISC;
using namespace NLNET;
using namespace std;


// call back for synchronize time and day
static void cbKamiImpact( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	NLMISC::CEntityId charId;
	uint32 depositId;
	uint32 score;

	// Send message to AIS for kami impact
	msgin.serial( charId );
	msgin.serial( depositId );
	msgin.serial( score );
}

// classic init(), update() and release()
void CEGSInterface::init()
{
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "KAMI_IMPACT",		cbKamiImpact	},
	};
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );
}

void CEGSInterface::update()
{
	H_AUTO(EGSUpdate);
	// flush output messages
}

void CEGSInterface::release()
{
}

void CUserEventMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIInstance *aii = CAIS::instance().getAIInstance(InstanceNumber);
	if (aii != NULL)
	{
		CGroup *grp = aii->findGroup(GrpAlias);	
		if (grp)
		{
			// ok, we found the correct group!
			grp->setEventParams(Params);
			grp->setEvent(EventId);
			grp->setEventParams(vector<string>());
			// nothing more to do
			return;
		}
	}

	nlwarning("CUserEventMsgImp event %u for group alias %u : no such group in instance %u !", EventId, GrpAlias, InstanceNumber);
}


void CSetEscortTeamIdImp::callback (const std::string &name, NLNET::TServiceId id)
{
	// for each alias, retrieve the group and sets the escort team
	for (uint i=0; i<Groups.size(); ++i)
	{
		CAIInstance *aii = CAIS::instance().getAIInstance(InstanceNumber);
		if (aii)
		{
			CGroup	*grp = aii->findGroup(Groups[i]);	//	Fake, to remove when EGS can tell us the corresponding world number.
			if (grp)
			{
				grp->setEscortTeamId(TeamId);
			}
			else
				nlwarning("CSetEscortTeamIdImp : can't find group with alias %u", Groups[i]);
		}
		else
		{
			nlwarning("CSetEscortTeamIdImp::callback : can't find AIInstance %u", InstanceNumber);
		}

	}
}


//////////////////////////////////////////////////////////////////////////
//	Loot Finished or no loot, EGS ask to remove the creature.

void CCreatureDespawnImp::callback (const std::string &name, NLNET::TServiceId id)
{
	for (uint i = 0; i < Entities.size(); i++)
	{
		CAIEntityPhysical*const	ep = CAIS::instance().getEntityPhysical(Entities[i]);
		if	(!ep)
			continue;	//	not spawn
	
		switch(ep->getRyzomType())
		{
		case RYZOMID::npc:
			// ok, warn the grp that this bot is totally looted, so we can despawn it
			{
				CSpawnBotNpc *botNpc = static_cast<CSpawnBotNpc *>(ep);
				botNpc->spawnGrp().addBotToDespawnAndRespawnTime(&botNpc->getPersistent(), 1, botNpc->spawnGrp().getPersistent().respawnTime());	
			}
			break;
		case RYZOMID::creature:	//	Fauna.
			{
				CSpawnBotFauna	*botFauna = static_cast<CSpawnBotFauna*>(ep);

				if (simulateBug(6))
				{
					botFauna->spawnGrp().addBotToDespawnAndRespawnTime(&botFauna->getPersistent(),1,2);	//	1 second.					
				}
				else
				{
					// set time before respawn
					sint32	RespawnTime= botFauna->spawnGrp().getPersistent().timer(CGrpFauna::RESPAWN_TIME);
					botFauna->spawnGrp().addBotToDespawnAndRespawnTime(&botFauna->getPersistent(), 1, RespawnTime);	//	1 tick before despawn
				}
			}
			break;
		case RYZOMID::pack_animal:	//	Pet.
			{
				CSpawnBotPet	*spawnBotPet = static_cast<CSpawnBotPet*>(ep);
				spawnBotPet->getPersistent().despawnBot(); // direct despawn.
			}
			break;
			
		default:
			nlwarning("CCreatureDespawnImp Entity %d is not processed", Entities[i].getIndex());
			break;
		}
	}	
}

//////////////////////////////////////////////////////////////////////////

void CAddHandledAIGroupImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CCont<CAIInstance> &instances = CAIS::instance().AIList();
	
	for (uint i=0; i<instances.size(); ++i)
	{
		CAIInstance *aii = instances[i];
		if (!aii)
			continue;
		
		CGroupNpc *grp = dynamic_cast<CGroupNpc*>(aii->findGroup(GroupAlias));	
		if (grp != NULL)
		{
			nlinfo("CAddHandledAIGroupImp on group %u user:%s miss:%d", GroupAlias, PlayerRowId.toString().c_str(), MissionAlias);
			// ok, we found the correct group!
			grp->addHandle(PlayerRowId, MissionAlias, DespawnTimeInTick);
			return;
		}
	}
	
	// debug only : AIS can receive message for not the same AiInstance as the group
	//nlwarning("CAddHandledAIGroupImp group alias %u : not found !", GroupAlias);
}

//////////////////////////////////////////////////////////////////////////

void CDelHandledAIGroupImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CCont<CAIInstance> &instances = CAIS::instance().AIList();
	
	for (uint i=0; i<instances.size(); ++i)
	{
		CAIInstance *aii = instances[i];
		if (!aii)
			continue;
		
		CGroupNpc *grp = dynamic_cast<CGroupNpc*>(aii->findGroup(GroupAlias));	
		if (grp != NULL)
		{
			nlinfo("CDelHandledAIGroupImp on group %u user:%s miss:%d", GroupAlias, PlayerRowId.toString().c_str(), MissionAlias);
			// ok, we found the correct group!
			grp->delHandle(PlayerRowId, MissionAlias);
			return;
		}
	}
	// debug only : AIS can receive message for not the same AiInstance as the group
	//nlwarning("CDelHandledAIGroupImp group alias %u : no found !", GroupAlias);
}

