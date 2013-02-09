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
#include "nel/net/unified_network.h"
#include "nel/net/service.h"

#include "guild_manager/fame_manager.h"
#include "egs_mirror.h"
#include "player_manager/character.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "guild_manager/guild.h"
#include "guild_manager/guild_manager.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

CFameManager	*CFameManager::_Instance = NULL;

extern			CMirror						Mirror;

NLMISC::TInstanceCounterData CFameManager::TFameOwnerWrite::TFameOwnerWriteInstanceCounter::_InstanceCounterData("TFameOwnerWrite");

sint16	CFameManager::TFameOwnerWrite::CivilisationPropIndex = 0;
sint16	CFameManager::TFameOwnerWrite::GuildPropIndex = 0;
sint16	CFameManager::TFameOwnerWrite::FameMemoryPropIndex = 0;
sint16	CFameManager::TFameOwnerWrite::FirstFamePropIndex = 0;

// Definitions for Fame lookup arrays.
// Neutrals are always row 0
#define kNeutral 0
// This is the adjustment for cult position in the lookup table, starting at 1
#define kBeginCults1 (PVP_CLAN::BeginCults - 1)
// This is the adjustment for civilisation position in the lookup table, starting at 1
#define kBeginCivs1 (PVP_CLAN::BeginCivs - 1)

/*
void CFameContainer::serial(NLMISC::IStream &f)
{
	CStaticFames &sf = CStaticFames::getInstance();

	uint version = f.serialVersion(getCurrentVersion());

	if (version >= 2)
	{
		f.serial(_LastGuildStatusChange);
		f.serial(_LastFameChangeDate);

		if (f.isReading())
		{
			uint32 size;
			f.serial(size);
			for (uint i=0; i<size; ++i)
			{
				string factionName;
				f.serial(factionName);
				if (!factionName.empty())
				{
					uint32 factionIndex = sf.getFactionIndex(factionName);
					if( factionIndex != CStaticFames::INVALID_FACTION_INDEX)
					{
						_FactionInfos[factionIndex].serial(f);
					}
					else
					{
						TFameContainerEntry foo;
						foo.serial(f);
					}
				}
			}
		}
		else
		{
			uint32 size = _FactionInfos.size();
			f.serial(size);
			
			std::map<uint32, TFameContainerEntry>::iterator first(_FactionInfos.begin()), last(_FactionInfos.end());
			for (; first != last; ++first)
			{
				string factionName;
				factionName = sf.getFactionName(first->first);
				f.serial(factionName);
				if (!factionName.empty())
				{
					f.serial(first->second);
				}
			}
		}
	}
	else
	{	
		// serial old fame save
		vector<sint32>		fames;
		vector<sint32>		memories;
		vector<EGSPD::CFameTrend::TFameTrend>	trends;
		f.serialCont(fames);
		f.serialCont(memories);
		f.serial(_LastGuildStatusChange);

		if (version >= 1)
		{
			f.serial(_LastFameChangeDate);
			uint32 size = trends.size();
			f.serial(size);
			trends.resize(size);
			for (uint i=0; i<size; ++i)
				f.serialEnum(trends[i]);
		}

		// rebuild the new data structures
		_FactionInfos.clear();
		uint32 size = max(trends.size(), max(fames.size(), memories.size()));

		for (uint i=0; i<size; ++i)
		{
			TFameContainerEntry entry;
			if (i < fames.size())
				entry.Fame = fames[i];
			if (i < memories.size())
				entry.FameMemory = memories[i];
			if (i< trends.size())
				entry.LastFameChangeTrend = trends[i];

			_FactionInfos.insert(make_pair(i, entry));
		}
	}
}

uint32 CFameContainer::getCurrentVersion()
{
	return 2;
}
*/

CFameManager::CFameManager()
{
	//array of callback items
	NLNET::TUnifiedCallbackItem _cbArray[] =
	{
		{ "FAME_DELTA",				cbFameDelta		}, 
	};
	
	// register call back for fame manager
	CUnifiedNetwork::getInstance()->addCallbackArray( _cbArray, sizeof(_cbArray) / sizeof(_cbArray[0]) );

	{
		// Restrict namespace use to this block.
		using namespace PVP_CLAN;

		// Initialize our tables that store fame values, references to the CVariables in egs_variables.h.
		// - Initialize starting values.
		//   "None" and "neutral" factions are impossible, not part of the table.
		FameStart[Fyros-BeginCivs][Kami-BeginClans] = &FameStartFyrosvKami;
		FameStart[Fyros-BeginCivs][Karavan-BeginClans] = &FameStartFyrosvKaravan;
		FameStart[Fyros-BeginCivs][Fyros-BeginClans] = &FameStartFyrosvFyros;
		FameStart[Fyros-BeginCivs][Matis-BeginClans] = &FameStartFyrosvMatis;
		FameStart[Fyros-BeginCivs][Tryker-BeginClans] = &FameStartFyrosvTryker;
		FameStart[Fyros-BeginCivs][Zorai-BeginClans] = &FameStartFyrosvZorai;
		FameStart[Matis-BeginCivs][Kami-BeginClans] = &FameStartMatisvKami;
		FameStart[Matis-BeginCivs][Karavan-BeginClans] = &FameStartMatisvKaravan;
		FameStart[Matis-BeginCivs][Fyros-BeginClans] = &FameStartMatisvFyros;
		FameStart[Matis-BeginCivs][Matis-BeginClans] = &FameStartMatisvMatis;
		FameStart[Matis-BeginCivs][Tryker-BeginClans] = &FameStartMatisvTryker;
		FameStart[Matis-BeginCivs][Zorai-BeginClans] = &FameStartMatisvZorai;
		FameStart[Tryker-BeginCivs][Kami-BeginClans] = &FameStartTrykervKami;
		FameStart[Tryker-BeginCivs][Karavan-BeginClans] = &FameStartTrykervKaravan;
		FameStart[Tryker-BeginCivs][Fyros-BeginClans] = &FameStartTrykervFyros;
		FameStart[Tryker-BeginCivs][Matis-BeginClans] = &FameStartTrykervMatis;
		FameStart[Tryker-BeginCivs][Tryker-BeginClans] = &FameStartTrykervTryker;
		FameStart[Tryker-BeginCivs][Zorai-BeginClans] = &FameStartTrykervZorai;
		FameStart[Zorai-BeginCivs][Kami-BeginClans] = &FameStartZoraivKami;
		FameStart[Zorai-BeginCivs][Karavan-BeginClans] = &FameStartZoraivKaravan;
		FameStart[Zorai-BeginCivs][Fyros-BeginClans] = &FameStartZoraivFyros;
		FameStart[Zorai-BeginCivs][Matis-BeginClans] = &FameStartZoraivMatis;
		FameStart[Zorai-BeginCivs][Tryker-BeginClans] = &FameStartZoraivTryker;
		FameStart[Zorai-BeginCivs][Zorai-BeginClans] = &FameStartZoraivZorai;

		// Initialize Max values.
		// - Cults
		// NOTE: Row 0 is for Neutral, so cults for rows start at 1, but start at 0 for columns.
		FameMaxCults[kNeutral][Kami-BeginCults] = &FameMaxNeutralvKami;
		FameMaxCults[kNeutral][Karavan-BeginCults] = &FameMaxNeutralvKaravan;
		FameMaxCults[Kami-kBeginCults1][Kami-BeginCults] = &FameMaxKamivKami;
		FameMaxCults[Kami-kBeginCults1][Karavan-BeginCults] = &FameMaxKamivKaravan;
		FameMaxCults[Karavan-kBeginCults1][Kami-BeginCults] = &FameMaxKaravanvKami;
		FameMaxCults[Karavan-kBeginCults1][Karavan-BeginCults] = &FameMaxKaravanvKaravan;

		// - Civilisations
		// NOTE: Row 0 is for Neutral, so civss for rows start at 1, but start at 0 for columns.
		FameMaxCivs[kNeutral][Fyros-BeginCivs] = &FameMaxNeutralvFyros;
		FameMaxCivs[kNeutral][Matis-BeginCivs] = &FameMaxNeutralvMatis;
		FameMaxCivs[kNeutral][Tryker-BeginCivs] = &FameMaxNeutralvTryker;
		FameMaxCivs[kNeutral][Zorai-BeginCivs] = &FameMaxNeutralvZorai;
		FameMaxCivs[Fyros-kBeginCivs1][Fyros-BeginCivs] = &FameMaxFyrosvFyros;
		FameMaxCivs[Fyros-kBeginCivs1][Matis-BeginCivs] = &FameMaxFyrosvMatis;
		FameMaxCivs[Fyros-kBeginCivs1][Tryker-BeginCivs] = &FameMaxFyrosvTryker;
		FameMaxCivs[Fyros-kBeginCivs1][Zorai-BeginCivs] = &FameMaxFyrosvZorai;
		FameMaxCivs[Matis-kBeginCivs1][Fyros-BeginCivs] = &FameMaxMatisvFyros;
		FameMaxCivs[Matis-kBeginCivs1][Matis-BeginCivs] = &FameMaxMatisvMatis;
		FameMaxCivs[Matis-kBeginCivs1][Tryker-BeginCivs] = &FameMaxMatisvTryker;
		FameMaxCivs[Matis-kBeginCivs1][Zorai-BeginCivs] = &FameMaxMatisvZorai;
		FameMaxCivs[Tryker-kBeginCivs1][Fyros-BeginCivs] = &FameMaxTrykervFyros;
		FameMaxCivs[Tryker-kBeginCivs1][Matis-BeginCivs] = &FameMaxTrykervMatis;
		FameMaxCivs[Tryker-kBeginCivs1][Tryker-BeginCivs] = &FameMaxTrykervTryker;
		FameMaxCivs[Tryker-kBeginCivs1][Zorai-BeginCivs] = &FameMaxTrykervZorai;
		FameMaxCivs[Zorai-kBeginCivs1][Fyros-BeginCivs] = &FameMaxZoraivFyros;
		FameMaxCivs[Zorai-kBeginCivs1][Matis-BeginCivs] = &FameMaxZoraivMatis;
		FameMaxCivs[Zorai-kBeginCivs1][Tryker-BeginCivs] = &FameMaxZoraivTryker;
		FameMaxCivs[Zorai-kBeginCivs1][Zorai-BeginCivs] = &FameMaxZoraivZorai;
	} // End of block for using namespace
}

