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

#include "nel/misc/quat.h"
#include "nel/misc/common.h"
#include "nel/3d/track_sampled_quat.h"
#include "nel/3d/track_sampled_quat_small_header.h"

using namespace NLMISC;
using namespace std;


namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// Quaternion compression
// ***************************************************************************
// ***************************************************************************

const	double	NL3D_OO32767= 1.0f/32767;
const	double	NL3D_OO65535= 1.0f/65535;

#ifdef NL3D_TSQ_ALLOW_QUAT_COMPRESS
// ***************************************************************************
void		CQuatPack::pack(const CQuat &quat)
{
	/*
		This is the most precise/faster compression we can have. Some other tries have been made.

		- deducing w from x,y,z is possible with w= 1-sqrt(x^2+y^2+z^2) (with tradeoff of the W sign)
			but very not precise.
		- Transform the quaternion to an AxisAngle is possible, but slower (some cos/sin or LUT).
			Axis is encoded with sint16, and angle is encoded with uint16.
		- The same than above, but encode the axis as X/Y only, and deduce Z from
			them, is possible but precision problems arise.

		You can see that the operation "deduce a 3/4 member from unit length rule" is definetly not precise.

		Hence this simpler but workable way.
	*/

	// normalize the quaterion.
	CQuatD nquat= quat;
	nquat.normalize();

	sint	ax= (sint)floor(nquat.x * 32767 + 0.5);
	sint	ay= (sint)floor(nquat.y * 32767 + 0.5);
	sint	az= (sint)floor(nquat.z * 32767 + 0.5);
	sint	aw= (sint)floor(nquat.w * 32767 + 0.5);
	clamp(ax, -32767, 32767);
	clamp(ay, -32767, 32767);
	clamp(az, -32767, 32767);
	clamp(aw, -32767, 32767);
	x= ax;
	y= ay;
	z= az;
	w= aw;
}

// ***************************************************************************
void		CQuatPack::unpack(CQuat &quat)
{
	// unpack x/y/z.
	CQuatD	quatD;
	quatD.x= x * NL3D_OO32767;
	quatD.y= y * NL3D_OO32767;
	quatD.z= z * NL3D_OO32767;
	quatD.w= w * NL3D_OO32767;
	quatD.normalize();

	quat= quatD;
}
#endif


// ***************************************************************************
// ***************************************************************************
// CTrackSampledQuat
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTrackSampledQuat::CTrackSampledQuat()
{
}

// ***************************************************************************
CTrackSampledQuat::~CTrackSampledQuat()
{
}

// ***************************************************************************
void					CTrackSampledQuat::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- split class with base CTrackSampledCommon (must add a version in it).
	Version 0:
		- base version.
	*/
	sint	ver= f.serialVersion(1);

	if( ver<=0 )
	{
		// serial Time infos, directly in CTrackSampledCommon
		f.serial(_LoopMode);
		f.serial(_BeginTime);
		f.serial(_EndTime) ;
		f.serial(_TotalRange);
		f.serial(_OOTotalRange);
		f.serial(_DeltaTime);
		f.serial(_OODeltaTime);
		f.serial(_TimeBlocks);
	}
	else
	{
		// serial Time infos.
		CTrackSampledCommon::serialCommon(f);
	}

	// serial Keys.
	f.serial(_Keys);
}

// ***************************************************************************
void	CTrackSampledQuat::build(const std::vector<uint16> &timeList, const std::vector<CQuat> &keyList,
	float beginTime, float endTime)
{
	nlassert( endTime>beginTime || (beginTime==endTime && keyList.size()<=1) );
	nlassert( keyList.size()==timeList.size() );
	uint i;

	// reset.
	uint	numKeys= (uint)keyList.size();
	_Keys.clear();
	_TimeBlocks.clear();

	// Build Common time information
	CTrackSampledCommon::buildCommon(timeList, beginTime, endTime);


	// Compute All Key values.
	//===================
	_Keys.resize(numKeys);
	for(i=0; i<numKeys;i++)
	{
		_Keys[i].pack(keyList[i]);
	}

}

