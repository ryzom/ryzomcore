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



#ifndef RY_EGS_STATIC_GAME_SHEET_H
#define RY_EGS_STATIC_GAME_SHEET_H

// game share
#include "game_share/brick_types.h"
#include "game_share/slot_equipment.h"
#include "game_share/characteristics.h"
#include "game_share/scores.h"
#include "game_share/skills.h"
#include "game_share/people.h"
#include "game_share/item_family.h"

#include "game_share/properties.h"
#include "game_share/ecosystem.h"
#include "game_share/rm_family.h"
#include "game_share/mp_category.h"
#include "server_share/mirror_equipment.h"
#include "game_share/damage_types.h"
#include "game_share/constants.h"
#include "game_share/memorization_set_types.h"
#include "server_share/respawn_point_type.h"
#include "game_share/continent.h"
#include "server_share/npc_description_messages.h"

// georges
#include "nel/georges/load_form.h"

#include "egs_sheets/egs_static_game_item.h"
#include "egs_sheets/egs_static_harvestable.h"
#include "../entity_structure/resists.h"
#include "../entity_structure/statistic.h"

#include <map>
#include <string>

/**
 * CStaticGameBrick
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticGameBrick
{
public :
	/// Id of family of brick
	uint16					FamilyId;//
	/// index in family
	uint16					IndexInFamily;//
	/// type of the brick (combat, magic...)
	BRICK_TYPE::EBrickType	Type;//
	/// mandatory families
	std::vector<uint16>		MandatoryFamilies;//
	/// optional families
	std::vector<uint16>		OptionalFamilies;//
	/// the race to which belongs this brick (Unknown = belong to all races)
	EGSPD::CPeople::TPeople	Race;//
	/// the brick level
	uint16					Level;//
	/// The brick price
	std::string				Price;
	/// Skill used
	SKILLS::ESkills			Skill;

	/// the SheetId
	NLMISC::CSheetId		SheetId;//

	/// constructor
	CStaticGameBrick()
	{
		FamilyId = 0;
		IndexInFamily = 0;
		Type = BRICK_TYPE::UNKNOWN;
		Level = 0;
		Price = std::string("0");
		_InMap = false;
	}

	/// destructor
	virtual ~CStaticGameBrick()
	{
/*		if (_InMap)
		{
//			nlinfo("REMOVE brick %s, familyId = %u, index = %u",SheetId.toString().c_str(), FamilyId, IndexInFamily);
			_Bricks.erase( std::make_pair(FamilyId, IndexInFamily) );
		}
*/	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 8 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// static returns the CStaticGameBrick objectr of specified family and pos in family
	static CStaticGameBrick * getBrick(uint16 familyId, uint16 indexInFamily)
	{
		std::map< std::pair<uint16, uint16>, CStaticGameBrick *>::const_iterator it = _Bricks.find( std::make_pair( familyId,indexInFamily) );
		if ( it != _Bricks.end() )
		{
			CStaticGameBrick *brick = (*it).second;
			return brick;
		}
		else
		{
			return NULL;
		}
	}

	/// serial
	void serial( NLMISC::IStream &f ) throw(NLMISC::EStream);

	/// called when the sheet is removed
	void removed() {}

private:
	/// map the pair (family, indexInFamily) to the CStaticGameBrick object (pointer)
	static std::map< std::pair<uint16, uint16>, CStaticGameBrick *> _Bricks;

	///
	bool	_InMap;
};


