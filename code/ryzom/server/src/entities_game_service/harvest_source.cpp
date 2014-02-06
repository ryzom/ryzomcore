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
#include "harvest_source.h"
#include "deposit.h"
#include "egs_globals.h"
#include "phrase_manager/phrase_utilities_functions.h"
#include "entity_manager/entity_base.h"
#include "player_manager/player_manager.h"
#include "player_manager/player.h"
#include "player_manager/character.h"
#include "forage_progress.h"
#include "range_selector.h"
#include "egs_sheets/egs_sheets.h"
#include "nel/misc/variable.h"
#include "phrase_manager/s_effect.h"
#include "server_share/r2_vision.h"


using namespace NLMISC;
using namespace NLNET;
using namespace std;

NL_INSTANCE_COUNTER_IMPL(CHarvestSource);
NL_INSTANCE_COUNTER_IMPL(CHarvestSourceManager);

NL_ISO_TEMPLATE_SPEC CSimpleEntityManager<CHarvestSource>	*CSimpleEntityManager<CHarvestSource>::_Instance = NULL;

uint NbAutoSpawnedForageSources = 0;


const NLMISC::TGameCycle MaxT = 1200; // 2 min for prospected sources (TODO: external data)
const float CommonRiskThreshold = 127.0f;
const float ThresholdD1 = CommonRiskThreshold;
const float ThresholdD2 = CommonRiskThreshold * 1.20f;
const float ThresholdE = CommonRiskThreshold;
const float ThresholdC = CommonRiskThreshold;

const float DeltaMoveBarPerTick = (DeltaMoveBarPerSec / 10.0f);
const float DeltaResetBarPerTick = (DeltaResetBarPerSec / 10.0f);

sint32 ForageSourceDisplay = -1;

static const NLMISC::TGameCycle Locked = ~0;
static const NLMISC::TGameCycle IniTime = (~0)-1; // highest positive integer (suitable for unsigned)


// Minimum extraction session:
// 6 extractions = 24 seconds with 4 seconds per extraction
// Maximum extraction session:
// 80 extractions = 120 seconds with 1.5 second per extraction



// MaxA: 3.5
// MaxS: 1/15
// nlctassert(MaxAS>0);
const float MaxAS = (3.5f / 15.0f);

// MaxRequiredQ: 250; InitResultQ: 1 => MaxDeltaQ: 26
// nlctassert(MaxDeltaQ>0);
const float MaxQ = 250.0f;
const float MaxDeltaQ = 26.0f;

// { D, E, C } impacted by (0=Qtty, 1=Qlty, 2=Both)
uint ImpactSchemes [6][3] = { { 0, 1, 2 }, { 0, 2, 1 }, { 1, 0, 2 }, { 1, 2, 0 }, { 2, 0, 1 }, { 2, 1, 0 } };
// Observed impact on D (/10):     1            3            6           10            3            1
// Observed impact on E (/10):     6            3            1            1            3           10
// Note: if modifying this schemes, please change FORAGE_SOURCE_IMPACT_MODE in phrase_en.txt.
uint SpecialNewbieImpactSchemeD = 10;
uint16 LowDangerMappings [2] = { SpecialNewbieImpactSchemeD+1, SpecialNewbieImpactSchemeD+4 };

sint8 ExplosionResetPeriod = 50; // 5 s

CHarvestSource AutoSpawnSourceIniProperties;

/*
 * Access to singleton
 */
CHarvestSourceManager *CHarvestSourceManager::getInstance()
{
	return (CHarvestSourceManager*)_Instance;
}

/*
 * Initialization of source manager
 */
void CHarvestSourceManager::init( TDataSetIndex baseRowIndex, TDataSetIndex size )
{
	CSimpleEntityManager<CHarvestSource>::init( baseRowIndex, size );

	// Note: Now, most of these values are overridden by deposit settings (see CFgProspectionPhrase::autoSpawnSource())
	AutoSpawnSourceIniProperties.setLifetime( 6000 ); // 10 min
	AutoSpawnSourceIniProperties.setProspectionExtraExtractionTime( 0 ); // no extra time
	//AutoSpawnSourceIniProperties.setDistVis( 100 );
}

void CHarvestSourceManager::release()
{
	delete (CHarvestSourceManager*)_Instance;
}

/*
 * HarvestSource constructor
 */
CHarvestSource::CHarvestSource()
{
	_NbExtractions = 0;
	_ForageSite = NULL;
	_DepositForK = NULL;

	// Set default values (TODO: external data)
	_LifeTime = 200; // 20 s
	_ExtractionTime = 250; // 25 s for initial extraction time (now, recalculated using quality of material)
	_ExtraExtractionTime = 25; // 2.5 s
	_IncludedBonusExtractionTime = 0;
	_T = Locked; // until spawn is completed
	_S = 0.025f; // 4 s per delivery (initial value may be not used by the extraction action)
	_A = 0.1f;
	_Q = 1.0f;
	_D = 0;
	_E = 0;
	_TargetRTProps[TargetD] = 0;
	_TargetRTProps[TargetE] = 0;
	//_C = 0;
	_MaxQuality = ~0;
	_IImpactMappingScheme = 0;
	_N = 0;	
	_IsInNewbieMode = false;
	_BonusForAPct = 0;
	//_DistanceVisibility = 80;
	//_StealthVisibility = VISIBILITY_RIGHTS::All;
	_IsExtractionInProgress = false;
	_ExplosionResetCounter = -1;
	_IsAutoSpawned = false;
	_SafeSource = false;
	_NbEventTriggered = 0;
}


/*
 * HarvestSource destructor
 */
CHarvestSource::~CHarvestSource()
{
	// unregister the deposit auto spawn if any
	setDepositAutoSpawn(NULL);
}


/*
 * setDepositAutoSpawn
 */
void	CHarvestSource::setDepositAutoSpawn(CDeposit *deposit)
{
	// unregister the current deposit, if any
	if(_DepositAutoSpawn)
	{
		_DepositAutoSpawn->decreaseAutoSpawnedSources();
		_DepositAutoSpawn= NULL;
	}

	// register the new one if any
	if(deposit)
	{
		_DepositAutoSpawn= deposit;
		_DepositAutoSpawn->increaseAutoSpawnedSources();
	}
}


/*
 * Init the source. All pointers must be valid (but forageSite may be NULL).
 * Return false if the current quantity in the deposit is 0.
 */
bool CHarvestSource::init( const CHarvestSource& ini, const NLMISC::CVector2f& pos,
	CRecentForageSite *forageSite,
	CDeposit *depositForK,
	const CStaticDepositRawMaterial *rmInfo, float quantityRatio )
{
	initFrom( ini );
	setPos( pos );
	setForageSite( forageSite );
	bool isNonEmpty = setRawMaterial( rmInfo, quantityRatio );

	// Set link to deposit for kami anger level
	_DepositForK = depositForK;

	return isNonEmpty;
}


