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



// include files
#include "stdpch.h"

#include "npc_description_msg.h"
#include "ai.h"
#include "ai_mgr_pet.h"
#include "ai_share/ai_actions.h"
#include "aids_interface.h"
#include "ai_player.h"
#include "ai_grp_pet.h"
#include "ai_bot_npc.h"
#include "ai_mgr_npc.h"
#include "ai_grp_npc.h"
#include "ai_bot_fauna.h"
#include "ai_mgr_fauna.h"
#include "ai_grp_fauna.h"
#include "ai_profile_fauna.h"	// for CCorpseFaunaProfile
#include "dyn_mission.h"
#include "mirrors.h"


#include "game_share/tick_event_handler.h"
#include "messages.h"
#include "server_share/msg_brick_service.h"
#include "server_share/msg_ai_service.h"
#include "server_share/effect_message.h"
// ai_share
#include "ai_share/ai_actions_dr.h"


#include "egs_interface.h"


#include "dyn_grp_inline.h"

using namespace NLMISC;
using namespace NLNET;
using namespace AITYPES;

//--------------------------------------------------------------------------
// CTransportClass message receivers
//--------------------------------------------------------------------------

class CCharacterBotChatBeginEndReceiver: public CCharacterBotChatBeginEnd
{
	virtual void callback (const std::string &name, NLNET::TServiceId id)
	{
		// make sure bot chat start vector size is even
		if (BotChatStart.size()&1)
		{
			nlwarning("CCharacterBotChatBeginEndReceiver::callback(): Invalid BotChatStart vector length");
			return;
		}

		// make sure bot chat end vector size is even
		if (BotChatEnd.size()&1)
		{
			nlwarning("CCharacterBotChatBeginEndReceiver::callback(): Invalid BotChatEnd vector length");
			return;
		}

//		nlwarning("Cannot initiate Chat between Bot, will be correct in next AI integration");

		// start new bot chats
		{
			for (uint i=0;i<BotChatStart.size();i+=2)
				CAIS::instance().beginBotChat(BotChatStart[i+1],BotChatStart[i]);
		}
		
		// terminate ended bot chats
		{
			for (uint i=0;i<BotChatEnd.size();i+=2)
				CAIS::instance().endBotChat(BotChatEnd[i+1],BotChatEnd[i]);
		}

	}
};

class CCharacterDynChatBeginEndReceiver: public CCharacterDynChatBeginEnd
{
	virtual void callback (const std::string &name, NLNET::TServiceId id)
	{
		// report to the npcs
		for ( uint i=0; i!=DynChatStart.size(); ++i )
			CAIS::instance().beginDynChat( DynChatStart[i]);
		for ( uint i=0; i!=DynChatEnd.size(); ++i )
			CAIS::instance().endDynChat( DynChatEnd[i] );
	}

};


//-------------------------------------------------------------------------
// callback for AIDS seviceUp() event
//--------------------------------------------------------------------------
/*
static void cbAIDSServiceUp(  const std::string &serviceName, uint16 serviceId, void *arg )
{
	CMsgAIServiceUp(100.0f,100.0f).send(uint8(serviceId));
}
*/

//-------------------------------------------------------------------------
// the callback table
//--------------------------------------------------------------------------