/**
 * CStaticXpStagesTable
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticXpStagesTable
{
public :
	struct SXpStage
	{
		uint16 SkillLevel;

		uint32	XpForPointSkill;
		float	SpPointMultiplier;
		
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};

	struct SStageTable
	{
		std::vector< SXpStage > StageTable;
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	};


	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 2; }

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);

	// return a reference on Xp stage corresponding to level and stage table
	const SXpStage* getXpStage( uint32 level, uint16 stage ) const;

	/// destructor
	virtual ~CStaticXpStagesTable() {}

	/// called when the sheet is removed
	void removed() {}

	/// called to copy from another sheet (operator= + care ptrs)
	void reloadSheet(const CStaticXpStagesTable &o);
	
	std::vector< SStageTable > XpStagesTables;
};

/**
 * CStaticStagesTypeSkillTable
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticStagesTypeSkillTable
{
public :
	struct SSkillStageTypeAndCoeff
	{
		uint16	StageType;
		float	Coeff;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) { f.serial( StageType); f.serial( Coeff ); }
	};

	/// read sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 2 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) { f.serialCont( SkillToStageType ); }

	/// destructor
	virtual ~CStaticStagesTypeSkillTable() {}

	/// called when the sheet is removed
	void removed() {}

	std::map< std::string, SSkillStageTypeAndCoeff > SkillToStageType;
};

/**
 * CStaticPacts
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticPacts
{
public:
	struct SPact
	{
		uint16	LoseHitPointsLevel;
		uint16	LoseStaminaLevel;
		uint16	LoseSapLevel;
		uint16	LoseSkillsLevel;
		NLMISC::TGameCycle	Duration;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( LoseHitPointsLevel); 
			f.serial( LoseStaminaLevel ); 
			f.serial( LoseSapLevel ); 
			f.serial( LoseSkillsLevel );
			f.serial( Duration );
		}
	};

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 2; }

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) { f.serialCont( PactLose ); }

	/// destructor
	virtual ~CStaticPacts() {}

	/// called when the sheet is removed
	void removed() {}

	std::vector< SPact > PactLose;
};

//////////////////////////////////////////////////////////////////////////////
// IStaticCreatures                                                         //
//////////////////////////////////////////////////////////////////////////////

/// This sheet interface breaks coding standards as methods have a capital
/// initial. Though old sheets had public members, so it's better now. And
/// you're free to refactor all EGS code to use lowercase initials
class IStaticCreatures
: public NLMISC::CRefCount
{
public:
	virtual ~IStaticCreatures() { }
	
public:
	///@name Properties
	//@{
	virtual EGSPD::CPeople::TPeople			getRace() const = 0;
	virtual uint8							getGender() const = 0;
	virtual uint8							getSize() const = 0;
	virtual uint16							getLevel() const = 0;
	virtual uint16							getAttackLevel() const = 0;
	virtual uint16							getDefenseLevel() const = 0;
	virtual uint16							getXPLevel() const = 0;
	virtual float							getXPGainOnCreature() const = 0;
	virtual uint16							getTauntLevel() const = 0;
	virtual uint8							getNbPlayers() const = 0;
	virtual sint32							getCharacteristics(size_t index) const = 0;
	virtual sint32							getScores(size_t index) const = 0;
	virtual float							getRegen(size_t index) const = 0;
	virtual SMirrorEquipment const&			getItems(size_t index) const = 0;
	virtual std::string						getLootTable(uint i) const = 0;
	virtual uint8							getMeleeReachValue() const = 0;
	
	virtual bool							getDodgeAsDefense() const = 0;
	
	virtual CCreatureResists const&			getResists() const = 0;
	
	virtual NLMISC::CSheetId const&			getActionOnDeath() const = 0;
	
	virtual float							getWalkSpeed() const = 0;
	virtual float							getRunSpeed() const = 0;
	virtual CProperties const&				getProperties() const = 0;
	virtual ECOSYSTEM::EECosystem			getEcosystem() const = 0;
	virtual std::string						getBagInventorySheet() const = 0;
	
	/// amount of damage done by this creature for each hit
	virtual uint32							getCreatureDamagePerHit() const = 0;
	/// amount of damage done by this creature for each hit (yoyo: used for reloading of success chance table)
	virtual uint32							getCreatureDamagePerHitWithoutAverageDodge() const = 0;
	/// creature attack latency (in ticks)
	virtual NLMISC::TGameCycle				getAttackLatency() const = 0;
	
	//protections of the creatures
	virtual std::vector<SProtection> const&	getProtections() const = 0;
	
	/// Index of the faction of this creature
	virtual uint32							getFaction() const = 0;
	/// Tells whether use the _FameByKill defined in the sheet (valid) or the global value (not valid)
	virtual bool							getFameByKillValid() const = 0;
	/// Quantity of fame gained for each kill of that bot (for faction defined by getFaction, usually negative, use global value if getFameByKillValid is false)
	virtual sint32							getFameByKill() const = 0;
	// Entity Collision Radius
	virtual float							getColRadius() const = 0;
	// Entity collision box	
	virtual float							getColLength() const = 0;
	virtual float							getColWidth() const = 0;

	// damage shield
	virtual uint16							getDamageShieldDamage() const = 0;
	virtual uint16							getDamageShieldHpDrain() const = 0;
	//@}
	
	///@name Methods
	//@{
	/// get the raw materials (mp) vector
	virtual std::vector<CStaticCreatureRawMaterial> const& getMps() const = 0;
	
	/// get the items for missions
	virtual std::vector<NLMISC::CSheetId> const& getItemsForMissions() const = 0;
	//@}
};
typedef NLMISC::CSmartPtr<IStaticCreatures> IStaticCreaturesPtr;
typedef NLMISC::CSmartPtr<IStaticCreatures const> IStaticCreaturesCPtr;

//////////////////////////////////////////////////////////////////////////////
// CStaticCreaturesProxy                                                    //
//////////////////////////////////////////////////////////////////////////////

class CStaticCreaturesProxy
: public IStaticCreatures
{
public:
	CStaticCreaturesProxy(IStaticCreaturesCPtr sheet) : _Sheet(sheet) { }
	
public:
	///@name IStaticCreatures interface
	//@{
	virtual EGSPD::CPeople::TPeople			getRace() const { return _Sheet->getRace(); }
	virtual uint8							getGender() const { return _Sheet->getGender(); }
	virtual uint8							getSize() const { return _Sheet->getSize(); }
	virtual uint16							getLevel() const { return _Sheet->getLevel(); }
	virtual uint16							getAttackLevel() const { return _Sheet->getAttackLevel(); }
	virtual uint16							getDefenseLevel() const { return _Sheet->getDefenseLevel(); }
	virtual uint16							getXPLevel() const { return _Sheet->getXPLevel(); }
	virtual float							getXPGainOnCreature() const { return _Sheet->getXPGainOnCreature(); }
	virtual uint16							getTauntLevel() const { return _Sheet->getTauntLevel(); }
	virtual uint8							getNbPlayers() const { return _Sheet->getNbPlayers(); }
	virtual sint32							getCharacteristics(size_t index) const { return _Sheet->getCharacteristics(index); }
	virtual sint32							getScores(size_t index) const { return _Sheet->getScores(index); }
	virtual float							getRegen(size_t index) const { return _Sheet->getRegen(index); }
	virtual SMirrorEquipment const&			getItems(size_t index) const { return _Sheet->getItems(index); }
	virtual std::string						getLootTable(uint i) const { return _Sheet->getLootTable(i); }
	virtual uint8							getMeleeReachValue() const { return _Sheet->getMeleeReachValue(); }
	
	virtual bool							getDodgeAsDefense() const { return _Sheet->getDodgeAsDefense(); }
	
	virtual CCreatureResists const&			getResists() const { return _Sheet->getResists(); }
	
	virtual NLMISC::CSheetId const&			getActionOnDeath() const { return _Sheet->getActionOnDeath(); }
	
	virtual float							getWalkSpeed() const { return _Sheet->getWalkSpeed(); }
	virtual float							getRunSpeed() const { return _Sheet->getRunSpeed(); }
	virtual CProperties const&				getProperties() const { return _Sheet->getProperties(); }
	virtual ECOSYSTEM::EECosystem			getEcosystem() const { return _Sheet->getEcosystem(); }
	virtual std::string						getBagInventorySheet() const { return _Sheet->getBagInventorySheet(); }
	
	virtual uint32							getCreatureDamagePerHit() const { return _Sheet->getCreatureDamagePerHit(); }
	virtual uint32							getCreatureDamagePerHitWithoutAverageDodge() const { return _Sheet->getCreatureDamagePerHitWithoutAverageDodge(); }
	virtual NLMISC::TGameCycle				getAttackLatency() const { return _Sheet->getAttackLatency(); }
	
	virtual std::vector<SProtection> const&	getProtections() const { return _Sheet->getProtections(); }
	
	virtual uint32							getFaction() const { return _Sheet->getFaction(); }
	virtual bool							getFameByKillValid() const { return _Sheet->getFameByKillValid(); }
	virtual sint32							getFameByKill() const { return _Sheet->getFameByKill(); }
	virtual float							getColRadius() const { return _Sheet->getColRadius(); }
	virtual float							getColLength() const { return _Sheet->getColLength(); }
	virtual float							getColWidth() const { return _Sheet->getColWidth(); }

	virtual uint16							getDamageShieldDamage() const { return _Sheet->getDamageShieldDamage(); }
	virtual uint16							getDamageShieldHpDrain() const { return _Sheet->getDamageShieldHpDrain(); }
	
	virtual std::vector<CStaticCreatureRawMaterial> const&	getMps() const { return _Sheet->getMps(); }
	
	virtual std::vector<NLMISC::CSheetId> const&	getItemsForMissions() const { return _Sheet->getItemsForMissions(); }
	//@}
	
protected:
	IStaticCreaturesCPtr	_Sheet;
};
typedef NLMISC::CSmartPtr<CStaticCreaturesProxy> CStaticCreaturesProxyPtr;
typedef NLMISC::CSmartPtr<CStaticCreaturesProxy const> CStaticCreaturesProxyCPtr;

//////////////////////////////////////////////////////////////////////////////
// CStaticCreatures                                                         //
//////////////////////////////////////////////////////////////////////////////

enum TAttributeType
{
	//BASICS
	at_race,
	at_gender,
	at_size,
	at_level,
	at_player_skill_level,
	at_nb_players,
	at_player_hp_level,
	at_nb_hit_to_kill_player,
	at_ecosystem,
	at_type,
	at_fame,
	at_fame_by_kill,
	at_fame_for_guard_attack,
	at_life,
	at_liferegen,
	at_attack_speed,
	at_attack_level,
	at_defense_level,
	at_xp_level,
	at_taunt_level,
	at_melee_reach_value,
	at_xp_gain_on_creature,
	at_local_code,
	at_dodge_as_defense,
	at_walk_speed,
	at_run_speed,
	at_selectable,
	at_attackable,
	at_lhstate,
	
	//PROTECT
	at_protect_piercing,
	at_protect_slashing,
	at_protect_blunt,
	at_protect_rot,
	at_protect_acid,
	at_protect_cold,
	at_protect_fire,
	at_protect_poison,
	at_protect_electricity,
	at_protect_shock,
	
	//RESISTS
	at_resists_fear,
	at_resists_sleep,
	at_resists_stun,
	at_resists_root,
	at_resists_snare,
	at_resists_slow,
	at_resists_madness,
	at_resists_blind,
	at_resists_acid,
	at_resists_cold,
	at_resists_electricity,
	at_resists_fire,
	at_resists_poison,
	at_resists_rot,
	at_resists_shockwave,

	
	at_unknown
};
/**
 * CStaticCreatures
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 * \author Jerome Vuarand
 * \date 2005
 */
