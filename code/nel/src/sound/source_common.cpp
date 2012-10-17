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

#include "stdsound.h"

#include "nel/sound/source_common.h"
#include "nel/sound/audio_mixer_user.h"


using namespace NLMISC;

namespace NLSOUND
{

CSourceCommon::CSourceCommon(TSoundId id, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
:	_Priority(MidPri),
	_Playing(false),
	_Looping(false),
	_Position(CVector::Null),
	_Velocity(CVector::Null),
	_Direction(CVector::Null),
	_Gain(1.0f),
	_Pitch(1.0f),
	_RelativeMode(false),
	_InitialGain(1.0f),
	_PlayStart(0),
	_Spawn(spawn),
	_SpawnEndCb(cb),
	_CbUserParam(cbUserParam),
	_Cluster(cluster),
	_UserVarControler(id->getUserVarControler()),
	_GroupController(groupController ? groupController : id->getGroupController())
{
	CAudioMixerUser::instance()->addSource(this);
	_GroupController->addSource(this);

	// get a local copy of the sound parameter
	_InitialGain = _Gain = id->getGain();
	_Pitch = id->getPitch();
	_Looping = id->getLooping();
	_Priority = id->getPriority();
	_Direction = id->getDirectionVector();
}

CSourceCommon::~CSourceCommon()
{
	_GroupController->removeSource(this);
	CAudioMixerUser::instance()->removeSource(this);
}


/*
 * Change the priority of the source
 */
void CSourceCommon::setPriority( TSoundPriority pr)
{
	_Priority = pr;

	// The AudioMixer redispatches as necessary in the update() function [PH]
	// Redispatch the tracks if needed
	//if ( redispatch )
	//{
	//	CAudioMixerUser::instance()->balanceSources();
	//}
}

/*
 * Set looping on/off for future playbacks (default: off)
 */
void					CSourceCommon::setLooping( bool l )
{
	_Looping = l;
}

/*
 * Return the looping state
 */
bool					CSourceCommon::getLooping() const
{
	return _Looping;
}

/*
 * Play
 */
void					CSourceCommon::play()
{
	CAudioMixerUser::instance()->incPlayingSource();
	_Playing = true;
	_PlayStart = CTime::getLocalTime();

	if (_UserVarControler != CStringMapper::emptyId())
		CAudioMixerUser::instance()->addUserControledSource(this, _UserVarControler);
}

/*
 * Stop playing
 */
void					CSourceCommon::stop()
{
	CAudioMixerUser::instance()->decPlayingSource();
	_Playing = false;

	if (_UserVarControler != CStringMapper::emptyId())
		CAudioMixerUser::instance()->removeUserControledSource(this, _UserVarControler);
}

/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void					CSourceCommon::setPos( const NLMISC::CVector& pos )
{
	_Position = pos;
}

/* Get the position vector.
 * If the source is stereo, return the position vector which reference was passed to set3DPositionVector()
 */
const NLMISC::CVector &CSourceCommon::getPos() const
{
	//if ( _3DPosition == NULL )
	//{
	return _Position;
	//}
	//else
	//{
	//	return *_3DPosition;
	//}

}

/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void					CSourceCommon::setPitch( float pitch )
{
//	nlassert( (pitch > 0) && (pitch <= 1.0f ) );
	_Pitch = pitch;
}


/*
 * Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
 */
void					CSourceCommon::setVelocity( const NLMISC::CVector& vel )
{
	_Velocity = vel;
}

/*
 * Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
 */
void					CSourceCommon::setDirection( const NLMISC::CVector& dir )
{
	_Direction = dir;
}

/* Set the gain (volume value inside [0 , 1]). (default: 1)
 * 0.0 -> silence
 * 0.5 -> -6dB
 * 1.0 -> no attenuation
 * values > 1 (amplification) not supported by most drivers
 */
void					CSourceCommon::setGain( float gain )
{
	clamp(gain, 0.0f, 1.0f);
	_InitialGain = _Gain = gain;
	updateFinalGain();
}

/* Set the gain amount (value inside [0, 1]) to map between 0 and the nominal gain
 * (which is getSource()->getGain()). Does nothing if getSource() is null.
 */
void					CSourceCommon::setRelativeGain( float gain )
{
	clamp(gain, 0.0f, 1.0f);
	_Gain = _InitialGain * gain;
	updateFinalGain();
}

/*
 * Return the relative gain (see setRelativeGain()), or the absolute gain if getSource() is null.
 */
float					CSourceCommon::getRelativeGain() const
{
	if (_InitialGain == 0.0f)
		return 0.0f;
	else
		return _Gain / _InitialGain;
}

/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
 */
void					CSourceCommon::setSourceRelativeMode( bool mode )
{
	_RelativeMode = mode;
}


uint32					CSourceCommon::getTime()
{
	if (!_Playing)
		return 0;
	// default implementation
	return uint32(CTime::getLocalTime() - _PlayStart);
}


} // NLSOUND
