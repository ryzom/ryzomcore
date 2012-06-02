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



#ifndef RY_BRICK_FAMILIES_H
#define RY_BRICK_FAMILIES_H

#include "nel/misc/types_nl.h"
#include "brick_types.h"

namespace BRICK_FAMILIES
{
	enum TBrickFamily
	{
		// COMBAT
		// ********
		BeginCombat= 0,
			// ROOT COMBAT
			BeginCombatRoot= BeginCombat,
				BFPA= BeginCombatRoot,
			EndCombatRoot= BFPA,

			// MANDATORY COMBAT
			//BeginCombatMandatory,
			//EndCombatMandatory = ???,

			// OPTION COMBAT
			BeginCombatOption,
				BFOA = BeginCombatOption,
				BFOB,
				BFOC,
				BFOD,
				BFOE,
				BFOF,
			EndCombatOption = BFOF,

			// PARAMETER COMBAT
			BeginCombatParameter,
				BFMA = BeginCombatParameter,
				BFMB,
				BFMC,
				BFMD,
				BFME,
				BFMF,
				BFMG,
				BFMH,
				BFMRF,	// Range dmg Fire
				BFMRT,	// Range dmg Poison
				BFMRW,	// Range dmg Shockwave
				BFMRE,	// Range dmg Electricity

				BFHME,
				BFAHHME,
				BFAHCME,
				BFAHAME,
				BFAHHAME,
				BFAHLME,
				BFAHFME,
				BFKME,
				BFAKHME,
				BFAKCME,
				BFAKAME,
				BFAKHAME,
				BFAKLME,
				BFAKFME,
				BFQME,
				BFAQHME,
				BFAQCME,
				BFAQAME,
				BFAQHAME,
				BFAQLME,
				BFAQFME,

				// deprecated: only used in saibricks
				BFM1MC,
				BFM2MC,
				BFM1HMC,
				BFM1PMC,
				BFM1BMC,
				BFM1SMC,
				BFM2PMC,
				BFM2BMC,
				BFM2SMC,
				BFR1MC,
				BFR2MC,
				BFM1MD,
				BFM2MD,
				BFR1MD,
				BFR2MD,
				BFM1ME,
				BFM2ME,
				BFR1ME,
				BFR2ME,
				BFM1MF,
				BFM2MF,
				BFR1MF,
				BFR2MF,
				BFM1BMG,
				BFM2BMG,
				BFM2SMG,
				BFM1BMH,
				BFM2BMH,
				BFM2SMH,
				BFM1BMI,
				BFM2BMI,
				BFM1SMJ,
				BFM1PMK,
				BFM2PMK,
				BFR2LFML,
				BFM2SSFML,
				BFM2SAFML,
				BFM1SAFML,
				BFM1BMTMM,
				BFM2BMTMM,
				BFR1HTMM,
				BFR1BTMM,
				BFM1BSZMN,
				BFM1PSZMN,
				BFR2BZMN,
				BFM2PPZMN,
				BFM1PDMMO,
				BFR1PMMO,
				BFM1SSMMO,
				BFR2RMMO,
				BFMMQ,
				BFMR,
				BFMK,
				BFMP,
				// end deprecated
			EndCombatParameter = BFMP,

			// CREDIT COMBAT
			BeginCombatCredit,
				BFCA = BeginCombatCredit,
				BFCB,
				BFCC,
				BFCD,
				BFCE,
				BFCF,
				BFCG,
			EndCombatCredit = BFCG,
		EndCombat= EndCombatCredit,

		// MAGIC
		// ********
		BeginMagic,
			// ROOT MAGIC
			BeginMagicRoot= BeginMagic,
			BMPA = BeginMagicRoot,
			EndMagicRoot = BMPA,

			// MANDATORY MAGIC
			BeginMagicMandatory,
			BMDALEA = BeginMagicMandatory,
			BMDHTEA,
			BMOALEA,
			BMOELEA,
			BMOETEA,
			BMSTEA,
			EndMagicMandatory = BMSTEA,

			// OPTION MAGIC
			BeginMagicOption,
			BMOF = BeginMagicOption,
			BMOG,
			BMOH,
			BMOR,
			BMOV,
			EndMagicOption = BMOV,

			// PARAMETER MAGIC
			BeginMagicParameter,
			BMTODMB = BeginMagicParameter,
			BMDALMF,
			BMDALMM,
			BMDALMS,
			BMDHTMA,
			BMDHTMP,
			BMDHTMT,
			BMOALMA,
			BMOALMB,
			BMOALMD,
			BMOALMM,
			BMOALMR,
			BMOELMA,
			BMOELMC,
			BMOELME,
			BMOELMF,
			BMOELMP,
			BMOELMR,
			BMOELMS,
			BMOETMA,
			BMOETMC,
			BMOETME,
			BMOETMF,
			BMOETMP,
			BMOETMR,
			BMOETMS,
			BMSTMA,
			BMSTMC,
			BMSTMP,
			BMSTMT,
			EndMagicParameter = BMSTMT,

