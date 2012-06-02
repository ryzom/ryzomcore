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

#include "mirrors.h"
#include "game_share/tick_event_handler.h"
#include "nel/net/service.h"
#include "game_share/misc_const.h"
#include "game_share/fame.h"
#include "ai_bot_npc.h"
#include "ai_grp_npc.h"
#include "server_share/r2_variables.h"
#include "server_share/r2_vision.h"


#include "ai.h"
#include "ai_player.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;
using namespace MBEHAV;

CMirror				CMirrors::Mirror;
CMirroredDataSet	*CMirrors::DataSet = NULL;
CMirroredDataSet	*CMirrors::FameDataSet = NULL;

const uint			MAX_NB_ENTITIES_ISOLATED = 25000;
CEntityId			*IsolatedEntityId = NULL;
sint32				*IsolatedX = NULL;
sint32				*IsolatedY = NULL;
sint32				*IsolatedZ = NULL;
float				*IsolatedTheta = NULL;
uint32				*IsolatedSheet = NULL;
uint32				*IsolatedSheetServer = NULL;
TDataSetRow			*IsolatedTarget = NULL;	//	sint32
uint32				*IsolatedMode = NULL; // only mode enum
uint32				*IsolatedBehaviour = NULL;
uint32				*IsolatedCurrentHitPoints = NULL;
uint32				*IsolatedMaxHitPoints = NULL;
uint32				*IsolatedBestRoleLevel = NULL;
uint8				*IsolatedCombatState = NULL;

extern CVariable<double>	RingMaxSelectDist; 

/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	CMirrors::initMirror();
}

void cbMirrorReadyForAddEntity( CMirror *mirror )
{

	// Init fx manager
	const TDeclaredEntityRangeOfType& declERT = TheDataset.getDeclaredEntityRanges();
	{
		TDeclaredEntityRangeOfType::const_iterator it = declERT.find( RYZOMID::fx_entity );
		const TDeclaredEntityRange& range = GET_ENTITY_TYPE_RANGE(it);
		CFxEntityManager::getInstance()->init( range.baseIndex(), range.size() );
	}
}


#define initIsolatedPropTable( name, thetype, defaultvalue ) \
	name = new thetype [MAX_NB_ENTITIES_ISOLATED]; \
	for ( i=0; i!=MAX_NB_ENTITIES_ISOLATED; ++i ) \
		name[i] = defaultvalue

/*
 * Initialisation 1
 */
void CMirrors::init( void (*cbUpdate)(), void (*cbSync)(), void (*cbRelease)() )
{
	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	datasetNames.push_back( "fame" );
	Mirror.init( datasetNames, cbMirrorIsReady, cbUpdate, cbSync, cbRelease, AISTag );
	Mirror.addCallbackWhenMirrorReadyForUse( cbMirrorReadyForAddEntity );
}


/*
 * Init after the mirror init
 */
