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

#ifndef NL_ANIMATION_SET_H
#define NL_ANIMATION_SET_H

#include "nel/misc/types_nl.h"
#include "nel/3d/animation.h"
#include "nel/3d/skeleton_weight.h"
#include "nel/misc/smart_ptr.h"
#include <map>
#include <string>
#include <vector>

namespace NLMISC
{
class IStream;
struct EStream;
}

namespace NL3D
{

class IDriver;
class CShapeBank;

/**
 * An CAnimationSet is set of CAnimation. It manages global channel ID for a set of
 * animations with many tracks. An Id is associed with each track name.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationSet : public NLMISC::CRefCount
{
public:

	enum { NotFound=0xffffffff };

	/**
	  * \param headerOptim if true, the animationSet cannot be serialized (nlassert), but could save
	  *	lot of memory load: CTrackSampleQuat memory are optimized (48 bytes to 12) and map<string, id>
	  *	per animation is replaced with a (IdChannelInAnimSet, IdTrackInAnim) association
	  */
	CAnimationSet (bool headerOptim= false);
	~CAnimationSet ();

	/**
	  * Get channel ID count. This number is the count of different track name in the animation set.
	  */
	uint getNumChannelId () const;

	/** Get a channel Name by its Id.
	  */
	const std::string &getChannelName(uint channelId) const
	{
		nlassert(channelId<_ChannelName.size());
		return _ChannelName[channelId];
	}

	/**
	  * Get a channel ID with its name. If no channel is found, method returns NotFound.
	  */
	uint getChannelIdByName (const std::string& name) const
	{
		// Look for an id with this name
		std::map <std::string, uint32>::const_iterator ite=_ChannelIdByName.find (name);
		if (ite!=_ChannelIdByName.end ())
			return ite->second;
		else
			return NotFound;
	}

	/**
	  * Get a animation ID by name. If no channel is found, method returns NotFound.
	  */
	uint getAnimationIdByName (const std::string& name) const
	{
		// Look for an id with this name
		std::map <std::string, uint32>::const_iterator ite=_AnimationIdByName.find (name);
		if (ite!=_AnimationIdByName.end ())
			return ite->second;
		else
			return NotFound;
	}

	/**
	  * Get animations count.
	  */
	uint getNumAnimation () const
	{
		return (uint)_Animation.size();
	}

	/**
	  * Get animation name.
	  */
	const std::string& getAnimationName (uint animationId) const
	{
		nlassert(animationId<_AnimationName.size());
		return _AnimationName[animationId];
	}

	/**
	  * Get a read only animation pointer.
	  */
	const CAnimation* getAnimation (uint animationId) const
	{
		nlassert(animationId<_Animation.size());
		return _Animation[animationId];
	}

	/**
	  * Get a writable animation pointer.
	  */
	CAnimation* getAnimation (uint animationId)
	{
		nlassert(animationId<_Animation.size());
		return _Animation[animationId];
	}

	/**
	  * Get skeleton weight count.
	  */
	uint getNumSkeletonWeight () const
	{
		return (uint)_SkeletonWeight.size();
	}

	/**
	  * Get a skeleton weight ID by name. If no skeleton weight is found, method returns NotFound.
	  */
	uint getSkeletonWeightIdByName (const std::string& name) const
	{
		// Look for an id with this name
		std::map <std::string, uint32>::const_iterator ite=_SkeletonWeightIdByName.find (name);
		if (ite!=_SkeletonWeightIdByName.end ())
			return ite->second;
		else
			return NotFound;
	}

	/**
	  * Get skeleton template name.
	  */
	const std::string& getSkeletonWeightName (uint skeletonId) const
	{
		nlassert(skeletonId<_SkeletonWeightName.size());
		return _SkeletonWeightName[skeletonId];
	}

	/**
	  * Get a read only skeleton weight pointer.
	  */
	const CSkeletonWeight* getSkeletonWeight (uint skeletonId) const
	{
		nlassert(skeletonId<_SkeletonWeight.size());
		return _SkeletonWeight[skeletonId];
	}

	/**
	  * Get a writable skeleton weight pointer.
	  */
	CSkeletonWeight* getSkeletonWeight (uint skeletonId)
	{
		nlassert(skeletonId<_SkeletonWeight.size());
		return _SkeletonWeight[skeletonId];
	}

	/**
	  * Add an animation to the set. The pointer of the animation must be allocated with new.
	  * It is then handled by the animation set.
	  *
	  *	WARNING: it assert if you call addAnimation() after build(), while the animation set is in HeadOptim mode
	  *
	  * \param name is the name of the animation.
	  * \param animation is the animation pointer.
	  * \return the id of the new animation.
	  */
	uint addAnimation (const char* name, CAnimation* animation);

	/**
	  * Add a skeleton weight to the set. The pointer of the skeletonWeight must be allocated with new.
	  * It is then handled by the animation set.
	  *
	  * \return the id of the new skeleton.
	  */
	uint addSkeletonWeight (const char* name, CSkeletonWeight* skeletonWeight);

	/**
	  * Reset the animation set.
	  */
	void reset ();

	/**
	  * Final build of the animation set.
	  *
	  * First, for each animation you want to add to the set, you must add the animation in the set.
	  *
	  * When all animations are built, call this method to finalize the set.
	  *
	  * NoOp if already built
	  */
	void build ();

	/// Serial the template
	void serial (NLMISC::IStream& f);

	/**
	  * Helper method.
	  * Load an animation set from animation files in a directory, then call build().
	  * \param path Path to look at for animations
	  * \param recurse whether to recurse to load animation in sub folders
	  * \param ext the extension for animation files
	  * \param wantWarningMessage displays warning if some of the files could not be loaded
	  * \return true if everything loaded ok
	  */
	bool			loadFromFiles
					(
						const std::string &path,
						bool recurse = true,
						const char *ext = "anim",
						bool wantWarningMessage = true
					);

	/** Set the animation Set in "Low Memory" mode by skipping some keys
	  * Each added animation will loose some keys for CTrackSampledQuat and CTrackSampledVector
	  *	\param sampleDivisor if set to 5 for instance, the number of keys will be divided (ideally) by 5.
	  *		if 0, set to 1. if 1 => no key skip (default to 1)
	  */
	void setAnimationSampleDivisor(uint sampleDivisor);

	/** see setAnimationSampleDivisor
	  */
	uint getAnimationSampleDivisor() const;

	/// see CAnimationSet ctor
	bool	isAnimHeaderOptimized() const {return _AnimHeaderOptimisation;}

	/** For SkeletonSpawnScript (SSS) animation.
	 *	call this after build(). This method preload ALL the shapes/texture
	 *	that can be spawned due to any animation
	 */
	void	preloadSSSShapes(IDriver &drv, CShapeBank &shapeBank);


private:
	std::vector <CAnimation*>		_Animation;
	std::vector <CSkeletonWeight*>	_SkeletonWeight;
	std::vector <std::string>		_ChannelName;
	std::vector <std::string>		_AnimationName;
	std::vector <std::string>		_SkeletonWeightName;
	std::map <std::string, uint32>	_ChannelIdByName;
	std::map <std::string, uint32>	_AnimationIdByName;
	std::map <std::string, uint32>	_SkeletonWeightIdByName;
	uint							_SampleDivisor;
	bool							_AnimHeaderOptimisation;
	bool							_Built;
	/// All Shapes that can be spawned in any animation
	std::set<std::string>			_SSSShapes;


	void	buildChannelNameFromMap();

};


} // NL3D


#endif // NL_ANIMATION_SET_H

/* End of animation_set.h */
