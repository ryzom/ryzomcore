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

#ifndef NL_TRACK_H
#define NL_TRACK_H

#include "nel/misc/types_nl.h"
#include "nel/misc/common.h"

#include "nel/3d/animation_time.h"
#include "nel/3d/u_track.h"

#include "nel/3d/animated_value.h"


namespace NL3D
{


using NLMISC::CQuat;
using NLMISC::CVector;


class	CTrackSampleCounter;
class	CTrackSamplePack;


// ***************************************************************************
/**
 * The track store an animation of an animated value. This animation can be interpolated
 * by several ways.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class ITrack : public UTrack
{
public:
	/**
	  * Virtual destructor.
	  */
	virtual ~ITrack() {}

	/**
	  * Evaluation of the value of the track for this time.
	  *
	  * The result is stored in CAnimatedValueBlock to simplify access at the polymorphic values.
	  * The track choose which value to assign, and return the one modified
	  */
	virtual const IAnimatedValue &eval (const TAnimationTime& date, CAnimatedValueBlock &avBlock)=0;

	/**
	  * get LoopMode. 2 mode only: "constant" (<=>false), and "loop" (<=> true).
	  *	NB: same mode if time < getBeginTIme() and if time > getEndTime()
	  */
	virtual bool getLoopMode() const=0;

	/** typically used by CAnimation to lower the number of keys. not supported by default
	  */
	virtual void applySampleDivisor(uint /* sampleDivisor */) {}

	/** used by CAnimation to compress the header of CTrackSampledQuat
	 *	supported only by CTrackSampledQuat
	 */
	virtual bool	applyTrackQuatHeaderCompressionPass0(CTrackSampleCounter &/* quatCounter */) {return false;}
	virtual ITrack	*applyTrackQuatHeaderCompressionPass1(uint &/* globalKeyOffset */, CTrackSamplePack &/* quatPacker */) {return NULL;}

	/// \name From UTrack
	// @{

	virtual bool interpolate (TAnimationTime time, float& res);
	virtual bool interpolate (TAnimationTime time, sint32& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CRGBA& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CVector& res);
	virtual bool interpolate (TAnimationTime time, NLMISC::CQuat& res);
	virtual bool interpolate (TAnimationTime time, std::string& res);
	virtual bool interpolate (TAnimationTime time, bool& res);

	// @}
};


// ***************************************************************************
/**
 * ITrack interface for default tracks.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class ITrackDefault : public ITrack
{
public:
	TAnimationTime getBeginTime () const
	{
		return 0.f;
	}
	TAnimationTime getEndTime () const
	{
		return 0.f;
	}
	virtual bool getLoopMode() const {return true;}
};



// ***************************************************************************
// ***************************************************************************
// TrackDefault implemenations.
// ***************************************************************************
// ***************************************************************************



/**
 * ITrackDefault implementation for blendable values.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackDefaultBlendable : public ITrackDefault
{
public:

	CTrackDefaultBlendable()
	{
	}
	CTrackDefaultBlendable(const T &val)
	{
		_Value= val;
	}

	/// set the default value.
	void	setDefaultValue(const T &val)
	{
		_Value= val;
	}

	/// get the default value.
	const T &getDefaultValue() const
	{
		return _Value;
	}


	/// Serial the template
	virtual void serial (NLMISC::IStream& f)
	{
		// Serial version
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (_Value);
	}

protected:

	T		_Value;
};


/**
 * ITrackDefault implementation for blendable values.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class T>
class CTrackDefaultNotBlendable : public ITrackDefault
{
public:

	CTrackDefaultNotBlendable()
	{
	}
	CTrackDefaultNotBlendable(const T &val)
	{
		_Value= val;
	}

	/// set the default value.
	void	setDefaultValue(const T &val)
	{
		_Value= val;
	}

	/// get the default value.
	const T &getDefaultValue() const
	{
		return _Value;
	}


	/// Serial the template
	virtual void serial (NLMISC::IStream& f)
	{
		// Serial version
		(void)f.serialVersion (0);

		// Serial the value
		f.serial (_Value);
	}

protected:

	// The default value
	T		_Value;
};


#define	NL3D_TRACKDEF_CTOR(_Son, _Father, _T)	\
	_Son() {}									\
	_Son(const _T &v) : _Father<_T>(v) {}

#define	NL3D_TRACKDEF_EVAL(_Val_)	\
	virtual const IAnimatedValue &eval (const TAnimationTime& /* date */, CAnimatedValueBlock &avBlock)	\
	{																								\
		avBlock._Val_.Value= _Value;																\
		return avBlock._Val_;																		\
	}


// Predefined types
class CTrackDefaultFloat : public CTrackDefaultBlendable<float>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultFloat, CTrackDefaultBlendable, float);
	NLMISC_DECLARE_CLASS (CTrackDefaultFloat);
	NL3D_TRACKDEF_EVAL(ValFloat)
};
class CTrackDefaultVector : public CTrackDefaultBlendable<CVector>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultVector, CTrackDefaultBlendable, CVector);
	NLMISC_DECLARE_CLASS (CTrackDefaultVector);
	NL3D_TRACKDEF_EVAL(ValVector)
};
class CTrackDefaultQuat : public CTrackDefaultBlendable<CQuat>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultQuat, CTrackDefaultBlendable, CQuat);
	NLMISC_DECLARE_CLASS (CTrackDefaultQuat);
	NL3D_TRACKDEF_EVAL(ValQuat)
};
class CTrackDefaultInt : public CTrackDefaultBlendable<sint32>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultInt, CTrackDefaultBlendable, sint32);
	NLMISC_DECLARE_CLASS (CTrackDefaultInt);
	NL3D_TRACKDEF_EVAL(ValInt)
};

class CTrackDefaultRGBA : public CTrackDefaultBlendable<NLMISC::CRGBA>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultRGBA, CTrackDefaultBlendable, NLMISC::CRGBA);
	NLMISC_DECLARE_CLASS (CTrackDefaultRGBA);
	NL3D_TRACKDEF_EVAL(ValRGBA)
};


class CTrackDefaultString : public CTrackDefaultNotBlendable<std::string>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultString, CTrackDefaultNotBlendable, std::string);
	NLMISC_DECLARE_CLASS (CTrackDefaultString);
	NL3D_TRACKDEF_EVAL(ValString)
};
class CTrackDefaultBool : public CTrackDefaultNotBlendable<bool>
{
public:
	NL3D_TRACKDEF_CTOR(CTrackDefaultBool, CTrackDefaultNotBlendable, bool);
	NLMISC_DECLARE_CLASS (CTrackDefaultBool);
	NL3D_TRACKDEF_EVAL(ValBool)
};

} // NL3D


#endif // NL_TRACK_H

/* End of track.h */
