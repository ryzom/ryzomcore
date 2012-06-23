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



#ifndef RY_DEPOSIT_H
#define RY_DEPOSIT_H

#include "nel/ligo/primitive.h"
#include "egs_sheets/egs_static_deposit.h"
#include "game_share/time_weather_season/time_and_season.h"
#include "game_share/ecosystem.h"
#include "game_share/rm_family.h"
#include "harvest_info.h"
#include "mission_manager/ai_alias_translator.h"
#include "egs_variables.h"
#include "nel/misc/random.h"
#include "nel/misc/variable.h"


class CDeposit;

const uint16 MaxNbActiveSources = (uint16)~0;

/**
 * A recent forage site prevents from extracting too much material from the same place in a short time.
 * When an extraction is succesful, a timer is launched. If no more extraction is done then, the site
 * is destroyed after the time is elapsed. If an extraction fails (no more stock available), the site
 * will be blocked for extraction (isDepleted() will be true) during the specified time, then freed.
 * Note: the timer resolution is abstract. The actual time in ticks will depend on the rate of update.
 */
class CRecentForageSite
{
public:

	/// Create forage site. Precondition: dep not NULL.
	CRecentForageSite( CDeposit *dep, const NLMISC::CVector& pos ) : _Deposit(dep), _Pos(pos), _TimeToLive(ForageSiteNbUpdatesToLive.get()), _LowScopeStock(ForageSiteStock.get()), _NbActiveSources(0) {}

	/// Add a source
	bool					addActiveSource() { if ( _NbActiveSources == MaxNbActiveSources ) return false; ++_NbActiveSources; return true; }

	/// Remove a source
	void					removeActiveSource() { nlassert(_NbActiveSources!=0); --_NbActiveSources; } // TEMP: assert

	/// Return the quantity available in the deposit (or FLT_MAX if there is no quantity constraint)
	float					getQuantityInDeposit();

	/**
	 * Consume 1 unit of forage site stock by a source in the (independant on the quantity extracted).
	 * Reset life time of forage site if successful.
	 * Additionally, consume n units of deposit stock (dependant on the quantity extracted).
	 * Return the actual quantity that is extracted (can be less that requested if the deposit is (nearly or fully) empty.
	 */
	float					consume( float n );

	/// Consume all stock at once
	void					depleteAll() { _LowScopeStock = 0; _TimeToLive = ForageSiteNbUpdatesToLive.get(); }

	/** Update forage site. Return false if the forage site lifetime is ended and the object must be destroyed (which won't occur while there are active sources)
	 * This method does not need to be called every tick, but at the rate of CDeposit::lowFreqUpdate().
	 */
	bool					lowFreqUpdate() { if ( _TimeToLive != 0 ) { --_TimeToLive; return true; } else return hasActiveSources(); }

	/// Return deposit (never NULL).
	const CDeposit			*deposit() const { return _Deposit; }

	/// Return center position
	const NLMISC::CVector&	pos() const { return _Pos; }

	/// Test if site is depleted
	bool					isDepleted() const { return _LowScopeStock == 0; }

	/// Test if site contains a position
	bool					contains( const NLMISC::CVector& pos ) const;

	/// Display debug or stat info
	void					display( NLMISC::CLog& log ) const;

	/// True if the owned deposit allo depletion risk
	bool					allowDepletionRisk() const;

protected:

	/// Test if site has sources
	bool					hasActiveSources() const { return _NbActiveSources!=0; }

private:

	/// Pointer to deposit (deposits don't move in memory). Never NULL.
	CDeposit			*_Deposit;

	/// Center of site area
	NLMISC::CVector		_Pos;

	///
	uint16				_TimeToLive;

	///
	uint16				_LowScopeStock;

	///
	uint16				_NbActiveSources;
};

typedef std::list<CRecentForageSite> CRecentForageSites;


/**
 * Type of ecosystem or terrain.
 * CEcotype ojects are loaded from primitives, are used to build CDeposit objects, then erased.
 */
class CEcotypeZone : public NLLIGO::CPrimZone
{
public:
	
	/// Init
	bool			build( const NLLIGO::CPrimZone* zone );

	/// Return ecotype
	TEcotype		ecotype() const { return _Ecotype; }

private:

	///
	TEcotype		_Ecotype;
};

typedef std::vector< CEcotypeZone* > CEcotypeZones;


/**
 * Properties of auto-spawn sources of a deposit
 */