			// CREDIT MAGIC
			BeginMagicCredit,
			BMCA = BeginMagicCredit,
			BMCC,
			BMCP,
			BMCR,
			EndMagicCredit = BMCR,
		EndMagic= EndMagicCredit ,

		// FABER
		// ********
			BeginFaber,
			// ROOT FABER
			BeginFaberRoot= BeginFaber,
				BCPA= BeginFaberRoot,
			EndFaberRoot= BCPA,

			// MANDATORY FABER
			BeginFaberMandatory,
				BCCMEA = BeginFaberMandatory,
				BCCREA,
				BCCPEA,
				BCCAEA,
				BCCAEB,
				BCCAEC,
				BCCAED,
				BCCSEA,
				BCBMEA,
				BCBREA,
				BCFMEA,
				BCFREA,
				BCFPEA,
				BCFAEA,
				BCFAEB,
				BCFAEC,
				BCFAED,
				BCFSEA,
				BCFJEA,
				BCMMEA,
				BCMREA,
				BCMPEA,
				BCMAEA,
				BCMAEB,
				BCMAEC,
				BCMAED,
				BCMSEA,
				BCMJEA,
				BCTMEA,
				BCTREA,
				BCTPEA,
				BCTAEA,
				BCTAEB,
				BCTAEC,
				BCTAED,
				BCTSEA,
				BCTJEA,
				BCZMEA,
				BCZREA,
				BCZPEA,
				BCZAEA,
				BCZAEB,
				BCZAEC,
				BCZAED,
				BCZSEA,
				BCZJEA,
				BCRMEA,
				BCRAEA,
				BCKAMMI,
				BCKARMI,
				BCOKAMM01,
				BCOKAMR01,
				BCOKAMT01,
				BCOKARM01,
				BCOKARR01,
				BCOKART01,
				BCOKAMM02,
				BCOKAMR02,
				BCOKAMT02,
				BCOKARM02,
				BCOKARR02,
				BCOKART02,
			EndFaberMandatory = BCOKART02,

			// OPTION FABER
			BeginFaberOption,
				BCOA = BeginFaberOption,
				BCOB,
				BCOC,
				BCOD,
                BCFAOA,
                BCFMOA,
                BCFROA,
                BCFPOA,
                BCFSOA,
                BCFJOA,
                BCMAOA,
                BCMMOA,
                BCMROA,
                BCMPOA,
                BCMSOA,
                BCMJOA,
                BCTAOA,
                BCTMOA,
                BCTROA,
                BCTPOA,
                BCTSOA,
                BCTJOA,
                BCZAOA,
                BCZMOA,
                BCZROA,
                BCZPOA,
                BCZSOA,
                BCZJOA,
                BCFAOB,
                BCFMOB,
                BCFROB,
                BCFPOB,
                BCFSOB,
                BCFJOB,
                BCMAOB,
                BCMMOB,
                BCMROB,
                BCMPOB,
                BCMSOB,
                BCMJOB,
                BCTAOB,
                BCTMOB,
                BCTROB,
                BCTPOB,
                BCTSOB,
                BCTJOB,
                BCZAOB,
                BCZMOB,
                BCZROB,
                BCZPOB,
                BCZSOB,
                BCZJOB,
                BCFAOC,
                BCFMOC,
                BCFROC,
                BCFPOC,
                BCFSOC,
                BCFJOC,
                BCMAOC,
                BCMMOC,
                BCMROC,
                BCMPOC,
                BCMSOC,
                BCMJOC,
                BCTAOC,
                BCTMOC,
                BCTROC,
                BCTPOC,
                BCTSOC,
                BCTJOC,
                BCZAOC,
                BCZMOC,
                BCZROC,
                BCZPOC,
                BCZSOC,
                BCZJOC,
                BCFAOD,
                BCFMOD,
                BCFROD,
                BCFPOD,
                BCFSOD,
                BCFJOD,
                BCMAOD,
                BCMMOD,
                BCMROD,
                BCMPOD,
                BCMSOD,
                BCMJOD,
                BCTAOD,
                BCTMOD,
                BCTROD,
                BCTPOD,
                BCTSOD,
                BCTJOD,
                BCZAOD,
                BCZMOD,
                BCZROD,
                BCZPOD,
                BCZSOD,
                BCZJOD,
			EndFaberOption = BCZJOD,