void CMirrors::initMirror()
{
	//Mirror.declareEntityTypeUser( RYZOMID::player );
	
	Mirror.declareEntityTypeOwner( RYZOMID::npc,		TotalMaxNpc.get()	);
	Mirror.declareEntityTypeOwner( RYZOMID::creature,	TotalMaxFauna.get()+TotalMaxPet.get() );
	Mirror.declareEntityTypeOwner( RYZOMID::fx_entity, TotalMaxFx );
	
	DataSet = &(Mirror.getDataSet("fe_temp"));
	DataSet->declareProperty( "AIInstance", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "X", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "Y", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "Z", PSOReadWrite );
	DataSet->declareProperty( "Theta", PSOReadWrite );
	DataSet->declareProperty( "Sheet", PSOReadWrite );
	DataSet->declareProperty( "SheetServer", PSOReadWrite );
	DataSet->declareProperty( "NPCAlias", PSOWriteOnly );
	DataSet->declareProperty( "Mode", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "Behaviour", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "Target", PSOReadWrite | PSONotifyChanges );
	DataSet->declareProperty( "CurrentHitPoints", PSOReadOnly );
	DataSet->declareProperty( "CurrentRunSpeed", PSOReadOnly );
	DataSet->declareProperty( "CurrentWalkSpeed", PSOReadOnly );
	DataSet->declareProperty( "MaxHitPoints", PSOReadOnly );
	DataSet->declareProperty( "BestRoleLevel", PSOReadOnly );
	DataSet->declareProperty( "CombatState", PSOReadOnly );
	DataSet->declareProperty( "TeamId", PSOReadOnly | PSONotifyChanges );
	DataSet->declareProperty( "InOutpostZoneAlias", PSOReadWrite );
	DataSet->declareProperty( "InOutpostZoneSide", PSOReadWrite );
	DataSet->declareProperty( "VisualPropertyA", PSOReadWrite );
	DataSet->declareProperty( "VisualPropertyB", PSOReadWrite );
	DataSet->declareProperty( "VisualPropertyC", PSOReadWrite );
	DataSet->declareProperty( "ActionFlags", PSOReadWrite );
	DataSet->declareProperty( "VisionCounter", PSOReadOnly );
	DataSet->declareProperty( "Fuel", PSOReadOnly );
	DataSet->declareProperty( "WhoSeesMe", PSOReadWrite );
	DataSet->declareProperty( "ContextualProperty", PSOReadWrite );
	
	initRyzomVisualPropertyIndices( *DataSet );

	// init fame dataset
	FameDataSet = &(Mirror.getDataSet("fame"));
	CFameInterface::getInstance().setFameDataSet(FameDataSet);

	Mirror.setNotificationCallback( CMirrors::processMirrorUpdates );
}

// a big bad global var !
extern CAIEntityPhysical	*TempSpeaker;
extern CBotPlayer			*TempPlayer;


