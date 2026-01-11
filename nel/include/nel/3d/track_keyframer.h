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

#ifndef NL_TRACK_KEYFRAMER_H
#define NL_TRACK_KEYFRAMER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/track.h"
#include "nel/3d/key.h"
#include <map>
#include <memory>
#include "nel/misc/matrix.h"



namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// Templates for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack interface for keyframer.
 *
 * The ITrack and animated value types MUST match else assertions will be raised.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT>
class ITrackKeyFramer : public ITrack, public UTrackKeyframer
{
public:
	// Some types
	typedef std::map <TAnimationTime, CKeyT>	TMapTimeCKey;


	/// ctor.
	ITrackKeyFramer ()
	{
		_Dirty= false;
		_RangeLock= true;
		_LoopMode= false;
	}


	/// Destructor
	~ITrackKeyFramer ()
	{
	}

	/**
	  * Add a key in the keyframer.
	  *
	  * The key passed is duplicated in the track.
	  *
	  * \param key is the key value to add in the keyframer.
	  * \param time is the time of the key to add in the keyframer.
	  */
	void addKey (const CKeyT &key, TAnimationTime time)
	{
		// Insert the key in the map
#ifdef NL_COMP_VC6
		_MapKey.insert (TMapTimeCKey::value_type (time, key));
#else
		_MapKey.insert (typename TMapTimeCKey::value_type (time, key));
#endif
		// must precalc at next eval.
		_Dirty= true;
	}

	/// set an explicit animation range. (see getBeginTime() / setEndTime() ).
	void	unlockRange(TAnimationTime begin, TAnimationTime end)
	{
		_RangeLock= false;
		_RangeBegin= begin;
		_RangeEnd= end;
		_Dirty= true;
	}

	/// range is computed from frist and last key time (default).
	void	lockRange()
	{
		_RangeLock= true;
		_Dirty= true;
	}

	/// return true if Range is locked to first/last key. use getBeginTime and getEndTime to get the effective begin/end range times...
	bool	isRangeLocked() const {return _RangeLock;}


	/// rangeDelta is (length of effective Range) - (length of LastKey-FirstKey). NB: if RangeLock, rangeDelta==0.
	TAnimationTime	getRangeDelta() const
	{
		// update track.
		testAndClean();

		return _RangeDelta;
	}


	/// set LoopMode. 2 mode only: "constant" (<=>false), and "loop" (<=> true). same mode for in and out...
	void	setLoopMode(bool loop) {_LoopMode= loop; _Dirty= true;}

	/// get LoopMode. From ITrack
	virtual bool getLoopMode() const {return _LoopMode;}


	/// From ITrack.
	virtual const IAnimatedValue &eval (const TAnimationTime& inDate, CAnimatedValueBlock &avBlock)
	{
		float	date= inDate;
		const CKeyT *previous=NULL;
		const CKeyT *next=NULL;
		TAnimationTime datePrevious = 0;
		TAnimationTime dateNext = 0;

		// must precalc ??
		testAndClean();

		// let son choose the animated value
		IAnimatedValue	&result= chooseAnimatedValue(avBlock);

		// No keys?
		if(_MapKey.empty())
			return result;


		// Loop gestion.
		if(_LoopMode && _MapKey.size()>1 )
		{
			nlassert(_LoopEnd > _LoopStart);

			// force us to be in interval [_LoopStart, _LoopEnd[.
			if( date<_LoopStart || date>=_LoopEnd )
			{
				double	d= (date-_LoopStart)*_OOTotalRange;

				// floor(d) is the truncated number of loops.
				d= date- floor(d)*_TotalRange;
				date= (float)d;

				// For precision problems, ensure correct range.
				if(date<_LoopStart || date >= _LoopEnd)
					date= _LoopStart;
			}
		}


		// Return upper key
		typename TMapTimeCKey::iterator ite=_MapKey.upper_bound (date);

		// First next ?
		if (ite!=_MapKey.end())
		{
			// Next
			next= &(ite->second);
			dateNext=ite->first;
		}
		// loop mgt.
		else if	(_LoopMode && _MapKey.size()>1 )
		{
			// loop to first!!
			next= &(_MapKey.begin()->second);
			// must slerp from last to first,
			dateNext= _LoopEnd;
		}
		else if (!_LoopMode && _MapKey.size()>=1 )
		{
			// clamp to the last
			typename TMapTimeCKey::iterator iteLast= ite;
			iteLast--;
			next= &(iteLast->second);
		}


		// First previous ?
		if ((!_MapKey.empty())&&(ite!=_MapKey.begin()))
		{
			if (ite!=_MapKey.end())
			{
				// Previous
				ite--;
				previous= &(ite->second);
				datePrevious=ite->first;
			}
		}
		else if (!_MapKey.empty())
		{
			// Clamp at beginTime
			next= &(ite->second);
			dateNext=ite->first;
		}

		// Call evalutation fonction
		evalKey (previous, next, datePrevious, dateNext, date, result);

		return result;
	}


