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

// nel
#include "nel/misc/string_conversion.h"
#include "nel/misc/i18n.h"

#include "rm_family.h"

using namespace std;
using namespace NLMISC;

namespace RM_FABER_TYPE
{
	// The conversion table
	const CStringConversion<TRMFType>::CPair stringTable [] =
	{
		{ "MpL", MPL },
		{ "MpH", MPH },
		{ "MpP", MPP },
		{ "MpM", MPM },
		{ "MpG", MPG },
		{ "MpC", MPC },
		{ "MpGA", MPGA },
		{ "MpPE", MPPE },
		{ "MpCA", MPCA },
		{ "MpE", MPE },
		{ "MpEN", MPEN },
		{ "MpPR", MPPR },
		{ "MpCR", MPCR },
		{ "MpRI", MPRI },
		{ "MpRE", MPRE },
		{ "MpAT", MPAT },
		{ "MpSU", MPSU },
		{ "MpED", MPED },
		{ "MpBT", MPBT },
		{ "MpPES", MPPES },
		{ "MpSH", MPSH },
		{ "MpTK", MPTK },
		{ "MpJH", MPJH },
		{ "MpCF", MPCF },
		{ "MpVE", MPVE },
		{ "MpMF", MPMF }
	};

	std::string TypeToSheetEntry[]=
	{
		"A MpL (Blade)",
		"B MpH (Hammer)",
		"C MpP (Point)",
		"D MpM (Shaft)",
		"E MpG (Grip)",
		"F MpC (Counterweight)",
		"G MpGA (Trigger)",
		"H MpPE (Firing pin)",
		"I MpCA (Barrel)",
		"J MpE (Explosive)",
		"K MpEN (Ammo jacket)",
		"L MpPR (Ammo bullet)",
		"M MpCR (Armor shell)",
		"N MpRI (Armor interior coating)",
		"O MpRE (Armor interieur stuffing)",
		"P MpAT (Armor clip)",
		"Q MpSU (Jewel stone support)",
		"R MpED (Jewel stone)",
		"S MpBT (Blacksmith tool)",
		"T MpPES (Pestle tool)",
		"U MpSH (Sharpener tool)",
		"V MpTK (Tunneling Knife)",
		"W MpJH (Jewelry hammer)",
		"X MpCF (Campfire)",
		"Y MpVE (Clothes)",
		"Z MpMF (Magic Focus)",
	};

	CStringConversion<TRMFType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);

	// convert type id to type name string
	const std::string& toString( TRMFType faber_type )
	{
		return conversion.toString(faber_type);
	}

	// convert type name to type enum value
	TRMFType toFaberType( const std::string& str )
	{
		return conversion.fromString(str);
	}

	const std::string &faberTypeToSheetEntry(TRMFType type)
	{
		nlctassert( (sizeof(TypeToSheetEntry)/sizeof(TypeToSheetEntry[0])) == NUM_FABER_TYPE );

		if(type>=NUM_FABER_TYPE)
		{
			static	std::string empty;
			return empty;
		}

		return TypeToSheetEntry[type];
	}

	/// Client: use the CI18N
	const ucstring& toLocalString( TRMFType e )
	{
		return CI18N::get("mpft" + toString(e));
	}

	std::string toIconDefineString( TRMFType e )
	{
		return string("item_part_icon_") + RM_FABER_TYPE::toString(e);
	}

};


namespace RM_FAMILY
{
	/// Get the Localized UCString
	const ucstring& toLocalString( TRMFamily e )
	{
		return CI18N::get("mpfam" + toString(e));
	}
};


namespace RM_GROUP
{
	/// Get the Localized UCString
	const ucstring& toLocalString( TRMGroup e )
	{
		return CI18N::get("mpgroup" + toString(e));
	}
};


namespace RM_FABER_PROPERTY
{
	/// Get the Localized UCString
	const ucstring& toLocalString( TRMFProperty e )
	{
		return CI18N::get("mpprop" + toString(e));
	}
};


// With which quality the RawMaterial can be used for craft
namespace RM_FABER_QUALITY
{
	// The conversion table
	const CStringConversion<TFaberQuality>::CPair stringTable [] =
	{
		{ "Slightly", SLIGHTLY },
		{ "Moderately", MODERATELY },
		{ "Quite", QUITE },
		{ "Extremely", EXTREMELY }
	};

