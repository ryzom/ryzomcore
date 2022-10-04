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
#include "nel/net/service.h"
#include "game_share/mode_and_behaviour.h"
#include "game_share/mirror_prop_value.h"
#include "game_share/time_weather_season/time_date_season_manager.h"

#include "egs_mirror.h"
#include "guild_manager/fame_manager.h"
#include "guild_manager/guild_manager.h"

#include "harvest_source.h"

#include <vector>

using namespace std;
using namespace NLNET;


CMirror					Mirror;
CMirroredDataSet		*FeTempDataset = NULL;
CMirroredDataSet		*FameDataset = NULL;
TPropertyIndex			DSFirstPropertyAvailableImpulseBitSize;

// These must be set before being able to receive a callback (used in cbMirrorIsReady)
uint32					MaxNbPlayers, MaxNbObjects, MaxNbNpcSpawnedByEGS, MaxNbForageSources, MaxNbToxicClouds;
uint32					MaxNbGuilds;


/*
 * Initialisation of the mirror
 */
void	initMirror( void (*cbUpdate)(), void (*cbSync)() )
{
	vector<string> dataSetsToLoad;
	dataSetsToLoad.push_back( "fe_temp" );
	dataSetsToLoad.push_back( "fame");
	Mirror.init( dataSetsToLoad, cbMirrorIsReadyForInit, cbUpdate, cbSync );
	Mirror.addCallbackWhenMirrorReadyForUse( cbMirrorReadyForAddEntity );
}

/*
 * Initialisation of the properties in the mirror when the mirror service is up
 */