	virtual TAnimationTime getBeginTime () const
	{
		// must precalc ??
		testAndClean();

		return _RangeBegin;
	}
	virtual TAnimationTime getEndTime () const
	{
		// must precalc ??
		testAndClean();

		return _RangeEnd;
	}


	/// Serial the template
	virtual void serial (NLMISC::IStream& f)
	{
		// Serial version
		(void)f.serialVersion (0);

		f.serialCont(_MapKey);
		f.serial(_RangeLock, _RangeBegin, _RangeEnd);
		f.serial(_LoopMode);

		if(f.isReading())
			_Dirty= true;
	}

	/** From UTrackKeyframer, retrieve the keys that are in the given range [t1, t2] of the track
	  * \param result a vector that will be cleared, and filled with the date ofthe keys
	  */
	void getKeysInRange(TAnimationTime t1, TAnimationTime t2, std::vector<TAnimationTime> &result);


private:
	mutable	bool		_Dirty;
	bool				_LoopMode;
	bool				_RangeLock;
	float				_RangeBegin;	// if RangeLock==true, valid only when track cleaned.
	float				_RangeEnd;		// if RangeLock==true, valid only when track cleaned.
	// Valid only when cleaned.
	float				_RangeDelta;
	float				_LoopStart;
	float				_LoopEnd;
	float				_TotalRange;
	float				_OOTotalRange;


	// update track if necessary.
	void		testAndClean() const
	{
		if(_Dirty)
		{
			ITrackKeyFramer<CKeyT>	*self= const_cast<ITrackKeyFramer<CKeyT>*>(this);
			self->compile();
			_Dirty= false;
		}
	}


protected:
	TMapTimeCKey		_MapKey;


	/// This is for Deriver compile(), because _RangeDelta (getRangeDelta()) is himself computed in compile().
	float	getCompiledRangeDelta()
	{
		return _RangeDelta;
	}


	/**
	  * Precalc keyframe runtime infos for interpolation (OODTime...). All keys should be processed.
	  * This is called by eval when necessary. Deriver should call ITrackKeyFramer::compile() first, to compile basic
	  * Key runtime info.
	  */
	virtual void compile   ()
	{
		float	timeFirstKey;
		float	timeLastKey;

		// Compute time of first/last key.
		if( !_MapKey.empty() )
		{
			typename TMapTimeCKey::const_iterator ite;

			// Get first key
			ite=_MapKey.begin ();
			timeFirstKey= ite->first;

			// Get last key
			ite=_MapKey.end ();
			ite--;
			timeLastKey= ite->first;
		}
		else
		{
			timeFirstKey= 0.0f;
			timeLastKey= 0.0f;
		}


		// Compute RangeBegin / RangeEnd. (if not user provided).
		if(_RangeLock)
		{
			_RangeBegin= timeFirstKey;
			_RangeEnd= timeLastKey;
		}


		// Compute _RangeDelta.
		if(_RangeLock)
		{
			_RangeDelta= 0;
		}
		else
		{
			_RangeDelta= (_RangeEnd - _RangeBegin) - (timeLastKey - timeFirstKey);
		}

		// Misc range.
		_TotalRange= _RangeEnd - _RangeBegin;
		if(_TotalRange>0.0f)
			_OOTotalRange= 1.0f/_TotalRange;
		// start of loop / ned.
		_LoopStart= timeFirstKey;
		_LoopEnd= timeFirstKey + _TotalRange;


		// After _RangeDelta computed, compute OO delta times.
		typename TMapTimeCKey::iterator	it= _MapKey.begin();
		for(;it!=_MapKey.end();it++)
		{
			typename TMapTimeCKey::iterator	next= it;
			next++;
			if(next!=_MapKey.end())
				it->second.OODeltaTime= 1.0f/(next->first - it->first);
			else if(_RangeDelta>0.0f)
				// after last key, must slerp to first key.
				it->second.OODeltaTime= 1.0f/_RangeDelta;
			else
				it->second.OODeltaTime= 0.0f;
		}

	}

