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



#ifndef NL_HARVEST_SOURCE_H
#define NL_HARVEST_SOURCE_H

#include "nel/misc/vector_2f.h"
#include "game_share/base_types.h"
#include "egs_mirror.h"

#include "phrase_manager/simple_entity_manager.h"
#include "phrase_manager/toxic_cloud.h"

class CStaticDepositRawMaterial;
class CRecentForageSite;
class CEntityBase;
class CDeposit;


namespace VISIBILITY_RIGHTS
{
	/// Visibility rights
	enum TVisGroup
	{
		All,
		Private,
		Team,
		Guild
	};

	/// Convert a numeric value to TVisGroup
	inline TVisGroup	fromInt( uint8 i )
	{
		return (TVisGroup)i;
	}
}


#define _S _RTProps[S]
#define _A _RTProps[A]
#define _Q _RTProps[Q]
#define _D _RTProps[D]
#define _E _RTProps[E]
#define _C _RTProps[C]


/**
 * Forage source
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CHarvestSource
{
	NL_INSTANCE_COUNTER_DECL(CHarvestSource);
public:

	enum TRealTimeProp
	{
		S, NoDrop=S,		// Source speed (in steps per tick (<1)). Not droppable.
		A=1,				// Source aperture (RM quantity delivered per step). Request 0 or "care".
		Q=2, ReduceDmg=Q,	// Average quality of the delivered RM
		NbPosRTProps,
		D = NbPosRTProps,	// Level before source is destroyed / deposit is disabled. Request a negative value for "life absorption by player".
		E,					// Level before damaging events
		//C,				// Level before creature spawn
		NbRTProps
	};

	enum TTargetRTProp		// Subset of TRealTimeProp
	{
		TargetD,
		TargetE,
		NbTargetRTProps
	};

	enum TCareDelta
	{
		DeltaD,
		DeltaE,
		//DeltaC,
		NbCareDeltas
	};

	enum TKamiAngerProp
	{
		ReservedForS,
		KamiOffNum,
		KamiAngerDec,
		NbKamiAngerProps
	};

	/*enum TStatClassForage
	{
		BasicPlainAverage, FinePrime, ChoiceSelect, ExcellentSuperb, SupremeMagnificient, NbStatQualities
	};*/

	struct CReduceDamageEvent
	{
		NLMISC::TGameCycle	Time;
		float				Ratio;
	};

	typedef std::vector<CReduceDamageEvent> CRDEvents;

	typedef std::vector<TDataSetRow> CForagers;

	/// Constructor (a source does not need init() if it's only a template source passed to init())
	CHarvestSource();

	/// Destructor
	~CHarvestSource();

	// Change the properties (for a template source)
	void		setLifetime( NLMISC::TGameCycle t ) { _LifeTime = t; }
	void		setProspectionExtraExtractionTime( NLMISC::TGameCycle t ) { _ExtraExtractionTime = t + _IncludedBonusExtractionTime; } // Locked is forbidden
	void		setBonusExtraExtractionTime( NLMISC::TGameCycle t ) { _ExtraExtractionTime += t - _IncludedBonusExtractionTime; _IncludedBonusExtractionTime = t; }
	void		setS( float p ) { _S = p; }
	void		setA( float p ) { _A = p; }
	void		setQ( float p ) { _Q = p; }
	void		setD( float p ) { _D = p; }
	void		setE( float p ) { _E = p; }
	//void		setC( float p ) { _C = p; }
	void		setN( float p ) { _N = p; }
	//void		setDistVis( sint32 p ) { _DistanceVisibility = p; }
	//void		setStealthVis( VISIBILITY_RIGHTS::TVisGroup r ) { _StealthVisibility = r; }

	/// Init the source. All pointers must be valid (but forageSite may be NULL). Return false if the current quantity in the deposit is 0.
	bool		init( const CHarvestSource& ini,
					  const NLMISC::CVector2f& pos,
					  CRecentForageSite *forageSite,
					  CDeposit *depositForK,
					  const CStaticDepositRawMaterial *rmInfo, float quantityRatio );

	/// Set the deposit that has auto-spawned this source (if any). NULL to unlink the curent (if any)
	void		setDepositAutoSpawn(CDeposit *deposit);
		
	/// Set bonus for quantity ratio
	void		setBonusForA( uint8 percent ) { _BonusForAPct = percent; }

	/**
	 * Prepare the source as an unpublished entity in mirror. Return false in case of failure. 
	 * Must be called *after* init().
	 * prospectorDataSetRow must be either null (isNull()) or an accessible row (isAccessible()).
	 */
	bool		spawnBegin( uint8 knowledgePrecision, const TDataSetRow& prospectorDataSetRow, bool isAutoSpawned );

	/**
	 * Complete the source spawn: publish the entity in mirror or delete it
	 * Caution: if authorized, the source object is deleted!
	 */
	void		spawnEnd( bool authorized );
	
	/// Tick update. Return false if the source's life is ended.
	bool		update();

	/**
	 * Begin an extraction action. Once the extraction process is started, the source remains in
	 * extraction mode until the extraction time is elapsed (even if players stop/restart
	 * extracting). Set isNewbie to true if the extractor should have low risks.
	 * Return true if the forager is the extractor (the first one on the source)
	 */
	bool		beginExtraction( const TDataSetRow& forager, bool isNewbie=false );

	/**
	 * Update the source state with an extraction.
	 * Warning: this can lead to the death of the player (e.g. kami wrath threshold reached)
	 * => player->forageProgress() can become NULL and therefore must be tested.
	 *
	 * \param reqPosProps [in/out] Requested positive props (rS, rA, rQ). Can be lowered in output if a drop occurs.
	 * \param absPosProps [in] Absorption ratio (<=1.0f) of positive props (aS, aA, aQ): a higher value reduces the negative impact of the extraction.
	 * \param qualityCeilingFactor [in] Factor for quality target value (relative to reqPosProps[Q])
	 * \param qualitySlowFactor [in] Quality increase speed factor
	 * \param results [in/out] Resulting positive values (S, A, Q). In input, please provide the previous results.
	 * \param successFactor [in] The success factor ratio to be used.
	 * \param lifeAbsorberRatio [in] In not zero, extractingEntityRow will get some damage, converting lifeAbsorberRatio percent of the decrease of D.
	 * \param extractingEntityRow [in] See lifeAbsorberRatio.
	 * negative impact on the source life (D).
     * \param propDrop [out] NoDrop=all ok; A=quantity drop; Q=quality drop.
	 */
	void		extractMaterial( float *reqPosProps, float *absPosProps, float qualityCeilingFactor, float qualitySlowFactor, float *results, float successFactor, uint8 lifeAbsorberRatio, const TDataSetRow& extractingEntityRow, CHarvestSource::TRealTimeProp& propDrop );

	/// Update the source state with a care (DeltaD, DeltaE, DeltaC). Output: isUseful if the care is not from the "begin zone"
	void		takeCare( float *deltas, bool *isUseful );

	/// Reduce the damage of the next blowing up (will work if between ForageReduceDamageTimeWindow and blowing up time)
	void		reduceBlowingUpDmg( float ratio );

	/// Damage an entity
	static void	hitEntity( RYZOMID::TTypeId aggressorType, CEntityBase *entity, sint32 hpDamageAmount, sint32 hpDamageAmountWithoutArmour, bool isIntentional, sint32 hpAvoided=0 );

	/// Recalculate the remaining extraction time depending on the requested quality
	void		recalcExtractionTime( float requestedQuality );

	/**
	 * Return the prospector or a null datasetrow if there was no prospection (auto-spawn)
	 * The accessibility of this datasetrow must be checked before use.
	 */
	const TDataSetRow&			getProspectorDataSetRow() const;

	// Accessors
	const NLMISC::CSheetId&		materialSheet() const { return _MaterialSheet; }
	const TDataSetRow&			rowId() const { return _DataSetRow; }
	const NLMISC::CVector2f&	pos() const { return _Pos; }
	float						quantity() const { return _N; }
	//NLMISC::TGameCycle		extractionTime() const { return _ExtractionTime + _ExtraExtractionTime; }
	const NLMISC::TGameCycle&	timeToLive() const { return _T; }
	bool						wasProspected() const { return _ExtraExtractionTime != 0; }
	NLMISC::TGameCycle			deliveryPeriod() const { return (NLMISC::TGameCycle)(1.0f / _S); }
	float						speed() const { return _S; }
	float						aperture() const { return _A; }
	float						quality() const { return _Q; }
	float						maxQuality() const { return _MaxQuality; }
	const CRecentForageSite		*forageSite() const { return _ForageSite; }
	CDeposit					*depositForK() const { return _DepositForK; }
	bool						isExtractionInProgress() const { return _IsExtractionInProgress; }
	uint8						nbEventTriggered() const { return _NbEventTriggered; }
	const CForagers&			foragers() const { return _Foragers; }

	/// Return the stat quality of the raw material [0..100]
	uint16						getStatQuality() const;

	// Accessors for testing
	float						getD() const { return _D; }
	float						getE() const { return _E; }
	//float						getC() const { return _C; }
	uint						getImpactScheme() const { return _IImpactMappingScheme; }
	void						resetSource( const CHarvestSource& ini ) { _NbExtractions = 0; initFrom( ini ); }

	void						setSafeSource(bool value) { _SafeSource = value; }
	