			// CREDIT FABER
			BeginFaberCredit,
				BCFACA = BeginFaberCredit,
				BCFMCA,
				BCFRCA,
				BCFPCA,
				BCFSCA,
				BCFJCA,
				BCCMCA,
				BCCRCA,
				BCCPCA,
				BCMACA,
				BCMMCA,
				BCMRCA,
				BCMPCA,
				BCMSCA,
				BCMJCA,
				BCTACA,
				BCTMCA,
				BCTRCA,
				BCTPCA,
				BCTSCA,
				BCTJCA,
				BCZACA,
				BCZMCA,
				BCZRCA,
				BCZPCA,
				BCZSCA,
				BCZJCA,
				BCKAMBCA,
				BCKARBCA,
				BCFTCA,
				BCMTCA,
				BCTTCA,
				BCZTCA,
			EndFaberCredit = BCZTCA,

			// MISC FABER
			BeginFaberRawMaterial,
				FARawMaterial = BeginFaberRawMaterial,
			EndFaberRawMaterial = FARawMaterial,

			BeginFaberTool,
				FATool = BeginFaberTool,
			EndFaberTool = FATool,
		EndFaber= EndFaberTool,

		// FORAGE PROSPECTION
		// ******************
		BeginForageProspection,
			BeginForageProspectionRoot= BeginForageProspection,
                BHFPPA = BeginForageProspectionRoot,
				BHFSPA,
				BHFGPA,
			EndForageProspectionRoot = BHFGPA,

			BeginForageProspectionOption,
				BHFPPOA = BeginForageProspectionOption,
				BHFPPOB,
				BHFPPOC,
				BHFPPOD,
				BHFPPOE,
				BHFPPOF,
				BHFPPOG,
				BHFPPOH,
				BHFPPOI,
				BHFPPOJ,
				BHFPPOK,
				BHFPPOL,
				BHFPSOA,
			EndForageProspectionOption = BHFPSOA,

			BeginForageProspectionParameter,
				BHFPMA = BeginForageProspectionParameter,
				BHFPMB,
				BHFPRMFMA,
				BHFPRMFMB,
				BHFPRMFMC,
				BHFPRMFMD,
				BHFPRMFME,
				BHFPRMFMF,
				BHFPRMFMG,
				BHFPRMFMH,
				BHFPRMFMI,
				BHFPRMFMJ,
				BHFPRMFMK,
				BHFPRMFML,
				BHFPRMFMM,
				BHFPRMFMN,
				BHFPRMFMO,
				BHFPRMFMP,
				BHFPRMFMQ,
				BHFPRMFMR,
				BHFPRMFMS,
				BHFPRMFMT,
				BHFPRMFMU,
				BHFPRMFMV,
				BHFPRMFMW,
				BHFPRMFMX,
				BHFPMC,
				BHFPMD,
				BHFPME,
				BHFPMF,
				BHFPMG,
				BHFPMH,
				BHFPMI,
				BHFPMJ,
				BHFPMK,
				BHFPML,
				BHFPMM,
			EndForageProspectionParameter = BHFPMM,

			BeginForageProspectionCredit,
                BHFPCA = BeginForageProspectionCredit,
			EndForageProspectionCredit = BHFPCA,

		EndForageProspection = EndForageProspectionCredit,

		// FORAGE EXTRACTION
		// *****************
		BeginForageExtraction,
			BeginForageExtractionRoot= BeginForageExtraction,
                BHFEPA = BeginForageExtractionRoot,
			EndForageExtractionRoot = BHFEPA,

			BeginForageExtractionOption,
				BHFEOA = BeginForageExtractionOption,
				BHFEOB,
				BHFEOC,
				BHFEOD,
				BHFEOE,
                BHFEOF,
				BHFEOG,
				BHFEOH,
 			EndForageExtractionOption = BHFEOH,

			BeginForageExtractionMandatory,
				BHFEEA = BeginForageExtractionMandatory,
				BHFEEB,
				BHFEEC,
			EndForageExtractionMandatory = BHFEEC,

			BeginForageExtractionParameter,
				BHFEMA = BeginForageExtractionParameter,
				BHFEMB,
				BHFEMC,
				BHFEMD,
				BHFEME,
				BHFEMF,
				BHFEMG,
				BHFEMK,
			EndForageExtractionParameter = BHFEMK,

			BeginForageExtractionCredit,
                 BHFECA = BeginForageExtractionCredit,
			EndForageExtractionCredit = BHFECA,

		EndForageExtraction = EndForageExtractionCredit,

