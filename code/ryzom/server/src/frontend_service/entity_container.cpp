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
#include "entity_container.h"
#include "frontend_service.h"
#include <vector>

using namespace std;
using namespace NLMISC;
using namespace CLFECOMMON;

const	TDataSetIndex LAST_VISION_CHANGE = LAST_CHANGED;


CMirroredDataSet *CEntity::DataSet = NULL;

CEntityContainer *CEntityContainer::I = NULL;

TPropertyIndex		CEntityContainer::_VisualPropIndexToDataSet[CLFECOMMON::NB_VISUAL_PROPERTIES];

// Property indices that are not in game_share/ryzom_mirror_properties.h
TPropertyIndex DSPropertyTickPos;
TPropertyIndex DSPropertyLocalX;
TPropertyIndex DSPropertyLocalY;
TPropertyIndex DSPropertyLocalZ;
//TPropertyIndex DSPropertyStunned;
TPropertyIndex DSFirstPropertyAvailableImpulseBitSize;


// Use the callbacks in game_share/ryzom_mirror_properties.cpp
extern std::string BehaviourToStringCb( void *value );
extern std::string ModeToStringCb( void *value );


/*
 * Constructor
 */
CEntityContainer::CEntityContainer() : EntityToClient(NULL), _DataSet(NULL)
{
	nlassert( !I );
	I = this;
}


/*
 * Initialisation 2
 */
void	cbMirrorIsReady( CMirror *mirror )
{
	CFrontEndService::instance()->postInit();
}


/*
 * Fast check if an addition has been notified
 */
bool	cbAdditionNotifiedCallback( const TDataSetRow& entityIndex )
{
	return CEntityContainer::I->getEntity(entityIndex)->TickPosition.isReadable();
}


void cbNotificationCallback()
{
	CEntityContainer::I->updateMirror();
}


/*
 * Initialisation
 */
void CEntityContainer::init( CClientIdLookup *cl, void (*cbUpdate)(), void (*cbSync)() )
{
	EntityToClient = cl;

	// Init the mirror system
	vector<string> datasetNames;
	datasetNames.push_back( "fe_temp" );
	_Mirror.init( datasetNames, cbMirrorIsReady, cbUpdate, cbSync );
}


/*
 * Initialisation of the properties
 */
