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


#ifndef GAME_ITEM_H
#define GAME_ITEM_H

// misc
#include "nel/misc/eval_num_expr.h"
#include "nel/misc/log.h"
#include "nel/misc/variable.h"
#include "nel/misc/enum_bitset.h"
#include "nel/misc/deep_ptr.h"

// game share
#include "game_share/ryzom_entity_id.h"
#include "game_share/protection_type.h"
#include "game_share/rm_family.h"
#include "game_share/inventories.h"
#include "game_share/characteristics.h"
#include "game_share/type_skill_mod.h"
#include "game_share/resistance_type.h"
#include "game_share/skills.h"
#include "game_share/guild_grade.h"

#include "egs_log_filter.h"
#include "player_inventory.h"
#include "item_worn_state.h"

#include "egs_variables.h"

#include "backward_compatibility/protection_type_back_compat.h"


//commented by ace because there s too much crash in releaseInventories #define GAME_PTR_DEBUG

class CCharacter;
class CStaticItem;


//------------------------------------------------------------------------
// Craft parameters for setting item stat factor
//------------------------------------------------------------------------
struct CCraftParameters
{
	// Intermediate craft factors parameters
	// Common factor
	float	Durability;
	uint32	nbDurability;
	float	Weight;
	uint32	nbWeight;
	float	SapLoad;
	uint32	nbSapLoad;
	float	quality;
	uint32	nbQuality;
	float	StatEnergy;
	uint32  nbStatEnergy;

	// weapons factor
	float	Dmg;						// melee weapon, range weapon (modifier), ammo
	uint32	nbDmg;
	float	Speed;						// ammos (modifier), melee weapon, range weapon
	uint32	nbSpeed;
	float	Range;						// ammo, range weapon (modifier)
	uint32	nbRange;
	float	DodgeModifier;				// not for ammo, but for armor too
	uint32	nbDodgeModifier;
	float	ParryModifier;				// not for ammo, but for armor too
	uint32	nbParryModifier;
	float	AdversaryDodgeModifier;		// not for ammo
	uint32	nbAdversaryDodgeModifier;
	float	AdversaryParryModifier;		// not for ammo
	uint32	nbAdversaryParryModifier;

	// magic focus factor
	float	ElementalCastingTimeFactor;
	uint32	nbElementalCastingTimeFactor;
	float	ElementalPowerFactor;
	uint32	nbElementalPowerFactor;
	float	OffensiveAfflictionCastingTimeFactor;
	uint32	nbOffensiveAfflictionCastingTimeFactor;
	float	OffensiveAfflictionPowerFactor;
	uint32	nbOffensiveAfflictionPowerFactor;
	float	HealCastingTimeFactor;
	uint32	nbHealCastingTimeFactor;
	float	HealPowerFactor;
	uint32	nbHealPowerFactor;
	float	DefensiveAfflictionCastingTimeFactor;
	uint32	nbDefensiveAfflictionCastingTimeFactor;
	float	DefensiveAfflictionPowerFactor;
	uint32	nbDefensiveAfflictionPowerFactor;

	// armor factor
	float	ProtectionFactor;
	uint32	nbProtectionFactor;
	float	MaxSlashingProtection;
	uint32	nbMaxSlashingProtection;
	float	MaxBluntProtection;
	uint32	nbMaxBluntProtection;
	float	MaxPiercingProtection;
	uint32	nbMaxPiercingProtection;
	std::vector< uint8 > Color;

	// jewel protection
	float	AcidProtectionFactor;
	uint32	nbAcidProtectionFactor;
	float	ColdProtectionFactor;
	uint32	nbColdProtectionFactor;
	float	FireProtectionFactor;
	uint32	nbFireProtectionFactor;
	float	RotProtectionFactor;
	uint32	nbRotProtectionFactor;
	float	ShockWaveProtectionFactor;
	uint32	nbShockWaveProtectionFactor;
	float	PoisonProtectionFactor;
	uint32	nbPoisonProtectionFactor;
	float	ElectricityProtectionFactor;
	uint32	nbElectricityProtectionFactor;

	// jewel resistance
	float	DesertResistanceFactor;
	uint32	nbDesertResistanceFactor;
	float	ForestResistanceFactor;
	uint32	nbForestResistanceFactor;
	float	LacustreResistanceFactor;
	uint32	nbLacustreResistanceFactor;
	float	JungleResistanceFactor;
	uint32	nbJungleResistanceFactor;
	float	PrimaryRootResistanceFactor;
	uint32	nbPrimaryRootResistanceFactor;

	// armor and jewel buff
	sint32	HpBuff;
	sint32	SapBuff;
	sint32	StaBuff;
	sint32	FocusBuff;

