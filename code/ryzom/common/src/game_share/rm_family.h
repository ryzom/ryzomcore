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



#ifndef RY_RM_FAMILY_H
#define RY_RM_FAMILY_H

#include "nel/misc/types_nl.h"


// Raw material faber type
namespace RM_FABER_TYPE
{
	/*
	 *	WARNING: if you modify,remove or add some elements in this enum, you should:
	 *		- search mpft*  in wk.uxt (for localisation text)
	 *		- search item_part_icon_* in all xml (for binding of item part to gfx icon)
	 *			particulary in bot_chat_v4.xml, where you may have to reorganize the list of item_part filter
	 */
	enum TRMFType
	{
		MPL= 0,
		MPH,
		MPP,
		MPM,
		MPG,
		MPC,
		MPGA,
		MPPE,
		MPCA,
		MPE,
		MPEN,
		MPPR,
		MPCR,
		MPRI,
		MPRE,
		MPAT,
		MPSU,
		MPED,
		MPBT,
		MPPES,
		MPSH,
		MPTK,
		MPJH,
		MPCF,
		MPVE,
		MPMF,

		Unknown,
		NUM_FABER_TYPE = Unknown
	};

	extern std::string TypeToSheetEntry[];

	/**
	  * get the right string from the given enum value
	  * \param faber_type the TRMFType value to convert
	  * \return the string associated to this enum number (UNKNOWN if the enum number not exist)
	  */
	const std::string& toString( TRMFType faber_type );

	/**
	  * get the right TRMFamily from its string
	  * \param str the input string
	  * \return the TRMFamily associated to this string (unknown if the string cannot be interpreted)
	  */
	TRMFType toFaberType( const std::string& str );

	/** From the enum, retrieve the entry field to read in Georges.
	 */
	const std::string &faberTypeToSheetEntry(TRMFType type);

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMFType e );

	/// For Client Interface, return the define name of the type (eg: "item_part_icon_MpL")
	std::string toIconDefineString( TRMFType e );

};


// Raw material family
namespace RM_FAMILY
{
	typedef uint32 TRMFamily;

	const TRMFamily Unknown = 0;

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMFamily e );

	/// Debug string
	inline std::string toString( TRMFamily e ) { return NLMISC::toString(e); }

	/// Extract number from string
	inline TRMFamily toFamily( const std::string& s ) { TRMFamily ret; NLMISC::fromString(s, ret); return ret; }
};


// Raw material group
namespace RM_GROUP
{
	typedef uint32 TRMGroup;

	const TRMGroup Unknown = 0;

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMGroup e );

	// Note: the group names are accessible on server by CMP::rmGroupToString()

	/// Debug string
	inline std::string toString( TRMGroup e ) { return NLMISC::toString(e); }

	/// Extract number from string
	inline TRMGroup toGroup( const std::string& s ) { TRMGroup ret; NLMISC::fromString(s, ret); return ret; }
};

// Raw material property
namespace RM_FABER_PROPERTY
{
	typedef	uint32 TRMFProperty;

	const TRMFProperty Unknown = 0;

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMFProperty e );

	/// Debug string
	inline std::string toString( TRMFProperty e ) { return NLMISC::toString(e); }

	/// Extract number from string
	inline TRMFProperty toFaberProperty( const std::string& s ) { TRMFProperty ret; NLMISC::fromString(s, ret); return ret; }

};


// Raw material property depth
namespace RM_FABER_QUALITY
{
	enum TFaberQuality
	{
		SLIGHTLY,
		MODERATELY,
		QUITE,
		EXTREMELY,

		Unknown,
		NUM_FABER_QUALITY = Unknown
	};

	/**
	  * get the right string from the given enum value
	  * \param fq the value to convert
	  * \return the string associated to this enum number (UNKNOWN if the enum number not exist)
	  */
	const std::string& toString( TFaberQuality v );

	/**
	  * get the right TFaberQuality from its string
	  * \param str the input string
	  * \return the TFaberQuality associated to this string (unknown if the string cannot be interpreted)
	  */
	TFaberQuality toFaberQuality( const std::string& str );

	/// Get the Localized UCString
	const ucstring& toLocalString( TFaberQuality e );

}


// Raw material color
namespace RM_COLOR
{
	enum {NumColors= 8};

	/// true if valid color.
	inline bool		validColor( sint value ) {return value>=0 && value<NumColors;}