void CEntityContainer::initMirror()
{
	// Declare the property needed as readOnly, not notifying the changes
	_DataSet = &(_Mirror.getDataSet("fe_temp"));
	_DataSet->declareProperty( "X", PSOReadOnly | PSONotifyChanges ); // for mileage calculation
	_DataSet->declareProperty( "Y", PSOReadOnly | PSONotifyChanges ); // for mileage calculation
	_DataSet->declareProperty( "Z", PSOReadOnly );
	_DataSet->declareProperty( "Theta", PSOReadOnly );
	_DataSet->declareProperty( "Sheet", PSOReadOnly );
	_DataSet->declareProperty( "NPCAlias", PSOReadOnly );
	_DataSet->declareProperty( "Behaviour", PSOReadOnly );
	_DataSet->declareProperty( "NameIndex", PSOReadOnly );
	_DataSet->declareProperty( "Target", PSOReadOnly );
	_DataSet->declareProperty( "Mode", PSOReadOnly );
	_DataSet->declareProperty( "VisualPropertyA", PSOReadOnly );
	_DataSet->declareProperty( "VisualPropertyB", PSOReadOnly );
	_DataSet->declareProperty( "VisualPropertyC", PSOReadOnly );
	_DataSet->declareProperty( "EntityMounted", PSOReadOnly );
	_DataSet->declareProperty( "RiderEntity", PSOReadOnly );
	_DataSet->declareProperty( "ContextualProperty", PSOReadOnly );
	_DataSet->declareProperty( "TickPos", PSOReadOnly );
	_DataSet->declareProperty( "LocalX", PSOReadOnly );
	_DataSet->declareProperty( "LocalY", PSOReadOnly );
	_DataSet->declareProperty( "LocalZ", PSOReadOnly );
	_DataSet->declareProperty( "Bars", PSOReadOnly );
	//_DataSet->declareProperty( "Stunned", PSOReadOnly ); // to block or not incoming messages from the client
	_DataSet->declareProperty( "AvailableImpulseBitSize", PSOWriteOnly ); // to avoid other FS to receive changes
	_DataSet->declareProperty( "TargetList", PSOReadOnly );
	_DataSet->declareProperty( "VisualFX", PSOReadOnly );	
	_DataSet->declareProperty( "GuildSymbol", PSOReadOnly );
	_DataSet->declareProperty( "GuildNameId", PSOReadOnly );
	_DataSet->declareProperty( "EventFactionId", PSOReadOnly );
	_DataSet->declareProperty( "PvpMode", PSOReadOnly );
	_DataSet->declareProperty( "PvpClan", PSOReadOnly );
	_DataSet->declareProperty( "OwnerPeople", PSOReadOnly );
	_DataSet->declareProperty( "OutpostInfos", PSOReadOnly );

	
	// All will be ready when _Mirror.mirrorIsReady()
	// and _Mirror.propIsAllocated("_EId_fe_temp")
	// and _Mirror.propIsAllocated(all the properties above)

	// Set the dataset
	CEntity::DataSet = _DataSet;

	// Map the property indices
	//_VisualPropIndexToDataSet.resize( _DataSet->nbProperties() );
	DSPropertyPOSX = mapVisualPropIndex( "X", PROPERTY_POSX );
	DSPropertyPOSY = mapVisualPropIndex( "Y", PROPERTY_POSY );
	DSPropertyPOSZ = mapVisualPropIndex( "Z", PROPERTY_POSZ );
	DSPropertyORIENTATION = mapVisualPropIndex( "Theta", PROPERTY_ORIENTATION );
	DSPropertySHEET = mapVisualPropIndex( "Sheet", PROPERTY_SHEET );
	DSPropertyBEHAVIOUR = mapVisualPropIndex( "Behaviour", PROPERTY_BEHAVIOUR );
	DSPropertyNAME_STRING_ID = mapVisualPropIndex( "NameIndex", PROPERTY_NAME_STRING_ID );
	DSPropertyTARGET_ID = mapVisualPropIndex( "Target", PROPERTY_TARGET_ID );
	DSPropertyMODE = mapVisualPropIndex( "Mode", PROPERTY_MODE );
	DSPropertyVPA = mapVisualPropIndex( "VisualPropertyA", PROPERTY_VPA );
	DSPropertyVPB = mapVisualPropIndex( "VisualPropertyB", PROPERTY_VPB );
	DSPropertyVPC = mapVisualPropIndex( "VisualPropertyC", PROPERTY_VPC );
	DSPropertyENTITY_MOUNTED_ID = mapVisualPropIndex( "EntityMounted", PROPERTY_ENTITY_MOUNTED_ID );
	DSPropertyRIDER_ENTITY_ID = mapVisualPropIndex( "RiderEntity", PROPERTY_RIDER_ENTITY_ID );
	DSPropertyCONTEXTUAL = mapVisualPropIndex( "ContextualProperty", PROPERTY_CONTEXTUAL );
	DSPropertyBARS = mapVisualPropIndex( "Bars", PROPERTY_BARS );
	DSPropertyTickPos = getPropertyIndex( "TickPos" );
	DSPropertyLocalX = getPropertyIndex( "LocalX" );
	DSPropertyLocalY = getPropertyIndex( "LocalY" );
	DSPropertyLocalZ = getPropertyIndex( "LocalZ" );
	//DSPropertyStunned = getPropertyIndex( "Stunned" );
	DSFirstPropertyAvailableImpulseBitSize = getPropertyIndex( "AvailableImpulseBitSize" );
	DSPropertyTARGET_LIST = mapVisualPropIndex( "TargetList", PROPERTY_TARGET_LIST );
	DSPropertyVISUAL_FX = mapVisualPropIndex( "VisualFX", PROPERTY_VISUAL_FX );
	DSPropertyGUILD_SYMBOL = mapVisualPropIndex( "GuildSymbol", PROPERTY_GUILD_SYMBOL );
	DSPropertyGUILD_NAME_ID = mapVisualPropIndex( "GuildNameId", PROPERTY_GUILD_NAME_ID );
	DSPropertyEVENT_FACTION_ID = mapVisualPropIndex( "EventFactionId", PROPERTY_EVENT_FACTION_ID );
	DSPropertyPVP_MODE = mapVisualPropIndex( "PvpMode", PROPERTY_PVP_MODE );
	DSPropertyPVP_CLAN = mapVisualPropIndex( "PvpClan", PROPERTY_PVP_CLAN );
	DSPropertyOWNER_PEOPLE = mapVisualPropIndex( "OwnerPeople", PROPERTY_OWNER_PEOPLE );
	DSPropertyOUTPOST_INFOS = mapVisualPropIndex( "OutpostInfos", PROPERTY_OUTPOST_INFOS );
	DSPropertyNPC_ALIAS = getPropertyIndex( "NPCAlias" );

	// Resize the entity container
	_Entities.resize( _DataSet->maxNbRows() );
	EntityToClient->init( _DataSet->maxNbRows() );

	// Set the display callbacks (because not using initRyzomVisualPropertyIndices())
	_DataSet->setDisplayCallback( DSPropertyBEHAVIOUR, BehaviourToStringCb );
	_DataSet->setDisplayCallback( DSPropertyMODE, ModeToStringCb );

	_Mirror.setNotificationCallback( cbNotificationCallback );
}