void CFameManager::mirrorIsReady()
{
	const static std::string civilisation("Civilisation");
	const static std::string guild("Guild");
	const static std::string fameMemory("FameMemory");
	const static std::string firstFame("Fame_0");

	
//	Mirror.declareEntityTypeOwner( RYZOMID::guild,		5000	);		// max number of guild
//	Mirror.declareEntityTypeOwner( RYZOMID::civilisation,		10	);	// max number of civilisation


	CStaticFames &staticFame = CStaticFames::getInstance();

	// declare the fame properties
	TheFameDataset.declareProperty( civilisation, PSOReadWrite );
	TheFameDataset.declareProperty( guild,		PSOReadWrite );
	TheFameDataset.declareProperty( fameMemory,		PSOReadWrite );
	
	for (uint i=0; i<MAX_FACTION; ++i)
	{
		string propName = toString("Fame_%u", i);
		TheFameDataset.declareProperty( propName,		PSOReadWrite );
	}

	// init the property index for fast access
	TFameOwnerWrite::CivilisationPropIndex = TheFameDataset.getPropertyIndex(civilisation);
	TFameOwnerWrite::GuildPropIndex = TheFameDataset.getPropertyIndex(guild);
	TFameOwnerWrite::FameMemoryPropIndex = TheFameDataset.getPropertyIndex(fameMemory);
	TFameOwnerWrite::FirstFamePropIndex = TheFameDataset.getPropertyIndex(firstFame);

	CFameInterface::getInstance().setFameDataSet(&TheFameDataset, false);
	CFameInterface::getInstance().registerFameOverload(this);
}

void CFameManager::mirrorReadyToAdd()
{
	// create the civilisation record in the dataset
	for (uint i=EGSPD::CPeople::Playable; i<EGSPD::CPeople::EndPlayable; ++i)
	{
		CEntityId civId(RYZOMID::civilisation, i);
		Mirror.createAndDeclareEntity(civId);
		TDataSetRow	entityIndex = TheFameDataset.getDataSetRow(civId);

		TFameOwnerWrite	*fow = new TFameOwnerWrite(TheFameDataset, entityIndex);

		// restaure static fame
		CStaticFames &sf = CStaticFames::getInstance();

		uint factionIndex = sf.getFactionIndex(EGSPD::CPeople::toString(EGSPD::CPeople::TPeople(i)));
		const vector<TStringId> &names = sf.getFactionNames();
		for (uint j=0; j<names.size() && names[i] != CStringMapper::emptyId(); ++j)
		{
			fow->Fames[j] = sf.getStaticFameIndexed(factionIndex, j);
		}

		_FamesOwners.insert(make_pair(entityIndex, fow));

		// TODO : restore civilisation fames
	}
}

void CFameManager::addPlayer(const CEntityId &playerId, const EGSPD::CFameContainerPD &fameContainer, EGSPD::CPeople::TPeople civilisation)
{
	TDataSetRow entityIndex = TheFameDataset.getDataSetRow(playerId);
	if (TheFameDataset.isAccessible(entityIndex))
	{
		// 1st, create the fame memory record
		CEntityId memoryId(RYZOMID::fame_memory, playerId.getShortId(), playerId.getCreatorId(), playerId.getDynamicId());
		Mirror.createAndDeclareEntity(memoryId);
		TDataSetRow	memoryIndex = TheFameDataset.getDataSetRow(memoryId);
		TFameOwnerWrite	*fowmem = new TFameOwnerWrite(TheFameDataset, memoryIndex);
		_FamesOwners.insert(make_pair(memoryIndex, fowmem));

		// 2nd, create the player record
		TFameOwnerWrite *fow = new TFameOwnerWrite(TheFameDataset, entityIndex);
		_FamesOwners.insert(make_pair(entityIndex, fow));
		fow->FameMemory = memoryIndex;
		
		// restore the fame value
		for ( map<CSheetId,EGSPD::CFameContainerEntryPD>::const_iterator it = fameContainer.getEntriesBegin(); it != fameContainer.getEntriesEnd(); ++it )
		{
			uint idx = CStaticFames::getInstance().getFactionIndexFromSheet( (*it).second.getSheet() );
			if ( idx == CStaticFames::INVALID_FACTION_INDEX )
				continue;
			fow->Fames[idx] = (*it).second.getFame();
			fowmem->Fames[idx] = (*it).second.getFameMemory();
			fow->LastFameChangeTrends[idx] = (*it).second.getLastFameChangeTrend();
		}

		// Force the clans to have a fame value (for new players with no fame values set).
		PVP_CLAN::TPVPClan playerRace = PVP_CLAN::getClanFromPeople(civilisation);

		uint32 nbFame = CStaticFames::getInstance().getNbFame();
		for (uint32 faction=0; faction<nbFame; ++faction)
		{
			if (fow->Fames[faction] == NO_FAME)
			{
				if (faction < CStaticFames::getInstance().getFirstTribeFameIndex())
					fow->Fames[faction] = getStartFame(playerRace, PVP_CLAN::getClanFromIndex(faction));
			//	else
			//		fow->Fames[faction] = CStaticFames::getInstance().getStaticFameIndexed(PVP_CLAN::getFactionIndex(playerRace), faction);
			}
		}
		
		fow->LastGuildStatusChange = fameContainer.getLastGuildStatusChange();
		/// update the guild memory
		updatePlayerFame(entityIndex);

		// restore the fame trend
		fow->LastFameChangeDate = fameContainer.getLastFameChangeDate();

		updateFameTrend(entityIndex);

		CCharacter *character = PlayerManager.getChar(playerId);
		if (character)
		{
			// Send the threshold information
//			character->_PropertyDatabase.setProp("FAME:THRESHOLD_TRADE", FameMinToTrade/kFameMultipler );
			CBankAccessor_PLR::getFAME().setTHRESHOLD_TRADE(character->_PropertyDatabase, FameMinToTrade/kFameMultipler );
//			character->_PropertyDatabase.setProp("FAME:THRESHOLD_KOS", FameMinToKOS/kFameMultipler );
			CBankAccessor_PLR::getFAME().setTHRESHOLD_KOS(character->_PropertyDatabase, FameMinToKOS/kFameMultipler );
			character->resetFameDatabase();

			// Make sure they belong to a guild.
			uint32 guildID = character->getGuildId();
			if (guildID != 0)
			{
				// Now, make sure their guild didn't change allegiance while they were offline.
				character->canBelongToGuild(guildID, true);
			}
		}
	}
	else
	{
		nlwarning("FAME: unknown player id %s", playerId.toString().c_str());
	}
}

void CFameManager::savePlayerFame(const NLMISC::CEntityId &playerId, EGSPD::CFameContainerPD &fameContainer)
{
	TDataSetRow entityIndex = TheFameDataset.getDataSetRow(playerId);
	if (TheFameDataset.isAccessible(entityIndex))
	{
		TFameContainer::iterator it(_FamesOwners.find(entityIndex));
		if (it != _FamesOwners.end())
		{
			TFameOwnerWrite *fow = it->second;
			// save the fame memory record
			CEntityId memoryId = TheFameDataset.getEntityId(fow->FameMemory.getValue());
			{
				TFameContainer::iterator it(_FamesOwners.find(fow->FameMemory.getValue()));
				if (it != _FamesOwners.end())
				{
					// save the fame memory
					for (uint i= 0; i<MAX_FACTION; ++i)
					{
						CSheetId id  = CStaticFames::getInstance().getFactionSheet( i );
						if ( id == CSheetId::Unknown )
							continue;
						EGSPD::CFameContainerEntryPD* entry = fameContainer.getEntries( id );
						if( entry == NULL )
							entry = fameContainer.addToEntries( id );
						EGS_PD_AST(entry);
						entry->setFameMemory( it->second->Fames[i] );
					}
					
//					fameContainer._FamesMemory.resize(MAX_FACTION);
//					for (uint i=0; i<MAX_FACTION; ++i)
//						fameContainer._FamesMemory[i] = it->second->Fames[i];
				}
			}

			// save the fame
//			fameContainer._Fames.resize(MAX_FACTION);
			for (uint i= 0; i<MAX_FACTION; ++i)
			{
				CSheetId id  = CStaticFames::getInstance().getFactionSheet( i );
				if ( id == CSheetId::Unknown )
					continue;
				EGSPD::CFameContainerEntryPD* entry = fameContainer.getEntries( id );
				nlassert( entry );
				if( entry == NULL )
					entry = fameContainer.addToEntries( id );
				EGS_PD_AST(entry);
				entry->setFame( it->second->Fames[i] );
				entry->setLastFameChangeTrend( fow->LastFameChangeTrends[i] );
			}

			// save the last guild change.
			fameContainer.setLastGuildStatusChange( it->second->LastGuildStatusChange );

			// save the fame trend
			fameContainer.setLastFameChangeDate( fow->LastFameChangeDate );
//			fameContainer._LastFameChangeTrends.resize(MAX_FACTION);
		}
		else
		{
			nlwarning("FAME: Can't find entity %u into fame container !", entityIndex.getIndex());
		}
	}
/*	else
	{
		const CCharacter * c = PlayerManager.getChar(playerId);
		if( c == 0 || !c->isCreateCharSave() )
			nlwarning("FAME: unknown player id %s", playerId.toString().c_str());
	}
*/}

