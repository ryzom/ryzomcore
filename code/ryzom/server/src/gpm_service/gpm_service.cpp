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

// misc
#include "nel/misc/command.h"
#include "nel/misc/vector_2d.h"
#include "nel/misc/vectord.h"
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/path.h"
#include "nel/misc/sheet_id.h"
//
#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
//
// game share
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"
#include "game_share/mirrored_data_set.h"
#include "game_share/ryzom_mirror_properties.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/synchronised_message.h"
#include "server_share/used_continent.h"
#include "server_share/pet_interface_msg.h"
#include "game_share/utils.h"
//
// r2 share
#include "server_share/r2_variables.h"
//
// local
#include "gpm_service.h"
#include "world_position_manager.h"
#include "vision_delta_manager.h"
#include "client_messages.h"
#include "sheets.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


using namespace std;
using namespace NLMISC;
using namespace NLNET;

CGlobalPositionManagerService *pCGPMS= NULL;

NLLIGO::CLigoConfig LigoConfig;


/*
 * Get var from config file
 */
// Sint version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, sint32 &variable, sint32 defaultValue = 0)
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? ptr->asInt() : defaultValue);
	return success;
}
// Uint version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, uint32 &variable, uint32 defaultValue = 0)
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? ptr->asInt() : defaultValue);
	return success;
}
// Bool version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, bool &variable, bool defaultValue = false)
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? (ptr->asInt() != 0) : defaultValue);
	return success;
}
// Float version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, float &variable, float defaultValue = 0.0f)
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? ptr->asFloat() : defaultValue);
	return success;
}
// Double version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, double &variable, double defaultValue = 0)
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? ptr->asDouble() : defaultValue);
	return success;
}
// String version
bool	getVarFromConfigFile(CConfigFile &cf, const string &name, string &variable, const string &defaultValue = string(""))
{
	CConfigFile::CVar	*ptr = cf.getVarPtr(name);
	bool	success;
	variable = ((success = (ptr != NULL)) ? ptr->asString() : defaultValue);
	return success;
}

#define GET_VAR_FROM_CF(var, def) getVarFromConfigFile(ConfigFile, #var, var, def);

/*-----------------------------------------------------------------*\
						cbConnection
\*-----------------------------------------------------------------*/
void cbConnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
} // cbConnection //


TGameCycle TickStopGameCycle = 0;

/*-----------------------------------------------------------------*\
						cbDisconnection
\*-----------------------------------------------------------------*/
void cbDisconnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	if( serviceName == string("TICKS") )
	{
		TickStopGameCycle = CTickEventHandler::getGameCycle();
		nlinfo( "tick stop -> %u", TickStopGameCycle );
	}

	// remove all entities created by this service
	//CWorldPositionManager::removeAiVisionEntitiesForService( serviceId );
	if (!IsRingShard)
	{
		CWorldPositionManager::triggerUnsubscribe(serviceId);
	}

} // cbDisconnection //


void cbEGSConnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
// The follwoing code commented out by Sadge because no refference could be found to reception of this message
//	// transmit the names of the used continents to the EGS
//	try
//	{
//		CUsedContinent::TUsedContinentCont continents = CUsedContinent::instance().getContinents();
//
//		vector< string > continentsNames;
//		for (uint i=0; i<continents.size(); ++i)
//		{
//			continentsNames.push_back(continents[i].ContinentName);
//		}
///*		CConfigFile::CVar& cvUsedContinents = pCGPMS->ConfigFile.getVar("UsedContinents");
//		for ( uint i = 0; (sint)i<cvUsedContinents.size(); ++i)
//		{
//			if (cvUsedContinents.asString(i) != "")
//			{
//				continentsNames.push_back(cvUsedContinents.asString(i));
//			}
//		}
//*/
//		CMessage msgout("USED_CONTINENTS");
//		msgout.serialCont( continentsNames ),
//		CUnifiedNetwork::getInstance()->send( "EGS", msgout );
//	}
//	catch(const EUnknownVar &)
//	{
//		nlwarning("<cbEGSConnection> UsedContinents not found, no continent used");
//	}

	// send ESG all IA creatures
	//CWorldPositionManager::sendAgentsToEGS();

} // cbEGSConnection //

