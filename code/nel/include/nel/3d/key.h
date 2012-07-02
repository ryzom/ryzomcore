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

#ifndef NL_KEY_H
#define NL_KEY_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/quat.h"
#include "nel/misc/stream.h"
#include "nel/misc/rgba.h"

// NOT TESTED, JUST COMPILED. FOR PURPOSE ONLY.

namespace NL3D
{


// ***************************************************************************
/**
 * Interface for a key of a keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKey
{
public:
	/// synonym for T.
	typedef	T	TValueType;

public:

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (Value);
	};

	/// The key value
	T					Value;


// *********************
public:
	// PRIVATE. used by ITrackKeyFramer, not serialised (compiled).
	// 1/(nextKeyTime-thisKeyTime).
	float				OODeltaTime;

};


// ***************************************************************************
/**
 * Implementation of CKey for TCB keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKeyTCB : public CKey<T>
{
public:

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (this->Value);
		f.serial (Tension);
		f.serial (Continuity);
		f.serial (Bias);
		f.serial (EaseTo);
		f.serial (EaseFrom);
	};

	float	Tension;
	float	Continuity;
	float	Bias;
	float	EaseTo;
	float	EaseFrom;


// *********************
public:
	// PRIVATE. used by ITrackKeyFramer, not serialised (compiled).
	// computed tangents.
	T		TanTo, TanFrom;
	// computed ease parameters, with next key.
	float	Ease0, Ease1;
	float	EaseK, EaseKOverEase0, EaseKOverEase1;

};


// ***************************************************************************
/**
 * Implementation of CKey for Bezier keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CKeyBezier : public CKey<T>
{
public:

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (this->Value);
		f.serial (InTan);
		f.serial (OutTan);
		f.serial (Step);
	};

	/// \name Tangents.
	/** Those are NOT the true Bezier control points: they are tangents relative to Value, and relative to keyTime:
	 *		CPIn= Value + InTan * dt0/3;		\n
	 *		CPOut= Value + OutTan * dt1/3;		\n
	 *	where:	dt0= curKey.time - prevKey.time		\n
	 *			dt1= nextKey.time - curKey.time		\n
	 *
	 * and when not possible (first/last key), dt= getRangeDelta() is used.
	 *
	 */
	// @{
	T		InTan;
	T		OutTan;
	bool	Step;
	// @}
};


// ***************************************************************************
// Special implementation for Quaternions.
// ***************************************************************************


// ***************************************************************************
/**
 * Implementation of CKeyTCB for rotation.
 * WARNING!!! the value (an angleaxis!!) is a rotation relative to the preceding key!!  (unlike CKeyBezier)
 * WARNING!!! the axis of the value (an angleaxis) is relative to World Space!!, not relative to preceding key basis. (like in 3DS Max).
 *
 * \author Lionel berenguier
 * \author Nevrax France
 * \date 2001
 */
template<> class	CKeyTCB<NLMISC::CAngleAxis> : public CKey<NLMISC::CAngleAxis>
{
public:

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (Value);
		f.serial (Tension);
		f.serial (Continuity);
		f.serial (Bias);
		f.serial (EaseTo);
		f.serial (EaseFrom);
	};

	float	Tension;
	float	Continuity;
	float	Bias;
	float	EaseTo;
	float	EaseFrom;


// *********************
public:
	// PRIVATE. used by ITrackKeyFramer, not serialised (compiled).
	// Local AngleAxis to the preceding Key (axis is in local basis, unlike "Value" where the axis is in World space ).
	NLMISC::CAngleAxis	LocalAngleAxis;
	// computed quaternions/tangents.
	NLMISC::CQuat		Quat, A, B;
	// computed ease parameters, with next key.
	float	Ease0, Ease1;
	float	EaseK, EaseKOverEase0, EaseKOverEase1;
};


// ***************************************************************************
/**
 * Implementation of CKeyBezier for rotation. (no tangents for "bezier rotation", it is a "smooth rotation").
 * WARNING!!! the Value (a Quat!!) is a ABSOLUTE rotation (unlike CKeyTCB)
 *
 * \author Lionel berenguier
 * \author Nevrax France
 * \date 2001
 */
template<> class	CKeyBezier<NLMISC::CQuat> : public CKey<NLMISC::CQuat>
{
public:

	/// Serial
	void serial (NLMISC::IStream& f) throw (NLMISC::EStream)
	{
		// Version number
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (Value);
	};


// *********************
public:
	// PRIVATE. used by ITrackKeyFramer, not serialised (compiled).
	// computed quaternions/tangents.
	NLMISC::CQuat		A;
};


// ***************************************************************************
// Predefined types
// ***************************************************************************

// ** Const - Linear keys
typedef	CKey<std::string>		CKeyString;
typedef	CKey<bool>				CKeyBool;
typedef	CKey<float>				CKeyFloat;
typedef	CKey<NLMISC::CVector>	CKeyVector;
typedef	CKey<NLMISC::CQuat>		CKeyQuat;
typedef	CKey<NLMISC::CRGBA>		CKeyRGBA;
typedef	CKey<sint32>			CKeyInt;
// NB: For precision and optimisation (space/speed), RGBA and sint32 const/linear tracks use CKeyRGBA and CKeyInt keys repsectively.


// ** TCB keys
typedef	CKeyTCB<float>				CKeyTCBFloat;
typedef	CKeyTCB<NLMISC::CVector>	CKeyTCBVector;
typedef	CKeyTCB<NLMISC::CAngleAxis>	CKeyTCBQuat;
// NB: RGBA and sint32 TCB tracks use CKeyTCBVector and CKeyTCBFloat respectively.

// ** Bezier keys
typedef	CKeyBezier<float>			CKeyBezierFloat;
typedef	CKeyBezier<NLMISC::CVector>	CKeyBezierVector;
typedef	CKeyBezier<NLMISC::CQuat>	CKeyBezierQuat;
// NB: RGBA and sint32 bezier tracks use CKeyBezierVector and CKeyBezierFloat respectively.



} // NL3D


#endif // NL_KEY_H

/* End of key.h */