/*
 * Set the mapping and return the dataset property index
 */
TPropertyIndex CEntityContainer::mapVisualPropIndex( const std::string& propName, TPropIndex vPropIndex )
{
	nlassert ( vPropIndex < CLFECOMMON::NB_VISUAL_PROPERTIES );
	_VisualPropIndexToDataSet[vPropIndex] = _DataSet->getPropertyIndex( propName );
	if ( _VisualPropIndexToDataSet[vPropIndex] == INVALID_PROPERTY_INDEX )
		nlerror( "ERROR: Property %s not found in dataset", propName.c_str() );
	return _VisualPropIndexToDataSet[vPropIndex];
}


/*
 * Return the property index at startup
 */
TPropertyIndex CEntityContainer::getPropertyIndex( const std::string& propName ) const
{
	TPropertyIndex propertyIndex = _DataSet->getPropertyIndex( propName );
	if ( propertyIndex == INVALID_PROPERTY_INDEX )
		nlerror( "ERROR: Property %s not found in dataset", propName.c_str() );
	return propertyIndex;
}


/*uint32 NbcX, NbcY;
NLMISC_VARIABLE( uint32, NbcX, "" );
NLMISC_VARIABLE( uint32, NbcY, "" );
CValueSmootherTemplate<uint32> Nbcx(20);
CValueSmootherTemplate<uint32> Nbcy(20);

NLMISC_DYNVARIABLE( uint32, NbcX, "" )
{
	if ( get )
	{
		*pointer = Nbcx.getSmoothValue();
	}
}

NLMISC_DYNVARIABLE( uint32, NbcY, "" )
{
	if ( get )
	{
		*pointer = Nbcy.getSmoothValue();
	}
}*/


/*
 * Update (called every cycle by the mirror system)
 */