	CCraftParameters();
};


//------------------------------------------------------------------------
// Item Craft parameters
//------------------------------------------------------------------------
struct CItemCraftParameters
{
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS

	// Intermediate craft factors parameters
	// Common factor
	float	Durability;
	float	Weight;
	float	SapLoad;
	float	StatEnergy;					// for price compute rules

	// weapons factor
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
	uint8	Color;

	// jewel protection
	BACK_COMPAT::OLD_PROTECTION_TYPE::TOldProtectionType Protection; // WARNING: kept for compatibility with old resistance system, converted when item loaded (post apply)
	PROTECTION_TYPE::TProtectionType Protection1;
	float	Protection1Factor;
	PROTECTION_TYPE::TProtectionType Protection2;
	float	Protection2Factor;
	PROTECTION_TYPE::TProtectionType Protection3;
	float	Protection3Factor;

	// jewel resistance
	float	DesertResistanceFactor;
	float	ForestResistanceFactor;
	float	LacustreResistanceFactor;
	float	JungleResistanceFactor;
	float	PrimaryRootResistanceFactor;

	// magic focus factor
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

	CItemCraftParameters();

	void clear();

	float getCraftParameterValue( RM_FABER_STAT_TYPE::TRMStatType statType ) const;

	RM_FABER_STAT_TYPE::TRMStatType getBestItemStat() const;

	/**
	 * \return the current version of the class. Useful for managing old versions of saved players
	 * WARNING : the version number should be incremented when the serial method is modified
	 */
	static inline uint32 getCurrentVersion() { return 5; }

	/// serial validated point for a character
	void serial(NLMISC::IStream &f);

	// operator !=
	bool operator!=(const CItemCraftParameters &p) const;

	// operator ==
	bool operator==(const CItemCraftParameters &p) const;

	// operator =
	const CItemCraftParameters& operator = ( const CCraftParameters& p );

	// return true if protection are egual (equivalent)
	bool checkProtectionEgality(const CItemCraftParameters &p) const;

private:
	// copy the three bests protections
	void keepTheThreeBestProtection( const CCraftParameters& p );

	// copy the three bests resistances
	void keepTheThreeBestResistance( const CCraftParameters& p );
};

//------------------------------------------------------------------------
// advance declaration of classes with circular references
class CGameItem;
class CGameItemEntry;
//class CGameItemManager;

/**
 * CGameItemVector
 *
 * \author Sadge
 * \author Nevrax France
 * \date 2004
 */

class CGameItemVector
{
public:
	uint32 size() const;
	CGameItemEntry& operator[](uint32 idx);
	void extend();
	uint32 getUniqueIndex(const CGameItem& item);
	virtual ~CGameItemVector();

private:
	// this is a private class belonging to CGameItem
	friend class CGameItem;
	CGameItemVector();

private:
	std::vector<CGameItemEntry *> m_Data;

};