/*-----------------------------------------------------------------*\
						EVSConnection
\*-----------------------------------------------------------------*/
void cbEVSConnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	EVSUp = true;
}

/*-----------------------------------------------------------------*\
						EVSDisconnection
\*-----------------------------------------------------------------*/
void cbEVSDisconnection( const std::string &serviceName, NLNET::TServiceId serviceId, void *arg )
{
	EVSUp = false;
}


/*--------------------------------------------------------------*\
						cbSync()
\*--------------------------------------------------------------*/
void cbSync()
{
	// update World Position Manager time
	if( !TickStopGameCycle )
	{
		if (!IsRingShard) { CWorldPositionManager::setCurrentTick( CTickEventHandler::getGameCycle() ); }
	}
	nlinfo( "Sync -> %u", CTickEventHandler::getGameCycle() );
} // cbSync //



/*
 * Crash Callback
 */
string	crashCallback()
{
	return CWorldPositionManager::getPlayersPosHistory();
}



/****************************************************************\
 ************** callback table for input message ****************
\****************************************************************/
TUnifiedCallbackItem CbArray[] =
{
	{ "CB_UNUSED",					NULL }
};


/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	pCGPMS->initMirror();
}


/****************************************************************\
							init()
\****************************************************************/
// init the service
void CGlobalPositionManagerService::init()
{
	setVersion (RYZOM_VERSION);

	// keep pointer on class
	pCGPMS = this;

	// set update time out
	setUpdateTimeout(100);

	CUnifiedNetwork::getInstance()->setServiceUpCallback( string("*"), cbConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "EGS", cbEGSConnection, 0);
	CUnifiedNetwork::getInstance()->setServiceUpCallback( "EVS", cbEVSConnection, 0);

	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbDisconnection, 0);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( "EVS", cbEVSDisconnection, 0);


	uint32		WorldMapSizeX;
	uint32		WorldMapSizeY;
	uint32		VisionDistance;
	uint32		PrimitiveMaxSize;
	uint32		NbWorldImages;
	bool		LoadPacsPrims;
	bool		LoadPacsCol;

	// init the class transport system
	TRANSPORT_CLASS_REGISTER (CGPMPlayerPrivilegeInst);

	GET_VAR_FROM_CF(CheckPlayerSpeed, true);
	GET_VAR_FROM_CF(Verbose, false);

	GET_VAR_FROM_CF(WorldMapSizeX, 2100);
	GET_VAR_FROM_CF(WorldMapSizeY, 2970);
	GET_VAR_FROM_CF(VisionDistance, 250000);
	GET_VAR_FROM_CF(PrimitiveMaxSize, 8);
	GET_VAR_FROM_CF(NbWorldImages, 31);

	GET_VAR_FROM_CF(LoadPacsCol, false);
	GET_VAR_FROM_CF(LoadPacsPrims, true);


	CGpmSheets::init();

	// World Position Manager init
	if (!IsRingShard)
	{
		CWorldPositionManager::init(WorldMapSizeX, WorldMapSizeY, VisionDistance, PrimitiveMaxSize, (uint8)NbWorldImages, LoadPacsPrims, LoadPacsCol);
	}

	// Init ligo
	if (!LigoConfig.readPrimitiveClass ("world_editor_classes.xml", false))
	{
		// Should be in l:\leveldesign\world_editor_files
		nlerror ("Can't load ligo primitive config file world_editor_classes.xml");
	}
