// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef NL_U_ANIMATION_SET_H
#define NL_U_ANIMATION_SET_H

#include "nel/misc/types_nl.h"
#include "animation_time.h"


namespace NL3D
{

class UAnimation;

// ***************************************************************************
/**
 * An animation Set is a set of animation, loaded from file. It contains a set of Skeleton TempalteWeight too.
 *
 *	NB: If the animation set is created with addAnimation() method (not with createAnimationSet(animsetFile))
 *	header optimisation is allowed => less memory load.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class UAnimationSet
{
protected:

	/// Constructor
	UAnimationSet() {}
	/// Constructor
	virtual	~UAnimationSet() {}

public:

	enum { NotFound=0xffffffff };

	/// \name Set build.
	// @{
	/** Set the animation Set in "Low Memory" mode by skipping some keys
	  * Each added animation will loose some keys for CTrackSampledQuat and CTrackSampledVector
	  *	\param sampleDivisor if set to 5 for instance, the number of keys will be divided (ideally) by 5.
	  *		if 0, set to 1. if 1 => no key skip (default to 1)
	  */
	virtual void setAnimationSampleDivisor(uint sampleDivisor) =0;

	/** see setAnimationSampleDivisor
	  */
	virtual uint getAnimationSampleDivisor() const =0;

	/**
	  *  Add an animation in the animation set. After adding all your animations, call build().
	  *  This method use CPath to search the animation file.
	  *	 WARNING: it assert if you call addAnimation() after build()
	  *
	  * \param fileName is the animation filename
	  * \param animName is the name of the animation in the animation set.
	  * \param displayMissingFileWarning A warning will be displayed in the log if the anim file is not found
	  * \return the id of the new animation or NotFound if the file is not found.
	  */
	virtual	uint addAnimation (const char* fileName, const char* animName, bool displayMissingFileWarning  = true) =0;

	/**
	  *  Build the animation set. Call build after adding all your animations.
	  *	 NoOp if already built
	  */
	virtual	void build () =0;

	/**
	  *  Add a skeleton weight in the animation set.
	  *  This method use CPath to search the skeleton file.
	  *
	  * \param fileName is the skeleton weight filename
	  * \param skelName is the name of the skeleton weight in the animation set.
	  * \return the id of the new skeleton or NotFound if the file is not found.
	  */
	virtual	uint addSkeletonWeight (const char* fileName, const char* skelName) =0;

	// @}

	/// \name Animations mgt.
	// @{
	/**
	  * Get animations count.
	  */
	virtual	uint getNumAnimation () const =0;

	/**
	  * Get a animation ID by name. If no animation is found, method returns NotFound.
	  */
	virtual	uint getAnimationIdByName (const std::string& name) const  =0;

	/**
	  * Get animation name.
	  */
	virtual	const std::string& getAnimationName (uint animationId) const =0;

	/**
	  * Get a writable animation pointer.
	  *
	  * \return the end time.
	  */
	virtual UAnimation *getAnimation (uint animationId) =0;

	// @}


	/// \name SkeletonWeight mgt.
	// @{
	/**
	  * Get skeleton weight count.
	  */
	virtual	uint getNumSkeletonWeight () const =0;

	/**
	  * Get a SkeletonWeight ID by name. If no SkeletonWeight is found, method returns NotFound.
	  */
	virtual	uint getSkeletonWeightIdByName (const std::string& name) const =0;

	/**
	  * Get skeleton template name.
	  */
	virtual	const std::string& getSkeletonWeightName (uint skeletonId) const =0;
	// @}


	/// \name Channel mgt.
	// @{

	/**
	  * Get a channel ID with its name. If no channel is found, method returns NotFound.
	  */
	virtual	uint getChannelIdByName (const std::string& name) const =0;

	// @}


};


} // NL3D


#endif // NL_U_ANIMATION_SET_H

/* End of u_animation_set.h */