// ***************************************************************************
const IAnimatedValue	&CTrackSampledQuat::eval (const TAnimationTime& date, CAnimatedValueBlock &avBlock)
{
	/* IF YOU CHANGE THIS CODE, CHANGE too CTrackSampledQuatSmallHeader
	 */

	// Eval time, and get key interpolation info
	uint	keyId0;
	uint	keyId1;
	float	interpValue;
	TEvalType	evalType= evalTime(date, _Keys.size(), keyId0, keyId1, interpValue);

	// Discard?
	if( evalType==EvalDiscard )
		return avBlock.ValQuat;
	// One Key? easy, and quit.
	else if( evalType==EvalKey0 )
	{
		_Keys[keyId0].unpack(avBlock.ValQuat.Value);
	}
	// interpolate
	else if( evalType==EvalInterpolate )
	{
		CQuatPack	valueKey0= _Keys[keyId0];
		CQuatPack	valueKey1= _Keys[keyId1];

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
void CTrackSampledQuat::applySampleDivisor(uint sampleDivisor)
{
	if(sampleDivisor<=1)
		return;

	// **** build the key indices to keep, and rebuild the timeBlocks
	static	std::vector<uint32>		keepKeys;
	applySampleDivisorCommon(sampleDivisor, keepKeys);

	// **** rebuild the keys
	NLMISC::CObjectVector<CQuatPack, false>		newKeys;
	newKeys.resize((uint32)keepKeys.size());
	for(uint i=0;i<newKeys.size();i++)
	{
		newKeys[i]= _Keys[keepKeys[i]];
	}
	// copy
	_Keys= newKeys;

	// TestYoyo
	/*nlinfo("ANIMQUAT:\t%d\t%d\t%d\t%d", sizeof(*this), _TimeBlocks.size(),
		_TimeBlocks.size()?_TimeBlocks[0].Times.size():0,
		_Keys.size() * sizeof(CQuatPack));*/
}


// ***************************************************************************
bool	CTrackSampledQuat::applyTrackQuatHeaderCompressionPass0(class CTrackSampleCounter &quatCounter)
{
	// if there is more than 1 timeBlock, fails
	if(_TimeBlocks.size()>1)
		return false;

	// Support only 255 keys and not 256!!! cause _NumKeys is encoded in 8 bits!
	if(_Keys.size()>=256)
		return false;

	// if the number of keys ovveride the uint16 limit, abort
	if(_Keys.size()+quatCounter.NumKeys > 65536)
		return false;

	// Search if the Track header is the same as one of the quatCounter.
	// NB: O(N*N) but quatCounter.TrackHeaders should be very small
	uint	headerIndex;
	for(headerIndex=0;headerIndex<quatCounter.TrackHeaders.size();headerIndex++)
	{
		CTrackSampleHeader	&tsh= quatCounter.TrackHeaders[headerIndex];
		if( tsh.LoopMode == _LoopMode &&
			tsh.BeginTime == _BeginTime &&
			tsh.EndTime == _EndTime &&
			tsh.TotalRange == _TotalRange &&
			tsh.OOTotalRange == _OOTotalRange &&
			tsh.DeltaTime == _DeltaTime &&
			tsh.OODeltaTime == _OODeltaTime )
			break;
	}
	if(headerIndex==quatCounter.TrackHeaders.size())
	{
		// then must increment the TrackHeaders. must not ovverride the uint8 limit
		if(quatCounter.TrackHeaders.size()==256)
			return false;
		else
		{
			nlassert(quatCounter.TrackHeaders.size()<256);
			CTrackSampleHeader	tsh;
			tsh.LoopMode = _LoopMode;
			tsh.BeginTime = _BeginTime;
			tsh.EndTime = _EndTime;
			tsh.TotalRange = _TotalRange;
			tsh.OOTotalRange = _OOTotalRange;
			tsh.DeltaTime = _DeltaTime;
			tsh.OODeltaTime = _OODeltaTime;
			quatCounter.TrackHeaders.push_back(tsh);
		}
	}
	// else ok, one Header match

	// increment the number of keys in the packed data
	quatCounter.NumKeys+= _Keys.size();

	// at least this track can be compressed
	return true;
}

// ***************************************************************************
ITrack	*CTrackSampledQuat::applyTrackQuatHeaderCompressionPass1(uint &globalKeyOffset, class CTrackSamplePack &quatPacker)
{
	// if there is more than 1 timeBlock, fails
	if(_TimeBlocks.size()>1)
		return NULL;

	// Support only 255 keys and not 256!!! cause _NumKeys is encoded in 8 bits!
	if(_Keys.size()>=256)
		return NULL;

	// if the number of keys ovveride the uint16 limit, abort
	if(_Keys.size()+globalKeyOffset > 65536)
		return NULL;

	// Search if the Track header is the same as one of the quatPacker.
	// NB: O(N*N) but quatPacker.TrackHeaders should be very small
	uint	headerIndex;
	for(headerIndex=0;headerIndex<quatPacker.TrackHeaders.size();headerIndex++)
	{
		CTrackSampleHeader	&tsh= quatPacker.TrackHeaders[headerIndex];
		if( tsh.LoopMode == _LoopMode &&
			tsh.BeginTime == _BeginTime &&
			tsh.EndTime == _EndTime &&
			tsh.TotalRange == _TotalRange &&
			tsh.OOTotalRange == _OOTotalRange &&
			tsh.DeltaTime == _DeltaTime &&
			tsh.OODeltaTime == _OODeltaTime )
			break;
	}
	if(headerIndex==quatPacker.TrackHeaders.size())
	{
		return NULL;
	}

	// OK! this track can be converted to a CTrackSampledQuatSmallHeader
	uint	keyIndex= globalKeyOffset;

	// increment the number of keys in the packed data
	globalKeyOffset+= _Keys.size();

	// **** fill the packer struct
	uint	i;
	for(i=0;i<_Keys.size();i++)
	{
		quatPacker.Times[keyIndex+i]= _TimeBlocks[0].Times[i];
		quatPacker.Keys[keyIndex+i]= _Keys[i];
	}

	// **** Build the compressed quat, and return it
	return new CTrackSampledQuatSmallHeader(&quatPacker, (uint8)headerIndex, (uint8)_Keys.size(), keyIndex);
}


} // NL3D

