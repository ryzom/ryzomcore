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




#ifndef GS_FAME_H
#define GS_FAME_H

#include "nel/misc/string_mapper.h"
#include "mirrored_data_set.h"
#include "mirror_prop_value.h"
#include "pvp_clan.h"
#include "nel/misc/variable.h"

/** NB : if you change the MAX_FACTION, you MUST also do the following :
 *			1 - change the mirror description file data_share/mirror_sheet/fame.dataset to add the additionnal entry (or remove if you reduce the value)
 *			2 - change the excel worksheet \\server\leveldesign\static_fame.xls :
 *						* resize the sheet (on x and y axis, the sheet must remain square),
 *						* update the macro Module1.ExportFame ligne 6 : Range("A1:CV100").Select : update the range to the new worksheet size
 */
const uint		MAX_FACTION	= 100;
const sint32	NO_FAME			= 0x7FFFFFFF;
extern NLMISC::CVariable<bool> UseAsymmetricStaticFames;


/** Give a simple access to
 *	all the static fame values.
 *	Static fames values include tribe to tribe and tribe to civilisation fames.
 */
class CStaticFames
{
public:
	class CTribeCultThreshold
	{
	public:
		CTribeCultThreshold()
		{
			Kami = 0;
			Karavan = 0;
			Neutral = 0;
		}

		void setKami(sint32 t) { Kami = t; }
		void setKaravan(sint32 t) { Karavan = t; }
		void setNeutral(sint32 t) { Neutral = t; }

		sint32 getKami() const { return Kami; }
		sint32 getKaravan() const { return Karavan; }
		sint32 getNeutral() const { return Neutral; }

	private:
		sint32 Kami;
		sint32 Karavan;
		sint32 Neutral;
	};

	class CTribeCultThresholdPerCiv
	{
	public:
		bool getCultThresholdForCiv( PVP_CLAN::TPVPClan civ, sint32& kami, sint32& karavan, sint32& neutral) const
		{
			const CTribeCultThreshold * tc = 0;
			switch( civ )
			{
			case PVP_CLAN::Matis:
				tc = &Matis;
				break;
			case PVP_CLAN::Fyros:
				tc = &Fyros;
				break;
			case PVP_CLAN::Tryker:
				tc = &Tryker;
				break;
			case PVP_CLAN::Neutral:
				tc = &Neutral;
				break;
			default:
				return false;
			}
			kami = tc->getKami();
			karavan = tc->getKaravan();
			neutral = tc->getNeutral();
			return true;
		}

		uint32				FameIndex;
		CTribeCultThreshold Matis;
		CTribeCultThreshold Fyros;
		CTribeCultThreshold Tryker;
		CTribeCultThreshold Zorai;
		CTribeCultThreshold Neutral;
	};

	// declare scoped constant value
	enum
	{
		/// Tag value for invalid faction index.
		INVALID_FACTION_INDEX = 0xffffffff
	};

	static CStaticFames &getInstance()
	{
		if (_Instance == NULL)
			_Instance = new CStaticFames();

		return *_Instance;
	}

	// release singleton
	static void releaseInstance();

	// destructor
	~CStaticFames();

	/// Get the list of faction mapped names.
	const std::vector<NLMISC::TStringId> &getFactionNames()		{ return _FactionNames; }

	/// Get the name of a faction.
	const std::string &getFactionName(uint32 factionIndex);

	/// Convert the mapped name of a faction into an indexed value (allowing very fast access).
	uint	getFactionIndex(NLMISC::TStringId factionName);
	/// Same as above but with that name as a string.
	uint	getFactionIndex(const std::string &factionName);

	/** Get the fame value between two factions. There is no constraint on faction order because
	 *	fame values are reciprocal.
	 */
	sint32	getStaticFame(NLMISC::TStringId faction1, NLMISC::TStringId faction2);

	/// Same as above but with string name instead of mapped string.
	sint32	getStaticFame(const std::string &faction1, const std::string &faction2);

	/// Same as above but direct index instead of mapped string.
	sint32	getStaticFameIndexed(uint factionIndex1, uint factionIndex2);

	/** Get the propagation factor value between two factions. There is no constraint on faction order because
	 *	values are reciprocal.
	 */
	float	getPropagationFactorIndexed(uint factionIndex1, uint factionIndex2) const;

	/// get the sheet associated with a faction
	const NLMISC::CSheetId & getFactionSheet(uint32 factionIndex)
	{
		if (factionIndex < _FactionSheets.size())
			return _FactionSheets[factionIndex];
		else
			return NLMISC::CSheetId::Unknown;
	}

	/// get the sheet associated with a faction
	uint getFactionIndexFromSheet(const NLMISC::CSheetId & sheet)
	{
		TFactionNameSheetList::iterator it = _FactionSheetIndex.find( sheet );
		if ( it != _FactionSheetIndex.end() )
			return (*it).second;
		else
			return INVALID_FACTION_INDEX;
	}