/*	// read the continent name translator
	map<string, string>	translator;
	{
		CConfigFile::CVar *v = IService::getInstance()->ConfigFile.getVarPtr("ContinentNameTranslator");
		if (v)
		{
			for (sint i=0; i<v->size()/2; ++i)
			{
				string s1, s2;
				s1 = v->asString(i*2);
				s2 = v->asString(i*2+1);
				translator[s1] = s2;
			}
		}
	}
*/

	// todo: r2 GPMS doesn't read pacs for now - this will have to be fixed later
	if (!IsRingShard)
	{
		// init continents
		try
		{
			CUsedContinent::TUsedContinentCont continents = CUsedContinent::instance().getContinents();
	//		CConfigFile::CVar& cvUsedContinents = ConfigFile.getVar("UsedContinents");
	//		uint	i;
	//		for (i=0; (sint)i<cvUsedContinents.size(); ++i)
	//			if (cvUsedContinents.asString(i) != "")
			for (uint i=0; i<continents.size(); ++i)
			{
				string name = continents[i].ContinentName;
				name = CUsedContinent::instance().getPhysicalContinentName(name);
	//			if (translator.find(name) != translator.end())
	//				name = translator[name];
	//			CWorldPositionManager::loadContinent(cvUsedContinents.asString(i), cvUsedContinents.asString(i), i);
				CWorldPositionManager::loadContinent(name, continents[i].ContinentName, continents[i].ContinentInstance);
			}
		}
		catch(const EUnknownVar &)
		{
			nlwarning("<CGlobalPositionManagerService::init> UsedContinents not found, no continent used");
		}
	}

	NLLIGO::Register();

	CMessages::init();
	CClientMessages::init();

	//
	if (!IsRingShard) { CWorldPositionManager::initPatatManager(); }

	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	Mirror.init( datasetNames, cbMirrorIsReady, gpmsUpdate, cbSync );
	Mirror.setServiceMirrorUpCallback( "EGS", cbEGSConnection, 0);

	setCrashCallback(crashCallback);

	if (IsRingShard)
	{
		// setup the R2 Vision object and move checker object
		pCGPMS->RingVisionDeltaManager=new CVisionDeltaManager;
		pCGPMS->RingVisionUniverse= new R2_VISION::CUniverse;
		pCGPMS->RingVisionUniverse->registerVisionDeltaManager(RingVisionDeltaManager);
		pCGPMS->MoveChecker= new CMoveChecker;
	}

} // init //


/*
 * Init after the mirror init
 */
void CGlobalPositionManagerService::initMirror()
{
/*
	// Allow to add a few entities manually (using the command addEntity)
	Mirror.declareEntityTypeOwner( RYZOMID::player, 10 );
	Mirror.declareEntityTypeOwner( RYZOMID::npc, 500 );
*/

	DataSet = &(Mirror.getDataSet("fe_temp"));
	DataSet->declareProperty( "X", PSOReadWrite | PSONotifyChanges, "X" );		// group notification on X
	DataSet->declareProperty( "Y", PSOReadWrite | PSONotifyChanges, "X" );		// group notification on X
	DataSet->declareProperty( "Z", PSOReadWrite | PSONotifyChanges, "X" );		// group notification on X
	DataSet->declareProperty( "Theta", PSOReadWrite | PSONotifyChanges, "X" );	// group notification on X
	DataSet->declareProperty( "AIInstance", PSOReadOnly | PSONotifyChanges );
	DataSet->declareProperty( "WhoSeesMe", PSOReadOnly | PSONotifyChanges );
	DataSet->declareProperty( "LocalX", PSOReadWrite );
	DataSet->declareProperty( "LocalY", PSOReadWrite );
	DataSet->declareProperty( "LocalZ", PSOReadWrite );
	DataSet->declareProperty( "TickPos", PSOReadWrite );
	DataSet->declareProperty( "Sheet", PSOReadWrite );
	DataSet->declareProperty( "Mode", PSOReadOnly /*| PSONotifyChanges*/ );
	DataSet->declareProperty( "Behaviour", PSOReadOnly );
	DataSet->declareProperty( "Cell", PSOReadWrite );
	DataSet->declareProperty( "VisionCounter", PSOReadWrite );
	DataSet->declareProperty( "CurrentRunSpeed", PSOReadOnly );
	DataSet->declareProperty( "CurrentWalkSpeed", PSOReadOnly );
	DataSet->declareProperty( "RiderEntity", PSOReadOnly );
	DataSet->declareProperty( "Fuel", PSOReadOnly );
	initRyzomVisualPropertyIndices( *DataSet );

	Mirror.setNotificationCallback( IsRingShard? CGlobalPositionManagerService::ringShardProcessMirrorUpdates: CGlobalPositionManagerService::processMirrorUpdates );
}


/****************************************************************\
							update()
\****************************************************************/
// main loop
bool CGlobalPositionManagerService::update()
{
	return true;
} // update //


