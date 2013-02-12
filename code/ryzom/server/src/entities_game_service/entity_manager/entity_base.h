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



#ifndef ENTITY_BASE_H
#define ENTITY_BASE_H


/////////////
// INCLUDE //
/////////////
// game share
#include "server_share/action_flags.h"
#include "game_share/client_action_type.h"
#include "game_share/constants.h"
#include "server_share/creature_size.h"
#include "game_share/mode_and_behaviour.h"
#include "server_share/mirror_equipment.h"
#include "game_share/properties.h"
#include "game_share/effect_families.h"
#include "game_share/mount_people.h"
#include "server_share/r2_variables.h"
//
#include "server_share/r2_variables.h"
//
#include "entity_structure/entity_persistant_data.h"
#include "gameplay_module_lib/module_core.h"
#include "entity_manager/bypass_check_flags.h"

#include "entity_list_link.h"
#include "egs_mirror.h"
#include "egs_sheets/egs_static_game_sheet.h"

class CEntityBase;
class CSLinkEffect;
class CSEffect;

typedef NLMISC::CSmartPtr<CSLinkEffect> CSLinkEffectPtr;
typedef NLMISC::CSmartPtr<CSEffect> CSEffectPtr;

/**
 * CEquipmentSlots
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2002
 */
struct CEquipmentSlots
{
	// Extern mirrored properties for items
	SMirrorEquipment	Headdress;
	SMirrorEquipment	Head;
	SMirrorEquipment	Face;
	SMirrorEquipment	EarL;
	SMirrorEquipment	EarR;
	SMirrorEquipment	Neck;
	SMirrorEquipment	Shoulders;
	SMirrorEquipment	Back;
	SMirrorEquipment	Chest;
	SMirrorEquipment	Arms;
	SMirrorEquipment	WristL;
	SMirrorEquipment	WristR;
	SMirrorEquipment	Hands;
	SMirrorEquipment	FingerL;
	SMirrorEquipment	FingerR;
	SMirrorEquipment	Legs;
	SMirrorEquipment	AnkleL;
	SMirrorEquipment	AnkleR;
	SMirrorEquipment	Feet;

	struct CSheath
	{
		SMirrorEquipment	HandL;
		SMirrorEquipment	HandR;
		SMirrorEquipment	Ammo;
	};

	CSheath Sheath[ NB_SHEATH ];

	/**
	 *	Default constructor
	 */
	CEquipmentSlots();

	/**
	 * Serial
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);
};


/**
 * CEntityBase
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2001
 */
class CEntityBase : public CEntityBasePersistantData, public IModuleCore
{
public:
	// Start by declaring methods for persistent load/ save operations
	// The following macro is defined in persistent_data.h
	// At time of writing it evaluated to:
	//	void store(CPersistentDataRecord &pdr) const;
	//	void apply(CPersistentDataRecord &pdr);

	DECLARE_PERSISTENCE_METHODS


//	/// memorize xp gain per agressor until creature death
//	struct TDelayedXpGain
//	{
//		double xp;
//		SKILLS::ESkills skill;
//	};
//	typedef std::map < NLMISC::CEntityId, std::vector< TDelayedXpGain > > TDelayedXpGainContainer;
	

	//////////////////////////////////////////////////////////
	/// exception when trying to get access on an unknown stat
	//////////////////////////////////////////////////////////
	class EInvalidStat : public std::exception
	{
	public:
		const char *what( const std::string& var) const throw()
		{
			_ExceptionString = std::string("<CEntityBase> : Invalid stat name ") + var;
			return _ExceptionString.c_str();
		}
		virtual ~EInvalidStat() throw() {}
	private:
		mutable std::string _ExceptionString;
	};


	//////////////////
	/// Api of CEntity
	//////////////////
	/**
	 * Default constructor
	 */
	CEntityBase(bool noSkills);
	
	/**
	 * Destructor
	 */
	virtual ~CEntityBase();

	// clear() method used by apply() method
	void clear();

	/**
	 * Set the entity id
	 * \param id is the entity id
	 */
	inline void setId( const NLMISC::CEntityId& id ) { _Id = id; }

	/**
	 * Get the entity id
	 * \return the entity id
	 */
	const NLMISC::CEntityId& getId() const { return _Id; }

	/**
	 * Get the mirror entity row id
	 * \return the entity id
	 */
	inline const TDataSetRow& getEntityRowId() const { return _EntityRowId; }

	/**
	 * Return the state of this character
	 * \return the state 
	 */
	inline CEntityState& getState() { return _EntityState; }
	inline const CEntityState& getState() const { return _EntityState; }