void	CMirrors::processMirrorUpdates()
{
	TDataSetRow	entityIndex;
	CEntityId	*pEntityId;

	CFameInterface &fi = CFameInterface::getInstance();
	
	// Process added entities
	DataSet->beginAddedEntities();
	while	((entityIndex = DataSet->getNextAddedEntity())	!=	LAST_CHANGED)
	{
		const CEntityId& entityId = DataSet->getEntityId(entityIndex);

		// Create CAIPlayer object and add to maps
		if	(entityId.getType() != RYZOMID::player)
			continue;

		{
			CMirrorPropValue<uint32>	_instanceNumber(*DataSet, entityIndex, DSPropertyAI_INSTANCE);
			const	uint32	askedInstance=_instanceNumber();
			CAIInstance *const	aii = CAIS::instance().getAIInstance(askedInstance);

			if	(!aii)
			{
				// The player is not in an AIInstance of that ais.
				// Floods on player connections, so no warning
			//	nlwarning("AIInstance %d not found to spawn player %s", askedInstance, entityId.toString().c_str());
			//	FOREACH(it,CCont<CAIInstance>, CAIS::instance().AIList())
			//	{
			//		nlwarning("exist AIInstance %d at index %d", (*it)->getInstanceNumber(), (*it)->getIndex());
			//	}
				continue;
			}
			// store the player in the correct instance.
			aii->getPlayerMgr()->addSpawnedPlayer(entityIndex, entityId);
		}

	}
	DataSet->endAddedEntities();

	FameDataSet->beginAddedEntities();
	while( (entityIndex = FameDataSet->getNextAddedEntity()) != LAST_CHANGED )
	{
		fi.createFameOwner(*FameDataSet, entityIndex);
	}
	FameDataSet->endAddedEntities();

	// Process removed entities
	DataSet->beginRemovedEntities();
	while( (entityIndex = DataSet->getNextRemovedEntity(&pEntityId)) != LAST_CHANGED )
	{
		CAIEntityPhysical	*entityPhysPtr	=	CAIS::instance().getEntityPhysical(entityIndex);
		//	just to check if its a player .. and remove it from Manager and Maps ..
		if	(	entityPhysPtr
			&&	entityPhysPtr->getRyzomType()==RYZOMID::player)
		{
			CBotPlayer	*botPlayer=NLMISC::safe_cast<CBotPlayer*>(entityPhysPtr);
			botPlayer->getOwner()->removeDespawnedPlayer(entityIndex);
		}
		
	}
	DataSet->endRemovedEntities();

	FameDataSet->beginRemovedEntities();
	while( (entityIndex = FameDataSet->getNextRemovedEntity(&pEntityId)) != LAST_CHANGED )
	{
		fi.removeFameOwner(*FameDataSet, entityIndex);
	}
	FameDataSet->endRemovedEntities();

	// Process properties changed and notified in the mirror
	TPropertyIndex propIndex;
	DataSet->beginChangedValues();
	DataSet->getNextChangedValue( entityIndex, propIndex );
	while	(entityIndex != LAST_CHANGED)
	{
		if (propIndex == DSPropertyPOSX || propIndex == DSPropertyPOSX )
		{
			const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
			if	(entityId.getType() == RYZOMID::player)
			{
				CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
				if (entityPhys)
				{
					CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);					
					if (player)
					{						
						CAIInstance* aiInstance = player->getAIInstance();
						aiInstance->updateZoneTrigger(player);						
					}
				}
			}
		}
		else if	(propIndex == DSPropertyTEAM_ID)
		{
			const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
			if	(entityId.getType() == RYZOMID::player)
			{
				CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
				if (entityPhys)
				{
					CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);
					if (player)
						player->getOwner()->updatePlayerTeam(entityIndex);
				}

			}

		}
		else if	(propIndex == DSPropertyAI_INSTANCE)
		{
			const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
			if	(entityId.getType() == RYZOMID::player)
			{
				CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
				if (entityPhys)
				{
					// this player is already in a manager, despawn it
					CBotPlayer	*const	botPlayer=NLMISC::safe_cast<CBotPlayer*>(entityPhys);
					botPlayer->getOwner()->removeDespawnedPlayer(entityIndex);
				}

				// could be done with mirrored values. (!?).
				CMirrorPropValue<uint32>	_instanceNumber(*DataSet, entityIndex, DSPropertyAI_INSTANCE);
				const	uint32	askedInstance=_instanceNumber();
				
				CAIInstance *aii = CAIS::instance().getAIInstance(askedInstance);

				if	(aii!=NULL)
				{
					// store the player in the correct instance.
					aii->getPlayerMgr()->addSpawnedPlayer(entityIndex, entityId);
				}
				else
				{
					if (askedInstance!=~0)
					{
						// no need to warn for ai number instance not in this AIS !
//						nlwarning("AIInstance %u not found on AIInstance changement for player %s", askedInstance, entityId.toString().c_str());
//						FOREACH(it,CCont<CAIInstance>, CAIS::instance().AIList())
//						{
//							nlwarning("exist AIInstance %d at index %d", (*it)->getInstanceNumber(), (*it)->getIndex());
//						}
					}
				}
			}
		}
		else if	(propIndex == DSPropertyTARGET_ID)
		{
			CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
			if (entityPhys)
			{
				const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
				if	(entityId.getType() == RYZOMID::player)
				{
					CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);

					// check if target is a local npc bot then handle bot targeted event
					CMirrorPropValueRO<TYPE_TARGET_ID> tgt(TheDataset, entityIndex, DSPropertyTARGET_ID);
					CAIEntityPhysical *target = CAIS::instance().getEntityPhysical(tgt());

					// update the targering list
					if	(entityIndex==TDataSetRow())	// no target.
					{
						player->setTarget(NULL);
					}
					else
					{
						if (player->isAggressive())
						{
							CAIEntityPhysical	*oldTarget=player->getTarget();
							if	(	!oldTarget 
								||	oldTarget!=target	)
								player->setTarget(target);
						}
						else
						{
							CAIEntityPhysical	*oldTarget=player->getVisualTarget();
							if	(	!oldTarget 
								||	oldTarget!=target	)
								player->setVisualTarget(target);
						}
					}

					CSpawnBotNpc *bnpc = dynamic_cast<CSpawnBotNpc *>(target);
					if (bnpc)
					{
						TempSpeaker = bnpc;
						TempPlayer = player;
						CGroupNpc	&grpNpc = bnpc->getPersistent().grp();
						
					

						if (IsRingShard)
						{
							CAIPos playerPos(player->pos());
							CAIPos npcPos(bnpc->pos());
							double dist = npcPos.quickDistTo(playerPos);
							bool dm = false;

							uint64 whoSeesMe = CMirrors::whoSeesMe(player->dataSetRow());
							if (!R2_VISION::isEntityVisibleToPlayers(whoSeesMe))
							{
								// The player is invisible, its a Dm / Gm: no zone event must be triggered
								dm = true;
							}
							
							if (!dm && dist <= RingMaxSelectDist)
							{
								grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerTargetNpc);
							}
						}
						else
						{
							// generate en event on this bot group
							grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerTargetNpc);
						}
							
					
							
					//		grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerTargetNpc);
//						grpNpc.getEventContainer().EventPlayerTargetNpc.processStateEvent(&grpNpc);

						// if player is in follow mode, then generate an suplementary event
						if	(player->getFollowMode())
							grpNpc.processStateEvent(grpNpc.getEventContainer().EventPlayerFollowNpc);
//							grpNpc.getEventContainer().EventPlayerFollowNpc.processStateEvent(&grpNpc);

						TempPlayer = NULL;
						TempSpeaker = NULL;
					}
				}
			}
		}
		else if	(propIndex == DSPropertyMODE)
		{
			CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
			if (entityPhys)
			{
				const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
				if	(entityId.getType() == RYZOMID::player)
				{
					CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);

					// if the player have a target, move it to the correct target list.

					if (player->isAggressive())
					{
						CAIEntityPhysical	*target=player->getVisualTarget();
						if (target)
							player->setTarget(target);
					}
					else
					{
						CAIEntityPhysical	*target=player->getTarget();
						if (target)
							player->setVisualTarget(target);
					}
				}
			}
		}
