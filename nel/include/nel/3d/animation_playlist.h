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

#ifndef NL_ANIMATION_PLAYLIST_H
#define NL_ANIMATION_PLAYLIST_H

#include "nel/misc/types_nl.h"
#include "nel/3d/channel_mixer.h"


namespace NL3D
{


/**
 * This class
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationPlaylist : public NLMISC::CRefCount
{
public:
	enum
	{
		//
		empty=0xffffffff
	};

	/// Wrap mode for the play list
	enum TWrapMode
	{
		/// Clamp the animation time. (default)
		Clamp=0,

		/// Repeat the animation.
		Repeat,

		/// Disable the animation.
		Disable,

		WrapModeCount
	};

	/// Constructor
	CAnimationPlaylist();

	/**
	  * Empty the playlist. Each slot is set to its default value.
	  */
	void emptyPlayList ();

	/**
	  * Set the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param animation is the animation number to use in this slot. To empty the slot, use CAnimationPlaylist::empty.
	  */
	void setAnimation (uint8 slot, uint animation);

	/**
	  * Get the animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the animation number in use in this slot. Return CAnimationPlaylist::empty if the slot is empty.
	  */
	uint getAnimation (uint8 slot) const;

	/**
	  * Set the skeleton weight animation of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param skeletonId is the skeleton weight number to use in this slot. To empty the slot, use CAnimationPlaylist::empty.
	  * \param inverted is false if the weights must be used as they are, true if the weights to used are 1.f - weight.
	  */
	void setSkeletonWeight (uint8 slot, uint skeletonId, bool inverted=false);

	/**
	  * Get the skeleton weight of a slot. Default value is empty.
	  *
	  * \param slot is the id of the slot to set.
	  * \param inverted will receive the invert flag.
	  * \return the skeleton weight number in use in this slot. Return CAnimationPlaylist::empty if the slot is empty.
	  */
	uint getSkeletonWeight (uint8 slot, bool &inverted) const;

	/**
	  * Set animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param timeOrigin time origin to use in the slot.
	  */
	void setTimeOrigin (uint8 slot, TGlobalAnimationTime timeOrigin);

	/**
	  * Get animation time origin, ie, the time in the playlist for which slot time is the startTime of the slot animation.
	  * Default value is 0.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return time origin used in the slot.
	  */
	TGlobalAnimationTime getTimeOrigin (uint8 slot) const;

	/**
	  * Set animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param speedFactor is the factor to use in this slot. The animation will go speedFactor* faster.
	  */
	void setSpeedFactor (uint8 slot, float speedFactor);

	/**
	  * Get animation speed factor.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \return the speed factor used in the slot.
	  */
	float getSpeedFactor (uint8 slot) const;

	/**
	  * Set animation start weight. This is the weight for this animation use at the beginning of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param startWeight is the factor to use in this slot.
	  * \param time is the playlist time for which the start weight is set. Default is 0.f.
	  */
	void setStartWeight (uint8 slot, float startWeight, TGlobalAnimationTime time);

	/**
	  * Get animation start weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default time is 0.f.
	  * \return the start weight used in the slot.
	  */
	float getStartWeight (uint8 slot, TGlobalAnimationTime& time) const;

	/**
	  * Set animation end weight. This is the weight for this animation use at the end of the animation slot.
	  * Default value is 1.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param time is the playlist time for which the end weight is set. Default time is 0.f.
	  * \param endWeight is the factor to use in this slot.
	  */
	void setEndWeight (uint8 slot, float endWeight, TGlobalAnimationTime time);

	/**
	  * Get animation end weight.
	  * Default value is 1.f.
	  *
	  * \param slot is the id to get the slot.
	  * \param time will get the playlist time for which the start weight is set. Default is 0.f.
	  * \return the end weight used in the slot.
	  */
	float getEndWeight (uint8 slot, TGlobalAnimationTime& time) const;

	/**
	  * Set weight smoothness.
	  * This is the smoothness of the weight interpolation.
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \param smoothness is the smoothness value.
	  */
	void setWeightSmoothness (uint8 slot, float smoothness);

