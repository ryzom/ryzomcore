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



#ifndef RY_EGS_STATIC_GAME_ITEM_H
#define RY_EGS_STATIC_GAME_ITEM_H

// game share
#include "game_share/item_family.h"
#include "game_share/item_type.h"
#include "game_share/slot_types.h"
#include "game_share/damage_types.h"
#include "game_share/armor_types.h"
#include "game_share/weapon_types.h"
#include "game_share/shield_types.h"
#include "game_share/crafting_tool_type.h"
#include "game_share/ecosystem.h"
#include "game_share/rm_family.h"
#include "game_share/mp_category.h"
#include "game_share/skills.h"
#include "game_share/teleport_types.h"
#include "game_share/item_infos.h"
#include "server_share/item_service_type.h"
#include "server_share/taming_tool_type.h"
#include "game_share/item_origin.h"
#include "game_share/protection_type.h"
#include "game_share/range_weapon_type.h"
#include "game_share/sp_type.h"
#include "game_share/item_special_effect.h"
//Nel georges
#include "nel/georges/u_form.h"
#include "nel/georges/u_form_elm.h"
#include "nel/georges/u_form_loader.h"
// std
#include <list>
#include <map>
#include <vector>

#include "egs_sheets/egs_static_brick.h"


struct TCommandTicket
{
	NL_INSTANCE_COUNTER_DECL(TCommandTicket);
public:
	
	virtual void serial(class NLMISC::IStream &f)
	{
		f.serial( Command );
		f.serial( Priviledge );
		f.serial( NbRun );
	}

	std::string		Command;
	std::string		Priviledge;
	uint32			NbRun;
};

namespace GUILD_OPTION
{
	enum TType
	{
		PlayerMainBuilding,
		GuildMainBuilding,
//		GuildRmFight,
//		GuildRmMagic,
//		GuildRmHarvest,
//		GuildRmCraft,
		Unknown,
	};
	TType fromString( const std::string & str );
	const std::string & toString( TType type );
//	inline EGSPD::CSPType::TSPType toSPType( TType type )
//	{
//		switch(type)
//		{
//		case GuildRmFight:
//			return EGSPD::CSPType::Fight;
//		case GuildRmMagic:
//			return EGSPD::CSPType::Magic;
//		case GuildRmHarvest:
//			return EGSPD::CSPType::Harvest;
//		case GuildRmCraft:
//			return EGSPD::CSPType::Craft;
//		default:
//			return EGSPD::CSPType::EndSPType;
//		}
//	}
}


struct CGuildOption
{
	NL_INSTANCE_COUNTER_DECL(CGuildOption);
public:

	virtual void serial(class NLMISC::IStream &f)
	{
//		f.serial( XpCost );
		f.serial( MoneyCost );
		if ( f.isReading() )
		{
			std::string str;
			f.serial(str);
			Type = GUILD_OPTION::fromString(str);
			if ( Type == GUILD_OPTION::Unknown )
			{
				nlwarning( "Invalid guild option %s",str.c_str() );
			}
		}
		else
		{
			std::string str = GUILD_OPTION::toString(Type);
			f.serial(str);
		}
	}
	
	GUILD_OPTION::TType	Type;
//	uint16				XpCost;
	uint32				MoneyCost;
};


struct CConsumable
{
	NL_INSTANCE_COUNTER_DECL(CConsumable);
public:

	CConsumable() : Family(0), LoopTimer(0), MaxNbLoops(1), OverdoseTimer(0), Data(0), ConsumptionTime(0) {}

	virtual void serial(class NLMISC::IStream &f);

	uint16		Family; // consumable family, this is NOT PERSISTENT, use FAMILY AS A STRING

	float		LoopTimer; // loop timer in seconds
	uint16		MaxNbLoops; // max number of loops
	float		OverdoseTimer; // timer during which it's impossible to use the same consumable family

	float		ConsumptionTime; // consumption time in seconds
	
	// params
	std::vector<TBrickParam::IIdPtr> Params;
	/// params as strings
	std::vector<std::string>	StringParams;

	//* Break when hit: If a creature hits the player, consumption is immediatly aborted
	//* Sit: player can consume item sit down
	//* Stand Up: player can consume item stand up, including while fighting
	//* Swim: player can consume item while swiming
	//* Mektoub: player can consume item while riding a mektoub
	union
	{			
		uint16 Data;
		