protected:

	/// Init dynamic props from template source (except pos, _N and _MaxQuality... see other methods below, and others such as _IImpactMappingScheme).
	void		initFrom( const CHarvestSource& ini )
				{ _LifeTime=ini._LifeTime; _ExtractionTime=ini._ExtractionTime; _ExtraExtractionTime=ini._ExtraExtractionTime; _S=ini._S; _A=ini._A; _Q=ini._Q; _D=ini._D; _E=ini._E; /*_C=ini._C;*/ /*_DistanceVisibility=ini._DistanceVisibility; _StealthVisibility=ini._StealthVisibility;*/ }

	/// Set the position
	void		setPos( const NLMISC::CVector2f& pos ) { _Pos = pos; }

	/// Set the link to the forage site
	void		setForageSite( CRecentForageSite *forageSite ) { _ForageSite = forageSite; }

	/// Set the raw material, the initial amount and the max quality, or return false if the current quantity in the deposit is 0.
	bool		setRawMaterial( const CStaticDepositRawMaterial *rmInfo, float quantityRatio );

	/// Update visual properties & related stuff
	void		updateVisuals();

	/// Send a message to all the players extracting on the source
	void		sendMessageToExtractors( const char *msg );

	/// Send a message with a parameter to all the players extracting on the source
	void		sendMessageToExtractors( const char *msg, sint32 param );

	/// When the threshold of E is reached
	void		makeDamagingEvent();

	/// A continuous damaging event 
	void		spawnToxicCloud();

	/// A one-time damaging event
	void		explode();

	/// Despawn the source in mirror, and exit from forage site
	void		despawn();

	/// Get the damage factor of a source [0..1] (for explosion or toxic cloud)
	float		getDamageFactor() const;

	/// Inc the number of event triggered
	void		setEventTriggered() { if ( _NbEventTriggered < 255 ) ++_NbEventTriggered; }

	/// Set a new mapping scheme of property impact
	void		setNewImpactScheme();

	/// Impact a realtime prop
	void		impactRTProp( TTargetRTProp iProp, float targetValue )
	{
		_TargetRTProps[iProp] = targetValue;
	}

