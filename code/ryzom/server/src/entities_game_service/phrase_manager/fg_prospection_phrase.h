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



#ifndef NL_FG_PROSPECTION_PHRASE_H
#define NL_FG_PROSPECTION_PHRASE_H

#include "forage_phrase.h"
#include "game_share/ecosystem.h"
#include "game_share/time_weather_season/time_and_season.h"


/// Possible reasons for not finding anything
enum TNothingFoundReason
{
	NFUnknown,
	NFNoDepositHere,					//
	NFInvalidCurrentSeason,				//
	NFInvalidCurrentTimeOfDay,			//
	NFInvalidCurrentWeather,			//
	NFInvalidEcotype,					//
	NFNoDepositForFilter,				//
	NFNoLocalMaterialForFilter,			//
	NFStatEnergyTooHigh,				//
	NFStatEnergyTooHighLocal,			//
	NFStatEnergyDifferent,				//
	NFStatEnergyDifferentLocal,			//
	NFSiteDepleted,						//
	NFDepositDepleted,					//
	NFCantSpawnSource,					//
	NFNbReasons // if you change this enum, change NothingFoundReasonStrs in .cpp!
};


class CDeposit;

/**
 * Common class for search actions (including prospection)
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CFgSearchPhrase : public CForagePhrase
{
public:

	/// Constructor
	CFgSearchPhrase();

	/// \name Override methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );

	/**
	 * evaluate phrase
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate();
	
	/**
	 * validate phrase
	 * \return true if phrase is valide
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch() = 0;
	virtual void apply() = 0;
	//@}
	
	/**
	 * set the actor
	 */
	virtual void setActor( const TDataSetRow &entityRowId ){}
	
	//@}

	/**
	 * called when the action is interupted
	 */
	virtual void stop();
	//@}

	///\unused basic methods from CSPhrase
	//@{
	virtual void setPrimaryTarget( const TDataSetRow& ) {}
	//@}

protected: // because CSearchPhrase is a common trunc

	/// Terrain specialization
	TEcotype					_EcotypeSpec;

	/// Material specialization (group)
	RM_GROUP::TRMGroup			_MaterialGroupFilter;

	/// Material specialization (family)
	RM_FAMILY::TRMFamily		_MaterialFamilyFilter;

	/// Craftable item part index specialization
	uint						_CraftableItemPartFilter;

	/// Range (radius in m)
	float						_ForageRange;

	/// Angle (in radian)
	float						_ForageAngle;

	/// Multi spot limit
	uint16						_MultiSpotLimit;

	/// Number of attempts (when filtered)
	uint16						_NbAttempts;

	/// Initial source properties
	CHarvestSource				_SourceIniProperties;

	/// Skill value of the skill used
	sint32						_UsedSkillValue;

	/// Knowledge precision
	uint8						_KnowledgePrecision;

	/// Max or exact RM energy searched
	uint8						_MaterialEnergy;

	/// True if _MaterialEnergy is the exact searched, false if max searched
	bool						_FindExactMaterialEnergy;

	/// Locator enabled
	bool						_IsLocateDepositProspection;
};


class CDeposit;

/**
 * Prospection action
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2003
 */
class CFgProspectionPhrase : public CFgSearchPhrase
{
public:

	/// Constructor
	CFgProspectionPhrase() : CFgSearchPhrase() {}

	/// Launch the action
	virtual bool	launch();

	/// Apply the action
	virtual void	apply();

	/// Called at the end of the latency time
	virtual void	end();

	/// Generate one or more sources when applying the action. Return the number of sources spawned.
	uint			generateSources( CCharacter *player );

	///
	void			startLocateDeposit( CCharacter *player );

	///
	void			stopLocateDeposit( CCharacter *player );

	/**
	 * Display forageable raw materials near by or at the exact position of a player
	 * (extendedInfo is for non-exactPos mode only).
	 */
	static void		displayRMNearBy( CCharacter *player, bool onlyAtExactPos, bool extendedInfo, NLMISC::CLog *log = NLMISC::InfoLog );

	/**
	 * Select a raw material that can be found at the specified position. (static)
	 * OptFastFloorBegin()/OptFastFloorEnd() must enclose one or more calls to this method.
	 *
	 * \param sourcePos The specified position
	 * \param weather The current weather
	 * \param ecotypeSpec The ecotype specialization, or ECOSYSTEM::common
	 * \param groupFilter If not Unknown, searches only a RM from the group (ignored if familyFilter is not Unknown)
	 * \param familyFilter If not Unknown, searches only a RM from the family
	 * \param craftableItemPartFilter If not ~0, searches only a RM useful to craft the specified item part
	 * \param maxStatEnergy Discards RM that have a higher energy than specified 
	 * \param fromNeighboorhood If true, selects randomly a RM from a small area, otherwise returns only a RM at the exact pos
	 * \param testIfSiteDepleted Set it to false for map generation.
	 * \param ignoreContextMatch If true, the season/time/weather will not be checked (useful for maps)
	 *
	 * \param deposit If a RM is found, deposit will point to the deposit containing the RM. If no RM is found,
	 *   deposit will be NULL if finding a RM with the specified filters is impossible in this area in
	 *   the current context (season, weather, time of day...), or non-NULL if another attempt is needed to find one.
	 * \param depositForK The deposit at the position, matching the context, that has the low kami anger level
	 *   (NULL if deposit is NULL).
	 * \param reason The reason why nothing has been found, if the method returns NULL.
	 * \return If a RM is found, returns it, otherwise NULL.
	 */
	static const CStaticDepositRawMaterial *selectRMAtPos(
		const NLMISC::CVector& sourcePos,
		const CRyzomTime::EWeather& weather,
		const TEcotype& ecotypeSpec,
		const RM_GROUP::TRMGroup& groupFilter,
		const RM_FAMILY::TRMFamily& familyFilter,
		uint craftableItemPartFilter,
		uint8 statEnergy,
		bool findExactStatEnergy,
		bool fromNeighbourHood,
		bool testIfSiteDepleted,
		CDeposit **deposit,
		CDeposit **depositForK,
		TNothingFoundReason& reason,
		bool ignoreContextMatch=false );

	/// Spawn a source automatically from the deposit, if it matches the current context
	static bool		autoSpawnSource( const NLMISC::CVector& pos, CDeposit *deposit );

private:

	/**
	 * Reorder matchingDeposits so that [begin(), matchingEnd] is the range matching the filters.
	 * If no deposit matches the filter, return a reason different from NFUnknown.
	 */
	static void filterDeposits(
		std::vector<CDeposit*>& matchingDeposits,
		const CRyzomTime::EWeather& weather,
		const TEcotype& ecotypeSpec,
		const RM_GROUP::TRMGroup& groupFilter,
		const RM_FAMILY::TRMFamily& familyFilter,
		uint craftableItemPartFilter,
		uint8 statEnergy,
		bool findExactStatEnergy,
		std::vector<CDeposit*>::iterator& matchingEnd,
		TNothingFoundReason& reason );

};


#endif // NL_FG_PROSPECTION_PHRASE_H

/* End of fg_prospection_phrase.h */