/**
 * CGameItem
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CGameItem
{
#if 0
	NLMISC_COMMAND_FRIEND(testParanoidItemSystem);
#endif

	friend class CInventoryBase;
	friend class CRefInventory;

#if 0
	NL_INSTANCE_COUNTER_DECL(CGameItem);
#endif
public:

	/// returns true if the two param items are stackable
	static bool areStackable(const CGameItemPtr item1, const CGameItemPtr item2);

public :
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	struct CPersistentApplyArg
	{
		/// ctor
		explicit CPersistentApplyArg(CCharacter * owner = NULL) : Owner(owner), InventorySlot(INVENTORIES::INVALID_INVENTORY_SLOT) {}

		/// input param
		CCharacter * Owner;
		/// output param
		uint32 InventorySlot;
	};

	DECLARE_PERSISTENCE_METHODS_WITH_APPLY_ARG(CPersistentApplyArg & applyArgs);

	/// Get the item unique id
	const INVENTORIES::TItemId &getItemId()	{	return _ItemId;}

	/// return the inventory that physically contains this item (can be null)
	const CInventoryPtr &getInventory() const
	{
		return _Inventory;
	}

	/// return the slot in the inventory that physically contains this item
	uint32 getInventorySlot() const
	{
		return _InventorySlot;
	}

	/// return the inventory that reference this item (can be null)
	const CInventoryPtr &getRefInventory() const
	{
		return _RefInventory;
	}

	/// return the slot in the inventory that reference this item
	uint32 getRefInventorySlot() const
	{
		return _RefInventorySlot;
	}

	/// Call stack size update on all needed inventory
	void callStackSizeChanged(uint32 previousStackSize);
	/// Call item changed on any all needed inventory
	void callItemChanged(INVENTORIES::TItemChangeFlags changeFlags);



	/// a clear() method used before applying a pdr record at load time to avoid parasite data problems
	void clear();

	/// send the item name Id to the specified player, and return the associated text id. Return 0 if the item has no text id
	uint32 sendNameId(CCharacter *user);

	/// Delete an item in the sub item list
//	void deleteChildItem(uint index);

	/// get recommended skill needed for use item
	uint32 recommended() const { return _Recommended; }
	/// set recommended skill needed for use item
	void recommended( uint32 r ) { _Recommended = r; computeRequiredLevel(); }
	/// get quality (= recommended for item not usable with skill (as raw material, letter...)
	uint16 quality() const { return (uint16)_Recommended; }
	/// set quality (= recommended for item not usable with skill (as raw material, letter...)
	void quality( uint16 q ) { recommended( (uint32) q ); }
	/// get current durability
	uint32 durability() const { if(_HP > maxDurability()) _HP = maxDurability(); return _HP; }
	/// get max durability
	uint32 maxDurability() const;
	/// get weight
	uint32 weight() const;
	/// get stat energy of item
	float getStatEnergy();
	// get class of item
	RM_CLASS_TYPE::TRMClassType getItemClass();
	/// estimate stat energy of an item ( used for item crafted before stat energy are used)
	float estimateStatEnergy();
	/// get the total weight of the item stack
	uint32 getStackWeight(uint32 limitQuantity = UINT_MAX) const;
	/// get the total bulk of the item stack
	uint32 getStackBulk(uint32 limitQuantity = UINT_MAX) const;
	/// get damage factor
	float damageFactor() const;
	/// get damage/max damage
	void damage( sint32 skill, uint32& currentDamage, uint32& maxDamage ) const;
	/// get hit rate
	float hitRate() const;
	// get sap load
	uint32 sapLoad() const { return _SapLoad; }
	// get maxSapLoad
	uint32 maxSapLoad() const;
	// reload sap load of item
	void reloadSapLoad( uint32 sapAdded );
	// consume sapLoad
	void consumeSapLoad( uint32 sapConsumed );
	// apply an enchantment to item
	void applyEnchantment( const std::vector< NLMISC::CSheetId >& action );
	// get enchantment
	const std::vector< NLMISC::CSheetId >& getEnchantment() const { return _Enchantment; }
	// set sapLoad for recharge sap item
	void setSapLoad( uint32 sap );
	// reset enchantment
	void resetEnchantment();
	// get range in meters
	float range() const;
	// get dodge modifier
	sint32 dodgeModifier() const;
	// get parry modifier
	sint32 parryModifier() const;
	// get adversary dodge modifier
	sint32 adversaryDodgeModifier() const;
	// get adversary parry modifier
	sint32 adversaryParryModifier() const;

	// get protection factor
	float protectionFactor() const;
	// get maximum slashing protection
	uint32 maxSlashingProtection() const;
	// get maximum blunt protection
	uint32 maxBluntProtection() const;
	// get maximum piercing protection
	uint32 maxPiercingProtection() const;
	// get color
	uint8 color() const { return _CraftParameters == 0 ? DefaultColor : _CraftParameters->Color; }

	// get one of the three possible protection, legal protection number are 1,2 or 3
	void magicProtection(uint32 protectionNumber, PROTECTION_TYPE::TProtectionType& protectionType, uint32& protectionValue) const;
	// return protection gived by item for a protection type
	uint32 magicProtection(PROTECTION_TYPE::TProtectionType protectionType) const;

	// return resistance gived by item for a resistance type
	uint32 magicResistance(RESISTANCE_TYPE::TResistanceType resistanceType) const;

	// get Hit points buff
	sint32 hpBuff() const { return _CraftParameters == 0 ? 0 : _CraftParameters->HpBuff; }
	// get Sap points buff
	sint32 sapBuff() const { return _CraftParameters == 0 ? 0 : _CraftParameters->SapBuff; }
	// get Sta points buff
	sint32 staBuff() const { return _CraftParameters == 0 ? 0 : _CraftParameters->StaBuff; }
	// get Hit points buff
	sint32 focusBuff() const { return _CraftParameters == 0 ? 0 : _CraftParameters->FocusBuff; }

	// get craft buffs as flags
	uint8 buffFlags();

	/// get the Hit points buff provided by armors
	/// it must be added to hpBuff() which does not include this bonus
	sint32 armorHpBuff() const;

	//get casting time factor for elemental spells
	float getElementalCastingTimeFactor() const;// { return _CraftParameters.ElementalCastingTimeFactor; }
	//get damage factor for elemental spells
	float getElementalPowerFactor() const;// { return _CraftParameters.ElementalPowerFactor; }
	//get casting time factor for offensive affliction spells
	float getOffensiveAfflictionCastingTimeFactor() const;
	//get power factor for offensive affliction spells
	float getOffensiveAfflictionPowerFactor() const;
	//get casting time factor for heal spells
	float getHealCastingTimeFactor() const;
	//get power factor for heal spells
	float getHealPowerFactor() const;
	//get casting time factor for defensive affliction spells
	float getDefensiveAfflictionCastingTimeFactor() const;
	//get power for defensive affliction spells
	float getDefensiveAfflictionPowerFactor() const;

	/// accessors to the item phrase
	const std::string &getPhraseId() const { static const std::string empty; return _PhraseId ? *_PhraseId : empty; }
	bool isPhraseLiteral() const { return _PhraseLiteral; }
	void setPhraseIdInternal(const std::string &str, bool literal = false) { if (!_PhraseId) _PhraseId = new std::string(); *_PhraseId = str; _PhraseLiteral = literal; }
	void setPhraseId(const std::string &str, bool literal = false);

	// return the enchantment value to be displayed in the client
	uint16 getClientEnchantValue() const;

	/// get the wear per action (Hp lost for 1 action)
	float getWearPerAction() const;

	/// Set item stats with craft parameters
	void setCraftParameters( const CCraftParameters& param );

	/**
	 * remove HP
	 * \param hpLost the number of hp to remove
	 * \return the number of hp really removed
	 */
	uint32 removeHp( double hpLost );

	/**
	 * add HP
	 * \param hpGain the number of hp to add
	 */
	void addHp( double hpGain );

	/// get item worn state
	ITEM_WORN_STATE::TItemWornState getItemWornState() const { return _CurrentWornState;	}

	/// compute item worn state
	void computeItemWornState();

	/**
	 * get the item locked state
	 * \return the number of locked item in the stack
	 */
	uint32 getLockCount() const {return _LockCount;}

	/**
	 * set the item locked state
	 * \param state :  the new number of locked items
	 */
	void setLockCount(uint32 state);

	/// Get the number quantity of non locked items (StackSize - LockState)
	uint32	getNonLockedStackSize();

	/**
	 * get the item creator
	 * \return the creator
	 */
	const NLMISC::CEntityId &getCreator() const { return _CreatorId; }

	/**
	 * set the item creator id
	 * \param id the creator Id
	 */
	void setCreator( const NLMISC::CEntityId &id ) { _CreatorId = id; }

	// return total game cycle item are left in sell store
	NLMISC::TGameCycle getTotalSaleCycle() const { return _TotalSaleCycle; }

	// set total game cycle item are left in sell store
	void setTotalSaleCycle( NLMISC::TGameCycle t ) { _TotalSaleCycle = t; }

	/// \return a pointer on a Copy of this item
	CGameItemPtr getItemCopy();

	/// \return the character looting this item (used for item on the ground only
	const NLMISC::CEntityId & getLooter(){ return _Looter;}

	/// \return the character looting this item (used for item on the ground only
	void setLooter(const NLMISC::CEntityId & looter){ _Looter = looter;}

	/// Return the size of the item stack
	uint32	getStackSize() const { return _StackSize; }
	/// Set the size of the item stack
	/// WARNING: if the item has an inventory, the inventory weight and bulk will be updated
	void setStackSize(uint32 size);
	/** Fill (add quantity) the stack with as possible quantities.
	 *	Return the number of item that dont fit in the stack
	 */
	uint32 fillStack(uint32 addQt);

	uint32 getMaxStackSize() const;

	/**
	 *	Return true if the item is on the ground, false else
	 */
