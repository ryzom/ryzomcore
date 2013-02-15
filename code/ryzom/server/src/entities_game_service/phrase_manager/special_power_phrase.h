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



#ifndef RYZOM_SPECIAL_POWER_PHRASE_H
#define RYZOM_SPECIAL_POWER_PHRASE_H

//
#include "special_power.h"
#include "phrase_manager/s_phrase.h"
#include "entity_manager/bypass_check_flags.h"
#include "egs_sheets/egs_static_brick.h"


class CGameItem;

/**
 * This class represents a sabrina phrase used for special powers such as auras
 * \author David Fleury
 * \author Nevrax France
 * \date 2003
 */
class CSpecialPowerPhrase : public CSPhrase
{
public:

	/// ctor
	CSpecialPowerPhrase();

	/// dtor
	virtual ~CSpecialPowerPhrase();

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
	 * \return true if phrase is valid
	 */
	virtual bool validate();
	virtual bool update();
	virtual void execute();
	virtual bool launch();
	virtual void apply();
	
	/**
	 * called at the end of the latency time
	 */
	virtual void end();

	/**
	 * called when brutally stop the phrase
	 */
	virtual void stop();
	//@}

	/// init phrase from a consumbale item
	bool buildFromConsumable(const TDataSetRow & actorRowId, const CStaticItem *itemForm, uint16 quality);

	/// get actor
	inline const TDataSetRow &getActor() const { return _ActorRowId;}
	/// get targets
	const std::vector<TDataSetRow> &getTargets() const { return _Targets; }
	/// get additional recast time
	inline NLMISC::TGameCycle getAdditionalRecastTime() const { return _AddRecastTime; }
	/// get consumable family id
	inline uint16 getConsumableFamilyId() const { return _ConsumableFamilyId; }

private:
	/// process prams (bricks or consumable items)
	virtual void processParams(const std::vector<TBrickParam::IIdPtr> &params, bool isConsumable, uint16 quality);

	/// apply all powers
	void applyPowers();

	/// acting entity
	TDataSetRow					_ActorRowId;	

	/// special power object
	std::vector<CSpecialPower*>	_Powers;

	/// affected entities
	std::vector<TDataSetRow>	_Targets;

	/// additional recast time
	NLMISC::TGameCycle			_AddRecastTime;

	/// flags to bypass some checks
	CBypassCheckFlags			_BypassCheckFlags;

	// consumable family Id
	uint16						_ConsumableFamilyId;
};

#endif // RYZOM_SPECIAL_POWER_PHRASE_H

/* End of special_power_phrase.h */