	/// accessors to the coords
	inline sint32 getX() const { return _EntityState.X; }
	inline sint32 getY() const { return _EntityState.Y; }
	inline sint32 getZ() const { return _EntityState.Z; }
	inline float getHeading() const { return _EntityState.Heading; }
	inline NLMISC::CVector getPosition() const { return NLMISC::CVector( ((float)_EntityState.X) * 0.001f, ((float)_EntityState.Y) * 0.001f, ((float)_EntityState.Z) * 0.001f ); }


	/**
	 * Set state of this character
	 * \param the state 
	 */
	inline void setState( const CEntityState& es ) { _EntityState = es; }

	/**
	 * Set state of this character
	 * \param the state
	 */
	void setState( const COfflineEntityState& es ) { _EntityState.storeToTemp( es ); }

	/**
	 * Set the Name
	 * \param name is name of entity
	 */
	
	void setName( const ucstring& name );

	/**
	 * get the Name
	 * \return name of entity
	 */
	
	inline const ucstring& getName() const { return _Name; }

	/**
	 * Set the Surname
	 * \param name is surname of entity
	 */
//	inline void setSurname( const ucstring& name ) { _Surname = name; }

	/**
	 * get the Surname
	 * \return surname of entity
	 */
//	inline const ucstring& getSurname() const { return _Surname; }

	/**
	 * get the Race of the entity
	 * \return race of the entity
	 */
	inline EGSPD::CPeople::TPeople getRace() const { return _Race; }

	/**
	 * get the Gender of the entity
	 * \return gender of the entity
	 */
	inline uint8 getGender() const { return _Gender; }

	/**
	 * Set the size of the entity
	 * \param size the entity size
	 */
	inline void setSize( CREATURE_SIZE::ECreatureSize size ) { _Size = (uint8)size; }

	/**
	 * get the Surname
	 * \return size of the entity
	 */
	inline  CREATURE_SIZE::ECreatureSize getSize() const { return (CREATURE_SIZE::ECreatureSize) _Size; }


	/**
	 * Add the properties to the mirror (except the entity state)
	 * If keepSheetId is false, the sheet id in the object will not be used (useful to take the value in the mirror instead)
	 */
	void addPropertiesToMirror( const TDataSetRow& entityIndex, bool keepSheetId=true );

	/**
	 * Add the current entity state into the mirror
	 */
	void mirrorizeEntityState( bool copyValueToMirror=true, TDataSetRow entityIndex = TDataSetRow()	);

	/**
	 * Return the physical characteristics of this entity
	 * \return physical characteristics
	 */
	inline CPhysicalCharacteristics& getPhysCharacs() { return _PhysCharacs; }

	/**
	 * Return the physical scores of this entity
	 * \return physical score
	 */
	inline CPhysicalScores& getPhysScores() { return _PhysScores; }

	/**
	 * get the value of the specified skill
	 * \param skillName name of the skill
	 * \return skillValue (if skill not found return 0)
	 */
/*	inline sint32 getSkillValue( const std::string &skillName )
	{ 
		const SSkill *skill = _Skills.getSkillStruct( skillName );
		if (skill == NULL || skill->Current == 0)
		{
			nlwarning("<CEntityBase::getSkillValue> Cannot find skill struc %s", skillName.c_str() );
			return 0;
		}
		return skill->Current;
	}

*/	/**
	 * get the current value of the specified characteristic
	 * \param characName name of the characteristic
	 * \return charac value (if characName not found return 0)
	 */
	inline sint32 getCharacteristicCurrentValue( const std::string &characName )
	{ 
		const SCharacteristicsAndScores *charac = _PhysCharacs.getCharacteristicStruct( characName );
		if (charac == NULL || charac->Current == 0)
		{
			nlwarning("<CEntityBase::getCharacteristicCurrentValue> Cannot find charac struc %s", characName.c_str() );
			return 0;
		}
		return charac->Current;
	}


	/**
	 * Return the characteristics of this entity
	 * \return characteristics of the entity
	 */
	inline CPhysicalCharacteristics &getCharacteristics() { return _PhysCharacs; }


	/**
	 * Return the scores of this entity
	 * \return scores of the entity
	 */
	inline CPhysicalScores &getScores() { return _PhysScores; }


	/**
	 * Return the special modifiers on this entity
	 * \return scores of the entity
	 */
	inline CSpecialModifiers &getSpecialModifiers() { return _SpecialModifiers; }
	

	/**
	 * get the items in the equipment slots of the entity
	 */
	inline const CEquipmentSlots &getEquipment() const { return _Items; }