//	bool isOnTheGround() const{ return _IsOnTheGround; }

	/**
	 *  flag the item as "on" the ground
	 */
//	void setAsOnTheGround() { _IsOnTheGround = true; }

	/**
	 * get the item static form
	 */
	const CStaticItem *getStaticForm() const { return _Form; }

	/**
	 * get the item sheet id
	 * \return the item's sheet id
	 */
	const NLMISC::CSheetId& getSheetId() const { return _SheetId; }
	NLMISC::CSheetId& getSheetId() { return _SheetId; }

	/// accessor to the destroyable property
	bool isDestroyable() const { return _Destroyable; }
	void destroyable( bool b ) { _Destroyable = b; }

	/// accessor to the dropable property
	bool isDropable() const { return _Dropable; }
	void dropable( bool b ) { _Dropable = b; }

	/**
	 * load this item from a file
	 */
//	void legacyLoad( NLMISC::IStream &f, uint16 characterSerialVersion, CCharacter *owner );

	/**
	 * load item from a file with marker format
	 */
//	 void legacyLoadItem( NLMISC::IStream &f );

	/**
	 * Dump the item stats in the console or in a text file
	 * \param fileName is the name of the file
	 */
	void dumpGameItemStats( const std::string& fileName ="");

	/// get Vector of CSheetId used for craft item