private:

	/// Raw material
	NLMISC::CSheetId	_MaterialSheet;

	/// Entity representing the source, valid after spawn()
	TDataSetRow			_DataSetRow;

	/// Source position (coordinates in meters)
	NLMISC::CVector2f	_Pos;

	/// Link to forage site (they can't move in memory or be deleted while the source is present). Can be NULL (usually for tests)
	CRecentForageSite	*_ForageSite;

	/// Link to deposit used for kami anger level (they can't move or deleted after init)
	CDeposit			*_DepositForK;

	/// Link to deposit used for auto spawned sources? Should be NULL or same as _DepositForK. Yoyo: I use a CRefPtr for security...
	NLMISC::CRefPtr<CDeposit>			_DepositAutoSpawn;
	
	/// Source remaining Time To Live in ticks (decremented every update when !_IsExtractionInProgress)
	NLMISC::TGameCycle	_LifeTime;

	/// The total time for extraction (_T initial value before it decreases). Locked is forbidden.
	NLMISC::TGameCycle	_ExtractionTime;

	/// Extra time for extraction
	NLMISC::TGameCycle	_ExtraExtractionTime;

	/// Amount of extra time (included in _ExtraExtractionTime) that comes from the extractor's bonus passive bricks
	NLMISC::TGameCycle	_IncludedBonusExtractionTime;

	// Time remaining for extraction (starts to decrease when the 1st extraction begins). Locked until the source is fully spawned.
	NLMISC::TGameCycle	_T;

	/// Quantity of raw material in the source
	float				_N;

	/// Real-time properties
	float				_RTProps [NbRTProps];

	/// Target value for D, E
	float				_TargetRTProps [NbTargetRTProps];

	/// Max quality of the raw material
	uint16				_MaxQuality;

	/// Mapping of property impact
	uint16				_IImpactMappingScheme;

	/// Number of extractions done
	uint16				_NbExtractions;

	/// True if the impact scheme must be a friendly one
	bool				_IsInNewbieMode;

	/// Bonus for quantity rate (aperture)
	uint8				_BonusForAPct;

	/// Distance of visibility
	//sint32				_DistanceVisibility;

	/// Stealth mode (visibility priviledges)
	//VISIBILITY_RIGHTS::TVisGroup	_StealthVisibility;

	/// List of foragers
	CForagers			 _Foragers;

	/// List of 'reduce damage' events
	CRDEvents			_Events;

	/// True if one or more extractions are in progress
	bool				_IsExtractionInProgress;

	/// Trigger reset of explosion visual fx
	sint8				_ExplosionResetCounter;

	/// True if the source was auto-spawned
	bool				_IsAutoSpawned;

	/// True if the source is safe (cannot explode)
	bool				_SafeSource;

	/// Number of source risk events triggered
	uint8				_NbEventTriggered;
};

/**
 * Harvest source manager class
 */
class CHarvestSourceManager : public CSimpleEntityManager<CHarvestSource>
{
	NL_INSTANCE_COUNTER_DECL(CHarvestSourceManager);
public:

	/// Singleton access
	static CHarvestSourceManager *getInstance();

	/// Initialization
	void		init( TDataSetIndex baseRowIndex, TDataSetIndex size );

	/// Release
	static void	release();
};


#endif // NL_HARVEST_SOURCE_H

/* End of harvest_source.h */
