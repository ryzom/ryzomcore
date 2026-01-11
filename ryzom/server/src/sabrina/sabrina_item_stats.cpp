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
#include "nel/misc/command.h"
// game_share
#include "game_share/tick_event_handler.h"
// sabrina
#include "sabrina_item_stats.h"

using namespace std;
using namespace NLMISC;


//--------------------------------------------------------------
//					Global Variables
//--------------------------------------------------------------

bool UseWeaponDamageFactorModifier = true;


//--------------------------------------------------------------
//					Data Tables
//--------------------------------------------------------------

struct CHitChance
{
	int DeltaLevel;
	float PercentSuccess;
	int MaxLevelBoost;
	float XPMultiplier;	
};

const CHitChance TableChanceMeleeHitByAdversaryLevel[]=
{
	{7,	95.0f,	 0,	2.1f},
	{6,	95.0f,	 0,	2.1f},	
	{5,	95.0f,	 0,	2.1f},	
	{4,	90.0f,	 0,	1.9f},	
	{3,	80.0f,	 0,	1.7f},	
	{2,	70.0f,	 0,	1.5f},	
	{1,	60.0f,	 1,	1.2f},	
	{0,	50.0f,	 0,	1.0f},	
	{-1,	40.0f,	 0,	0.8f},	
	{-2,	30.0f,	 0,	0.3f},	
	{-3,	20.0f,	 0,	0.1f},	
	{-4,	10.0f,	 0,	0.0f},	
	{-5,	5.0f,	 0,	0.0f}
};

const CHitChance TableChanceSpellHitByAdversaryLevel[]=
{
	{7,	95.0f, 	0,	2.1f},
	{6,	95.0f,	 0,	2.1f},	
	{5,	95.0f,	 0,	2.1f},	
	{4,	90.0f,	 0,	1.9f},	
	{3,	80.0f,	 0,	1.7f},	
	{2,	70.0f,	 0,	1.5f},	
	{1,	60.0f,	1,	1.2f},	
	{0,	50.0f,	 0,	1.0f},	
	{-1,	40.0f,	 0,	0.8f},	
	{-2,	30.0f,	 0,	0.3f},	
	{-3,	20.0f,	 0,	0.1f},	
	{-4,	10.0f,	 0,	0.0f},	
	{-5,	5.0f,	 0,	0.0f}
};

// note on size modifiers
// attacker is smaller than defender +1
// attacker is a bit bigger -2
// attacker is much bigger -4

struct CWeaponGenerics
{
	int Quality;
	int RateOfFire;
	float DamageFactor;
	float SecondaryDamageFactor; // damage factor when entity level > weapon quality
};

const CWeaponGenerics TableMeleeWeaponGenerics[]=
{
	// light weapon			medium weapon			heavy weapon
	{1,1,6.0f,2.5f},	  	{1,1,8.0f,3.7f},   		{1,1,17.92f,8.5f},   
	{2,1,3.75f,2.25f},	  	{2,1,5.0f,3.0f},   		{2,1,11.2f,7.0f},    
	{3,1,3.0f,2.25f},	  	{3,1,4.0f,3.0f},   		{3,1,8.96f,7.0f},    
	{4,1,2.25f,2.25f},	  	{4,1,3.0f,3.0f},   		{4,1,6.72f,6.72f},   
	{5,1,2.25f,2.25f},	  	{5,1,3.0f,3.0f},   		{5,1,6.72f,6.72f},   
	{6,1,2.25f,2.25f},	  	{6,1,3.0f,3.0f},   		{6,1,6.72f,6.72f},   
	{7,1,2.25f,2.25f},	  	{7,1,3.0f,3.0f},   		{7,1,6.72f,6.72f},   
	{8,1,3.0f,3.0f},	  	{8,1,4.0f,4.0f},   		{8,1,8.96f,8.96f},   
	{9,1,3.0f,3.0f},	  	{9,1,4.0f,4.0f},   		{9,1,8.96f,8.96f},   
	{10,1,3.0f,3.0f},	  	{10,1,4.0f,4.0f},  		{10,1,8.96f,8.96f},  
	{11,1,3.75f,3.75f},	  	{11,1,5.0f,5.0f},  		{11,1,11.2f,11.2f},  
	{12,1,3.75f,3.75f},	  	{12,1,5.0f,5.0f},  		{12,1,11.2f,11.2f},  
	{13,1,3.75f,3.75f},	  	{13,1,5.0f,5.0f},  		{13,1,11.2f,11.2f},  
	{14,1,4.5f,4.5f},	  	{14,1,6.0f,6.0f},  		{14,1,13.44f,13.44f},
	{15,1,5.25f,5.25f},	  	{15,1,7.0f,7.0f},  		{15,1,15.68f,15.68f},
	{16,1,6.0f,6.0f},	  	{16,1,8.0f,8.0f},  		{16,1,17.92f,17.92f},
	{17,1,6.75f,6.75f},	  	{17,1,9.0f,9.0f},  		{17,1,20.16f,20.16f},
	{18,1,7.5f,7.5f},	  	{18,1,10.0f,10.0f},		{18,1,22.4f,22.4f},  
	{19,1,8.25f,8.25f},	  	{19,1,11.0f,11.0f},		{19,1,24.64f,24.64f},
	{20,1,9.75f,9.75f},	  	{20,1,13.0f,13.0f},		{20,1,29.12f,29.12f},
	{21,1,11.25f,11.25f}, 	{21,1,15.0f,15.0f},		{21,1,33.6f,33.6f},  
};
const uint32 NumWeaponGenerics= sizeof(TableMeleeWeaponGenerics)/(3*sizeof(TableMeleeWeaponGenerics[0]));