		struct
		{
			uint8	BreakWhenHit: 1;
			uint8	Sit		    : 1;
			uint8	StandUp		: 1;
			uint8	Swim		: 1;
			uint8	Mektoub		: 1;
			//
			uint8	Unused		: 3;
		} Flags;
	};		

public:
	// static map used to map family name to family index
	static std::map< std::string, uint16 >	FamiliesFromName;
	// static array used to map family index to family name
	static std::vector<std::string>			FamiliesFromIndex;

	static const std::string &getFamilyName(uint16 index)
	{
		static const std::string def("Unknown");
		BOMB_IF(index >= FamiliesFromIndex.size(),"bad index", return def);
		return FamiliesFromIndex[index];
	}

	static uint16 getFamilyIndex(const std::string &name)
	{
		const std::map< std::string, uint16 >::const_iterator it = FamiliesFromName.find(name);
		if (it == FamiliesFromName.end())
			return 0xffff;
		else
			return (*it).second;
	}
};


struct CXpCatalyser
{
	NL_INSTANCE_COUNTER_DECL(CXpCatalyser);
public:

	CXpCatalyser() : IsRingCatalyser(false), XpBonus(100) {}

	virtual void serial(class NLMISC::IStream &f);

	// true if this catalyser comes from ring session
	bool IsRingCatalyser;

	// xp bonus = amount of xp to add for one item used
	uint32		XpBonus;
};

struct CCosmetics
{
	NL_INSTANCE_COUNTER_DECL(CCosmetics);
public:

	virtual void serial(class NLMISC::IStream &f);
	uint32				VPValue;
};

struct SItemSpecialEffect
{
	enum	{MaxEffectPerItem= 4};

	ITEM_SPECIAL_EFFECT::TItemSpecialEffect EffectType;
//	std::string	EffectType;
	double		EffectArgFloat[MaxEffectPerItem];
	std::string	EffectArgString[MaxEffectPerItem];
	SItemSpecialEffect() { }
	// return false if the effect cannot be built. + warning inside
	bool	build(std::string const& str);
	virtual void serial(class NLMISC::IStream &f);
};

struct SItemSpecialEffects
{
	NL_INSTANCE_COUNTER_DECL(SItemSpecialEffects);
public:
	
	std::vector<SItemSpecialEffect>	Effects;
	virtual void serial(class NLMISC::IStream &f);
};

struct SWeapon
{
	inline SWeapon()
	{
		DamageFactor = 0;
		DamageType = DMGTYPE::UNDEFINED;
		WeaponType = WEAPONTYPE::UNKNOWN;
	}

	virtual ~SWeapon() {}

	// weapon type
	WEAPONTYPE::EWeaponType	WeaponType;

	// damage type
	DMGTYPE::EDamageType	DamageType;

	// damage factor (if fixed)
	float					DamageFactor;

	virtual void serial(class NLMISC::IStream &f)
	{
		f.serialEnum( WeaponType );
		f.serialEnum( DamageType );
		f.serial( DamageFactor );
	}
};

struct SMeleeWeapon : public SWeapon
{
	NL_INSTANCE_COUNTER_DECL(SMeleeWeapon);
public:

	inline SMeleeWeapon() : SWeapon()
	{
		RateOfFire = 0.0f;
		Latency = 0.0f;
		ReachValue = 0;
	}

	virtual ~SMeleeWeapon() {}

	// rate of fire in seconds
	float	RateOfFire;

	// latency in seconds
	float	Latency;

	/// 'length' value of the weapon (french 'allonge')
	uint8	ReachValue;

	virtual void serial(class NLMISC::IStream &f)
	{
		SWeapon::serial( f );
		f.serial( RateOfFire );
		f.serial( Latency );
		f.serial( ReachValue );
	}
};

struct SRangeWeapon : public SWeapon
{
	NL_INSTANCE_COUNTER_DECL(SRangeWeapon);
public:

	virtual ~SRangeWeapon() {}