//	const std::vector< NLMISC::CSheetId >& getRmUsedForCraft() const { return _RmUsedForCraft; }

	/// add sheet id of RM used for craft item
	void addRmUsedForCraft( const NLMISC::CSheetId& sheet ) { /* _RmUsedForCraft.push_back( sheet ); */ }

	/**
	 * display item infos
	 */
	void displayInLog(NLMISC::CLog &log);
	bool getStats(const std::string &stats, std::string &final );

	/// accessors to the action latency end date
	inline NLMISC::TGameCycle getLatencyEndDate(){ return _LatencyEndDate; }

	inline void setLatencyEndDate( NLMISC::TGameCycle latencyEndDate ){ _LatencyEndDate = latencyEndDate; }

	/// set the max sap load craft parameter
	inline void setMaxSapLoad(float value)
	{
		if( _CraftParameters ) _CraftParameters->SapLoad = value;
	}

	/// Does this item use the New Requirement System?
	bool					getUseNewSystemRequirement() const {return _UseNewSystemRequirement;}
	/// get required skill
	inline SKILLS::ESkills	getRequiredSkill() const { return _RequiredSkill; }
	/// set required skill
	inline void setRequiredSkill(SKILLS::ESkills skill) { _RequiredSkill = skill; }
	/// get required skill level
	inline uint16 getRequiredSkillLevel() const { return _RequiredSkillLevel; }
	/// set required skill level
	inline void setRequiredSkillLevel( uint16 l ) { _RequiredSkillLevel = l; }
	/// get required skill 2
	inline SKILLS::ESkills	getRequiredSkill2() const { return _RequiredSkill2; }
	/// set required skill 2
	inline void setRequiredSkill2(SKILLS::ESkills skill) { _RequiredSkill2 = skill; }
	/// get required skill level
	inline uint16 getRequiredSkillLevel2() const { return _RequiredSkillLevel2; }
	/// set required skill level
	inline void setRequiredSkillLevel2( uint16 l ) { _RequiredSkillLevel2 = l; }

	inline bool getLockedByOwner() const { return _LockedByOwner; }
	void setLockedByOwner(bool value);
	inline EGSPD::CGuildGrade::TGuildGrade getAccessGrade() const { return _AccessGrade; }
	void setAccessGrade(EGSPD::CGuildGrade::TGuildGrade value);

	inline bool getMovable() const { return _Movable; }
	inline void setMovable(bool value) { _Movable = value; }

	inline bool getUnMovable() const { return _UnMovable; }
	inline void setUnMovable(bool value) { _UnMovable = value; }

	/// get required stat
	inline CHARACTERISTICS::TCharacteristics getRequiredCharac() const { return _RequiredCharac; }
	/// set required stat
	inline void setRequiredCharac(CHARACTERISTICS::TCharacteristics charac) { _RequiredCharac = charac; }
	///get min required stat level
	inline uint16 getRequiredCharacLevel() const { return _RequiredCharacLevel; }
	///set min required stat level
	inline void setRequiredCharacLevel( uint16 l ) { _RequiredCharacLevel = l; }

	// Recompute the Requirement from form
	void computeRequirementFromForm();



	/// get skill mods
	inline const std::vector<CTypeSkillMod> &getTypeSkillMods() const { return _TypeSkillMods; }
	/// set skill mods
	inline void setTypeSkillMods(const std::vector<CTypeSkillMod> &mods) { _TypeSkillMods = mods; }

	/// get craft parameters
	const CItemCraftParameters *getCraftParameters() const { return _CraftParameters.ptr(); }

	/// get custom string (for scroll-like items)
	const std::string &getCustomText() const { static const std::string empty; return _CustomText ? *_CustomText : empty; }
	/// set custom string (for scroll-like items)
	void setCustomText(const std::string &val);

	uint8 getPetIndex() const { return _PetIndex; }
	void setPetIndex(uint8 val) { _PetIndex = val; }

protected:
	friend class CFaberPhrase;
	// set Default Color (for craft only)
	void setDefaultColor() { if( _CraftParameters ) _CraftParameters->Color = DefaultColor; }

	/// set link information between item and container inventory (used by CInventoryBase)
	void setInventory(const CInventoryPtr &inv, uint32 slot);

	/// set link information between item and reference inventory (used by CRefInventory)
	void setRefInventory(const CInventoryPtr &inv, uint32 slot);

