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

#include "std3d.h"

#include "nel/3d/track_sampled_quat_small_header.h"
#include "nel/misc/algo.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
CTrackSampledQuatSmallHeader::CTrackSampledQuatSmallHeader(CTrackSamplePack *pack, uint8 headerIndex, uint8 numKeys, uint16 keyIndex)
{
	nlassert(pack);
	_TrackSamplePack= pack;
	_IndexTrackHeader= headerIndex;
	_NumKeys= numKeys;
	_KeyIndex= keyIndex;
}

// ***************************************************************************
CTrackSampledQuatSmallHeader::~CTrackSampledQuatSmallHeader()
{
}


// ***************************************************************************
bool					CTrackSampledQuatSmallHeader::getLoopMode() const
{
	return _TrackSamplePack->TrackHeaders[_IndexTrackHeader].LoopMode;
}

// ***************************************************************************
TAnimationTime			CTrackSampledQuatSmallHeader::getBeginTime () const
{
	return _TrackSamplePack->TrackHeaders[_IndexTrackHeader].BeginTime;
}

// ***************************************************************************
TAnimationTime			CTrackSampledQuatSmallHeader::getEndTime () const
{
	return _TrackSamplePack->TrackHeaders[_IndexTrackHeader].EndTime;
}

// ***************************************************************************
CTrackSampledQuatSmallHeader::TEvalType				CTrackSampledQuatSmallHeader::evalTime (const TAnimationTime& date, uint &keyId0, uint &keyId1, float &interpValue)
{
	/* IF YOU CHANGE THIS CODE, CHANGE too CTrackSampledQuatCommon
	 */

	// Empty? quit
	if(_NumKeys==0)
		return EvalDiscard;

	// One Key? easy, and quit.
	if(_NumKeys==1)
	{
		keyId0= 0;
		return EvalKey0;
	}

	// Get the Track header info
	CTrackSampleHeader	&trackHeader= _TrackSamplePack->TrackHeaders[_IndexTrackHeader];

	// manage Loop
	//=====================
	float	localTime;
	if(trackHeader.LoopMode)
	{
		nlassert(trackHeader.TotalRange>0);
		// get relative to BeginTime.
		localTime= date-trackHeader.BeginTime;

		// force us to be in interval [0, _TotalRange[.
		if( localTime<0 || localTime>=trackHeader.TotalRange )
		{
			double	d= localTime*trackHeader.OOTotalRange;

			// floor(d) is the truncated number of loops.
			d= localTime- floor(d)*trackHeader.TotalRange;
			localTime= (float)d;

			// For precision problems, ensure correct range.
			if(localTime<0 || localTime >= trackHeader.TotalRange)
				localTime= 0;
		}

	}
	else
	{
		// get relative to BeginTime.
		localTime= date-trackHeader.BeginTime;
	}

	// get times ptr in packed data
	uint8	*times= &_TrackSamplePack->Times[_KeyIndex];


	// Find the first key before localTime
	//=====================
	// get the frame in the track.
	sint	frame= (sint)floor(localTime*trackHeader.OODeltaTime);
	// clamp to uint8
	clamp(frame, 0, 255);

	// get the key.
	keyId0= searchLowerBound(times, _NumKeys, (uint8)frame);

	// Get the Frame of Key0.
	uint		frameKey0= times[keyId0];


	// Interpolate with next key
	//=====================

	// If not the last Key
	if(keyId0<(uint)_NumKeys-1)
	{
		// Get the next key.
		keyId1= keyId0+1;
		uint		frameKey1= times[keyId1];

		// unpack time.
		float	time0= frameKey0*trackHeader.DeltaTime;
		float	time1= frameKey1*trackHeader.DeltaTime;

		// interpolate.
		float	t= (localTime-time0);
		// If difference is one frame, optimize.
		if(frameKey1-frameKey0==1)
			t*= trackHeader.OODeltaTime;
		else
			t/= (time1-time0);
		clamp(t, 0.f, 1.f);

		// store this interp value.
		interpValue= t;

		return EvalInterpolate;
	}
	// else (last key of anim), just eval this key.
	return EvalKey0;
}

// ***************************************************************************
const IAnimatedValue	&CTrackSampledQuatSmallHeader::eval (const TAnimationTime& date, CAnimatedValueBlock &avBlock)
{
	/* IF YOU CHANGE THIS CODE, CHANGE too CTrackSampledQuatSmallHeader
	 */

	// **** Eval time, and get key interpolation info
	uint	keyId0;
	uint	keyId1;
	float	interpValue;
	TEvalType	evalType= evalTime(date, keyId0, keyId1, interpValue);

	// **** Eval Keys
	// get starting keys
	CQuatPack	*keys= &_TrackSamplePack->Keys[_KeyIndex];

	// Discard?
	if( evalType==EvalDiscard )
		return avBlock.ValQuat;
	// One Key? easy, and quit.
	else if( evalType==EvalKey0 )
	{
		keys[keyId0].unpack(avBlock.ValQuat.Value);
	}
	// interpolate
	else if( evalType==EvalInterpolate )
	{
		CQuatPack	valueKey0= keys[keyId0];
		CQuatPack	valueKey1= keys[keyId1];

		// If the 2 keys have same value, just unpack.
		if(valueKey0 == valueKey1)
		{
			valueKey0.unpack(avBlock.ValQuat.Value);
		}
		// else interpolate
		else
		{
			// unpack key value.
			CQuat	quat0, quat1;
			valueKey0.unpack(quat0);
			valueKey1.unpack(quat1);

			// interpolate
			avBlock.ValQuat.Value= CQuat::slerp(quat0, quat1, interpValue);
		}
	}
	else
	{
		nlstop;
	}

	return avBlock.ValQuat;
}


// ***************************************************************************
void					CTrackSampledQuatSmallHeader::serial(NLMISC::IStream &/* f */)
{
	// CTrackSampledQuatSmallHeader not designed to be serialsied
	nlstop;
}



} // NL3D