/*
 * Set the raw material, the initial amount and the max quality, or return false if the current quantity in the deposit is 0.
 */
bool CHarvestSource::setRawMaterial( const CStaticDepositRawMaterial *rmInfo, float quantityRatio )
{
	H_AUTO(CHarvestSource_setRawMaterial);
	
	_MaterialSheet = rmInfo->MaterialSheet;

	// Get corresponding initial quantity & max quality
	const CStaticItem *staticItem = CSheets::getForm( _MaterialSheet );
	if ( staticItem && staticItem->Mp )
	{
		/// The quantity is a fraction of the 'Stackable' property (but it is limited by the constraints of the deposit)
		_N = min( max( (((float)staticItem->Stackable) * quantityRatio), 1.0f ), _ForageSite->getQuantityInDeposit() );

		// Select either the MaxQuality in the deposit primitive or in the RM sheet (if -1 in the deposit, or no deposit)
		sint16 depositMaxQuality = _ForageSite ? _ForageSite->deposit()->maxQuality() : -1;
		if ( depositMaxQuality != -1 )
			_MaxQuality = (uint16)depositMaxQuality;
		else
			_MaxQuality = staticItem->Mp->MaxQuality;
		//nldebug( "Quality limited by source to %hu", _MaxQuality );

		if ( _N == 0 )
			return false;
	}
	else
	{
		nlwarning( "%s is not a valid raw material sheet", _MaterialSheet.toString().c_str() );
		return false;
	}
	return true;
}


/*
 * Recalculate the remaining extraction time depending on the requested quality.
 * Does nothing if the source is in "extra extraction time".
 * _ExtraExtractionTime is included in _ExtraExtractionTime.
 *
 * Precondition: _ExtractionTime != 0
 */
void CHarvestSource::recalcExtractionTime( float requestedQuality )
{
	if ( _T > _ExtraExtractionTime )
	{
		nlassert( _ExtractionTime > _ExtraExtractionTime );
		float timeRatio = ((float)_T) / ((float)_ExtractionTime);
		//float fExtractionTime = requestedQuality*(0.3092f*10.0f) + (22.0f*10.0f); // Q10 -> 25 s; Q250 -> 1'40 s
		float fExtractionTime = requestedQuality*ForageExtractionTimeSlopeGC.get() + ForageExtractionTimeMinGC.get() + (float)_ExtraExtractionTime;
		_ExtractionTime = (NLMISC::TGameCycle)fExtractionTime;
		_T = (NLMISC::TGameCycle)(fExtractionTime * timeRatio);
	}
}


/*
 * Prepare the source as an unpublished entity in mirror. Return false in case of failure. 
 * Must be called *after* init().
 * prospectorDataSetRow must be either null (isNull()) or an accessible row (isAccessible()).
 */
bool CHarvestSource::spawnBegin( uint8 knowledgePrecision, const TDataSetRow& prospectorDataSetRow, bool isAutoSpawned )
{
	H_AUTO(CHarvestSource_spawn);
	
	// Add into mirror (but unpublished)
	CEntityId entityId = CEntityId::getNewEntityId( RYZOMID::forageSource );
	if ( ! Mirror.createEntity( entityId ) )
		return false;
	_DataSetRow = TheDataset.getDataSetRow( entityId );
	_IsAutoSpawned = isAutoSpawned;

	// Set the sheet id (including knowledge information)
	uint sourceFXIndex = _ForageSite ? (uint)_ForageSite->deposit()->sourceFXIndex() : 0;
	CMirrorPropValue<TYPE_SHEET> sheet( TheDataset, _DataSetRow, DSPropertySHEET );
	sheet = CSheetId( toString( "%u_%u.forage_source", sourceFXIndex, knowledgePrecision ) ).asInt();

	// For knowledge 3, fit the sheet id of the RM sitem into NAME_STRING_ID (instead of a string id) !
	CMirrorPropValue<TYPE_NAME_STRING_ID> nameId( TheDataset, _DataSetRow, DSPropertyNAME_STRING_ID );
	nameId = (knowledgePrecision == 3) ? _MaterialSheet.asInt() : 0;
	if ( knowledgePrecision != 0 )
	{
		const CStaticItem *staticItem = CSheets::getForm( materialSheet() );
		if ( staticItem && staticItem->Mp )
		{
			// Set additional knowledge info into VISUAL_FX
			CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _DataSetRow, DSPropertyVISUAL_FX );
			switch ( knowledgePrecision )
			{
			case 1: visualFx = (TYPE_VISUAL_FX)staticItem->Mp->getGroup(); break; // initializes the prop, but only 10 bits allowed
			case 2: // family sent for 2 & 3 (for knowledge 3, used only as icon index)
			case 3: visualFx = (TYPE_VISUAL_FX)staticItem->Mp->Family; break; // initializes the prop, but only 10 bits allowed
			default:; // default value is 0
			}
			if ( (visualFx() & 0x400) != 0 )
			{
				nlwarning( "FG: Family or group index exceeding max!" ); // bit 10 is reserved for explosion FX
				visualFx = 0;
			}
		}
	}

	// Set the target (the prospector, or a nul datasetrow if auto-spawned)
	CMirrorPropValue<TYPE_TARGET_ID> targetRow( TheDataset, _DataSetRow, DSPropertyTARGET_ID );
	targetRow = prospectorDataSetRow;

	// Add to manager so that update() will be called at each game cycle
	CHarvestSourceManager::getInstance()->addEntity( this ); // we don't do it in spawnEnd(), because the source would remain unpublished forever if the AIS quit before replying to the transport class message

	return true;
}


/*
 * Complete the source spawn: publish the entity in mirror or delete it.
 * Caution: if authorized, the source object is deleted!
 */
void CHarvestSource::spawnEnd( bool authorized )
{
	if ( ! authorized )
	{
		despawn();
		CHarvestSourceManager::getInstance()->destroyEntity( _DataSetRow );
		return;
	}

	// Unlock update
	_ExtractionTime = IniTime; // will trigger calculation in recalcExtractionTime() because _T > _ExtraExtractionTime
	_T = _ExtractionTime;      // ratio 1.0 => beginning of extraction time

	if ( _IsAutoSpawned )
		++NbAutoSpawnedForageSources;

	// Set the initial position
	CMirrorPropValue<TYPE_POSX> posX( TheDataset, _DataSetRow, DSPropertyPOSX );
	CMirrorPropValue<TYPE_POSY> posY( TheDataset, _DataSetRow, DSPropertyPOSY );
	posX = (TYPE_POSX)(_Pos.x * 1000.0f);
	posY = (TYPE_POSY)(_Pos.y * 1000.0f);

	// Set the WhoSeesMe bitfield (distance of visibility by players (0-31) and creatures (32-63)) (now constant)
	uint32 nbBitsOn = /*_DistanceVisibility*/80 * 32 / 250; // (250 m is the max, corresponding to 32 bits on
	const uint64 distanceBitfield = IsRingShard? R2_VISION::buildWhoSeesMe(R2_VISION::WHOSEESME_VISIBLE_MOB,false): ((uint64)1 << (uint64)nbBitsOn) - 1; // (hide to creatures)
	CMirrorPropValue<TYPE_WHO_SEES_ME> whoSeesMe(TheDataset, _DataSetRow, DSPropertyWHO_SEES_ME );
	whoSeesMe = distanceBitfield;

	// Set selectable property to true
	CMirrorPropValue<TYPE_CONTEXTUAL> contextualProperties(TheDataset, _DataSetRow, DSPropertyCONTEXTUAL );
	CProperties prop(0);
	prop.selectable(true);
	contextualProperties = prop;

	// Update bars and other variable properties
	updateVisuals();

	// Publish in mirror
	TheDataset.declareEntity( _DataSetRow );
}