struct CAutoSpawnProperties
{
	NL_INSTANCE_COUNTER_DECL(CAutoSpawnProperties);
public:
	/// In game cycles, but depends of the update frequency of the deposits
	uint32			SpawnPeriodGc;

	/// Time before disappearance when no extraction has started (game cycles)
	uint32			LifeTimeGc;

	/// Time remaining since the beginning of the first extraction
	uint32			ExtractionTimeGc;

	/// Minimium Number of Auto-Spawned Sources that must always exist
	uint32			MinimumSpawnedSources;
};


/**
 * Quantity constraints
 */
struct CQuantityConstraints
{
	NL_INSTANCE_COUNTER_DECL(CQuantityConstraints);
public:
	/// Current quantity (shared by all materials in the deposit)
	float			CurrentQuantity;

	/// Day of the next respawn (valid if CurrentQuantity is 0)
	uint32			NextRespawnDay;

	/// Initial quantity
	uint16			InitialQuantity;

	/// Time before the initial quantity becomes available backs, in Ryzom Days (note: 30 days for a whole season)
	uint16			RespawnTimeRyzomDays;

	/// Return the current quantity. If 0 and the respawn time is elapsed, unlock. 
	float			getCurrentQuantity();

	/// Consume. If the current quantity falls to 0, lock. Return the actual consumed quantity.
	float			consumeQuantity( float consumed );
};


namespace NLMISC
{
	class IVariable;
	class CWordsDictionary;
}

