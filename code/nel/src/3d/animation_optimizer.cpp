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

#include "nel/3d/animation_optimizer.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/vectord.h"
#include "nel/3d/track.h"
#include "nel/3d/track_keyframer.h"
#include "nel/3d/animation.h"
#include "nel/3d/track_sampled_quat.h"
#include "nel/3d/track_sampled_vector.h"


using	namespace NLMISC;
using	namespace std;


namespace NL3D
{


// ***************************************************************************
CAnimationOptimizer::CAnimationOptimizer()
{
	_SampleFrameRate= 30;
	_QuaternionThresholdLowPrec= 1.0 - 0.0001;
	_QuaternionThresholdHighPrec= 1.0 - 0.000001;
	_VectorThresholdLowPrec= 0.001;
	_VectorThresholdHighPrec= 0.0001;
}


// ***************************************************************************
void		CAnimationOptimizer::setQuaternionThreshold(double lowPrecThre, double highPrecThre)
{
	nlassert(lowPrecThre>=0);
	nlassert(highPrecThre>=0);
	_QuaternionThresholdLowPrec= 1.0 - lowPrecThre;
	_QuaternionThresholdHighPrec= 1.0 - highPrecThre;
}


// ***************************************************************************
void		CAnimationOptimizer::setVectorThreshold(double lowPrecThre, double highPrecThre)
{
	nlassert(lowPrecThre>=0);
	nlassert(highPrecThre>=0);
	_VectorThresholdLowPrec= lowPrecThre;
	_VectorThresholdHighPrec= highPrecThre;
}


// ***************************************************************************
void		CAnimationOptimizer::setSampleFrameRate(float frameRate)
{
	nlassert(frameRate>0);
	_SampleFrameRate= frameRate;
}


// ***************************************************************************
void		CAnimationOptimizer::optimize(const CAnimation &animIn, CAnimation &animOut)
{
	// reset animOut
	contReset(animOut);

	// Parse all tracks of the animation.
	set<string>		setString;
	animIn.getTrackNames (setString);
	set<string>::iterator	it;

	for(it=setString.begin();it!=setString.end();it++)
	{
		const string	&trackName= *it;
		uint	trackId= animIn.getIdTrackByName(trackName);
		nlassert(trackId!=CAnimation::NotFound);
		const ITrack	*track= animIn.getTrack(trackId);

		// If the track is optimisable.
		ITrack	*newTrack;
		if(isTrackOptimisable(track))
		{
			// choose the threshold according to precision wanted
			if( isLowPrecisionTrack(trackName) )
			{
				_QuaternionThreshold= _QuaternionThresholdLowPrec;
				_VectorThreshold= _VectorThresholdLowPrec;
			}
			else
			{
				_QuaternionThreshold= _QuaternionThresholdHighPrec;
				_VectorThreshold= _VectorThresholdHighPrec;
			}

			// optimize it.
			newTrack= optimizeTrack(track);
		}
		else
		{
			// just clone it.
			newTrack= cloneTrack(track);
		}

		// Add it to the animation
		animOut.addTrack(trackName, newTrack);
	}

	// Parse all SSS shapes of the animation (important for preload of those shapes)
	const vector<string>	&shapes= animIn.getSSSShapes();
	for(uint i=0;i<shapes.size();i++)
		animOut.addSSSShape(shapes[i]);

	// Set min animation length
	animOut.setMinEndTime (animIn.getEndTime ());
	nlassert (animOut.getEndTime() == animIn.getEndTime());
}

// ***************************************************************************
ITrack		*CAnimationOptimizer::cloneTrack(const ITrack	*trackIn)
{
	CMemStream	memStream;

	// write to the stream.
	ITrack	*trackInSerial= const_cast<ITrack*>(trackIn);
	memStream.serialPolyPtr(trackInSerial);

	// read from the stream.
	memStream.invert();
	ITrack	*ret= NULL;
	memStream.serialPolyPtr(ret);

	return ret;
}


// ***************************************************************************
bool		CAnimationOptimizer::isTrackOptimisable(const ITrack	*trackIn)
{
	nlassert(trackIn);

	// If the track is a Linear, Bezier or a TCB track, suppose we can optimize it. Constant may not be interressant....
	if(	dynamic_cast<const CTrackKeyFramerTCBQuat*>(trackIn) ||
		dynamic_cast<const CTrackKeyFramerBezierQuat*>(trackIn) ||
		dynamic_cast<const CTrackKeyFramerLinearQuat*>(trackIn) )
		return true;

	// If the track is a Linear, Bezier or a TCB track, suppose we can optimize it. Constant may not be interressant....
	if(	dynamic_cast<const CTrackKeyFramerTCBVector*>(trackIn) ||
		dynamic_cast<const CTrackKeyFramerBezierVector*>(trackIn) ||
		dynamic_cast<const CTrackKeyFramerLinearVector*>(trackIn) )
		return true;

	return false;
}


// ***************************************************************************
ITrack		*CAnimationOptimizer::optimizeTrack(const ITrack	*trackIn)
{
	// Get track param.
	float beginTime= trackIn->getBeginTime();
	float endTime= trackIn->getEndTime();
	nlassert(endTime>=beginTime);

	// Get num Sample
	uint	numSamples= (uint)ceil( (endTime-beginTime)*_SampleFrameRate);
	numSamples= max(1U, numSamples);
	nlassert(numSamples<65535);


	// Optimize Quaternion track??
	//================
	// eval the track only to get its value type!!
	CAnimatedValueBlock		avBlock;
	const IAnimatedValue	&valueType= ((ITrack*)trackIn)->eval(0, avBlock);
	if( dynamic_cast<const CAnimatedValueQuat *>(&valueType) )
	{
		// sample the animation. Store result in _TimeList/_QuatKeyList
		sampleQuatTrack(trackIn, beginTime, endTime, numSamples);

		// check if the sampled track can be reduced to a TrackDefaultQuat. Test _QuatKeyList.
		if( testConstantQuatTrack() )
		{
			// create a default Track Quat.
			CTrackDefaultQuat	*trackDefault= new CTrackDefaultQuat;
			// setup the uniform value.
			trackDefault->setDefaultValue(_QuatKeyList[0]);

			// return the result.
			return trackDefault;
		}
		// else optimize the sampled animation, and build.
		else
		{
			// optimize.
			optimizeQuatTrack();

			// Create a sampled quaternion track
			CTrackSampledQuat	*trackSQ= new CTrackSampledQuat;

			// Copy loop from track.
			trackSQ->setLoopMode(trackIn->getLoopMode());

			// Build it.
			trackSQ->build(_TimeList, _QuatKeyList, beginTime, endTime);

			// return result.
			return trackSQ;
		}
	}
	// Optimize Position track??
	//================
	else if( dynamic_cast<const CAnimatedValueVector *>(&valueType) )
	{
		// sample the animation. Store result in _TimeList/_VectorKeyList
		sampleVectorTrack(trackIn, beginTime, endTime, numSamples);

		// check if the sampled track can be reduced to a TrackDefaultVector. Test _VectorKeyList.
		if( testConstantVectorTrack() )
		{
			// create a default Track Vector.
			CTrackDefaultVector	*trackDefault= new CTrackDefaultVector;
			// setup the uniform value.
			trackDefault->setDefaultValue(_VectorKeyList[0]);

			// return the result.
			return trackDefault;
		}
		// else optimize the sampled animation, and build.
		else
		{
			// optimize.
			optimizeVectorTrack();

			// Create a sampled Vector track
			CTrackSampledVector	*trackSV= new CTrackSampledVector;

			// Copy loop from track.
			trackSV->setLoopMode(trackIn->getLoopMode());

			// Build it.
			trackSV->build(_TimeList, _VectorKeyList, beginTime, endTime);

			// return result.
			return trackSV;
		}
	}
	else
	{
		// Must be a quaternion track or vector track for now.
		nlstop;
		// Avoid warning.
		return cloneTrack(trackIn);
	}
}


// ***************************************************************************
// ***************************************************************************
// Quaternion optimisation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CAnimationOptimizer::sampleQuatTrack(const ITrack *trackIn, float beginTime, float endTime, uint numSamples)
{
	// resize tmp samples
	_TimeList.resize(numSamples);
	_QuatKeyList.resize(numSamples);

	// Sample the animation.
	float	t= beginTime;
	float	dt= 0;
	if(numSamples>1)
		dt= (endTime-beginTime)/(numSamples-1);
	for(uint i=0;i<numSamples; i++, t+=dt)
	{
		CQuat	quat;

		// make exact endTime match (avoid precision problem)
		if(i==numSamples-1)
			t= endTime;

		// evaluate the track
		const_cast<ITrack*>(trackIn)->interpolate(t, quat);

		// normalize this quaternion
		quat.normalize();

		// force on same hemisphere according to precedent frame.
		if(i>0)
		{
			quat.makeClosest(_QuatKeyList[i-1]);
		}

		// store time and key.
		_TimeList[i]= i;
		_QuatKeyList[i]= quat;
	}

}

// ***************************************************************************
bool		CAnimationOptimizer::testConstantQuatTrack()
{
	uint	numSamples= (uint)_QuatKeyList.size();
	nlassert(numSamples>0);

	// Get the first sample as the reference quaternion, and test others from this one.
	CQuat	quatRef= _QuatKeyList[0];
	for(uint i=0;i<numSamples;i++)
	{
		// All values must be nearly equal to the reference quaternion.
		if(!nearlySameQuaternion(quatRef, _QuatKeyList[i]))
			return false;
	}

	// ok.
	return true;
}


// ***************************************************************************
void		CAnimationOptimizer::optimizeQuatTrack()
{
	uint	numSamples= (uint)_QuatKeyList.size();
	nlassert(numSamples>0);

	// <=2 key? => no opt possible..
	if(numSamples<=2)
		return;

	// prepare dest opt
	std::vector<uint16>		optTimeList;
	std::vector<CQuat>		optKeyList;
	optTimeList.reserve(numSamples);
	optKeyList.reserve(numSamples);

	// Add the first key.
	optTimeList.push_back(_TimeList[0]);
	optKeyList.push_back(_QuatKeyList[0]);
	double	timeRef= _TimeList[0];
	CQuatD	quatRef= _QuatKeyList[0];

	// For all keys, but the first and the last, test if can remove them.
	for(uint i=1; i<numSamples-1; i++)
	{
		CQuatD	quatCur= _QuatKeyList[i];
		CQuatD	quatNext= _QuatKeyList[i+1];
		double	timeCur= _TimeList[i];
		double	timeNext= _TimeList[i+1];

		// must add the key?
		bool	mustAdd= false;

		// If the Delta time are too big, abort (CTrackSampledQuat limitation)
		if(timeNext-timeRef>255)
		{
			mustAdd= true;
		}
		// If the next quaternion or the current quaternion are not on same hemisphere than ref, abort.
		else if( CQuatD::dotProduct(quatCur, quatRef)<0 || CQuatD::dotProduct(quatNext, quatRef)<0 )
		{
			mustAdd= true;
		}
		// else, test interpolation
		else
		{
			// If the 3 quats are nearly equals, it is ok (avoid interpolation)
			if( nearlySameQuaternion(quatRef, quatCur) && nearlySameQuaternion(quatRef, quatNext) )
				mustAdd= false;
			else
			{
				// interpolate.
				CQuatD	quatInterpolated;
				double	t= (timeCur-timeRef)/(timeNext/timeRef);
				quatInterpolated= CQuatD::slerp(quatRef, quatNext, (float)t);

				// test if cur and interpolate are equal.
				if( !nearlySameQuaternion(quatCur, quatInterpolated) )
					mustAdd= true;
			}
		}

		// If must add the key to the optimized track.
		if(mustAdd)
		{
			optTimeList.push_back(_TimeList[i]);
			optKeyList.push_back(_QuatKeyList[i]);
			timeRef= _TimeList[i];
			quatRef= _QuatKeyList[i];
		}
	}

	// Add the last key.
	optTimeList.push_back(_TimeList[numSamples-1]);
	optKeyList.push_back(_QuatKeyList[numSamples-1]);

	// copy the optimized track to the main one.
	_TimeList= optTimeList;
	_QuatKeyList= optKeyList;
}


// ***************************************************************************
bool		CAnimationOptimizer::nearlySameQuaternion(const CQuatD &quat0, const CQuatD &quat1)
{
	// true if exactly same, or exactly inverse
	if(quat0==quat1 || quat0==-quat1)
		return true;

	// Else compute the rotation to go from qRef to q. Use double for better presion.
	CQuatD	quatDif;
	quatDif= quat1 * quat0.conjugate();
	// inverse the quaternion if necessary. ie make closest to the identity quaternion.
	if(quatDif.w<0)
		quatDif= -quatDif;

	// compare "angle threshold"
	return (quatDif.w >= _QuaternionThreshold);
}


// ***************************************************************************
// ***************************************************************************
// Vector optimisation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CAnimationOptimizer::sampleVectorTrack(const ITrack *trackIn, float beginTime, float endTime, uint numSamples)
{
	// resize tmp samples
	_TimeList.resize(numSamples);
	_VectorKeyList.resize(numSamples);

	// Sample the animation.
	float	t= beginTime;
	float	dt= 0;
	if(numSamples>1)
		dt= (endTime-beginTime)/(numSamples-1);
	for(uint i=0;i<numSamples; i++, t+=dt)
	{
		CVector	vector;

		// make exact endTime match (avoid precision problem)
		if(i==numSamples-1)
			t= endTime;

		// evaluate the track
		const_cast<ITrack*>(trackIn)->interpolate(t, vector);

		// store time and key.
		_TimeList[i]= i;
		_VectorKeyList[i]= vector;
	}

}

// ***************************************************************************
bool		CAnimationOptimizer::testConstantVectorTrack()
{
	uint	numSamples= (uint)_VectorKeyList.size();
	nlassert(numSamples>0);

	// Get the first sample as the reference Vectorer, and test others from this one.
	CVector	vectorRef= _VectorKeyList[0];
	for(uint i=0;i<numSamples;i++)
	{
		// All values must be nearly equal to the reference vector.
		if(!nearlySameVector(vectorRef, _VectorKeyList[i]))
			return false;
	}

	// ok.
	return true;
}


// ***************************************************************************
void		CAnimationOptimizer::optimizeVectorTrack()
{
	uint	numSamples= (uint)_VectorKeyList.size();
	nlassert(numSamples>0);

	// <=2 key? => no opt possible..
	if(numSamples<=2)
		return;

	// prepare dest opt
	std::vector<uint16>		optTimeList;
	std::vector<CVector>	optKeyList;
	optTimeList.reserve(numSamples);
	optKeyList.reserve(numSamples);

	// Add the first key.
	optTimeList.push_back(_TimeList[0]);
	optKeyList.push_back(_VectorKeyList[0]);
	double		timeRef= _TimeList[0];
	CVectorD	vectorRef= _VectorKeyList[0];

	// For all keys, but the first and the last, test if can remove them.
	for(uint i=1; i<numSamples-1; i++)
	{
		CVectorD	vectorCur= _VectorKeyList[i];
		CVectorD	vectorNext= _VectorKeyList[i+1];
		double	timeCur= _TimeList[i];
		double	timeNext= _TimeList[i+1];

		// must add the key?
		bool	mustAdd= false;

		// If the Delta time are too big, abort (CTrackSampledVector limitation)
		if(timeNext-timeRef>255)
		{
			mustAdd= true;
		}
		// else, test interpolation
		else
		{
			// If the 3 Vectors are nearly equals, it is ok (avoid interpolation)
			if( nearlySameVector(vectorRef, vectorCur) && nearlySameVector(vectorRef, vectorNext) )
				mustAdd= false;
			else
			{
				// interpolate.
				CVectorD	vectorInterpolated;
				double	t= (timeCur-timeRef)/(timeNext/timeRef);
				vectorInterpolated= vectorRef*(1-t) + vectorNext*t;

				// test if cur and interpolate are equal.
				if( !nearlySameVector(vectorCur, vectorInterpolated) )
					mustAdd= true;
			}
		}

		// If must add the key to the optimized track.
		if(mustAdd)
		{
			optTimeList.push_back(_TimeList[i]);
			optKeyList.push_back(_VectorKeyList[i]);
			timeRef= _TimeList[i];
			vectorRef= _VectorKeyList[i];
		}
	}

	// Add the last key.
	optTimeList.push_back(_TimeList[numSamples-1]);
	optKeyList.push_back(_VectorKeyList[numSamples-1]);

	// copy the optimized track to the main one.
	_TimeList= optTimeList;
	_VectorKeyList= optKeyList;
}


// ***************************************************************************
bool		CAnimationOptimizer::nearlySameVector(const CVectorD &v0, const CVectorD &v1)
{
	// true if exactly same
	if(v0==v1)
		return true;

	// Else compute the dif, use double for better precision
	CVectorD	vDif;
	vDif= v1-v0;

	// compare norm
	return (vDif.norm() <= _VectorThreshold);
}


// ***************************************************************************
// ***************************************************************************
// LowPrecisionTrack
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void		CAnimationOptimizer::addLowPrecisionTrack(const std::string &name)
{
	_LowPrecTrackKeyName.push_back(name);
}

// ***************************************************************************
void		CAnimationOptimizer::clearLowPrecisionTracks()
{
	_LowPrecTrackKeyName.clear();
}

// ***************************************************************************
bool		CAnimationOptimizer::isLowPrecisionTrack(const std::string &trackName)
{
	for(uint i=0; i<_LowPrecTrackKeyName.size(); i++)
	{
		// if find a substr of the key, it is a low prec track
		if( trackName.find(_LowPrecTrackKeyName[i]) != string::npos )
			return true;
	}

	// no key found
	return false;
}


} // NL3D