	// get client database index from fame index
	uint getDatabaseIndex( uint fameIndex ) { nlassert( _FameIndexToDatabaseIndex.size() > fameIndex ); return _FameIndexToDatabaseIndex[ fameIndex ]; }

	// get first tribe fame index
	uint getFirstTribeFameIndex() { return _FirstTribeFameIndex; }

	// get number of tribe fame index
	uint getNbTribeFameIndex() { return _FameTableSize - _FirstTribeFameIndex; }

	// return number of fame in table
	uint getNbFame() { return _FameTableSize; }

	// get tribe threshold per civ allegiance
	const std::vector<CTribeCultThresholdPerCiv>& getTribeThresholdVector() const { return _TribeCultThresholdPerCiv; }

private:

	/// private ctor
	CStaticFames();

	/// static fame loader
	void loadStaticFame( const std::string& filename );

	/// static fame tribe threshold
	void loadTribeThreshold( const std::string& filename );

	/// singleton instance
	static	CStaticFames *_Instance;

	/// faction name index table
	typedef	CHashMap<NLMISC::TStringId, uint, NLMISC::CStringIdHashMapTraits>	TFactionNameIndexList;
	TFactionNameIndexList	_FactionNameIndex;
	std::vector<NLMISC::TStringId>			_FactionNames;

	// faction databaseIndex <=> fameIndex translation table
	std::vector< uint >						_FameIndexToDatabaseIndex;
	uint									_FirstTribeFameIndex;

	/// faction sheet - name table
	typedef	CHashMap<NLMISC::CSheetId, uint, NLMISC::CSheetIdHashMapTraits>	TFactionNameSheetList;
	TFactionNameSheetList	_FactionSheetIndex;
	std::vector<NLMISC::CSheetId>			_FactionSheets;

	/// cult threshold for tribe depending of civilization allegiance
	std::vector<CTribeCultThresholdPerCiv>	_TribeCultThresholdPerCiv;

	/// Size of the fame table (the table is a square)
	uint		_FameTableSize;
	/// fame table
	sint32		*_FameTable;
	/// propagation factor table
	float		*_PropagationFactorTable;
};

/** Access and effect on all the fame in the world.
 *	This class can either static fame or dynamic fame.
 *	Static fame are initial fame between civilisation,
 *	fame between civilisation and tribe and beween tribe.
 *	Dynamic fame include civilisation to civilisation,
 *	guild to civilisation, guild to tribe, player to
 *	civilisation and player to tribe.
 */
class CFameInterface
{
public:
	friend class CFameManager;
	/// Singleton accessor
	static CFameInterface &getInstance()
	{
		if (_Instance == NULL)
		{
			_Instance = new CFameInterface;
		}

		return *_Instance;
	}

	/// Initialise the dataset pointer (call this when mirror is ready).
	void setFameDataSet(CMirroredDataSet *fameDataSet, bool declareProperty = true);

	/// get back the fame dataset
	CMirroredDataSet *getFameDataSet()		{	return _FameDataSet; }

	/** Method to call when a fame entry is created in the mirror.
	 *	This method is called from the service mirror notification callback.
	 */
	void	createFameOwner(CMirroredDataSet& dataSet, const TDataSetRow& entityRow)
	{
		nlassertex(_FameDataSet, ("You must set the fame dataset pointer before calling this function"));
		nlassert(_FamesOwners.find(entityRow) == _FamesOwners.end());

		TFameOwner	*fo = new TFameOwner(dataSet, entityRow);
		_FamesOwners.insert(std::make_pair(entityRow, fo));
	}
	/** Method to call when a fame entry is deleted from the mirror.
	 *	This method is called from the service mirror notification callback.
	 */
	void	removeFameOwner(CMirroredDataSet& /* dataSet */, const TDataSetRow& entityRow)
	{
		nlassertex(_FameDataSet, ("You must set the fame dataset pointer before calling this function"));
		TFameContainer::iterator it(_FamesOwners.find(entityRow));

		nlassert(it != _FamesOwners.end());
		delete it->second;
		_FamesOwners.erase(it);
	}

	//@{
	//@name Fame querying
	/** Get the fame of a civilisation, guild or player versus a tribe or civilisation
	 *	The name is a mapped name and is the same as in CStaticFame class.
	 */
	sint32	getFame(const NLMISC::CEntityId &entityId, NLMISC::TStringId faction, bool modulated = false, bool returnUnknowValue = false);
	/** Get the fame of a civilisation, guild or player versus a tribe or civilisation
	 *	The name is a mapped name and is the same as in CStaticFame class.
	 */
	sint32	getFameIndexed(const NLMISC::CEntityId &entityId, uint32 factionIndex, bool modulated = false, bool returnUnknowValue = false);
	//@}