	// return weight of equipped weapons
	virtual uint32 getWeightOfEquippedWeapon() { return 1; }

	/**
	 * Return the type of this entity, the type is the sheet id
	 * \return the type
	 */
	inline NLMISC::CSheetId getType() const { return NLMISC::CSheetId(_SheetId); }

	/**
	 *	Set the value of a var
	 * \param var is the name of the variable
	 * \param value is the new value for the variable
	 * \return true if the value has been set, false if an error occured
	 */
	bool setValue( const std::string& var, const std::string& value );

	/**
	 *	Modify the value of a var
	 * \param var is the name of the variable
	 * \param value is the modification value
	 * \return true if the value has been changed, false if an error occured
	 */
	bool modifyValue( const std::string& var, const std::string& value );

	/**
	 *	Get the value of the variable
	 * \param var is the name of the variable
	 * \param value receive the current value of the variable
	 * \return true if the var has been found, false otherwise
	 */
	bool getValue( const std::string& var, std::string& value );

	/**
	 * Add a modifier to entity
	 * \param modifier is the modifier want to apply
	 */
//	void addModifier( const SActiveModifiers& modifier );

	/**
	 * Make all necessary update for entity at each ticks (manage modifier...)
	 */
	//virtual void tickUpdate( bool regenActive, uint8 timeMask );

	/**
	 * Serial: reading off-mirror, writing from mirror
	 */
	void serial(NLMISC::IStream &f) throw(NLMISC::EStream);

	/**
	 * Load George Sheet
	 * \param sheetId is id of sheet of entity
	 */
	void loadSheetEntity( const NLMISC::CSheetId& sheetId );

	/**
	 * set the mode without position or orientation information 
	 * \param mode the new mode
	 * \param forceUpdate if true, set mode without check gameplay rules (for EGS use only)
	 * \param disengage true to call phrase manager disengage, false otherwise
	 */
	virtual void setMode( MBEHAV::EMode mode, bool forceUpdate = false, bool disengage = true) = 0;

	/**
	 * get the current mode 
	 * \return the current mode
	 */
	inline MBEHAV::EMode getMode() const { return MBEHAV::EMode(_Mode.getValue().Mode); }

	/**
	 * set the mode with position or orientation information
	 * \param mode the new mode (with position or orientation)
	 */
	virtual void setMode( MBEHAV::TMode mode ) = 0;

	/**
	 * set the behaviour
	 * \param behaviour the new behaviour
	 */
	 void setBehaviour( MBEHAV::CBehaviour behaviour, bool forceUpdate = false );

	/**
	 * get the current behaviour 
	 * \return the current behaviour
	 */
	inline  const MBEHAV::CBehaviour &getBehaviour() const { return _Behaviour.getValue(); }

	/**
	 * set the current target 
	 * \param target the new target
	 */
	virtual void setTarget( const NLMISC::CEntityId& targetId, bool sendMessage = true );

	/**
	 * get the current target 
	 * \return the current target CEntityId
	 */
	inline const NLMISC::CEntityId& getTarget() const
	{ 
		TDataSetRow presentTarget = _Target();
		return TheDataset.isAccessible( presentTarget ) ? TheDataset.getEntityId( presentTarget ) : NLMISC::CEntityId::Unknown;
	}

	/**
	 * get current target (copy it, do not use the method several times, because points to shared memory)
	 * \return the current target TDataSetRow
	 */
	inline const TDataSetRow& getTargetDataSetRow() const { return _Target(); }

	/// cancel any static effect in progress (ie effects canceled if the player moves like casting, harvest, faber...)
	void cancelStaticEffects()
	{ return; }

	/**
	 * Memorize agressor
	 */
//	inline void addAgressor( NLMISC::CEntityId Id ) { _Agressor.insert( Id ); }

	/**
	 * remove an agressor
	 */
//	inline void removeAgressor( NLMISC::CEntityId Id ) { _Agressor.erase( Id ); }

	/// get the stunned state of the entity
	inline bool isStunned() const 
	{ 
		if (_Stunned.isInitialized()) return _Stunned;  
		else return false;
	}

	// stun the entity
	void stun();

	// wake the entity (from a stun)
	void wake();

	// Entity want mout another

	/**
	 *	get a stat value
	 * \param var is the name of the variable
	 * \return the stat value
	 */
	sint32 getStatValue ( const std::string& stat ){ return lookupStat(stat); }

	///\ return the regen2 variable
	float getRegen2();

	///\ return the score2 variable
	float getScore2();