		// HARVEST
		// ********
		BeginHarvest,
			BeginHarvestRoot= BeginHarvest,
			RootHarvest,
			EndHarvestRoot= RootHarvest,
		EndHarvest = EndHarvestRoot,

		// TRAINING
		// ********
		BeginTraining,
			BTFOC = BeginTraining,
			BTHP,
			BTSAP,
			BTSTA,

		// special for carac buying
		BeginCharacBuy,
			BPPC = BeginCharacBuy,
			BPPM,
			BPPI,
			BPPW,
			BPPS,
			BPPB,
			BPPD,
			BPPL,
		EndCharacBuy = BPPL,

		EndTraining = EndCharacBuy,

		// Bonuses
		BeginBonus,
			BPBCA = BeginBonus,	// Craft: Durability Bonus
			BPBHFEA,			// Harvest Forage Extraction: Time Bonus
			BPBGLA,				// Generic Landmark: Extender Bonus
		EndBonus = BPBGLA,

		// TITLE
		// ******
		BeginTitle,
		BPTEA = BeginTitle,
		EndTitle = BPTEA,

		// SPECIAL FOR INTERFACE (no interaction)
		// ********
		BeginInterface,
			BIF = BeginInterface,		// Interface for Fight Action representation
			BIG,						// Interface General bricks.
		EndInterface = BIG,

		// FOR SPECIAL POWERS
		// ********
		BeginSpecialPowers,
			// root power
			BeginPowerRoot = BeginSpecialPowers,
				BSXPA = BeginSpecialPowers, // root power/aura
			EndPowerRoot = BSXPA,

			// mndatory power
			BeginPowerMandatory,
				BSXEA = BeginPowerMandatory, // power
				BSXEB, // aura
				BSCEA, // consumable power
			EndPowerMandatory = BSCEA,

			// parameters power
			BeginPowerParameter,
				BeginFightPowerParameter = BeginPowerParameter,
				BSFMA = BeginFightPowerParameter, // taunt power
				BSFMB, // shielding power
				BSFMC, // stamina aura
				BSFMD, // protection aura
				BSFME, // umbrella aura
				BSFMF, // berserk
				BSFMG, // war cry
				BSFMH, // heal stamina
				BSFMI, // fire wall
				BSFMJ, // thorn wall
				BSFMK, // water wall
				BSFML, // lightning
				EndFightPowerParameter = BSFML,

				BSXMA, // life aura
				BSXMB, // invulnerability
				BSXMC, // heal Hp

				BSDMA, // speed

				// G for general ??
				BSGMA, // heal focus
				BSGMB, // enchant weapon
					BSGMBA,
					BSGMBC,
					BSGMBE,
					BSGMBF,
					BSGMBP,
					BSGMBR,
					BSGMBS,

				BeginMagicPowerParameter,
				BSMMA = BeginMagicPowerParameter, // sap aura
				BSMMB, // anti magic shield
				BSMMC, // balance hp
				BSMMD, // heal sap
				EndMagicPowerParameter = BSMMD,

				// consumable powers
				BSCMA, // heal Hp
				BSCMB, // heal Sap
				BSCMC, // heal Sta
				BSCMD, // heal Focus
			EndPowerParameter = BSCMD,

			BeginMagicPowerCredit,
				BSXCA = BeginMagicPowerCredit, // recast time
			EndMagicPowerCredit = BSXCA,

		EndSpecialPowers = EndMagicPowerCredit,

		// FOR TIMED ACTIONS
		// ********
		BeginTimedActions,
			BAPA = BeginTimedActions,
		EndTimedActions = BAPA,

		/* If you add a new brick Type, you should change
			isRootFamily(), isMandatoryFamily(), isOptionFamily(), isCreditFamily()
			brickType()
		*/
		BeginProcEnchantement,
			BEPA = BeginProcEnchantement,
		EndProcEnchantement = BEPA,


		NbFamilies,
		Unknown,

		// Yoyo: just for code below work (isRoot etc....). remove entries when true families described
		AutoCodeCheck,

		BeginCombatMandatory= AutoCodeCheck,
		EndCombatMandatory= AutoCodeCheck,

		BeginFaberParameter= AutoCodeCheck,
		EndFaberParameter= AutoCodeCheck,

		BeginHarvestMandatory= AutoCodeCheck,
		EndHarvestMandatory= AutoCodeCheck,
		BeginHarvestOption= AutoCodeCheck,
		EndHarvestOption= AutoCodeCheck,
		BeginHarvestCredit= AutoCodeCheck,
		EndHarvestCredit= AutoCodeCheck,
		BeginHarvestParameter= AutoCodeCheck,
		EndHarvestParameter= AutoCodeCheck,

