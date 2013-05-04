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



#ifndef RYZOM_FABER_PHRASE_H
#define RYZOM_FABER_PHRASE_H

#include "phrase_manager/s_phrase.h"
#include "faber_action.h"


/**
 * This class represents a faber phrase in the Sabrina system. See CSPhrase for methods definitions
 * \author Alain Saffray
 * \author Nevrax France
 * \date 2003
 */
class CFaberPhrase : public CSPhrase
{
public:

	/// ctor
	CFaberPhrase();

	/// dtor
	virtual ~CFaberPhrase() { _Mps.clear(); _MpsFormula.clear(); if(_FaberAction) delete _FaberAction; }

	/// \name Override methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks, bool buildToExecute = true );

	/**
	 * evaluate phrase
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate();
	
	/**
	 * validate phrase
	 * \return true if phrase is valide
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch();
	virtual void apply();
	//@}
	
	
	/**
	 * called at the end of the latency time
	 */
	virtual void end();
	//@}

	/**
	 * called when the action is interupted
	 */
	virtual void stop();
	//@}

	///\unused basic methods from CSPhrase
	//@{
	virtual void setPrimaryItem( CGameItemPtr itemPtr ){}
	virtual void setSecondaryItem( CGameItemPtr itemPtr ){}
	virtual void addConsumableItem( CGameItemPtr itemPtr ){}
	virtual void setPrimaryTarget( const TDataSetRow& ) {}
	//@}

	// Craft methodes
	inline const TDataSetRow & getActor() const { return _ActorRowId;}
	inline sint32 getSabrinaCost() const { return (sint32)(_SabrinaCost * _SabrinaRelativeCost); }
	inline sint32 getFocusCost() const { return _FocusCost; }
	inline const CStaticItem *  getCraftedItemStaticForm() const { return _CraftedItemStaticForm; }
	inline const CStaticBrick * getRootFaberPlan() const { return _RootFaberPlan; }
	inline const std::vector< const CStaticItem * > & getMps() const { return _Mps; }
	inline const std::vector< const CStaticItem * > & getMpsFormula() { return _MpsFormula; }
	inline uint16 getLowerRmQuality() const { return _LowerRmQuality; }
	
	// get recommended skill for use item
	inline uint32 getRecommendedSkill() const { return _Recommended; }

	// item stats modifiers
	inline sint16 getMBOQuality() const { return _MBOQuality; }
	inline float getMBODurability() const { return _MBODurability; }
	inline float getMBOWeight() const { return _MBOWeight; }
	inline float getMBODmg() const { return _MBODmg; }
	inline float getMBOSpeed() const { return _MBOSpeed; }
	inline float getMBORange() const { return _MBORange; }
	inline float getMBOProtection() const { return _MBOProtection; }
	inline float getMBOSapLoad() const { return _MBOSapLoad; }

	// energy buff on item
	inline sint32 getMBOHitPoint() { return _MBOHitPoint; }
	inline sint32 getMBOSap() { return _MBOSap; }
	inline sint32 getMBOStamina() { return _MBOStamina; }
	inline sint32 getMBOFocus() { return _MBOFocus; }

	// bonus for magic casting with item
	inline float getMBOElementalCastingTimeFactor() const { return _MBOElementalCastingTimeFactor; }
	inline float getMBOElementalPowerFactor() const { return _MBOElementalPowerFactor; }
	inline float getMBOOffensiveAfflictionCastingTimeFactor() const { return _MBOOffensiveAfflictionCastingTimeFactor; }
	inline float getMBOOffensiveAfflictionPowerFactor() const { return _MBOOffensiveAfflictionPowerFactor; }
	inline float getMBOHealCastingTimeFactor() const { return _MBOHealCastingTimeFactor; }
	inline float getMBOHealPowerFactor() const { return _MBOHealPowerFactor; }
	inline float getMBODefensiveAfflictionCastingTimeFactor() const { return _MBODefensiveAfflictionCastingTimeFactor; }
	inline float getMBODefensiveAfflictionPowerFactor() const { return _MBODefensiveAfflictionPowerFactor; }

	// craft item for system item instanciate
	CGameItemPtr systemCraftItem( const NLMISC::CSheetId& sheet, const std::vector< NLMISC::CSheetId >& Mp, const std::vector< NLMISC::CSheetId >& MpFormula );
	inline void setSystemCraftedItem( CGameItemPtr item ) { _CraftedItem = item; }

private:
	// Faber action
	IFaberAction *				_FaberAction;

	/// acting entity
	TDataSetRow					_ActorRowId;

	/// total cost (sabrina system)
	sint32						_SabrinaCost;
	/// Relative cost must be added to total cost
	float						_SabrinaRelativeCost;
	/// focus cost of the faber action
	sint32						_FocusCost;
	/// faber time in ticks
	NLMISC::TGameCycle			_FaberTime;

	// craft action params for result
	const CStaticItem *			_CraftedItemStaticForm;
	bool						_RootFaberBricks;
	const CStaticBrick * 		_RootFaberPlan;
	std::vector< const CStaticItem * > _Mps;
	std::vector< const CStaticItem * > _MpsFormula;
	uint16						_LowerRmQuality;

	// recommended wanted
	uint32						_Recommended;

	// stats modifiers on item
	sint16						_MBOQuality;
	float						_MBODurability;
	float						_MBOWeight;
	float						_MBODmg;
	float						_MBOSpeed;
	float						_MBORange;
	float						_MBOProtection;
	float						_MBOSapLoad;

	// energy buff on item
	sint32						_MBOHitPoint;
	sint32						_MBOSap;
	sint32						_MBOStamina;
	sint32						_MBOFocus;

	// bonus for magic casting with item
	float						_MBOElementalCastingTimeFactor;
	float						_MBOElementalPowerFactor;
	float						_MBOOffensiveAfflictionCastingTimeFactor;
	float						_MBOOffensiveAfflictionPowerFactor;
	float						_MBOHealCastingTimeFactor;
	float						_MBOHealPowerFactor;
	float						_MBODefensiveAfflictionCastingTimeFactor;
	float						_MBODefensiveAfflictionPowerFactor;
	
	CGameItemPtr				_CraftedItem; // only for internal use for system craft, no persistant pointers
};

#endif // RYZOM_FABER_PHRASE_H

/* End of faber_phrase.h */