	// Set rider entity
	void setRiderEntity( const TDataSetRow& rowId ) { _RiderEntity = rowId; }

	// get rider entity
	const TDataSetRow& getRiderEntity() const { return _RiderEntity(); }

	// Set entity mounted
	void setEntityMounted( const TDataSetRow& entityRowId ) { _EntityMounted = entityRowId; }

	// get entity mounted
	const TDataSetRow& getEntityMounted() const { return _EntityMounted(); }

	// get reference on contextual property
	CMirrorPropValueAlice< CProperties, CPropLocationPacked<2> >& getContextualProperty() 
	{ 
		return _ContextualProperty; 
	}

	// Set the race of the owner of this entity
	void setOwnerPeople( MOUNT_PEOPLE::TMountPeople people ) { _OwnerPeople = (TYPE_OWNER_PEOPLE)people; }
	

	/**
	 *	get a reference to a stat value
	 * \param var is the name of the variable
	 * \return ref on the stat value
	 */
	sint32& lookupStat( const std::string& stat ) throw (EInvalidStat);

	/**
	 *	get a reference on a characterristics value
	 * \param c is enum of characteristic, st is enum of subtype of characterisitics (like max, current..)
	 * \return ref on the stat value
	 */
	sint32& lookupStat( CHARACTERISTICS::TCharacteristics c, SCharacteristicsAndScores::TCharacteristicsAndScoreSubType st ) throw (CEntityBase::EInvalidStat);

	/**
	 *	get a reference on a scores value
	 * \param score is enum of score, st is enum of subtype of score (like max, current..)
	 * \return ref on the stat value
	 */
	sint32& lookupStat( SCORES::TScores score, SCharacteristicsAndScores::TCharacteristicsAndScoreSubType st ) throw (CEntityBase::EInvalidStat);

	/**
	 *	get a reference on a skill value
	 * \param skill is enum of Skills, st is enum of subtype of skill (like base, current..)
	 * \return ref on the stat value
	 */
	sint32& lookupStat( SKILLS::ESkills skill, SSkill::ESkillSubType st ) throw (CEntityBase::EInvalidStat);

	/**
	 *	get a reference on a SpecialModifiers value
	 * \param sm is enum of SpecialModifiers
	 * \return ref on the stat value
	 */
	sint32& lookupStat( CSpecialModifiers::ESpecialModifiers sm ) throw (CEntityBase::EInvalidStat);
	const sint32& lookupStat( CSpecialModifiers::ESpecialModifiers sm ) const throw (CEntityBase::EInvalidStat);

	/// accessors on hp (read only)
	inline sint32 currentHp() const { return _PhysScores._PhysicalScores[SCORES::hit_points].Current;}

	/// accessors on max hp (read only)
	inline sint32 maxHp() const { return _PhysScores._PhysicalScores[SCORES::hit_points].Max;}

	/// accessor on score
	inline sint32 getCurrentScore(SCORES::TScores score) const 
	{ 
		if (score < 0 || score > SCORES::NUM_SCORES)
			return 0;
		return _PhysScores._PhysicalScores[score].Current;
	}
	
	/// accessor on max score (read only)
	inline sint32 getMaxScore(SCORES::TScores score) const 
	{ 
		if (score < 0 || score > SCORES::NUM_SCORES)
			return 0;
		return _PhysScores._PhysicalScores[score].Max;
	}

	/**
	 * change hp with a delta value
	 * \param deltaValue the delta value applied on hp
	 * \return bool true if the entity died from the changement
	 */
	virtual bool changeCurrentHp(sint32 deltaValue, TDataSetRow responsibleEntity = TDataSetRow());

	/**
	 * change given score with a delta value (NOT to use for hitpoints, use changeCurrentHP instead
	 * \param deltaValue the delta value applied on stamina
	 */
	inline void changeScore(SCORES::TScores score, sint32 delta)
	{
		if (score >= SCORES::unknown || score < 0)
			return;

		if (score == SCORES::hit_points)
		{
			changeCurrentHp(delta, TDataSetRow());
			return;
		}

		// if entity is mezzed and delta is <0 unmezz it
		if (_MezzCount && delta < 0)
		{
			unmezz();
		}

		_PhysScores._PhysicalScores[score].Current = _PhysScores._PhysicalScores[score].Current + delta;
		if (_PhysScores._PhysicalScores[score].Current <= 0)
		{
			_PhysScores._PhysicalScores[score].Current = 0;
			setScoreBar(score,0);
		}
		else if ( _PhysScores._PhysicalScores[score].Current >= _PhysScores._PhysicalScores[score].Max)
		{
			_PhysScores._PhysicalScores[score].Current = _PhysScores._PhysicalScores[score].Max;
			setScoreBar(score,~0);
		}
		else
		{
			if( _PhysScores._PhysicalScores[score].Max > 0)
			{
				uint32 bar;
				if( score == SCORES::hit_points )
				{
					bar = (uint32)( (1023 * _PhysScores._PhysicalScores[score].Current) / _PhysScores._PhysicalScores[score].Max);
				}
				else
				{
					bar = (uint32)( (127 * _PhysScores._PhysicalScores[score].Current) / _PhysScores._PhysicalScores[score].Max);
				}
				setScoreBar(score,bar);
			}
			else
				setScoreBar(score,0);
			
		}
	}