	virtual void serial(class NLMISC::IStream &f)
	{
		SWeapon::serial( f );
		f.serialEnum(AreaType);
		switch(AreaType) 
		{
		case RANGE_WEAPON_TYPE::Gatlin:
			f.serial(Gatling.Angle);
			f.serial(Gatling.Base);
			f.serial(Gatling.Height);
			break;
		case RANGE_WEAPON_TYPE::Missile:
			f.serial(Missile.Radius);
			f.serial(Missile.MinFactor);
			break;
		}
	}
	RANGE_WEAPON_TYPE::TRangeWeaponType AreaType;
	union
	{
		struct
		{
			uint8 Angle;
			float Base;
			float Height;
		}Gatling;
		struct
		{
			float Radius;
			float MinFactor;
		}Missile;
	};
	
};


struct SAmmo : public SWeapon
{
	NL_INSTANCE_COUNTER_DECL(SAmmo);
public:

	inline SAmmo() : SWeapon()
	{
		ShortRangeLimit = 0.0f;
		MediumRangeLimit = 0.0f;
		LongRangeLimit = 0.0f;

		RateOfFire = 0.0f;
		Latency = 0.0f;
		AmmoType = 1;
	}

	virtual ~SAmmo() {}

	// short range (max) in meters
	float	ShortRangeLimit;

	// medium range limit in meters
	float	MediumRangeLimit;

	// long range limit in meters
	float	LongRangeLimit;

	// rate of fire in seconds
	float	RateOfFire;

	// latency in seconds
	float	Latency;

	// ammo type (1 or 2)
	uint8	AmmoType;

	virtual void serial(class NLMISC::IStream &f)
	{
		SWeapon::serial( f );
		f.serial( ShortRangeLimit );
		f.serial( MediumRangeLimit );
		f.serial( LongRangeLimit );
		f.serial( RateOfFire );
		f.serial( Latency );
		f.serial( AmmoType );
	}
};

struct SArmor
{
	NL_INSTANCE_COUNTER_DECL(SArmor);
public:

	// ctor
	inline SArmor() : ArmorType( ARMORTYPE::UNKNOWN ),Protections(DMGTYPE::NBTYPES){}

	// serial
	virtual void serial(class NLMISC::IStream &f)
	{
		f.serialEnum( ArmorType );
		f.serialCont( Protections );
	}

	// armor type
	ARMORTYPE::EArmorType		ArmorType;
	
	// protections (vector index is the damage type)
	std::vector<SProtection>	Protections;
};


struct SShield : public SArmor
{
	NL_INSTANCE_COUNTER_DECL(SShield);
public:

	inline SShield() : SArmor(),ShieldType(SHIELDTYPE::NONE),Unbreakable(false){}
	
	virtual void serial(class NLMISC::IStream &f)
	{
		SArmor::serial(f);
		f.serialEnum( ShieldType );
		f.serial(Unbreakable);
	}
	// shield type
	SHIELDTYPE::EShieldType			ShieldType;

	// true if the item can't be worn
	bool Unbreakable;
};


const uint CREATURE_OR_DEPOSIT_MP_CHAR = 5;

struct CMP
{
	NL_INSTANCE_COUNTER_DECL(CMP);
public:

	CMP()
	{
		Category = MP_CATEGORY::Undefined;
		Family	 = RM_FAMILY::Unknown;
		//IsForMission = false;
	}

	/// Init the 'group <--> string' mapping
	static void loadGroups( const char *definitionFile );

	/// Set the group of the raw material
	void setGroup( RM_GROUP::TRMGroup group )
	{
		if ( _RMGroupsByFamily.size() <= (uint)Family )
			_RMGroupsByFamily.resize( ((uint)Family) + 1, RM_GROUP::Unknown);
		nlassert( (_RMGroupsByFamily[Family] == RM_GROUP::Unknown) || (_RMGroupsByFamily[Family] == group) );
		_RMGroupsByFamily[Family] = group;
	}


	struct TMpFaberParameters
	{
		RM_FABER_TYPE::TRMFType MpFaberType;
		ITEM_ORIGIN::EItemOrigin CraftCivSpec;

		// common factor
		float	Durability;
		float	Weight;
		float	SapLoad;
		
		// weapons factor (and armor)
		float	Dmg;						// melee weapon, range weapon (modifier), ammo
		float	Speed;						// ammos (modifier), melee weapon, range weapon
		float	Range;						// ammo, range weapon (modifier)
		float	DodgeModifier;				// not for ammo, but for armor too
		float	ParryModifier;				// not for ammo, but for armor too
		float	AdversaryDodgeModifier;		// not for ammo
		float	AdversaryParryModifier;		// not for ammo
		