/**
 * \author Nicolas Brigand, Alain Saffray, Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CDeposit : public NLLIGO::CPrimZone, public NLMISC::CRefCount
{
public:

	/// Constructor
	CDeposit() : _AutoSpawnSourcePt(NULL), _QuantityConstraintsPt(NULL), _Ecotype(ECOSYSTEM::common_ecosystem), _FilterPhase(0), _KamiAnger(0.0f), _MinQuality(-1), _MaxQuality(-1), _SourceFXIndex(0), _CanProspect(false), _Enabled(false), _CurrentNbAutoSpawnedSources(0), _AllowDepletionRisk(true) {}

	/// Destructor
	~CDeposit();

	/// Add an ecotype information
	static void		addEcotype( CEcotypeZone *ecotypeZone ) { _EcotypeZones.push_back( ecotypeZone ); }

	/// Init deposit
	bool			build( const NLLIGO::CPrimZone* zone );

	/// Clear all ecotype information, after having built the deposits
	static void		clearEcotypes();

	// Update deposit (especially recent forage sites).
	void			lowFreqUpdate();	

	// Update deposit that need to auto spawn because number of harvest sources is too low
	void			autoSpawnUpdate();	
	
	// fill CHarvestInfos with deposit founded
	///void harvestInfo( const NLMISC::CEntityId& charId, HARVEST_INFOS::CHarvestInfos& infos );
	
	// character take MP, kami eat this fool character ?
	//void characterTakeRM( const NLMISC::CEntityId&  charId, uint32 depositIndexContent, uint16 quantity );
	
	// Display deposit content
	void			displayContent( NLMISC::CLog * log = NLMISC::InfoLog, bool extendedInfo=false, NLMISC::CWordsDictionary *itemDictionary=NULL );

	const std::string& name() const { return _Name; }

	/// Return the number of MPS in the deposit
	uint			getContentSize() const { return (uint)_RawMaterials.size(); }

	/// Return the MPS
	const			std::vector<CStaticDepositRawMaterial>& getContents() const { return _RawMaterials; }

	/// Return the terrain type
	TEcotype		ecotype() const { return _Ecotype; }

	/// Return the MinQuality
	sint16			minQuality() const { return _MinQuality; }

	/// Return the MaxQuality or -1 if it has to be taken from the raw material information
	sint16			maxQuality() const { return _MaxQuality; }

	/// Return the index of the visual shape/FX of the sources from this deposit
	uint16			sourceFXIndex() const { return _SourceFXIndex; }

	/// Return true if the deposit is currently enabled
	bool			enabled() const { return _Enabled; }
	
	/// Enable(true)/Disable(false) the deposit 
	void			enable(bool bEnable) { _Enabled = bEnable; }
	
	/// Return true if the deposit can be prospected
	bool			canProspect() const { return _CanProspect; }

	/// Return true if the submitted terrain matches the deposit
	bool			matchEcotype( TEcotype t ) const
					{ return t==_Ecotype; }

	/// Return true if the submitted season matches the deposit
	bool			matchSeason( CRyzomTime::ESeason s ) const
					{ return _SeasonFilter.empty() || (std::find( _SeasonFilter.begin(), _SeasonFilter.end(), s ) != _SeasonFilter.end()); }

	/// Return true if the submitted weather matches the deposit
	bool			matchWeather( CRyzomTime::EWeather w ) const
					{ return _WeatherFilter.empty() || (std::find( _WeatherFilter.begin(), _WeatherFilter.end(), w ) != _WeatherFilter.end()); }

	/// Return true if the submitted time of day matches the deposit
	bool			matchTimeOfDay( CRyzomTime::ETimeOfDay t ) const
					{ return _TimeOfDayFilter.empty() || (std::find( _TimeOfDayFilter.begin(), _TimeOfDayFilter.end(), t ) != _TimeOfDayFilter.end()); }

	/// Return true if the deposit contains at least one RM of the specified family
	bool			hasFamily( RM_FAMILY::TRMFamily family ) const;

	/// Return true if the deposit contains at least one RM of the specified group
	bool			hasGroup( RM_GROUP::TRMGroup group ) const;

	/// Return true if the deposit contains at least one RM than can craft the specified item part
	bool			hasRMForItemPart( uint itemPartIndex ) const;

	/// Return true if the deposit contains at least one RM with energy lower_eq than the specified value
	bool			hasLowerStatEnergy( uint8 maxStatEnergy ) const;

	/// Return true if the deposit contains at least one RM with energy equalling the specifing value
	bool			hasExactStatEnergy( uint8 statEnergy ) const;

	/// Return the kami anger level (or -1 if disabled)
	float			kamiAnger() const { return _KamiAnger; }

	/// Set the kami anger level
	void			setKamiAnger( float newKamiAnger ) { _KamiAnger = newKamiAnger; }

	/// Increment the kami anger level. React if a threshold is reached. Return the kami anger level (see kamiAnger())
	float			incKamiAnger( float delta, const std::vector<TDataSetRow>& foragers );

	/// Decrement the kami anger level (result clamped to 0)
	float			decKamiAnger( float delta ) { if ( _KamiAnger != -1.0f ) { _KamiAnger -= delta; if ( _KamiAnger < 0 ) _KamiAnger = 0; } return _KamiAnger; }

	/// True if the deposit allow depletion risk for recent forage site
	bool			allowDepletionRisk() const {return _AllowDepletionRisk;}

	/// Return the auto spawn properties (if auto-spawn is on, otherwise return NULL)
	const CAutoSpawnProperties *getAutoSpawnProperties() const { return _AutoSpawnSourcePt; }

	/// Return the quantuty constraints (if any, otherwise return NULL)
	const CQuantityConstraints *getQuantityConstraints() const { return _QuantityConstraintsPt; }

	/// Return the quantity available in the deposit (or FLT_MAX if there is no quantity constraint)
	float			getMaxQuantity() { return _QuantityConstraintsPt ? _QuantityConstraintsPt->getCurrentQuantity() : FLT_MAX; }

	/// Consume. Return the actual consumed quantity (may be lower if there is no more to get)
	float			consumeQuantity( float requested ) { if ( _QuantityConstraintsPt ) return _QuantityConstraintsPt->consumeQuantity( requested ); else return requested; }

	/**
	 * Get a random RM from the neighbourhood of the specified position.
	 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
	 */
	const CStaticDepositRawMaterial	*getRandomRMAtPos( const NLMISC::CVector& pos, bool testIfSiteDepleted, bool& isDepleted );

	/**
	 * Get the RM at the specified position (no random for map generation).
	 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
	 * \param testIfSiteDepleted Set it to false for map generation.
	 */
	const CStaticDepositRawMaterial *getRMAtPos( const NLMISC::CVector& pos, bool testIfSiteDepleted, bool& isDepleted );
	
	/// Return always a forage site
	CRecentForageSite				*findOrCreateForageSite( const NLMISC::CVector& pos );

	/// Display forage sites info
	void			displayRecentForageSites( NLMISC::CLog& log ) const;

	/// Empty all forage sites
	void			depleteAllRecentForageSites();

	// For auto-spawn source minimum number. Internaly used by CHarvestSource only
	void			decreaseAutoSpawnedSources();
	void			increaseAutoSpawnedSources();
	