static void cbAddEntities( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
static void cbR2GoLive( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
static void cbR2StopLive( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
static void cbR2StartInstance( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbR2NpcBotScriptById( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbR2NpcGroupScriptByName( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
static void cbValidateSourceSpawn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
static void cbValidateSimpleSourceSpawn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostCreateSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostSpawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostDespawnSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostDeleteSquad( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostDespawnAllSquads( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostSetOwner( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostSetAttacker( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostSetState( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostEvent( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbOutpostSetBuildingBotSheet( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventCreateNpcGroup( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventNpcGroupScript( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventFaunaBotSetRadii( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventFaunaBotResetRadii( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventBotCanAggro( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbEventBotSheet( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbAskBotDespawnNotification( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbSpawnEasterEgg( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );
extern void cbDespawnEasterEgg( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );


TUnifiedCallbackItem CbArray[] = 
{
	{	"ADDED_ENTITIES",					cbAddEntities,					},
	{	"SRC_SPWN_VLD",						cbValidateSourceSpawn			},
	{	"S_SRC_SPWN_VLD",					cbValidateSimpleSourceSpawn		},
	{	"R2_GOLIVE",						cbR2GoLive						},
	{	"R2_STOPLIVE",						cbR2StopLive					},
	{	"R2_STARTINSTANCE",					cbR2StartInstance				},
	{	"R2_NPC_BOT_SCRIPT_BY_ID",			cbR2NpcBotScriptById			},
	{	"R2_NPC_GROUP_SCRIPT_BY_NAME",		cbR2NpcGroupScriptByName		},
	{	"OUTPOST_CREATE_SQUAD",				cbOutpostCreateSquad			},
	{	"OUTPOST_SPAWN_SQUAD",				cbOutpostSpawnSquad				},
	{	"OUTPOST_DESPAWN_SQUAD",			cbOutpostDespawnSquad			},
	{	"OUTPOST_DELETE_SQUAD",				cbOutpostDeleteSquad			},
	{	"OUTPOST_DESPAWN_ALL_SQUADS",		cbOutpostDespawnAllSquads		},
	{	"OUTPOST_OWNER",					cbOutpostSetOwner				},
	{	"OUTPOST_ATTACKER",					cbOutpostSetAttacker			},
	{	"OUTPOST_STATE",					cbOutpostSetState				},
	{	"OUTPOST_EVENT",					cbOutpostEvent					},
	{	"OUTPOST_SET_BUILDING_BOT_SHEET",	cbOutpostSetBuildingBotSheet	},
	{	"EVENT_CREATE_NPC_GROUP",			cbEventCreateNpcGroup			},
	{	"EVENT_NPC_GROUP_SCRIPT",			cbEventNpcGroupScript			},
	{	"EVENT_FAUNA_BOT_SET_RADII",		cbEventFaunaBotSetRadii			},
	{	"EVENT_FAUNA_BOT_RESET_RADII",		cbEventFaunaBotResetRadii		},
	{	"EVENT_BOT_CAN_AGGRO",				cbEventBotCanAggro				},
	{	"EVENT_BOT_SHEET",					cbEventBotSheet					},
	{	"ASK_BOT_DESPAWN_NOTIFICATION",		cbAskBotDespawnNotification		},
	{	"SPAWN_EASTER_EGG",					cbSpawnEasterEgg				},
	{	"DESPAWN_EASTER_EGG",				cbDespawnEasterEgg				},
};



void CMessages::release()
{
}

void CMessages::notifyBotDespawn(NLNET::TServiceId serviceId, uint32 botAlias, const NLMISC::CEntityId& botId)
{
	CMessage msgout("BOT_DESPAWN_NOTIFICATION");
	msgout.serial(botAlias);
	msgout.serial(const_cast<NLMISC::CEntityId&>(botId));
	CUnifiedNetwork::getInstance()->send(serviceId, msgout);
}


void CMessages::notifyBotDeath(NLNET::TServiceId serviceId, uint32 botAlias, const NLMISC::CEntityId& botId)
{
	CMessage msgout("BOT_DEATH_NOTIFICATION");
	msgout.serial(botAlias);
	msgout.serial(const_cast<NLMISC::CEntityId&>(botId));
	CUnifiedNetwork::getInstance()->send(serviceId, msgout);
}

void CMessages::notifyBotStopNpcControl(NLNET::TServiceId serviceId, uint32 botAlias, const NLMISC::CEntityId& botId)
{
	CMessage msgout("BOT_STOPCCONTROL_NOTIFICATION");
	msgout.serial(botAlias);
	msgout.serial(const_cast<NLMISC::CEntityId&>(botId));
	CUnifiedNetwork::getInstance()->send(serviceId, msgout);
}

//--------------------------------------------------------------------------
// Messages from the DSS
//--------------------------------------------------------------------------

#include "nel/ligo/primitive.h"
#include "nel/ligo/primitive_utils.h"

#include "ai_share/ai_share.h"
#include "ai_share/ai_actions.h"

#include "game_share/task_list.h"





// Load Scenarip phase 1 (remove old entities)
class CTaskRingGoLive1 : public CTask<uint32>
{
public:
	CTaskRingGoLive1(uint32 aiInstance, bool isBase)
		:_AiInstance(aiInstance), _IsBase(isBase){}

	virtual void doOperation()
	{
		nldebug("Tick %u CTaskRingGoLive1 (Despawn) %u %u", getTime(), _AiInstance, _IsBase);
			std::string actFilename = toString("r2.%04d.act.primitive", _AiInstance);
			std::string baseFilename= toString("r2.%04d.base.primitive", _AiInstance);

			ICommand::execute(toString("createDynamicAIInstance %d", _AiInstance), *InfoLog);
			ICommand::execute(toString("unloadPrimitiveFile \"%s\"", actFilename.c_str()), *InfoLog);

			if (_IsBase)
			{
				ICommand::execute(toString("unloadPrimitiveFile \"%s\"", baseFilename.c_str()), *InfoLog);
			}

	}
private:
	uint32 _AiInstance;
	bool _IsBase;
};




// Load Scenarip phase 2 (insert new entities)
class CTaskRingGoLive2 : public CTask<uint32>
{

public:
	CTaskRingGoLive2(TSessionId sessionId, uint32 aiInstance, bool isBase)
		:_SessionId(sessionId), _AiInstance(aiInstance), _IsBase(isBase){}

	virtual void doOperation()
	{
			
		nldebug("Tick %u, CTaskRingGoLive2 (Spawn) session %u instance %u %u", getTime(), _SessionId.asInt(), _AiInstance, _IsBase);
		CAIActions::IExecutor* executer;
		executer = CAIActions::getExecuter();

		ICommand::execute(toString("createDynamicAIInstance %d", _AiInstance), *InfoLog);

		AI_SHARE::CAIActionsDataRecordPtr dataRec;
		dataRec.init(&Pdr);

		dataRec.applyToExecutor(*executer);
		{			
			std::string cmd = toString("spawnManagers r2.%04d.%s", _SessionId.asInt(), _IsBase?"base":"act");
			ICommand::execute(cmd, *InfoLog);
		}
		{			
			std::string cmd = toString("spawnManagers r2.%04d.%s", _SessionId.asInt(), _IsBase?"base.zone_trigger":"act.zone_trigger");
			ICommand::execute(cmd, *InfoLog);
		}
	}

public:
	CPersistentDataRecord	Pdr;

private:
	uint32 _AiInstance;
	TSessionId _SessionId;
	bool _IsBase;	
};





static void cbR2GoLive( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	
	TSessionId sessionId;
	uint32 aiInstance;
	bool isBase;
	uint32 size;
	
	msgin.serial(sessionId);
	msgin.serial(aiInstance);
	msgin.serial(isBase);
	msgin.serial(size);
	if (size != 0)
	{
		// case where we want to start an instance but (at start of an edition session) but animation not started
	
		uint32 tick = CTimeInterface::gameCycle();
		CTaskRingGoLive1* task1 = new CTaskRingGoLive1( aiInstance, isBase);
		CTaskRingGoLive2* task2 = new CTaskRingGoLive2( sessionId, aiInstance, isBase);
		task2->Pdr.clear();


		uint8* buffer = new uint8[size];
		msgin.serialBuffer(buffer, size);	
		task2->Pdr.fromBuffer((const char*)buffer, size);
		delete [] buffer;
			

		CAIS::instance().addTickedTask(tick + 1,task1);
		CAIS::instance().addTickedTask(tick + 2, task2);

		
	}

	if( isBase )
	{
		// ack to unblock DSS session queue
		CMessage msgout("SESSION_ACK");
		msgout.fastWrite( aiInstance );
		CUnifiedNetwork::getInstance()->send("DSS",msgout);
		nlinfo( "R2An: ack sent to DSS for anim session %u", aiInstance );
	}
	
	return;
}




class CTaskR2StopLive : public CTask<uint32>
{

public:
	CTaskR2StopLive(uint32 aiInstance, bool isBase)
		:_AiInstance(aiInstance), _IsBase(isBase){}

	virtual void doOperation()
	{
		ICommand::execute(toString("createDynamicAIInstance %d", _AiInstance), *InfoLog);

		
		// load a primitiv.binprim from DSS
		//ICommand::execute(toString("despawnManagers %s", isBase?"base":"act"), *InfoLog);
		ICommand::execute(toString("unloadPrimitiveFile \"r2.%04d.act.primitive\"", _AiInstance), *InfoLog);

		//isBase == false <=> stop act
		// isBase == true <=> stop Live
		if (_IsBase)
		{
			ICommand::execute(toString("unloadPrimitiveFile \"r2.%04d.base.primitive\"", _AiInstance), *InfoLog);
			CAIS::instance().destroyAIInstance(_AiInstance, true);
		}

	}


private:
	uint32 _AiInstance;
	bool _IsBase;	
};

static void cbR2StopLive( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{

	bool isBase;
	uint32 aiInstance;
	msgin.serial(aiInstance);
	msgin.serial(isBase);

	uint32 tick = CTimeInterface::gameCycle();
	CTask<uint32>* task1 = new CTaskR2StopLive( aiInstance, isBase);	
	CAIS::instance().addTickedTask(tick ,task1);

}




class CTaskR2StartInstance: public CTask<uint32>
{

public:
	CTaskR2StartInstance(uint32 aiInstance)
		:_AiInstance(aiInstance){}

	virtual void doOperation()
	{
		// destroy the instance before exist
		CAIS::instance().destroyAIInstance(_AiInstance, false);
		ICommand::execute(toString("createDynamicAIInstance %d", _AiInstance), *InfoLog);
	}


private:
	uint32 _AiInstance;
};

static void cbR2StartInstance( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{	
	uint32 aiInstance;
	msgin.serial(aiInstance);	

	uint32 tick = CTimeInterface::gameCycle();
	CTask<uint32>* task1 = new CTaskR2StartInstance( aiInstance);	
	CAIS::instance().addTickedTask(tick ,task1);

	
}

//--------------------------------------------------------------------------
// Messages from the GPMS
//--------------------------------------------------------------------------

//
static void cbAddEntities( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
//	This is where we should deal with player record creation
}


//--------------------------------------------------------------------------
// Messages from the EGS
//--------------------------------------------------------------------------
void cbValidateSourceSpawn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(ValidateSourceSpawn);

	// Read/write header
	CMessage msgout( "SRC_SPWN_VLD_R" );
	uint32 nbSources;
	//CVector prospectingPos;
	//msgin.fastRead( prospectingPos );

	// Get world position of prospector
	const RYAI_MAP_CRUNCH::CWorldMap& worldMap = CWorldContainer::getWorldMap();
	//RYAI_MAP_CRUNCH::CWorldPosition prospectorWorldPos;
	//CAIVector prospectingAiPos( (double)prospectingPos.x, (double)prospectingPos.y );
	//bool isProspectingPosValid = worldMap.setWorldPosition( (double)prospectingPos.z, prospectorWorldPos, prospectingAiPos );

	msgin.fastRead( nbSources );
	msgout.fastWrite( nbSources );

	for ( uint i=0; i!=nbSources; ++i )
	{
		// Read request
		CVector2f pos2f;
		TDataSetRow	sourceDataSetRow;
		msgin.serial( pos2f );
		msgin.serial( sourceDataSetRow );
		msgout.serial( sourceDataSetRow );

		// Check position
		RYAI_MAP_CRUNCH::CWorldPosition wPos;
		CAIVector aiPos( (double)pos2f.x, (double)pos2f.y );
		std::vector<RYAI_MAP_CRUNCH::CWorldPosition> wPosList;

		bool canSpawnHere = worldMap.setWorldPosition( AITYPES::vp_auto, wPos, aiPos );

		//worldMap.getWorldPositions( wPosList, aiPos );
		//for ( std::vector<RYAI_MAP_CRUNCH::CWorldPosition>::const_iterator it=wPosList.begin(); it!=wPosList.end(); ++it )
		{
			// Discard if the position is in water or in interiors
			if ( canSpawnHere && (wPos.getFlags() & (RYAI_MAP_CRUNCH::Water | RYAI_MAP_CRUNCH::Interior)) )
				canSpawnHere = false;
				//continue;
	
			msgout.fastWrite( canSpawnHere );

			// TODO: Browse the possible surfaces for the position (x,y), then keep only the one(s)
			// for which a pathfinding from the prospector to the source limited to 5 surfaces succeeds
		}
	}

	// Send reply
	sendMessageViaMirror( serviceId, msgout ); // via mirror because receiver will access mirror
	//nldebug( "Processed %u forage source pos validations", nbSources );
}


void cbValidateSimpleSourceSpawn( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	H_AUTO(ValidateSourceSpawn);

	// Read/write header
	CMessage msgout( "SRC_SPWN_VLD_R" );

	// Read request
	uint32 nbSources = 1;
	msgout.fastWrite( nbSources );
	CVector2f pos2f;
	TDataSetRow	sourceDataSetRow;
	msgin.serial( pos2f );
	msgin.serial( sourceDataSetRow );
	msgout.serial( sourceDataSetRow );

	// Check position
	const RYAI_MAP_CRUNCH::CWorldMap& worldMap = CWorldContainer::getWorldMap();
	RYAI_MAP_CRUNCH::CWorldPosition wPos;
	CAIVector aiPos( (double)pos2f.x, (double)pos2f.y );
	bool canSpawnHere = worldMap.setWorldPosition( AITYPES::vp_auto, wPos, aiPos );

	// Discard if the position is in water or in interiors
	if ( canSpawnHere && (wPos.getFlags() & (RYAI_MAP_CRUNCH::Water | RYAI_MAP_CRUNCH::Interior)) )
		canSpawnHere = false;

	msgout.fastWrite( canSpawnHere );

	// Send reply
	sendMessageViaMirror( serviceId, msgout ); // via mirror because receiver will access mirror
	//nldebug( "Processed a simple forage source pos validation" );

}


//--------------------------------------------------------------------------
// CTransportClass callbacks
//--------------------------------------------------------------------------
/*
void CMsgAIOpenMgrs::callback (const std::string &serviceName, uint8 sid)
{
	if (Name.size()!=Type.size() || Name.size()!=MgrId.size())
	{
		nlwarning("Size missmatch in vectors in CMsgAIOpenMgrs message - message ignored");
		return;
	}

	// iterate through the entries in the message opening new managers
	for (uint i=0;i<Name.size();++i)
	{
		// look for a type that matches the type name given
		EMgrType type=mgrType(Type[i].c_str());

		if (type!=MgrTypeBadType)
		{
			// open the manger as requested
			CAIS::newMgr(type,MgrId[i],Name[i],std::string());
		}
		else
		{
			// failed to identify type so give up
			nlwarning("Failed to translate '%s' into a known type - request to open manager %04d (%s) ignored",
				Type[i].c_str(),MgrId[i],Name[i].c_str());
			std::string msgStr("Failed to open "+Name[i]+" due to bad type: '"+Type[i]+"'"); 
			CMsgAIFeedback(msgStr).send("AIDS");
		}
	}
}
*/

class CAITauntImp : public CAITauntMsg
{
public:
	void	callback (const std::string &name, NLNET::TServiceId id);
protected:
private:
};

class	CAIPlayerRespawnMsgImp : public CAIPlayerRespawnMsg
{
public:
	virtual void callback (const std::string &name, NLNET::TServiceId id);
};

class	CAIAskForInfosOnEntityImp
		:public	CAIAskForInfosOnEntityMsg
{
public:
	void callback (const std::string &name, NLNET::TServiceId id);
};


class CEnableAggroOnPlayerImp
	:public	CEnableAggroOnPlayerMsg
{
public:
	void callback (const std::string &name, NLNET::TServiceId id);
};

class CSetBotHeadingImp : public CSetBotHeadingMsg
{
public:
	void callback(const std::string &name, NLNET::TServiceId id);
};

class CChangeActionFlagMsgImp
:public	CChangeActionFlagMsg
{
public:
	void callback (const std::string &name, NLNET::TServiceId id);
};

class CChangeCreatureModeMsgImp
:public	CChangeCreatureModeMsg
{
public:
	void callback (const std::string &name, NLNET::TServiceId id);
};

//-------------------------------------------------------------------------
// singleton initialisation and release
//--------------------------------------------------------------------------

void CMessages::init()
{
	// incoming from AIDS
	TRANSPORT_CLASS_REGISTER( CMsgAICloseMgrs );
	TRANSPORT_CLASS_REGISTER( CMsgAIBackupMgrs );
	TRANSPORT_CLASS_REGISTER( CMsgAIUploadActions );
	TRANSPORT_CLASS_REGISTER( CMsgAISpawnMgrs );
	TRANSPORT_CLASS_REGISTER( CMsgAIDespawnMgrs	);

	// outgoing to AIDS
	TRANSPORT_CLASS_REGISTER( CMsgAIServiceUp );
	TRANSPORT_CLASS_REGISTER( CMsgAIManagerUp );
	TRANSPORT_CLASS_REGISTER( CMsgAIFeedback );

	// Classes used for comms with EGS (outgoing)
//	TRANSPORT_CLASS_REGISTER( CNpcBotDescription );
	TRANSPORT_CLASS_REGISTER( CCharacterBotChatBeginEndReceiver );
	TRANSPORT_CLASS_REGISTER( CCharacterDynChatBeginEndReceiver );
	TRANSPORT_CLASS_REGISTER( CFaunaBotDescription );
	TRANSPORT_CLASS_REGISTER( CUserEventMsgImp );
	TRANSPORT_CLASS_REGISTER( CSetEscortTeamIdImp );
	TRANSPORT_CLASS_REGISTER( CCreatureDespawnImp );
	TRANSPORT_CLASS_REGISTER( CDelHandledAIGroupImp );
	TRANSPORT_CLASS_REGISTER( CAddHandledAIGroupImp );
	TRANSPORT_CLASS_REGISTER( CHandledAIGroupSpawnedMsg );
	TRANSPORT_CLASS_REGISTER( CHandledAIGroupDespawnedMsg );
	TRANSPORT_CLASS_REGISTER( CGiveItemRequestMsg );
	TRANSPORT_CLASS_REGISTER( CReceiveItemRequestMsg );
	
	//	Classes used for comms between AI and EGS
	TRANSPORT_CLASS_REGISTER (CPetSpawnMsgImp);
	TRANSPORT_CLASS_REGISTER (CPetSpawnConfirmationMsg);

	TRANSPORT_CLASS_REGISTER (CPetCommandMsgImp);
	TRANSPORT_CLASS_REGISTER (CPetCommandConfirmationMsg);
	
	TRANSPORT_CLASS_REGISTER (CBSAIDeathReport);
	TRANSPORT_CLASS_REGISTER (CCAisActionMsg);
	
	TRANSPORT_CLASS_REGISTER (CEGSExecutePhraseMsg);
	TRANSPORT_CLASS_REGISTER (CEGSExecuteAiActionMsg);

	TRANSPORT_CLASS_REGISTER (CAddEffectsMessage);
	TRANSPORT_CLASS_REGISTER (CRemoveEffectsMessage);

//	TRANSPORT_CLASS_REGISTER (COutpostList);
//	TRANSPORT_CLASS_REGISTER (COutpostDescription);
	TRANSPORT_CLASS_REGISTER (CDutyDescription);
//	TRANSPORT_CLASS_REGISTER (COutpostStateImp);
//	TRANSPORT_CLASS_REGISTER (COutpostWarEnd);

	TRANSPORT_CLASS_REGISTER (CAITauntImp);

	TRANSPORT_CLASS_REGISTER (CAILostAggroMsg);
	TRANSPORT_CLASS_REGISTER (CAIGainAggroMsg);

	TRANSPORT_CLASS_REGISTER (CAIPlayerRespawnMsgImp);

	TRANSPORT_CLASS_REGISTER (CAIAskForInfosOnEntityImp);
	TRANSPORT_CLASS_REGISTER (CAIInfosOnEntityMsg);
	TRANSPORT_CLASS_REGISTER (CEnableAggroOnPlayerImp);
	TRANSPORT_CLASS_REGISTER (CChangeActionFlagMsgImp);

	TRANSPORT_CLASS_REGISTER (CPetSetOwnerImp);

	TRANSPORT_CLASS_REGISTER (CSetBotHeadingImp);

	// messages related to instance management
	TRANSPORT_CLASS_REGISTER (CReportAICollisionAvailableMsg);
	TRANSPORT_CLASS_REGISTER (CReportStaticAIInstanceMsg);
	TRANSPORT_CLASS_REGISTER (CReportAIInstanceDespawnMsg);
	TRANSPORT_CLASS_REGISTER (CWarnBadInstanceMsgImp);
	TRANSPORT_CLASS_REGISTER (CCreatureSetUrlMsg);
	TRANSPORT_CLASS_REGISTER (CChangeCreatureMaxHPMsg)
	TRANSPORT_CLASS_REGISTER (CChangeCreatureHPMsg);
	TRANSPORT_CLASS_REGISTER (CChangeCreatureModeMsgImp);
	TRANSPORT_CLASS_REGISTER (CQueryEgs);
	
	// setup the callback array
	CUnifiedNetwork::getInstance()->addCallbackArray( CbArray, sizeof(CbArray)/sizeof(CbArray[0]) );

	// hook up callback to service up of data service
//	CUnifiedNetwork::getInstance()->setServiceUpCallback( std::string("AIDS"), cbAIDSServiceUp, 0);
}



//////////////////////////////////////////////////////////////////////////
//	CallBack part
//////////////////////////////////////////////////////////////////////////

void	CEnableAggroOnPlayerImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIEntityPhysical	*phys=CAIS::instance().getEntityPhysical(EntityRowId);
	if	(!phys)
	{
		// When go from test to edition the ai instance is destroyed
		if (!IsRingShard)
		{
			nlwarning("CEnableAggroOnPlayerImp::callback: unknow entity %s on this AIS !", EntityRowId.toString().c_str());
		}
		
		return;
	}

	switch(phys->getRyzomType())
	{
	case RYZOMID::player:
		{
			CBotPlayer *const bot=safe_cast<CBotPlayer*>(phys);
			bot->setAggroable(EnableAggro);
			if (!EnableAggro)
				bot->forgotAggroForAggroer();
		}
		break;
	default:
		nlwarning("CEnableAggroOnPlayerImp::callback: non player entity ! %s(%u) on this AIS !", phys->getEntityId().toString().c_str(), EntityRowId.toString().c_str());
		break;
	}

}


void	CAIAskForInfosOnEntityImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIInfosOnEntityMsg	msg;
	msg.AskerRowID=AskerRowID;
	msg.EntityRowId=EntityRowId;

	CArrayStringWriter	stringWriter(msg.Infos);

	CAIEntityPhysical	*phys=CAIS::instance().getEntityPhysical(EntityRowId);
	if	(phys)
	{
		switch(phys->getRyzomType())
		{
		case RYZOMID::creature:
		case RYZOMID::npc:
		case RYZOMID::pack_animal:
			{
				const CBot *const bot=safe_cast<const CBot*>(&phys->getPersistent());
				std::vector<std::string> strings = bot->getMultiLineInfoString();
				msg.Infos.insert(msg.Infos.end(), strings.begin(), strings.end());
			}
			break;
		default:
			std::vector<std::string> strings = phys->getMultiLineInfoString();
			msg.Infos.insert(msg.Infos.end(), strings.begin(), strings.end());
			break;
		}
	}
	else
	{
		nlwarning("CAIAskForInfosOnEntityImp::callback: entity %s is unknow on this AIS.", EntityRowId.toString().c_str());
		stringWriter.append(toString("%s : unknow entity index %s", IService::getInstance()->getServiceShortName().c_str(), EntityRowId.toString().c_str()));
	}
	msg.send("EGS");
}

void	CChangeActionFlagMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	const uint32 size = (uint32)Entities.size();
	nlassert( size == ActionFlags.size() && size == Values.size());
	
	for (uint32 i = 0 ; i < size ; ++i)
	{
		CAIEntityPhysical	*const	phys=CAIS::instance().getEntityPhysical(Entities[i]);
		if	(!phys)
			continue;

		if	(Values[i])
			phys->setActionFlags((RYZOMACTIONFLAGS::TActionFlag)ActionFlags[i]);
		else
			phys->removeActionFlags((RYZOMACTIONFLAGS::TActionFlag)ActionFlags[i]);
	}
}

void	CChangeCreatureModeMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIEntityPhysical	*phys=CAIS::instance().getEntityPhysical(CreatureId);
	if	(!phys)
		return;

	static_cast<CModEntityPhysical*>(phys)->setMode( MBEHAV::EMode(NewMode) );
}

//
////----------------------------------------------------------------
//// EGS -> AIS ask infos on entity
////----------------------------------------------------------------
//class CAIAskForInfosOnEntityMsg : public NLNET::CTransportClass
//{
//public:
//	TDataSetRow		EntityRowId;
//	
//	virtual void description ()
//	{
//		className ("CAIAskForInfosOnEntityMsg");
//		property ("EntityRowId", PropDataSetRow, TDataSetRow(), EntityRowId);
//	}
//	virtual void callback (const std::string &name, uint8 id) {}
//};
//
////----------------------------------------------------------------
//// AIS -> EGS returns infos on entity as a vector of strings
////----------------------------------------------------------------
//class CAIInfosOnEntityMsg : public NLNET::CTransportClass
//{
//public:
//	TDataSetRow					EntityRowId;
//	std::vector<std::string>	Infos;
//	
//	virtual void description ()
//	{
//		className ("CAIInfosOnEntityMsg");
//		property ("EntityRowId", PropDataSetRow, TDataSetRow(), EntityRowId);
//		propertyCont ("Infos", PropString, Infos);
//	}
//	virtual void callback (const std::string &name, uint8 id) {}
//};


void CMsgAIUploadActions::callback(const std::string &serviceName, NLNET::TServiceId sid)
{
	for(uint i=0;i<Data.size();)
	{
		uint argCount;
		char action[9];
		std::vector<CAIActions::CArg> args;

		if (Data.size()-i<sizeof(char)+sizeof(uint64))
		{
			nlwarning("Unexpected end of input data in CMsgAIUploadActions message");
			CAIDSInterface::warning("Unexpected end of input data in CMsgAIUploadActions()");
			return;
		}
		argCount=Data[i]; i++;
		uint k;
		for (k=0;k<8;++i,++k)
			action[k]=Data[i];
		action[k]=0;
		for (uint j=0;j<argCount;++j)
		{
			args.push_back(CAIActions::CArg::serialFromString(Data,i));
			if	(i<=Data.size())
				continue;

			nlwarning("Unexpected end of input data in CMsgAIUploadActions message");
			CAIDSInterface::warning("Unexpected end of input data in CMsgAIUploadActions()");
			return;
		}
		CAIActions::execute(action,args);
	}
}

void CMsgAISpawnMgrs::callback(const std::string &serviceName, NLNET::TServiceId sid)
{
	nlwarning("*** CMsgAISpawnMgrs message received from AIDS but not treated ***");
	CMsgAIFeedback("*** CMsgAISpawnMgrs message received from AIDS but not treated ***").send("AIDS");
}

void CMsgAIDespawnMgrs::callback(const std::string &serviceName, NLNET::TServiceId sid)
{
	nlwarning("*** CMsgAIDespawnMgrs message received from AIDS but not treated ***");
	CMsgAIFeedback("*** CMsgAIDespawnMgrs message received from AIDS but not treated ***").send("AIDS");
}

void CMsgAICloseMgrs::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	nlassert(false);	//	AIDataService.
//	for (uint i=0;i<MgrId.size();++i)
//		CAIS::instance().AIList()[0]->deleteMgr(MgrId[i]);
}

void CMsgAIBackupMgrs::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
	nlassert(false);	//	AIDataService.

	nlwarning("*** Backup message received from AIDS but not treated ***");
	CMsgAIFeedback("*** Backup message received from AIDS but not treated ***").send("AIDS");
}


//--------------------------------------------------------------------------
// CTransportClass stub callbacks for outgoing messages
//--------------------------------------------------------------------------

void CMsgAIServiceUp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
}

void CMsgAIManagerUp::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
}

void CMsgAIFeedback::callback (const std::string &serviceName, NLNET::TServiceId sid)
{
}


void CBSAIDeathReport::callback(const std::string &name, NLNET::TServiceId id)
{
	for (uint i=0; i<Bots.size(); ++i)
	{
		CAIEntityPhysical* phys=CAIS::instance().getEntityPhysical(Bots[i]);
		if	(!phys)
		{
			if	(Zombies[i])
			{
				nldebug("CBSAIDeathReport::callback: ZombyUnKnown entity %s not handled here", Bots[i].toString().c_str());
			}
			// removed because of multi AIS
			//nldebug("CBSAIDeathReport::callback: entity %s not handled here", Bots[i].toString().c_str());
			continue;
		}

#if !FINAL_VERSION
		nlassert(phys->getRyzomType()!=RYZOMID::player);
#else
		if	(phys->getRyzomType()==RYZOMID::player)
		{
			if	(Zombies[i])
			{
				nldebug("CBSAIDeathReport::callback: ZombyPlayer entity %s !!", Bots[i].toString().c_str());
			}
			continue;
		}
#endif
		static_cast<CModEntityPhysical*>(phys)->setMode(MBEHAV::DEATH);
		if	(Zombies[i])
		{
			if	(dynamic_cast<CModEntityPhysical*>(phys)==NULL)
			{
				nldebug("CBSAIDeathReport::callback: ZombyCastError entity %s !!", Bots[i].toString().c_str());
			}
			else
			{
				nldebug("CBSAIDeathReport::callback: ZombyDEATHSet entity %s !!", Bots[i].toString().c_str());
			}

		}
		
		switch(phys->getRyzomType())
		{
		case RYZOMID::npc:
			{
				const	CSpawnBotNpc	*const	sb	=	static_cast<CSpawnBotNpc*>(phys);
						CSpawnGroupNpc	&spawnGrp	=	sb->spawnGrp();
				const	CMgrNpc			&mgr		=	spawnGrp.getPersistent().mgr();

				spawnGrp.addBotToDespawnAndRespawnTime	(&(sb->getPersistent()), spawnGrp.getPersistent().despawnTime(), spawnGrp.getPersistent().respawnTime());
				
				if (spawnGrp.getPersistent().getSquadLeader(false)==&sb->getPersistent())
					spawnGrp.getPersistent().processStateEvent(mgr.EventSquadLeaderKilled);
				

				CBotNpc* bot = NLMISC::safe_cast<CBotNpc*>(&(sb->getPersistent()));
				spawnGrp.botHaveDied	(bot);
				bot->notifyBotDeath();
				
				
				spawnGrp.getPersistent().processStateEvent(mgr.EventBotKilled);

				if	(!spawnGrp.isGroupAlive(0*1))
					spawnGrp.getPersistent().processStateEvent(mgr.EventGrpEliminated);
			}
			break;
		case RYZOMID::creature:
			{
				CSpawnBotFauna		*const	sb	=	static_cast<CSpawnBotFauna*>(phys);
				CSpawnGroupFauna	&spawnGrp	=	sb->spawnGrp();
				CMgrFauna			&mgr		=	spawnGrp.getPersistent().mgr();

				sb->setAIProfile(new	CCorpseFaunaProfile(sb));
				{
					const	sint32	DespawnTime=spawnGrp.getPersistent().timer(CGrpFauna::CORPSE_TIME);
					const	sint32	RespawnTime=spawnGrp.getPersistent().timer(CGrpFauna::RESPAWN_TIME);
					//	set the _Bot to be despawn and gives the control to its group.
					// despawn the corpse after DespawnTime, and hence respawn after DespawnTime+RespawnTime since now.
					spawnGrp.addBotToDespawnAndRespawnTime(&sb->getPersistent(),(uint32) DespawnTime, (uint32) DespawnTime + (uint32) RespawnTime);
				}

				spawnGrp.getPersistent().processStateEvent(mgr.EventBotKilled);
				if	(spawnGrp.isGroupAlive())
					break;

				spawnGrp.getPersistent().processStateEvent(mgr.EventGrpEliminated);
			}
			break;
		default:
			break;
		}

	}

}


void CEGSExecutePhraseMsg::callback(const std::string &name, NLNET::TServiceId id)
{
}


void	CAITauntImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIEntityPhysical	*entity=CAIS::instance().getEntityPhysical(TargetRowId);

	if	(!entity)
		return;

	CBotAggroOwner	*aggroOwner=NULL;

	switch	(entity->getRyzomType())
	{
	case RYZOMID::creature:
	case RYZOMID::npc:
		aggroOwner=NLMISC::safe_cast<CSpawnBot*>(entity);
		break;
	default:
		break;
	}

	if (!aggroOwner)
		return;
	
	aggroOwner->maximizeAggroFor(PlayerRowId);
}

void	CAIPlayerRespawnMsgImp::callback (const std::string &name, NLNET::TServiceId id)
{
	CAIEntityPhysical	*entity=CAIS::instance().getEntityPhysical(PlayerRowId);
	if	(!entity)
		return;

	CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entity);
	if	(!player)
		return;

	player->forgotAggroForAggroer();
}



void	sAggroLost(TDataSetRow playerBot, TDataSetRow targetBot)
{
	CAIEntityPhysical	*entity=CAIS::instance().getEntityPhysical(playerBot);	//	necessary ?
	if	(	!entity
		||	CMirrors::getEntityId(playerBot).getType()!=RYZOMID::player)
		return;

	CAILostAggroMsg	msg(targetBot, playerBot);
	msg.send("EGS");

	CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entity);
	if	(!player)
		return;
	player->removeAggroer(targetBot);
}


void	sAggroGain(TDataSetRow playerBot, TDataSetRow targetBot)
{
	CAIEntityPhysical	*entity=CAIS::instance().getEntityPhysical(playerBot);	//	necessary ?
	if	(	!entity
		||	CMirrors::getEntityId(playerBot).getType()!=RYZOMID::player)
		return;

	CAIGainAggroMsg	msg(targetBot, playerBot);
	msg.send("EGS");

	CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entity);
	if	(!player)
		return;
	player->addAggroer(targetBot);
}


void CSetBotHeadingImp::callback(const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CAIEntityPhysical *ep =CAIS::instance().getEntityPhysical(BotRowId);

	if (ep == NULL)
	{
		nlwarning("CSetBotHeadingImp : invalid bot id : %s (%u)", TheDataset.getEntityId(BotRowId).toString().c_str(), BotRowId.getIndex());
		return;
	}

	// ok, we have an entity physical, try to make it a npc
	CSpawnBotNpc *sb = dynamic_cast<CSpawnBotNpc*>(ep);

	if (sb == NULL)
	{
		nlwarning("CSetBotHeadingImp : bot id %s (%u) is not an NPC !", TheDataset.getEntityId(BotRowId).toString().c_str(), BotRowId.getIndex());
		return;
	}

	// set the theta
	sb->setTheta(CAngle(double(Heading)));
}


void CWarnBadInstanceMsgImp::callback(const std::string &serviceName, NLNET::TServiceId serviceId)
{
	CAIS::instance().warnBadInstanceMsgImp(serviceName, serviceId, *this);
}

#include "event_reaction_include.h"