		// armor factor
		float	ProtectionFactor;
		float	MaxSlashingProtection;
		float	MaxBluntProtection;
		float	MaxPiercingProtection;
		
		// jewel protection
		float	AcidProtection;
		float	ColdProtection;
		float	FireProtection;
		float	RotProtection;
		float	ShockWaveProtection;
		float	PoisonProtection;
		float	ElectricityProtection;
		
		// jewel resistance
		float	DesertResistance;
		float	ForestResistance;
		float	LacustreResistance;
		float	JungleResistance;
		float	PrimaryRootResistance;

		// magic focus factors
		float	ElementalCastingTimeFactor;
		float	ElementalPowerFactor;
		float	OffensiveAfflictionCastingTimeFactor;
		float	OffensiveAfflictionPowerFactor;
		float	HealCastingTimeFactor;
		float	HealPowerFactor;
		float	DefensiveAfflictionCastingTimeFactor;
		float	DefensiveAfflictionPowerFactor;
		
		// armor and jewel buff
		sint32	HpBuff;
		sint32	SapBuff;
		sint32	StaBuff;
		sint32	FocusBuff;

		TMpFaberParameters()
		{
			MpFaberType = RM_FABER_TYPE::Unknown;
			CraftCivSpec = ITEM_ORIGIN::UNKNOWN;
			Durability = 0.0f;
			Weight = 0.0f;
			SapLoad = 0.0f;
			
			Dmg = 0.0f;
			Speed = 0.0f;
			Range = 0.0f;
			DodgeModifier = 0.0f;
			ParryModifier = 0.0f;
			AdversaryDodgeModifier = 0.0f;
			AdversaryParryModifier = 0.0f;
			
			ProtectionFactor = 0.0f;
			MaxSlashingProtection = 0.0f;	
			MaxBluntProtection = 0.0f;	
			MaxPiercingProtection = 0.0f;	

			AcidProtection = 0.0f;
			ColdProtection = 0.0f;
			FireProtection = 0.0f;
			RotProtection = 0.0f;
			ShockWaveProtection = 0.0f;
			PoisonProtection = 0.0f;
			ElectricityProtection = 0.0f;

			DesertResistance = 0.0f;
			ForestResistance = 0.0f;
			LacustreResistance = 0.0f;
			JungleResistance = 0.0f;
			PrimaryRootResistance = 0.0f;
			
			ElementalCastingTimeFactor				= 0.0f;
			ElementalPowerFactor					= 0.0f;
			OffensiveAfflictionCastingTimeFactor	= 0.0f;
			OffensiveAfflictionPowerFactor			= 0.0f;
			HealCastingTimeFactor					= 0.0f;
			HealPowerFactor							= 0.0f;
			DefensiveAfflictionCastingTimeFactor	= 0.0f;
			DefensiveAfflictionPowerFactor			= 0.0f;
			
			// armor and jewel buff
			HpBuff = 0;
			SapBuff = 0;
			StaBuff = 0;
			FocusBuff = 0;
		}

		inline void serial(class NLMISC::IStream &f)
		{
			f.serial( Durability );
			f.serial( Weight );
			f.serial( SapLoad );
			
			f.serial( Dmg );
			f.serial( Speed );
			f.serial( Range );
			f.serial( DodgeModifier );
			f.serial( ParryModifier );
			f.serial( AdversaryDodgeModifier );
			f.serial( AdversaryParryModifier );
			
			f.serial( ProtectionFactor );
			f.serial( MaxSlashingProtection );
			f.serial( MaxBluntProtection );
			f.serial( MaxPiercingProtection );

			f.serial( AcidProtection );
			f.serial( ColdProtection );
			f.serial( FireProtection );
			f.serial( RotProtection );
			f.serial( ShockWaveProtection );
			f.serial( PoisonProtection );
			f.serial( ElectricityProtection );
			
			f.serial( DesertResistance );
			f.serial( ForestResistance );
			f.serial( LacustreResistance );
			f.serial( JungleResistance );
			f.serial( PrimaryRootResistance );

			f.serial( ElementalCastingTimeFactor );
			f.serial( ElementalPowerFactor );
			f.serial( OffensiveAfflictionCastingTimeFactor );
			f.serial( OffensiveAfflictionPowerFactor );
			f.serial( HealCastingTimeFactor );
			f.serial( HealPowerFactor );
			f.serial( DefensiveAfflictionCastingTimeFactor );
			f.serial( DefensiveAfflictionPowerFactor );
			
			// armor and jewel buff
			f.serial( HpBuff );
			f.serial( SapBuff );
			f.serial( StaBuff );
			f.serial( FocusBuff );

			if (f.isReading())
			{
				std::string value;
				f.serial(value);
				MpFaberType = RM_FABER_TYPE::toFaberType(value);
				f.serial(value);
				CraftCivSpec = ITEM_ORIGIN::stringToEnum(value);
			}
			else
			{
				std::string value;				
				value = RM_FABER_TYPE::toString(MpFaberType);
				f.serial(value);				
				value = ITEM_ORIGIN::enumToString(CraftCivSpec);
				f.serial(value);
			}
		};
	};

