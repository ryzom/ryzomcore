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

#include "stdopenal.h"
#include "sound_driver_al.h"
#include "listener_al.h"
#include "effect_al.h"
#include "buffer_al.h"
#include "source_al.h"
#include "ext_al.h"

// #define NLSOUND_DEBUG_GAIN

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CSourceAL::CSourceAL(CSoundDriverAL *soundDriver) :
_SoundDriver(NULL), _Buffer(NULL), _Source(AL_NONE),
_DirectFilter(AL_FILTER_NULL), _EffectFilter(AL_FILTER_NULL), 
_IsPlaying(false), _IsPaused(false), _StartTime(0), _IsStreaming(false), _RelativeMode(false), 
_Pos(0.0f, 0.0f, 0.0f), _Gain(NLSOUND_DEFAULT_GAIN), _Alpha(1.0), 
_MinDistance(1.0f), _MaxDistance(sqrt(numeric_limits<float>::max())), 
_Effect(NULL), _Direct(true), 
_DirectGain(NLSOUND_DEFAULT_DIRECT_GAIN), _EffectGain(NLSOUND_DEFAULT_EFFECT_GAIN), 
_DirectFilterType(ISource::FilterLowPass), _EffectFilterType(ISource::FilterLowPass), 
_DirectFilterEnabled(false), _EffectFilterEnabled(false), 
_DirectFilterPassGain(NLSOUND_DEFAULT_FILTER_PASS_GAIN), _EffectFilterPassGain(NLSOUND_DEFAULT_FILTER_PASS_GAIN)
{
	// create the al source
	alGenSources(1, &_Source);
	alTestError();
	
	// configure rolloff
	if (soundDriver->getOption(ISoundDriver::OptionManualRolloff))
	{
		alSourcef(_Source, AL_ROLLOFF_FACTOR, 0);
		alTestError();
	}
	else
	{
		alSourcef(_Source, AL_ROLLOFF_FACTOR, soundDriver->getRolloffFactor());
		alTestError();
	}
	
	// create filters
	if (soundDriver->getOption(ISoundDriver::OptionEnvironmentEffects))
	{
		alGenFilters(1, &_DirectFilter);
		alFilteri(_DirectFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		alFilterf(_DirectFilter, AL_LOWPASS_GAIN, NLSOUND_DEFAULT_DIRECT_GAIN);
		alFilterf(_DirectFilter, AL_LOWPASS_GAINHF, NLSOUND_DEFAULT_FILTER_PASS_GAIN);
		alTestError();
		alGenFilters(1, &_EffectFilter);
		alFilteri(_EffectFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		alFilterf(_EffectFilter, AL_LOWPASS_GAIN, NLSOUND_DEFAULT_EFFECT_GAIN);
		alFilterf(_EffectFilter, AL_LOWPASS_GAINHF, NLSOUND_DEFAULT_FILTER_PASS_GAIN);
		alTestError();
	}
	
	// if everything went well, the source will be added in the sounddriver
	_SoundDriver = soundDriver;
}

CSourceAL::~CSourceAL()
{
	CSoundDriverAL *soundDriver = _SoundDriver;
	release();
	if (soundDriver) soundDriver->removeSource(this);
}

void CSourceAL::release()
{
	if (_Source != AL_NONE) { alDeleteSources(1, &_Source); _Source = AL_NONE; }
	if (_DirectFilter != AL_FILTER_NULL) { alDeleteFilters(1, &_DirectFilter); _DirectFilter = AL_FILTER_NULL; }
	if (_EffectFilter != AL_FILTER_NULL) { alDeleteFilters(1, &_EffectFilter); _EffectFilter = AL_FILTER_NULL; }
	_SoundDriver = NULL;
}

/// (Internal) Update the 3d changes.
void CSourceAL::updateManualRolloff()
{
	CVector distanceVector = _RelativeMode ? _Pos : (_Pos - CListenerAL::getInstance()->getPos());
	float distanceSquare = distanceVector.sqrnorm();
	float rolloff = ISource::computeManualRolloff(_Alpha, distanceSquare, _MinDistance, _MaxDistance);
	alSourcef(_Source, AL_GAIN, _Gain * rolloff);
	alTestError();
#ifdef NLSOUND_DEBUG_GAIN
	ALfloat gain;
	alGetSourcef(_Source, AL_GAIN, &gain);
	nlwarning("Called updateManualRolloff(), physical gain is %f, configured gain is %f, distanceSquare is %f, rolloff is %f", gain, _Gain, distanceSquare, rolloff);
	alTestError();
#endif
}

/// Enable or disable streaming mode. Source must be stopped to call this.
void CSourceAL::setStreaming(bool streaming)
{
	nlassert(isStopped());

	// bring the source type to AL_UNDETERMINED
	alSourcei(_Source, AL_BUFFER, AL_NONE);
	alTestError();
	_Buffer = NULL;
	_IsStreaming = streaming;
}

/* Set the buffer that will be played (no streaming)
 * If the buffer is stereo, the source mode becomes stereo and the source relative mode is on,
 * otherwise the source is considered as a 3D source.
 */
void CSourceAL::setStaticBuffer( IBuffer *buffer )
{
	// Stop source
	alSourceStop(_Source);
	alTestError();

	// Set buffer
	if ( buffer == NULL )
	{
		alSourcei(_Source, AL_BUFFER, AL_NONE );
		alTestError();
		_Buffer = NULL;
	}
	else
	{
		CBufferAL *bufferAL = dynamic_cast<CBufferAL *>(buffer);
		alSourcei(_Source, AL_BUFFER, bufferAL->bufferName() );
		alTestError();

		// Set relative mode if the buffer is stereo
		setSourceRelativeMode( bufferAL->isStereo() );

		_Buffer = bufferAL;
	}
}


IBuffer *CSourceAL::getStaticBuffer()
{
	return _Buffer;
}

/// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming.
/// Should be called by a thread which checks countStreamingBuffers every 100ms.
void CSourceAL::submitStreamingBuffer(IBuffer *buffer)
{
	CBufferAL *bufferAL = static_cast<CBufferAL *>(buffer);
	ALuint bufferName = bufferAL->bufferName();
	nlassert(bufferName);
	
	if (!bufferAL->isBufferLoaded())
	{
		nlwarning("AL: MUSICBUG: Streaming buffer was not loaded, skipping buffer. This should not happen.");
		return;
	}
	
	alSourceQueueBuffers(_Source, 1, &bufferName);
	alTestError();
	_QueuedBuffers.push(bufferAL);
	
	// Resume playback if the internal OpenAL source stopped due to buffer underrun.
	ALint srcstate;
	alGetSourcei(_Source, AL_SOURCE_STATE, &srcstate);
	alTestError();
	if (_IsPlaying && (srcstate == AL_STOPPED || srcstate == AL_INITIAL))
	{
		nlwarning("AL: Streaming buffer underrun, resuming playback.");
		play();
	}
}

/// Return the amount of buffers in the queue (playing and waiting). 3 buffers is optimal.
uint CSourceAL::countStreamingBuffers() const
{
	// a bit ugly here, but makes a much easier/simpler implementation on both drivers
	ALint buffersProcessed;
	alGetSourcei(_Source, AL_BUFFERS_PROCESSED, &buffersProcessed);
	while (buffersProcessed)
	{
		ALuint bufferName = _QueuedBuffers.front()->bufferName();
		alSourceUnqueueBuffers(_Source, 1, &bufferName);
		alTestError();
		const_cast<std::queue<CBufferAL *> &>(_QueuedBuffers).pop();
		--buffersProcessed;
	}
	// return how many are left in the queue
	//ALint buffersQueued;
	//alGetSourcei(_SourceName, AL_BUFFERS_QUEUED, &buffersQueued);
	//alTestError();
	//return (uint)buffersQueued;
	return (uint)_QueuedBuffers.size();
}

/// Set looping on/off for future playbacks (default: off)
void CSourceAL::setLooping( bool l )
{
	alSourcei(_Source, AL_LOOPING, l?AL_TRUE:AL_FALSE );
	alTestError();
}

/// Return the looping state
bool CSourceAL::getLooping() const
{
	ALint b;
	alGetSourcei(_Source, AL_LOOPING, &b );
	alTestError();
	return ( b == AL_TRUE );
}

/// Play the static buffer (or stream in and play)
bool CSourceAL::play()
{
#ifdef NLSOUND_DEBUG_GAIN
	if (_IsStreaming)
	{
		nlwarning("Called play on a streaming source");
	}
#endif

	// Commit 3D changes before starting play
	if (_SoundDriver->getOption(ISoundDriver::OptionManualRolloff))
		updateManualRolloff();

	if (_Buffer)
	{
		// Static playing mode
		_IsPaused = false;
		alSourcePlay(_Source);
		_IsPlaying = (alGetError() == AL_NO_ERROR);
		if (_IsPlaying)
			_StartTime = CTime::getLocalTime();
		return _IsPlaying;
	}
	else if (_IsStreaming)
	{
		_IsPaused = false;
		/* NEW */
		// called by user as well as by code to resume after buffer underrun
		if (!_IsPlaying) // set start time if not playing yet
			_StartTime = CTime::getLocalTime();
		_IsPlaying = true; // this play always virtually succeed but may not actually be playing
		if (_QueuedBuffers.size()) // ensure buffers have actually queued
		{
			alSourcePlay(_Source);
			if (alGetError() != AL_NO_ERROR)
			{
				nlwarning("AL: MUSICBUG: Unknown error while trying to play streaming source.");
			}
		}
		else
		{
			nlwarning("AL: MUSICBUG: Trying to play stream with no buffers queued.");
		}
		return true;
		/* OLD
		alSourcePlay(_Source);
		_IsPlaying = (alGetError() == AL_NO_ERROR);
		if (_IsPlaying)
			_StartTime = CTime::getLocalTime(); // TODO: Played time should freeze when buffering fails, and be calculated based on the number of buffers played plus passed time. This is necessary for synchronizing animation with sound.
		return _IsPlaying;
		*/
		// Streaming mode
		//nlwarning("AL: Cannot play null buffer; streaming not implemented" );
		//nlstop;
	}
	else
	{
		nlwarning("Invalid play call, not streaming and no static buffer assigned");
		return false;
	}
}

/// Stop playing
void CSourceAL::stop()
{
	_StartTime = 0;

	if ( _Buffer != NULL )
	{
		// Static playing mode
		_IsPlaying = false;
		_IsPaused = false;
		alSourceStop(_Source);
		alTestError();
	}
	else
	{
		// TODO: Verify streaming mode?
		_IsPlaying = false;
		_IsPaused = false;
		alSourceStop(_Source);
		alTestError();
		// unqueue buffers
		while (_QueuedBuffers.size())
		{
			ALuint bufferName = _QueuedBuffers.front()->bufferName();
			alSourceUnqueueBuffers(_Source, 1, &bufferName);
			_QueuedBuffers.pop();
			alTestError();
		}
		// Streaming mode
		//nlwarning("AL: Cannot stop null buffer; streaming not implemented" );
		//nlstop;
	}
}

/// Pause. Call play() to resume.
void CSourceAL::pause()
{
	if ( _Buffer != NULL )
	{
		if (_IsPaused) nlwarning("AL: Called pause() while _IsPaused == true!");

		// Static playing mode
		if (!isStopped())
		{
			_IsPaused = true;
			alSourcePause(_Source);
			alTestError();
		}
	}
	else
	{
		// TODO: Verify streaming mode?
		_IsPaused = true;
		alSourcePause(_Source);
		alTestError();
		// Streaming mode
		//nlwarning("AL: Cannot pause null buffer; streaming not implemented" );
		//nlstop;
	}
}

/// Return true if play() or pause(), false if stop().
bool CSourceAL::isPlaying() const
{
	//return !isStopped() && !_IsPaused;
	if (_Buffer != NULL)
	{
		ALint srcstate;
		alGetSourcei(_Source, AL_SOURCE_STATE, &srcstate);
		alTestError();
		return (srcstate == AL_PLAYING || srcstate == AL_PAUSED);
	}
	else
	{
		// streaming mode
		return _IsPlaying;
	}
}

/// Return true if playing is finished or stop() has been called.
bool CSourceAL::isStopped() const
{
	if (_Buffer != NULL)
	{
		ALint srcstate;
		alGetSourcei(_Source, AL_SOURCE_STATE, &srcstate);
		alTestError();
		return (srcstate == AL_STOPPED || srcstate == AL_INITIAL);
	}
	else
	{
		// streaming mode
		return !_IsPlaying;
	}
}

/// Return true if the playing source is paused
bool CSourceAL::isPaused() const
{
	if (_Buffer != NULL)
	{
		ALint srcstate;
		alGetSourcei(_Source, AL_SOURCE_STATE, &srcstate);
		alTestError();
		return (srcstate == AL_PAUSED);
	}
	else
	{
		// streaming mode
		return _IsPaused;
	}
}

/// Returns the number of milliseconds the source has been playing
uint32 CSourceAL::getTime()
{
	if (!_StartTime) return 0;
	return (uint32)(CTime::getLocalTime() - _StartTime);
}

/// Set the position vector.
void CSourceAL::setPos(const NLMISC::CVector& pos, bool /* deffered */)
{
	_Pos = pos;
	// Coordinate system: conversion from NeL to OpenAL/GL:
	alSource3f(_Source, AL_POSITION, pos.x, pos.z, -pos.y );
	alTestError();
}

/// Get the position vector.
const NLMISC::CVector &CSourceAL::getPos() const
{
	return _Pos;
}

/// Set the velocity vector (3D mode only)
void CSourceAL::setVelocity( const NLMISC::CVector& vel, bool /* deferred */)
{
	// Coordsys conversion
	alSource3f(_Source, AL_VELOCITY, vel.x, vel.z, -vel.y );
	alTestError();
}

/// Get the velocity vector
void CSourceAL::getVelocity( NLMISC::CVector& vel ) const
{
	ALfloat v[3];
	alGetSourcefv(_Source, AL_VELOCITY, v );
	alTestError();
	// Coordsys conversion
	vel.set( v[0], -v[2], v[1] );
}

/// Set the direction vector (3D mode only)
void CSourceAL::setDirection( const NLMISC::CVector& dir )
{
	// Coordsys conversion
	alSource3f(_Source, AL_DIRECTION, dir.x, dir.z, -dir.y );
	alTestError();
}

/// Get the direction vector
void CSourceAL::getDirection( NLMISC::CVector& dir ) const
{
	ALfloat v[3];
	alGetSourcefv(_Source, AL_DIRECTION, v );
	alTestError();
	// Coordsys conversion
	dir.set( v[0], -v[2], v[1] );
}

/// Set the gain (volume value inside [0 , 1]).
void CSourceAL::setGain(float gain)
{
	_Gain = std::min(std::max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	if (!_SoundDriver->getOption(ISoundDriver::OptionManualRolloff))
	{
		alSourcef(_Source, AL_GAIN, _Gain);
		alTestError();
	}
#ifdef NLSOUND_DEBUG_GAIN
	else
	{
		nlwarning("Called setGain(), manual rolloff, commit deferred to play or update, physical gain is %f, configured gain is %f", gain, _Gain);
	}
#endif
}

/// Get the gain
float CSourceAL::getGain() const
{
#ifdef NLSOUND_DEBUG_GAIN
	ALfloat gain;
	alGetSourcef(_Source, AL_GAIN, &gain);
	nlwarning("Called getGain(), physical gain is %f, configured gain is %f", gain, _Gain);
	alTestError();
#endif
	return _Gain;
}

/// Shift the frequency. 1.0f equals identity, each reduction of 50% equals a pitch shift
void CSourceAL::setPitch(float pitch)
{
	alSourcef(_Source, AL_PITCH, std::min(std::max(pitch, NLSOUND_MIN_PITCH), NLSOUND_MAX_PITCH));
	alTestError();
}

/// Get the pitch
float CSourceAL::getPitch() const
{
	ALfloat pitch;
	alGetSourcef(_Source, AL_PITCH, &pitch);
	alTestError();
	return pitch;
}

/// Set the source relative mode. If true, positions are interpreted relative to the listener position.
void CSourceAL::setSourceRelativeMode( bool mode )
{
	_RelativeMode = mode;
	alSourcei(_Source, AL_SOURCE_RELATIVE, mode?AL_TRUE:AL_FALSE );
	alTestError();
}

/// Get the source relative mode (3D mode only)
bool CSourceAL::getSourceRelativeMode() const
{
	//ALint b;
	//alGetSourcei(_Source, AL_SOURCE_RELATIVE, &b );
	//alTestError();
	//return (b==AL_TRUE);
	return _RelativeMode;
}

/// Set the min and max distances (3D mode only)
void CSourceAL::setMinMaxDistances( float mindist, float maxdist, bool /* deferred */)
{
	nlassert( (mindist >= 0.0f) && (maxdist >= 0.0f) );

	static float maxSqrt = sqrt(std::numeric_limits<float>::max());
 	if (maxdist >= maxSqrt)
 	{
 	       nlwarning("SOUND_DEV (OpenAL): Ridiculously high max distance set on source");
 	       maxdist = maxSqrt;
 	}

	_MinDistance = mindist;
	_MaxDistance = maxdist;
	if (!_SoundDriver->getOption(ISoundDriver::OptionManualRolloff))
	{
		alSourcef(_Source, AL_REFERENCE_DISTANCE, mindist);
		alSourcef(_Source, AL_MAX_DISTANCE, maxdist);
		alTestError();
	}
}

/// Get the min and max distances
void CSourceAL::getMinMaxDistances( float& mindist, float& maxdist ) const
{
	/*alGetSourcef(_Source, AL_REFERENCE_DISTANCE, &mindist );
	alGetSourcef(_Source, AL_MAX_DISTANCE, &maxdist );
	alTestError();*/
	mindist = _MinDistance;
	maxdist = _MaxDistance;
}

/// Set the cone angles (in radian) and gain (in [0 , 1]) (3D mode only)
void CSourceAL::setCone( float innerAngle, float outerAngle, float outerGain )
{
	nlassert( (outerGain >= 0.0f) && (outerGain <= 1.0f ) );
	alSourcef(_Source, AL_CONE_INNER_ANGLE, radToDeg(innerAngle) );
	alSourcef(_Source, AL_CONE_OUTER_ANGLE, radToDeg(outerAngle) );
	alSourcef(_Source, AL_CONE_OUTER_GAIN, outerGain );
	alTestError();
}

/// Get the cone angles (in radian)
void CSourceAL::getCone( float& innerAngle, float& outerAngle, float& outerGain ) const
{
	float ina, outa;
	alGetSourcef(_Source, AL_CONE_INNER_ANGLE, &ina );
	innerAngle = degToRad(ina);
	alGetSourcef(_Source, AL_CONE_OUTER_ANGLE, &outa );
	outerAngle = degToRad(outa);
	alGetSourcef(_Source, AL_CONE_OUTER_GAIN, &outerGain );
	alTestError();
}

/** Set the alpha value for the volume-distance curve
 *
 *	Useful only with OptionManualRolloff. value from -1 to 1 (default 0)
 *
 *  alpha.0: the volume will decrease linearly between 0dB and -100 dB
 *  alpha = 1.0: the volume will decrease linearly between 1.0 and 0.0 (linear scale)
 *  alpha = -1.0: the volume will decrease inversely with the distance (1/dist). This
 *                is the default used by DirectSound/OpenAL
 *
 *  For any other value of alpha, an interpolation is be done between the two
 *  adjacent curves. For example, if alpha equals 0.5, the volume will be halfway between
 *  the linear dB curve and the linear amplitude curve.
 */
void CSourceAL::setAlpha(double a)
{
	_Alpha = a;
}

/// (Internal) Setup the effect send filter.
void CSourceAL::setupDirectFilter()
{
	if (_Direct && _DirectGain > 0)
	{
		if (_DirectGain < 1 || (_DirectFilterPassGain < 1 && _DirectFilterEnabled))
		{
			// direct gain is lowered or a filter is applied
			alFilterf(_DirectFilter, AL_BANDPASS_GAIN, _DirectGain);
			if (_DirectFilterEnabled)
			{
				alFilterf(_DirectFilter, AL_BANDPASS_GAINLF, _DirectFilterPassGain);
				if (_DirectFilterType == FilterBandPass)
					alFilterf(_DirectFilter, AL_BANDPASS_GAINHF, _DirectFilterPassGain);
			}
			else
			{
				alFilterf(_DirectFilter, AL_BANDPASS_GAINLF, 1.0f);
				if (_DirectFilterType == FilterBandPass)
					alFilterf(_DirectFilter, AL_BANDPASS_GAINHF, 1.0f);
			}
			alSourcei(_Source, AL_DIRECT_FILTER, _DirectFilter);
		}
		else
		{
			// no filtering
			alSourcei(_Source, AL_DIRECT_FILTER, AL_FILTER_NULL);
		}
	}
	else
	{
		// mute
		alFilterf(_DirectFilter, AL_BANDPASS_GAIN, 0.0f);
		alSourcei(_Source, AL_DIRECT_FILTER, _DirectFilter);
	}
	alTestError();
}

/// Enable or disable direct output [true/false], default: true
void CSourceAL::setDirect(bool enable)
{
	_Direct = enable;
	setupDirectFilter();
}

/// Return if the direct output is enabled
bool CSourceAL::getDirect() const
{
	return _Direct;
}

/// Set the gain for the direct path
void CSourceAL::setDirectGain(float gain)
{
	_DirectGain = min(max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	setupDirectFilter();
}

/// Get the gain for the direct path
float CSourceAL::getDirectGain() const
{
	return _DirectGain;
}

/// Enable or disable the filter for the direct channel
void CSourceAL::enableDirectFilter(bool enable)
{
	_DirectFilterEnabled = enable;
	setupDirectFilter();
}

/// Check if the filter on the direct channel is enabled
bool CSourceAL::isDirectFilterEnabled() const
{
	return _DirectFilterEnabled;
}

/// Set the filter parameters for the direct channel
void CSourceAL::setDirectFilter(TFilter filterType, float /* lowFrequency */, float /* highFrequency */, float passGain)
{
	_DirectFilterType = filterType;
	_DirectFilterPassGain = passGain;
	switch (filterType)
	{
	case FilterHighPass:
		alFilteri(_DirectFilter, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
		break;
	case FilterLowPass:
		alFilteri(_DirectFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		break;
	case FilterBandPass:
		alFilteri(_DirectFilter, AL_FILTER_TYPE, AL_FILTER_BANDPASS);
		break;
	}
	setupDirectFilter();
}

/// Get the filter parameters for the direct channel
void CSourceAL::getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = _DirectFilterType;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF;
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF;
	passGain = _DirectFilterPassGain;
}

/// Set the direct filter gain
void CSourceAL::setDirectFilterPassGain(float passGain)
{
	_DirectFilterPassGain = min(max(passGain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	setupDirectFilter();
}

/// Get the direct filter gain
float CSourceAL::getDirectFilterPassGain() const
{
	return _DirectFilterPassGain;
}

/// (Internal) Setup the effect send filter.
void CSourceAL::setupEffectFilter()
{
	if (_Effect && _EffectGain > 0)
	{
		if (_EffectGain < 1 || (_EffectFilterPassGain < 1 && _EffectFilterEnabled))
		{
			// effect gain is lowered or a filter is applied
			alFilterf(_EffectFilter, AL_BANDPASS_GAIN, _EffectGain);
			if (_EffectFilterEnabled)
			{
				alFilterf(_EffectFilter, AL_BANDPASS_GAINLF, _EffectFilterPassGain);
				if (_EffectFilterType == FilterBandPass)
					alFilterf(_EffectFilter, AL_BANDPASS_GAINHF, _EffectFilterPassGain);
			}
			else
			{
				alFilterf(_EffectFilter, AL_BANDPASS_GAINLF, 1.0f);
				if (_EffectFilterType == FilterBandPass)
					alFilterf(_EffectFilter, AL_BANDPASS_GAINHF, 1.0f);
			}
			alSource3i(_Source, AL_AUXILIARY_SEND_FILTER, _Effect->getAuxEffectSlot(), 0, _EffectFilter);
		}
		else
		{
			// no filtering
			alSource3i(_Source, AL_AUXILIARY_SEND_FILTER, _Effect->getAuxEffectSlot(), 0, AL_FILTER_NULL);
		}
	}
	else
	{
		// mute
		alSource3i(_Source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
	}
	alTestError();
}

/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
void CSourceAL::setEffect(CEffectAL *effect)
{
	_Effect = effect;
	setupEffectFilter();
}

/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
void CSourceAL::setEffect(IReverbEffect *reverbEffect)
{
	setEffect(reverbEffect ? dynamic_cast<CEffectAL *>(reverbEffect) : NULL);
}

/// Get the effect send for this source
IEffect *CSourceAL::getEffect() const
{
	// return _Effect ? NLMISC::safe_cast<IEffect *>(_Effect) : NULL;
	return NULL;
}

/// Set the gain for the effect path
void CSourceAL::setEffectGain(float gain)
{
	_EffectGain = min(max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	setupEffectFilter();
}

/// Get the gain for the effect path
float CSourceAL::getEffectGain() const
{
	return _EffectGain;
}

/// Enable or disable the filter for the effect channel
void CSourceAL::enableEffectFilter(bool enable)
{
	_EffectFilterEnabled = enable;
	setupEffectFilter();
}

/// Check if the filter on the effect channel is enabled
bool CSourceAL::isEffectFilterEnabled() const
{
	return _EffectFilterEnabled;
}

/// Set the filter parameters for the effect channel
void CSourceAL::setEffectFilter(TFilter filterType, float /* lowFrequency */, float /* highFrequency */, float passGain)
{
	_EffectFilterType = filterType;
	_EffectFilterPassGain = passGain;
	switch (filterType)
	{
	case FilterHighPass:
		alFilteri(_EffectFilter, AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
		break;
	case FilterLowPass:
		alFilteri(_EffectFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		break;
	case FilterBandPass:
		alFilteri(_EffectFilter, AL_FILTER_TYPE, AL_FILTER_BANDPASS);
		break;
	}
	setupEffectFilter();
}

/// Get the filter parameters for the effect channel
void CSourceAL::getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = _EffectFilterType;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF;
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF;
	passGain = _EffectFilterPassGain;
}

/// Set the effect filter gain
void CSourceAL::setEffectFilterPassGain(float passGain)
{
	_EffectFilterPassGain = min(max(passGain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	setupEffectFilter();
}

/// Get the effect filter gain
float CSourceAL::getEffectFilterPassGain() const
{
	return _EffectFilterPassGain;
}

} // NLSOUND