class CStaticCreatures
: public CStaticHarvestable
, public IStaticCreatures
{
public:
	/// constructor
	CStaticCreatures()
	: _Protections(DMGTYPE::NBTYPES)
	{
	}
	/// destructor
	virtual ~CStaticCreatures() { }
	
public:
	///@name IStaticCreatures implementation
	//@{
	virtual EGSPD::CPeople::TPeople			getRace() const { return _Race; }
	virtual void							setRace(EGSPD::CPeople::TPeople race) {_Race = race;};
	virtual uint8							getGender() const { return _Gender; }
	virtual uint8							getSize() const { return _Size; }
	virtual uint16							getLevel() const { return _Level; }
	virtual uint16							getAttackLevel() const { return _AttackLevel; }
	virtual uint16							getDefenseLevel() const { return _DefenseLevel; }
	virtual uint16							getXPLevel() const { return _XPLevel; }
	virtual float							getXPGainOnCreature() const { return _XPGainOnCreature; }
	virtual uint16							getTauntLevel() const { return _TauntLevel; }
	virtual uint8							getNbPlayers() const { return _NbPlayers; }
	virtual sint32							getCharacteristics(size_t index) const { return _Characteristics[index]; }
	virtual sint32							getScores(size_t index) const { return _Scores[index]; }
	virtual float							getRegen(size_t index) const { return _Regen[index]; }
	virtual SMirrorEquipment const&			getItems(size_t index) const { return _Items[index]; }
	virtual std::string						getLootTable(uint i) const { if (i<_LootTables.size()) return _LootTables[i]; else return ""; }
	virtual uint							getLootTableCount() const { return (uint)_LootTables.size(); }
	virtual uint8							getMeleeReachValue() const { return _MeleeReachValue; }
	
	virtual bool							getDodgeAsDefense() const { return _DodgeAsDefense; }
	
	virtual CCreatureResists const&			getResists() const { return _Resists; }
	
	virtual NLMISC::CSheetId const&			getActionOnDeath() const { return _ActionOnDeath; }
	
	virtual float							getWalkSpeed() const { return _WalkSpeed; }
	virtual float							getRunSpeed() const { return _RunSpeed; }
	virtual CProperties const&				getProperties() const { return _Properties; }
	virtual ECOSYSTEM::EECosystem			getEcosystem() const { return _Ecosystem; }
	virtual std::string						getBagInventorySheet() const { return _BagInventorySheet; }
	
	virtual uint32							getCreatureDamagePerHit() const { return _CreatureDamagePerHit; }
	virtual uint32							getCreatureDamagePerHitWithoutAverageDodge() const { return _CreatureDamagePerHitWithoutAverageDodge; }
	virtual NLMISC::TGameCycle				getAttackLatency() const { return _AttackLatency; }
	
	virtual std::vector<SProtection> const&	getProtections() const { return _Protections; }
	
	virtual uint32							getFaction() const { return _Faction; }
	virtual bool							getFameByKillValid() const { return _FameByKillValid; }
	virtual sint32							getFameByKill() const { return _FameByKill; }
	virtual float							getColRadius() const { return _ColRadius; }
	virtual float							getScale() const { return _Scale; }
	virtual float							getColLength() const { return _ColLength; }
	virtual float							getColWidth() const { return _ColWidth; }

	virtual uint16							getDamageShieldDamage() const { return _DamageShieldDamage; }
	virtual uint16							getDamageShieldHpDrain() const { return _DamageShieldHpDrain; }

	virtual std::vector<CStaticCreatureRawMaterial> const& getMps() const { return CStaticHarvestable::getMps(); }
	
	virtual std::vector<NLMISC::CSheetId> const& getItemsForMissions() const { return CStaticHarvestable::getItemsForMissions(); }
	//@}
	
private:
	EGSPD::CPeople::TPeople		_Race;
	uint8		_Gender;	
	uint8		_Size;
	uint16		_Level;
	uint32 _PlayerHpLevel;
	float _NbHitToKillPlayer;
	uint16		_AttackLevel;
	uint16		_DefenseLevel;
	uint16		_XPLevel;
	float		_XPGainOnCreature;
	uint16		_TauntLevel;
	uint8		_NbPlayers;
	sint32		_Characteristics[ CHARACTERISTICS::NUM_CHARACTERISTICS ];
	sint32		_Scores[ SCORES::NUM_SCORES ];
	float		_Regen[ SCORES::NUM_SCORES ];
	sint32		_Skills[ SKILLS::NUM_SKILLS ];
	SMirrorEquipment _Items[ SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT ];
	std::vector<std::string> _LootTables;
	uint8		_MeleeReachValue;
	
	bool		_DodgeAsDefense;
	
	CCreatureResists	_Resists;
	
	NLMISC::CSheetId	_ActionOnDeath;
	
	float		_WalkSpeed;
	float		_RunSpeed;
	CProperties	_Properties;
	ECOSYSTEM::EECosystem	_Ecosystem;
	std::string	_BagInventorySheet;
	
	/// amount of damage done by this creature for each hit
	uint32				_CreatureDamagePerHit;
	/// amount of damage done by this creature for each hit (yoyo: used for reloading of success chance table)
	uint32				_CreatureDamagePerHitWithoutAverageDodge;
	/// creature attack latency (in ticks)
	NLMISC::TGameCycle	_AttackLatency;
	
	//protections of the creatures
	std::vector<SProtection> _Protections;
	
	/// Index of the faction of this creature
	uint32		_Faction;
	/// Tells whether use the _FameByKill defined in the sheet (valid) or the global value (not valid)
	bool		_FameByKillValid;
	/// Quantity of fame gained for each kill of that bot (for faction defined in _Faction, usually negative, use global value if _FameByKillValid is false)
	sint32		_FameByKill;
	// Entity Collision Radius
	float		_ColRadius;
	// Entity Scale
	float		_Scale;
	// Entity collision box	
	float		_ColLength;
	float		_ColWidth;
	
	// damage shield
	uint16		_DamageShieldDamage;
	uint16		_DamageShieldHpDrain;
	
public:
	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );
	
	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion ();
	
	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	
	/// called when the sheet is removed
	void removed() { }
	
	/// called to copy from another sheet (operator= + care ptrs)
	void reloadSheet(const CStaticCreatures &o);
	
	/// recompute CreatureDamagePerHit according to success chance table and CreatureDamagePerHitWithoutAverageDodge
	void compileCreatureDamagePerHit();

	//method used specifically to modify protect table
	bool applyProtectModification(uint index, const std::string &attr, const std::string &newValue);
	//apply attributes modifications defined in a UserModel
	
	// return *true* if there are errors
	bool applyUserModel(CCustomElementId modelId, const std::vector<std::string> &scriptData);
};
typedef NLMISC::CSmartPtr<CStaticCreatures> CStaticCreaturesPtr;
typedef NLMISC::CSmartPtr<CStaticCreatures const> CStaticCreaturesCPtr;