void CGlobalPositionManagerService::_checkAddCharacterToRingAIInstance(sint32 aiInstance)
{
	TCharactersPerAIInstance::iterator it= _CharactersPerAIInstance.find(aiInstance);
	if (it==_CharactersPerAIInstance.end())
	{
		// this is the first character to be added to this instance so initialise it
		_CharactersPerAIInstance[aiInstance]=1;
		nlinfo("Creating new AIInstance in ring vision universe: %d",aiInstance);
		pCGPMS->RingVisionUniverse->createInstance(aiInstance,0);
	}
	else
	{
		++it->second;
		nldebug("Increasing number of entities in aiInstance %d to %d",aiInstance,it->second);
	}
}

void CGlobalPositionManagerService::_checkRemoveCharacterFromRingAIInstance(sint32 aiInstance)
{
	TCharactersPerAIInstance::iterator it= _CharactersPerAIInstance.find(aiInstance);
	BOMB_IF(it==_CharactersPerAIInstance.end(),NLMISC::toString("BUG: Can't find ai instance %d to remove character from",aiInstance),return);

	// decrement count of characters in ai instance and remove the instance if its empty
	--it->second;
	if (it->second<1)
	{
		// this was the last character in the instance so delete it
		nlinfo("removing AIInstance because last character has just left: %d",aiInstance);
		_CharactersPerAIInstance.erase(it);
		pCGPMS->RingVisionUniverse->removeInstance(aiInstance);
	}
	else
	{
		nldebug("Decreasing number of entities in aiInstance %d to %d",aiInstance,it->second);
	}
}

