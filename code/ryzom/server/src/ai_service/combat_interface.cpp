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
// Game share
#include "server_share/msg_brick_service.h"
#include "game_share/tick_event_handler.h"
#include "server_share/effect_message.h"

#include "combat_interface.h"
#include "ai.h"
#include "ai_entity_physical.h"
#include "ai_entity_physical_inline.h"
#include "ai_instance.h"
#include "ai_mgr.h"
#include "ai_profile_npc.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using	namespace	EFFECT_FAMILIES;

// GLOBALS
std::list <CCombatInterface::CEvent> CCombatInterface::_events;

static bool verboseLog=false;


// MACROS
#define LOG if (!verboseLog) {} else nlinfo

static void cbServiceUpEGS( const string& serviceName, NLNET::TServiceId serviceId, void * )
{
	LOG("Combat Interface: EGS service mirror up");

	// ask the brick service to give me event reports
	CMessage msgRegister("REGISTER_AI_EVENT_REPORTS");
	sendMessageViaMirror ("EGS", msgRegister);
}

void CCombatInterface::init()
{
	LOG("Combat Interface: init()");

	// register the incoming transport classes
	TRANSPORT_CLASS_REGISTER(CBSAIEventReportMsg);
	TRANSPORT_CLASS_REGISTER(CEGSExecuteMsg);

	CSheetId::init();

	// setup service up callbacks
	CMirrors::Mirror.setServiceMirrorUpCallback( "EGS", cbServiceUpEGS, 0);
}

void CCombatInterface::release()
{
	LOG("Combat Interface: release()");
}


void CBSAIEventReportMsg::callback(const std::string &name, NLNET::TServiceId id) 
{
	CCombatInterface::CEvent	event;
	
	for (uint i=0;i<Originator.size();i++)
	{
		uint8	actionType=ActionType[i];
		if (	(	actionType!=ACTNATURE::FIGHT
				||	actionType!=ACTNATURE::OFFENSIVE_MAGIC
				||	actionType!=ACTNATURE::CURATIVE_MAGIC	)
			&&	AggroAdd[i]==0	)
			continue; 

		event._originatorRow	=	Originator[i];
		event._targetRow		=	Target[i];
		event._weight			=	AggroAdd[i];
		clamp(event._weight,-1,+1);
		event._nature			=	(ACTNATURE::TActionNature)actionType;

		CCombatInterface::_events.push_back(event);
	}

}

NLMISC_COMMAND(verboseCombatLog,"Turn on or off or check the state of verbose combat logging","")
{
	if(args.size()>1)
		return false;

	if(args.size()==1)
		StrToBool	(verboseLog, args[0]);

	log.displayNL("verboseCombatLogging is %s",verboseLog?"ON":"OFF");
	return true;
}

//-----------------------------------------------
// cbChangeMode :
//-----------------------------------------------

//void cbChangeMode( CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
//{
//	TDataSetRow row;
//	msgin.serial( row );
//
//	// read the new Mode
//	MBEHAV::TMode mode;
//	msgin.serial( mode );
//
//	CAIEntityPhysical	*phys=CAIS::getEntityPhysical(row);
//
//	if (	phys
//		&&	phys->getRyzomType()!=RYZOMID::player
//		&&	phys->isAlive()	)
//	{
//		static_cast<CModEntityPhysical*>(phys)->setModeStruct(mode);
//	}
//
//} // cbChangeMode //

void	CAddEffectsMessage::callback	(const std::string &name, NLNET::TServiceId id)
{
	for (uint32	i=0;i<Entities.size();i++)
	{
		CAIEntityPhysical	*phys=CAIS::instance().getEntityPhysical(Entities[i]);
		if	(!phys)
			continue;
		
		switch	((TEffectFamily)Families[i])
		{
		case Stun:
		case CombatStun:
		case Mezz:
			phys->stun()++;
			break;

		case Root:
			phys->root()++;
			break;

		case Blind:
			phys->blind()++;
			break;

		case Fear:
			phys->fear()++;
			break;

		default:
			break;
		}
		
	}
	
}

void	CRemoveEffectsMessage::callback	(const std::string &name, NLNET::TServiceId id)
{
	for (uint32	i=0;i<Entities.size();i++)
	{
		CAIEntityPhysical	*phys=CAIS::instance().getEntityPhysical(Entities[i]);
		if	(!phys)
			continue;
		
		switch	((TEffectFamily)Families[i])
		{
		case Stun:
		case CombatStun:
		case Mezz:
			phys->stun()--;
			if(!phys->isStuned())
			{
				CSpawnBot * spawnBot = dynamic_cast<CSpawnBot*>(phys);
				if(spawnBot)
				{
					spawnBot->updateProfile(10);
				}
			}
			break;
		
		case Root:
			phys->root()--;
			if(!phys->isRooted())
			{
				CSpawnBot * spawnBot = dynamic_cast<CSpawnBot*>(phys);
				if(spawnBot)
				{
					spawnBot->updateProfile(10);
				}
			}
			break;
		
		case Blind:
			phys->blind()--;
			break;
		
		case Fear:
			phys->fear()--;
			break;
		
		default:
			break;
		}
		
	}
	
}
