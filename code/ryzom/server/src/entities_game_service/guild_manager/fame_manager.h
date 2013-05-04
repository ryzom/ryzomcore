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




#ifndef FAME_MANAGER_H
#define FAME_MANAGER_H

#include "game_share/fame.h"
#include "game_share/people.h"
#include "game_share/pvp_clan.h"
#include "egs_pd.h"
#include "egs_variables.h"
#include "nel/misc/variable.h"

/*
struct TFameContainerEntry
{
	sint32		Fame;
	sint32		FameMemory;
	EGSPD::CFameTrend::TFameTrend	LastFameChangeTrend;

	TFameContainerEntry()
	{
		Fame = NO_FAME;
		FameMemory = NO_FAME;
		LastFameChangeTrend = EGSPD::CFameTrend::FameSteady;
	}

	void serial(NLMISC::IStream &f)
	{
		f.serial(Fame);
		f.serial(FameMemory);
		f.serialEnum(LastFameChangeTrend);
	}
};
*/
/*
// class for serial and temporary store of fame
class CFameContainer : public NLMISC::CRefCount
{
	friend class CFameManager;

	/// Container for factions values
	std::map<uint32, TFameContainerEntry>	_FactionInfos;
	/// Contained fame value
//	std::map<sint32, sint32>		_Fames;
	/// Contained memory fame value (for player)
//	std::map<sint32, sint32>		_FamesMemory;
	/// Date of last guild status change (for player)
	NLMISC::TGameCycle		_LastGuildStatusChange;
	/// Date of last fame variation (used to reset variation indicator after a timeout)
	NLMISC::TGameCycle		_LastFameChangeDate;
	/// Tendance of last fame variation, reseted after timeout
//	std::map<sint32, TFameTrend>	_LastFameChangeTrends;
public:

	CFameContainer()
		: _LastGuildStatusChange(0),
		_LastFameChangeDate(0)
	{}

	/// serial the fame value
	void serial(NLMISC::IStream &f);

	uint32 getCurrentVersion();
};
*/

class CFameManager : public CFameInterface::IFameOverload
{
public:
	static CFameManager &getInstance()
	{
		if (!_Instance)
		{
			_Instance = new CFameManager;
		}

		return *_Instance;
	}

	/// init the mirror data when ready.
	void mirrorIsReady();
	/// add civilisation entity when ready
	void mirrorReadyToAdd();

	/// Add a player in the fame mirror and restore the fame value stored in fameContainer.
	void addPlayer(const NLMISC::CEntityId &playerId, const EGSPD::CFameContainerPD &fameContainer, EGSPD::CPeople::TPeople civilisation);
	/// Save the current fame value of the player in the fameContainer
	void savePlayerFame(const NLMISC::CEntityId &playerId, EGSPD::CFameContainerPD &fameContainer);
	/// Remove a player from the fame mirror.
	void removePlayer(const NLMISC::CEntityId &playerId);

	/// Add a guild in the fame mirror and restore the fame value stored in fame container.
	void addGuild(const NLMISC::CEntityId &guildId, const EGSPD::CFameContainerPD &fameContainer, EGSPD::CPeople::TPeople civilisation);
	/// Update the fame mirror with new value (used for guild replication between shard)
	void updateGuildFame(const NLMISC::CEntityId &guildId, const EGSPD::CFameContainerPD &fameContainer);
	/// Save the current fame value of the player in the fameContainer
	void saveGuildFame(const NLMISC::CEntityId &guildId, EGSPD::CFameContainerPD &fameContainer);
	/// Remove a guild from the fame mirror and save the fame value in the fame container.
	void removeGuild(const NLMISC::CEntityId &guildId);

	/// set the guild id of a player
	void setPlayerGuild( const NLMISC::CEntityId & playerId, const NLMISC::CEntityId & guildId, bool resetLastGuildStatusChange);
	/// clear the player guild
	void clearPlayerGuild( const NLMISC::CEntityId & playerId );

//	void createPlayerFame(CEntityId playerId);
//	void createGuildFame(CEntityId guildId);

	/// serial an entity fame in the specified stream ( works for guild/civ/players )
//	void serialFames( const NLMISC::CEntityId & eId, NLMISC::IStream & f);