	CStringConversion<TFaberQuality> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);

	// convert type id to type name string
	const std::string& toString( TFaberQuality fq )
	{
		return conversion.toString(fq);
	}

	// convert type name to type enum value
	TFaberQuality toFaberQuality( const std::string& str )
	{
		return conversion.fromString(str);
	}

	/// Client: use the CI18N
	const ucstring& toLocalString( TFaberQuality e )
	{
		return CI18N::get("mpfq" + toString(e));
	}

}


namespace RM_COLOR
{

	const	std::string ColorTable[]=
	{
		"Red",
		"Beige",
		"Green",
		"Turquoise",
		"Blue",
		"Purple",
		"White",
		"Black",
	};

	const	std::string UnknownColor= "Unknown";

	const std::string& toString( sint value )
	{
		nlctassert(sizeof(ColorTable)/sizeof(ColorTable[0]) == NumColors);

		if( validColor(value) )
			return ColorTable[value];
		else
			return UnknownColor;
	}

	/// Get the Localized UCString
	const ucstring& toLocalString( sint value )
	{
		return CI18N::get("mpcol" + toString(value));
	}

};//RM_COLOR


namespace RM_FABER_STAT_TYPE
{

	// The conversion table
	const CStringConversion<TRMStatType>::CPair stringTable [] =
	{
		{ "Durability", Durability },
		{ "Weight", Weight },
		{ "SapLoad", SapLoad },
		{ "DMG", DMG },
		{ "Speed", Speed },
		{ "Range", Range },
		{ "DodgeModifier", DodgeModifier },
		{ "ParryModifier", ParryModifier },
		{ "AdversaryDodgeModifier", AdversaryDodgeModifier },
		{ "AdversaryParryModifier", AdversaryParryModifier },
		{ "ProtectionFactor", ProtectionFactor },
		{ "MaxSlashingProtection", MaxSlashingProtection },
		{ "MaxBluntProtection", MaxBluntProtection },
		{ "MaxPiercingProtection", MaxPiercingProtection },
		{ "AcidProtection", AcidProtection },
		{ "ColdProtection", ColdProtection },
		{ "FireProtection", FireProtection },
		{ "RotProtection", RotProtection },
		{ "ShockWaveProtection", ShockWaveProtection },
		{ "PoisonProtection", PoisonProtection },
		{ "ElectricityProtection", ElectricityProtection },
		{ "DesertResistance", DesertResistance },
		{ "ForestResistance", ForestResistance },
		{ "LacustreResistance", LacustreResistance },
		{ "JungleResistance", JungleResistance },
		{ "PrimaryRootResistance", PrimaryRootResistance },
		{ "ElementalCastingTimeFactor", ElementalCastingTimeFactor },
		{ "ElementalPowerFactor", ElementalPowerFactor },
		{ "OffensiveAfflictionCastingTimeFactor", OffensiveAfflictionCastingTimeFactor },
		{ "OffensiveAfflictionPowerFactor", OffensiveAfflictionPowerFactor },
		{ "DefensiveAfflictionCastingTimeFactor", DefensiveAfflictionCastingTimeFactor },
		{ "DefensiveAfflictionPowerFactor", DefensiveAfflictionPowerFactor },
		{ "HealCastingTimeFactor", HealCastingTimeFactor },
		{ "HealPowerFactor", HealPowerFactor },
	};

	CStringConversion<TRMStatType> conversion(stringTable, sizeof(stringTable) / sizeof(stringTable[0]),  Unknown);

	const std::string& toString( TRMStatType stats )
	{
		// must change the convsersion table
		nlctassert(NumRMStatType == sizeof(stringTable)/sizeof(stringTable[0]));
		return conversion.toString(stats);
	}

	const ucstring& toLocalString( TRMStatType stats )
	{
		// must change en.uxt
		nlctassert(NumRMStatType == sizeof(stringTable)/sizeof(stringTable[0]));
		return CI18N::get("mpstat" + NLMISC::toString((uint)stats));
	}

	// Array saying for which item part built, what stat is useful
	class CItemPartToStat
	{
	public:
		bool	StatRelevant[RM_FABER_TYPE::NUM_FABER_TYPE * NumRMStatType];

