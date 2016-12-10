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

#ifndef NL_ANIMATION_SET_USER_H
#define NL_ANIMATION_SET_USER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"

#include "nel/3d/u_animation_set.h"

#include "nel/3d/animation_set.h"


namespace NL3D
{


class	CPlayListManagerUser;
class	CDriverUser;


// ***************************************************************************
/**
 * UAnimationSet implementation
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CAnimationSetUser : public UAnimationSet
{
private:
	NLMISC::CSmartPtr<CAnimationSet>		_AnimationSet;
	CDriverUser								*_Owner;
	friend class CPlayListManagerUser;
	friend class CSceneUser;

public:

	/// Constructor
	CAnimationSetUser(CDriverUser *owner, bool headerOptim = true)
	{
		nlassert(owner);
		_Owner= owner;

		nlassert((uint)UAnimationSet::NotFound == (uint)CAnimationSet::NotFound );

		// create a smartptred animation set. Allow header compression
		_AnimationSet= new CAnimationSet(headerOptim);
	}

	/// Constructor
	CAnimationSetUser(CDriverUser *owner, NLMISC::IStream	&f)
	{
		nlassert(owner);
		_Owner= owner;

		nlassert((uint)UAnimationSet::NotFound == (uint)CAnimationSet::NotFound );

		// create a smartptred animation set. DO NOT Allow header compression, cause serial()
		_AnimationSet= new CAnimationSet(false);

		_AnimationSet->serial(f);
	}

	virtual void setAnimationSampleDivisor(uint sampleDivisor);

	virtual uint getAnimationSampleDivisor() const;

	/**
	  *  Add an animation in the animation set. After adding all your animations, call build().
	  *
	  * \param fileName is the animation filename
	  * \param animName is the name of the animation in the animation set.
	  * \return NotFound if the file is not found.
	  */
	uint addAnimation (const char* fileName, const char* animName, bool displayMissingFileWarning  = true)
	{
		// Allocate an animation
		CUniquePtr<CAnimation> anim (new CAnimation);

		// Read it
		NLMISC::CIFile file;
		std::string path = NLMISC::CPath::lookup (fileName, false, displayMissingFileWarning);
		if (path.empty())
			path = fileName;
		if ( file.open ( path ) )
		{
			// Serial the animation
			file.serial (*anim);

			// Add the animation
			uint id=_AnimationSet->addAnimation (animName, anim.release());

			// Return id
			return id;
		}
		else return UAnimationSet::NotFound;
	}

	/**
	  *  Build the animation set. Call build after adding all your animations.
	  */
	virtual	void build ();

	/**
	  *  Add a skeleton weight in the animation set.
	  *  This method use CPath to search the skeleton file.
	  *
	  * \param fileName is the skeleton weight filename
	  * \param animName is the name of the skeleton weight in the animation set.
	  * \return the id of the new skeleton or NotFound if the file is not found.
	  */
	virtual uint addSkeletonWeight (const char* fileName, const char* skelName)
	{
		// Allocate an animation
		CUniquePtr<CSkeletonWeight> skeletonWeight (new CSkeletonWeight);

		// Read it
		NLMISC::CIFile file;
		if (file.open ( NLMISC::CPath::lookup( fileName ) ) )
		{
			// Serial the animation
			file.serial (*skeletonWeight);

			// Add the animation
			uint id=_AnimationSet->addSkeletonWeight (skelName, skeletonWeight.release());

			// Return id
			return id;
		}
		else return UAnimationSet::NotFound;
	}

	/// \name Animations mgt.
	// @{
	/**
	  * Get animations count.
	  */
	virtual	uint getNumAnimation () const
	{
		return _AnimationSet->getNumAnimation();
	}

	/**
	  * Get an animation ID by name. If no animation is found, method returns NotFound.
	  */
	virtual	uint getAnimationIdByName (const std::string& name) const
	{
		return _AnimationSet->getAnimationIdByName(name);
	}

	/**
	  * Get animation name.
	  */
	virtual	const std::string& getAnimationName (uint animationId) const
	{
		if(animationId>=getNumAnimation())
			nlerror("getAnimation*(): bad animation Id");
		return _AnimationSet->getAnimationName(animationId);
	}

	/**
	  * Get a writable animation pointer.
	  *
	  * \return the end time.
	  */
	virtual UAnimation* getAnimation (uint animationId);

	// @}


	/// \name SkeletonWeight mgt.
	// @{
	/**
	  * Get skeleton weight count.
	  */
	virtual	uint getNumSkeletonWeight () const
	{
		return _AnimationSet->getNumSkeletonWeight();
	}

	/**
	  * Get a SkeletonWeight ID by name. If no SkeletonWeight is found, method returns NotFound.
	  */
	virtual	uint getSkeletonWeightIdByName (const std::string& name) const
	{
		return _AnimationSet->getSkeletonWeightIdByName(name);
	}

	/**
	  * Get skeleton template name.
	  */
	virtual	const std::string& getSkeletonWeightName (uint skeletonId) const
	{
		if(skeletonId>=getNumSkeletonWeight())
			nlerror("getSkeletonWeight*(): bad SkeletonWeight Id");
		return _AnimationSet->getSkeletonWeightName(skeletonId);
	}

	// @}


	/// \name Channel mgt.
	// @{

	virtual	uint getChannelIdByName (const std::string& name) const
	{
		return _AnimationSet->getChannelIdByName(name);
	}

	// @}

	// Access the animation set
	const CAnimationSet* getAnimationSet () const;
};

} // NL3D


#endif // NL_ANIMATION_SET_USER_H

/* End of animation_set_user.h */
