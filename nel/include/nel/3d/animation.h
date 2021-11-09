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

#ifndef NL_ANIMATION_H
#define NL_ANIMATION_H

#include "nel/misc/types_nl.h"

#include "nel/3d/animation_time.h"
#include "nel/3d/u_animation.h"

#include <memory>
#include <map>
#include <set>
#include <vector>

namespace NLMISC
{
class IStream;
struct EStream;
}

namespace NL3D
{

class ITrack;
class CAnimationSet;

/**
 * This class describes animations for several tracks. Each track works
 * with a IAnimatedValue for the IAnimatable objects.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimation : public UAnimation
{
public:
	/// ctor
	CAnimation();
	/// Destructor
	virtual ~CAnimation ();

	/// \name Public interface.

	enum { NotFound=0xffffffff };

	/** Get track with its name.
	  *
	  *	WARNING: slower if applyAnimHeaderCompression() has been called. try use getIdTrackByChannelId() instead
	  *
	  * \param name is the name of the desired track.
	  * \return CAnimation::NotFound if the track doesn't exist else the id of the track.
	  */
	uint getIdTrackByName (const std::string& name) const;

	/**
	  * Fill the set of string with the name of the channels.
	  *
	  * \return the count of track in this animation
	  */
	void getTrackNames (std::set<std::string>& setString) const;

	/** see applyAnimHeaderCompression()
	  * \return CAnimation::NotFound if the track doesn't exist (or anim header not compressed)
	  *		else return the id of the track.
	  */
	uint getIdTrackByChannelId (uint16 channelId) const;

	/** Get a const track pointer
	  *
	  * \param channelId is the id of the desired channel.
	  */
	const ITrack* getTrack (uint trackId) const
	{
		// Get the trackId-th track pointer
		return _TrackVector[trackId];
	}

	/** Get a track pointer
	  *
	  * \param channelId is the id of the desired channel.
	  */
	ITrack* getTrack (uint trackId)
	{
		// Get the trackId-th track pointer
		return _TrackVector[trackId];
	}

	/** Add a track at the end of the track list.
	  *
	  * This method is used to insert tracks in the animation.
	  * Tracks must be allocated with new. The pointer is then handeled
	  * by the CAnimation.
	  */
	void addTrack (const std::string& name, ITrack* pChannel);

	/// Serial the template
	void serial (NLMISC::IStream& f);

	/// Set animation min end time
	void setMinEndTime (TAnimationTime minEndTime);

	/// \name From UAnimation
	// @{

	virtual UTrack*		getTrackByName (const char* name);
	virtual void		releaseTrack (UTrack* track);
	virtual TAnimationTime	getBeginTime () const;
	virtual TAnimationTime	getEndTime () const;
	virtual	bool			allTrackLoop() const;

	// @}


	/** For SkeletonSpawnScript (SSS) animation.
	 *	Add manualy the shapes that can be spawned by the channel "spawn_script" in this animation
	 *	This add into a vector (avoid duplicates)
	 *
	 *	Then CAnimationSet::preloadSSSShapes() can be used after CAnimationSet::build() to force loading
	 *	into the ShapeBank/Texture of thoses shapes, so there is no problem of shape loading during animation
	 */
	void							addSSSShape(const std::string &shape);
	const std::vector<std::string>	&getSSSShapes() const {return _SSSShapes;}


	/// \name CAnimationSet private
	// @{
	/** For each track that support it (CTrackSampled for instance), divide its number of sampled keys,
	  * to lower the memory lod. Used typically by CAnimationSet
	  */
	void	applySampleDivisor(uint sampleDivisor);

	/** For CTrackSampledQuat only, compress header. Used typically by CAnimationSet
	  * NB: Animation cannot be serialized after this operation (unserialisable tracks)
	  */
	void	applyTrackQuatHeaderCompression();

	/** Used by CAnimationSet to lower the memory Size. After this, you can
	  *	(and should for better performances) use getIdTrackByChannelId()
	  *	Does not support more than 65536 channels (nlassert)
	  */
	void	applyAnimHeaderCompression(CAnimationSet *animationSetOwner, const std::map <std::string, uint32> &channelMap);

	// @}


private:
	/// \name Members
	typedef std::map<std::string, uint32> TMapStringUInt;
	typedef std::vector<ITrack* > TVectAPtrTrack;

	// Animation name
	std::string			_Name;

	// Map to get a channel id with a name. EMPTY if applyAnimHeaderCompression() called
	TMapStringUInt		_IdByName;

	// Vector of channel pointer.
	TVectAPtrTrack		_TrackVector;

	// Force animation min end time
	TAnimationTime		_MinEndTime;

	/// \name Anim time caching
	// @{
		mutable TAnimationTime		_BeginTime;
		mutable TAnimationTime		_EndTime;
		mutable bool				_AnimLoop;
		mutable bool				_BeginTimeTouched;
		mutable bool				_EndTimeTouched;
		mutable bool				_AnimLoopTouched;
	// @}

	/// CTrackSampledQuat header compression
	class CTrackSamplePack			*_TrackSamplePack;

	// Sorted array of ChannelId. Same size as _TrackVector. EMPTY if applyAnimHeaderCompression() NOT called
	std::vector<uint16>	_IdByChannelId;
	// The AnimationSet. NULL if applyAnimHeaderCompression() NOT called
	class CAnimationSet	*_AnimationSetOwner;

	// see addSSSShape()
	std::vector<std::string>		_SSSShapes;

};


} // NL3D


#endif // NL_ANIMATION_H

/* End of animation.h */