		CItemPartToStat()
		{
			memset(StatRelevant, 0, RM_FABER_TYPE::NUM_FABER_TYPE*NumRMStatType*sizeof(bool));
		}

		void	setStatLine(RM_FABER_TYPE::TRMFType ft, const uint8 vals[NumRMStatType])
		{
			nlassert(sizeof(uint8)==sizeof(bool));
			nlassert(ft<RM_FABER_TYPE::NUM_FABER_TYPE);
			memcpy(StatRelevant + ft*NumRMStatType, vals, NumRMStatType*sizeof(bool));
		}

	};
	static CItemPartToStat		ItemPartToStat;


	bool	isStatRelevant(RM_FABER_TYPE::TRMFType ft, TRMStatType fs)
	{
		// must change the setup below
		nlctassert(NumRMStatType == 34 && RM_FABER_TYPE::NUM_FABER_TYPE == 26);
		if(ft>=RM_FABER_TYPE::NUM_FABER_TYPE || fs>=NumRMStatType)
			return false;

		// build
		static bool init= false;
		if(!init)
		{
			init= true;
			// Hardcoded array
			//						 Dur Wgt Sap Dmg Spd Rng Dog Par ADo APa Prt Slh Bln Prc Apt CPt FPt RPt SPt PPt EPt DRt FRt LRt JRt PRt DAC DAP DHC DHP OAC OAP OEC OEP
			const uint8	mpL[]=		{1  ,1  ,1  ,1  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Blade
			const uint8	mpH[]=		{1  ,1  ,1  ,1  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Hammer
			const uint8	mpP[]=		{1  ,1  ,1  ,1  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Point
			const uint8	mpM[]=		{1  ,1  ,1  ,1  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Shaft
			const uint8	mpG[]=		{1  ,1  ,1  ,0  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Grip
			const uint8	mpC[]=		{1  ,1  ,1  ,0  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Counterweight
			const uint8	mpGA[]=		{1  ,1  ,1  ,0  ,1  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Trigger
			const uint8	mpPE[]=		{1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Firing pin
			const uint8	mpCA[]=		{1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Barrel
			const uint8	mpE[]=		{1  ,1  ,0  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Explosive
			const uint8	mpEN[]=		{1  ,1  ,0  ,0  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Ammo jacket
			const uint8	mpPR[]=		{1  ,1  ,0  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Ammo bullet
			const uint8	mpCR[]=		{1  ,1  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Armor shell
			const uint8	mpRI[]=		{1  ,1  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Lining
			const uint8	mpRE[]=		{1  ,1  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Stuffing
			const uint8	mpAT[]=		{1  ,1  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Armor clip
			const uint8	mpSU[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	,1	,1	,1	,1	,1	,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Jewel stone support
			const uint8	mpED[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Jewel stone
			const uint8	mpBT[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Blacksmith tool
			const uint8	mpPES[]=	{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Pestle tool
			const uint8	mpSH[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Sharpener tool
			const uint8	mpTK[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// TunnelingKnife
			const uint8	mpJH[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Jewelry hammer
			const uint8	mpCF[]=		{1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Campfire
			const uint8	mpVE[]=		{1  ,1  ,0  ,0  ,0  ,0  ,1  ,1  ,0  ,0  ,1  ,1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0	};		// Clothes
			const uint8	mpMF[]=		{1  ,1  ,1  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,1  ,1  ,1  ,1  ,1  ,1  ,1  ,1	};		// Magic Focus
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPL, mpL);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPH, mpH);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPP, mpP);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPM, mpM);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPG, mpG);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPC, mpC);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPGA, mpGA);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPPE, mpPE);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPCA, mpCA);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPE, mpE);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPEN, mpEN);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPPR, mpPR);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPCR, mpCR);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPRI, mpRI);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPRE, mpRE);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPAT, mpAT);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPSU, mpSU);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPED, mpED);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPBT, mpBT);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPPES, mpPES);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPSH, mpSH);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPTK, mpTK);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPJH, mpJH);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPCF, mpCF);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPVE, mpVE);
			ItemPartToStat.setStatLine(RM_FABER_TYPE::MPMF, mpMF);
		}