struct CArmorGenerics
{
	int Quality ;
	int PercentProtection;
	int MaxProtection ;
};

const CArmorGenerics TableArmorGenerics[]=
{
	// light		medium			heavy
	{1,20,2},		{1,35,3},		{1,50,4},          
	{2,20,2},	 	{2,35,3},      	{2,50,5},          
	{3,20,2},	 	{3,35,4},      	{3,50,6},          
	{4,20,3},	 	{4,35,5},      	{4,50,7},          
	{5,20,3},	 	{5,35,6},      	{5,50,8},          
	{6,20,4},	 	{6,35,7},      	{6,50,10},         
	{7,20,5},	 	{7,35,8},      	{7,50,12},         
	{8,20,6},	 	{8,35,10},     	{8,50,14},         
	{9,20,7},	 	{9,35,12},     	{9,50,17},         
	{10,20,8},	 	{10,35,14},    	{10,50,21},        
	{11,20,10},	 	{11,35,17},    	{11,50,25},        
	{12,20,12},	 	{12,35,21},    	{12,50,30},        
	{13,20,14},	 	{13,35,25},    	{13,50,36},        
	{14,20,17},	 	{14,35,30},    	{14,50,43},        
	{15,20,21},	 	{15,35,36},    	{15,50,51},        
	{16,20,25},	 	{16,35,43},    	{16,50,62},        
	{17,20,30},	 	{17,35,52},    	{17,50,74},        
	{18,20,35},	 	{18,35,62},    	{18,50,89},        
	{19,20,43},	 	{19,35,75},    	{19,50,106},       
	{20,20,51},	 	{20,35,89},    	{20,50,128},       
	{21,20,61},	 	{21,35,107},   	{21,50,153},       
};
const uint32 NumArmorGenerics= sizeof(TableArmorGenerics)/(3*sizeof(TableArmorGenerics[0]));


//--------------------------------------------------------------
//					CWeaponStats methods
//--------------------------------------------------------------

CWeaponStats::CWeaponStats()
{
	_SpeedInTicks = 0;
	_Damage = 0;
	_Quality = 0;
	_Range = 0;
	_DmgType = DMGTYPE::UNDEFINED;
	_Skill = SKILLS::unknown;
	_Family = ITEMFAMILY::UNDEFINED;
}

CWeaponStats::CWeaponStats(const CGameItemPtr &itemPtr,uint16 quality)
{
	_SpeedInTicks = 0;
	_Damage = 0;
	_Quality = quality;
	_Range = 0;
	_DmgType = DMGTYPE::UNDEFINED;
	_Skill = SKILLS::unknown;
	_Family = ITEMFAMILY::UNDEFINED;

	if (itemPtr == NULL || itemPtr->getStaticForm() == NULL )
		return;

	_Family = itemPtr->getStaticForm()->Family;

	switch(_Family)
	{
	case ITEMFAMILY::MELEE_WEAPON:
		if (!itemPtr->getStaticForm()->MeleeWeapon)
			return;
		_DmgType = itemPtr->getStaticForm()->MeleeWeapon->DamageType;
		_Skill = itemPtr->getStaticForm()->Skill;
		break;

	case ITEMFAMILY::RANGE_WEAPON:
		if (!itemPtr->getStaticForm()->RangeWeapon)
			return;
		_DmgType = itemPtr->getStaticForm()->RangeWeapon->DamageType;
		_Skill = itemPtr->getStaticForm()->Skill;
		_Range = itemPtr->getRange();
		break;

	case ITEMFAMILY::AMMO:
		if (!itemPtr->getStaticForm()->Ammo)
			return;
		_DmgType = itemPtr->getStaticForm()->Ammo->DamageType;
		_Range = itemPtr->getRange();
		break;

	default:
		break;
	};

	if (_Skill == SKILLS::unknown)
	{
		nlwarning("<CCombatWeapon::CCombatWeapon> Error : item %s skill is Unknown", itemPtr->getStaticForm()->Name.c_str());
	}

	_SpeedInTicks = uint16(itemPtr->getSpeed() / CTickEventHandler::getGameTimeStep());	
	_Quality = itemPtr->quality();

	if ( UseWeaponDamageFactorModifier )
		_Damage = itemPtr->getDamage() * 260 / (10 + _Quality*10);
	else
		_Damage = itemPtr->getDamage();


	// SkillValue must be init in the calling class
	_SkillValue = 0;
} // CCombatWeapon //