	/// Update the fame memory value, one player at each tick
	void tickUpdate();

	/// update the fame value (interpolate memory) for a player.
	void updatePlayerFame(const TDataSetRow &playerIndex);
	/// update the fame trend for all entity
	void updateFameTrend(const TDataSetRow &entityIndex);

	EGSPD::CFameTrend::TFameTrend	getFameTrendIndexed(const TDataSetRow &entityIndex, uint32 factionIndex);
	float		getPlayerMemoryBlendIndexed(const TDataSetRow &entityIndex);

	/// set entity fame without propagation (player character or guild)
	void setEntityFame(const NLMISC::CEntityId & entityId, uint32 faction, sint32 fame, bool setDirectValue=false);

	// Fame system
	//  These take clans as parameters, restricted by call.
	//  These return the proper fame values, or NO_FAME if there's an error.
	// - GgtStartFame: playerClan must be a Civilization, targetClan must be any non-neutral clan
	sint32 getStartFame(PVP_CLAN::TPVPClan playerClan, PVP_CLAN::TPVPClan targetClan);
	// - getMaxFameByClan: playerClan must be Neutral or the same type (Cult or Clan) as targetClan,
	//   targetClan must be any non-neutral clan.
	sint32 getMaxFameByClan(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> playerClans, PVP_CLAN::TPVPClan targetClan);
	sint32 getMaxFameByFactionIndex(std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> playerClans, uint32 factionIndex);
	
	// Ensures that the fame values are properly capped based on allegiance.
	void enforceFameCaps(const NLMISC::CEntityId &entityId, std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance);

	// Set tribe fame cap and ensures fame values are properly capped, based on allegiance
	void setAndEnforceTribeFameCap(const NLMISC::CEntityId &entityId, std::pair<PVP_CLAN::TPVPClan, PVP_CLAN::TPVPClan> allegiance);

	// Callback function for when one of the threshold CVariables are changed.
	static void thresholdChanged(NLMISC::IVariable &var);

private:

	CFameManager();

	/// set the civilisation id of a guild
	void setGuildCivilisation( const NLMISC::CEntityId & playerId, EGSPD::CPeople::TPeople civilisation );
	/// set the civilisation id of a player
	void setPlayerCivilisation( const NLMISC::CEntityId & playerId, EGSPD::CPeople::TPeople civilisation );

	// WARNING: this values are used to translate fame related phrases, DO NOT change them
	enum TFamePropagation
	{
		no_propagation		= 0,
		ally_propagation	= 1,
		enemy_propagation	= 2
	};

	void	 addFameIndexed(const NLMISC::CEntityId &entityId, uint32 faction, sint32 deltaFame, const std::string &serviceName, bool propagate, TFamePropagation propagationType = no_propagation);
	/// FameInterface adder implementation.
	virtual void addFameIndexed(const NLMISC::CEntityId &entityId, uint factionIndex, sint32 deltaFame, bool propagate);
	/// Fame interface get fame implementation
	sint32	getFameIndexed(const NLMISC::CEntityId &entityId, uint32 factionIndex, bool modulated = false, bool returnUnknownValue = false);
	virtual const TDataSetRow &getCivilisationIndex(const NLMISC::CEntityId &entityId);
	virtual const TDataSetRow &getGuildIndex(const NLMISC::CEntityId &entityId);
	virtual const TDataSetRow &getFameMemoryIndex(const NLMISC::CEntityId &entityId);

	static void cbFameDelta( NLNET::CMessage& msgin, const std::string &serviceName, NLNET::TServiceId serviceId );

	// fame data for civilisation, guild or player entity
	struct TFameOwnerWrite
	{
		NL_INSTANCE_COUNTER_DECL(TFameOwnerWrite);
	public:
		/// Civilisation index inside the fame dataset (valid for player and guild entity)
		CMirrorPropValueAlice<TDataSetRow>		Civilisation;
		/// Guild index inside the fame dataset (valid for player only)
		CMirrorPropValueAlice<TDataSetRow>		Guild;
		/// Fame memory index inside the fame dataset (valid for player only)
		CMirrorPropValueAlice<TDataSetRow>		FameMemory;
		/// declare the 100 fame mirror value
		CMirrorPropValueAlice<sint32>			Fames[MAX_FACTION];

