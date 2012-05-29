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




#ifndef RY_EGS_STATIC_BRICK_H
#define RY_EGS_STATIC_BRICK_H

// Nel georges
#include "nel/georges/u_form.h"
// game share
#include "game_share/brick_types.h"
#include "game_share/brick_families.h"
#include "game_share/skills.h"
#include "game_share/people.h"
#include "game_share/action_nature.h"
#include "game_share/slot_equipment.h"
#include "game_share/item_family.h"
#include "game_share/rm_family.h"
#include "game_share/crafting_tool_type.h"
#include "server_share/target.h"
#include "game_share/characteristics.h"

//
#include "egs_static_brick.cpp.h"


/**
 * Structure to handle a player skill (generally within a skill list)
 */
class CPlayerSkill
{
public:

	/// Constructor (nothing done; use as a struct)
	CPlayerSkill() {}

	/// Init from a string such as "SFM 50". If skill unknown, return false.
	bool		initFromString( const std::string& skillAndValue );

	/* Return true if the current skill is the same as other or more skilled
	 * Ex: (SFM1SG 160, SFM1SG 150) -> true
	 *     (SFM1SG 160, SFM1S 140) -> true
	 */
	/*bool		isAsSkilledAs( const CPlayerSkill& other ) const
	{
		// To make it work again, convert Code and other.Code to string
		if ( Code.substr( 0, other.Code.size() ) == other.Code )
			return ( MaxChildrenValue >= other.Value );
		else
			return false;
	}*/

	/// Return true if Value <= valueLimit
	bool		isSkillValueLowerThan( sint valueLimit ) const
	{
		return (Value <= valueLimit);
	}

	/// Serial
	void		serial( NLMISC::IStream& s )
	{
		s.serialEnum( Code );
		s.serial( Value );
	}

	//std::string	Code;
	SKILLS::ESkills	Code;
	sint32		Value;
};

// add params to brick (input for code generator)
void addParam(const std::string &paramStr, std::vector<TBrickParam::IIdPtr> &params);