protected:

	/// Select the raw materials, using the specified filters and _Ecotype
	void			selectRMsByFilters( std::vector<std::string>& exactRMCodesS, const std::vector<std::string>& rmFamilyFilterS, const std::vector<std::string>& itemPartsFilterS, const std::vector<std::string>& craftCivS, uint minEnergy, uint maxEnergy );

	std::string		getSeasonStr() const;
	std::string		getTimeOfDayStr() const;
	std::string		getWeatherStr() const;

	/**
	 * Get the ecotype zone under the position.
	 * This information is valid only at init time. After the deposits are built, this information
	 * can be found only in deposits.
	 * If not found, a NULL pointer is returned.
	 */
	static CEcotypeZone *getEcotypeZone( const NLMISC::CVector& pos );

	/// auto spawn a source in the deposit, in the given bbox
	void		autoSpawnSource(const NLMISC::CVector &cornerMin, const NLMISC::CVector &cornerMax);

private:

	/// The ecotype zones (only valid at init, cleared after deposit built)
	static CEcotypeZones						_EcotypeZones;


	/// List of raw materials (const)
	std::vector< CStaticDepositRawMaterial >	_RawMaterials;

	/// Type of terrain (const)
	TEcotype									_Ecotype;

	/// Season filter (const)
	std::vector<CRyzomTime::ESeason>			_SeasonFilter;

	/// Weather filter (const)
	std::vector<CRyzomTime::EWeather>			_WeatherFilter;

	/// Time of day filter (const)
	std::vector<CRyzomTime::ETimeOfDay>			_TimeOfDayFilter;

	/// Phase for noise value generator (deduced from season, weather, time... filters)
	uint										_FilterPhase;

	/// Recent forage site list (per deposit, which means forage won't affect another overlapped deposit)
	CRecentForageSites							_RecentForageSites;

	/// Deposit name
	std::string									_Name;

	/// Not null if the deposit can spawn sources without foraging action
	CAutoSpawnProperties						*_AutoSpawnSourcePt;

	/// From 0 to KAMI_ANGER_THRESHOLD 1 & 2, or -1 for N/A; will be persistant some day...
	float										_KamiAnger;

	/// Min quality or -1 if there is no inferior limit
	sint16										_MinQuality;

	/// Max quality or -1 if it has to be taken from the raw material information
	sint16										_MaxQuality;

	/// Not null if the deposit has main quantity constraints
	CQuantityConstraints						*_QuantityConstraintsPt;

	/// Index of the visual shape/FX of the sources from this deposit
	uint16										_SourceFXIndex;

	/// Can a player prospect on this deposit
	bool										_CanProspect;

	/// Is this deposit enabled or disabled (for deposits that are not permanent)
	bool										_Enabled;

	/// Is this deposit allows depletion risk. True by default
	bool										_AllowDepletionRisk;

	// deposit id
	//uint										_Id;

	/// Current Number of AutoSpawned Sources in this deposit
	uint32										_CurrentNbAutoSpawnedSources;
};


/**
 * Predicate to compare two pointed deposits using their kami anger level.
 * If a value is -1, it is not considered as lower.
 */
struct CHasLowerKamiAngerPred : std::binary_function< CDeposit*, CDeposit*, bool >
{
	bool operator() ( CDeposit *d1, CDeposit *d2 )
	{
		return (d1->kamiAnger() >= 0) && (d1->kamiAnger() < d2->kamiAnger());
	}
};


/*
 * Return the quantity available in the deposit (or FLT_MAX if there is no quantity constraint)
 */
inline float	CRecentForageSite::getQuantityInDeposit()
{
	return _Deposit->getMaxQuantity();
}


/*
 * Consume 1 unit of forage site stock by a source in the (independant on the quantity extracted).
 * Reset life time of forage site if successful.
 * Additionally, consume n units of deposit stock (dependant on the quantity extracted).
 * Return the actual quantity that is extracted (can be less that requested if the deposit is (nearly or fully) empty.
 */
inline float	CRecentForageSite::consume( float n )
{
	if ( _LowScopeStock != 0 )
	{
		--_LowScopeStock;
		_TimeToLive = ForageSiteNbUpdatesToLive.get();
	}
	return _Deposit->consumeQuantity( n );
}


/*
 * Return true if the deposit allows depletion risk
 */
inline bool CRecentForageSite::allowDepletionRisk() const
{
	return _Deposit && _Deposit->allowDepletionRisk();
}



#endif // RY_DEPOSIT_H

/* End of deposit.h */