void CGlobalPositionManagerService::ringShardProcessMirrorUpdates()
{
	// Process entities added to mirror
	TheDataset.beginAddedEntities();
	TDataSetRow entityIndex = TheDataset.getNextAddedEntity();
	while ( entityIndex != LAST_CHANGED )
	{
		// lookup stats for the entity in the mirror
		const NLMISC::CEntityId &eid= TheDataset.getEntityId( entityIndex );
		CMirrorPropValueRO<sint32> aiInstance	( TheDataset, entityIndex, DSPropertyAI_INSTANCE );
		CMirrorPropValueRO<sint32> x			( TheDataset, entityIndex, DSPropertyPOSX );
		CMirrorPropValueRO<sint32> y			( TheDataset, entityIndex, DSPropertyPOSY );
		CMirrorPropValueRO<TYPE_WHO_SEES_ME> whoSeesMe	( TheDataset, entityIndex, DSPropertyWHO_SEES_ME );
		R2_VISION::TInvisibilityLevel invisibility= (R2_VISION::TInvisibilityLevel)(whoSeesMe&((1<<R2_VISION::NUM_WHOSEESME_BITS)-1));

		// increment count of number of characters in AIInstance, instanciating new instance if required
		pCGPMS->_checkAddCharacterToRingAIInstance(aiInstance);

		// if we have a player then set them up in the move checker
		if (eid.getType()==RYZOMID::player)
		{
			pCGPMS->MoveChecker->teleport(entityIndex, x, y, CTickEventHandler::getGameCycle());
		}

		// add entity to the ring vision universe object
		pCGPMS->RingVisionUniverse->addEntity(entityIndex,aiInstance,x,y,invisibility,eid.getType()==RYZOMID::player);
		entityIndex = TheDataset.getNextAddedEntity();
	}
	TheDataset.endAddedEntities();

	// Process entities removed from mirror
	TheDataset.beginRemovedEntities();
	CEntityId *id;
	entityIndex = TheDataset.getNextRemovedEntity( &id );
	while ( entityIndex != LAST_CHANGED )
	{
		// check that the character being removed exists in the r2VisionUniverse and get hold of their AIInstance
		const R2_VISION::SUniverseEntity* theEntity= pCGPMS->RingVisionUniverse->getEntity(entityIndex);
		BOMB_IF(theEntity==NULL,"Failed to identify the character that the mirror tells us is being removed",continue);
		uint32 aiInstance= theEntity->AIInstance;

		// remove entity from the ring vision universe object
		pCGPMS->RingVisionUniverse->removeEntity(entityIndex);

		// decrement the count of characters in the given instance and remove it if need be
		pCGPMS->_checkRemoveCharacterFromRingAIInstance(aiInstance);

		// prepare to iterate...
		entityIndex = TheDataset.getNextRemovedEntity( &id );
	}
	TheDataset.endRemovedEntities();

	// Process properties changed and notified in the mirror
	TPropertyIndex propIndex;
	TheDataset.beginChangedValues();
	TheDataset.getNextChangedValue( entityIndex, propIndex );
	while ( entityIndex != LAST_CHANGED )
	{
		if (propIndex == DSPropertyAI_INSTANCE)
		{
			// lookup stats for the entity in the mirror
			sint32 aiInstance=	CMirrorPropValueRO<sint32>( TheDataset, entityIndex, DSPropertyAI_INSTANCE );
			sint32 x= CMirrorPropValueRO<sint32>( TheDataset, entityIndex, DSPropertyPOSX );
			sint32 y= CMirrorPropValueRO<sint32>( TheDataset, entityIndex, DSPropertyPOSY );
			CMirrorPropValueRO<TYPE_WHO_SEES_ME> whoSeesMe ( TheDataset, entityIndex, DSPropertyWHO_SEES_ME );
			R2_VISION::TInvisibilityLevel invisibility= (R2_VISION::TInvisibilityLevel)(whoSeesMe&((1<<R2_VISION::NUM_WHOSEESME_BITS)-1));

			// if we have a player then remove them from the move checker
			if (TheDataset.getEntityId(entityIndex).getType()==RYZOMID::player)
			{
				pCGPMS->MoveChecker->teleport(entityIndex, x, y, CTickEventHandler::getGameCycle());
			}

			// check that the character being teleported exists in the r2VisionUniverse and get hold of their old AIInstance
			const R2_VISION::SUniverseEntity* theEntity= pCGPMS->RingVisionUniverse->getEntity(entityIndex);
			BOMB_IF(theEntity==NULL, "Failed to identify the character that the mirror tells us is being removed",
				TheDataset.getNextChangedValue( entityIndex, propIndex ); continue);
			sint32 oldAiInstance= theEntity->AIInstance;

			// check whether this is a real teleportation or just a move
			if (oldAiInstance==aiInstance)
			{
				// the aiInstance hasn't changed so just perform a move
				// note: this happens systematicaly on appearance of a new entity - the ai instance is
				// setup once via code that manages appearance of new entities in mirror and it's setup
				// again here... the coordinates are probably the same too but since I can't guarantee
				// it I prefer to let the code do its stuff
				pCGPMS->RingVisionUniverse->setEntityPosition(entityIndex,x,y);
			}
			else
			{
				// make sure the instance we're teleporting to exists
				pCGPMS->_checkAddCharacterToRingAIInstance(aiInstance);

				// teleport entity within the ring vision universe
				pCGPMS->RingVisionUniverse->teleportEntity(entityIndex,aiInstance,x,y,invisibility);

				// if this was the last entity in their old instance then get rid of the instance
				pCGPMS->_checkRemoveCharacterFromRingAIInstance(oldAiInstance);
			}
		}
		else if (propIndex == DSPropertyPOSX)
		{
			// lookup stats for the entity in the mirror
			CMirrorPropValueRO<sint32> x ( TheDataset, entityIndex, DSPropertyPOSX );
			CMirrorPropValueRO<sint32> y ( TheDataset, entityIndex, DSPropertyPOSY );

			// update the cell
			CMirrorPropValue1DS<TYPE_CELL> cell ( TheDataset, entityIndex, DSPropertyCELL );
			uint32	cx = (uint16) ( + x()/CWorldPositionManager::getCellSize() );
			uint32	cy = (uint16) ( - y()/CWorldPositionManager::getCellSize() );

			cell = (cx<<16) + cy;

			// move entity within the ring vision universe
			pCGPMS->RingVisionUniverse->setEntityPosition(entityIndex,x,y);
		}
		else if (propIndex == DSPropertyWHO_SEES_ME)
		{
			// lookup stats for the entity in the mirror
			CMirrorPropValueRO<TYPE_WHO_SEES_ME> whoSeesMe ( TheDataset, entityIndex, DSPropertyWHO_SEES_ME );

			uint32 visibilityValue= ((uint32)whoSeesMe==0)? R2_VISION::WHOSEESME_INVISIBLE_DM: ((uint32)(whoSeesMe+1)==0)? R2_VISION::WHOSEESME_VISIBLE_MOB: (uint32)whoSeesMe;

			// apply the change to the entity
			pCGPMS->RingVisionUniverse->setEntityInvisibilityInfo(entityIndex, visibilityValue);
		}
		TheDataset.getNextChangedValue( entityIndex, propIndex );
	}
	TheDataset.endChangedValues();
}