void CFameManager::removePlayer(const CEntityId &playerId)
{
	TDataSetRow entityIndex = TheFameDataset.getDataSetRow(playerId);
	if (TheFameDataset.isAccessible(entityIndex))
	{
		TFameContainer::iterator it(_FamesOwners.find(entityIndex));
		if (it != _FamesOwners.end())
		{
			TFameOwnerWrite *fow = it->second;

			// remove the fame memory record
			CEntityId memoryId = TheFameDataset.getEntityId(fow->FameMemory.getValue());
			{
				TFameContainer::iterator it(_FamesOwners.find(fow->FameMemory.getValue()));
				if (it != _FamesOwners.end())
				{
					delete it->second;

					if (it->first == _LastUpdatedRow)
					{
						TFameContainer::iterator next(it);
						next++;
						if (next != _FamesOwners.end())
							_LastUpdatedRow = next->first;
						else
							_LastUpdatedRow = TDataSetRow();
					}
					_FamesOwners.erase(it);
				}
				Mirror.removeEntity(memoryId);
			}

			// remove the player data (dataset record is removed elsewhere)
			if (it->first == _LastUpdatedRow)
			{
				TFameContainer::iterator next(it);
				next++;
				if (next != _FamesOwners.end())
					_LastUpdatedRow = next->first;
				else
					_LastUpdatedRow = TDataSetRow();
			}

			delete it->second;
			_FamesOwners.erase(it);
		}
		else
		{
			nlwarning("FAME: Can't find entity %u into fame container !", entityIndex.getIndex());
		}
	}
	else
	{
		nlwarning("FAME: unknown player id %s", playerId.toString().c_str());
	}
}

void CFameManager::addGuild(const CEntityId &guildId, const EGSPD::CFameContainerPD &fameContainer, EGSPD::CPeople::TPeople civilisation)
{
	H_AUTO(FMaddGuild);
	Mirror.createAndDeclareEntity(guildId);
	TDataSetRow	entityIndex = TheFameDataset.getDataSetRow(guildId);

	TFameOwnerWrite	*fow = new TFameOwnerWrite(TheFameDataset, entityIndex);

	_FamesOwners.insert(make_pair(entityIndex, fow));

	updateGuildFame(guildId, fameContainer);

	// restore the fame value
	for ( map<CSheetId,EGSPD::CFameContainerEntryPD>::const_iterator it = fameContainer.getEntriesBegin(); it != fameContainer.getEntriesEnd(); ++it )
	{
		uint idx = CStaticFames::getInstance().getFactionIndexFromSheet( (*it).second.getSheet() );
		if ( idx == CStaticFames::INVALID_FACTION_INDEX )
			continue;
		fow->Fames[idx] = (*it).second.getFame();
		fow->LastFameChangeTrends[idx] = (*it).second.getLastFameChangeTrend();
	}
//	for (uint i=0; i<fameContainer._Fames.size(); ++i)
//		fow->Fames[i] = fameContainer._Fames[i];

}

void CFameManager::updateGuildFame(const CEntityId &guildId, const EGSPD::CFameContainerPD &fameContainer)
{
	TDataSetRow	entityIndex = TheFameDataset.getDataSetRow(guildId);

	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	BOMB_IF(it == _FamesOwners.end(), "Failed to find the fame owner zriter for guild "<<guildId, return);

	TFameOwnerWrite	*fow = it->second;

	// cleanup the fames
	for (uint i=0; i<MAX_FACTION; ++i)
	{
		fow->Fames[i] = NO_FAME;
		fow->LastFameChangeTrends[i] = EGSPD::CFameTrend::FameSteady;
	}

	// update the fame values
	for ( map<CSheetId,EGSPD::CFameContainerEntryPD>::const_iterator it = fameContainer.getEntriesBegin(); it != fameContainer.getEntriesEnd(); ++it )
	{
		uint idx = CStaticFames::getInstance().getFactionIndexFromSheet( (*it).second.getSheet() );
		if ( idx == CStaticFames::INVALID_FACTION_INDEX )
			continue;
		fow->Fames[idx] = (*it).second.getFame();
		fow->LastFameChangeTrends[idx] = (*it).second.getLastFameChangeTrend();
	}
//	for (uint i=0; i<fameContainer._Fames.size(); ++i)
//		fow->Fames[i] = fameContainer._Fames[i];

}

void CFameManager::saveGuildFame(const NLMISC::CEntityId &guildId, EGSPD::CFameContainerPD &fameContainer)
{
	H_AUTO( CFameManager_saveGuildFame )
	TDataSetRow entityIndex = TheFameDataset.getDataSetRow(guildId);
	if (TheFameDataset.isAccessible(entityIndex))
	{
		TFameContainer::iterator it(_FamesOwners.find(entityIndex));
		if (it != _FamesOwners.end())
		{
			// save the fame
			for (uint i= 0; i<MAX_FACTION; ++i)
			{
				CSheetId id  = CStaticFames::getInstance().getFactionSheet( i );
				if ( id == CSheetId::Unknown )
					continue;
				EGSPD::CFameContainerEntryPD* entry = fameContainer.getEntries( id );
				if( entry == NULL )
					entry = fameContainer.addToEntries( id );
				EGS_PD_AST(entry);
				nlassert( entry );
				{
					H_AUTO( CFameManager_entry_setFame )
					entry->setFame( it->second->Fames[i] );
				}
				{
					H_AUTO( CFameManager_entry_setLastFameChangeTrend )
					entry->setLastFameChangeTrend( it->second->LastFameChangeTrends[i] );
				}
			}
			
//			fameContainer._Fames.resize(MAX_FACTION);
//				fameContainer._Fames[i] = it->second->Fames[i];
		}
		else
		{
			nlwarning("FAME: Can't find guild %u into fame container !", entityIndex.getIndex());
		}
	}
	else
	{
		nlwarning("FAME: unknown guild id %s", guildId.toString().c_str());
	}
}

void CFameManager::removeGuild(const CEntityId &guildId)
{
	TDataSetRow entityIndex = TheFameDataset.getDataSetRow(guildId);
	if (TheFameDataset.isAccessible(entityIndex))
	{
		TFameContainer::iterator it(_FamesOwners.find(entityIndex));
		if (it != _FamesOwners.end())
		{
			if (it->first == _LastUpdatedRow)
			{
				TFameContainer::iterator next(it);
				next++;
				if (next != _FamesOwners.end())
					_LastUpdatedRow = next->first;
				else
					_LastUpdatedRow = TDataSetRow();
			}

			delete it->second;
			_FamesOwners.erase(it);
		}
		else
		{
			nlwarning("FAME: Can't find entity %u into fame container !", entityIndex.getIndex());
		}
		Mirror.removeEntity( guildId );
	}
	else
	{
		nlwarning("FAME: unknown guild id %s", guildId.toString().c_str());
	}
}

void CFameManager::setGuildCivilisation( const NLMISC::CEntityId & guildId, EGSPD::CPeople::TPeople civilisation )
{
	TDataSetRow guildIndex = TheFameDataset.getDataSetRow(guildId);
	
	// check if the civ exists
	uint8 id = EGSPD::getCivilisationId( civilisation );
	if ( id == 0xFF )
	{
		nlwarning("FAME :setGuildCivilisation-> guild %s, civ %u is invalid",guildId.toString().c_str(),civilisation );
		return;
	}
	CEntityId civId(RYZOMID::civilisation, id);
	TDataSetRow civIndex = TheFameDataset.getDataSetRow(civId);
	
	TFameContainer::iterator it = _FamesOwners.find( civIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME:setPlayerCivilisation-> unknown civ eId %s", civId.toString().c_str(), id );
		return;
	}
	
	// check if the player exists
	it = _FamesOwners.find( guildIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME:setPlayerCivilisation-> unknown guild eId %s", guildId.toString().c_str() );
		return;
	}
	if ( (*it).second == NULL )
	{
		nlwarning("FAME:setPlayerCivilisation-> guild eId %s has a NULL entry", guildId.toString().c_str() );
		return;
	}
	(*it).second->Civilisation = civIndex;
}

void CFameManager::setPlayerCivilisation( const NLMISC::CEntityId & playerId, EGSPD::CPeople::TPeople civilisation )
{
	TDataSetRow playerIndex = TheFameDataset.getDataSetRow(playerId);

	// check if the civ exists
	uint8 id = EGSPD::getCivilisationId( civilisation );
	if ( id == 0xFF )
	{
		nlwarning("FAME :setPlayerCivilisation-> user %s, civ %u is invalid",playerId.toString().c_str(),civilisation );
		return;
	}
	CEntityId civId(RYZOMID::civilisation, id);
	TDataSetRow civIndex = TheFameDataset.getDataSetRow(civId);
	
	TFameContainer::iterator it = _FamesOwners.find( civIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME:setPlayerCivilisation-> unknown civ eId %s  ( id = %u) ", playerId.toString().c_str(), id );
		return;
	}

	// check if the player exists
	it = _FamesOwners.find( playerIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME:setPlayerCivilisation-> unknown player eId %s", playerId.toString().c_str() );
		return;
	}
	if ( (*it).second == NULL )
	{
		nlwarning("FAME:setPlayerCivilisation-> player eId %s has a NULL entry", playerId.toString().c_str() );
		return;
	}
	(*it).second->Civilisation = civIndex;
}

void CFameManager::setPlayerGuild( const NLMISC::CEntityId & playerId, const NLMISC::CEntityId & guildId, bool resetLastGuildStatusChange )
{
	TDataSetRow playerIndex = TheFameDataset.getDataSetRow(playerId);
	TDataSetRow GuildIndex = TheFameDataset.getDataSetRow(guildId);

	if (PlayerManager.getChar(playerId) == NULL)
	{
		// offline char, nothing to do
		return;
	}

	// check if the guild exists
	TFameContainer::iterator it = _FamesOwners.find( GuildIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME: unknown guild eId %s  ( id = %u) ", guildId.toString().c_str(), (uint32)guildId.getShortId() );
		return;
	}
	TFameOwnerWrite &guildFow = *(it->second);
	it = _FamesOwners.find( playerIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME: unknown player eId %s", playerId.toString().c_str() );
		return;
	}
	if (  it->second == NULL )
	{
		nlwarning("FAME: player eId %s has a NULL entry", playerId.toString().c_str() );
		return;
	}
	TFameOwnerWrite &fow = *(it->second);
	fow.Guild = GuildIndex;

	// retreive the current fame memory
	it = _FamesOwners.find(fow.FameMemory);
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: player eId %s has no fame memory record !", playerId.toString().c_str() );
		return;
	}

	if (resetLastGuildStatusChange)
	{
		// set the guild fame memory
		for (uint i=0; i<MAX_FACTION; ++i)
		{
			fow.LastGuildFame[i] = it->second->Fames[i];
		}
		fow.LastGuildStatusChange = CTickEventHandler::getGameCycle();
	}
}