	///
	inline void setBars()
	{
		register TYPE_BARS barresValue = 0;
		register sint32 barLevel;
		SCharacteristicsAndScores &hp = _PhysScores._PhysicalScores[SCORES::hit_points];
		sint32 maxhp = hp.Max;
		if ( maxhp != 0 )
			barLevel = (sint32)( (1023 * hp.Current) / maxhp);
		else
			barLevel = 0;
		barresValue = (uint32)barLevel & 0x7ff;

		// bots only use the hp bar
		if (_Id.getType() != RYZOMID::player)
		{
			_StatusBars = barresValue;
			return;
		}
		
		if( _PhysScores._PhysicalScores[SCORES::stamina].Max > 0 )
		{
			barLevel = (uint32)( (127 * _PhysScores._PhysicalScores[SCORES::stamina].Current) / _PhysScores._PhysicalScores[SCORES::stamina].Max);
		}
		else
		{
			barLevel = 0;
		}
		barresValue = barresValue | (barLevel<<11);

		if( _PhysScores._PhysicalScores[SCORES::sap].Max > 0 )
		{
			barLevel = (uint32)( (127 * _PhysScores._PhysicalScores[SCORES::sap].Current) / _PhysScores._PhysicalScores[SCORES::sap].Max);
		}
		else
		{
			barLevel = 0;
		}
		barresValue = barresValue | (barLevel<<18);

		if( _PhysScores._PhysicalScores[SCORES::focus].Max > 0 )
		{
			barLevel = (uint32)( (127 * _PhysScores._PhysicalScores[SCORES::focus].Current) / _PhysScores._PhysicalScores[SCORES::focus].Max);
		}
		else
		{
			barLevel = 0;
		}
		_StatusBars = barresValue | (barLevel<<25);
	}

	/// set all bars
	inline void setBars(uint32 val)
	{
		_StatusBars = val;
	}

	/// set given score bar
	inline void setScoreBar(SCORES::TScores score, uint32 value)
	{
		switch(score)
		{
		case SCORES::hit_points:
			setHpBar(value);
			break;
		case SCORES::stamina:
			setStaminaBar(value);
			break;
		case SCORES::sap:
			setSapBar(value);
			break;
		case SCORES::focus:
			setFocusBar(value);
			break;
		default:
			nlwarning("<setScoreBar> Score %s not managed",SCORES::toString(score).c_str());
			return;
		};
	}

	/// set hp bar
	inline void setHpBar( uint32 hpBar )
	{
		hpBar &= 0x7ff;
		_StatusBars = _StatusBars & (0xfffff800); //erase last 11 digits (0..10)
		_StatusBars = _StatusBars | uint32(hpBar);
	}

	/// set stamina bar
	inline void setStaminaBar( uint32 staminaBar )
	{
		staminaBar &= 0x7f;
		_StatusBars = _StatusBars & (0xfffc07ff); //erase stamina digits (11..16)
		_StatusBars = _StatusBars | (staminaBar << 11);
	}

	/// set sap bar
	inline void setSapBar( uint32 sapBar )
	{
		sapBar &= 0x7f;
		_StatusBars = _StatusBars & (0xfe03ffff); //erase sap digits (17..23)
		_StatusBars = _StatusBars | (sapBar << 18);
	}

	/// set focus bar
	inline void setFocusBar( uint32 focusBar )
	{
		focusBar &= 0x7f;
		_StatusBars = _StatusBars & (0x01ffffff); //erase focus digits (24..31)
		_StatusBars = _StatusBars | (focusBar << 25);
	}

	/// get the number of bag
//	uint8 getNbBag() const { return _NbBag; }

	// set a flag
	inline uint32 getActionFlag() const { return _ActionFlags.getValue(); }