void CGlobalPositionManagerService::processMirrorUpdates()
{
	// Process entities added to mirror
	TheDataset.beginAddedEntities();
	TDataSetRow entityIndex = TheDataset.getNextAddedEntity();
	while ( entityIndex != LAST_CHANGED )
	{
		//nldebug( "%u: OnAddEntity %d", CTickEventHandler::getGameCycle(), entityIndex );
		CWorldPositionManager::onAddEntity( entityIndex );
		entityIndex = TheDataset.getNextAddedEntity();
	}
	TheDataset.endAddedEntities();

	// Process entities removed from mirror
	TheDataset.beginRemovedEntities();
	CEntityId *id;
	entityIndex = TheDataset.getNextRemovedEntity( &id );
	while ( entityIndex != LAST_CHANGED )
	{
		//nldebug( "%u: OnRemoveEntity %d", CTickEventHandler::getGameCycle(), entityIndex );
		CWorldPositionManager::onRemoveEntity(entityIndex);
		entityIndex = TheDataset.getNextRemovedEntity( &id );
	}
	TheDataset.endRemovedEntities();

	// Process properties changed and notified in the mirror
	TPropertyIndex propIndex;
	TheDataset.beginChangedValues();
	TheDataset.getNextChangedValue( entityIndex, propIndex );
	while ( entityIndex != LAST_CHANGED )
	{
		//nldebug( "%u: OnPosChange %d", CTickEventHandler::getGameCycle(), entityIndex );
		//nlassert( propIndex == DSPropertyPOSX ); // (X, Y, Z, Theta) use "group notification" with prop X (and are the only ones with notification)

		if (propIndex == DSPropertyPOSX)
		{
			// TEMP: while we don't handle all by entity indices, we need to test if the entityId has been notified (added)
			CWorldEntity	*entity = CWorldPositionManager::getEntityPtr(entityIndex);
			if (entity)
			{
				if (entity->getType() == CWorldEntity::Player && entity->CheckMotion)
				{
					// this is a player and has to check motion
					// then reset the player position according to the mirror pos
					H_AUTO(gpmsUpdateResetServerPosition);
					CWorldPositionManager::resetPrimitive(entity);
					entity->PosInitialised = true;
				}
				else
				{
					H_AUTO(gpmsUpdateServerPosition);
					//nlinfo("Update %s pos: %d,%d,%d - %d", entity->Id.toString().c_str(), entity->X(), entity->Y(), entity->Z(), entity->X.getWriterServiceId());
					CWorldPositionManager::updateEntityPosition(entity);
				}
			}
		}
		//nldebug( "Pos changed from mirror E%d", entityIndex  );
		TheDataset.getNextChangedValue( entityIndex, propIndex );
	}
	TheDataset.endChangedValues();
}


/****************************************************************\
						gpmsUpdate()
\****************************************************************/
void CGlobalPositionManagerService::gpmsUpdate()
{
	if ( ! pCGPMS->Mirror.mirrorIsReady() )
		return;

	H_AUTO(gpmsUpdate);

	// process vision update for non-ring shards
	if (!IsRingShard)
	{
		// also update internal clock (increase tick counter by one)
		H_TIME(PositionManagerUpdate, CWorldPositionManager::update(););

		uint	i;
		for (i=0; i<pCGPMS->Tracked.size(); ++i)
			CWorldPositionManager::displayEntity(CWorldPositionManager::getEntityIndex(pCGPMS->Tracked[i]));
	}

	// process vision update for ring shards
	if (IsRingShard)
	{
		H_AUTO(ringVisionUpdate);
		pCGPMS->RingVisionUniverse->update();
		pCGPMS->RingVisionDeltaManager->update();
	}

} // gpmsUpdate //


/****************************************************************\
							release()
\****************************************************************/
void CGlobalPositionManagerService::release()
{
	CMessages::release();

	if (!IsRingShard)
	{
		CWorldPositionManager::release();
	}

	CGpmSheets::release();

}// release //




NLNET_SERVICE_MAIN( CGlobalPositionManagerService, "GPMS", "gpm_service", 0, CbArray, "", "" );