/**
 * Faber class for sabrina bricks (for bricks are faber plan mandatory)
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
struct CFaber : public NLMISC::CRefCount
{
	NL_INSTANCE_COUNTER_DECL(CFaber);
public:
	
	struct TRawMaterial 
	{
		// Needed Rm faber part of item 
		RM_FABER_TYPE::TRMFType	MpType;
		// Quantity of this Mp needed
		uint16	Quantity;
		
		void serial(class NLMISC::IStream &f)
		{
			f.serialEnum( MpType );
			f.serial( Quantity );
		}
	};

	struct TRawMaterialFormula
	{
		// Needed Rm formula faber part of item 
		NLMISC::CSheetId MpType;
		// Quantity of this Mp needed
		uint16	Quantity;
		
		void serial(class NLMISC::IStream &f)
		{
			f.serial( MpType );
			f.serial( Quantity );
		}
	};

	CFaber()
	{
		NbItemsPerUnit = 1;

		Durability = 1.0f;
		Weight = 1.0f;
		Dmg = 1.0f;
		Speed = 1.0f;
		SapLoad = 1.0f;
		Range = 1.0f;
		
		DodgeModifier = 1.0f;
		ParryModifier = 1.0f;
		AdversaryDodgeModifier = 1.0f;
		AdversaryParryModifier = 1.0f;
		
		ProtectionFactor = 1.0f;
		MaxSlashingProtection = 1.0f;
		MaxBluntProtection = 1.0f;
		MaxPiercingProtection = 1.0f;
		
		AcidProtectionFactor = 1.0f;
		ColdProtectionFactor = 1.0f;
		FireProtectionFactor = 1.0f;
		RotProtectionFactor = 1.0f;
		ShockWaveProtectionFactor = 1.0f;
		PoisonProtectionFactor = 1.0f;
		ElectricityProtectionFactor = 1.0f;

		DesertResistanceFactor = 1.0f;
		ForestResistanceFactor = 1.0f;
		LacustreResistanceFactor = 1.0f;
		JungleResistanceFactor = 1.0f;
		PrimaryRootResistanceFactor = 1.0f;

		ElementalCastingTimeFactor = 1.0f;
		ElementalPowerFactor = 1.0f;
		OffensiveAfflictionCastingTimeFactor = 1.0f;
		OffensiveAfflictionPowerFactor = 1.0f;
		HealCastingTimeFactor = 1.0f;
		HealPowerFactor = 1.0f;
		DefensiveAfflictionCastingTimeFactor = 1.0f;
		DefensiveAfflictionPowerFactor = 1.0f;

		HpBonusPerLevel = 0.0f;
		SapBonusPerLevel = 0.0f;
		StaBonusPerLevel = 0.0f;
		FocusBonusPerLevel = 0.0f;
		AllowPartialSuccess= true;
	}
	
	void serial(class NLMISC::IStream &f)
	{
		f.serial( CraftedItem );
		f.serial( NbItemsPerUnit );
		
		f.serial( Durability );
		f.serial( Weight );
		f.serial( Dmg );
		f.serial( Speed );
		f.serial( SapLoad );
		f.serial( Range );
		
		f.serial( DodgeModifier );
		f.serial( ParryModifier );
		f.serial( AdversaryDodgeModifier );
		f.serial( AdversaryParryModifier );
		
		f.serial( ProtectionFactor );
		f.serial( MaxSlashingProtection );
		f.serial( MaxBluntProtection );
		f.serial( MaxPiercingProtection );
		
		f.serial( AcidProtectionFactor );
		f.serial( ColdProtectionFactor );
		f.serial( FireProtectionFactor );
		f.serial( RotProtectionFactor );
		f.serial( ShockWaveProtectionFactor );
		f.serial( PoisonProtectionFactor );
		f.serial( ElectricityProtectionFactor );

		f.serial( DesertResistanceFactor );
		f.serial( ForestResistanceFactor );
		f.serial( LacustreResistanceFactor );
		f.serial( JungleResistanceFactor );
		f.serial( PrimaryRootResistanceFactor );

		f.serial( ElementalCastingTimeFactor );
		f.serial( ElementalPowerFactor );
		f.serial( OffensiveAfflictionCastingTimeFactor );
		f.serial( OffensiveAfflictionPowerFactor );
		f.serial( HealCastingTimeFactor );
		f.serial( HealPowerFactor );
		f.serial( DefensiveAfflictionCastingTimeFactor );
		f.serial( DefensiveAfflictionPowerFactor );

		f.serial( HpBonusPerLevel );
		f.serial( SapBonusPerLevel );
		f.serial( StaBonusPerLevel );
		f.serial( FocusBonusPerLevel );

		f.serial( AllowPartialSuccess );
		
		if (f.isReading())
		{
			uint8 size;
			f.serial(size);
			NeededMps.resize(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMps[i]);
			}

			f.serial(size);
			NeededMpsFormula.resize(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMpsFormula[i]);
			}
		}
		else
		{
			uint8 size = (uint8)NeededMps.size();
			f.serial(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMps[i] ); 
			}

			size = (uint8)NeededMpsFormula.size();
			f.serial(size);
			for (uint8 i = 0; i < size ; ++i)
			{
				f.serial( NeededMpsFormula[i] ); 
			}
		}		
	}
	
	// Crafted item
	NLMISC::CSheetId				CraftedItem;
	/// number of items built with a single 'pass' (eg. with 1 wood I can make 2 arrows, so i'll never make 1 arrow but at least 2)
	uint8							NbItemsPerUnit;	
	/// creation specific
	std::vector< TRawMaterial >		NeededMps;
	/// creation specific (formula part)
	std::vector< TRawMaterialFormula >		NeededMpsFormula;
	/// stats plan factors
	float	Durability;
	float	Weight;
	float	Dmg;
	float	Speed;
	float	SapLoad;
	float	Range;

	float	DodgeModifier;
	float	ParryModifier;
	float	AdversaryDodgeModifier;
	float	AdversaryParryModifier;

	float	ProtectionFactor;
	float	MaxSlashingProtection;
	float	MaxBluntProtection;
	float	MaxPiercingProtection;

	float	AcidProtectionFactor;
	float	ColdProtectionFactor;
	float	FireProtectionFactor;
	float	RotProtectionFactor;
	float	ShockWaveProtectionFactor;
	float	PoisonProtectionFactor;
	float	ElectricityProtectionFactor;

	float	DesertResistanceFactor;
	float	ForestResistanceFactor;
	float	LacustreResistanceFactor;
	float	JungleResistanceFactor;
	float	PrimaryRootResistanceFactor;

	float	ElementalCastingTimeFactor;
	float	ElementalPowerFactor;
	float	OffensiveAfflictionCastingTimeFactor;
	float	OffensiveAfflictionPowerFactor;
	float	HealCastingTimeFactor;
	float	HealPowerFactor;
	float	DefensiveAfflictionCastingTimeFactor;
	float	DefensiveAfflictionPowerFactor;

	/// stats factor
	float	HpBonusPerLevel;
	float	SapBonusPerLevel;
	float	StaBonusPerLevel;
	float	FocusBonusPerLevel;

	/// Some Plans can Force no Partial Success (instead replace full success)
	bool	AllowPartialSuccess;
};

typedef NLMISC::CSmartPtr<CFaber> CFaberPtr;

/**
 * Base class for sabrina bricks
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CStaticBrick
{
public:	
	// Target Restrictions
	TARGET::TTargetRestriction TargetRestriction;

	/// name of the brick
	std::string					Name;

	/// the brick family
	BRICK_FAMILIES::TBrickFamily Family;

	///type of the action of the brick
	ACTNATURE::TActionNature	Nature;

	///skills linked with the brick
	std::vector<SKILLS::ESkills> Skills;

	/// index in family
	uint8						IndexInFamily;

	/// the brick sheet id
	NLMISC::CSheetId			SheetId;

	/// Cost (>0) or credit (<0) (sabrina)
	sint16						SabrinaValue;

	/// relative Cost (>0.0f) or relative credit (<0.0f), 0.0 if not used
	float						SabrinaRelativeValue;

	/// power value of the brick (used for fight modifiers for instance, to compare with weapon required skill value)
	uint16						PowerValue;

	/// params 
	std::vector<TBrickParam::IIdPtr> Params;

	/// params as strings
	std::vector<std::string>	StringParams;

	/// skill points price
	uint32						SkillPointPrice;

	/// localisation forced by this brick if any,a phrase cannot have more than one brick setting the localisation (unless they set the same one)
	SLOT_EQUIPMENT::TSlotEquipment	ForcedLocalisation;

	/// mandatory families
	std::set<BRICK_FAMILIES::TBrickFamily>	MandatoryFamilies;
	/// optional families
	std::set<BRICK_FAMILIES::TBrickFamily>	OptionalFamilies;
	/// credit families
	std::set<BRICK_FAMILIES::TBrickFamily>	CreditFamilies;

	/// defines flag
	std::string					ForbiddenDef;
	/// forbidden flag
	std::string					ForbiddenExclude;

	/// range table used
//	NLMISC::CSheetId						RangeTable;

	/// Skills required to learn the brick (OR)
	std::vector<CPlayerSkill>				LearnRequiresOneOfSkills;

	/// Bricks required to learn the brick (AND: all of them are required)
	std::vector<NLMISC::CSheetId>			LearnRequiresBricks;

	/// Civilisation restriction
	EGSPD::CPeople::TPeople					CivRestriction;

	/// Crafting tool
	TOOL_TYPE::TCraftingToolType			ToolType;
	/// Crafting duration
	float									CraftingDuration;
	/// Faber plan
	CFaberPtr								Faber;
	/// Min Casting Time (1/10sec).
	uint8									MinCastTime;
	/// Max Casting Time (1/10sec).
	uint8									MaxCastTime;
	/// Min Range (meters).
	uint8									MinRange;
	/// Max Range (meters).
	uint8									MaxRange;
	/// faction
	std::string								Faction;
	/// minimum fame value to get this brick
	sint32									MinFameValue;
	/// when false the brick can't be used when player has no item in hand
	bool									UsableWithEmptyHands;
	
public:	
	/// Constructor
	CStaticBrick();

	/// Destructor
	virtual ~CStaticBrick();
	
	/// Serialisation
	virtual void serial(class NLMISC::IStream &f);

	/// read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	// read static bricks from gived root
	void readStaticBrick ( const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId );
		
	// return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 34 + ( SKILLS::NUM_SKILLS << 16 ); } 

	/// called when the sheet is removed
	void removed() {}

	// Load faber
	void loadFaber( const NLGEORGES::UFormElm &root, const NLMISC::CSheetId &sheetId );

	// get skill from index
	inline SKILLS::ESkills getSkill(uint8 i) const 
	{ 
		if (i<Skills.size())
			return Skills[i]; 
		else
			return SKILLS::unknown;
	}

	// get brick from family/index
	static const CStaticBrick *getBrickFromFamilyIndex(uint16 family, uint16 index);
			
	/// called to copy from another sheet (operator= + care ptrs)
	void reloadSheet(const CStaticBrick &o);
	
private:
	/// map the pair (family, indexInFamily) to the CStaticGameBrick object
	static std::map< std::pair<uint16, uint16>, NLMISC::CSheetId> _Bricks;
};


#endif // RY_EGS_STATIC_BRICK_H

/* End of egs_static_brick.h */