/*
 * Return the prospector or a null datasetrow if there was no prospection (auto-spawn)
 * The accessibility of this datasetrow must be checked before use.
 */
const TDataSetRow& CHarvestSource::getProspectorDataSetRow() const
{
	CMirrorPropValueRO<TYPE_TARGET_ID> targetRow( TheDataset, _DataSetRow, DSPropertyTARGET_ID );
	return targetRow();
}


/*
 * Despawn the source in mirror, and exit from forage site
 */
void CHarvestSource::despawn()
{
	H_AUTO(CHarvestSource_despawn);
	
	// Remove from mirror
	CEntityId entityId = TheDataset.getEntityId( _DataSetRow );
	Mirror.removeEntity( entityId );

	//nldebug( "--- %p from %p", this, _ForageSite );

	// Exit from forage site (the entering is done outside class, because a failure may prevent to create the source)
	if ( _ForageSite )
		_ForageSite->removeActiveSource();

	if ( _IsAutoSpawned )
		--NbAutoSpawnedForageSources;

	// End forage sessions and calculate XP and give result
	if ( ! _Foragers.empty() )
	{
		// The first forager is the only one who can extract. Give result & dispatch XP
		CForagers::const_iterator it = _Foragers.begin();
		CCharacter *player = PlayerManager.getChar( *it );
		if ( player && player->forageProgress() )
		{
			if ( player->forageProgress()->sourceRowId() == _DataSetRow ) // check if he has not changed his target
			{
				player->giveForageSessionResult( this );
			}
		}

		// End sessions of care takers (all excluding the first element)
		for ( ++it; it!=_Foragers.end(); ++it )
		{
			player = PlayerManager.getChar( *it );
			if ( player && player->forageProgress() )
			{
				if ( player->forageProgress()->sourceRowId() == _DataSetRow ) // check if he has not changed his target
				{
					player->endForageSession();
				}
			}
		}
	}
}


/*
 * Helper for updateVisiblePostPos()
 */
inline void updateBarValueAtTick( float& destValue, float& currentValue )
{
	float diff = destValue - currentValue;
	if ( diff > 0 )
	{
		currentValue = std::min( destValue, currentValue + DeltaMoveBarPerTick );
	}
	else if ( diff < 0 )
	{
		float delta = (currentValue == 0) ? DeltaResetBarPerTick : DeltaMoveBarPerTick;
		currentValue = std::max( destValue, currentValue - delta );
	}
}


/*
 * Update
 */
bool CHarvestSource::update()
{
	H_AUTO(CHarvestSource_update);
	
	if ( _IsExtractionInProgress )
	{
		// Make the bar transitions smooth (needs to be done on server to match timing on client)
		updateBarValueAtTick( _TargetRTProps[TargetD], _D );
		updateBarValueAtTick( _TargetRTProps[TargetE], _E );

		// Test damaging event risk
		if ( _E > ThresholdE )
		{
			makeDamagingEvent();
			setEventTriggered();
			impactRTProp( TargetE, 0 );
		}
		// Test spawn risk
		/*if ( _C > ThresholdC )
		{
			//sendMessageToExtractors( "CREATURE_SPAWN" );
			// TODO
			//setEventTriggered();
			_C = 0;
		}*/

		// Test depletion risk (if the bar transition is still in progress, wait)
		if ( (_D > ThresholdD1) )
		{
			// if high risk value, deplete all the forage site (if exist and allow depletion)
			if ( _ForageSite && _ForageSite->allowDepletionRisk() && _TargetRTProps[TargetD] > ThresholdD2 )
			{
				sendMessageToExtractors( "FORAGE_SOURCE_SITE_DEPLETED" );
				_ForageSite->depleteAll();
			}
			// else will just kill the current source (send appropriate message)
			else
				sendMessageToExtractors( "FORAGE_SOURCE_DEPLETED" );
			// in all case kill the current source, and add bad event
			setEventTriggered();
			despawn();
			return false;
		}
		// Test remaining time
		else if ( _T == 0 )
		{
			despawn();
			return false;
		}
		else
		{
			--_T;
			float naturalMoveThreshold = CommonRiskThreshold - 2.0f;
			if ( _TargetRTProps[TargetD]+ForageExtractionNaturalDDeltaPerTick.get() < naturalMoveThreshold ) // don't auto-move if it makes it trigger the event
				impactRTProp( TargetD, _TargetRTProps[TargetD] + ForageExtractionNaturalDDeltaPerTick.get() );
			if (!_SafeSource)
			{
				if ( _TargetRTProps[TargetE]+ForageExtractionNaturalEDeltaPerTick.get() < naturalMoveThreshold )
					impactRTProp( TargetE, _TargetRTProps[TargetE] + ForageExtractionNaturalEDeltaPerTick.get() );
			}
			if ( _DataSetRow.getIndex() == (TDataSetIndex)ForageSourceDisplay )
				nldebug( "T: %u", _T );
			updateVisuals();
			return true;
		}
	}
	else
	{
		// Remain locked if the source is not fully spanwed yet
		if ( _T == Locked )
			return true;

		// Test end of lifetime
		if ( _LifeTime == 0 )
		{
			despawn();
			return false;
		}
		else
		{
			--_LifeTime;
			if ( _DataSetRow.getIndex() == (TDataSetIndex)ForageSourceDisplay )
				nldebug( "_LifeTime: %u", _LifeTime );
			return true;
		}
	}
}


/*
 * Update visual properties & related stuff
 */