	// set a flag
	// remove the 'inline' for bug checking in combat phrase
	void setActionFlag( RYZOMACTIONFLAGS::TActionFlag flag, bool value = true );

	// check action flag is init in mirror
	inline bool isActionFlagInit() { return _ActionFlags.isInitialized(); }

	// interface for recent pvp action flag management in PvP
	inline bool getPVPRecentActionFlag() { return false; }

	// protected slot
	inline virtual void protectedSlot( SLOT_EQUIPMENT::TSlotEquipment slot) { _ProtectedSlot = slot; }
	inline SLOT_EQUIPMENT::TSlotEquipment protectedSlot() const { return _ProtectedSlot; }

	// dodge or parry as defense
	inline virtual void dodgeAsDefense( bool b) { _DodgeAsDefense = b; }
	inline bool dodgeAsDefense() const { return _DodgeAsDefense; }

	///get the sabrina effects on a creature
	inline const std::vector<CSEffectPtr>& getSEffects() { return _SEffects; }

	// add an effect to this entity
	virtual bool addSabrinaEffect( CSEffect *effect );

	/** 
	 * remove an effect on this entity. The effect is not deleted.
	 * \param effect the effect to remove
	 * \param activateSpleepingEffect activate or not sleeping effects of the same family, default = true, only false when clearing all effects
	 * \return true if no other effect of this family is active, false otherwise
	 */
	virtual bool removeSabrinaEffect( CSEffect *effect, bool activateSleepingEffect = true );

	// add a link to this entity
	virtual void addLink( CSLinkEffect *effect );

	// remove a link from this entity. The effect is not deleted
	virtual void removeLink( CSLinkEffect *effect, float factorOnSurvivalTime);

	// stop all links (a broken link last a few more seconds before being deleted)
	virtual void stopAllLinks(float factorOnSurvivalTime = 1.0f);

	/// get current number of links cast by this entity
	inline uint8 getNbLinks() const { return (uint8)_SEffectLinks.size(); }

	/**
	 * look for a particular effect. 
	 * \return a smart pointer on the most powerful matching effect. NULL if none was found
	 */ 
	const CSEffectPtr &lookForActiveEffect( EFFECT_FAMILIES::TEffectFamily effectType );

	// return a reference on all active effects
	const std::vector< CSEffectPtr >& getAllActiveEffects() const { return 	_SActiveEffects; }

	/// unmezz entity
	void unmezz();

	/// get mezz count
	inline uint8 mezzCount() const { return _MezzCount; }

	/**
	 * apply the effect of the armor/shield on damage. Update the armor items if necessary
	 * \return the remaining damages
	 */
	virtual sint32 applyDamageOnArmor( DMGTYPE::EDamageType dmgType, sint32 damage, SLOT_EQUIPMENT::TSlotEquipment forcedSlot = SLOT_EQUIPMENT::UNDEFINED ) =0;

	///\return the malus on spell casting due to armor
	virtual float getArmorCastingMalus() { return 0.0f; }

	/// Return the damage using current armor, done by an explosion (e.g. forage source explosion)
	virtual float getActualDamageFromExplosionWithArmor( float dmg ) const =0;

	inline virtual void clearCurrentAction() {}

	inline virtual void setCurrentAction(CLIENT_ACTION_TYPE::TClientActionType,NLMISC::TGameCycle) {}

	///returns true if the entity is dead
	inline bool isDead() const { return _IsDead; }

	/// can entity use any action ? (combat, craft...) returns true if entity can start an action
	virtual bool canEntityUseAction(CBypassCheckFlags bypassCheckFlags = CBypassCheckFlags::NoFlags, bool sendMessage = true) const;

	/// can entity defend ? (dodge/parry) returns true if entity can defend itself
	virtual bool canEntityDefend();

	/// can entity moves ?
	inline bool canEntityMove() const { return ( !_Stunned.getValue() && _PreventEntityMoves == 0 ); }

	/// inc prevent entity moves var
	inline void incPreventEntityMove() { ++_PreventEntityMoves; }
	/// dec prevent entity moves var
	inline void decPreventEntityMove() { if (_PreventEntityMoves>0) --_PreventEntityMoves; }

	// tp wanted for an entity
	virtual void tpWanted( sint32 x, sint32 y, sint32 z , bool useHeading = false, float heading = 0.0f , uint8 continent = 0xFF, sint32 cell = 0) = 0;

// memorize xp gain per agressor for offensive action
//	void addAgressorXp( const NLMISC::CEntityId& agressor, double xp, const std::string& Skill );

	// giveAgressorXp: give to agressor memorized xp gain
//	void giveAgressorXp();
		
