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

#ifndef NL_ANIMATION_OPTIMIZER_H
#define NL_ANIMATION_OPTIMIZER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/quat.h"
#include "nel/misc/vectord.h"


namespace NL3D
{

using	NLMISC::CQuat;
using	NLMISC::CQuatD;
using	NLMISC::CVector;
using	NLMISC::CVectorD;

class	CAnimation;
class	ITrack;

// ***************************************************************************
/**
 * Purpose of this class is to optimize for memory and speed a CAnimation.
 *	Quaternion tracks are optimized. They are transformed to CTrackSampledQuat or CTrackDefaultQuat.
 *	Vector tracks are optimized. They are transformed to CTrackSampledVector or CTrackDefaultVector.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CAnimationOptimizer
{
public:

	/// Constructor
	CAnimationOptimizer();

	/** Set the Quaternion Error Thresholds. must be >=0.
	 *	Default is 0.0001 and 0.000001. Above this value, 2 quaternions are said different.
	 *	NB: comparing 2 quaternions is made by multiplying the one with the inverse of the other.
	 *	the W component is then tested and must be >1-threshold (modulo sign) to conclude to equality.
	 *	Since quat.W=cos(angle/2), you can deduce the angle threshold with angleThre=acos(1-thre)*2
	 *
	 *	Give 2 values, one For Low precision and High precision. Default setup use high precision. Use
	 *	addLowPrecisionTrack() to drive low precision tracks.
	 */
	void		setQuaternionThreshold(double lowPrecThre, double highPrecThre);

	/** Same principle as for setQuaternionThreshold(), but for vector tracks (positions/scale).
	 *	Default is 0.001 and 0.0001. Above this value, 2 vectors are said different.
	 *	NB: comparing 2 vectors is made by geting the norm of the difference
	 *
	 *	Give 2 values, one For Low precision and High precision. Default setup use high precision. Use
	 *	addLowPrecisionTrack() to drive low precision tracks.
	 */
	void		setVectorThreshold(double lowPrecThre, double highPrecThre);

	/** see setQuaternionThreshold(). Any track which contains this name will be considered as a
	 *	low precision track. Default setup is empty, so all tracks are "high precision" tracks.
	 *	A good setup is for example addLowPrecisionTrack("Finger") and addLowPrecisionTrack("Ponytail")
	 *	Warning: case sensitive
	 */
	void		addLowPrecisionTrack(const std::string &name);

	/// see addLowPrecisionTrack. This clears the array.
	void		clearLowPrecisionTracks();

	/** Set the Sample Frame Rate (>0)
	 *	Default is 30 fps. NB: final numSamples must be <65535, else assert at optimize() time.
	 */
	void		setSampleFrameRate(float frameRate);


	/** optimize an animation
	 *	animOut is the same as animIn, but optimized.
	 *	If a track can't be optimized, it is just duplicated in animOut.
	 */
	void		optimize(const CAnimation &animIn, CAnimation &animOut);


// ******************
private:
	float		_SampleFrameRate;
	double		_QuaternionThresholdLowPrec;
	double		_QuaternionThresholdHighPrec;
	double		_QuaternionThreshold;
	double		_VectorThresholdLowPrec;
	double		_VectorThresholdHighPrec;
	double		_VectorThreshold;


	// see addLowPrecisionTrack
	std::vector<std::string>	_LowPrecTrackKeyName;

	// For Sampling of Quaternion Tracks.
	std::vector<uint16>		_TimeList;
	std::vector<CQuat>		_QuatKeyList;

	// For Sampling of Vector Tracks.
	std::vector<CVector>	_VectorKeyList;

private:

	// Clone a ITrack (using a CMemStream)
	ITrack		*cloneTrack(const ITrack	*trackIn);

	// true if the track can be optimized.
	bool		isTrackOptimisable(const ITrack	*trackIn);

	// return an optimized version of trackIn. Type returned should be different (eg: CTrackSampledQuat)
	ITrack		*optimizeTrack(const ITrack	*trackIn);

	// see addLowPrecisionTrack()
	bool		isLowPrecisionTrack(const std::string &trackName);


	/// Quaternion optimisation.
	// @{

	/** sample the track from beginTime to endTime, sample to numSamples, such that
	 *	key[0].Time==beginTime and key[numSamples-1].Time==endTime.
	 *	NB: quaternions are normalized, and are makeClosest()-ed from sample to the next.
	 */
	void		sampleQuatTrack(const ITrack *trackIn, float beginTime, float endTime, uint numSamples);

	/** Test if the current track is constant (ie always same quaternion value)
	 *	NB: test if q==qRef or q==-qRef of course.
	 */
	bool		testConstantQuatTrack();

	/** optimze the current track
	 */
	void		optimizeQuatTrack();

	/** return true if suppose same quaternion. NB: quaternion must be normalized.
	 *	NB: test if quat1==quat0 or quat1==-quat0 of course.
	 */
	bool		nearlySameQuaternion(const CQuatD &quat0, const CQuatD &quat1);

	// @}


	/// Vector optimisation.
	// @{

	/** sample the track from beginTime to endTime, sample to numSamples, such that
	 *	key[0].Time==beginTime and key[numSamples-1].Time==endTime.
	 */
	void		sampleVectorTrack(const ITrack *trackIn, float beginTime, float endTime, uint numSamples);

	/** Test if the current track is constant (ie always same Vector value)
	 */
	bool		testConstantVectorTrack();

	/** optimze the current track
	 */
	void		optimizeVectorTrack();

	/** return true if suppose same vector.
	 */
	bool		nearlySameVector(const CVectorD &v0, const CVectorD &v1);

	// @}

};


} // NL3D


#endif // NL_ANIMATION_OPTIMIZER_H

/* End of animation_optimizer.h */