//////////////////////////////////////////////////
// for pre-memorized sentences at create character
//////////////////////////////////////////////////
class CSentenceStatic
{
public:
	std::string						Name;
	std::vector<NLMISC::CSheetId>	BricksIds;
	
	/// Serialisation
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
	{
		f.serial( Name );
		f.serialCont( BricksIds );
	}
};

/**
 * CStaticCharacters: This form is used for create special setting character for tests
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticCharacters
{
public:
/*	struct SSheath
	{
		SMirrorEquipment Right;
		SMirrorEquipment Left;
		SMirrorEquipment Ammo0;
		SMirrorEquipment Ammo1;
		SMirrorEquipment Ammo2;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( Right );
			f.serial( Left );
			f.serial( Ammo0 );
			f.serial( Ammo1 );
			f.serial( Ammo2 );
		}
	};
*/
	EGSPD::CPeople::TPeople	Race;
//	uint8 Race;
	uint8 Gender;
	uint8 Size;
	uint16 Level;
//Deprecated
//	uint16 Role;
	std::string Surname;
	std::string Name;
	std::string Characteristics[ CHARACTERISTICS::NUM_CHARACTERISTICS ];
	std::string Scores[ SCORES::NUM_SCORES ];
	std::string Skills[ SKILLS::NUM_SKILLS ];
	SMirrorEquipment Items[ SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT ];