void CHarvestSource::updateVisuals()
{
	H_AUTO(CHarvestSource_updateVisuals);
	
	// Set the bars on the entity
	TYPE_BARS statusBar;
	float d = (_TargetRTProps[TargetD] > ForageCareBeginZone.get()) ? _TargetRTProps[TargetD] : 0;
	if ( d > CommonRiskThreshold )
		d = CommonRiskThreshold;
	float e = (_TargetRTProps[TargetE] > ForageCareBeginZone.get()) ? _TargetRTProps[TargetE] : 0;
	if ( e > CommonRiskThreshold )
		e = CommonRiskThreshold;
	statusBar = (_T==IniTime) ? 127 : uint32((_T*127/_ExtractionTime/2)*2); // time progression (round off to 2 to reduce flooding by 30% (taking D & E into account))
	if ( _N != 0 )
		statusBar = statusBar | ((uint32(_N)+1) << 7); // round off to next integer (TODO: ratio qtty/initial_qtty)
	statusBar = statusBar | (uint32((127.0f-d)/**127.0f*/) << 14);
	if (!_SafeSource) // If source is safe let 0 (client will display it full and with a special color)
		statusBar = statusBar | (uint32((127.0f-e)/**127.0f*/) << 21);
	statusBar = statusBar | (((uint32)(_IsExtractionInProgress)) << 28);
	//statusBar = statusBar | (uint32(_C/**127.0f*/) << 28);
	CMirrorPropValue<TYPE_BARS> statusBarProp( TheDataset, _DataSetRow, DSPropertyBARS );
	statusBarProp = statusBar;

	/*if ( (_N < 0) || (_N > 127) )
		nlwarning( "FG: N = %g", _N );
	if ( (_D < 0) || (_D > 127) )
		nlwarning( "FG: N = %g", _D );
	if ( (_E < 0) || (_E > 127) )
		nlwarning( "FG: N = %g", _E );*/

	// Set kami angry level information & extra time state (store in orientation!)
	float angryLevel7 = 127.0f - (127.0f * _DepositForK->kamiAnger() / ForageKamiAngerThreshold2.get());
	uint extraTime7 = _ExtraExtractionTime * 127 / _ExtractionTime;
	uint inclBonusExtraTime7 = _IncludedBonusExtractionTime * 127 / _ExtractionTime;

	CMirrorPropValue<TYPE_ORIENTATION> angryLevelExtraTimeProp( TheDataset, _DataSetRow, DSPropertyORIENTATION );
	TYPE_ORIENTATION angryLevelExtraTimePropV = angryLevel7;
	if ( _IsExtractionInProgress )
		angryLevelExtraTimePropV += (( ((float)inclBonusExtraTime7) * 128.0f + (float)extraTime7) * 128.0f);
	angryLevelExtraTimeProp = angryLevelExtraTimePropV; // stored as float, should be converted to 21b uint by FS

	// Reset explosion visual fx if needed
	if ( _ExplosionResetCounter == 0 )
	{
		CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _DataSetRow, DSPropertyVISUAL_FX );
		visualFx = (visualFx() & 0x3FF); // unset bit 10
		_ExplosionResetCounter = -1;
	}
	else if ( _ExplosionResetCounter > 0 )
	{
		--_ExplosionResetCounter;
	}
}


/*
 *
 */
void CHarvestSource::sendMessageToExtractors( const char *msg )
{
	H_AUTO(CHarvestSource_sendMessageToExtractors);
	
	CForagers::const_iterator it;
	for ( it=_Foragers.begin(); it!=_Foragers.end(); ++it )
	{
		const TDataSetRow& extRowId = (*it);
		PHRASE_UTILITIES::sendDynamicSystemMessage( extRowId, msg );
	}
}


/*
 *
 */
void CHarvestSource::sendMessageToExtractors( const char *msg, sint32 param )
{
	H_AUTO(CHarvestSource_sendMessageToExtractors);
	
	CForagers::const_iterator it;
	for ( it=_Foragers.begin(); it!=_Foragers.end(); ++it )
	{
		const TDataSetRow& extRowId = (*it);
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = param;
		PHRASE_UTILITIES::sendDynamicSystemMessage( extRowId, msg, params );
	}
}


CVariable<sint32> ForageForceImpactScheme( "egs", "ForageForceImpactScheme", "", -1 );

/*
 * Begin an extraction action. Once the extraction process is started, the source remains in
 * extraction mode until the extraction time is elapsed (even if players stop/restart
 * extracting).
 * Return true if the forager is the extractor (the first one on the source)
 */
bool CHarvestSource::beginExtraction( const TDataSetRow& forager, bool isNewbie )
{
	H_AUTO(CHarvestSource_beginExtraction);
	
	bool foragerIsFirstExtractor = false;
	if ( ! _IsExtractionInProgress )
	{
		_IsExtractionInProgress = true;
		foragerIsFirstExtractor = true;
		_IsInNewbieMode = isNewbie; // only the first one sets the mode

		// Tell him the max quality
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = (sint32)_MaxQuality;
		PHRASE_UTILITIES::sendDynamicSystemMessage( forager, "FORAGE_SOURCE_MAXLEVEL", params );

		// Set the impact scheme
		setNewImpactScheme(); // at this time, _Foragers is empty so nobody is told yet
	}

	// Add player to extractor to list (if new)
	CForagers::iterator it = find( _Foragers.begin(), _Foragers.end(), forager );
	if ( it == _Foragers.end() )
	{
		_Foragers.push_back( forager );

		// Tell him the impact mode
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = (sint32)_IImpactMappingScheme;
		PHRASE_UTILITIES::sendDynamicSystemMessage( forager, "FORAGE_SOURCE_IMPACT_MODE", params );
	}
	else if ( it == _Foragers.begin() )
	{
		foragerIsFirstExtractor = true;
	}

	return foragerIsFirstExtractor;
}


/*
 * Set a new mapping scheme of property impact
 */
void CHarvestSource::setNewImpactScheme()
{
	H_AUTO(CHarvestSource_setNewImpactScheme);
	
	// Set mapping scheme of property impact
	if ( _IsInNewbieMode )
	{
		// Force "low dangers" for 1 newbie extractor
		_IImpactMappingScheme = (uint16)LowDangerMappings[RandomGenerator.rand( 1 )];
	}
	else
	{
		// Normal dangers
		if ( ForageForceImpactScheme.get() == -1 )
			_IImpactMappingScheme = (uint16)RandomGenerator.rand( 5 );
		else
			_IImpactMappingScheme = (uint16)ForageForceImpactScheme.get();
	}

	sendMessageToExtractors( "FORAGE_SOURCE_IMPACT_MODE", (sint32)_IImpactMappingScheme );

#ifdef NL_DEBUG
	nldebug( "FG: map scheme: %u", _IImpactMappingScheme );
#endif
}


bool ForceDropProp = false;
NLMISC_VARIABLE( bool, ForceDropProp, "" );

CVariable<float> ForceAbsorption( "egs", "ForceAbsorption", "", 0 );


/*
 * Update the source state with an extraction (see doc in .h).
 */
