

#ifndef EGS_PLACE_HOLDER_H
#define EGS_PLACE_HOLDER_H

#include "nel/misc/smart_ptr.h"
#include "nel/misc/enum_bitset.h"
#include "nel/misc/sheet_id.h"
#include "nel/net/cvar_log_filter.h"
#include "game_share/skills.h"
#include "game_share/characteristics.h"
#include "game_share/type_skill_mod.h"
#include "game_share/item_family.h"
#include "game_share/item_type.h"
#include "game_share/inventories.h"
#include "entities_game_service/game_item_manager/weapon_craft_parameters.h"
#include "entities_game_service/weapon_damage_table.h"

NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_giinfo,  GameItemLogEnabled,             true)

class CGameItemPtr;

class CInventoryBase : public NLMISC::CRefCount
{
public:
	enum	// for pseudo constantes
	{
		INSERT_IN_FIRST_FREE_SLOT = 0xFFFFFFFF,
		REMOVE_MAX_STACK_QUANTITY = 0xFFFFFFFF,
		INVALID_INVENTORY_SLOT = INSERT_IN_FIRST_FREE_SLOT,
	};

	enum TItemChange
	{
		itc_bulk			= 1<<0,
		itc_weight			= 1<<1,
		itc_enchant			= 1<<2,
		itc_hp				= 1<<3,
		itc_inserted		= 1<<4,
		itc_removed			= 1<<5,
		itc_lock_state		= 1<<6,
		itc_info_version	= 1<<7,
		itc_worned			= 1<<8,
	};

	enum TInventoryOpResult
	{
		ior_overbulk,
	};


	typedef NLMISC::CEnumBitset<TItemChange>	TItemChangeFlags;

	uint getSlotCount() {return 0;};

	virtual void onItemStackSizeChanged(uint32 slot, uint32 previousStackSize) {}
	virtual void onItemChanged(uint32 slot, CInventoryBase::TItemChangeFlags changeFlags) {}

	virtual CGameItemPtr removeItem(uint32 slot, uint32 quantity = REMOVE_MAX_STACK_QUANTITY, TInventoryOpResult * res = NULL);

	virtual TInventoryOpResult insertItem(CGameItemPtr &item, uint32 slot = INSERT_IN_FIRST_FREE_SLOT, bool autoStack = false) { return ior_overbulk;}

};

typedef NLMISC::CSmartPtr<class CInventoryBase>	CInventoryPtr;

namespace ITEM_WORN_STATE
{
	enum TItemWornState
	{
		Unspoiled = 0,
		WornState1,
		WornState2,
		WornState3,
		WornState4,
		Worned,		
	};
}

class CPlayerManager
{
};

struct CMP
{
	uint16						StatEnergy; // 0..100

};

class CStaticBrick
{
};

class CStaticItem
{
public:
	float getBaseWeight() const;

	std::string					Name;
	uint32						Weight;
	uint32						Bulk;
	
	NLMISC::CSheetId			CraftPlan;
	SKILLS::ESkills				RequiredSkill;
	SKILLS::ESkills				RequiredSkill2;
	CHARACTERISTICS::TCharacteristics	RequiredCharac;
	std::vector<CTypeSkillMod>	TypeSkillMods;
	float						RequiredSkillQualityFactor;
	sint16						RequiredSkillQualityOffset;
	uint16						MinRequiredSkillLevel;
	float						RequiredSkillQualityFactor2;
	sint16						RequiredSkillQualityOffset2;
	uint16						MinRequiredSkillLevel2;
	float						RequiredCharacQualityFactor;
	sint16						RequiredCharacQualityOffset;
	uint16						MinRequiredCharacLevel;
	uint32						Stackable;
	ITEMFAMILY::EItemFamily		Family;
	CMP	*						Mp;
	ITEM_TYPE::TItemType		Type;

};

class CSheets
{
public:
	static CStaticItem *getForm(const NLMISC::CSheetId &sheet) {return NULL;};
	static const CStaticBrick* getSBrickForm( const NLMISC::CSheetId& sheetId );
};
//NL_DECLARE_CVAR_INFO_LOG_FUNCTION(egs_giinfo,  GameItemLogEnabled,             true)

extern NLMISC::CVariable<float>					WornState1;
extern NLMISC::CVariable<float>					WornState2;
extern NLMISC::CVariable<float>					WornState3;
extern NLMISC::CVariable<float>					WornState4;

class CGameItemManager
{
public:
};


extern CGameItemManager GameItemManager;

class CCharacter
{
public:
	inline const TDataSetRow& getEntityRowId() const 
	{ 
		static TDataSetRow foo;

		return foo;
	}

	const CInventoryPtr &getInventory(INVENTORIES::TInventory id) const;
};



#endif