void CFameManager::clearPlayerGuild( const NLMISC::CEntityId & playerId )
{
	TDataSetRow playerIndex = TheFameDataset.getDataSetRow(playerId);
	TFameContainer::iterator it = _FamesOwners.find( playerIndex );
	if ( it == _FamesOwners.end() )
	{
		nlwarning("FAME: unknown player eId %s", playerId.toString().c_str() );
		return;
	}

	TFameOwnerWrite &fow = *(it->second);

	fow.Guild = TDataSetRow();

	// retreive the current fame memory
	it = _FamesOwners.find(fow.FameMemory);
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: player eId %s has no fame memory record !", playerId.toString().c_str() );
		return;
	}
	// set the guild fame memory
	for (uint i=0; i<MAX_FACTION; ++i)
	{
		fow.LastGuildFame[i] = it->second->Fames[i];
	}
	fow.LastGuildStatusChange = CTickEventHandler::getGameCycle();
}

void CFameManager::cbFameDelta( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId )
{
	CEntityId	entityId;
	uint32		faction;
	sint32		deltaFame;
	bool		propagate;

	// entity index in fame dataset
	msgin.serial(entityId);
	// faction index
	msgin.serial(faction);
	// the delta value
	msgin.serial(deltaFame);
	msgin.serial(propagate);

	getInstance().addFameIndexed(entityId, faction, deltaFame, serviceName, propagate);
	
	// We don't inform the client right now, the timer will take care of this
	//character->sendEventForMissionAvailabilityCheck();
}

void CFameManager::addFameIndexed(const CEntityId &entityId, uint32 faction, sint32 deltaFame, const std::string &serviceName, bool propagate, TFamePropagation propagationType)
{
	// static string manager param table
	SM_STATIC_PARAMS_2(fameMsgParams, STRING_MANAGER::faction, STRING_MANAGER::integer);

	const TDataSetRow entityIndex = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator	it(_FamesOwners.find(entityIndex));

	CStaticFames &sf = CStaticFames::getInstance();

	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: Receiving fame value for an unknow entityIndex %s from service '%s' (delta = %d)",
			entityId.toString().c_str(),
			serviceName.c_str(),
			deltaFame);

		return;
	}

	if (faction >= MAX_FACTION)
	{
		nlwarning("FAME: addFameIndexed: trying to add fame to invalid faction %u (max of %u faction) for player %s, from service %s",
			faction,
			MAX_FACTION,
			entityId.toString().c_str(),
			serviceName.c_str());

		return;
	}

	TFameOwnerWrite &fow = *(it->second);

	// set the faction name in the string msg param table
	fameMsgParams[0].Enum = faction;
	fameMsgParams[1].Int = propagationType;

	double fame = fow.Fames[faction];
	if (fame == NO_FAME)
	{
		CCharacter* character = PlayerManager.getChar( entityId );
		if (character)
			fame = CStaticFames::getInstance().getStaticFameIndexed(PVP_CLAN::getFactionIndex(PVP_CLAN::getClanFromPeople(character->getRace())), faction);
		else
			fame = 0;
	}

	const double FAME_GAIN_FACTOR = (FameAbsoluteMax/25.0)+FameAbsoluteMax;
	double realDeltaFame = 0.;
	// Non linear fame gain
    if (deltaFame > 1)
	{
        // gain de fame : toujours log
        if (fame > 0)
            realDeltaFame = ((FAME_GAIN_FACTOR - fame) / FameAbsoluteMax) * deltaFame;
        else
            realDeltaFame = ((-FAME_GAIN_FACTOR - fame) / - FameAbsoluteMax) * deltaFame;
	}
    else
	{
		if (fame < 0)
			realDeltaFame = ((-FAME_GAIN_FACTOR - fame) / -FameAbsoluteMax) * deltaFame;
		else
			realDeltaFame = ((FAME_GAIN_FACTOR - fame) / FameAbsoluteMax) * deltaFame;
	}

	fame += realDeltaFame;

	// set fame tendance info
	fow.LastFameChangeDate = CTickEventHandler::getGameCycle();
	fow.LastFameChangeTrends[faction] = deltaFame > 0 ? EGSPD::CFameTrend::FameUpward : EGSPD::CFameTrend::FameDownward;

	CFameInterface &fi = CFameInterface::getInstance();
	// update database and send message
	if (entityId.getType() == RYZOMID::player)
	{
		nldebug("FAME: Updating fame for character %s as P:%d",
			entityId.toString().c_str(),
			sint32(fame));
		// just update one player DB
		// retreive the char info 
		CCharacter *c =  PlayerManager.getChar(entityId);
		// See if still qualify to have declaration
		if ( c )
		{
			// Bound the fame based on current allegiance.
			sint32 maxFame = getMaxFameByFactionIndex(c->getAllegiance(), faction);
			clamp(fame,FameAbsoluteMin,maxFame);
			// Check to make sure player still qualifies to be in declared allegiances.
			c->verifyClanAllegiance(PVP_CLAN::getClanFromIndex(faction), sint32(fame));
			c->setFameValuePlayer(faction, sint32(fame), maxFame, fow.LastFameChangeTrends[faction]);
			if (deltaFame > 0)
				CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "FAME_GAIN_CHAR", fameMsgParams );
			else
				CCharacter::sendDynamicSystemMessage( c->getEntityRowId(), "FAME_LOST_CHAR", fameMsgParams );

			// if character is in guild, guild win fame too
			CGuild *pGuild = CGuildManager::getInstance()->getGuildFromId(c->getGuildId());
			if( pGuild != 0 )
			{
				if( pGuild->getMemberCount() > 0 ) // guild with zero members must never occurs
					addFameIndexed( pGuild->getEId(), faction, deltaFame / pGuild->getMemberCount(), serviceName, propagate, propagationType );
				else
					nlwarning("Guild %d have no member (getMemberCount() return 0)", c->getGuildId());
			}
		}
		else
			nlwarning("addFameIndexed for a player : invalid user %s",entityId.toString().c_str());
	}
	else if (entityId.getType() == RYZOMID::guild)
	{
		nldebug("FAME: updating fame for guild %s...", entityId.toString().c_str());

		CGuild *g = CGuildManager::getInstance()->getGuildFromId((uint32)(entityId.getShortId()));
		if ( g )
		{
			// Bound the fame based on current allegiance.
			sint32 maxFame = getMaxFameByFactionIndex(g->getAllegiance(), faction);
			clamp(fame,FameAbsoluteMin,maxFame);
			g->verifyClanAllegiance(PVP_CLAN::getClanFromIndex(faction), sint32(fame));
			g->setFameValueGuild(faction, sint32(fame), maxFame, fow.LastFameChangeTrends[faction]);
		}

		// update DB for all the players in this guild
		sint32 gf = fi.getFameIndexed(entityId, faction, false, true);
		TFameContainer::iterator first(_FamesOwners.begin()), last(_FamesOwners.end());
		for (; first != last; ++first)
		{
			TFameOwnerWrite *fow = first->second;
			if (fow->Guild() == entityIndex)
			{
				// guild fame (using memory)
				if (fow->LastGuildStatusChange != 0)
				{
					// need to read in memory record
					if (fow->FameMemory().isValid())
						gf = fi.getFameIndexed(TheFameDataset.getEntityId(fow->FameMemory), faction, false, true);
				}
				else if (fow->Guild().isValid())
				{
					gf = fi.getFameIndexed(TheFameDataset.getEntityId(fow->Guild()), faction, false, true);
				}
				
				nldebug("FAME: Updating guild fame for entityId %s as G:%d",
					entityId.toString().c_str(),
					sint32(fame));
			}
		}
	}
	else
	{
		nlwarning("FAME: Invalid entity Id %s", entityId.toString().c_str());
		return;
	}

	// Set the fame, now bound by entity type.
	fow.Fames[faction] = sint32(fame);

	// Propagate fame
	if (propagate)
	{
		// Kami/Karavan special case:
		// When gaining kami/karavan fame, this fame do not propagate to
		// tribes/races, whereas when gaining tribe/race fame character gains
		// kami/karavan fame. This is a normal case in asymmetric fame.
		NLMISC::TStringId factionKaravan = NLMISC::CStringMapper::map("karavan");
		NLMISC::TStringId factionKami = NLMISC::CStringMapper::map("kami");
		if (UseAsymmetricStaticFames || (faction!=CStaticFames::getInstance().getFactionIndex(factionKaravan) && faction!=CStaticFames::getInstance().getFactionIndex(factionKami)))
		{
			// Propagate to each fame
			int nbFame = CStaticFames::getInstance().getNbFame();
			for (int iFaction=0; iFaction<nbFame; ++iFaction)
			{
				float propagation = CStaticFames::getInstance().getPropagationFactorIndexed(faction, iFaction);
				// Skip propagation if factor is null
				if (propagation!=0.f)
				{
					sint32 propagatedFame = (sint32)(realDeltaFame*propagation);
					addFameIndexed(entityId, iFaction, propagatedFame, serviceName, false, propagationType);
				}
			}
		}
	}
}

void CFameManager::addFameIndexed(const CEntityId &entityId, uint factionIndex, sint32 deltaFame, bool propagate)
{
	addFameIndexed(entityId, factionIndex, deltaFame, NLNET::IService::getInstance()->getServiceShortName(), propagate);
}