void CEntityContainer::updateMirror()
{
	//nldebug( "%u: updateMirror", CTickEventHandler::getGameCycle() );
	H_AUTO(UpdateEntities)

	// Process added entities
	_DataSet->beginAddedEntities();
	TDataSetRow entityIndex = _DataSet->getNextAddedEntity();
	while ( entityIndex != LAST_CHANGED )
	{
		// The front-end is not aware of the entity index. Use the CEntityId to map it.
		// This is done here because the vision provider needs it when scanning the changed properties
		// of a client to send the discrete ones to himself. Note: MapSidToUid for this client
		// must be called before (receiving CL_ID from the EGS)
		const CEntityId& entityId = _DataSet->getEntityId( entityIndex );
		TClientId clientid = EntityToClient->getClientId( entityId );
		if ( clientid != INVALID_CLIENT )
		{
			CClientHost *clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid];
//#ifdef NL_DEBUG
			if ( clienthost )
			{
//#endif
				clienthost->setEntityIndex( entityIndex );
				EntityToClient->addEntityIndex( entityIndex, clientid );
				nlinfo( "%u: Setting mirror row to C%hu uid %u E%u", CTickEventHandler::getGameCycle(), clientid, clienthost->Uid, entityIndex.getIndex() );
				CFrontEndService::instance()->receiveSub()->ConnectionStatLog->displayNL( "Setting mirror row to C%hu uid %u E%u", clientid, clienthost->Uid, entityIndex.getIndex() );
				//nldebug( "dataset %s, maxNbRows %d", _DataSet->name().c_str(), _DataSet->maxNbRows() );
//#ifdef NL_DEBUG
			}
			else
			{
				// Should not occur
				nlwarning( "No client corresponding to C%hu E%u", clientid, entityIndex.getIndex() );
			}
//#endif
		}

		// Init entity structure
		_Entities[entityIndex.getIndex()].initFields( entityIndex, entityId );

		/*
		// TEMP workaround for vision with entityIds (not entityIndices)
		pair<TMapOfEarlyVisionAssociations::iterator, TMapOfEarlyVisionAssociations::iterator> range = EarlyVisionIns.equal_range( entityId );
		if ( range.first != EarlyVisionIns.end() )
		{
			TMapOfEarlyVisionAssociations::iterator ite; 
			for ( ite=range.first; ite!=range.second; ++ite )
			{
				// Reinsert in vision now we know the entityIndex (works because VisionIn not cleared before next processVision)
				nldebug( "Reinserting %s into vision of E%u", entityId.toString().c_str(), (*ite).second.first.getIndex()  );
				_Entities[(*ite).second.first.getIndex()].insertIntoVisionIn( (*ite).second.first, entityId, (*ite).second.second.first, (*ite).second.second.second );
			}
			EarlyVisionIns.erase( range.first, range.second );
		}
		*/

		// Get next
		entityIndex = _DataSet->getNextAddedEntity();
	}
	_DataSet->endAddedEntities();

	// Process removed entities
	_DataSet->beginRemovedEntities();
	CEntityId *entityId;
	entityIndex = _DataSet->getNextRemovedEntity( &entityId );
	while ( entityIndex != LAST_CHANGED )
	{
		// Check if the entity is a client on this FE (unspawning)
		TClientId clientid = EntityToClient->getClientId( entityIndex );
		if ( clientid != INVALID_CLIENT )
		{
			// Unset the entity index
			LOG_VISION( "FEVIS:%u: Removing entity index %u for client %hu (%s)", CTickEventHandler::getGameCycle(), entityIndex.getIndex(), clientid, entityId->toString().c_str() );
			EntityToClient->removeEntityIndex( entityIndex );
			CClientHost *clienthost = CFrontEndService::instance()->sendSub()->clientIdCont()[clientid];
			if ( clienthost )
			{
				clienthost->setEntityIndex( TEntityIndex() );

				// Reset the vision of the client
				CFrontEndService::instance()->PrioSub.VisionProvider.resetVision( clienthost );
			}

			// Set properties to uninitialized state
			_Entities[entityIndex.getIndex()].invalidateProperties();
		}
		entityIndex = _DataSet->getNextRemovedEntity( &entityId );
	}
	_DataSet->endRemovedEntities();

	//uint nbx, nby = 0;
	//H_BEFORE(ScanPosXY);
	// Compute mileages with X and Y		TODO: mileage computation may be useless now
	//H_BEFORE(beginChangeX)
	_DataSet->beginChangedValuesForProp( DSPropertyPOSX );
	//H_AFTER(beginChangeX)
	while( (entityIndex = _DataSet->getNextChangedValueForProp()) != LAST_CHANGED )
	{
		//H_BEFORE(InsideWhileX)
		_Entities[entityIndex.getIndex()].updateMileageX( entityIndex );
		//++nbx;
		//H_AFTER(InsideWhileX)
	}
	//H_BEFORE(endChangeX)
	_DataSet->endChangedValuesForProp();
	//H_AFTER(endChangeX)
	//H_BEFORE(beginChangeY)
	_DataSet->beginChangedValuesForProp( DSPropertyPOSY );
	//H_AFTER(beginChangeY)
	while( (entityIndex = _DataSet->getNextChangedValueForProp()) != LAST_CHANGED )
	{
		//H_BEFORE(InsideWhileY)
		_Entities[entityIndex.getIndex()].updateMileageY( entityIndex );
		//++nby;
		//H_AFTER(InsideWhileY)
	}
	//H_BEFORE(endChangeY)
	_DataSet->endChangedValuesForProp();
	//H_AFTER(endChangeY)
	//H_AFTER(ScanPosXY);
	//Nbcx.addValue( nbx );
	//Nbcy.addValue( nby );
}


