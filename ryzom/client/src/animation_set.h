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



#ifndef CL_ANIMATION_SET_H
#define CL_ANIMATION_SET_H

/////////////
// INCLUDE //
/////////////
// Misc
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
// Client
#include "animation_type.h"
#include "animation_state.h"
// std
#include <map>


///////////
// CLASS //
///////////

/**
 * Class to manage an animation set.
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CAnimationSet
{
private:
	double _MaxDist;
	double _Angle;
	double _AboutFaceAngle;
	double _SpeedToRun;
	double _SpeedToWalk;
	double _WalkDist;
	double _RunDist;
	double _WalkLength;
	double _RunLength;

private:

	/// AnimationState cache
	std::vector<CAnimationState>	_AnimationStates;
	CAnimationSetSheet				*_Sheet;

public:
	/// Constructor
	CAnimationSet();

	/**
	 * Build the animation set according to the georges file. (read in the sheet manager)
	 */
	void init(CAnimationSetSheet *sheet, NL3D::UAnimationSet *animationSet);

	/**
	 * Return the distance after the one you have to break the idle.
	 * \return double : this distance.
	 */
	double maxDist() const {return _MaxDist;}
	/**
	 * Return the distance after the one you have to break the idle.
	 * \return double : this distance.
	 */
	double angle() const {return _Angle;}
	/**
	 * Return the distance after the one you have to break the idle.
	 * \return double : this distance.
	 */
	double aboutFaceAngle() const {return _AboutFaceAngle;}
	/**
	 * When in walk animation, after this speed the animation should switch in run mode.
	 * \return double : a speed.
	 */
	double speedToRun() const {return _SpeedToRun;}
	/**
	 * When in run animation, before this speed the animation should switch in walk mode.
	 * \return double : a speed.
	 */
	double speedToWalk() const {return _SpeedToWalk;}

	/// Return the animation state from its Id. Your id must be valid, no check for optimal performances.
	const	CAnimationState *getAnimationState(const TAnimStateId& stateId) const
	{
		if ((uint)stateId<_AnimationStates.size ())
			return &(_AnimationStates[stateId]);
		else
			return NULL;
	}

	// get number of animation states
	uint	getNumAnimationState() const { return (uint)_AnimationStates.size(); }
	// get an animation state by its index
	CAnimationState *getAnimationStateByIndex(uint index);

	/// Return the covered distance by 1 walking animation.
	double walkDist() const {return _WalkDist;}
	/// Return the covered distance by 1 running animation.
	double runDist() const {return _RunDist;}
	/// Return the length (in sec) of the walking animation.
	double walkLength() const {return _WalkLength;}
	/// Return the length (in sec) of the running animation.
	double runLength() const {return _RunLength;}

	// get the sheet name
	std::string getSheetName() const
	{
		std::string name;
		if(_Sheet) return _Sheet->Name;
		return name;
	}
};


#endif // CL_ANIMATION_SET_H

/* End of animation_set.h */