EGSPD::CFameTrend::TFameTrend	CFameManager::getFameTrendIndexed(const TDataSetRow &entityIndex, uint32 factionIndex)
{
	// retrieve the fame owner
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: entity %x doesn't exist in fame owners containers!", entityIndex.getIndex());
		return EGSPD::CFameTrend::FameSteady;
	}

	return  it->second->LastFameChangeTrends[factionIndex];
}

float CFameManager::getPlayerMemoryBlendIndexed(const TDataSetRow &entityIndex)
{
	// retrieve the fame owner
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: entity %x doesn't exist in fame owners containers!", entityIndex.getIndex());
		return 1.0f;
	}

	CEntityId eid = TheFameDataset.getEntityId(entityIndex);
	if (eid.getType() != RYZOMID::player)
	{
		nlwarning("FAME: entity %s(%x) is not a player, can't return a valid fame memory blend!", eid.toString().c_str(), entityIndex.getIndex());
		return 1.0f;
	}

	if (it->second->LastGuildStatusChange != 0)
	{
		// retreive the current date
		TGameCycle now = CTickEventHandler::getGameCycle();

		uint deltaT = now - it->second->LastGuildStatusChange;

		if (deltaT <= 0)
			return 1.0f;

		float blend = float(deltaT / double(FameMemoryInterpolation));
		clamp(blend, 0.0f, 1.0f);

		return blend;
	}
	else
		return 1.0f;
}


sint32	CFameManager::getFameIndexed(const CEntityId &entityId, uint32 factionIndex, bool modulated, bool returnUnknownValue)
{
	// retrieve the fame owner
	const TDataSetRow entityIndex = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: entity %s doesn't exist in fame owners containers!", entityId.toString().c_str());
		return 0;
	}

	CStaticFames	&staticFame = CStaticFames::getInstance();
	if (factionIndex >=  MAX_FACTION)
	{
		if( factionIndex != 0xffffffff && factionIndex != NO_FAME )
			nlwarning("FAME: entity %s : factionIndex %u is out of limit (%u)", entityId.toString().c_str(), factionIndex, MAX_FACTION);
		return 0;
	}

	TFameOwnerWrite	*fo = it->second;
	const CEntityId	&id = TheFameDataset.getEntityId(entityIndex);

	// return direct entity fame
	sint32 fame = fo->Fames[factionIndex]();

	CCharacter* character = PlayerManager.getChar( entityId );
	if (!returnUnknownValue && fame == NO_FAME)
	{
		if (character)
			fame = CStaticFames::getInstance().getStaticFameIndexed(PVP_CLAN::getFactionIndex(PVP_CLAN::getClanFromPeople(character->getRace())), factionIndex);
		else
			fame = 0;
	}
	
	// clamp fame upper bound to neutral max if entity is declared as "None"
	if( character)
	{
		pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegience = character->getAllegiance();

		if( allegience.first == PVP_CLAN::None )
		{
			if( PVP_CLAN::getClanFromIndex(factionIndex) >= PVP_CLAN::BeginCults && PVP_CLAN::getClanFromIndex(factionIndex) <= PVP_CLAN::EndCults )
			{
				fame = min(fame, FameMaxCults[kNeutral][PVP_CLAN::getClanFromIndex(factionIndex)-PVP_CLAN::BeginCults]->get());
			}
		}
		else if( allegience.second == PVP_CLAN::None )
		{
			if( PVP_CLAN::getClanFromIndex(factionIndex) >= PVP_CLAN::BeginCivs && PVP_CLAN::getClanFromIndex(factionIndex) <= PVP_CLAN::EndCivs )
			{
				fame = min(fame, FameMaxCivs[kNeutral][PVP_CLAN::getClanFromIndex(factionIndex)-PVP_CLAN::BeginCivs]->get());
			}
		}
	}

	return sint32(fame);
}

const TDataSetRow &CFameManager::getCivilisationIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;
	const TDataSetRow entityIndex = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		return invalidIndex;
	}
	else
	{
		return it->second->Civilisation;
	}
}

const TDataSetRow &CFameManager::getGuildIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;
	const TDataSetRow entityIndex = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		return invalidIndex;
	}
	else
	{
		return it->second->Guild;
	}
}

const TDataSetRow &CFameManager::getFameMemoryIndex(const CEntityId &entityId)
{
	static const TDataSetRow invalidIndex;
	const TDataSetRow entityIndex = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	if (it == _FamesOwners.end())
	{
		return invalidIndex;
	}
	else
	{
		return it->second->FameMemory;
	}
}

void CFameManager::setEntityFame(const NLMISC::CEntityId & entityId, uint32 faction, sint32 fame, bool setDirectValue)
{
	if (entityId.getType() != RYZOMID::player && entityId.getType() != RYZOMID::guild)
		return;

	CCharacter* ch = NULL;
	CGuild* gu = NULL;
	if (entityId.getType() == RYZOMID::player)
	{
		ch =  PlayerManager.getChar(entityId);
	}
	else
	{
		if (entityId.getType() == RYZOMID::guild)
		{
			gu = CGuildManager::getInstance()->getGuildFromId((uint32)(entityId.getShortId()));
		}
	}


	const TDataSetRow rowId = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it = _FamesOwners.find(rowId);
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: unknown entity index %s (fame = %d)", entityId.toString().c_str(), fame);
		return;
	}

	if (faction >= MAX_FACTION)
	{
		nlwarning("FAME: invalid faction %u (max of %u faction) for entity %s", faction, MAX_FACTION, entityId.toString().c_str());
		return;
	}

	TFameOwnerWrite & fow = *(it->second);

	if( setDirectValue )
	{
		// change the scale( because this value comes from a user command)
		fame = fame*(FameAbsoluteMax/100);
	}

	if (fame == NO_FAME)
	{
		CCharacter* character = PlayerManager.getChar( entityId );
		if (character)
			fame = CStaticFames::getInstance().getStaticFameIndexed(PVP_CLAN::getFactionIndex(PVP_CLAN::getClanFromPeople(character->getRace())), faction);
		else
			fame = 0;
	}
	clamp(fame, FameAbsoluteMin, FameAbsoluteMax);

	// get old fame
	sint32 oldFame = fow.Fames[faction];
	if (oldFame == NO_FAME)
	{
		CCharacter* character = PlayerManager.getChar( entityId );
		if (character)
			oldFame = CStaticFames::getInstance().getStaticFameIndexed(PVP_CLAN::getFactionIndex(PVP_CLAN::getClanFromPeople(character->getRace())), faction);
		else
			oldFame = 0;
	}
	clamp(oldFame, FameAbsoluteMin, FameAbsoluteMax);

	const sint32 deltaFame = fame - oldFame;
	if (deltaFame == 0 && fow.Fames[faction] != NO_FAME )
		return;

	// set fame
	fow.Fames[faction] = fame;

	// set fame trend info
	fow.LastFameChangeDate = CTickEventHandler::getGameCycle();
	fow.LastFameChangeTrends[faction] = deltaFame > 0 ? EGSPD::CFameTrend::FameUpward : EGSPD::CFameTrend::FameDownward;

	// update database and send message
	if(ch)
	{
		nldebug("FAME: set fame for character %s as P:%d", entityId.toString().c_str(), fame);

		sint32 maxFame = getMaxFameByFactionIndex(ch->getAllegiance(), faction);
		ch->setFameValuePlayer(faction, fame, maxFame, fow.LastFameChangeTrends[faction]);

		SM_STATIC_PARAMS_1(params, STRING_MANAGER::faction);
		params[0].Enum = faction;

		if (deltaFame > 0)
			CCharacter::sendDynamicSystemMessage( ch->getEntityRowId(), "FAME_GAIN_CHAR", params );
		else
			CCharacter::sendDynamicSystemMessage( ch->getEntityRowId(), "FAME_LOST_CHAR", params );
	} 
	else if(gu)
	{
		sint32 maxFame = getMaxFameByFactionIndex(gu->getAllegiance(), faction);
		gu->setFameValueGuild(faction, fame, maxFame, fow.LastFameChangeTrends[faction]);
	}
	else
		nlwarning("FAME: unknown/invalid entity %s",entityId.toString().c_str());
}

// - GetStartFame: playerClan must be a Civilization, targetClan must be any non-neutral clan
sint32 CFameManager::getStartFame(PVP_CLAN::TPVPClan playerClan, PVP_CLAN::TPVPClan targetClan)
{
	// Make sure our parameters are correct.  Player clan must be one of the civilizations,
	//  and target clan can be any of the clans.
	if (playerClan < PVP_CLAN::BeginCivs || playerClan > PVP_CLAN::EndCivs
		|| targetClan < PVP_CLAN::BeginClans || targetClan > PVP_CLAN::EndClans)
	{
		// Bad parameters, return error.
		return NO_FAME;
	}

	// Parameters are within values, so return the lookup.
	return *FameStart[playerClan-PVP_CLAN::BeginCivs][targetClan-PVP_CLAN::BeginClans];
}