	/**
	  * get the right string from the given uint value (stored as a uint in the sheet)
	  * \param family the TRMColor value to convert
	  * \return the string associated to this number (UNKNOWN if the enum number not exist)
	  */
	const std::string& toString( sint value );

	/// Get the Localized UCString
	const ucstring& toLocalString( sint value );

};

// Raw material stats
namespace RM_FABER_STAT_TYPE
{
	enum TRMStatType
	{
		Durability= 0,
		Weight,
		SapLoad,
		DMG,
		Speed,
		Range,
		DodgeModifier,
		ParryModifier,
		AdversaryDodgeModifier,
		AdversaryParryModifier,
		ProtectionFactor,
		MaxSlashingProtection,
		MaxBluntProtection,
		MaxPiercingProtection,
		AcidProtection,
		ColdProtection,
		RotProtection,
		FireProtection,
		ShockWaveProtection,
		PoisonProtection,
		ElectricityProtection,
		DesertResistance,
		ForestResistance,
		LacustreResistance,
		JungleResistance,
		PrimaryRootResistance,
		ElementalCastingTimeFactor,
		ElementalPowerFactor,
		OffensiveAfflictionCastingTimeFactor,
		OffensiveAfflictionPowerFactor,
		DefensiveAfflictionCastingTimeFactor,
		DefensiveAfflictionPowerFactor,
		HealCastingTimeFactor,
		HealPowerFactor,

		NumRMStatType,
		Unknown= NumRMStatType,
	};

	// get the georges identifier
	const std::string& toString( TRMStatType stats );

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMStatType stats );

	/// For each Raw material faber type, does this Stat is relevant?
	bool	isStatRelevant(RM_FABER_TYPE::TRMFType ft, TRMStatType fs);

	/** For Craft. Common EGS and client Code: Stretch the Item Stats.
     *	\param array Stat array of float [0,1]
	 *	\param statBitField, for each stat, a bit to say if used for the item crafted
	 *	\param addBonusRule. must be true server side. add or not the bonus for "best stat +35"
	 */
	void	stretchItemStats(float array[NumRMStatType], uint64 statBitField, bool addBonusRule= true);

	/** For Craft. Common EGS and client Code: get final stat validity (protection and resist: take 3 biggest values)
     *	\param array Stat array of float [0,1]
	 *	\param statBitField, for each stat, a bit to say if used for the item crafted
	 *	\return statBitField, with any magic prot or resist removed
	 */
	uint64	getStatFinalValidity(const float array[NumRMStatType], uint64 statBitField);

	/** true if the stat is a Magic Resistance stat
	 */
	bool	isMagicResistStat(TRMStatType fs);

	/** true if the stat is a Magic Protection stat
	 */
	bool	isMagicProtectStat(TRMStatType fs);
};

// raw material class
namespace RM_CLASS_TYPE
{
	enum TRMClassType
	{
		Basic = 0,
		Fine,
		Choice,
		Excellent,
		Supreme,

		NumTRMClassType,
		Unknown = NumTRMClassType
	};

	enum TRMClassEnergyLimit
	{
		MinBasicEnergy = 0,
		MaxBasicEnergy = 20,
		MinFineEnergy = 21,
		MaxFineEnergy = 35,
		MinChoiceEnergy = 36,
		MaxChoiceEnergy = 50,
		MinExcellentEnergy = 51,
		MaxExcellentEnergy = 65,
		MinSupremeEnergy = 66,
		MaxSupremeEnergy = 100
	};

	/// Get the Localized UCString
	const ucstring& toLocalString( TRMClassType classType );

	/// return the item class for a given stat energy [0..100]
	inline TRMClassType getItemClass(uint32 energy)
	{
		if( energy <= RM_CLASS_TYPE::MaxBasicEnergy )
			return RM_CLASS_TYPE::Basic;
		if( energy <= RM_CLASS_TYPE::MaxFineEnergy )
			return RM_CLASS_TYPE::Fine;
		if( energy <= RM_CLASS_TYPE::MaxChoiceEnergy )
			return RM_CLASS_TYPE::Choice;
		if( energy <= RM_CLASS_TYPE::MaxExcellentEnergy )
			return RM_CLASS_TYPE::Excellent;
		return RM_CLASS_TYPE::Supreme;
	}

};


#endif // RY_RM_FAMILY_H
/* End of rm_family.h */