		// get
		return ItemPartToStat.StatRelevant[ft*NumRMStatType + fs];
	}


	// ***************************************************************************
	// GamePlay tuning
	const	float	StretchStatMinDeltaWanted= 0.3f;
	const	float	StretchStatMaxDeltaFactor= 2.0f;
	const	float	StretchStatBonusDeltaThreshold= 0.35f;	// if the player get +35% or more in one stat
	const	float	StretchStatBonusValue= 0.1f;			// then +10%!
	// stretch the item stats
	void	stretchItemStats(float array[NumRMStatType], uint64 statBitField, bool addBonusRule)
	{
		uint	i;

		// *** ensure 0-1.
		for(i=0;i<NumRMStatType;i++)
		{
			clamp(array[i], 0.f, 1.f);
		}

		// *** compute the mean of each stat used, the max stat value
		float	energy= 0;
		uint	numStatUsed= 0;
		float	maxStat= 0;
		uint	bestStat= 0;
		for(i=0;i<NumRMStatType;i++)
		{
			// if the item use this stat
			if(statBitField&((uint64)(1)<<i))
			{
				energy+= array[i];
				numStatUsed++;
				if(array[i]>maxStat)
				{
					maxStat= array[i];
					bestStat= i;
				}
			}
		}
		// mean, and difference max-mean
		float	meanStat= 0;
		if(numStatUsed)
			meanStat= energy / numStatUsed;
		float	maxDelta= maxStat-meanStat;
		// if all stats are equals, no-op
		if(maxDelta==0.f)
			return;

		// *** if the player has succed in applying a nearly perfect bonus for its best stat (eg:+40%), add an additional bonus
		float	bestStatBonusValue= 0.f;
		// also, add this bonus only if addBonusRule
		if(addBonusRule && maxDelta>=StretchStatBonusDeltaThreshold)
		{
			bestStatBonusValue= StretchStatBonusValue;
		}

		// *** stretch the maxDelta so it reaches at least StretchStatMinDeltaWanted (eg: +30%)
		if(maxDelta < StretchStatMinDeltaWanted)
		{
			float	stretchFactor= StretchStatMinDeltaWanted / maxDelta;
			stretchFactor= min(stretchFactor, StretchStatMaxDeltaFactor);

			// For all stats, stretch the delta
			float	newEnergy=0;
			float	numStatNot0= 0;
			float	numStatNot1= 0;
			for(i=0;i<NumRMStatType;i++)
			{
				// if the item use this stat
				if(statBitField&((uint64)(1)<<i))
				{
					float	delta= array[i] - meanStat;
					delta*= stretchFactor;
					// add the delta to the mean
					array[i]= meanStat+delta;
					clamp(array[i], 0.f, 1.f);

					/// new sum of stats
					newEnergy+= array[i];

					// count not clamped stats
					if(array[i]>0.f)
						numStatNot0++;
					if(array[i]<1.f)
						numStatNot1++;
				}
			}

			/// loss or gain of energy because of Clamp 0 or 1 ?
			float	deltaEnergy = newEnergy - energy;

			// while more than 0.1% of energy loss or gained (float precision...)
			uint	nbPass=0;
			while(fabs(deltaEnergy)>(0.001f*numStatUsed))
			{
				// redistribute delta among all stats not clamped
				float	deltaStat;
				// if we gain too much energy because of ClampTo0, decrement Not0 Stats.
				if(deltaEnergy>0)
					deltaStat= -deltaEnergy/numStatNot0;
				// if we loss too much energy because of ClampTo1, increment Not1 Stats.
				else
					deltaStat= -deltaEnergy/numStatNot1;

				// redistribute all stats, to get correct energy
				newEnergy=0;
				numStatNot0= 0;
				numStatNot1= 0;
				for(i=0;i<NumRMStatType;i++)
				{
					// if the item use this stat
					if(statBitField&((uint64)(1)<<i))
					{
						// add delta, and re-clamp
						array[i]+= deltaStat;
						clamp(array[i], 0.f, 1.f);
						newEnergy+= array[i];

						if(array[i]>0.f)
							numStatNot0++;
						if(array[i]<1.f)
							numStatNot1++;

					}
				}

				// This pass may have regenerate clamp problems.
				deltaEnergy = newEnergy - energy;

				// this cannot happen, because in the worst case, all stats are clamped to 0 or 1.
				nbPass++;
				nlassert(nbPass<=NumRMStatType);
			}
		}


		// **** add the best stat bonus value (at the end)
		if(bestStatBonusValue)
		{
			array[bestStat]+= bestStatBonusValue;
			clamp(array[bestStat], 0.f, 1.f);
		}
	}


	// ***************************************************************************
	// method used by getStatFinalValidity, for both magic protection and magic resist
	uint64  filterStatValidity3BestOne(const float array[NumRMStatType], uint64 bfIn, uint statStart, uint statEnd)
	{
		uint64	ret= bfIn;

		// init 3 bests
		const uint NumBests= 3;
		sint	bestStat[NumBests];
		float	bestValues[NumBests];
		for(uint i=0;i<NumBests;i++)
		{
			bestStat[i]= -1;
			bestValues[i]= -1;
		}

		// get 3 bests
		for(uint i=statStart;i<statEnd;i++)
		{
			// if the stat is really present
			if(bfIn & uint64(1)<<i)
			{
				// get the stat value
				float	val= array[i];
				if( val > bestValues[0] )
				{
					bestStat[2]= bestStat[1];
					bestValues[2]= bestValues[1];
					bestStat[1]= bestStat[0];
					bestValues[1]= bestValues[0];
					bestStat[0]= i;
					bestValues[0]= val;
				}
				else if( val > bestValues[1] )
				{
					bestStat[2]= bestStat[1];
					bestValues[2]= bestValues[1];
					bestStat[1]= i;
					bestValues[1]= val;
				}
				else if( val > bestValues[2] )
				{
					bestStat[2]= i;
					bestValues[2]= val;
				}
			}
		}

		// then keep only bits that are sets
		for(uint i=statStart;i<statEnd;i++)
		{
			// if the stat is really present
			if(bfIn & uint64(1)<<i)
			{
				// test if 1 of 3 best stat match the index
				bool	keep= false;
				for(uint bt=0;bt<NumBests;bt++)
				{
					if(bestStat[bt]==(sint)i)
					{
						keep= true;
						break;
					}
				}
				// don't keep? reset bit
				if(!keep)
				{
					ret&= ~(uint64(1)<<i);
				}
			}
		}

		return ret;
	}

	// ***************************************************************************
	uint64	getStatFinalValidity(const float array[NumRMStatType], uint64 statBitField)
	{
		uint64	ret= statBitField;

		// *** Magic Protections
		// should have 7 magic protection. Acid should be the 1st, and Eletrcity the last
		const uint	startMProt= AcidProtection;
		const uint	endMProt= ElectricityProtection+1;
		nlctassert(endMProt - startMProt == 7);

		// if the item has some magic protection
		const uint64	mProtBF= ((uint64(1) << endMProt)-1) - ((uint64(1) << startMProt)-1);
		if(statBitField & mProtBF)
		{
			ret= filterStatValidity3BestOne(array, ret, startMProt, endMProt);
		}

		// *** Magic Resistances
		// should have 5 magic resistances. Desert should be the 1st, and PrimRoot the last
		const uint	startMResist= DesertResistance;
		const uint	endMResist= PrimaryRootResistance+1;
		nlctassert(endMResist - startMResist == 5);

		// if the item has some magic Resistection
		const uint64	mResistBF= ((uint64(1) << endMResist)-1) - ((uint64(1) << startMResist)-1);
		if(statBitField & mResistBF)
		{
			ret= filterStatValidity3BestOne(array, ret, startMResist, endMResist);
		}

		return ret;
	}

	// ***************************************************************************
	bool	isMagicResistStat(TRMStatType fs)
	{
		// should have 5 magic resistances. Desert should be the 1st, and PrimRoot the last
		const sint	startMResist= DesertResistance;
		const sint	endMResist= PrimaryRootResistance+1;
		nlctassert(endMResist - startMResist == 5);

		return fs>=startMResist && fs<endMResist;
	}

	// ***************************************************************************
	bool	isMagicProtectStat(TRMStatType fs)
	{
		// should have 7 magic protection. Acid should be the 1st, and Electricity the last
		const sint	startMProt= AcidProtection;
		const sint	endMProt= ElectricityProtection+1;
		nlctassert(endMProt - startMProt == 7);

		return fs>=startMProt && fs<endMProt;
	}

};

namespace RM_CLASS_TYPE
{

const ucstring &toLocalString(TRMClassType classType)
{
	return CI18N::get(toString("uiItemRMClass%d", classType).c_str());
}



}
