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

#include "nel/misc/common.h"
#include "nel/3d/track_sampled_vector.h"

using namespace NLMISC;
using namespace std;


namespace NL3D
{

// ***************************************************************************
// ***************************************************************************
// CTrackSampledVector
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CTrackSampledVector::CTrackSampledVector()
{
}

// ***************************************************************************
CTrackSampledVector::~CTrackSampledVector()
{
}

// ***************************************************************************
void					CTrackSampledVector::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	// serial Time infos.
	CTrackSampledCommon::serialCommon(f);

	// serial Keys.
	f.serial(_Keys);
}

// ***************************************************************************
void	CTrackSampledVector::build(const std::vector<uint16> &timeList, const std::vector<CVector> &keyList,
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
		// no compression for now
		_Keys[i]= keyList[i];
	}

}

// ***************************************************************************
const IAnimatedValue	&CTrackSampledVector::eval (const TAnimationTime& date, CAnimatedValueBlock &avBlock)
{
	// Eval time, and get key interpolation info
	uint	keyId0;
	uint	keyId1;
	float	interpValue;
	TEvalType	evalType= evalTime(date, _Keys.size(), keyId0, keyId1, interpValue);

	// Discard?
	if( evalType==EvalDiscard )
		return avBlock.ValVector;
	// One Key? easy, and quit.
	else if( evalType==EvalKey0 )
	{
		avBlock.ValVector.Value= _Keys[keyId0];
	}
	// interpolate
	else if( evalType==EvalInterpolate )
	{
		const CVector	&valueKey0= _Keys[keyId0];
		const CVector	&valueKey1= _Keys[keyId1];

		// interpolate
		avBlock.ValVector.Value= valueKey0*(1-interpValue) + valueKey1*interpValue;
	}
	else
	{
		nlstop;
	}

	return avBlock.ValVector;
}


// ***************************************************************************
void CTrackSampledVector::applySampleDivisor(uint sampleDivisor)
{
	if(sampleDivisor<=1)
		return;

	// **** build the key indices to keep, and rebuild the timeBlocks
	static	std::vector<uint32>		keepKeys;
	applySampleDivisorCommon(sampleDivisor, keepKeys);

	// **** rebuild the keys
	NLMISC::CObjectVector<CVector, false>		newKeys;
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
		_Keys.size() * sizeof(CVector));*/
}


} // NL3D
