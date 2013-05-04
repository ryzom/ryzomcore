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

#include "nel/sound/driver/buffer.h"
#include "nel/sound/driver/source.h"

#include "nel/sound/simple_source.h"
#include "nel/sound/mixing_track.h"
#include "nel/sound/simple_sound.h"
#include "nel/sound/clustered_sound.h"

using namespace NLMISC;

namespace NLSOUND {

CSimpleSource::CSimpleSource(CSimpleSound *simpleSound, bool spawn, TSpawnEndCallback cb, void *cbUserParam, NL3D::CCluster *cluster, CGroupController *groupController)
	: CSourceCommon(simpleSound, spawn, cb, cbUserParam, cluster, groupController), 
	_SimpleSound(simpleSound),
	_Track(NULL), 
	_PlayMuted(false),
	_WaitingForPlay(false)
{
	nlassert(_SimpleSound != 0);

	// get a local copy of the simple sound parameter
	_Alpha = _SimpleSound->getAlpha();
}

CSimpleSource::~CSimpleSource()
{
	if (_Playing)
		stop();
	// Yoyo: security. with prec stop(), should not be needed, but a crash still raise
	// in "currentEvent->onEvent();" in audio_mixer_user.cpp
	// [KAETEMI TODO: Take a look at previous comment.]
	CAudioMixerUser::instance()->removeEvents(this);
}

void CSimpleSource::initPhysicalSource()
{
	CAudioMixerUser *mixer = CAudioMixerUser::instance();
	CTrack *track = mixer->getFreeTrack(this);
	if (track != NULL)
	{
		nlassert(track->hasPhysicalSource());
		_Track = track;
	}
}

void CSimpleSource::releasePhysicalSource()
{
	if (hasPhysicalSource())
	{
		CAudioMixerUser *mixer = CAudioMixerUser::instance();
		ISource *pSource = getPhysicalSource();
		nlassert(pSource != NULL);

		// free the track
		pSource->stop();
		pSource->setStaticBuffer(NULL);
		mixer->freeTrack(_Track);
		_Track = NULL;
	}
}

uint32 CSimpleSource::getTime()
{
	if (hasPhysicalSource())
		return getPhysicalSource()->getTime();
	else
		return 0;
}

IBuffer *CSimpleSource::getBuffer()
{
	return _SimpleSound->getBuffer();
}

/// Set looping on/off for future playbacks (default: off)
void CSimpleSource::setLooping(bool l)
{
	CSourceCommon::setLooping(l);
	if (hasPhysicalSource())
		getPhysicalSource()->setLooping( l );
}

CVector CSimpleSource::getVirtualPos() const
{
	if (getCluster() != 0)
	{
		// need to check the cluster status
		const CClusteredSound::CClusterSoundStatus *css = CAudioMixerUser::instance()->getClusteredSound()->getClusterSoundStatus(getCluster());
		if (css != 0)
		{
			// there is some data here, update the virtual position of the sound.
			float dist = (css->Position - getPos()).norm();
			CVector vpos(CAudioMixerUser::instance()->getListenPosVector() + css->Direction * (css->Dist + dist));
			vpos = _Position * (1-css->PosAlpha) + vpos*(css->PosAlpha);
			return vpos;
		}
	}

	return getPos();
}

/// Play
void CSimpleSource::play()
{
	// nldebug("CSimpleSource %p : play", this);

	CAudioMixerUser *mixer = CAudioMixerUser::instance();

	// -- Some test to check if we can play the source

	// Check if sample buffer is available and if the sound source is not too far
	if (_SimpleSound->getBuffer() == 0
		|| !_SimpleSound->getBuffer()->isBufferLoaded()
		//|| (mixer->getListenPosVector() - _Position).sqrnorm() > _SimpleSound->getMaxDistance() * _SimpleSound->getMaxDistance())
		|| (_RelativeMode ? getPos().sqrnorm() : (mixer->getListenPosVector() - getPos()).sqrnorm()) > _SimpleSound->getMaxDistance() * _SimpleSound->getMaxDistance())
	{
		// The sample buffer is not available, don't play (we don't know the length)
		_WaitingForPlay = false;
		if (_Spawn)
		{
			if (_SpawnEndCb != 0)
				_SpawnEndCb(this, _CbUserParam);
			
			delete this;
		}
		// nldebug("CSimpleSource %p : play FAILED !", (CAudioMixerUser::IMixerEvent*)this);
		return;
	}

	// -- Here we can play the source, either in a real track or as a muted source.

	// Try to obtain a track
	if (!hasPhysicalSource())
		initPhysicalSource();

	if (hasPhysicalSource())
	{
		ISource *pSource = getPhysicalSource();
		nlassert(pSource != NULL);

		// ok, we have a track to realy play, fill the data into the track
		pSource->setStaticBuffer(_SimpleSound->getBuffer());
		
		// pSource->setPos( _Position, false);
		pSource->setPos(getVirtualPos(), false);
		if (!_SimpleSound->getBuffer()->isStereo())
		{
			pSource->setMinMaxDistances(_SimpleSound->getMinDistance(), _SimpleSound->getMaxDistance(), false);
			setDirection(_Direction); // because there is a workaround inside
			pSource->setVelocity(_Velocity);
		}
		pSource->setGain(getFinalGain());
		pSource->setSourceRelativeMode(_RelativeMode);
		pSource->setLooping(_Looping);
		pSource->setPitch(_Pitch);
		pSource->setAlpha(_Alpha);
		
		// and play the sound
		bool play = pSource->play();		
		
#ifdef NL_DEBUG
		nlassert(play);
#else
		if (!play)
			nlwarning("Failed to play physical sound source. This is a serious error");
#endif

		// nldebug("CSimpleSource %p : REAL play done", (CAudioMixerUser::IMixerEvent*)this);
	}
	else
	{
		if (_Priority == HighestPri)
		{
			// This sound is not discardable, add it in waiting playlist
			mixer->addSourceWaitingForPlay(this);
			_WaitingForPlay = true;
			return;
		}
		// there is no available track, just do a 'muted' play
		mixer->addEvent(this, CTime::getLocalTime() + _SimpleSound->getDuration());
		_PlayMuted = true;
		mixer->incPlayingSourceMuted();
		// nldebug("CSimpleSource %p : MUTED play done", (CAudioMixerUser::IMixerEvent*)this);
	}

	CSourceCommon::play();
	_WaitingForPlay = false;
}

/// Mixer event call when doing muted play
void CSimpleSource::onEvent()
{
	// nldebug("CSimpleSource %p : stop EVENT", (CAudioMixerUser::IMixerEvent*)this);

	// A muted play is terminated.

	if (!_Playing)
		return;	
	// nlassert(_Playing);
	// nlassert(_Track == 0);

	_PlayMuted = false;
	CAudioMixerUser::instance()->decPlayingSourceMuted();
	
	stop();
}

/// Stop playing
void CSimpleSource::stop()
{
	// nldebug("CSimpleSource %p : stop", (CAudioMixerUser::IMixerEvent*)this);
	// nlassert(_Playing);

	if (_WaitingForPlay)
	{
		nlassert(!_Playing); // cannot already be playing if waiting for play
		CAudioMixerUser *mixer = CAudioMixerUser::instance();
		mixer->removeSourceWaitingForPlay(this);
	}

	if (!_Playing)
		return;

	if (hasPhysicalSource())
	{
		releasePhysicalSource();
	}
	else if (_PlayMuted)
	{
		CAudioMixerUser *mixer = CAudioMixerUser::instance();
		// clear the registered event because of a stop before normal end of play
		mixer->decPlayingSourceMuted();
		mixer->removeEvents(this);
	}
	
	CSourceCommon::stop();

	if (_Spawn)
	{
		if (_SpawnEndCb != NULL)
		{
			_SpawnEndCb(this, _CbUserParam);
		}

		delete this;
	}
}

/* Set the position vector (default: (0,0,0)).
 * 3D mode -> 3D position
 * st mode -> x is the pan value (from left (-1) to right (1)), set y and z to 0
 */
void CSimpleSource::setPos(const NLMISC::CVector& pos)
{
	CSourceCommon::setPos(pos);

	// Set the position
	if (hasPhysicalSource())
	{
		// getPhysicalSource()->setPos(pos);
		getPhysicalSource()->setPos(getVirtualPos());
	}
}


/*
 * Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
 */
void CSimpleSource::setVelocity(const NLMISC::CVector& vel)
{
	CSourceCommon::setVelocity(vel);

	// Set the velocity
	if (hasPhysicalSource())
	{
		// TODO : uncomment, test only
		getPhysicalSource()->setVelocity(vel);
	}
}


/*
 * Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
 */
void CSimpleSource::setDirection(const NLMISC::CVector& dir)
{
	CSourceCommon::setDirection(dir);

	// Set the direction
	if (hasPhysicalSource())
	{
		if (!_SimpleSound->getBuffer()->isStereo())
		{
			static bool coneset = false;
			if (dir.isNull()) // workaround
			{
				getPhysicalSource()->setCone(float(Pi * 2), float(Pi * 2), 1.0f); // because the direction with 0 is not enough for a non-directional source!
				getPhysicalSource()->setDirection(CVector::I);  // Don't send a 0 vector, DSound will complain. Send (1,0,0), it's omnidirectional anyway.
				coneset = false;
			}
			else
			{
//				if (!coneset)
				{
					getPhysicalSource()->setCone(_SimpleSound->getConeInnerAngle(), _SimpleSound->getConeOuterAngle(), _SimpleSound->getConeOuterGain());
					coneset = true;
				}
				getPhysicalSource()->setDirection(dir);
			}
		}
	}
}

void CSimpleSource::updateFinalGain()
{
	// Set the gain
	if (hasPhysicalSource())
		getPhysicalSource()->setGain(getFinalGain());
}

/* Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
 * of one octave. 0 is not a legal value.
 */
void CSimpleSource::setPitch(float pitch)
{
	CSourceCommon::setPitch(pitch);

	// Set the pitch
	if (hasPhysicalSource())
	{
		getPhysicalSource()->setPitch( pitch );
	}
}


/*
 * Set the source relative mode. If true, positions are interpreted relative to the listener position (default: false)
 */
void CSimpleSource::setSourceRelativeMode(bool mode)
{
	CSourceCommon::setSourceRelativeMode(mode);

	// Set the relative mode
	if (hasPhysicalSource())
	{
		getPhysicalSource()->setSourceRelativeMode( mode );
	}
}

/*
 * Get playing state. Return false if the source has stopped on its own.
 */
bool CSimpleSource::isPlaying()
{
	return _Playing;
}

} // NLSOUND