void CHarvestSource::extractMaterial( float *reqPosProps, float *absPosProps, float qualityCeilingFactor, float qualitySlowFactor, float *results, float successFactor, uint8 lifeAbsorberRatio, const TDataSetRow& extractingEntityRow, CHarvestSource::TRealTimeProp& propDrop )
{
	H_AUTO(CHarvestSource_extractMaterial);

	CCharacter* player = PlayerManager.getChar( extractingEntityRow );
	
	++_NbExtractions; // it's 1 at the first call, so that the initial value is used
	float nbe = (float)_NbExtractions;

	if ( (successFactor < 0.1f) || ForceDropProp )
	{
		if ( _NbExtractions < 6 )
			propDrop = Q; // don't drop A at the beginning (wait to reach 1 (cumulated) would be better)
		else
			propDrop = (TRealTimeProp)(RandomGenerator.rand(1)+1);
		nldebug( "Prop drop %u", propDrop );
	}

	// Aperture: converges towards the requested value, except when a drop occurs (0 at this step)
//	if ( reqPosProps[A] > 0 )
	{
		float obtainedQuantity = (propDrop == A) ? 0.0f : (results[A]*ForageQuantitySlowFactor.get() + reqPosProps[A]) / (ForageQuantitySlowFactor.get()+1);

		// Apply possible aperture bonus
		if ( _BonusForAPct != 0 )
		{
			obtainedQuantity += obtainedQuantity * (float)((uint)_BonusForAPct) * 0.01f;
		}
		
		// Extract material
		if ( _ForageSite )
			obtainedQuantity = _ForageSite->consume( obtainedQuantity ); // consume (and limit) in forage site & deposit
		if ( obtainedQuantity > _N ) // should not occur because _N uses the deposit quantity
			obtainedQuantity = _N;
		_N -= obtainedQuantity;
		_A = (_A*nbe + obtainedQuantity) / (nbe + 1.0f); // average per source
		_DepositForK->incKamiAnger( obtainedQuantity, _Foragers );
		if ( (obtainedQuantity == 0) && (results[A] != 0) && (! (propDrop == A)) )
			PHRASE_UTILITIES::sendDynamicSystemMessage( _Foragers[0], "FORAGE_DEPOSIT_IS_EMPTY" );
		results[A] = obtainedQuantity;
		
		// add spire effect ( quantity )
		if ( player )
		{
			const CSEffect* pEffect = player->lookForActiveEffect( EFFECT_FAMILIES::TotemHarvestQty );
			if ( pEffect != NULL )
			{
				results[A] *= ( 1.0f + pEffect->getParamValue() / 100.0f );
			}
		}
		// add item special effect
		if ( player )
		{
			std::vector<SItemSpecialEffect> effects = player->lookForSpecialItemEffects(ITEM_SPECIAL_EFFECT::ISE_FORAGE_ADD_RM);
			std::vector<SItemSpecialEffect>::const_iterator it, itEnd;
			double addedQty = 0.;
			for (it=effects.begin(), itEnd=effects.end(); it!=itEnd; ++it)
			{
				float rnd = RandomGenerator.frand();
				if (rnd<it->EffectArgFloat[0])
				{
					addedQty += it->EffectArgFloat[1];
					PHRASE_UTILITIES::sendItemSpecialEffectProcMessage(ITEM_SPECIAL_EFFECT::ISE_FORAGE_ADD_RM, player, NULL, (sint32)(it->EffectArgFloat[1]*100.));
				}
			}
			results[A] *= 1.0f + (float)addedQty;
		}
	}
//	else
//	{
//		results[A] = 0;
//	}

	// Speed: always the requested speed (otherwise, looks like a bug for the player when it's the speed of the action)
	results[S] = reqPosProps[S];
	_S = (_S*nbe + results[S]) / (nbe + 1.0f); // average per source

	// Quality: converges towards the requested value, except when a drop occurs (0 at this step)
	float usedReqQ = (propDrop == Q) ? 0.0f : reqPosProps[Q] * qualityCeilingFactor;
	float resQ = (results[Q]*qualitySlowFactor + usedReqQ) / (qualitySlowFactor+1);
	float maxQOfSource = (float)_MaxQuality;
	if ( resQ > maxQOfSource )
	{
		resQ = maxQOfSource;
		//if ( results[Q] < (float)_MaxQuality )
		//	nldebug( "Quality limited by source to %hu", _MaxQuality ); // TODO: tell the player(s)
	}
	if ( (resQ < _Q) || (resQ < reqPosProps[Q]) || (! ForageQualityCeilingClamp.get()) )
	{
		// Set Q only if not increasing and exceeding requested quality
		results[Q] = resQ;
	}
	else
	{
		// Clamp Q to the max required by the player
		results[Q] = reqPosProps[Q];
	}
	float prevQ = _Q;
	_Q = results[Q]; // now there is only one extractor => the Q of the source is the resulting Q
	if ( ((prevQ < reqPosProps[Q]) && (_Q == reqPosProps[Q])) || ((prevQ < maxQOfSource) && (_Q == maxQOfSource)) )
	{
		setNewImpactScheme(); // we just reached the max quality
	}

	// Calc impact of the new average values

	// Previously, the impact depended on the level of the extraction:
	// float quantityBaseImpact = _A * _S * ForageQuantityImpactFactor.get();
	// float qualityBaseImpact = (_Q - oldQ) * ForageQualityImpactFactor.get();
	//
	// Now it's constant (=max), but the amount of damage depends on the level of the extraction:
	float quantityBaseImpact = MaxAS * ForageQuantityImpactFactor.get();
	float qualityBaseImpact = MaxDeltaQ * ForageQualityImpactFactor.get();
	uint impactScheme = _IImpactMappingScheme;
	if ( impactScheme >= SpecialNewbieImpactSchemeD)
	{
		// Lower impacts for newbies
		impactScheme -= SpecialNewbieImpactSchemeD;
		quantityBaseImpact *= 0.5f;
		qualityBaseImpact *= 0.5f;
	}
	if ( ForceAbsorption.get() != 0 )
	{
		absPosProps[A] = ForceAbsorption.get();
		absPosProps[Q] = ForceAbsorption.get();
		absPosProps[S] = ForceAbsorption.get();
	}
	for ( uint i=D; i!=NbRTProps; ++i )
	{
		if (i==E && _SafeSource)
			break;
		uint impactType = ImpactSchemes[impactScheme][i-NbPosRTProps];
		float impact;
		switch ( impactType )
		{
		case 0 : impact = quantityBaseImpact * (1.0f - absPosProps[A]); break;
		case 1 : impact = qualityBaseImpact * (1.0f - absPosProps[Q]); break;
		default: impact = (quantityBaseImpact + qualityBaseImpact) / 2.0f * (1.0f - absPosProps[S]); break; // bound on the average of both, absorption of S
		}

		impact += RandomGenerator.frandPlusMinus( impact ); // result impact from 0 to impact*2
		
		// add spire effect ( aggressivity )
		if ( player )
		{
			const CSEffect* pEffect = player->lookForActiveEffect( EFFECT_FAMILIES::TotemHarvestAgg );
			if ( pEffect != NULL )
			{
				impact *= ( 1.0f - pEffect->getParamValue() / 100.0f );
			}
		}

		if ( impact < 0 ) impact = 0; // impact can't be negative
		if ( (i==D) && (lifeAbsorberRatio != 0) )
		{
			// Damage the life absorber, instead of impacting D
			CEntityBase *entity = CEntityBaseManager::getEntityBasePtr( extractingEntityRow ); // getEntityBasePtr() tests TheDataset.isAccessible( extractingEntity )
			if ( entity )
			{
				float impactOnHP = ((float)lifeAbsorberRatio) * impact * 0.01f;
				impact -= impactOnHP;
				float dmgRatio = impactOnHP * ForageHPRatioPerSourceLifeImpact.get();
				sint32 dmg = (sint32)((float)entity->maxHp() * dmgRatio);
				if ( dmg != 0 )
					CHarvestSource::hitEntity( RYZOMID::forageSource, entity, dmg, dmg, true );
			}
		}
		if ( (_TargetRTProps[i-D] < CommonRiskThreshold*0.90f) && (_TargetRTProps[i-D] + impact > CommonRiskThreshold) )
		{
			// Avoid a brutal unnatural end, make a step just before reaching threshold
			impactRTProp( (TTargetRTProp)(i-D), CommonRiskThreshold - 2.0f );
		}
		else
		{
			// Normal impact
			impactRTProp( (TTargetRTProp)(i-D), _TargetRTProps[i-D] + impact );
		}
	}
}


