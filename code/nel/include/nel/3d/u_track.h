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

#ifndef NL_U_TRACK_H
#define NL_U_TRACK_H

#include "nel/misc/stream.h"
#include "nel/misc/types_nl.h"

namespace NLMISC
{

class CQuat;
class CRGBA;
class CVector;

}

namespace NL3D
{

/**
 * A track is a fonction that interpolate a value over the time.
 *
 * Kind of interpolation is hidden to the user. It can be Bezier, TCB, linear, noise interpolation.
 * This interface give access to the interpolation fonction.
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UTrack : public NLMISC::IStreamable
{
public:

	/// \name Time range methods.
	// @{

	/**
	  * Get the begin time of the track
	  */
	virtual TAnimationTime getBeginTime () const=0;

	/**
	  * Get the end time of the track
	  */
	virtual TAnimationTime getEndTime () const=0;

	// @}

	/// \name Interpolation methods.
	// @{

	/**
	  * Interplation a float value. You should be sure that the track you use to interpolate
	  * your value is a float track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, float& res) =0;

	/**
	  * Interplation an integer value. You should be sure that the track you use to interpolate
	  * your value is an integer track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, sint32& res) =0;

	/**
	  * Interplation a CRGBA value. You should be sure that the track you use to interpolate
	  * your value is an CRGBA track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, NLMISC::CRGBA& res) =0;

	/**
	  * Interplation a CVector value. You should be sure that the track you use to interpolate
	  * your value is a CVector track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, NLMISC::CVector& res) =0;

	/**
	  * Interplation a CQuat value. You should be sure that the track you use to interpolate
	  * your value is a CQuat track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, NLMISC::CQuat& res) =0;

	/**
	  * Interplation a string value. You should be sure that the track you use to interpolate
	  * your value is a string track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, std::string& res) =0;

	/**
	  * Interplation a bool value. You should be sure that the track you use to interpolate
	  * your value is a bool track! An assertion will be raised in debug if the type is wrong.
	  *
	  * \param time is the time you want the evaluate the value. If time higher than the time
	  * gived by getEndTime (), the value returned is the interpolation value at getEndTime ().
	  * If time smaller than the time gived by getBeginTime (), the value returned is the
	  * interpolation value at getBeginTime ().
	  * \param res is the reference on the value to get the result.
	  * \return true if interplation is successful. false if the type asked is wrong.
	  */
	virtual bool interpolate (TAnimationTime time, bool& res) =0;

	// @}
};


/** This is a keyframer track interface.
  * Once you've got a track, you can know whether its a keyframer track by using a dynamic_cast
  */
class UTrackKeyframer
{
public:
	class UKeyLinearFloat
	{
	public:
		TAnimationTime	Time;
		float			Value;
	};

	class UKeyBezierFloat
	{
	public:
		TAnimationTime	Time;
		float			Value;
		float			TanIn, TanOut;
		bool			Step;
	};

	class UKeyTCBFloat
	{
	public:
		TAnimationTime	Time;
		float			Value;
		float			Tension;
		float			Continuity;
		float			Bias;
		float			EaseFrom;
		float			EaseTo;
	};

	UTrackKeyframer(){}
	virtual ~UTrackKeyframer(){}

	static	UTrackKeyframer	*createLinearFloatTrack();
	static	UTrackKeyframer	*createBezierFloatTrack();
	static	UTrackKeyframer	*createTCBFloatTrack();

public:
	/** Retrieve the keys that are in the given range ]t1, t2] of the track. They can then be evaluated
	  * \param result a vector that will be cleared, and filled with the date ofthe keys
	  */
	virtual void getKeysInRange(TAnimationTime t1, TAnimationTime t2, std::vector<TAnimationTime> &result)=0;

	/// Fail if not A Float Linear Keyframer
	virtual	bool	addLinearFloatKey(const UKeyLinearFloat &/* key */) {return false;}
	/// Fail if not A Float Bezier Keyframer
	virtual	bool	addBezierFloatKey(const UKeyBezierFloat &/* key */) {return false;}
	/// Fail if not A Float TCB Keyframer
	virtual	bool	addTCBFloatKey(const UKeyTCBFloat &/* key */) {return false;}
};



} // NL3D


#endif // NL_U_TRACK_H

/* End of u_track.h */
