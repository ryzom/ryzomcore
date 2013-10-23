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

#ifndef NL_ANIMATED_VALUE_H
#define NL_ANIMATED_VALUE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/rgba.h"


namespace NL3D
{


/**
 * A value handled by the animation system.
 * This value must be managed by a IAnimatable object.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class IAnimatedValue
{
public:
	virtual ~IAnimatedValue() {}

	/**
	  * The blend method. This method blend two animated values and store the result
	  * in the object. The two first args can be reference on the object itself.
	  * Idealy, it performs the operation this->value=(this->value*blendFactor + value*(1.f-blendFactor))
	  *
	  * \param value is the first value in the blend operation.
	  * \param blendFactor must be in the range [0..1].
	  */
	virtual void blend (const IAnimatedValue& value, float blendFactor) =0;

	/**
	  * An assignation method. This method assign a value to the object.
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value) =0;
};


/**
 * A template implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueBlendable : public IAnimatedValue
{
public:
	// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueBlendable<T>	*pValue=(CAnimatedValueBlendable<T>*)&value;

		// Blend
		Value=(T) (Value*blendFactor+pValue->Value*(1.f-blendFactor));
	}

	/**
	  * An assignation method. This method assign a value to the object.
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueBlendable<T>	*pValue=(CAnimatedValueBlendable<T>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value read and write
	T	Value;
};



/**
 * A QUATERNION implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<> class CAnimatedValueBlendable<NLMISC::CQuat> : public IAnimatedValue
{
public:
	/// A quat blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast.
		CAnimatedValueBlendable<NLMISC::CQuat>	*pValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&value;

		// blend.
		// Yoyo: no makeClosest is done, because the result seems to be better when done
		// before: for all blend values, and not one after one.
		Value= NLMISC::CQuat::slerp(Value, pValue->Value, 1-blendFactor);

	}

	/**
	  * An assignation method. This method assign a value to the object.
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueBlendable<NLMISC::CQuat>	*pValue=(CAnimatedValueBlendable<NLMISC::CQuat>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	NLMISC::CQuat	Value;
};


/**
 * A CRGBA implementation of IAnimatedValue.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<> class CAnimatedValueBlendable<NLMISC::CRGBA> : public IAnimatedValue
{
public:
	/// A quat blend method.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast.
		CAnimatedValueBlendable<NLMISC::CRGBA>	*pValue=(CAnimatedValueBlendable<NLMISC::CRGBA>*)&value;

		// blend.
		Value.blendFromui (pValue->Value, this->Value, (uint)(256.f*blendFactor));
	}

	/**
	  * An assignation method. This method assign a value to the object.
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueBlendable<NLMISC::CRGBA>	*pValue=(CAnimatedValueBlendable<NLMISC::CRGBA>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	NLMISC::CRGBA	Value;
};


/**
 * A template implementation of IAnimatedValue not blendable.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CAnimatedValueNotBlendable : public IAnimatedValue
{
public:
	/// A default blend method. Doesn't work for all type.
	virtual void blend (const IAnimatedValue& value, float blendFactor)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueNotBlendable<T>	*pValue=(CAnimatedValueNotBlendable<T>*)&value;

		// Boolean blend
		if (blendFactor<0.5f)
			Value=pValue->Value;
	}

	/**
	  * An assignation method. This method assign a value to the object.
	  *
	  * \param value is the new value.
	  */
	virtual void affect (const IAnimatedValue& value)
	{
		// Check types of value. typeid is slow, assert only in debug
#ifdef NL_DEBUG
		nlassert (typeid (value)==typeid(*this));
#endif

		// Cast
		CAnimatedValueNotBlendable<T>	*pValue=(CAnimatedValueNotBlendable<T>*)&value;

		// Blend
		Value=pValue->Value;
	}

	// The value
	T	Value;
};


typedef CAnimatedValueNotBlendable<bool>			CAnimatedValueBool;
typedef CAnimatedValueBlendable<sint32>				CAnimatedValueInt;
typedef CAnimatedValueBlendable<float>				CAnimatedValueFloat;
typedef CAnimatedValueBlendable<NLMISC::CVector>	CAnimatedValueVector;
typedef CAnimatedValueNotBlendable<std::string>		CAnimatedValueString;
typedef CAnimatedValueBlendable<NLMISC::CQuat>		CAnimatedValueQuat;
typedef CAnimatedValueBlendable<NLMISC::CRGBA>		CAnimatedValueRGBA;


// ***************************************************************************
/** This class must contain all the possible AnimatedValue, so the system can work
  *	It is used at ITrack evaluation to fill one of these values
  */
class CAnimatedValueBlock
{
public:
	CAnimatedValueBool		ValBool;
	CAnimatedValueInt		ValInt;
	CAnimatedValueFloat		ValFloat;
	CAnimatedValueVector	ValVector;
	CAnimatedValueString	ValString;
	CAnimatedValueQuat		ValQuat;
	CAnimatedValueRGBA		ValRGBA;
};


} // NL3D


#endif // NL_ANIMATED_VALUE_H

/* End of animated_value.h */