	///
//	virtual void storeAgressor( TDataSetRow actor ) {}

	/// get the resist value associated to effect type
	virtual uint32 getMagicResistance(EFFECT_FAMILIES::TEffectFamily effectFamily) = 0;
	
	/// get the resist value associated to damage type
	virtual uint32 getMagicResistance(DMGTYPE::EDamageType dmgType) = 0;
	
	/// inc creature resist modifier
	virtual void incResistModifier(EFFECT_FAMILIES::TEffectFamily, float factor) {}
	/// inc creature resist modifier
	virtual void incResistModifier(DMGTYPE::EDamageType, float factor) {}

	/// accessor on damage shield damage
	inline uint16 getDamageShieldDamage() const { return _DamageShieldDamage; }
	/// accessor on damage shield Hp Drain
	inline uint16 getDamageShieldHpDrain() const { return _DamageShieldHpDrain; }

	// damage shield modifiers
	inline void incDamageShield( uint16 value ) { _DamageShieldDamage += value; _DamageShieldHpDrain += value; }
	inline void decDamageShield( uint16 value ) { _DamageShieldDamage -= value; _DamageShieldHpDrain -= value; }

	/// remove all spells (effect on the entity and casted links)
	void removeAllSpells();

	/// Set instance number
	void setInstanceNumber( uint32 instanceNb )
    {
		if (IsRingShard)
		{
			// this is a ring shard, we ignore silently the change
            return;
        }
        _InstanceNumber = instanceNb;
    }

    void setRingShardInstanceNumber( uint32 instanceNb )
    {
        BOMB_IF(!IsRingShard,"Ignoring attempt to set AIInstance number because this is NOT a ring shard",return);
        _InstanceNumber = instanceNb;
    }

	uint32 getInstanceNumber(){ return _InstanceNumber(); }

	// return who see me flags
	TYPE_WHO_SEES_ME getWhoSeesMe() const { return _WhoSeesMe(); }
	void setWhoSeesMe( TYPE_WHO_SEES_ME val );

	/// get specific skill modifier for given race, always 0 for creatures, overridden for CCharacter
	virtual sint32 getSkillModifierForRace(EGSPD::CPeople::TPeople) const { return 0; }

	// toggle god mode
	void toggleGodMode() { _GodMode = ! _GodMode; }
	void setGodMode(bool g) { _GodMode = g; }
	bool godMode() const { return _GodMode; }

	void setInvulnerableMode(bool i) { _Invulnerable = i; }
	bool invulnerableMode() const { return _Invulnerable; }

	void setEventSpeedVariationModifier(float value) { _EventSpeedVariationModifier = value; }
	float getEventSpeedVariationModifier() { return _EventSpeedVariationModifier; }

	/// change the outpost alias
	virtual void setOutpostAlias( uint32 id );
	/// get the outpost alias
	uint32 getOutpostAlias() const;
	/// change the outpost side
	void setOutpostSide( OUTPOSTENUMS::TPVPSide side);
	/// get the outpost side
	OUTPOSTENUMS::TPVPSide getOutpostSide() const;
	/// update the outpost alias part in the visual property sent to client
	void updateOutpostAliasClientVP();
	/// update the outpost side part in the visual property sent to client
	void updateOutpostSideClientVP();
	/// true if entity is an outpost enemy
	bool isEntityAnOutpostEnemy( const NLMISC::CEntityId& id );
	/// return true if entity is a spire
	bool isSpire() const;
	/// get creature static form
	virtual const CStaticCreatures* getForm() const { return 0; }

protected:
	/**
	 * kill entity (when it's hit points reach 0)
	 * \param killerRowId if valid, the TDataSetRow of the entity which has killed the current entity
	 */
	virtual void kill(TDataSetRow killerRowId = TDataSetRow()) = 0;
		
	/// Apply modifier (immediately if Direct modifier (TicksDuration = 0) or add it to active modifiers vector if have a duration)
//	void applyModifier( const SActiveModifiers& modifier );

	// manage effects, remove effects with no time duration left, apply effects...
	void tickEffectUpdate();

	/// Set all entity stats modifiers to initale states
	virtual void resetEntityModifier();

	/// recompute all Max value
	void computeMaxValue();

	/// apply regenerate and clip currents value
	void applyRegenAndClipCurrentValue();

	/// entity is dead
	virtual void deathOccurs() = 0;

	/**
	 * look for a particular effect. 
	 * \param getHigherValue : set it to true to retrieve the effect with the higer value
	 * \return a smart pointer on the most powerful matching effect. NULL if none was found
	 */ 
	const CSEffectPtr &lookForSEffect( EFFECT_FAMILIES::TEffectFamily effectType, bool getHigherValue = true );