/*
 * Insert entityId, slot into VisionIn 
 */
//void CEntity::insertIntoVisionIn( TEntityIndex iviewer, const NLMISC::CEntityId& entityId, CLFECOMMON::TCLEntityId slot, bool replace )
void CEntity::insertIntoVisionIn( const TEntityIndex& iviewer, const TDataSetRow& entityIndex, CLFECOMMON::TCLEntityId slot, bool replace )
{
	// Find the index of the id
	//TEntityIndex iviewed = TheEntityContainer->entityIdToIndex( entityId );
	const TEntityIndex& iviewed = entityIndex;

	if ( !iviewed.isValid() )
	{
		//nldebug( "New viewed entity id %s not in mirror yet, storing for viewer E%u", entityId.toString().c_str(), iviewer.getIndex() );
		nlwarning( "Received invalid entity index for vision of E%u", iviewed.getIndex() );
		// TEMP Workaround (until GPMS vision uses entityindices)
		// removed, entityIndex will always be valid (according to Olivier)
		//CEntityContainer::I->EarlyVisionIns.insert( std::make_pair( entityId, std::make_pair( iviewer, std::make_pair( slot, replace ) ) ) );
		return;
	}
	//nlinfo( "*  Add E%u to VisionIn", iviewed );
	VisionIn.insert( std::make_pair( iviewed, std::make_pair( slot, replace ) ) );
}


// Property value
#define PV(propIndex) properties[propIndex]

#define dispDist(s,t) s += string(" \t - ") + ((dist<t) ? string("IN") : string("OUT")) + string(" RADIUS ") + toString( "(%g m)", t/1000.0f )


/*
 *
 */