private:

	//--------------------------------------------------------------------
	// type for items vector

	// the container object needs to be a friend in order to setup our private data correctly
	friend class CGameItemVector;

	//--------------------------------------------------------------------
	// singleton data

	static CGameItemVector	_Items;
	static uint32			_FirstFreeItem;

private:
	//--------------------------------------------------------------------
	// singleton interface used by the CGameItemPtr class

	// make CGameItemPtr a friend to give it decent access
	friend class CGameItemEntry;
	friend class CGameItemPtr;
	friend class CInventoryProxy;
	friend class COldGuildInventoryLoader;
	friend class COldPlayerRoomInventoryLoader;

	// Get hold of the nth item in the singleton's item vector
	static CGameItem * getItem(uint idx);

	// Allocate an unused item in the singleton's item vector
	static CGameItem *newItem();

	// Add an item to the list of 'free' items awaiting re-allocation
	static void deleteItem(CGameItem *item);

	// post load treatment
	void postApply(INVENTORIES::TInventory refInventoryId, CCharacter * owner);

	// return the CWeaponCraftParameters variable corresponding to protection type
	float getMagicProtectionCraftParateters( PROTECTION_TYPE::TProtectionType protection ) const;

public:
	static std::string showItemsStats();

private :
	//--------------------------------------------------------------------
	// ctors and dtors are now private for better control over allocation etc

	/**
	 * Constructor
	 */
	CGameItem()
		: 	_InventorySlot(INVENTORIES::INVALID_INVENTORY_SLOT),
			_RefInventorySlot(INVENTORIES::INVALID_INVENTORY_SLOT)
	{
	}

	/**
	 * Destructor
	 */
	~CGameItem()
	{
	}

#ifdef NL_CPP14
	CGameItem(const CGameItem &) = default;
	CGameItem &operator=(const CGameItem &) = default;
	CGameItem(CGameItem &&) = default;
	CGameItem &operator=(CGameItem &&) noexcept = default;
#endif

private:
	//--------------------------------------------------------------------
	// item initialisation and release code called on allocation/ deallocation

	/**
	 * pseudo Constructor
	 */
	void ctor();

	/**
	 * pseudo Constructor
	 */
//	void ctor( const NLMISC::CEntityId& id, const NLMISC::CSheetId& sheetId, uint32 recommended, sint16 slotCount, bool destroyable , bool dropable);
	void ctor( const NLMISC::CSheetId& sheetId, uint32 recommended, bool destroyable , bool dropable);

	// Compute required level (skills and charac) for wearing item
	void computeRequiredLevel();

	// Compute whether item has a skill/charac prerequisit
	void computeHasPrerequisit();

	// has item prerequisit
	bool hasPrerequisit() { return _HasPrerequisit; }

	// Recompute the Requirement from Old System (old values for old items)
	void computeRequirementFromOldSystem();

protected:
	/// The inventory that contains this item (NULL if none)
	CInventoryPtr	_Inventory;
	/// The inventory that reference this item (NULL if none)
	CInventoryPtr	_RefInventory;
	/// The slot inside the inventory
	uint32			_InventorySlot;
	/// The slot inside the reference inventory
	uint32			_RefInventorySlot;

private:
	/// Item unique id
	INVENTORIES::TItemId	_ItemId;
	/// looter id of the character currently looting this item (for items on the ground only)
	NLMISC::CEntityId	_Looter;
	/// sheet ref
	NLMISC::CSheetId	_SheetId;
	/// Item stack size
	uint32				_StackSize;

	/// Recommended skill for use
	uint32				_Recommended;
	/// current hit points
	mutable uint32		_HP;
	/// lost hp floating part (kept for wear)
	float				_LostHPremains;
	/// current worn state
	ITEM_WORN_STATE::TItemWornState _CurrentWornState;

	/// current sap load
	uint32				_SapLoad;

	/// all craft parameters
	NLMISC::CDeepPtr<CItemCraftParameters> _CraftParameters;
	/// entityId of the character who has created the Item via faber (if applicable, for item not created by playres, Creator = CEntityId::Unknown)

	NLMISC::CEntityId	_CreatorId;
	/// Position in the client inventory interface if item is owned by a player (sint16 juste because we need an "invalid" position (-1) for version compatibility)
	/// Vector of CSheetId used for craft this item