std::string CWeaponStats::toString() const
{
	const std::string temp = 
		NLMISC::toString("Damage = %u, Quality = %u, SkillValue = %u, dmgType = %s, Family = %s", 
		_Damage, _Quality, _SkillValue,DMGTYPE::toString(_DmgType).c_str(), ITEMFAMILY::toString(_Family).c_str() );
	return temp;
}


//--------------------------------------------------------------
//					CArmouStats methods
//--------------------------------------------------------------
CArmorStats::CArmorStats()
{
	_Quality=0;
	_Skill=SKILLS::unknown;
	_ArmorType=ARMORTYPE::UNKNOWN;
	for (int i=0;i<DMGTYPE::NBTYPES;++i)
	{
		_ProtectionFactor[i]=0;
		_ProtectionLimit[i]=0;
	}
}

CArmorStats::CArmorStats( const CStaticItem *item, uint16 quality )
{
	// initialise just like default constructor just in case we find an invalid item and decide to return
	_Quality=0;
	_Skill=SKILLS::unknown;
	_ArmorType=ARMORTYPE::UNKNOWN;
	for (int i=0;i<DMGTYPE::NBTYPES;++i)
	{
		_ProtectionFactor[i]=0;
		_ProtectionLimit[i]=0;
	}
	// make sure the item is valid
	if (!item || !item->Armor) return;

	// the item was valid so retrieve in the correct item stats
	_Quality = quality;
	_ArmorType = item->Armor->ArmorType;
	_Skill = item->Skill;

	// getArmorCharacteristics
	uint32 tblIdx= (quality==0)? 0: (quality >= NumArmorGenerics)? NumArmorGenerics*3: quality*3;
	uint8 percentProtect;
	uint16 maxProtect;
	switch(item->Armor->ArmorType)
	{
	case ARMORTYPE::HEAVY:	++tblIdx;
	case ARMORTYPE::MEDIUM:	++tblIdx;
	case ARMORTYPE::LIGHT:
		percentProtect= TableArmorGenerics[tblIdx].PercentProtection;
		maxProtect=		TableArmorGenerics[tblIdx].MaxProtection;
		break;

	default:
		nlwarning("CArmorStats::CArmorStats(): Unknown armor type %d, return 0", item->Armor->ArmorType);
		percentProtect = 0;
		maxProtect = 0;
	};

	for (int i=0;i<DMGTYPE::NBTYPES;++i)
	{
		_ProtectionFactor[i]=percentProtect;
		_ProtectionLimit[i]=maxProtect;
	}
} // CCombatArmor //

std::string CArmorStats::toString(bool overrideTitle) const
{
	std::string result= overrideTitle? "": "Armor";
	result+=NLMISC::toString(": %s,",ARMORTYPE::armorTypeToString(_ArmorType));
	result+=NLMISC::toString(", Quality: %d",_Quality);
	result+=NLMISC::toString(", Skill: %s",SKILLS::toString(_Skill));
	for (sint16 i=0;i<DMGTYPE::NBTYPES;++i)
		result+=NLMISC::toString(", %s(%d,%d)",DMGTYPE::toString((DMGTYPE::EDamageType)i),_ProtectionFactor[i],_ProtectionLimit[i]);
	return result;
}

float CArmorStats::getProtectionFactor(uint32 index)	const 
{
	if (index>=DMGTYPE::NBTYPES)
	{
		nlwarning("BUG: getProtectionFactor(uint32 index): index out of bounds: %d (should be < %d)",index,DMGTYPE::NBTYPES);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return 0;
	}
	return _ProtectionFactor[index]; 
}

uint32 CArmorStats::getProtectionLimit(uint32 index)		const 
{
	if (index>=DMGTYPE::NBTYPES)
	{
		nlwarning("BUG: getProtectionLimit(uint32 index): index out of bounds: %d (should be < %d)",index,DMGTYPE::NBTYPES);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return 0;
	}
	return _ProtectionLimit[index]; 
}

void CArmorStats::setProtectionFactor(uint32 index,float val)
{
	if (index>=DMGTYPE::NBTYPES)
	{
		nlwarning("BUG: setProtectionFactor(): index out of bounds: %d (should be < %d)",index,DMGTYPE::NBTYPES);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}
	_ProtectionFactor[index]=val; 
}

void CArmorStats::setProtectionLimit(uint32 index,uint32 val)
{
	if (index>=DMGTYPE::NBTYPES)
	{
		nlwarning("BUG: setProtectionLimit(): index out of bounds: %d (should be < %d)",index,DMGTYPE::NBTYPES);
		#ifdef NL_DEBUG
			nlstop
		#endif
		return;
	}
	_ProtectionLimit[index]=val; 
}

//--------------------------------------------------------------
//					NLMISC COMMANDS
//--------------------------------------------------------------

NLMISC_COMMAND(UseWeaponDamageFactorModifier,"toggle the use of the item damage factor (temporary patch item damage )","")
{
	UseWeaponDamageFactorModifier = !UseWeaponDamageFactorModifier;

	log.displayNL("UseWeaponDamageFactorModifier is %s",UseWeaponDamageFactorModifier?"used":"unused");
	return true;
}