/*		else if (propIndex == DSPropertyBEHAVIOUR)
		{
			CAIEntityPhysical	*entityPhys=CAIS::instance().getEntityPhysical(entityIndex);
			if (entityPhys)
			{
				const CEntityId		&entityId=DataSet->getEntityId(entityIndex);
				if	(entityId.getType() == RYZOMID::player)
				{
					CBotPlayer	*player=NLMISC::safe_cast<CBotPlayer*>(entityPhys);

					MBEHAV::EBehaviour	const behaviour = player->getBehaviour();
					if( behaviour == MBEHAV::CAST_OFF_SUCCESS );
					{
						CAIEntityPhysical	*target=player->getVisualTarget();
						if (target)
							player->setTarget(target);
					}
				}
			}
		}
*/		DataSet->getNextChangedValue( entityIndex, propIndex );
	}
	DataSet->endChangedValues();
}

void CMirrors::release()
{
	Mirror.release();
}


bool CMirrors::mirrorIsReady()
{
	return	Mirror.mirrorIsReady();
}


//---------------------------------------------------------------------------------
// Methods for retrieving data from mirrors

TDataSetRow	CMirrors::createEntity( CEntityId& entityId )
{
	// in ais, we always use entityId auto asigment by mirror
	if	(Mirror.createEntity( entityId , true))
		return	DataSet->getDataSetRow( entityId );
	else
		return	TDataSetRow();
}


void CMirrors::declareEntity( const TDataSetRow& entityIndex )
{
	DataSet->declareEntity( entityIndex ); // only in the main dataset
}


void CMirrors::removeEntity( const CEntityId& entityId )
{
	Mirror.removeEntity( entityId );
}