//	std::vector< NLMISC::CSheetId > _RmUsedForCraft;
	/// vector of sheetId of CStaticBrick define spell of enchanted item
	std::vector<NLMISC::CSheetId> _Enchantment;

	/// pointer on the associated static form
	const CStaticItem*	_Form;
	/// string associated with this item
	NLMISC::CDeepPtr<std::string> _PhraseId;

	/// skill modifiers against given ennemy types
	std::vector<CTypeSkillMod>	_TypeSkillMods;

	NLMISC::CDeepPtr<std::string> _CustomText;

	/// tick when the proc will be available again
	NLMISC::TGameCycle	_LatencyEndDate;
	/// image of the item in bag / equipment
//	uint16				_SlotImage;
	NLMISC::TGameCycle	_TotalSaleCycle;
	/// number of item locked
	uint32				_LockCount;

	// required skill
	uint16				_RequiredSkillLevel;
	uint16				_RequiredSkillLevel2;
	/// min required stat level and required stat
	uint16				_RequiredCharacLevel;
	// required skill
	SKILLS::ESkills		_RequiredSkill;
	SKILLS::ESkills		_RequiredSkill2;
	/// min required stat level and required stat
	CHARACTERISTICS::TCharacteristics	_RequiredCharac;
	/// whether the item has any skill requirements
	bool				_HasPrerequisit;
	bool				_UseNewSystemRequirement;

	/// phrase id is a literal, not a phrase
	bool				_PhraseLiteral;

	bool                _LockedByOwner;
	EGSPD::CGuildGrade::TGuildGrade _AccessGrade;

	bool                _UnMovable;
	bool                _Movable;
	uint8               _PetIndex;

	/// true if the object is destroyable
	bool				_Destroyable;
	/// true if the object is dropable
	bool				_Dropable;
	// true if the item is on the ground
	//	bool				_IsOnTheGround;

public:
	static const EGSPD::CGuildGrade::TGuildGrade DefaultAccessGrade = EGSPD::CGuildGrade::HighOfficer;
	static const uint8 DefaultColor = 1;

};

/**
 * SShadowItem
 * class used to represent a shadow item : a fake item that acts as a reference on the real one
 * \author Nicolas Brigand
 * \author Nevrax France
 * \date 2002
 */
/*struct SShadowItem
{
	uint16 Slot;
	uint16 Inventory;
	CGameItemPtr ItemPtr;
	bool IsInStack;
};
*/

//------------------------------------------------------------------------
// game item vector inline implementation

#ifdef NL_DEBUG
#define LOG_QUANTUM 6
#else
#define LOG_QUANTUM 10
#endif
#define QUANTUM (1<<LOG_QUANTUM)
#define QUANTUM_MASK (QUANTUM-1)

class CGameItemEntry : public CGameItem
{
public:
	CGameItemEntry()
	{
		PtrRefCount = 0;
	}
	~CGameItemEntry()
	{

	}

	uint32 AllocatorNext;
	uint32 VectorIdx;
	sint PtrRefCount;

};

inline CGameItemVector::CGameItemVector()
{
}

inline CGameItemVector::~CGameItemVector()
{
	for (uint32 i = 0; i < m_Data.size(); ++i)
		delete[] m_Data[i];
}

inline uint32 CGameItemVector::size() const
{
	return (uint32)m_Data.size() << LOG_QUANTUM;
}

inline CGameItemEntry &CGameItemVector::operator[](uint32 idx)
{
	return m_Data[idx >> LOG_QUANTUM][idx & QUANTUM_MASK];
}

inline void CGameItemVector::extend()
{
	uint32 baseIdx = size();
	CGameItemEntry *items = new CGameItemEntry[QUANTUM];
	nlassert(items);
	m_Data.push_back(items);
	for (uint32 i = 0; i < QUANTUM; ++i)
	{
		items[i].AllocatorNext = baseIdx + i + 1;
		items[i].VectorIdx = baseIdx + i;
	}
	egs_giinfo("Increased item vector size to %u items (%u bytes)", size(), size() * sizeof(CGameItem));
}

inline uint32 CGameItemVector::getUniqueIndex(const CGameItem& item)
{
	return static_cast<const CGameItemEntry &>(item).VectorIdx;
}

#undef LOG_QUANTUM
#undef QUANTUM
#undef QUANTUM_MASK

//------------------------------------------------------------------------
// ptr class added by sadge
// inline implementations

// link ptr to game item
inline void CGameItemPtr::incRef()
{
	if (!m_Idx)
		return;

	++CGameItem::_Items[m_Idx].PtrRefCount;
}

// unlink ptr from item
inline void CGameItemPtr::decRef()
{
	// cannot unlink a pointer pointing on nothing
	if (!m_Idx)
		return;

	CGameItemEntry *entry = &CGameItem::_Items[m_Idx];
	sint count = --CGameItem::_Items[m_Idx].PtrRefCount;
	if (!count)
	{
		CGameItem::deleteItem(static_cast<CGameItem *>(entry));
	}
	nlassert(count >= 0);
}