	/// return the correct type for this track
	virtual IAnimatedValue &chooseAnimatedValue(CAnimatedValueBlock &avBlock) =0;

	/**
	  * Evaluate the keyframe interpolation.
	  *
	  * i is the keyframe with the bigger time value that is inferior or equal than date.
	  *
	  * write result in correct avBlock var, and return the one modified
	  *
	  * \param previous is the i key in the keyframe. NULL if no key.
	  * \param next is the i+1 key in the keyframe. NULL if no key.
	  */
	virtual void evalKey   (const CKeyT* previous, const CKeyT* next,
							TAnimationTime datePrevious, TAnimationTime dateNext,
							TAnimationTime date, IAnimatedValue &result) =0;

};


// ***************************************************************************
// Key Tools.
// separated to just change this in RGBA and sint32 special implementation.

// just copy the content of a value issued from key interpolation, into a value.
template<class T, class TKeyVal> inline void	copyToValue(T &value, const TKeyVal &keyval)
{
	value = keyval;
}


// Vector to RGBA version.
inline void	copyToValue(NLMISC::CRGBA &col, const CVector &v)
{
	sint	i;

	i= (sint)(v.x*255); NLMISC::clamp(i,0,255); col.R= (uint8) i;
	i= (sint)(v.y*255); NLMISC::clamp(i,0,255); col.G= (uint8) i;
	i= (sint)(v.z*255); NLMISC::clamp(i,0,255); col.B= (uint8) i;
	col.A=255;
}


// float to sint32 version.
inline void	copyToValue(sint32 &value, const float &f)
{
	value= (sint32)floor(f+0.5f);
}


// ***************************************************************************
// ***************************************************************************
// Constant Keyframer.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstNotBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrackKeyFramer
	virtual void evalKey   (const CKeyT* previous, const CKeyT* next,
							TAnimationTime /* datePrevious */, TAnimationTime /* dateNext */,
							TAnimationTime /* date */, IAnimatedValue &result)
	{
		// Const key.
		if (previous)
			copyToValue(static_cast<CAnimatedValueNotBlendable<T>&>(result).Value, previous->Value);
		else
			if (next)
				copyToValue(static_cast<CAnimatedValueNotBlendable<T>&>(result).Value, next->Value);
	}
};


// ***************************************************************************
/**
 * ITrack implementation for Constant keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerConstBlendable : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							TAnimationTime /* datePrevious */, TAnimationTime /* dateNext */,
							TAnimationTime /* date */, IAnimatedValue &result )
	{
		// Const key.
		if (previous)
			copyToValue(static_cast<CAnimatedValueBlendable<T>&>(result).Value, previous->Value);
		else
			if (next)
				copyToValue(static_cast<CAnimatedValueBlendable<T>&>(result).Value, next->Value);
	}

};


// ***************************************************************************
// ***************************************************************************
// Linear Keyframer.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
/**
 * ITrack implementation for linear keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<class CKeyT, class T>
class CTrackKeyFramerLinear : public ITrackKeyFramer<CKeyT>
{
public:

	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyT* previous, const CKeyT* next,
							TAnimationTime datePrevious, TAnimationTime /* dateNext */,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueBlendable<T>	&resultVal= static_cast<CAnimatedValueBlendable<T>&>(result);

		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			// NB: in case of <CKeyInt,sint32> important that second terme is a float, so copyToValue(sint32, float) is used.
			copyToValue(resultVal.Value, previous->Value*(1.f-(float)date) + next->Value*(float)date);
		}
		else
		{
			if (previous)
				copyToValue(resultVal.Value, previous->Value);
			else
				if (next)
					copyToValue(resultVal.Value, next->Value);
		}

	}
};