		BeginForageProspectionMandatory= AutoCodeCheck,
		EndForageProspectionMandatory= AutoCodeCheck
	};

	/**
	 * get the right brick family from the input string
	 * \param str the input string
	 * \return the TBrickFamily associated to this string (Unknown if the string cannot be interpreted)
	 */
	TBrickFamily toSBrickFamily(const std::string &str);

	/**
	 * get the string associated to a brick family
	 * \param family the TBrickFamily to convert into a string
	 * \return the family as a string
	 */
	const std::string &toString(TBrickFamily family);

	/**
	 * \return true if the family is a root one
	 */
	inline bool isRootFamily(TBrickFamily family)
	{
		return ( (family >= BeginCombatRoot && family <=EndCombatRoot) ||
			(family >= BeginMagicRoot && family <=EndMagicRoot) ||
			(family >= BeginFaberRoot && family <=EndFaberRoot) ||
			(family >= BeginHarvestRoot && family <=EndHarvestRoot) ||
			(family >= BeginForageProspectionRoot && family <=EndForageProspectionRoot) ||
			(family >= BeginForageExtractionRoot && family <=EndForageExtractionRoot) ||
			(family >= BeginPowerRoot && family <=EndPowerRoot) ||
			(family >= BeginProcEnchantement && family <=EndProcEnchantement)
			) ;
	}

	/**
	 * \return true if the family is a mandatory one
	 */
	inline bool isMandatoryFamily(TBrickFamily family)
	{
		return ( (family >= BeginCombatMandatory && family <=EndCombatMandatory) ||
			(family >= BeginMagicMandatory && family <=EndMagicMandatory) ||
			(family >= BeginFaberMandatory && family <=EndFaberMandatory) ||
			(family >= BeginHarvestMandatory && family <=EndHarvestMandatory) ||
			(family >= BeginForageProspectionMandatory && family <=EndForageProspectionMandatory) ||
			(family >= BeginForageExtractionMandatory && family <=EndForageExtractionMandatory) ||
			(family >= BeginPowerMandatory && family <=EndPowerMandatory)
			);
	}

	/**
	 * \return true if the family is a optional one
	 */
	inline bool isOptionFamily(TBrickFamily family)
	{
		return ( (family >= BeginCombatOption && family <=EndCombatOption) ||
			(family >= BeginMagicOption && family <=EndMagicOption) ||
			(family >= BeginFaberOption && family <=EndFaberOption) ||
			(family >= BeginHarvestOption && family <=EndHarvestOption) ||
			(family >= BeginForageProspectionOption && family <=EndForageProspectionOption) ||
			(family >= BeginForageExtractionOption && family <=EndForageExtractionOption)
			);
	}

	/**
	 * \return true if the family is a cred one
	 */
	inline bool isCreditFamily(TBrickFamily family)
	{
		return ( (family >= BeginCombatCredit && family <=EndCombatCredit) ||
			(family >= BeginMagicCredit && family <=EndMagicCredit) ||
			(family >= BeginFaberCredit && family <=EndFaberCredit) ||
			(family >= BeginHarvestCredit && family <=EndHarvestCredit) ||
			(family >= BeginForageProspectionCredit && family <=EndForageProspectionCredit) ||
			(family >= BeginForageExtractionCredit && family <=EndForageExtractionCredit) ||
			(family >= BeginMagicPowerCredit && family <=EndMagicPowerCredit)
			);
	}

	/**
	 * \return true if the family is a parameter one
	 */
	inline bool isParameterFamily(TBrickFamily family)
	{
		return ( (family >= BeginCombatParameter && family <=EndCombatParameter) ||
			(family >= BeginMagicParameter && family <=EndMagicParameter) ||
			(family >= BeginFaberParameter && family <=EndFaberParameter) ||
			(family >= BeginHarvestParameter && family <=EndHarvestParameter) ||
			(family >= BeginForageProspectionParameter && family <=EndForageProspectionParameter) ||
			(family >= BeginForageExtractionParameter && family <=EndForageExtractionParameter) ||
			(family >= BeginPowerParameter && family <=EndPowerParameter)
			);
	}

	/**
	 * \return true if the family is a special charac buy family
	 */
	inline bool isCharacBuyFamily(TBrickFamily family)
	{
		return (family >= BeginCharacBuy && family <=EndCharacBuy);
	}

	/**
	 * get the brick type from the brick family
	 */
	BRICK_TYPE::EBrickType brickType( TBrickFamily family );


}; // BRICK_TYPE

#endif // RY_BRICK_FAMILIES_H
/* End of brick_families.h */