// - getMaxFameByClan: playerClan must be Neutral or the same type (Cult or Clan) as targetClan, targetClan must be any non-neutral clan.
sint32 CFameManager::getMaxFameByClan(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> playerClans, PVP_CLAN::TPVPClan targetClan)
{
	// Local variables for the lookup values.
	int playerLookup, targetLookup;

	// Determine if we're looking for Cult or Civ max fame, looking at targetClan.
	if (targetClan >= PVP_CLAN::BeginCults && targetClan <= PVP_CLAN::EndCults)
	{
		// Look up cults.  Cult is the first element of the pair for playerClans.
		playerLookup = playerClans.first;
		// Special rule for people with "None" for declared cult: no real limits
		if (playerLookup == PVP_CLAN::None)
		{
			// Clamp it to the absolute max.
			return FameAbsoluteMax;
		}
		// Make sure the player clan falls within our expected values.
		if (playerLookup == PVP_CLAN::Neutral
			|| (playerLookup >= PVP_CLAN::BeginCults && playerLookup <= PVP_CLAN::EndCults))
		{
			// Adjust values for lookup
			// - Turn the parameters into a base 1 lookup.  That is, the first Cult is 1, etc.
			playerLookup = playerLookup - kBeginCults1;
			targetLookup = targetClan - PVP_CLAN::BeginCults;
			// Clamp the player value.  Neutral will go below 0 when adjusted, so bump it back up to 0.
			clamp(playerLookup, 0, PVP_CLAN::NbClans);

			return *FameMaxCults[playerLookup][targetLookup];
		}
	}
	else
	{
		if (targetClan >= PVP_CLAN::BeginCivs && targetClan <= PVP_CLAN::EndCivs)
		{
			// Look up civs.  Civ is the second element of the pair for playerClans
			playerLookup = playerClans.second;
			// Special rule for people with "None" for declared cult: no real limits
			if (playerLookup == PVP_CLAN::None)
			{
				// Clamp it to the absolute max.
				return FameAbsoluteMax;
			}
			// Make sure the player clan falls within our expected values.
			if (playerLookup == PVP_CLAN::Neutral
				|| (playerLookup >= PVP_CLAN::BeginCivs && playerLookup <= PVP_CLAN::EndCivs))
			{
				// Adjust values for lookup
				// - Turn the parameters into a base 1 lookup.  That is, the first Civ is 1, etc.
				playerLookup = playerLookup - kBeginCivs1;
				targetLookup = targetClan - PVP_CLAN::BeginCivs;
				// Clamp the player value.  Neutral will go below 0 when adjusted, so bump it back up to 0.
				clamp(playerLookup, 0, PVP_CLAN::NbClans);

				return *FameMaxCivs[playerLookup][targetLookup];
			}
		}
	}

	// Wasn't caught above, probably a tribe.  Return a default value.
	return FameMaxDefault;
}

sint32 CFameManager::getMaxFameByFactionIndex(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance, uint32 factionIndex)
{
	PVP_CLAN::TPVPClan	pvpClan;

	// try first with a clan
	pvpClan= PVP_CLAN::getClanFromIndex(factionIndex);
	if(pvpClan != PVP_CLAN::Unknown)
		return getMaxFameByClan(allegiance, pvpClan);
	// search for tribe
	else
	{
		// No allegiance? => Max
		if( allegiance.first == PVP_CLAN::None || allegiance.second == PVP_CLAN::None )
			return FameAbsoluteMax;
		
		// look up in the tribe threshold clamp array
		const vector<CStaticFames::CTribeCultThresholdPerCiv> &tribeThres= CStaticFames::getInstance().getTribeThresholdVector();
		if(tribeThres.empty())
			return FameMaxDefault;
		else
		{
			// get correct index in tribe list
			initTribeThresholdIndex();
			uint32	ttIndex= ~0;
			if(factionIndex<_FactionIndexToTribeThresholdIndex.size())
				ttIndex= _FactionIndexToTribeThresholdIndex[factionIndex];
			if(ttIndex>=tribeThres.size())
				return FameMaxDefault;

			// get the 
			const CStaticFames::CTribeCultThreshold * tc = 0;
			
			switch( allegiance.second )
			{
			case PVP_CLAN::Matis:
				tc = &tribeThres[ttIndex].Matis;
				break;
			case PVP_CLAN::Fyros:
				tc = &tribeThres[ttIndex].Fyros;
				break;
			case PVP_CLAN::Tryker:
				tc = &tribeThres[ttIndex].Tryker;
				break;
			case PVP_CLAN::Zorai:
				tc = &tribeThres[ttIndex].Zorai;
				break;
			case PVP_CLAN::Neutral:
				tc = &tribeThres[ttIndex].Neutral;
				break;
			default:
				//nlwarning("Character %s have bad civilization allegiance...'%d/%s' !", entityId.toString().c_str(), allegiance.second, PVP_CLAN::toString(allegiance.second).c_str());
				return FameMaxDefault;
			}
			
			switch(allegiance.first)
			{
			case PVP_CLAN::Kami:
				return tc->getKami();
				break;
			case PVP_CLAN::Karavan:
				return tc->getKaravan();
				break;
			case PVP_CLAN::Neutral:
				return tc->getNeutral();
				break;
			default:
				//nlwarning("Character %s have bad cult allegiance...'%d/%s' !", entityId.toString().c_str(), allegiance.first, PVP_CLAN::toString(allegiance.first).c_str());
				return FameMaxDefault;
			}
		}
	}
}

void CFameManager::doInitTribeThresholdIndex()
{
	CStaticFames	&sf= CStaticFames::getInstance();
	_FactionIndexToTribeThresholdIndex.clear();
	_FactionIndexToTribeThresholdIndex.resize(sf.getNbFame(), ~0);
	// For All Tribe thresold index
	const vector<CStaticFames::CTribeCultThresholdPerCiv> &tribeThres= CStaticFames::getInstance().getTribeThresholdVector();
	for(uint i=0;i<tribeThres.size();i++)
	{
		// get the faction index
		uint32	factionIndex= tribeThres[i].FameIndex;
		// add in the remap table
		if(factionIndex>=_FactionIndexToTribeThresholdIndex.size())
			_FactionIndexToTribeThresholdIndex.resize(factionIndex+1, ~0);
		_FactionIndexToTribeThresholdIndex[factionIndex]= i;
	}
}


void CFameManager::enforceFameCaps(const NLMISC::CEntityId &entityId, std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance)
{
	const TDataSetRow rowId = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it = _FamesOwners.find(rowId);
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: unknown entity index %s",entityId.toString().c_str());
		return;
	}

	TFameOwnerWrite & fow = *(it->second);

	CCharacter* ch = NULL;
	CGuild* gu = NULL;
	if (entityId.getType() == RYZOMID::player)
	{
		ch =  PlayerManager.getChar(entityId);
	}
	else
	{
		if (entityId.getType() == RYZOMID::guild)
		{
			gu = CGuildManager::getInstance()->getGuildFromId((uint32)(entityId.getShortId()));
		}
	}

	// Use the fame variable as an intermediary in order to get proper conversions.
	uint32 theFactionIndex;
	sint32 fame;
	sint32 maxFame;

	// Check cults, first member of allegiance
	PVP_CLAN::TPVPClan theCult = allegiance.first;
	if (theCult != PVP_CLAN::None)
	{
		for (int looper = PVP_CLAN::BeginCults; looper <= PVP_CLAN::EndCults; looper++)
		{
			theFactionIndex = PVP_CLAN::getFactionIndex((PVP_CLAN::TPVPClan)looper);
			fame = fow.Fames[theFactionIndex];
			maxFame = getMaxFameByClan(allegiance,(PVP_CLAN::TPVPClan)looper);
			if( fame != NO_FAME)
			{
				clamp(fame,FameAbsoluteMin,maxFame);
				fow.Fames[theFactionIndex] = fame;
			}
			if (ch)
			{
				ch->setFameValuePlayer(theFactionIndex, fame, maxFame, fow.LastFameChangeTrends[theFactionIndex]);
			}
			if (gu)
			{
				gu->setFameValueGuild(theFactionIndex, fame, maxFame, fow.LastFameChangeTrends[theFactionIndex]);
			}
		}
	}
	// Check civs, second member of allegiance
	PVP_CLAN::TPVPClan theCiv = allegiance.second;
	if (theCiv != PVP_CLAN::None)
	{
		for (int looper = PVP_CLAN::BeginCivs; looper <= PVP_CLAN::EndCivs; looper++)
		{
			theFactionIndex = PVP_CLAN::getFactionIndex((PVP_CLAN::TPVPClan)looper);
			fame = fow.Fames[theFactionIndex];
			maxFame = getMaxFameByClan(allegiance,(PVP_CLAN::TPVPClan)looper);
			if( fame != NO_FAME)
			{
				clamp(fame,FameAbsoluteMin,maxFame);
				fow.Fames[theFactionIndex] = fame;
			}	
			if (ch)
			{
				ch->setFameValuePlayer(theFactionIndex, fame, maxFame, fow.LastFameChangeTrends[theFactionIndex]);
			}
			if (gu)
			{
				gu->setFameValueGuild(theFactionIndex, fame, maxFame, fow.LastFameChangeTrends[theFactionIndex]);
			}
		}
	}
}

void CFameManager::setAndEnforceTribeFameCap(const NLMISC::CEntityId &entityId, std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance)
{
	const TDataSetRow rowId = TheFameDataset.getDataSetRow(entityId);
	TFameContainer::iterator it = _FamesOwners.find(rowId);
	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: unknown entity index %s",entityId.toString().c_str());
		return;
	}

	TFameOwnerWrite & fow = *(it->second);

	CCharacter* ch = NULL;
	if (entityId.getType() == RYZOMID::player)
	{
		ch =  PlayerManager.getChar(entityId);
		nlassert(ch != 0);
	}
	else
	{
		nlwarning("FAME: Tribe fame is only for character, but entityId '%s' is not a character Eid", entityId.toString().c_str());
		return;
	}

	// if the player is undefined, no need to warning
	if( allegiance.first == PVP_CLAN::None || allegiance.second == PVP_CLAN::None )
		return;

	uint32 theFactionIndex;
	sint32 fame;
	sint32 threshold;
	const CStaticFames::CTribeCultThreshold * tc = 0;

	for( vector<CStaticFames::CTribeCultThresholdPerCiv>::const_iterator it = CStaticFames::getInstance().getTribeThresholdVector().begin(); it != CStaticFames::getInstance().getTribeThresholdVector().end(); ++it )
	{
		theFactionIndex = (*it).FameIndex;
	
		switch( allegiance.second )
		{
		case PVP_CLAN::Matis:
			tc = &(*it).Matis;
			break;
		case PVP_CLAN::Fyros:
			tc = &(*it).Fyros;
			break;
		case PVP_CLAN::Tryker:
			tc = &(*it).Tryker;
			break;
		case PVP_CLAN::Zorai:
			tc = &(*it).Zorai;
			break;
		case PVP_CLAN::Neutral:
			tc = &(*it).Neutral;
			break;
		default:
			nlwarning("Character %s have bad civilization allegiance...'%d/%s' !", entityId.toString().c_str(), allegiance.second, PVP_CLAN::toString(allegiance.second).c_str());
			return;
		}

		switch(allegiance.first)
		{
		case PVP_CLAN::Kami:
			threshold = tc->getKami();
			break;
		case PVP_CLAN::Karavan:
			threshold = tc->getKaravan();
			break;
		case PVP_CLAN::Neutral:
			threshold = tc->getNeutral();
			break;
		default:
			nlwarning("Character %s have bad cult allegiance...'%d/%s' !", entityId.toString().c_str(), allegiance.first, PVP_CLAN::toString(allegiance.first).c_str());
			return;
		}
	
		fame = fow.Fames[theFactionIndex];
		if( fame != NO_FAME )
		{
			clamp(fame,FameAbsoluteMin,threshold);
			fow.Fames[theFactionIndex] = fame;
		}
		ch->setFameValuePlayer(theFactionIndex, fame, threshold, fow.LastFameChangeTrends[theFactionIndex]);
	}
}