	inline void serial(class NLMISC::IStream &f)
	{
		f.serial( (uint32&)Family ); // The number never changes
		f.serialEnum( Ecosystem );				
		f.serialEnum( HarvestSkill );				
		f.serialEnum( Category );
		f.serial( StatEnergy );
		f.serial( MaxQuality );
		f.serial( Rarity );
		f.serial( MpColor );

		if( f.isReading() )
		{
			// Group
			uint32 group; // The number never changes
			f.serial( group );
			setGroup( (RM_GROUP::TRMGroup)group );

			// Parameters
			uint8 size;
			f.serial( size );
			MpFaberParameters.resize( size );
			for( uint i = 0; i < size; ++i )
			{
				f.serial( MpFaberParameters[ i ] );
			}
		}
		else
		{
			// Group
			f.serial( (uint32&)getGroup() ); // The number never changes

			// Parameters
			uint8 size = (uint8)MpFaberParameters.size();
			f.serial( size );
			for( uint i = 0; i < size; ++i )
			{
				f.serial( MpFaberParameters[ i ] );
			}
		}
	}

	typedef std::map<std::string, RM_GROUP::TRMGroup> CRmGroupByName;

	/// Get the group of which the family belongs to
	const RM_GROUP::TRMGroup&	getGroup() const { return _RMGroupsByFamily[Family]; }

	/// Return the number of raw material group slots. The actual number of groups is the number of non-empty slots.
	static uint					nbRmGroupSlots() { return (uint)_RMGroupNames.size(); }

	/// Return the name of a raw material group. Precondition: group < nbRmGroupSlots()
	static const std::string&	rmGroupToString( const RM_GROUP::TRMGroup& group ) { return _RMGroupNames[group]; }

	/// Return the raw material group corresponding to a name (return RM_GROUP::Unknown if not found)
	static RM_GROUP::TRMGroup	stringToRmGroup( const std::string& groupName ) { CRmGroupByName::const_iterator ig=_RMGroupsByName.find( groupName ); if ( ig!=_RMGroupsByName.end() ) return (*ig).second; else return RM_GROUP::Unknown; }

	/**
	 * Get the MpFaberParameters struct corresponding to the specified 'item part index'.
	 * Return NULL if the raw material can't be used for crafting the specified item part.
	 */
	const CMP::TMpFaberParameters *getMpFaberParameters( uint itemPartIndex )
	{
		for ( std::vector<CMP::TMpFaberParameters>::const_iterator ifp=MpFaberParameters.begin(); ifp!=MpFaberParameters.end(); ++ifp )
		{
			if ( (uint)((*ifp).MpFaberType) == itemPartIndex )
				return &(*ifp);
		}
		return NULL;
	}

	RM_FAMILY::TRMFamily		Family;
	ECOSYSTEM::EECosystem		Ecosystem;
	SKILLS::ESkills				HarvestSkill;
	MP_CATEGORY::TMPCategory	Category;
	uint16						StatEnergy; // 0..100
	sint16						MaxQuality;
	uint16						Rarity;
	uint8						MpColor;
	//bool						IsForMission;

	std::vector< TMpFaberParameters > MpFaberParameters;

private:

	/// Groups of families (indexed by RM_FAMILY::TRMFamily)
	static std::vector<RM_GROUP::TRMGroup> _RMGroupsByFamily;

	/// Group names indexed by RM_GROUP::TRMGroup
	static std::vector<std::string> _RMGroupNames;

	/// Groups by name
	static CRmGroupByName _RMGroupsByName;

};