void CEntity::displayProperties( const TEntityIndex& entityIndex, NLMISC::CLog *log, TClientId optClientId, CLFECOMMON::TCLEntityId optSlot ) const
{
	CMirrorPropValueRO<float> mmm( TheDataset, entityIndex, DSPropertyORIENTATION );

	uint64 properties[NB_VISUAL_PROPERTIES];
	fillVisualPropertiesFromMirror( properties, entityIndex );

	string lsTheta, lsSheet, lsBehav, lsName, lsTarget, lsMode, lsVPA, lsVPB, lsVPC, lsMount, lsRider, lsContextual, lsBars, lsEventFactionId, lsPvpMode, lsPvpClan, lsOwnerPeople, lsOutpostInfos;
	string sdist;
	if ( (optClientId != INVALID_CLIENT) && (optSlot != INVALID_SLOT) )
	{
		TPairState& pairState = CFrontEndService::instance()->PrioSub.VisionArray.getPairState( optClientId, optSlot );
		TCoord dist = pairState.DistanceCE;
		sdist = toString( " Dist %g m", ((float)dist)/1000.0f );

		CHistory *history = CFrontEndService::instance()->history();
		bool histohasvalue;
		TPropertiesValue lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_ORIENTATION, histohasvalue ).LastSent;
		lsTheta = histohasvalue ? toString( "%.2f", *(float*)&lastsent_value ) : "NOT SENT YET";
		dispDist(lsTheta,THRESHOLD_ORIENTATION);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_SHEET, histohasvalue ).LastSent;
		lsSheet = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsSheet,THRESHOLD_SHEET);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_BEHAVIOUR, histohasvalue ).LastSent;
		lsBehav = histohasvalue ? toString( "%"NL_I64"u", (uint64)lastsent_value ) : "NOT SENT YET";
		dispDist(lsBehav,THRESHOLD_BEHAVIOUR);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_NAME_STRING_ID, histohasvalue ).LastSent;
		lsName = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsName,THRESHOLD_NAME_STRING_ID);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_TARGET_ID, histohasvalue ).LastSent;
		lsTarget = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsTarget,THRESHOLD_TARGET_ID);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_MODE, histohasvalue ).LastSent;
		lsMode = histohasvalue ? toString( "%hu", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsMode,THRESHOLD_MODE);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_VPA, histohasvalue ).LastSent;
		lsVPA = histohasvalue ? toString( "%"NL_I64"u", lastsent_value ) : "NOT SENT YET";
		dispDist(lsVPA,THRESHOLD_VPA);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_VPB, histohasvalue ).LastSent;
		lsVPB = histohasvalue ? toString( "%"NL_I64"u", lastsent_value ) : "NOT SENT YET";
		dispDist(lsVPB,THRESHOLD_VPB);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_VPC, histohasvalue ).LastSent;
		lsVPC = histohasvalue ? toString( "%"NL_I64"u", lastsent_value ) : "NOT SENT YET";
		dispDist(lsVPC,THRESHOLD_VPC);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_ENTITY_MOUNTED_ID, histohasvalue ).LastSent;
		lsMount = histohasvalue ? toString( "%d", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsMount,THRESHOLD_ENTITY_MOUNTED_ID);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_RIDER_ENTITY_ID, histohasvalue ).LastSent;
		lsRider = histohasvalue ? toString( "%d", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsRider,THRESHOLD_RIDER_ENTITY_ID);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_CONTEXTUAL, histohasvalue ).LastSent;
		lsContextual = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsContextual,THRESHOLD_CONTEXTUAL);
		lsBars = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsBars,THRESHOLD_BARS);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_EVENT_FACTION_ID, histohasvalue ).LastSent;
		lsEventFactionId = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsEventFactionId,THRESHOLD_EVENT_FACTION_ID);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_PVP_MODE, histohasvalue ).LastSent;
		lsEventFactionId = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsPvpMode,THRESHOLD_PVP_MODE);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_PVP_CLAN, histohasvalue ).LastSent;
		lsPvpClan = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsPvpClan,THRESHOLD_PVP_CLAN);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_OWNER_PEOPLE, histohasvalue ).LastSent;
		lsOwnerPeople = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsOwnerPeople,PROPERTY_OWNER_PEOPLE);
		lastsent_value = history->getPropertyEntry( optClientId, optSlot, PROPERTY_OUTPOST_INFOS, histohasvalue ).LastSent;
		lsOutpostInfos = histohasvalue ? toString( "%u", (uint32)lastsent_value ) : "NOT SENT YET";
		dispDist(lsOutpostInfos,PROPERTY_OUTPOST_INFOS);
	}

	CMirrorPropValue<TYPE_SHEET> sheetValue( TheDataset, entityIndex, DSPropertySHEET );
	TGameCycle sheetTimestamp = sheetValue.getTimestamp(), deltaTSheet = CTickEventHandler::getGameCycle()-sheetTimestamp;
	string sheetSet = (sheetTimestamp!=0) ? toString( "set %u ticks (%.3f hours) ago", deltaTSheet, ((float)deltaTSheet)/36000.0f ) : "not set yet";

	log->displayNL( "| E%u: TickPos %u Mileage %u %s", entityIndex.getIndex(), TickPosition(), Mileage, sdist.c_str() );
	log->displayNL( "| Sheet %u \t %s (sheet %s)", (uint32)PV(PROPERTY_SHEET), lsSheet.c_str(), sheetSet.c_str() );
	log->displayNL( "| AbsPos: %d %d %d", (sint32)PV(PROPERTY_POSX), (sint32)PV(PROPERTY_POSY), (sint32)PV(PROPERTY_POSZ) );;
	log->displayNL( "| Theta %.2f \t %s", *(float*)(&PV(PROPERTY_ORIENTATION)), lsTheta.c_str() );
	log->displayNL( "| Behav %u \t %s", (uint32)PV(PROPERTY_BEHAVIOUR), lsBehav.c_str() );
	log->displayNL( "| Name %u \t %s", (uint32)PV(PROPERTY_NAME_STRING_ID), lsName.c_str() );
	TDataSetIndex target = (TDataSetIndex)PV(PROPERTY_TARGET_ID), mount = (TDataSetIndex)PV(PROPERTY_ENTITY_MOUNTED_ID), rider = (TDataSetIndex)PV(PROPERTY_RIDER_ENTITY_ID);
	log->displayNL( "| Target %d \t %s", target, lsTarget.c_str() );
	log->displayNL( "| Mode %hu \t %s", (uint8)PV(PROPERTY_MODE), lsMode.c_str() );
	log->displayNL( "| VPA %"NL_I64"u \t %s", PV(PROPERTY_VPA), lsVPA.c_str() );
	log->displayNL( "| VPB %"NL_I64"u \t %s", PV(PROPERTY_VPB), lsVPB.c_str() );
	log->displayNL( "| VPC %"NL_I64"u \t %s", PV(PROPERTY_VPC), lsVPC.c_str() );
	log->displayNL( "| Mount %d \t %s", mount, lsMount.c_str() );
	log->displayNL( "| Rider %d \t \t %s", rider, lsRider.c_str() );
	log->displayNL( "| Contextual %hu \t %s", (uint16)PV(PROPERTY_CONTEXTUAL), lsContextual.c_str() );
	log->displayNL( "| EventFactionId %u \t %s", (uint32)PV(PROPERTY_EVENT_FACTION_ID), lsEventFactionId.c_str() );
	log->displayNL( "| PvpMode %u \t %s", (uint32)PV(PROPERTY_PVP_MODE), lsPvpMode.c_str() );
	log->displayNL( "| PvpClan %u \t %s", (uint32)PV(PROPERTY_PVP_CLAN), lsPvpClan.c_str() );
	log->displayNL( "| OwnerPeople %u \t %s", (uint32)PV(PROPERTY_OWNER_PEOPLE), lsOwnerPeople.c_str() );
	log->displayNL( "| OutpostInfos %u \t %s", (uint32)PV(PROPERTY_OUTPOST_INFOS), lsOutpostInfos.c_str() );
}