void CFameManager::thresholdChanged(NLMISC::IVariable &var)
{
	// Send new thresholds to all players online.
	const CPlayerManager::TMapPlayers& player = PlayerManager.getPlayers();
	for( CPlayerManager::TMapPlayers::const_iterator it = player.begin(); it != player.end(); ++it )
	{
		CCharacter* character = it->second.Player->getActiveCharacter();
		if( character )
		{
			character->setFameBoundaries();
		}
	}
}

//void CFameManager::serialFames( const NLMISC::CEntityId & eId, NLMISC::IStream & f)
//{
//	TDataSetRow index;
//	if ( Mirror.mirrorIsReady() )
//		index = TheFameDataset.getDataSetRow(eId);
//	
//	std::string name;
//	uint32 value;
//	
//	f.xmlPush("fames");
//
//	if ( f.isReading() )
//	{
//		uint32 size;
//		f.xmlPush("size");
//			f.serial(size);
//		f.xmlPop();
//
//		for (uint i = 0; i < size; i++ )
//		{
//			f.xmlPush( NLMISC::toString("fame%u",i).c_str() );
//				f.serial( name );
//				f.serial( value );
//			f.xmlPop();
//
//			TStringId id = CStringMapper::map( name );
//			CFameInterface::getInstance().addFame(index, id, value);
//		}
//	}
//	else
//	{
//		const std::vector<NLMISC::TStringId> & fameIds = CStaticFames::getInstance().getFactionNames();		
//		uint32 size = fameIds.size();
//
//		f.xmlPush("size");
//			f.serial(size);
//		f.xmlPop();
//		
//		for (uint i = 0; i < size; i++ )
//		{
//			std::string name = CStringMapper::unmap( fameIds[i] );
//			value = NO_FAME;
//			if ( index.isValid() )
//				value = CFameInterface::getInstance().getFameIndexed(index, i, false, true);
//			f.xmlPush( NLMISC::toString("fame%u",i).c_str() );
//				f.serial( name );
//				f.serial( value );
//			f.xmlPop();
//		}
//	}
//	f.xmlPop();
//}


void CFameManager::tickUpdate()
{
	TFameContainer::iterator it;

	if (!Mirror.mirrorIsReady())
		return;

	if (!_LastUpdatedRow.isValid())
	{
		// not a valid row restart at beginning
		if (!_FamesOwners.empty())
		{
			it = _FamesOwners.begin();
			_LastUpdatedRow = it->first;
		}
	}
	else
	{
		// a valid row, try to advance to next one
		it = _FamesOwners.find(_LastUpdatedRow);
		if (it != _FamesOwners.end())
		{
			// the last row is valid, what about the next one...
			++it;
			if (it != _FamesOwners.end())
				// ok, the next row is valid
				_LastUpdatedRow = it->first;
			else
				// dawn, we are at the end of container, restart next tick
				_LastUpdatedRow = TDataSetRow();
		}
		else
			// the last row have been removed ? restart next tick
			_LastUpdatedRow = TDataSetRow();
	}


	if (_LastUpdatedRow.isValid())
	{
		// advance until we find a player row
		while (it != _FamesOwners.end() && TheFameDataset.getEntityId(it->first).getType() == RYZOMID::fame_memory)
			++it;

		if (it == _FamesOwners.end())
		{
			_LastUpdatedRow = TDataSetRow();
		}
		else
		{
			// update this row
			TFameOwnerWrite *fow = it->second;
			TDataSetRow entityIndex = it->first;
			CEntityId eid = TheFameDataset.getEntityId(entityIndex);
			_LastUpdatedRow = entityIndex;

			if (fow->LastGuildStatusChange != 0 || fow->LastFameChangeDate != 0)
			{
				// we need to update the fame history according to elapsed time
				if (eid.getType() == RYZOMID::player)
					updatePlayerFame(entityIndex);

				// update the fame trend
				updateFameTrend(entityIndex);

			} // need to interpolate
		} // nothing to update
	}
}

void CFameManager::updateFameTrend(const TDataSetRow &entityIndex)
{
	TFameContainer::iterator it(_FamesOwners.find(entityIndex));
	double alpha = 1.0f;

	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: cannot find fame info for entity row %u", entityIndex.getIndex());
		return;
	}
	TFameOwnerWrite *fow = it->second;

	// update the fame trend if needed
	if (fow->LastFameChangeDate != 0)
	{
		// we need to update the fame trend according to elapsed time
		TGameCycle now = CTickEventHandler::getGameCycle();
		bool neeClientUpdate = false;

		// check if we can reset the trend info to 'steady'
		if (now - fow->LastFameChangeDate > FameTrendResetDelay)
		{
			neeClientUpdate = true;
			// clean the date
			fow->LastFameChangeDate = 0;
			// reset all the trends
			for (uint i=0; i<MAX_FACTION; ++i)
			{
				fow->LastFameChangeTrends[i] = EGSPD::CFameTrend::FameSteady;
			}
		}

		if (neeClientUpdate)
		{
			CEntityId eid = TheFameDataset.getEntityId(entityIndex);
			if (eid.getType() == RYZOMID::player)
			{
				// reset fame trend on player
				CCharacter *character = PlayerManager.getChar(eid);
				if (character == NULL)
				{
					nlwarning("Can't find character '%s' (from fame row index %u) in player manager !", 
						eid.toString().c_str(), 
						entityIndex.toString().c_str());
					return;
				}
			}
			else if (eid.getType() == RYZOMID::guild)
			{
				CGuild *guild = CGuildManager::getInstance()->getGuildFromId((uint32)(eid.getShortId()));
				if (guild == NULL)
				{
					nlwarning("Can't find guild '%s' (from fame row index %u) in guild manager !", 
						eid.toString().c_str(), 
						entityIndex.toString().c_str());
					return;
				}
			}
		}
	}
}

void CFameManager::updatePlayerFame(const TDataSetRow &playerIndex)
{
	CCharacter *character = NULL;
	TFameContainer::iterator it(_FamesOwners.find(playerIndex));
	double		alpha = 1.0f;
	

	if (it == _FamesOwners.end())
	{
		nlwarning("FAME: updatePlayerFame: cannot find fame info for entity row %u", playerIndex.getIndex());
		return;
	}
	TFameOwnerWrite *fow = it->second;

	// we need to update the fame history according to elapsed time
	TGameCycle now = CTickEventHandler::getGameCycle();

	if (fow->LastGuildStatusChange != 0)
	{
		EGSPD::CFameTrend::TFameTrend	trends[MAX_FACTION];
		for (uint i=0; i<MAX_FACTION; ++i)
		{
			trends[i] = EGSPD::CFameTrend::FameSteady;
		}
		// retrieve the player fame memory data
		it = _FamesOwners.find(fow->FameMemory);
		if (it == _FamesOwners.end())
		{
			nlwarning("FAME: updatePlayerFame: Cannot find fame memory record %x for player %s",
				fow->FameMemory().getIndex(),
				TheFameDataset.getEntityId(playerIndex).toString().c_str());
			return;
		}
		TFameOwnerWrite *memory = it->second;

		if (fow->Guild().isValid())
		{
			it = _FamesOwners.find(fow->Guild);
			if (it == _FamesOwners.end())
			{
				nlwarning("FAME: updatePlayerFame: Cannot find guild fame record %x for player %s",
					fow->Guild().getIndex(),
					TheFameDataset.getEntityId(playerIndex).toString().c_str());
				return;
			}
			
			TFameOwnerWrite *guildFame = it->second;

			if (now - fow->LastGuildStatusChange > FameMemoryInterpolation)
			{
				// interpolation end, just fill the guild value inside the memory
				for (uint i=0; i<MAX_FACTION; ++i)
				{
					memory->Fames[i] = guildFame->Fames[i];
				}

				// stop interpolation
				fow->LastGuildStatusChange = 0;
				alpha = 1.0f;
			}
			else
			{
				// need to interpolate : 0 : last guild fame, 1 : new guild fame
				alpha = (now - fow->LastGuildStatusChange) / double(FameMemoryInterpolation);

				for (uint i=0; i<MAX_FACTION; ++i)
				{
					sint32 gf = guildFame->Fames[i]; 
					sint32 hf = fow->LastGuildFame[i];

					if (gf == NO_FAME && hf == NO_FAME)
					{
						// no need to interpolate
						memory->Fames[i] = NO_FAME;
					}
					else
					{
						if (gf == NO_FAME)
							gf = 0;
						if (hf == NO_FAME)
							hf = 0;
						memory->Fames[i] = sint32(hf*(1-alpha) + gf*alpha);

						trends[i] = gf < hf ? EGSPD::CFameTrend::FameDownward : EGSPD::CFameTrend::FameUpward;
					}
				}
			}
		}
		else
		{
			// no current guild, interpolate to 0 then set to NO_FAME at end of interpolation
			if (now - fow->LastGuildStatusChange > FameMemoryInterpolation)
			{
				// interpolation end, just fill the guild value inside the memory
				for (uint i=0; i<MAX_FACTION; ++i)
				{
					memory->Fames[i] = NO_FAME;
				}

				// stop interpolation
				fow->LastGuildStatusChange = 0;
				alpha = 1.0f;
			}
			else
			{
				// need to interpolate : 0 : last guild fame (memory), 1 : new guild fame
				alpha = (now - fow->LastGuildStatusChange) / double(FameMemoryInterpolation);

				for (uint i=0; i<MAX_FACTION; ++i)
				{
					sint32 hf = fow->LastGuildFame[i];

					if (hf == NO_FAME)
					{
						// no need to interpolate
						memory->Fames[i]  = NO_FAME;
					}
					else
					{
						memory->Fames[i] = sint32(hf*(1-alpha));

						trends[i] = memory->Fames[i] < 0 ? EGSPD::CFameTrend::FameDownward : EGSPD::CFameTrend::FameUpward;

					}
				}
			}
		}
	}
}