//	SSheath Sheaths[ NB_SHEATH ];
	std::vector< CSentenceStatic > MemorizedSentences;
	std::vector< NLMISC::CSheetId > KnownBricks;
	std::vector< NLMISC::CSheetId > Inventory;
	std::vector< NLMISC::CSheetId > PackAnimal;
	float WalkSpeed;
	float RunSpeed;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialEnum( Race );
		f.serial( Gender );
		f.serial( Size );
		f.serial( Level );
//Deprecated
//		f.serial( Role );
		f.serial( Surname );
		f.serial( Name );
		for( int c = 0; c < CHARACTERISTICS::NUM_CHARACTERISTICS; ++c )
		{
			f.serial( Characteristics[ c ] );
		}
		for( int sc = 0; sc < SCORES::NUM_SCORES; ++sc )
		{
			f.serial( Scores[ sc ] );
		}
		for( int s = 0; s < SKILLS::NUM_SKILLS; ++s )
		{
			f.serial( Skills[ s ] );
		}
		for( int e = 0; e < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++e )
		{
			f.serial( Items[ e ] );
		}
/*		for( int sh = 0; sh < NB_SHEATH; ++sh )
		{
			f.serial( Sheaths[ sh ] );
		}
*/		f.serialCont( MemorizedSentences );
		f.serialCont( KnownBricks );
		f.serialCont( Inventory );
		f.serialCont( PackAnimal );
		f.serial( WalkSpeed );
		f.serial( RunSpeed );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 10 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// destructor
	virtual ~CStaticCharacters() {}

	/// called when the sheet is removed
	void removed() {}
};