#define FILL_PROP( name ) \
	{ \
		CMirrorPropValueRO<TYPE_##name> prop( TheDataset, entityIndex, DSProperty##name ); \
		TYPE_##name *pt = (TYPE_##name*)&(properties[PROPERTY_##name]); \
		*pt = prop(); \
	}

void CEntity::fillVisualPropertiesFromMirror( uint64 properties[], const TEntityIndex& entityIndex )
{
	for ( sint i=0; i!=NB_VISUAL_PROPERTIES; ++i )
		properties[i] = 0;
	
	FILL_PROP( POSX );
	FILL_PROP( POSY );
	FILL_PROP( POSZ );
	FILL_PROP( ORIENTATION );
	FILL_PROP( SHEET );
	FILL_PROP( BEHAVIOUR );
	FILL_PROP( NAME_STRING_ID );
	FILL_PROP( TARGET_ID );
	FILL_PROP( MODE );
	FILL_PROP( VPA );
	FILL_PROP( VPB );
	FILL_PROP( VPC );
	FILL_PROP( ENTITY_MOUNTED_ID );
	FILL_PROP( RIDER_ENTITY_ID );
	FILL_PROP( CONTEXTUAL );
	FILL_PROP( GUILD_SYMBOL );
	FILL_PROP( GUILD_NAME_ID );
	FILL_PROP( EVENT_FACTION_ID );
	FILL_PROP( PVP_MODE );
	FILL_PROP( PVP_CLAN );
}


/*
 * Helper for positionIsInitialized()
 */
/*
bool CEntity::checkPositionInitialized( const TEntityIndex& entityIndex )
{
	if ( ! ((X() == 0) && (Y() == 0)) ) // not local pos because can be 0 0
	{
		PropInitialized[PROPERTY_POSITION] = true;
#ifdef NL_DEBUG
		nldebug( "%u: E%u: Position initialized to %d %d", CTickEventHandler::getGameCycle(), entityIndex.getIndex(), X(), Y() );
#endif
		return true;
	}
	else
		return false;
}
*/