/*
 * Update the source state with a care (see doc in .h)
 */
void CHarvestSource::takeCare( float *deltas, bool *isUseful )
{
	H_AUTO(CHarvestSource_takeCare);
	
	// Do not count care if from the "begin zone"
	if ( _TargetRTProps[TargetD] > ForageCareBeginZone.get() )
		*isUseful = true;
	if ( _TargetRTProps[TargetE] > ForageCareBeginZone.get() )
		*isUseful = true;
	//if ( deltas[DeltaD] > ForageCareBeginZone.get() )
	//	*isUseful = true;

	// Calc actual deltas
	deltas[DeltaD] += RandomGenerator.frandPlusMinus( deltas[DeltaD]*0.05f );
	deltas[DeltaE] += RandomGenerator.frandPlusMinus( deltas[DeltaE]*0.05f );
	//deltas[DeltaC] += RandomGenerator.frandPlusMinus( deltas[DeltaC]*0.05f );

	// TODO: impact on S,A,Q

	// Apply deltas
	float targetValue = _TargetRTProps[TargetD] - (deltas[DeltaD] * ForageCareFactor.get());
	if ( targetValue < 0 )
		targetValue = 0;
	impactRTProp( TargetD, targetValue );
	if (!_SafeSource)
	{
		targetValue = _TargetRTProps[TargetE] - (deltas[DeltaE] * ForageCareFactor.get());
		if ( targetValue < 0 )
			targetValue = 0;
		impactRTProp( TargetE, targetValue );
	}
	//_C -= (deltas[DeltaC] * ForageCareFactor.get());
	//if ( _C < 0 ) _C = 0;
}


/*
 * When the threshold of E is reached.
 */
void CHarvestSource::makeDamagingEvent()
{
	H_AUTO(CHarvestSource_makeDamagingEvent);
	
	sint32 r = RandomGenerator.rand( 1 );
	if ( r == 0 )
	{
		spawnToxicCloud();
	}
	else
	{
		explode();
	}

	_Events.clear();
	setNewImpactScheme();
}


/*
 *
 */
class CForageDamagingEventRangeSelector : public CRangeSelector
{
public:
	void buildTargetList( sint32 x, sint32 y, float radius /*, float minFactor*/ )
	{
		H_AUTO(CForageDamagingEventRangeSelector_buildTargetList);
		
		buildDisc( 0, x, y, radius, EntityMatrix, true );
	}

	float getFactor( uint entityIdx )
	{
		return 1.0f; // every entity in the radius gets the same damage, not depending of his location
	}				 // improvement note: for explosion, could be decreasing with the distance
};


/*
 * A continuous damaging event 
 */
void CHarvestSource::spawnToxicCloud()
{
	H_AUTO(CHarvestSource_spawnToxicCloud);
	
	sendMessageToExtractors( "SOURCE_TOXIC_CLOUD" );

	// Get random cloud params near the source (TODO: Z)
	float dmgFactor = getDamageFactor();
	sint32 iRadius = min( (sint32)2, (sint32)(dmgFactor * 3.0f) ); // => mapping [0..1] to {0, 1, 2}
	float Radiuses [3] = { 1.5f, 3.0f, 5.0f }; // corresponding to the 3 sheets
	float radius = Radiuses[iRadius];
	CVector cloudPos( _Pos.x, _Pos.y, 0.0f );
	if ( iRadius != 0 )
	{
		// For a big toxic cloud, shift the centre in a axis-aligned square of 4 m width (max dist = 2.8 m)
		const float MaxAxisDistFromSource = 2.0f;
		float dX = RandomGenerator.frand( MaxAxisDistFromSource*2.0f );
		float dY = RandomGenerator.frand( MaxAxisDistFromSource*2.0f );
		cloudPos.x += dX - MaxAxisDistFromSource;
		cloudPos.y += dY - MaxAxisDistFromSource;
	}

	// Spawn the toxic cloud
	CToxicCloud *tc = new CToxicCloud();
	tc->init( cloudPos, radius, (sint32)(dmgFactor * ToxicCloudDamage.get()), ToxicCloudUpdateFrequency );

	CSheetId sheet( toString( "toxic_cloud_%d.fx", iRadius ));
	if ( tc->spawn( sheet ) )
	{
		CEnvironmentalEffectManager::getInstance()->addEntity( tc );
#ifdef NL_DEBUG
		nldebug( "FG: Toxic cloud spawned (radius %g)", radius );
#endif
	}
	else
	{
		nlwarning( "FG: Unable to spawn toxic cloud (mirror range full?)" );
		delete tc;
	}
}


/*
 * Test which entities to hit (all entities except NPC and non-pets creatures)
 */
inline bool isAffectedByForageDamage( CEntityBase *entity )
{
	uint8 entityType = entity->getId().getType();
	return ! ((entityType == RYZOMID::npc) ||
			  ((entityType == RYZOMID::creature) &&
			   (entity->getRace() != EGSPD::CPeople::MektoubMount) &&
			   (entity->getRace() != EGSPD::CPeople::MektoubPacker)));
}


/*
 * A one-time damaging event
 */
