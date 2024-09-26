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



#ifndef RYZOM_HARVEST_PHRASE_H
#define RYZOM_HARVEST_PHRASE_H

#include "phrase_manager/s_phrase.h"


/**
 * This class represents a faber phrase in the Sabrina system. See CSPhrase for methods definitions
 * Now, used only by quartering.
 *
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CHarvestPhrase : public CSPhrase
{
public:

	/// ctor
	CHarvestPhrase();

	/// dtor
	virtual ~CHarvestPhrase();

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

	///\unused basic methods from CSPhrase
	//@{
	virtual void setPrimaryItem( CGameItemPtr itemPtr ){}
	virtual void setSecondaryItem( CGameItemPtr itemPtr ){}
	virtual void addConsumableItem( CGameItemPtr itemPtr ){}
	virtual void setPrimaryTarget( const TDataSetRow& ) {}
	//@}

	//@}
		
	/**
	 * called at the end of the latency time
	 */
	virtual void end();

	/**
	 * called when brutally stop the phrase
	 */
	virtual void stop();
	//@}

	inline const TDataSetRow & getActor() const { return _ActorRowId;}
	inline sint32 getSabrinaCost() const { return (sint32)(_SabrinaCost * _SabrinaRelativeCost); }
	inline sint32 getSabrinaCredit() const { return (sint32)(_SabrinaCredit * _SabrinaRelativeCredit); }

	inline void setRawMaterial( const NLMISC::CSheetId &rm ) { _RawMaterialId = rm; }
	inline void minQuality(uint16 min) { _MinQuality = min; }
	inline void maxQuality(uint16 max) { _MaxQuality = max; }
	inline void quantity(uint16 qty) { _Quantity = qty; }

	//inline void deposit(bool b) { _Deposit = b; }
	//inline bool deposit() const { return _Deposit; }

private:
	/// process harvest corpse result
	void harvestCorpseResult();

private:
	/// acting entity
	TDataSetRow					_ActorRowId;

	/// total cost (sabrina system)
	sint32						_SabrinaCost;
	/// relative cost must be added to total cost
	float						_SabrinaRelativeCost;
	/// total credit (sabrina system)
	sint32						_SabrinaCredit;
	/// relative credit must be added to total credit
	float						_SabrinaRelativeCredit;
	/// stamina cost of the harvest action
	sint32						_StaminaCost;
	/// hp cost
	sint32						_HPCost;
	/// harvest time in ticks
	NLMISC::TGameCycle			_HarvestTime;

	/// harvested raw material
	NLMISC::CSheetId			_RawMaterialId;
	/// min obtained quality
	uint16						_MinQuality;
	/// max obtained quality
	uint16						_MaxQuality;
	/// harvested quantity
	uint16						_Quantity;
	/// harvest a deposit or a creature (obsolete, deposit now done by foraging actions)
	//bool						_Deposit;

	/// root brick sheetId (used to send to client)
	NLMISC::CSheetId			_RootSheetId;

};

#endif // RYZOM_HARVEST_PHRASE_H

/* End of faber_phrase.h */