/**
 * CStaticLootSet
 *
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CStaticLootSet
{
public:
	struct SItemLoot
	{
		std::string Item;
		uint16		Level;
		uint16		Quantity;
		
		/// serialize
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( Item );
			f.serial( Level );
			f.serial( Quantity );
		}
	};

	std::vector< SItemLoot > ItemLoot;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialCont( ItemLoot );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1; }

	/// destructor
	virtual ~CStaticLootSet() {}

	/// called when the sheet is removed
	void removed() {}
};

/**
 * CStaticLootTable
 *
 * \author Alain Saffray, Stephane Coutelas
 * \author Nevrax France
 * \date 2002
 */
class CStaticLootTable
{
public:
	/// loot_set and proba
	std::map<NLMISC::CSheetId,uint16> LootSets;
	
	// used when a custom loot table is applied to the creature
	// proba and lootSets (no sheetId, because each loot set is script defined)
	// we *must* use a multimap because uint16 is the proba and if we have 3 items with the same proba and use a std::map, only one item will be available.
	std::multimap<uint16, CStaticLootSet> CustomLootSets;

	/// factor used to compute money amount from creature level
	float	MoneyLvlFactor;

	/// money static amount
	uint32	MoneyBase;

	/// probability of money drop
	float	MoneyDropProbability;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialCont( LootSets );
		f.serial( MoneyLvlFactor );
		f.serial( MoneyBase );
		f.serial( MoneyDropProbability );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

#ifndef NO_EGS_VARS
	/// select a loot set
	NLMISC::CSheetId selectRandomLootSet() const;