bool CMirrors::exists( const TDataSetRow& entityIndex )
{
	/// is that a bug??
	return DataSet->getEntityId( entityIndex ).asUint64() == 0;
	//return ! ((uint64)(DataSet->getEntityId( entityIndex )) != 0);
}

const NLMISC::CEntityId& CMirrors::getEntityId( const TDataSetRow& entityIndex )
{
	return DataSet->getEntityId( entityIndex );
}

TDataSetRow CMirrors::getDataSetRow( const NLMISC::CEntityId& entityId )
{
	return DataSet->getDataSetRow( entityId );
}

uint16	CMirrors::getTeamId(const TDataSetRow& entityIndex)
{
	CMirrorPropValueRO<TYPE_TEAM_ID> value( *DataSet, entityIndex, DSPropertyTEAM_ID );
//	if ( value()==0 )
//		return CTEAM::InvalidTeamId;
	return value;

}


CAICoord CMirrors::x( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_POSX> value( *DataSet, entityIndex, DSPropertyPOSX );
	if ( value()==0 )
		return CAICoord();
	return CAICoord((double)value()/(double)1000);
}

CAICoord CMirrors::y( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_POSY> value( *DataSet, entityIndex, DSPropertyPOSY );
	if ( value()==0 )
		return CAICoord();
	return CAICoord((double)value()/(double)1000);
}

sint32 CMirrors::z( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_POSZ> value( *DataSet, entityIndex, DSPropertyPOSZ );
	return value();
}

float CMirrors::theta( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_ORIENTATION> value( *DataSet, entityIndex, DSPropertyORIENTATION );
	return value();
}


void CMirrors::setPosAndTheta( const TDataSetRow& entityIndex, sint32 posX, sint32 posY, sint32 posZ, float angleRad )
{
#ifdef NL_DEBUG
	//nldebug( "%d: E%d: Setting pos to %d %d", CTickEventHandler::getGameCycle(), entityIndex, posX, posY );
	/*if ( posX == 0 )
		nldebug( "E%d: X is zero", entityIndex );
	if ( posY == 0 )
		nldebug( "E%d: Y is zero", entityIndex );*/
#endif

	CMirrorPropValue<TYPE_POSX> valueX( *DataSet, entityIndex, DSPropertyPOSX );
	CMirrorPropValue<TYPE_POSY> valueY( *DataSet, entityIndex, DSPropertyPOSY );
	CMirrorPropValue<TYPE_POSZ> valueZ( *DataSet, entityIndex, DSPropertyPOSZ );
	CMirrorPropValue<TYPE_ORIENTATION> valueT( *DataSet, entityIndex, DSPropertyORIENTATION );
	valueX = posX;
	valueY = posY;
	valueZ = posZ;
	valueT = angleRad;
	//nldebug( "%u: Pos&theta of E%d set", CTickEventHandler::getGameCycle(), entityIndex );
}


void	CMirrors::setTheta( const TDataSetRow& entityIndex, float angleRad )
{
	CMirrorPropValue<TYPE_ORIENTATION> valueT( *DataSet, entityIndex, DSPropertyORIENTATION );
	valueT = angleRad;
	//nldebug( "%u: Theta of E%d set", CTickEventHandler::getGameCycle(), entityIndex );
}


NLMISC::CSheetId CMirrors::sheet( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_SHEET> value( *DataSet, entityIndex, DSPropertySHEET );
	return NLMISC::CSheetId(value());
}

void CMirrors::initSheet( const TDataSetRow& entityIndex, const NLMISC::CSheetId& sheetId )
{
	CMirrorPropValue<TYPE_SHEET> value( *DataSet, entityIndex, DSPropertySHEET );
	value = sheetId.asInt();
}

NLMISC::CSheetId CMirrors::sheetServer( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_SHEET> value( *DataSet, entityIndex, DSPropertySHEET_SERVER );
	return NLMISC::CSheetId(value());
}

