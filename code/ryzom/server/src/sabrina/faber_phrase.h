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

#include "nel/misc/types_nl.h"
#include "s_phrase.h"
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
	virtual ~CFaberPhrase() { _Mps.clear(); if(_FaberAction) delete _FaberAction; }

	/// \name Override methods from CSPhrase
	//@{
	virtual bool build( const TDataSetRow & actorRowId, const std::vector< const CStaticBrick* >& bricks );

	/**
	 * evaluate phrase
	 * \param evalReturnInfos struct that will receive evaluation results
	 * \return true if eval has been made without errors
	 */
	virtual bool evaluate(CEvalReturnInfos *msg = NULL);
	
	/**
	 * validate phrase
	 * \return true if phrase is valide
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual void apply();
	//@}
	
	/**
	 * set the actor
	 */
	virtual void setActor( const TDataSetRow &entityRowId ){}
	
	/**
	 * called at the end of the latency time
	 */
	virtual void end(){}
	//@}

	///\unused basic methods from CSPhrase
	//@{
	virtual void setPrimaryItem( CGameItemPtr itemPtr ){}
	virtual void setSecondaryItem( CGameItemPtr itemPtr ){}
	virtual void addConsumableItem( CGameItemPtr itemPtr ){}
	virtual void setPrimaryTarget( const TDataSetRow& ) {}
	virtual void addTargetEntity( const TDataSetRow& ) {}
	//@}

	inline const TDataSetRow & getActor() { return _ActorRowId;}
	inline sint32 getSabrinaCost() { return _SabrinaCost; }
	inline sint32 getSabrinaCredit() { return _SabrinaCredit; }
	inline const CStaticItem * getRootFaberPlan() { return _RootFaberPlan; }
	inline const CStaticItem * getTool() { return _Tool; }
	inline std::vector< const CStaticItem * > getMps() { return _Mps; }

	
	inline sint16 getMBOQuality() { return _MBOQuality; }
	inline sint32 getMBODurability() { return _MBODurability; }
	inline float getMBOWeight() { return _MBOMBOWeight; }
	inline sint16 getMBODmg() { return _MBODmg; }
	inline float getMBOSpeed() { return _MBOSpeed; }
	inline float getMBORange() { return _MBORange; }
	inline sint16 getMBOProtection() { return _MBOProtection; }
	inline sint16 getMBOSapLoad() { return _MBOSapLoad; }

	// craft item for system item instanciate
	CGameItemPtr systemCraftItem( const NLMISC::CSheetId& sheet, const std::vector< NLMISC::CSheetId >& Mp );
	inline void setCraftedItem( CGameItemPtr item ) { _CraftedItem = item; }

private:
	// Faber action
	IFaberAction *				_FaberAction;

	/// acting entity
	TDataSetRow					_ActorRowId;

	/// total cost (sabrina system)
	sint32						_SabrinaCost;
	/// total credit (sabrina system)
	sint32						_SabrinaCredit;
	/// stamina cost of the faber action
	sint32						_StaminaCost;
	/// hp cost
	sint32						_HPCost;
	/// faber time in ticks
	NLMISC::TGameCycle			_FaberTime;

	// craft action params for result
	const CStaticItem * 		_RootFaberPlan;
	const CStaticItem *			_Tool;
	std::vector< const CStaticItem * > _Mps;

	sint16						_MBOQuality;
	sint32						_MBODurability;
	float						_MBOMBOWeight;
	sint16						_MBODmg;
	float						_MBOSpeed;
	float						_MBORange;
	sint16						_MBOProtection;
	sint16						_MBOSapLoad;
	CGameItemPtr				_CraftedItem; // only for internal use for system craft, no persistant pointers
};

#endif // RYZOM_FABER_PHRASE_H

/* End of faber_phrase.h */