struct CTamingTool
{
	NL_INSTANCE_COUNTER_DECL(CTamingTool);
public:

	CTamingTool()
	{
		Type = TAMING_TOOL_TYPE::Unknown;
		CommandRange = 0;
		MaxDonkeys = 0;
	}
	
	inline void serial(class NLMISC::IStream &f)
	{
		f.serialEnum( Type );
		f.serial( CommandRange );
		f.serial( MaxDonkeys );
	}
	
	TAMING_TOOL_TYPE::TTamingToolType Type;
	float	CommandRange;
	uint8	MaxDonkeys;
};


class IItemServiceData
{
	NL_INSTANCE_COUNTER_DECL(IItemServiceData);
public:
	/// build data needed by the given item service type, it can return NULL if the service type does not need data
	static IItemServiceData * buildItemServiceData(ITEM_SERVICE_TYPE::TItemServiceType itemServiceType);

	virtual IItemServiceData * clone() = 0;

	virtual ~IItemServiceData() {}
	virtual void serial(NLMISC::IStream & f) = 0;
};

struct CSpeedUpDPLossData : public IItemServiceData
{
	CSpeedUpDPLossData()
	{
		DurationInDays = 0.f;
	}

	virtual void serial(NLMISC::IStream & f)
	{
		f.serial(DurationInDays);
	}

	IItemServiceData * clone()	{ CSpeedUpDPLossData * ptr = new CSpeedUpDPLossData(); *ptr = *this; return ptr; }

	float DurationInDays;
};

/**
 * Class describing an Item for the brick_service - Armor, Weapon, Shield.... everything than can interact with sentences
 * \author David Fleury
 * \author Nevrax France
 * \date 2002
 */
class CStaticItem
{
public:
	typedef std::list< SLOTTYPE::TSlotType > TSlotList;

public:

	/// Constructor
	CStaticItem() { init(); }

	/// copy constructor
	CStaticItem( const CStaticItem& itm );

	/// init method
	void init();

	/// destructor
	virtual ~CStaticItem();

	/// Serialisation
	void serial(class NLMISC::IStream &f);