void CMirrors::initSheetServer( const TDataSetRow& entityIndex, const NLMISC::CSheetId& sheetId )
{
	nlassert(sheetId != NLMISC::CSheetId::Unknown);
	CMirrorPropValue<TYPE_SHEET> value( *DataSet, entityIndex, DSPropertySHEET_SERVER );
	value = sheetId.asInt();
}

void CMirrors::initNPCAlias( const TDataSetRow& entityIndex, TAIAlias alias )
{
	CMirrorPropValue<TYPE_ALIAS> value( *DataSet, entityIndex, DSPropertyNPC_ALIAS );
	value = alias;
}

TDataSetRow CMirrors::target( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_TARGET_ID> value( *DataSet, entityIndex, DSPropertyTARGET_ID );
	return value();
}

void CMirrors::setVPA( const TDataSetRow& entityIndex, const SAltLookProp &prop )
{
	CMirrorPropValue<SAltLookProp> value( *DataSet, entityIndex, DSPropertyVPA );
	value = prop;
}

void CMirrors::setVisualPropertyA( const TDataSetRow& entityIndex, const SPropVisualA &prop )
{
	CMirrorPropValue<SPropVisualA> value( *DataSet, entityIndex, DSPropertyVPA );
	value = prop;
}

void CMirrors::setVisualPropertyB( const TDataSetRow& entityIndex, const SPropVisualB &prop )
{
	CMirrorPropValue<SPropVisualB> value( *DataSet, entityIndex, DSPropertyVPB );
	value = prop;
}

void CMirrors::setVisualPropertyC( const TDataSetRow& entityIndex, const SPropVisualC &prop )
{
	CMirrorPropValue<SPropVisualC> value( *DataSet, entityIndex, DSPropertyVPC );
	value = prop;
}

void CMirrors::setBehaviour( const TDataSetRow& entityIndex, MBEHAV::EBehaviour b )
{
	CMirrorPropValue<MBEHAV::CBehaviour> value( *DataSet, entityIndex, DSPropertyBEHAVIOUR );
	value = b;
}

uint32 CMirrors::bestRoleLevel( const TDataSetRow& entityIndex )
{
	CMirrorPropValueRO<TYPE_BEST_ROLE_LEVEL> value( *DataSet, entityIndex, DSPropertyBEST_ROLE_LEVEL );
	return value();
}

uint64 CMirrors::whoSeesMe( const TDataSetRow& entityIndex )
{
	BOMB_IF( !DataSet->isAccessible(entityIndex), "Try to access to the WhoSeesMe property in the mirror but with an invalid ", return 0L);
	CMirrorPropValueRO<TYPE_WHO_SEES_ME> value( *DataSet, entityIndex, DSPropertyWHO_SEES_ME  );
	return value();
}

/*
 * Set target (row)
 */
void CMirrors::setTarget(const TDataSetRow& entityIndex, const TDataSetRow& target)
{
	CMirrorPropValue<TYPE_TARGET_ID> value( *DataSet, entityIndex, DSPropertyTARGET_ID );
	value = target;
}

/// :KLUDGE: This method implementation should be in game_share instead of AIS and EGS. Both me and the other one are lazy bastards.
void	MBEHAV::TMode::setModeAndPos( EMode mode, const TDataSetRow& entityIndex )
{
	Mode = mode;
	CMirrorPropValueRO<TYPE_POSX> propX( TheDataset, entityIndex, DSPropertyPOSX );
	Pos.X16 = (uint16)(propX() >> 4);
	CMirrorPropValueRO<TYPE_POSY> propY( TheDataset, entityIndex, DSPropertyPOSY );
	Pos.Y16 = (uint16)(propY() >> 4);
	//nldebug( "Setting MODE %s for E%u with current pos %d, %d", modeToString( mode ).c_str(), entityIndex.getIndex(), propX(), propY() );
}

#include "event_reaction_include.h"
