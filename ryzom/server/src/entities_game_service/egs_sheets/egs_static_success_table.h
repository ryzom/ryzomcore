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



#ifndef RY_EGS_STATIC_SUCCESS_TABLE_H
#define RY_EGS_STATIC_SUCCESS_TABLE_H

// Nel georges
#include "nel/georges/u_form.h"
// game share
#include "game_share/constants.h"
#include "game_share/action_nature.h"

// success table types
namespace SUCCESS_TABLE_TYPE
{
	enum TSuccessTableType
	{
		FightPhrase = 0,
		FightDefense,
		FightDefenseAI,
		ShieldUse,
		OffensiveMagicCast,
		CurativeMagicCast,
		MagicResistDirect,
		MagicResistLink,
		Craft,
		ForageExtract,
		BreakCastResist,
		
		NB_TABLE_TYPES,
		Unknown = NB_TABLE_TYPES,
	};
	
	/// convert a type to a string
	const std::string &toString(TSuccessTableType type);
	
	/// convert a string to a success table type
	TSuccessTableType toSuccessTableType( const std::string &str);

	/// convert action nature to success table type
	TSuccessTableType actionNatureToTableType( ACTNATURE::TActionNature actionNature);
};



// Success table for success probability computing and associated xp-gains
struct CSuccessXpLine
{
	sint8	RelativeLevel;
	uint8	SuccessProbability;
	uint8	PartialSuccessMaxDraw;
	float	XpGain;
	
	void serial(class NLMISC::IStream &f)
	{
		f.serial(RelativeLevel);
		f.serial(SuccessProbability);
		f.serial(PartialSuccessMaxDraw);
		f.serial(XpGain);
	}
};

/**
 * class for success tables 
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CStaticSuccessTable
{
public:	
	/// Serialisation
	virtual void serial(class NLMISC::IStream &f);
	
	/// read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);
	
	// return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 9; } 
	
	/// called when the sheet is removed
	void removed() {}

	/// called to copy from another sheet (operator= + care ptrs)
	void reloadSheet(const CStaticSuccessTable &o);
	
	/** 
	 * get success chances for relative level
	 * \param relativeLevel the action relative level (actor_level - difficulty_level, the higher the easier)
	 * \return success probabilities (0..99)
	 */
	inline uint8 getSuccessChance( sint32 relativeLevel ) const
	{
		// clip delta level to min / max value
		if( relativeLevel > MAX_DELTA_LVL ) relativeLevel = MAX_DELTA_LVL;
		if( relativeLevel < MIN_DELTA_LVL ) relativeLevel = MIN_DELTA_LVL;
		
		return _SuccessXpTable[ MIDDLE_DELTA_LVL - relativeLevel ].SuccessProbability;
	}
	
	/** 
	 * get partial success chances for relative level
	 * \param relativeLevel the action relative level (actor_level - difficulty_level, the higher the easier)
	 * \return partial or full success probabilities (0..99)
	 */
	inline uint8 getPartialSuccessChance( sint32 relativeLevel ) const
	{
		// clip delta level to min / max value
		if( relativeLevel > MAX_DELTA_LVL ) relativeLevel = MAX_DELTA_LVL;
		if( relativeLevel < MIN_DELTA_LVL ) relativeLevel = MIN_DELTA_LVL;
		
		return _SuccessXpTable[ MIDDLE_DELTA_LVL - relativeLevel ].PartialSuccessMaxDraw;
	}

	/**
	 * get success factor
	 * \param relativeLevel the action relative level (actor_level - difficulty_level, the higher the easier)
	 * \param tirage result of the randomizer
	 * \param fadeClip if lower success value is min success factor (default = false) (used for craft)
	 * \return success factor
	 */
	inline float getSuccessFactor( sint32 relativeLevel, uint8 tirage, bool fadeClip = false ) const;

	/**
	 * get XP Gain for given delta	
	 * \param deltaLevel the action delta level (actor_level - difficulty_level, the higher the easier)
	 * \return the xp gain 
	 */
	inline float getXPGain(sint32 deltaLevel) const
	{
		// clip delta level to min / max value
		if( deltaLevel > MAX_DELTA_LVL ) deltaLevel = MAX_DELTA_LVL;
		if( deltaLevel < MIN_DELTA_LVL ) deltaLevel = MIN_DELTA_LVL;
		
		return _SuccessXpTable[ MIDDLE_DELTA_LVL - deltaLevel ].XpGain;
	}

	/// static method to init an array on succes tables, to get a table from it's type
	static void initTables();

	/// get table from it's type
	static const CStaticSuccessTable *getTableForType(SUCCESS_TABLE_TYPE::TSuccessTableType type)
	{
#ifdef NL_DEBUG
		nlassert(_Init);
#endif
		if (type >= 0 && type < SUCCESS_TABLE_TYPE::NB_TABLE_TYPES)
			return _Tables[type];
		else
			return NULL;
	}

	/**
	 * get success chances for action type
	 * \param type the type of success table to use
	 * \param relativeLevel the action relative level (actor_level - difficulty_level, the higher the easier)
	 * \return success probabilities (0..99)
	 */ 
	static uint8 getSuccessChance(SUCCESS_TABLE_TYPE::TSuccessTableType type, sint32 relativeLevel)
	{
		 const CStaticSuccessTable *table = getTableForType(type);
		 if (table)
		 {
			 return table->getSuccessChance(relativeLevel);
		 }
		 else
			 return 0;
	}

	/**
	 * get success factor
	 * \param type the type of success table to use	
	 * \param relativeLevel the action relative level (actor_level - difficulty_level, the higher the easier)
	 * \param tirage result of the randomizer
	 * \param fadeClip if lower success value is min success factor (default = false) (used for craft)
	 * \return success factor
	 */
	static float getSuccessFactor(SUCCESS_TABLE_TYPE::TSuccessTableType type, sint32 relativeLevel, uint8 tirage, bool fadeClip = false)
	{
		const CStaticSuccessTable *table = getTableForType(type);
		if (table)
		{
			return table->getSuccessFactor(relativeLevel, tirage, fadeClip);
		}
		else
			return 0.0f;
	}

	/**
	 * get XP Gain for given delta	
	 * \param type the type of success table to use	
	 * \param deltaLevel the action delta level (actor_level - difficulty_level, the higher the easier)
	 * \return the xp gain 
	 */
	static float getXPGain(SUCCESS_TABLE_TYPE::TSuccessTableType type, sint32 deltaLevel)
	{
		const CStaticSuccessTable *table = getTableForType(type);
		if (table)
		{
			return table->getXPGain(deltaLevel);
		}
		else
			return 0.0f;
	}

	/// get AverageDodgeFactor
	static float getAverageDodgeFactor() { return _AverageDodgeFactor; }