// ctor	- default
inline CGameItemPtr::CGameItemPtr() : m_Idx(0)
{
	
}

// ctor	- copy
inline CGameItemPtr::CGameItemPtr(const CGameItemPtr &other) : m_Idx(other.m_Idx)
{
	incRef();
}

// ctor	- initialise from a CGameItem*
inline CGameItemPtr::CGameItemPtr(const CGameItem *item) : m_Idx(item ? static_cast<const CGameItemEntry *>(item)->VectorIdx : 0)
{
	incRef();
}

// dtor
inline CGameItemPtr::~CGameItemPtr()
{
	decRef();
}

// equivalent to: new CGameItem
inline CGameItem *CGameItemPtr::newItem(bool destroyable,bool dropable)
{
	CGameItem *item = CGameItem::newItem();
	item->ctor();
	item->_Destroyable=destroyable;
	item->_Dropable = dropable;
	*this=item;
	return item;
}

// equivalent to: if (this==NULL) {return new CGameItem;} else {return this;}
inline CGameItem *CGameItemPtr::newItemIfNull(bool destroyable,bool dropable)
{
	if (operator->()!=NULL)
		return operator->();

	return newItem(destroyable,dropable);
}

// equivalent to: new CGameItem(...)
//inline CGameItem *CGameItemPtr::newItem( const NLMISC::CEntityId& id, const NLMISC::CSheetId& sheetId, uint32 recommended, sint16 slotCount, bool destroyable, bool dropable )
inline CGameItem *CGameItemPtr::newItem( const NLMISC::CSheetId& sheetId, uint32 recommended, bool destroyable, bool dropable )
{
	CGameItem *item=CGameItem::newItem();
//	item->ctor(id,sheetId,recommended,slotCount,destroyable,dropable);
	item->ctor(sheetId, recommended, destroyable, dropable);
	*this=item;
	return item;
}

// = operator - for copying another CGameItemPtr
inline const CGameItemPtr &CGameItemPtr::operator=(const CGameItemPtr &other)
{
	decRef();

	m_Idx = other.m_Idx;

	incRef();

	return *this;
}

// = operator - for assignment of CGameItemPtr directly to a CGameItem
inline const CGameItemPtr &CGameItemPtr::operator=(const CGameItem *item)
{
	decRef();

	if (!item)
	{
		m_Idx = 0;
	}
	else
	{
		m_Idx = static_cast<const CGameItemEntry *>(item)->VectorIdx;
		nlassert(CGameItem::getItem(m_Idx) == item);

		incRef();
	}

	return *this;
}

// * operator - returning the item referenced by this pointer
inline CGameItem *CGameItemPtr::operator*()	const
{
	CGameItemEntry *entry = static_cast<CGameItemEntry *>(CGameItem::getItem(m_Idx));
	BOMB_IF(!entry->VectorIdx, "Attempting to access an item an item that is not allocated or has been freed", return 0);
	return static_cast<CGameItem *>(entry);
}

// -> operator - returning the item referenced by this pointer
inline CGameItem *CGameItemPtr::operator->() const
{
	// this is crash-safe even when m_Idx is wrong, it'll return item 0
	return CGameItem::getItem(m_Idx);
}

// () operator - returning the item referenced by this pointer
inline CGameItem *CGameItemPtr::operator()() const
{
	CGameItemEntry *entry = static_cast<CGameItemEntry *>(CGameItem::getItem(m_Idx));
	BOMB_IF(!entry->VectorIdx, "Attempting to access an item an item that is not allocated or has been freed", return 0);
	return static_cast<CGameItem *>(entry);
}

// == operator - compare 2 CGameItemPtrs
inline bool CGameItemPtr::operator==(const CGameItemPtr &other) const
{
	return (m_Idx == other.m_Idx);
}

// == operator - compare CGameItemPtrs to CGameItem*
inline bool CGameItemPtr::operator==(const CGameItem *item) const
{
	if (!item)
		return m_Idx == 0;

	return CGameItem::getItem(m_Idx) == item;
}

// != operator - compare 2 CGameItemPtrs
inline bool CGameItemPtr::operator!=(const CGameItemPtr &other) const
{
	return !(*this == other);
}

// != operator - compare CGameItemPtrs to CGameItem*
inline bool CGameItemPtr::operator!=(const CGameItem *item) const
{
	return !(*this == item);
}

inline bool CGameItemPtr::operator < (const CGameItemPtr &other) const
{
	return m_Idx < other.m_Idx;
}

#endif // GAME_ITEM_H

/* End of game_item.h */