	//@{
	//@name Fame modification
	/** Add some fame value for a civilisation, a guild or a player vs a tribe or civilisation.
	 *	The value can positive or negative.
	 *	Fame modifications are not immediately received because it's the EGS that apply the value.
	 */
	void	addFame(const NLMISC::CEntityId &entityId, NLMISC::TStringId faction, sint32 deltaFame, bool propagate = false);
	/** Add some fame value for a civilisation, a guild or a player vs a tribe or civilisation.
	 *	The value can positive or negative.
	 *	Fame modifications are not immediately received because it's the EGS that apply the value.
	 */
	void	addFameIndexed(const NLMISC::CEntityId &entityId, uint32 factionIndex, sint32 deltaFame, bool propagate = false);
	/** Set the value of the fame.
	 */
	//void	setFame(const NLMISC::CEntityId &entityId, NLMISC::TStringId faction, sint32 fameValue);
 	//@}

	//@{
	//@name Indirect fame feature querying.
	/// Get the civilisation datasetrow for the given entity.
	const TDataSetRow	&getCivilisationIndex(const NLMISC::CEntityId &entityId);
	/// Get the guild datasetrow for the given entity.
	const TDataSetRow	&getGuildIndex(const NLMISC::CEntityId &entityId);
	/// Get the fame memory datasetrow for the given entity.
	const TDataSetRow	&getFameMemoryIndex(const NLMISC::CEntityId &entityId);
	//@}

	/** Interface class designed to implemented and registered in the FameInterface by the
	 *	EGS services.
	 *	This class directly receive fame delta from addFame.
	 *	You must not use this feature with more than one service ! (only the EGS actualy).
	 */
	class IFameOverload
	{
	public:
		/** Receive fame delta value.
		 *	@param entityIndex index inside the fame dataset.
		 *	@param factionIndex index of the faction (as defined in CStaticFame).
		 *	@param deltaFame The delta fame value.
		 */
		virtual void addFameIndexed(const NLMISC::CEntityId &entityId, uint factionIndex, sint32 deltaFame, bool propagate) =0;
		/** Get the fame of a civilisation, guild or player versus a tribe or civilisation
		 *	The name is a mapped name and is the same as in CStaticFame class.
		 */
		virtual sint32 getFameIndexed(const NLMISC::CEntityId &entityId, uint32 factionIndex, bool modulated = true, bool returnUnknownValue = false) =0;
		virtual const TDataSetRow &getCivilisationIndex(const NLMISC::CEntityId &entityId) =0;
		virtual const TDataSetRow &getGuildIndex(const NLMISC::CEntityId &entityId) =0;
		virtual const TDataSetRow &getFameMemoryIndex(const NLMISC::CEntityId &entityId) =0;
	};
	/// Register the fame adder interface.
	void	registerFameOverload(IFameOverload *fameOverload)	{	_FameOverload = fameOverload; }

	// get faction index from faction name
	sint32	getFactionIndex(NLMISC::TStringId faction);

private:

	CFameInterface()
		: _FameDataSet(NULL),
		_FameOverload(NULL)
	{}

	// fame data for civilisation, guild or player entity
	struct TFameOwner
	{
		/// Civilisation index inside the fame dataset (valid for player and guild entity)
		CMirrorPropValueRO<TDataSetRow>		Civilisation;
		/// Guild index inside the fame dataset (valid for player only)
		CMirrorPropValueRO<TDataSetRow>		Guild;
		/// Fame memory index inside the fame dataset (valid for player only)
		CMirrorPropValueRO<TDataSetRow>		FameMemory;
		// declare the 100 fame mirror value
		CMirrorPropValueRO<sint32>			Fames[MAX_FACTION];

		TFameOwner(CMirroredDataSet& dataSet, const TDataSetRow& entityRow)
		{
			// WARNING : you must initialise your property in the same order as they are declared in the dataset
			Civilisation.init(dataSet, entityRow, CivilisationPropIndex);
			Guild.init(dataSet, entityRow, GuildPropIndex);
			FameMemory.init(dataSet, entityRow, FameMemoryPropIndex);

			for (uint i=0; i<MAX_FACTION; ++i)
			{
				Fames[i].init(dataSet, entityRow, FirstFamePropIndex+i);
			}
		}

		/// Property index
		static sint16	CivilisationPropIndex;
		static sint16	GuildPropIndex;
		static sint16	FameMemoryPropIndex;
		static sint16	FirstFamePropIndex;
	};

	/// pointer to the fame dataset
	CMirroredDataSet		*_FameDataSet;

	typedef CHashMap<TDataSetRow, TFameOwner*, TDataSetRow::CHashCode>	TFameContainer;
	/// storage for fame owner
	TFameContainer			_FamesOwners;

	/// EGS local fame adder.
	IFameOverload			*_FameOverload;

	/// Singleton instance pointer.
	static CFameInterface	*_Instance;
};

#endif // GS_FAME_H
