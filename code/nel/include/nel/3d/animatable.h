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

#ifndef NL_ANIMATABLE_H
#define NL_ANIMATABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/smart_ptr.h"

#include <string>
#include <vector>
#include <map>


namespace NL3D
{

class ITrack;
class CChannelMixer;
class IAnimatedValue;

/**
 * An animatable object.
 *
 * This object can have a set of animated values. At Max 32 animated values can be set (because of bit and touch mgt)
 * Animated values are animated by a CChannelMixer object.
 * Each value have a name and a default track.
 *
 * An IAnimatable may have IAnimatable sons (list of bones, list of materials etc...). The value count and valueId of
 * the IAnimatable DO NOT count those sons, but register() should register his sons too.
 * A father propagated touch system (setFather()) is implemented. When a son is touched, he touchs his fathers, his grandfather
 * and so on.
 *
 * When a class derives from IAnimatable, it must implement all the
 * interface's methods:
 *
 *	extend TAnimValues enum, beginning to BaseClass::AnimValueLast, and add a bit OwnerBit.
 *	ctor(): just type "IAnimatable::resize (AnimValueLast);"
 *	virtual IAnimatedValue* getValue (uint valueId);
 *	virtual const char *getValueName (uint valueId) const;
 *	virtual ITrack* getDefaultTrack (uint valueId);
 *
 *	virtual register(CChannelMixer *, const string &prefix);
 *
 *
 * Watch NL3D::ITransformable and NL3D::CTransform for a good example.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatable : public NLMISC::CRefCount
{
	friend class IAnimatedValue;
public:

	/**
	  * Default Constructor. Set number of value to 0.
	  * Deriver: should just write:  IAnimatable::resize (getValueCount());
	  *
	  */
	IAnimatable ()
	{
		_Father= NULL;
		_BitSet= 0;
	}

	virtual ~IAnimatable() {}
	/// \name Interface
	// @{
	/**
	  * The enum of animated values. (same system in CMOT). Deriver should extend this enum, beginning with OwnerBit= BaseClass::AnimValueLast.
	  *	The number of values MUST NOT EXCEED 32, for fast touch() system.
	  * "OwnerBit" system: each deriver of IAnimatable should had an entry "OwnerBit" in this TAnimValues. This bit will be set when
	  * an IAnimatedValue of this deriver part is touched, or if one of his IAnimatable sons is touched (see setFather()).
	  */
	enum	TAnimValues
	{
		AnimValueLast=0,
	};

	/**
	  * Get a value pointer.
	  *
	  * \param valueId is the animated value ID in the object. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return The pointer on the animated value.
	  */
	virtual IAnimatedValue* getValue (uint valueId) =0;

	/**
	  * Get animated value name.
	  *
	  * \param valueId is the animated value ID in the object we want the name. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return the name of the animated value.
	  */
	virtual const char *getValueName (uint valueId) const =0;

	/**
	  * Get default track pointer.
	  *
	  * \param valueId is the animated value ID in the object we want the default track. IGNORING IANIMATABLE SONS (eg: bones, materials...).
	  * \return The pointer on the default track of the value.
	  */
	virtual ITrack* getDefaultTrack (uint valueId) =0;

	/**
	  * register the Animatable to a channelMixer (using CChannelMixer::addChannel()). You MUST use this method to register Animatable.
	  * This method should:
	  *		- call is BaseClass method.
	  *		- register local AnimatableValues, with channel name:	prefix+getValueName().
	  *		- register local sons!!. eg: matlist[0]->registerToChannelMixer(chanMixer, prefix+"mat0.").
	  *
	  * \param chanMixer is the channel mixer. Should not be NULL. for anim detail purpose , the IAnimatable may store a RefPtr on this channel mixer.
	  * \param prefix prefix to be append to valueNames
	  */
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix=std::string()) =0;

	// @}



	/// \name Touch flags management
	// @{

	/**
	  * Say which (if any) IAnimatable owns this one. This is important for Touch propagation.
	  * By this system, Fathers and ancestors know if they must check their sons (isTouched() return true).
	  *
	  * \param father the father we must inform of our update.
	  * \param fatherOwnerBit What bit of father we must set when we are updated
	  */
	void	setFather(IAnimatable *father, uint fatherOwnerBit)
	{
		_Father= father; _FatherOwnerBit= fatherOwnerBit;

		// propagate the touch to the fathers.
		propagateTouch();
	}


	/**
	  * Touch a value because it has been modified.
	  *
	  * \param valueId is the animated value ID in the object we want to touch.
	  * \param ownerValueId is the bit of the IAnimatable part which owns this animated value.
	  */
	void touch (uint valueId, uint ownerValueId)
	{
		// Set the bit
		setFlag(valueId);
		// Set the owner bit
		setFlag(ownerValueId);

		// propagate the touch to the fathers.
		propagateTouch();
	}

	/**
	  * Return non 0 int if the value as been touched else 0.
	  *
	  * \param valueId is the animated value ID in the object we want to test the touch flag. or it may be an OwnerBit.
	  */
	uint32 isTouched (uint valueId) const
	{
		return _BitSet&(1<<valueId);
	}


	/**
	  * Change value count, bit are set to 0
	  *
	  * \param count is the new value count.
	  */
	void resize (uint count)
	{
		// with the "uint32 _BitSet" implementation, juste check the size is correct
		nlassert(count<=32);
	}
	// @}


private:

	// Use a uint32 to manage the flags
	uint32			_BitSet;
	// The owner of this IAnimatable.
	IAnimatable		*_Father;
	// What bit of father which must set when we are updated.
	uint			_FatherOwnerBit;

	void	propagateTouch()
	{
		IAnimatable		*pCur= this;
		// Stop when no father, or when father is already touched (and so the grandfather...!!!).
		while(pCur->_Father && !pCur->_Father->isTouched(_FatherOwnerBit))
		{
			// The Owner bit is the "something is touched" flag. touch it.
			pCur->_Father->setFlag(pCur->_FatherOwnerBit);
			pCur= pCur->_Father;
		}
	}


protected:
	/** This is a tool function which add a given value to a channel.
	  * \return -1 if the track was not found in the animationSet, else it return the channelId
	  *	as if returned by CAnimationSet::getChannelIdByName(channelName).
	  */
	sint	addValue(CChannelMixer *chanMixer, uint valueId, uint ownerValueId, const std::string &prefix, bool detail);

	/// This method clear a bit in the bitset.
	void	clearFlag(uint valueId)
	{
		_BitSet&= ~(1<<valueId);
	}

	/// This method set a bit in the bitset.
	void	setFlag(uint valueId)
	{
		_BitSet|= (1<<valueId);
	}

};


} // NL3D

#endif // NL_ANIMATABLE_H

/* End of animatable.h */