// ***************************************************************************
/**
 * Quaternions special implementation for linear keyframer.
 *
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template<> class CTrackKeyFramerLinear<CKeyQuat, CQuat> : public ITrackKeyFramer<CKeyQuat>
{
public:

	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyQuat* previous, const CKeyQuat* next,
							TAnimationTime datePrevious, TAnimationTime /* dateNext */,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueBlendable<CQuat>	&resultVal= static_cast<CAnimatedValueBlendable<CQuat>&>(result);

		if(previous && next)
		{
			// slerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);
			resultVal.Value= CQuat::slerp(previous->Value, next->Value, date);
		}
		else
		{
			if (previous)
				resultVal.Value=previous->Value;
			else
				if (next)
					resultVal.Value=next->Value;
		}
	}
};


// ***************************************************************************
/**
 * ITrack implementation for linear CRGBA keyframer.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
template<> class CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>: public ITrackKeyFramer<CKeyRGBA>
{
public:

	/// From ITrackKeyFramer
	virtual void evalKey (	const CKeyRGBA* previous, const CKeyRGBA* next,
							TAnimationTime datePrevious, TAnimationTime /* dateNext */,
							TAnimationTime date, IAnimatedValue &result )
	{
		CAnimatedValueBlendable<NLMISC::CRGBA>	&resultVal= static_cast<CAnimatedValueBlendable<NLMISC::CRGBA>&>(result);

		if(previous && next)
		{
			// lerp from previous to cur.
			date-= datePrevious;
			date*= previous->OODeltaTime;
			NLMISC::clamp(date, 0,1);

			// blend.
			resultVal.Value.blendFromui(previous->Value, next->Value, (uint)(date*256));
		}
		else
		{
			if (previous)
				resultVal.Value= previous->Value;
			else
				if (next)
					resultVal.Value= next->Value;
		}
	}
};



// ***************************************************************************
// ***************************************************************************
// TCB / Bezier Keyframer.
// ***************************************************************************
// ***************************************************************************


// Template implementation for TCB and Bezier.
#include "track_tcb.h"
#include "track_bezier.h"



// ***************************************************************************
// ***************************************************************************
// Predefined types for KeyFramer tracks.
// ***************************************************************************
// ***************************************************************************

#define	NL3D_TRACKKEYF_CHOOSE(_Val_)	\
virtual IAnimatedValue &chooseAnimatedValue(CAnimatedValueBlock &avBlock)	\
{																			\
	return avBlock._Val_;													\
}


// Const tracks.
class CTrackKeyFramerConstFloat : public CTrackKeyFramerConstBlendable<CKeyFloat,float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstFloat);
	NL3D_TRACKKEYF_CHOOSE(ValFloat)
};
class CTrackKeyFramerConstVector : public CTrackKeyFramerConstBlendable<CKeyVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstVector);
	NL3D_TRACKKEYF_CHOOSE(ValVector)
};
class CTrackKeyFramerConstQuat : public CTrackKeyFramerConstBlendable<CKeyQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstQuat);
	NL3D_TRACKKEYF_CHOOSE(ValQuat)
};
class CTrackKeyFramerConstInt : public CTrackKeyFramerConstBlendable<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstInt);
	NL3D_TRACKKEYF_CHOOSE(ValInt)
};
class CTrackKeyFramerConstRGBA : public CTrackKeyFramerConstBlendable<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstRGBA);
	NL3D_TRACKKEYF_CHOOSE(ValRGBA)
};

class CTrackKeyFramerConstString : public CTrackKeyFramerConstNotBlendable<CKeyString, std::string>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstString);
	NL3D_TRACKKEYF_CHOOSE(ValString)
};
class CTrackKeyFramerConstBool : public CTrackKeyFramerConstNotBlendable<CKeyBool, bool>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerConstBool);
	NL3D_TRACKKEYF_CHOOSE(ValBool)
};