	/// read georges sheet
	void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId);

	// return the version of this class, increments this value when the content of this class has changed
	inline static uint getVersion () { return 68 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// called when the sheet is removed
	void removed() {}

	/// called to copy from another sheet (operator= + care ptrs)
	void reloadSheet(const CStaticItem &o);

#ifndef NO_EGS_VARS
	/** Get the base weigth for an item.
	 *	This weight must be multiplied by the craft parameter weight value
	 *	to obtain the real item weight.
	 */
	float getBaseWeight() const;
#endif
	
	std::vector<SItemSpecialEffect> lookForEffects(ITEM_SPECIAL_EFFECT::TItemSpecialEffect effectType) const;

	uint32 getMaxStackSize() const;
	
	// Public members
	/*********** Basics ************/
	// the sheet Id
	NLMISC::CSheetId			SheetId;
	/// Item Name
	std::string					Name;
	/// origin
	ITEM_ORIGIN::EItemOrigin	Origin;
	/// item family
	ITEMFAMILY::EItemFamily		Family;
	/// item type
	ITEM_TYPE::TItemType		Type;
	/// sack to create when this item is drop on the floor
	std::string					Sack;
	/// stackable or not, and stack size
	uint32						Stackable;
	// Brick Faber plan used for craft item
	NLMISC::CSheetId			CraftPlan;
	// For tp item or pet ticket and other
	uint32						ItemPrice;
	// bulk of the item
	uint32						Bulk;
	// Weigth of item
	uint32						Weight;
	/// true if the object can be dropped or sold
	bool						DropOrSell;
	/// true if the object can be dropped or sold
	bool						ShardExchangeable;
	// True if item is selleable by NPC
	bool						Saleable;
	// True if itme is No Rent (= non persistant)
	bool						NoRent;
	// true if item can be consumed to produce an effect
	bool						Consumable;
	// the effect produced if item is consumed 
	NLMISC::CSheetId			EffectWhenConsumed;
	// the emote produced if the item is consumed
	std::string					EmoteWhenConsumed;
	
	// required skill
	SKILLS::ESkills				RequiredSkill;
	// min required skill level
	uint16						MinRequiredSkillLevel;
	// require skill quality factor
	float						RequiredSkillQualityFactor;
	// require skill quality offset
	sint16						RequiredSkillQualityOffset;
	// required skill
	SKILLS::ESkills				RequiredSkill2;
	/// min required skill level
	uint16						MinRequiredSkillLevel2;
	// require skill quality factor
	float						RequiredSkillQualityFactor2;
	// require skill quality offset
	sint16						RequiredSkillQualityOffset2;
	// min required characteristic level
	CHARACTERISTICS::TCharacteristics	RequiredCharac;
	// min required characteristic level 
	uint16						MinRequiredCharacLevel;
	// required characteristic quality factor
	float						RequiredCharacQualityFactor;
	// required characteristic quality offset
	sint16						RequiredCharacQualityOffset;
	// skill modifiers against given ennemy types
	std::vector<CTypeSkillMod>	TypeSkillMods;
	
	/************ Equipment infos *************/
	/// Equipment Slots
	std::vector<std::string>	Slots;
	float						WearEquipmentMalus;
	
	/// if the item is an armor, pointer on the structure containing the specific infos
	SArmor *					Armor;
	/// if the item is a melee weapon, pointer on the structure containing the specific infos
	SMeleeWeapon *				MeleeWeapon;
	/// if the item is a range weapon, pointer on the structure containing the specific infos
	SRangeWeapon *				RangeWeapon;
	/// if the item is an ammunition, pointer on the structure containing the specific infos
	SAmmo *						Ammo;
	/// MP related values
	CMP	*						Mp;
	/// if the item is a shield, pointer on the structure containing the specific infos
	SShield *					Shield;
	
	/// for crafting tools
	TOOL_TYPE::TCraftingToolType CraftingToolType;
	/// Skill and minimum for using item
	SKILLS::ESkills				Skill;
	uint16						MinSkill;

	/// if the item is an Training tool, pointer on Taming tool structure
	CTamingTool *				TamingTool;

	/// if the item is a guild option
	CGuildOption*				GuildOption;

	/// if the item is a cosmetic
	CCosmetics*					Cosmetics;

	/// if the item is a consumable
	CConsumable*				ConsumableItem;

	/// if the item is a xp catalyser
	CXpCatalyser*				XpCatalyser;

	/// if the item is a pet ticket (see PetSheet)
	float						PetHungerCount;

	/// if the item is a food item
	float						Calories; // garanteed to be non-null

	/// if the item is a service
	ITEM_SERVICE_TYPE::TItemServiceType	ItemServiceType;
	IItemServiceData *					ItemServiceData;

	/// for pet information (as sheetId)
	NLMISC::CSheetId			PetSheet;
	
	/*** teleport destination***/
	std::string						Destination;
	TELEPORT_TYPES::TTeleportType	TpType;
	ECOSYSTEM::EECosystem			TpEcosystem;
	
	/*********** Bag ***********/
	/// slot count
	uint32						WeightMax;
	uint32						BulkMax;
	sint16						SlotCount;
	
	/*********** 3D ***********/
	// color (-2 = none, -1 = user color, >= 0 is the color index)
	sint8						Color;
	
	// Model numbers for visual equipment items, zero if not a visual equipment
	uint16						ItemIdSheetToModelNumber;
	uint16						ItemIdSheetToModelNumberLeftHands;

	// time needed to equip this item
	NLMISC::TGameCycle			TimeToEquip;

	// Special effects on the item (procs, stats modifiers, etc.)
	SItemSpecialEffects*		ItemSpecialEffects;

	// if the item a a command ticket, related datas is here
	TCommandTicket*				CommandTicket;

private:
	void clearPtrs(bool doDelete);
};

void loadMeleeWeapon(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadRangeWeapon(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadAmmo(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadArmor(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadShield(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadFaberTool(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadHarvestTool(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadTamingTool(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadRawMaterial(  NLGEORGES::UFormElm &root, CStaticItem	*item, const NLMISC::CSheetId &sheetId );
void loadPet(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadFood( NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
//void loadGuildOption(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadCosmetics(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadItemService(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadConsumable(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadXpCatalyser(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadItemSpecialEffects(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );
void loadCommandTicket(  NLGEORGES::UFormElm &root, CStaticItem *item, const NLMISC::CSheetId &sheetId );

#endif // EGS_STATIC_GAME_ITEM_H

/* End of item.h */