		/** Fame memory. Used for player to remember guild fame status when they change/join/quit guild	
		 *	The reason for this is to reduce the resulting fame variation when the player change/join/quit a guild.
		 *	The table will receive the current evaluated player guild fame when the player guild status change.
		 *	Then, a progressive interpolation will slop the evaluated player guild fame from the new player guild fame 
		 *	value.
		 */
		sint32		LastGuildFame[MAX_FACTION];
		/// Date of the last player guild status change. 0 if not pertinent (either no change or last change is too old).
		NLMISC::TGameCycle	LastGuildStatusChange;
		/// Date of last fame variation (used to reset variation indicator after a timeout)
		NLMISC::TGameCycle	LastFameChangeDate;
		/// Tendance of last fame variation, reseted after timeout
		EGSPD::CFameTrend::TFameTrend		LastFameChangeTrends[MAX_FACTION];


		TFameOwnerWrite(CMirroredDataSet& dataSet, const TDataSetRow& entityRow)
		{
			Civilisation.init(dataSet, entityRow, CivilisationPropIndex);
			Civilisation = TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW );
			Guild.init(dataSet, entityRow, GuildPropIndex);
			Guild = TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW );
			FameMemory.init(dataSet, entityRow, FameMemoryPropIndex);
			FameMemory = TDataSetRow::createFromRawIndex(INVALID_DATASET_ROW );
			LastGuildStatusChange = 0;
			LastFameChangeDate = 0;
			
			for (uint i=0; i<MAX_FACTION; ++i)
			{
				Fames[i].init(dataSet, entityRow, FirstFamePropIndex+i);
				Fames[i] = NO_FAME;
				LastGuildFame[i] = NO_FAME;
				LastFameChangeTrends[i] = EGSPD::CFameTrend::FameSteady;
			}
		}

		/// Property index
		static sint16	CivilisationPropIndex;
		static sint16	GuildPropIndex;
		static sint16	FameMemoryPropIndex;
		static sint16	FirstFamePropIndex;
	};


	typedef CHashMap<TDataSetRow, TFameOwnerWrite*, TDataSetRow::CHashCode>	TFameContainer;
	/// storage for fame owner
	TFameContainer			_FamesOwners;

	/// Singleton instance.
	static CFameManager	*_Instance;

	/// last updated row index
	TDataSetRow			_LastUpdatedRow;

	/// For Tribes Threshold. FactionIndex to TribeIndex
	std::vector<uint32>		_FactionIndexToTribeThresholdIndex;
	// :OPTIM: init/doInit is dual to inline the _FactionIndexToTribeThresholdIndex.empty() test
	void		initTribeThresholdIndex()
	{
		if(_FactionIndexToTribeThresholdIndex.empty())
			doInitTribeThresholdIndex();
	}
	void		doInitTribeThresholdIndex();

	// FameStart - starting values for fame, [player's group][target group]
	// - There are no None or Neutral clans for starting fame, so leave those out.
	//   Also, you can't start as Kami or Karavan, so we begin at the Civilisation beginning.
	//   To reference these values, take the TPVPClan value and subtract the start value.
	NLMISC::CVariable<sint32>* FameStart[1+PVP_CLAN::EndCivs-PVP_CLAN::BeginCivs][PVP_CLAN::NbClans-PVP_CLAN::BeginClans];
	// FameMaxCult - maximum values for fame for Cults, [player's group][target group]
	// - Add 1 to the row to account for Neutrals.
	NLMISC::CVariable<sint32>* FameMaxCults[2+PVP_CLAN::EndCults-PVP_CLAN::BeginCults][1+PVP_CLAN::EndCults-PVP_CLAN::BeginCults];
	// FameMaxCiv - maximum values for fame for Civilisations, [player's group][target group]
	// - Add 1 to the row to account for Neutrals.
	NLMISC::CVariable<sint32>* FameMaxCivs[2+PVP_CLAN::EndCivs-PVP_CLAN::BeginCivs][1+PVP_CLAN::EndCivs-PVP_CLAN::BeginCivs];

	NLMISC_COMMAND_FRIEND(setFameMemory);
};


#endif