	const CStaticLootSet *selectRandomCustomLootSet() const;
#endif

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1; }

	/// destructor
	virtual ~CStaticLootTable() {}

	/// called when the sheet is removed
	void removed() {}
};


/**
 * CStaticLootTable
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
/*
class CStaticLootTable
{
public:
	struct SItemLoot
	{
		std::string Item;
		uint8		Probability;
		uint16		MinLevel;
		uint16		MaxLevel;
		uint8		RangeLevel;
		uint16		QuantityMin;
		uint16		QuantityMax;

		/// serialize
		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( Item );
			f.serial( Probability );
			f.serial( MinLevel );
			f.serial( MaxLevel );
			f.serial( RangeLevel );
			f.serial( QuantityMin );
			f.serial( QuantityMax );
		}
	};

	std::vector< SItemLoot > ItemLoot;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialCont( ItemLoot );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1; }

	/// destructor
	virtual ~CStaticLootTable() {}

	/// called when the sheet is removed
	void removed() {}
};
*/

/**
 * CStaticRaceSate : Stats by race (people) for character create
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticRaceStats
{
public:
	struct SDefaultEquipment
	{
		std::string DefaultFace;
		std::string DefaultChest;
		std::string DefaultArms;
		std::string DefaultLegs;
		std::string DefaultHands;
		std::string DefaultFeet;
		std::string DefaultHair;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( DefaultFace );
			f.serial( DefaultChest );
			f.serial( DefaultArms );
			f.serial( DefaultLegs );
			f.serial( DefaultHands );
			f.serial( DefaultFeet );
			f.serial( DefaultHair );
		}
	};

	EGSPD::CPeople::TPeople	Race;
//	uint8 Race; // Character race (people)
	sint32 Characteristics[ CHARACTERISTICS::NUM_CHARACTERISTICS ];	// Character characteristics
	std::string Scores[ SCORES::NUM_SCORES ];						// Character scores
	float ProgressionScore1;
	float ProgressionScore2;
	float ProgressionScore3;
	float ProgressionScore4;
	float ProgressionRegen1;
	float ProgressionRegen2;
	float ProgressionRegen3;
	float ProgressionRegen4;
	SDefaultEquipment MaleDefaultEquipment;
	SDefaultEquipment FemaleDefaultEquipment;

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serialEnum( Race );
		for( int c = 0; c < CHARACTERISTICS::NUM_CHARACTERISTICS; ++c )
		{
			f.serial( Characteristics[ c ] );
		}
		for( int sc = 0; sc < SCORES::NUM_SCORES; ++sc )
		{
			f.serial( Scores[ sc ] );
		}

		f.serial( ProgressionScore1 );
		f.serial( ProgressionScore2 );
		f.serial( ProgressionScore3 );
		f.serial( ProgressionScore4 );

		f.serial( ProgressionRegen1 );
		f.serial( ProgressionRegen2 );
		f.serial( ProgressionRegen3 );
		f.serial( ProgressionRegen4 );

		f.serial( MaleDefaultEquipment );
		f.serial( FemaleDefaultEquipment );
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 5; }

	/// destructor
	virtual ~CStaticRaceStats() {}

	/// called when the sheet is removed
	void removed() {}
};

/**
 * CStaticRole : Starting equipment and action by role at character create
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticRole
{
public:
	uint8 Role;
//	uint8 Race;
	EGSPD::CPeople::TPeople	Race;
	
	struct TMemorizedSentence
	{
		NLMISC::CSheetId sentence;
//		MEM_SET_TYPES::TMemorizationSetType memory;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
		{
			f.serial( sentence );
//			f.serialEnum( memory );
		}
	};
		
	// intial action an by point affecte din role (0 no choosed, 1 - 3)
	std::vector< TMemorizedSentence > MemorizedSentences1;
	SMirrorEquipment Items1[ SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT ];

	std::vector< TMemorizedSentence > MemorizedSentences2;
	SMirrorEquipment Items2[ SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT ];

	std::vector< TMemorizedSentence > MemorizedSentences3;
	SMirrorEquipment Items3[ SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT ];

	
	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) 
	{
		f.serial( Role );
		f.serialEnum( Race );

		f.serialCont( MemorizedSentences1 );
		for( int e = 0; e < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++e )
		{
			f.serial( Items1[ e ] );
		}

		f.serialCont( MemorizedSentences2 );
		for( int e = 0; e < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++e )
		{
			f.serial( Items2[ e ] );
		}

		f.serialCont( MemorizedSentences3 );
		for( int e = 0; e < SLOT_EQUIPMENT::NB_SLOT_EQUIPMENT; ++e )
		{
			f.serial( Items3[ e ] );
		}
	}

	/// read the sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// load known sentence and equipment
	void readGeorgesSentenceAndEquipment( NLGEORGES::UFormElm& root, const NLMISC::CSheetId &sheetId, const std::string& SentenceString, std::vector< TMemorizedSentence >& MemorizedSentences, const std::string& EquipmentString, SMirrorEquipment* Items );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 4 /*+ ( SKILLS::NUM_SKILLS << 16 )*/; }

	/// destructor
	virtual ~CStaticRole() { }

	/// called when the sheet is removed
	void removed() {}
};


