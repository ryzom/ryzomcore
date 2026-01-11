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



#ifndef CL_ANIMATION_MISC_H
#define CL_ANIMATION_MISC_H

/////////////
// INCLUDE //
/////////////
// misc
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
// 3d
#include "nel/3d/u_animation_set.h"
#include "nel/3d/animation_time.h"
#include "nel/3d/u_animation.h"


///////////
// CLASS //
///////////
/**
 * <Class description>
 * \author Guillaume PUZIN (GUIGUI)
 * \author Nevrax France
 * \date 2002
 */
class CAnimationMisc
{
public:
	/// Constructor
	CAnimationMisc();

	/**
	 * Function to get the position in an animation at timeOffset.
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \param timeOffset : time for the interpolation.
	 * \param result : is the reference on the value to get the result (position).
	 * \return bool : true if the parameter result is valid.
	 */
	static bool interpolate(NL3D::UAnimationSet *animationSet, uint idAnim, double timeOffset, NLMISC::CVector &result);
	/**
	 * Function to get the rotation in an animation at timeOffset.
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \param timeOffset : time for the interpolation.
	 * \param result : is the reference on the value to get the result (rotation).
	 * \return bool : true if the parameter result is valid.
	 */
	static bool interpolate(NL3D::UAnimationSet *animationSet, uint idAnim, double timeOffset, NLMISC::CQuat &result);

	/**
	 * Return an animation length (in sec).
	 * \param animationSet : search the animation in this set.
	 * \param string animName : Animation Name.
	 * \return double : the length of the animation or 0 if any pb.
	 * \warning This Method is slower than the one with the animation Id instead of the animation Name.
	 */
	static double getAnimationLength(NL3D::UAnimationSet *animationSet, const std::string &animName);
	/**
	 * Return an animation length (in sec).
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \return double : the length of the animation or 0 if any pb.
	 */
	static double getAnimationLength(NL3D::UAnimationSet *animationSet, uint idAnim);

	/**
 	 * Get the average speed of an animation (in meters/sec).
	 * \param animationSet : search the animation in this set.
	 * \param string animName : Animation Name.
	 * \return double : the average speed (in m/s).
	 */
	static double getAnimationAverageSpeed(NL3D::UAnimationSet *animationSet, const std::string &animName);
	/**
 	 * Get the average speed of an animation (in meters/sec).
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \return double : the average speed (in m/s).
	 */
	static double getAnimationAverageSpeed(NL3D::UAnimationSet *animationSet, uint idAnim);

	/**
 	 * Get the rotation done by the animation (in radian).
	 * \param animationSet : search the animation in this set.
	 * \param string animName : Animation Name.
	 * \return double : the rotation (in radian).
	 */
	static double getAnimationRotation(NL3D::UAnimationSet *animationSet, const std::string &animName);
	/**
 	 * Get the rotation done by the animation (in radian).
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \return double : the rotation (in radian).
	 */
	static double getAnimationRotation(NL3D::UAnimationSet *animationSet, uint idAnim);

	/** // Get the vector covered by the given animation.
	 * \param animationSet : search the animation in this set.
	 * \param idAnim : id of the animation.
	 * \param move : filled with the vector of the move if possible
	 * \return bool : true if 'move' has been filled, else false.
	 */
	static bool getAnimationMove(NL3D::UAnimationSet *animationSet, uint idAnim, NLMISC::CVector &move);
};


#endif // CL_ANIMATION_MISC_H

/* End of animation_misc.h */