void	cbMirrorIsReadyForInit( CMirror *mirror )
{
//	Mirror.declareEntityTypeOwner( RYZOMID::day_cycle, 1 ); // One time for server, DayCycle must be have index 0+NB_RESERVED_ROWS in dataset, do not change order of init
//	Mirror.declareEntityTypeOwner( RYZOMID::weather, 5 ); // TEMP max number of continents
	Mirror.declareEntityTypeOwner( RYZOMID::player, MaxNbPlayers );
	Mirror.declareEntityTypeOwner( RYZOMID::object, MaxNbObjects );
	Mirror.declareEntityTypeOwner( RYZOMID::npc, MaxNbNpcSpawnedByEGS );
	Mirror.declareEntityTypeOwner( RYZOMID::forageSource, MaxNbForageSources );
	Mirror.declareEntityTypeOwner( RYZOMID::fx_entity, MaxNbToxicClouds );

	// Fame dataset
	Mirror.declareEntityTypeOwner( RYZOMID::civilisation,	10	);		// max number of civilisation
	Mirror.declareEntityTypeOwner( RYZOMID::guild,			MaxNbGuilds	);	// max number of guild (total number of unified guilds)
	Mirror.declareEntityTypeOwner( RYZOMID::fame_memory,	MaxNbPlayers	);	// max number of fame_memory (one by connected player)

	// Properties
	FeTempDataset = &(mirror->getDataSet( "fe_temp" ));
	FeTempDataset->declareProperty( "X", PSOReadWrite );
	FeTempDataset->declareProperty( "Y", PSOReadWrite );
	FeTempDataset->declareProperty( "Z", PSOReadWrite | PSONotifyChanges );
	FeTempDataset->declareProperty( "AIInstance", PSOReadWrite );
	FeTempDataset->declareProperty( "Theta", PSOReadWrite );
	FeTempDataset->declareProperty( "Cell", PSOReadWrite );
	FeTempDataset->declareProperty( "Sheet", PSOReadWrite );
	FeTempDataset->declareProperty( "SheetServer", PSOReadWrite );
	FeTempDataset->declareProperty( "Behaviour", PSOReadWrite );
	FeTempDataset->declareProperty( "NameIndex", PSOReadWrite | PSONotifyChanges ); // set by IOS, by EGS only for forage
	FeTempDataset->declareProperty( "Target", PSOReadWrite );
	FeTempDataset->declareProperty( "Mode", PSOReadWrite );
	FeTempDataset->declareProperty( "VisualPropertyA", PSOReadWrite );
	FeTempDataset->declareProperty( "VisualPropertyB", PSOReadWrite );
	FeTempDataset->declareProperty( "VisualPropertyC", PSOReadWrite );
	FeTempDataset->declareProperty( "EntityMounted", PSOReadWrite );
	FeTempDataset->declareProperty( "RiderEntity", PSOReadWrite );
	FeTempDataset->declareProperty( "ContextualProperty", PSOReadWrite );
	FeTempDataset->declareProperty( "AvailableImpulseBitSize", PSOReadOnly ); // set by FS
	FeTempDataset->declareProperty( "CurrentHitPoints", PSOReadWrite );
	FeTempDataset->declareProperty( "MaxHitPoints", PSOReadWrite );
	FeTempDataset->declareProperty( "BestRole", PSOReadWrite );
	FeTempDataset->declareProperty( "BestRoleLevel", PSOReadWrite );
	FeTempDataset->declareProperty( "CurrentRunSpeed", PSOReadWrite );
	FeTempDataset->declareProperty( "CurrentWalkSpeed", PSOReadWrite );
	FeTempDataset->declareProperty( "Stunned", PSOReadWrite );
	FeTempDataset->declareProperty( "CombatState", PSOReadOnly );
//	FeTempDataset->declareProperty( "KamiFame", PSOReadWrite );
//	FeTempDataset->declareProperty( "KaravanFame", PSOReadWrite );
	FeTempDataset->declareProperty( "WhoSeesMe", PSOReadWrite );
	FeTempDataset->declareProperty( "Bars", PSOReadWrite );
	FeTempDataset->declareProperty( "TeamId", PSOReadWrite );
	FeTempDataset->declareProperty( "ActionFlags", PSOReadWrite );

	FeTempDataset->declareProperty( "GuildNameId", PSOReadWrite );
	FeTempDataset->declareProperty( "GuildSymbol", PSOReadWrite );

	FeTempDataset->declareProperty( "InOutpostZoneAlias", PSOReadWrite );
	FeTempDataset->declareProperty( "InOutpostZoneSide", PSOReadWrite );

	FeTempDataset->declareProperty( "PvpMode", PSOReadWrite );
	FeTempDataset->declareProperty( "PvpClan", PSOReadWrite );

	// This is the only service setting a target list. If it was not, some additional mutual exclusion were required when writing this property.
	// Note: this property requires to push:
	// 1) The distance between the source and the target (unit: 1/127th of 100 meters)
	// 2) The target TDataSetRow
	FeTempDataset->declareProperty( "TargetList", PSOReadWrite );

	FeTempDataset->declareProperty( "VisualFX", PSOReadWrite );	
	FeTempDataset->declareProperty( "Fuel", PSOReadWrite ); // written by AIS (follow mode) or GPMS (mount mode)

	FeTempDataset->declareProperty( "OwnerPeople", PSOReadWrite );

	FeTempDataset->declareProperty( "OutpostInfos", PSOReadWrite );

	initRyzomVisualPropertyIndices( *FeTempDataset );

	DSFirstPropertyAvailableImpulseBitSize = FeTempDataset->getPropertyIndex( "AvailableImpulseBitSize" );

	// init the fame dataset
	FameDataset = &(mirror->getDataSet( "fame" ));
	CFameManager::getInstance().mirrorIsReady();

	Mirror.setNotificationCallback( processMirrorUpdates );
}


/*
 * Clear any target lists from previous uncleared sessions.
 * The problem comes from the following facts
 * 1) The AIS manage the spawning/despawning of creatures & NPC,
 *    but they don't declare the property "TargetList"
 * 2) The EGS pushes some data in the target lists
 *    but does not always clear them after use (i.e. in magic actions).
 * The target list of an entity is eventually cleared at least:
 * - When a player row is reassigned (by the mirror in addEntityToDataSet())
 * - When a creature/NPC is despawned (by the EGS in processMirrorUpdates())
 * - When the EGS starts (by the following code). Reason: if the EGS is
 *   shutdown while there are still online entities (AIS not closed), and the
 *   MS is not closed, the target lists of the creatures/NPCs will remain.
 */