	///////////
	/// Members
	///////////
	/// Entity id
	NLMISC::CEntityId						_Id;
	TDataSetRow								_EntityRowId;

	/// Items slots
	CEquipmentSlots							_Items;
	
	/// Continent instance number. Can be either a static or dynamic instance.
	CMirrorPropValueAlice< uint32 >				_InstanceNumber;

	/// Entity  Mode
	CMirrorPropValueAlice< MBEHAV::TMode, CPropLocationPacked<2> >	_Mode;
	
	/// Entity Behaviours
	CMirrorPropValueAlice< MBEHAV::CBehaviour, CPropLocationPacked<2> >	_Behaviour;
	/// Current Target
	CMirrorPropValueAlice< TDataSetRow, CPropLocationPacked<2> >	_Target;

	/// the status bars on entity head
	CMirrorPropValueAlice<TYPE_BARS, CPropLocationPacked<2> > _StatusBars;

	/// Set of entities perform last offensive action against entity, cleared at each service tick
//	std::set< NLMISC::CEntityId >			_Agressor;

	/// memorize xp gain per agressor until creature death
//	TDelayedXpGainContainer					_AgressorXp;

	/// Verbose commuted if true, for debug
	bool									_Verbose;

	/// boolean indicating if the entity is stunned or not
	CMirrorPropValueAlice< bool, CPropLocationPacked<2> >	_Stunned;

	// mount management
	CMirrorPropValueAlice< TDataSetRow, CPropLocationPacked<2> >	_EntityMounted;
	CMirrorPropValueAlice< TDataSetRow, CPropLocationPacked<2> >	_RiderEntity;

	// Contextual properties management
	CProperties								_StaticContextualProperty;
	CMirrorPropValueAlice< CProperties, CPropLocationPacked<2> >	_ContextualProperty;

	/// flags used by AI service to know the state of the entity
	CMirrorPropValueAlice< uint16, CPropLocationPacked<2> >	_ActionFlags;	

	/// sabrina effects on this entity
	std::vector< CSEffectPtr >			_SEffects;
	/// keep the best effect of each family or any effect that is stackable, so all the 'active' effects
	std::vector< CSEffectPtr >			_SActiveEffects;
	/// links created by this entity
	std::vector< CSLinkEffectPtr >		_SEffectLinks;

	/// number of MEZZ effects on entity
	uint8								_MezzCount;

	/// Indicate if the entity is Dead, as we cannot always trust the mode (for creatures, as the AIS manages the mode)
	bool								_IsDead;

	/// number of effects preventing the entity from moving (root, stun, mezz...)
	uint8								_PreventEntityMoves;

	/// damage shield : damage inflicted to melee attackers
	uint16								_DamageShieldDamage;
	/// damage shield : HP drained from attacker (entity gains HP, attacker do NOT lose HP)
	uint16								_DamageShieldHpDrain;

	CEntityListLink<CEntityBase>		_ListLink;	

	// who sees me property
	CMirrorPropValue<TYPE_WHO_SEES_ME, CPropLocationPacked<2> > _WhoSeesMe;

	// Give the owner people (if have one). Used for pet, they then can have different size on client depending on the people
	CMirrorPropValueAlice<TYPE_OWNER_PEOPLE, CPropLocationPacked<2> > _OwnerPeople;
	
	// cheat god mode for testing
	bool						_GodMode;
	// cheat invumnerable mode (to all) for testing and GM/CSR
	bool						_Invulnerable;

	// Can be used to change the mob speed (1.f means normal speed)
	float								_EventSpeedVariationModifier;

	/// outpost alias
	CMirrorPropValue<TYPE_IN_OUTPOST_ZONE_ALIAS>	_OutpostAlias;

	/// outpost side
	CMirrorPropValue<TYPE_IN_OUTPOST_ZONE_SIDE>		_OutpostSide;

	/// Oupost infos
	union TOutpostInfos
	{
		uint16		Infos;
		
		struct
		{
			uint16	Id	: 15;
			uint16	Side: 1;
		} IdAndSide;
	};
	CMirrorPropValueAlice< uint16, CPropLocationPacked<2> >	_OutpostInfos;
};	


typedef NLMISC::CSmartPtr<CEntityBase> CEntityBasePtr;
typedef NLMISC::CRefPtr<CEntityBase> CEntityBaseRefPtr;

#endif // ENTITY_BASE_H

/* End of entity_base.h */