/**
 * CStaticSkillsTree
 *
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
class CStaticSkillsTree
{
public :
	struct SSkillData
	{
		SKILLS::ESkills Skill;
		std::string	SkillCode;
		uint16		MaxSkillValue;
		uint16		StageType;
		SKILLS::ESkills	ParentSkill;
		std::vector<SKILLS::ESkills> ChildSkills;

		void serial(class NLMISC::IStream &f) throw(NLMISC::EStream)
		{
			f.serialEnum( Skill );
			f.serial( SkillCode );
			f.serial( MaxSkillValue );
			f.serial( StageType );
			f.serialEnum( ParentSkill );
			
			if( f.isReading() )
			{
				uint16 size;
				f.serial( size );
				ChildSkills.resize( size );
				for( uint i = 0; i < size; ++i )
				{
					f.serialEnum( ChildSkills[ i ] );
				}
			}
			else
			{
				uint16 size = (uint16)ChildSkills.size();
				f.serial( size );
				for( std::vector<SKILLS::ESkills>::iterator it = ChildSkills.begin(); it != ChildSkills.end(); ++it )
				{
					f.serialEnum( (*it) );
				}
			}
		}
	};

	/// read sheet
	virtual void readGeorges( const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId );

	// return the version of this class, increments this value when the content of this class changed
	static uint getVersion () { return 1 + ( SKILLS::NUM_SKILLS << 16 ); }

	/// serialize
	void serial(class NLMISC::IStream &f) throw(NLMISC::EStream) { f.serialCont( SkillsTree ); }

	/// destructor
	virtual ~CStaticSkillsTree() {}

	/// called when the sheet is removed
	void removed() {}

	/** 
	 * get total number of possible skill points in the tree "under" a given skill (including this skill)
	 */
	uint32 getTreeSkillPointsUnderSkill(SKILLS::ESkills skill) const;

	/** 
	 * For a player, get total number of earned skill points in the tree "under" a given skill (including this skill) 
	 */
	uint32 getPlayerSkillPointsUnderSkill(const CSkills *skills, SKILLS::ESkills skill) const;

	std::vector< SSkillData > SkillsTree;
};

#endif // STATIC_GAME_SHEET_H

/* End of static_game_sheet.h */