void cleanOrphanTargetLists( NLNET::TServiceId serviceId )
{
	const TDeclaredEntityRangeOfType& declERT = TheDataset.getDeclaredEntityRanges();

	uint nbCleanedTargetLists = 0, nbCleanedCells = 0;
	sint nbPreviousKnownCells = CMirroredDataSet::getNbKnownSharedListCells();
	const uint NbTypesOfNotSpawnedByEGS = 2;
	RYZOMID::TTypeId typesOfNotSpawnedByEGS [NbTypesOfNotSpawnedByEGS] = { RYZOMID::npc, RYZOMID::creature };
	for ( uint i=0; i!=NbTypesOfNotSpawnedByEGS; ++i )
	{
		// Get all ranges of the selected entity type
		pair<TDeclaredEntityRangeOfType::const_iterator,TDeclaredEntityRangeOfType::const_iterator>
			itPair = declERT.equal_range( typesOfNotSpawnedByEGS[i] );
		for ( TDeclaredEntityRangeOfType::const_iterator it=itPair.first; it!=itPair.second; ++it )
		{
			// If the range belongs to the connecting service, scan for its orphan target lists and clear them
			const TDeclaredEntityRange& range = GET_ENTITY_TYPE_RANGE(it);
			if ( range.serviceId() != serviceId )
				continue;
			for ( TDataSetIndex entityIndex=range.baseIndex(); entityIndex<range.baseIndex()+range.size(); ++entityIndex )
			{
				CMirrorPropValueList<uint32> targetList( TheDataset, TheDataset.forceCurrentDataSetRow( entityIndex ), DSPropertyTARGET_LIST );
				if ( ! targetList.empty() )
				{
					uint nbCells = targetList.size();
					CMirroredDataSet::reportOrphanSharedListCells( (sint)nbCells );
					targetList.clear();
					//nldebug( "Orphan list for E%u", entityIndex );
					++nbCleanedTargetLists;
					nbCleanedCells += nbCells;
				}
			}
		}
	}
	nlinfo( "%u target lists from previous session cleaned at connection of %s (%u cells)",
		nbCleanedTargetLists, CUnifiedNetwork::getInstance()->getServiceUnifiedName( serviceId ).c_str(),
		nbCleanedCells );
	nlassert( CMirroredDataSet::getNbKnownSharedListCells() == nbPreviousKnownCells );
}


/*
 * Callback called when mirror is ready for manage entity
 */
void cbMirrorReadyForAddEntity( CMirror *mirror )
{
	CFameManager::getInstance().mirrorReadyToAdd();

	// Clean orphan target lists that could remain if the EGS crashed (or was stopped with remaining player) in a previous session
	cleanOrphanTargetLists( IService::getInstance()->getServiceId() );

	// Init harvest source manager
	const TDeclaredEntityRangeOfType& declERT = TheDataset.getDeclaredEntityRanges();
	{
		TDeclaredEntityRangeOfType::const_iterator it = declERT.find( RYZOMID::forageSource );
		//nlassert( it != declERT.end() );
		const TDeclaredEntityRange& range = GET_ENTITY_TYPE_RANGE(it);
		(new CHarvestSourceManager())->init( range.baseIndex(), range.size() );
	}
	{
		/************************************************************************/
		/* Add here all types managed as environmental effects
		/************************************************************************/
		TDeclaredEntityRangeOfType::const_iterator it = declERT.find( RYZOMID::fx_entity );
		nlassert( it != declERT.end() );
		const TDeclaredEntityRange& range = GET_ENTITY_TYPE_RANGE(it);
		(new CEnvironmentalEffectManager())->init( range.baseIndex(), range.size() );
	}

	// we can load the guild 
	CGuildManager::getInstance()->loadGuilds();
}




/*
 * Set mode and fill pos from mirror and compress
 */
void	MBEHAV::TMode::setModeAndPos( EMode mode, const TDataSetRow& entityIndex )
{
	Mode = mode;
	CMirrorPropValueRO<TYPE_POSX> propX( TheDataset, entityIndex, DSPropertyPOSX );
	Pos.X16 = (uint16)(propX() >> 4);
	CMirrorPropValueRO<TYPE_POSY> propY( TheDataset, entityIndex, DSPropertyPOSY );
	Pos.Y16 = (uint16)(propY() >> 4);
	//nldebug( "Setting MODE %s for E%u with current pos %d, %d", modeToString( mode ).c_str(), entityIndex.getIndex(), propX(), propY() );
}