public:
	/// table lines
	std::vector< CSuccessXpLine >	_SuccessXpTable;

	/// max success value (>=1)
	float	_MaxSuccessFactor;
	/// max partial success value (<=_MaxSuccessFactor)
	float	_MaxPartialSuccessFactor;
	/// min fade success value (<=_MaxPartialSuccessFactor)
	float	_MinPartialSuccessFactor;
	/// max normalized 'tirage' value before success factor starts to fade (if tirage < _FullSuccessRoll then factor = _MaxSuccess)
	uint8	_FullSuccessRoll;
	/// max normalized 'tirage' value, above this value successFactor = 0 (if tirage = _FadeRoll then factor = _FadeSuccess)
	uint8	_MinSuccessRoll;

	/// bool, set to true if tables array is init
	static bool	_Init;

	/// average dodge factor
	static float _AverageDodgeFactor;

	// the tables array
	static const CStaticSuccessTable * _Tables[SUCCESS_TABLE_TYPE::NB_TABLE_TYPES];
};



//--------------------------------------------------------------
//					getSuccessFactor()  
// Return SF (Success Factor) > 1 if critical success
// Return SF = 1 if success
// Return  1 < SF < 0 if partial success
// Return SF = 0 if Failure
//--------------------------------------------------------------
float CStaticSuccessTable::getSuccessFactor( sint32 relativeLevel, uint8 tirage, bool fadeClip ) const
{	
	// clip delta level to min / max value
	if( relativeLevel > MAX_DELTA_LVL ) relativeLevel = MAX_DELTA_LVL;
	if( relativeLevel < MIN_DELTA_LVL ) relativeLevel = MIN_DELTA_LVL;
	
	uint8 chances = _SuccessXpTable[ MIDDLE_DELTA_LVL - relativeLevel ].SuccessProbability;
	uint8 maxPartialDraw = _SuccessXpTable[ MIDDLE_DELTA_LVL - relativeLevel ].PartialSuccessMaxDraw;

	chances = std::min( (uint8)100, chances );
	tirage = std::min( (uint8)100, tirage );
	
	if( fadeClip )
	{
		tirage = std::min( tirage, (uint8)maxPartialDraw );
	}

	if( tirage > maxPartialDraw ) 
		return 0.0f;
	
	if( tirage <= chances) 
		return _MaxSuccessFactor;

	// we cannot have scaledFadeRoll == scaledFullSuccess at this point of code (we would have exit just above in such a case, so no check needed)
	return _MaxPartialSuccessFactor - ( _MaxPartialSuccessFactor - _MinPartialSuccessFactor ) * (tirage - chances) / (maxPartialDraw - chances);
}


#endif // RY_EGS_STATIC_SUCCESS_TABLE_H

/* End of egs_static_success_table.h */





