#define GET_GUILD(onlyLocal) \
	CGuild * guild = CGuildManager::getInstance()->getGuildByName( args[0] );\
	if ( guild == NULL )\
	{\
		/* try to find the guild with an id*/ \
		uint32 shardId =0; \
		uint32 guildId =0; \
		sscanf(args[0].c_str(), "%u:%u", &shardId, &guildId); \
		guild = CGuildManager::getInstance()->getGuildFromId((shardId<<20)+guildId); \
		\
		if (guild == NULL) \
		{ \
			log.displayNL("Invalid guild '%s'",args[0].c_str());\
			return true;\
		} \
	} \
	if (onlyLocal && guild->isProxy())\
	{\
		log.displayNL("The guild '%s' is a foreign guild, operation forbidden", guild->getName().toString().c_str());\
		return true;\
	} \


NLMISC_COMMAND(setFameMemory, "set a value in the fame history, reset the fame interpolation timer", "<character eid> <factionName> [-]<fameValue>")
{
	if (args.size() != 3)
		return false;

	CEntityId eid(args[0]);

	if (eid == CEntityId::Unknown || eid.getType() != RYZOMID::player)
	{
		log.displayNL("Invalid character eid '%s'", args[0].c_str());
		return false;
	}

	CStaticFames &sf = CStaticFames::getInstance();

	uint32 factionIndex = sf.getFactionIndex(args[1]);
	if (factionIndex == CStaticFames::INVALID_FACTION_INDEX)
	{
		log.displayNL("Invalid faction name '%s'", args[1].c_str());
		return false;
	}

	sint32 fame;
	NLMISC::fromString(args[2], fame);

	CFameManager &fm = CFameManager::getInstance();

	// retreive the fame owner
	TDataSetRow playerIndex = TheFameDataset.getDataSetRow(eid);

	if (!TheFameDataset.isAccessible(playerIndex))
	{
		log.displayNL("Error, cannot retreive dataset row for entity '%s'", eid.toString().c_str());
		return false;
	}

	CFameManager::TFameContainer::iterator it(fm._FamesOwners.find(playerIndex));
	if (it == fm._FamesOwners.end())
	{
		log.displayNL("Error, cannot retreive fame information for entity '%s'", eid.toString().c_str());
		return false;
	}

	CFameManager::TFameOwnerWrite *fow = it->second;

	fow->LastGuildFame[factionIndex] = fame;
	fow->LastGuildStatusChange = CTickEventHandler::getGameCycle();

	fm.updatePlayerFame(playerIndex);

	return true;
}

NLMISC_COMMAND (declareCharacterCult, "Make character declare a specific cult.", "<Eid> <Faction>")
{
	if (args.size() != 2)
		return false;

	// First argument is a player ID.
	CEntityId id;
	id.fromString( args[0].c_str() );
	CCharacter *c = PlayerManager.getChar(id);

	if (!c)
	{
		log.displayNL("Invalid character ID specified.");
		return false;
	}

	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	if (theClan == PVP_CLAN::Unknown)
	{
		log.displayNL("Invalid clan name specified.");
		return false;
	}

	if (c->setDeclaredCult(theClan))
	{
		log.displayNL("Player's cult allegiance was changed.");
	}
	else
	{
		log.displayNL("Player's cult allegiance was NOT changed. Does player have required fame? Does new clan match guild allowances?");
	}

	return true;
}

NLMISC_COMMAND (declareCharacterCiv, "Make character declare a specific civilization.", "<Eid> <Faction>")
{
	if (args.size() != 2)
		return false;

	// First argument is a player ID.
	CEntityId id;
	id.fromString( args[0].c_str() );
	CCharacter *c = PlayerManager.getChar(id);

	if (!c)
	{
		log.displayNL("Invalid character ID specified.");
		return false;
	}

	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	if (theClan == PVP_CLAN::Unknown)
	{
		log.displayNL("Invalid clan name specified.");
		return false;
	}

	if (c->setDeclaredCiv(theClan))
	{
		log.displayNL("Player's civilization allegiance was changed.");
	}
	else
	{
		log.displayNL("Player's civilization allegiance was NOT changed. Does player have required fame? Does new clan match guild allowances?");
	}

	return true;
}

NLMISC_COMMAND (adjustCharacterFame, "For a character, adjust a specific clan by indicated fame value.", "<Eid> <Faction> <[-]deltaFameChangeValue>")
{
	if (args.size() != 3)
		return false;

	// First argument is a player ID.
	CEntityId id;
	id.fromString( args[0].c_str() );

	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	uint32 factionIndex;

	if (theClan == PVP_CLAN::Unknown)
	{
		// Command may contains a faction name
		factionIndex = CStaticFames::getInstance().getFactionIndex(args[1]);
		if( factionIndex == CStaticFames::INVALID_FACTION_INDEX )
		{
			log.displayNL("Invalid clan or faction name specified.");
			return false;
		}
	}
	else
	{
		factionIndex = PVP_CLAN::getFactionIndex(theClan);
	}

	// Third argument is the value
	sint32 fameAdjustment;
	NLMISC::fromString(args[2], fameAdjustment);

	CFameInterface::getInstance().addFameIndexed(id, factionIndex, fameAdjustment);
	log.displayNL("Character's new fame value: %d",CFameInterface::getInstance().getFameIndexed(id,factionIndex));

	// We don't inform the client right now, the timer will take care of this
	//character->sendEventForMissionAvailabilityCheck();

	return true;
}

NLMISC_COMMAND (declareGuildCult, "Make guild declare a specific cult", "<Guild Name> <Faction>")
{
	if (args.size() != 2)
		return false;

	GET_GUILD(true);
//	// First argument is a guild .
//	CGuild *g = CGuildManager::getInstance()->getGuildByName( args[0] );
//
//	if (!g)
//	{
//		log.displayNL("Invalid guild ID specified.");
//		return false;
//	}
//
//	if (g->isProxy())
//	{
//		log.displayNL("Guild is a foreign guild, forbidden.");
//		return false;
//	}

	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	if (theClan == PVP_CLAN::Unknown)
	{
		log.displayNL("Invalid clan name specified.");
		return false;
	}

	if (guild->setDeclaredCult(theClan))
	{
		log.displayNL("Guild's allegiance was changed.");
	}
	else
	{
		log.displayNL("Guild's allegiance was NOT changed. Does guild have required fame?");
	}

	return true;
}

NLMISC_COMMAND (declareGuildCiv, "Make guild declare a specific civilization", "<Guild Name> <Faction>")
{
	if (args.size() != 2)
		return false;

	// First argument is a guild .
	GET_GUILD(true);
//	CGuild *g = CGuildManager::getInstance()->getGuildByName( args[0] );
//
//	if (!g)
//	{
//		log.displayNL("Invalid guild ID specified.");
//		return false;
//	}

	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	if (theClan == PVP_CLAN::Unknown)
	{
		log.displayNL("Invalid clan name specified.");
		return false;
	}

	if (guild->setDeclaredCiv(theClan))
	{
		log.displayNL("Guild's allegiance was changed.");
	}
	else
	{
		log.displayNL("Guild's allegiance was NOT changed. Does guild have required fame?");
	}

	return true;
}

NLMISC_COMMAND (adjustGuildFame, "For a guild, adjust a specific clan by indicated fame value.", "<Guild Name> <Faction> <[-]deltaFameValue")
{
	if (args.size() != 3)
		return false;

	// First argument is a guild name.
	GET_GUILD(true);
//	CGuild *g = CGuildManager::getInstance()->getGuildByName(args[0]);
//	if( g == 0)
//	{
//		log.displayNL("Invalid guild name specified.");
//		return false;
//	}
	CEntityId id = guild->getEId();


	// Second argument is a clan.
	PVP_CLAN::TPVPClan theClan = PVP_CLAN::fromString(args[1]);
	
	if (theClan == PVP_CLAN::Unknown)
	{
		log.displayNL("Invalid clan name specified.");
		return false;
	}

	// Third argument is the value
	sint32 fameAdjustment;
	NLMISC::fromString(args[2], fameAdjustment);

	CFameInterface::getInstance().addFameIndexed(id, PVP_CLAN::getFactionIndex(theClan), fameAdjustment);
	log.displayNL("Guild's new fame value: %d",CFameInterface::getInstance().getFameIndexed(id,PVP_CLAN::getFactionIndex(theClan)));

	return true;
}

NLMISC_COMMAND (testit, "testit", "")
{
	PVP_CLAN::TPVPClan pCult, pCiv, tClan;

	if (args.size() != 3)
	{
		pCult = PVP_CLAN::Kami;
		pCiv = PVP_CLAN::Fyros;
		tClan = PVP_CLAN::Fyros;
	}
	else
	{
		pCult = PVP_CLAN::fromString(args[0]);
		pCiv = PVP_CLAN::fromString(args[1]);
		tClan = PVP_CLAN::fromString(args[2]);
	}

	//int retval = CFameManager::getInstance().getStartFame(pCiv,tClan);
	int retval = CFameManager::getInstance().getMaxFameByClan(std::make_pair(pCult,pCiv),tClan);
	log.displayNL("Fame value = %d.", retval);

	return true;
}