	/**
	  * Get weight smoothness.
	  * This is the smoothness of the weight interpolation.
	  * Must be between 0.f and 1.f. 0.f is a sharp interpolation (linear), 1.f is a smooth interpolation (quadratic).
	  * Default value is 0.f.
	  *
	  * \param slot is the id of the slot to set.
	  * \return the smoothness value.
	  */
	float getWeightSmoothness (uint8 slot) const;

	/**
	  * Setup a channel mixer.
	  *
	  * For each slot, it sets :
	  * * the animation id used by the slot.
	  * * the animation time according with the begin time of each slot, the speed factor of each slot and the current time passed in parameter.
	  * * the skeleton weight id.
	  * * the weight interpolated with the start and end values. The weight value are clamped before and after the animation.
	  */
	void setupMixer (CChannelMixer& mixer, TGlobalAnimationTime time) const;

	/**
	  * Static interpolation method of blend value
	  *
	  * \param startBlendTime is the time when the blend starts.
	  * \param endBlendTime is the time when the blend ends.
	  * \param time is the current time.
	  * \param startBlend is the blend value at start.
	  * \param endBlend is the blend value at end.
	  * \param smoothness is the smoothnes value. (Must be between 0 and 1)
	  */
	static float getWeightValue (TGlobalAnimationTime startWeightTime, TGlobalAnimationTime endWeightTime, TGlobalAnimationTime time, float startWeight, float endWeight, float smoothness);

	/**
	  * Set the wrap mode to use in the play list.
	  *
	  * Wrap mode tells the play list how to use an animation when current time is not in the animation.
	  * \param wrapMode is the mode to use.
	  */
	void setWrapMode (uint8 slot, TWrapMode wrapMode);

	/**
	  * Get the wrap mode in use in the play list.
	  *
	  * Wrap mode tells the play list how to use an animation when current time is not in the animation.
	  * \return the wrap mode in use.
	  */
	TWrapMode getWrapMode (uint8 slot) const;

	/**
	  * Convert a playlist global time in local time in a slot animation including wrap evaluation.
	  *
	  * \param slot is the slot in which the local time must be computed
	  * \param time is the global time of the playlist
	  *
	  * \return the local time in the slot. If no animation are set in the slot, globalTime is returned.
	  */
	TAnimationTime getLocalTime (uint8 slot, TGlobalAnimationTime globalTime, const CAnimationSet& animSet) const;

	/**
	  * Compute weight of a slot at a given global playlist time
	  *
	  * \param slot is the slot in which the weight must be computed
	  * \param time is the global time of the playlist
	  *
	  * \return the weight of the slot for the given time.
	  */
	float getLocalWeight (uint8 slot, TGlobalAnimationTime globalTime) const;

	/**
	  * Serial
	  */
	void serial (NLMISC::IStream& f);

private:

	// Animation in slot
	uint32				_Animations[CChannelMixer::NumAnimationSlot];

	// Skeleton weight in slot
	uint32				_SkeletonWeight[CChannelMixer::NumAnimationSlot];

	// Invert Skeleton weight in slot
	bool				_InvertWeight[CChannelMixer::NumAnimationSlot];

	// Time origin in slot
	TGlobalAnimationTime	_TimeOrigin[CChannelMixer::NumAnimationSlot];

	// Speed factor in slot
	float				_SpeedFactor[CChannelMixer::NumAnimationSlot];

	// Start weight
	float				_StartWeight[CChannelMixer::NumAnimationSlot];

	// Start weight time
	TGlobalAnimationTime	_StartWeightTime[CChannelMixer::NumAnimationSlot];

	// End weight
	float				_EndWeight[CChannelMixer::NumAnimationSlot];

	// End weight time
	TGlobalAnimationTime	_EndWeightTime[CChannelMixer::NumAnimationSlot];

	// Smoothness of the weight interpolation
	float				_Smoothness[CChannelMixer::NumAnimationSlot];

	// Wrap mode
	TWrapMode			_WrapMode[CChannelMixer::NumAnimationSlot];
};


} // NL3D


#endif // NL_ANIMATION_PLAYLIST_H

/* End of animation_playlist.h */