void CHarvestSource::explode()
{
	H_AUTO(CHarvestSource_explode);

	sendMessageToExtractors( "SOURCE_EXPLOSION" );

	// Set the FX
	CMirrorPropValue<TYPE_VISUAL_FX> visualFx( TheDataset, _DataSetRow, DSPropertyVISUAL_FX );
	visualFx = (visualFx() | 0x400); // set bit 10
	_ExplosionResetCounter = ExplosionResetPeriod;

	// Get entities around the source, and hit them
	if ( HarvestAreaEffectOn )
	{
		// Calculate damage
		float fDmg = getDamageFactor() * ForageExplosionDamage.get();
		float dmgAvoided = fDmg;
		for ( CRDEvents::const_iterator iev=_Events.begin(); iev!=_Events.end(); ++iev )
		{
			const CReduceDamageEvent& event = (*iev);
			if ( CTickEventHandler::getGameCycle() - event.Time < ForageReduceDamageTimeWindow.get() )
			{
				//nldebug( "Damage %.1f x %.1f", fDmg, event.Ratio );
				fDmg *= event.Ratio; // multiple events are multiplied (e.g. 50% and 50% again do 25%)
			}
		}
		dmgAvoided = dmgAvoided - fDmg;
		bool wereAllEventsMissed = (! _Events.empty()) && (dmgAvoided == 0);

		// Make area of effect
		const float explosionRadius = 4.0f;
		CForageDamagingEventRangeSelector targetSelector;
		targetSelector.buildTargetList( (sint32)(_Pos.x * 1000.0f), (sint32)(_Pos.y * 1000.0f), explosionRadius );
		const vector<CEntityBase*>& targets = targetSelector.getEntities();
		for ( vector<CEntityBase*>::const_iterator it=targets.begin(); it!=targets.end(); ++it )
		{
			CEntityBase *entity = (*it);
			if ( entity && isAffectedByForageDamage( entity ) )
			{
				sint32 dmg = (sint32)(entity->getActualDamageFromExplosionWithArmor( fDmg ));
				CHarvestSource::hitEntity( RYZOMID::forageSource, entity, dmg, (sint32)(fDmg+dmgAvoided), false, (sint32)dmgAvoided );  // is not blocked by any armor

				if ( wereAllEventsMissed && (entity->getId().getType() == RYZOMID::player) )
				{
					PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), "SOURCE_DMG_REDUX_MISSED" );
				}
			}
		}
	}
}


/*
 * Reduce the damage of the next blowing up (if it is in near time delta)
 */
void	CHarvestSource::reduceBlowingUpDmg( float ratio )
{
	H_AUTO(CHarvestSource_reduceBlowingUpDmg);
	
	CReduceDamageEvent event;
	event.Time = CTickEventHandler::getGameCycle();
	event.Ratio = ratio;
	_Events.push_back( event );
}


/*
 * Get the damage factor of a source (for explosion or toxic cloud)
 */
float	CHarvestSource::getDamageFactor() const
{
	H_AUTO(CHarvestSource_getDamageFactor);

	// Map linearly using 5 -> 0.5, 80 -> 1.0 (to match previous algorithm)
	float statQualityFactor = ((((float)getStatQuality())+70.0f) / 150.0f);
	switch ( ImpactSchemes[_IImpactMappingScheme][E] )
	{
	case 0 : return _A * _S * statQualityFactor / MaxAS; break;
	case 1 : return _Q * statQualityFactor / MaxQ; break; // not using DeltaQ but Q
	default: return (_A*_S/MaxAS + _Q/MaxQ) * statQualityFactor / 2; break;
	}
}


/*
 * Return the stat quality of the raw material
 */
/*CHarvestSource::TStatClassForage	CHarvestSource::getStatQuality() const
{
	H_AUTO(CHarvestSource_getStatQuality);
	
	const CStaticItem *itemInfo = CSheets::getForm( _MaterialSheet );
	if ( itemInfo && itemInfo->Mp )
	{
		// (0..20 - 1) / 20 = 0
		// (21..40 - 1) / 20 = 1
		// (41..51 - 1) / 20 = 2;
		// (52..59) / 20 = 2
		// (61..79) / 20 = 3
		// (80..99) / 20 = 4
		// (100...) -> 4
		sint16 statEnergy = (sint16)(itemInfo->Mp->StatEnergy);
		TStatClassForage sq =
			(statEnergy < 51) ? (statEnergy - 1) / 20 :
			((statEnergy < 100) ? statEnergy / 20 : SupremeMagnificient);
		return sq;
	}
	nlwarning( "Invalid raw material %s", _MaterialSheet.toString().c_str() );
	return BasicPlainAverage;
}*/


/*
 * Return the stat quality of the raw material [0..100]
 * (Frequent values: 20 35 50 65 80)
 */
uint16	CHarvestSource::getStatQuality() const
{
	H_AUTO(CHarvestSource_getStatQuality);
	
	const CStaticItem *itemInfo = CSheets::getForm( _MaterialSheet );
	if ( itemInfo && itemInfo->Mp )
	{
		return itemInfo->Mp->StatEnergy;
	}
	nlwarning( "Invalid raw material %s", _MaterialSheet.toString().c_str() );
	return 0;
}


/*
 * Damage an entity
 */
void	CHarvestSource::hitEntity( RYZOMID::TTypeId aggressorType, CEntityBase *entity, sint32 hpDamageAmount, sint32 hpDamageAmountWithoutArmour, bool isIntentional, sint32 hpAvoided )
{
	H_AUTO(CHarvestSource_hitEntity);

	if ( entity->isDead())
		return;

	bool killed = entity->changeCurrentHp( -hpDamageAmount );
	if ( isIntentional )
	{
		SM_STATIC_PARAMS_1(params, STRING_MANAGER::integer);
		params[0].Int = hpDamageAmount;
		PHRASE_UTILITIES::sendDynamicSystemMessage( entity->getEntityRowId(), "FORAGE_ABSORB_DMG", params );
	}
	else
		PHRASE_UTILITIES::sendNaturalEventHitMessages( aggressorType, entity->getEntityRowId(), hpDamageAmount, hpDamageAmountWithoutArmour, hpAvoided );
	if ( killed )
		PHRASE_UTILITIES::sendDeathMessages( TDataSetRow(), entity->getEntityRowId() );
}


NLMISC_VARIABLE( sint32, ForageSourceDisplay, "Row index of source to verbose" );


/*
 * Testing
 */

TDataSetRow TestSourceRow;

void forageTestDoBegin()
{
	CHarvestSource templateSource, *testSource;
	templateSource.setLifetime( 1140 );
	templateSource.setProspectionExtraExtractionTime( 1140 );
	CStaticDepositRawMaterial rm;

	testSource = new CHarvestSource;
	CDeposit deposit;
	testSource->init( templateSource, CVector2f(1000.0f,1000.0f), NULL, &deposit, &rm, 1.0f );
	TDataSetRow dsr;
	if ( testSource->spawnBegin( 0, dsr, false ) )
	{
		TestSourceRow = testSource->rowId();
	}
	else
	{
		delete testSource;
	}
}

