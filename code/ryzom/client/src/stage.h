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



#ifndef CL_STAGE_H
#define CL_STAGE_H

/////////////
// INCLUDE //
/////////////
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/stream.h"


///////////
// CLASS //
///////////
/**
 * Class to manage a character.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CStage : public NLMISC::IStreamable
{
public:
	typedef std::map<uint32, sint64>	TStage;

private:
	double				_Time;
	TStage				_Stage;
	NLMISC::TGameCycle	_PredictedInterval;
	sint32				_LCTImpact;	// 0..256

public:
	NLMISC_DECLARE_CLASS(CStage);

	/// Constructor.
	CStage();
	/// Return "true" is the stage is empty.
	bool empty() const {return _Stage.empty();}
	/**
	 * Method to know if a property is present in the Stage.
	 * \param prop : the property to check.
	 * \return bool : 'true' if the property is present in the Stage.
	 */
	bool isPresent(uint prop) const {return (_Stage.find(prop) != _Stage.end());}

	/**
	 * Add a property in the stage(and may replace the old one if wanted).
	 * \param property : property to add.
	 * \param value : value of the property.
	 * \param replace : 'true' to replace the old property if there is one.
	 */
	void addProperty(uint property, sint64 value, bool replace);

	/** Remove the selected property from the stage.
	 * \param property : property to remove from this stage.
	 */
	void removeProperty(uint property);

	/**
	 * Return a pair according to the property asked. First element (the bool), indicate if the value (second element) is valid.
	 * \param uint prop : property asked (to get its value).
	 * \return pair<bool, sint64> : '.first' == 'true' if the property exist so value is valid. '.second' is the value if '.first' == 'true'.
	 */
	std::pair<bool, sint64> property(uint prop) const;

	const TStage &getStage() const {return _Stage;}
	TStage &getStage() {return _Stage;}

	const double &time() const {return _Time;}
	void time(const double &t) {_Time = t;}

	/** \name PREDICTED INTERVAL
	 * The next position update should arrive before PredictedInterval.
	 */
	//@{
	/// Get the Predicted Interval.
	const NLMISC::TGameCycle &predictedInterval() const {return _PredictedInterval;}
	/// Set the Predicted Interval.
	void predictedInterval(const NLMISC::TGameCycle &pI) {if(pI != 0) _PredictedInterval = pI;}
	//@}

	/** Get the position in the stage or return false.
	 * \param pos : will be filled with the position or left untouched.
	 * \return bool : true if pos has been filled.
	 */
	bool getPos(NLMISC::CVectorD &pos) const;

	/** LCTImpact (0..256) (used by CCharacterCL::updateStages())
	 *	It is used to remove the LCT for stages without POS (with some restrictions).
	 *	To avoid pop or stages skipped, the LCT impact is smoothly reduced
	 *	with time (see CCharacterCL::updateStages())
	 */
	void	setLCTImpact(sint32 lcti) {_LCTImpact= lcti;}
	sint32	getLCTImpact() const {return _LCTImpact;}

	/// Serialize entities.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
};

/**
 * Class to manage a character.
 * \author Guillaume PUZIN
 * \author Nevrax France
 * \date 2001
 */
class CStageSet : public NLMISC::IStreamable
{
public:
	typedef std::map<NLMISC::TGameCycle, CStage>	TStageSet;
	TStageSet										_StageSet;

public:
	NLMISC_DECLARE_CLASS(CStageSet);

	/**
	 * Try to add a new property for the stage corresponding to the Game Cycle.
	 * May also add a new stage if there is no stage for the Game Cycle.
	 * \warning If the property already exist, this method does not change the value.
	 * \warning If the Game Cycle is before the first element, this method do not try to add a stage but will try to add a property to the first one.
	 * \param TGameCycle gameCycle : This is the Key for the stage to search where to add the property.
	 * \param uint property : property to add.
	 * \param uint64 value : value of the property.
	 * \param predictedI : predicted interval in tick before the next position update.
	 * \return CStage * : pointer on the stage or 0.
	 */
	CStage *addStage(NLMISC::TGameCycle gameCycle, uint property, sint64 value);
	CStage *addStage(NLMISC::TGameCycle gameCycle, uint property, sint64 value, NLMISC::TGameCycle predictedI);

	/// Serialize entities.
	virtual void serial(class NLMISC::IStream &f) throw(NLMISC::EStream);
	/// Remove Positions except for those with a mode.
	void removePosWithNoMode();
};



#endif // CL_STAGE_H

/* End of stage.h */