// Linear tracks.
class CTrackKeyFramerLinearFloat : public CTrackKeyFramerLinear<CKeyFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearFloat);
	NL3D_TRACKKEYF_CHOOSE(ValFloat)

	virtual	bool	addLinearFloatKey(const UKeyLinearFloat &key)
	{
		CKeyFloat	k;
		k.OODeltaTime= 0.f;
		k.Value= key.Value;
		addKey(k, key.Time);
		return true;
	}
};
class CTrackKeyFramerLinearVector : public CTrackKeyFramerLinear<CKeyVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearVector);
	NL3D_TRACKKEYF_CHOOSE(ValVector)
};
class CTrackKeyFramerLinearQuat : public CTrackKeyFramerLinear<CKeyQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearQuat);
	NL3D_TRACKKEYF_CHOOSE(ValQuat)
};
class CTrackKeyFramerLinearInt : public CTrackKeyFramerLinear<CKeyInt, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearInt);
	NL3D_TRACKKEYF_CHOOSE(ValInt)
};
class CTrackKeyFramerLinearRGBA : public CTrackKeyFramerLinear<CKeyRGBA, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerLinearRGBA);
	NL3D_TRACKKEYF_CHOOSE(ValRGBA)
};


// TCB tracks.
class CTrackKeyFramerTCBFloat : public CTrackKeyFramerTCB<CKeyTCBFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBFloat);
	NL3D_TRACKKEYF_CHOOSE(ValFloat)

	virtual	bool	addTCBFloatKey(const UKeyTCBFloat &key)
	{
		CKeyTCBFloat	k;
		k.Value= key.Value;
		k.Bias= key.Bias;
		k.Continuity= key.Continuity;
		k.Tension= key.Tension;
		k.EaseFrom= key.EaseFrom;
		k.EaseTo= key.EaseTo;
		addKey(k, key.Time);

		return true;
	}
};
class CTrackKeyFramerTCBVector : public CTrackKeyFramerTCB<CKeyTCBVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBVector);
	NL3D_TRACKKEYF_CHOOSE(ValVector)
};
class CTrackKeyFramerTCBQuat : public CTrackKeyFramerTCB<CKeyTCBQuat, NLMISC::CAngleAxis>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBQuat);
	NL3D_TRACKKEYF_CHOOSE(ValQuat)
};
class CTrackKeyFramerTCBInt : public CTrackKeyFramerTCB<CKeyTCBFloat, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBInt);
	NL3D_TRACKKEYF_CHOOSE(ValInt)
};
class CTrackKeyFramerTCBRGBA : public CTrackKeyFramerTCB<CKeyTCBVector, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerTCBRGBA);
	NL3D_TRACKKEYF_CHOOSE(ValRGBA)
};


// Bezier tracks.
class CTrackKeyFramerBezierFloat : public CTrackKeyFramerBezier<CKeyBezierFloat, float>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierFloat);
	NL3D_TRACKKEYF_CHOOSE(ValFloat)

	virtual	bool	addBezierFloatKey(const UKeyBezierFloat &key)
	{
		CKeyBezierFloat	k;
		k.Value= key.Value;
		k.InTan= key.TanIn;
		k.OutTan= key.TanOut;
		k.Step= key.Step;
		addKey(k, key.Time);
		return true;
	}
};
class CTrackKeyFramerBezierVector : public CTrackKeyFramerBezier<CKeyBezierVector, CVector>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierVector);
	NL3D_TRACKKEYF_CHOOSE(ValVector)
};
class CTrackKeyFramerBezierQuat : public CTrackKeyFramerBezier<CKeyBezierQuat, CQuat>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierQuat);
	NL3D_TRACKKEYF_CHOOSE(ValQuat)
};
class CTrackKeyFramerBezierInt : public CTrackKeyFramerBezier<CKeyBezierFloat, sint32>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierInt);
	NL3D_TRACKKEYF_CHOOSE(ValInt)
};
class CTrackKeyFramerBezierRGBA : public CTrackKeyFramerBezier<CKeyBezierVector, NLMISC::CRGBA>
{
public:
	NLMISC_DECLARE_CLASS (CTrackKeyFramerBezierRGBA);
	NL3D_TRACKKEYF_CHOOSE(ValRGBA)
};

} // NL3D


#endif // NL_TRACK_KEYFRAMER_H

/* End of track_keyframer.h */