bool forageTestDoExtract(
	NLMISC::CLog& log,
	uint nbIterations,
	float reqPeriod,
	float reqA,
	float reqQ,
	float absorption,
	float successFactor )
{
	CHarvestSource *testSource = CHarvestSourceManager::getInstance()->getEntity( TestSourceRow );
	if ( ! testSource )
	{
		log.displayNL( "Call forageTestBegin first" );
		return true;
	}

	// Request and output results
	FILE *f = fopen( std::string(getLogDirectory() + "forage_test.csv").c_str(), "at" );
	FILE *f2 = fopen( std::string(getLogDirectory() + "forage_test.log").c_str(), "at" );
	float reqS = 1.0f / (reqPeriod * 10.0f);
	float req [CHarvestSource::NbPosRTProps];
	float abs [CHarvestSource::NbPosRTProps];
	float res [CHarvestSource::NbPosRTProps];
	static bool FirstTime = true;
	req[CHarvestSource::S] = reqS;
	req[CHarvestSource::A] = reqA;
	req[CHarvestSource::Q] = reqQ;
	abs[CHarvestSource::S] = absorption;
	abs[CHarvestSource::A] = absorption;
	abs[CHarvestSource::Q] = absorption;
	res[CHarvestSource::S] = 0.025f;
	res[CHarvestSource::A] = 0.0f;
	res[CHarvestSource::Q] = 0.0f;
	if ( FirstTime )
	{
		FirstTime = false;
		fprintf( f, "A;Q;D;E;C;reqS;reqA;reqQ;qty;scheme;limit;\n" );
	}
	testSource->beginExtraction( TDataSetRow::createFromRawIndex( INVALID_DATASET_INDEX ), false );
	bool eventD = false, eventE = false, eventC = false;
	for ( uint i=0; i!=nbIterations; ++i )
	{
		TDataSetRow row;
		CHarvestSource::TRealTimeProp propDrop;
		testSource->extractMaterial( req, abs, ForageQualityCeilingFactor.get(), ForageQualitySlowFactor.get(), res, successFactor, 0, row, propDrop );
		fprintf( f, "%g;%g;%g;%g;%g;%g;%g;%g;%g;%u;%u;\n",
			res[CHarvestSource::A], res[CHarvestSource::Q],
			testSource->getD(), testSource->getE(), 0.f /*testSource->getC()*/,
			reqS, reqA, reqQ,
			testSource->quantity(), testSource->getImpactScheme()*5, 127 );
		if ( (!eventD) && (testSource->getD() > 127) )
		{
			fprintf( f2, "D: %u\n", i );
			eventD = true;
		}
		if ( (!eventE) && (testSource->getE() > 127) )
		{
			fprintf( f2, "E: %u\n", i );
			eventE = true;
		}
		/*if ( (!eventC) && (testSource->getC() > 127) )
		{
			fprintf( f2, "C: %u\n", i );
			eventC = true;
		}*/
	}
	if ( !eventD )
		fprintf( f2, "D---\n" );
	if ( !eventE )
		fprintf( f2, "E---\n" );
	if ( !eventC )
		fprintf( f2, "C---\n" );
	fclose( f );
	fclose( f2 );

	return true;
}

void forageTestDoEnd()
{
	CHarvestSource *testSource = CHarvestSourceManager::getInstance()->getEntity( TestSourceRow );
	if ( ! testSource )
		return;

	CHarvestSourceManager::getInstance()->destroyEntity( testSource->rowId() );
	testSource = NULL;
}

NLMISC_COMMAND( forageTestBegin, "Start forage test", "" )
{
	forageTestDoBegin();
	return true;
}

NLMISC_COMMAND( forageTestExtract, "Make a test extraction (floats in percent)",
	"<nbIterations> <reqPeriod=2> <reqA=2> <reqQ=50> <absorption=10> <successFactor=100>" )
{
	// Read args
	sint n = (sint)args.size();
	uint nbIterations = 1;
	float reqPeriod = 2.0f;
	float reqA = 2.0f;
	float reqQ = 50.0f;
	float absorption = 0.1f;
	float successFactor = 1.0f;
	if ( n > 0 )
	{
		NLMISC::fromString(args[0], nbIterations);
		if ( n > 1 )
		{
			NLMISC::fromString(args[1], reqPeriod);
			if ( n > 2)
			{
				NLMISC::fromString(args[2], reqA);
				if ( n > 3 )
				{
					NLMISC::fromString(args[3], reqQ);
					if ( n > 4)
					{
						NLMISC::fromString(args[4], absorption);
						absorption /= 100.0f;
						if ( n > 5 )
						{
							NLMISC::fromString(args[5], successFactor);
							successFactor /= 100.0f;
						}
					}
				}
			}
		}
	}

	return forageTestDoExtract( log, nbIterations, reqPeriod, reqA, reqQ, absorption, successFactor );
}

NLMISC_COMMAND( forageTestBatch, "Batch forage tests", "" )
{
	uint nbIterations = 15;
	float reqPeriod;
	float reqA;
	float reqQ;
	float absorption;
	float successFactor = 1.0f;

	for ( absorption=0.1f; absorption<=0.8f; absorption+=0.7f )
	{
		forageTestDoBegin();
		for ( reqQ=1.0f; reqQ<=251.0f; reqQ+=50.0f )
		{
			for ( reqA=1.0f; reqA<=5.0f; reqA+=2.0f )
			{
				for ( reqPeriod=2.2f; reqPeriod>=0.2f; reqPeriod-=0.5f )
				{
					forageTestDoExtract( log, nbIterations, reqPeriod, reqA, reqQ, absorption, successFactor );
					CHarvestSource *testSource = CHarvestSourceManager::getInstance()->getEntity( TestSourceRow );
					if ( testSource )
					{
						CHarvestSource templateSource;
						templateSource.setLifetime( 1140 );
						templateSource.setProspectionExtraExtractionTime( 1140 );
						testSource->resetSource( templateSource );
						testSource->setN( 120 );
						testSource->setD( 0 );
						testSource->setE( 0 );
						//testSource->setC( 0 );
					}
				}
			}
		}
		forageTestDoEnd();
	}
	
	return true;
}

NLMISC_COMMAND( forageTestEnd, "End forage test", "" )
{
	// TODO: despawn spawned source!
	forageTestDoEnd();
	return true;
}



NLMISC_DYNVARIABLE( uint, NbForageSources, "Number of forage sources" )
{
	if ( get )
		*pointer = CHarvestSourceManager::getInstance()->nbEntities();
}

NLMISC_VARIABLE( uint, NbAutoSpawnedForageSources, "Number of auto-spawned forage sources" );

