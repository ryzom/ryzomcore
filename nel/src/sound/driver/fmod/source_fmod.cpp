// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#include "stdfmod.h"
#include "source_fmod.h"
#include "sound_driver_fmod.h"
#include "buffer_fmod.h"
#include "listener_fmod.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace NLMISC;
using namespace std;


namespace NLSOUND
{


// ******************************************************************

CSourceFMod::CSourceFMod( uint sourcename )
:	ISource(),
	_SourceName(sourcename)
{
	_Sample = NULL;
	_NextSample = NULL;
	_State = source_stopped;

	_PosRelative= false;
	_Loop = false;
	_Gain = NLSOUND_DEFAULT_GAIN;
	_Alpha = 0.0;
	_Pos= _Vel= CVector::Null;
	_Front= CVector::J;
	_MinDist= 1.f;
	_MaxDist= numeric_limits<float>::max();
	_Pitch= 1.0f;

	_FModChannel= -1;
}


// ******************************************************************

CSourceFMod::~CSourceFMod()
{
	//nldebug("Destroying FMod source");

	CSoundDriverFMod::getInstance()->removeSource(this);

	release();
}


// ******************************************************************

void CSourceFMod::release()
{
	// stop any pending play
	stop();
}


uint32	CSourceFMod::getTime()
{
	if (_Sample == 0 || _FModChannel==-1)
		return 0;

	TSampleFormat format;
	uint freq;
	_Sample->getFormat(format, freq);

	return uint32(1000.0f * FSOUND_GetCurrentPosition(_FModChannel) / (float)freq);
}

// ******************************************************************

void CSourceFMod::init()
{
}

// ******************************************************************

void CSourceFMod::reset()
{
	setPitch(1.0f);
	setLooping(false);
	setGain(1.0f);
}

/// Enable or disable streaming mode. Source must be stopped to call this.
void CSourceFMod::setStreaming(bool streaming)
{
	if (streaming) throw ESoundDriverNoBufferStreaming();
}

// ******************************************************************
void CSourceFMod::setStaticBuffer( IBuffer *buffer )
{
	if (_State == source_playing)
	{
		_State = source_swap_pending;
		_Sample = 0;
		_NextSample = buffer;
	}
	else
	{
		_Sample = buffer;
		_NextSample = NULL;
	}
}

// ***************************************************************************
IBuffer *CSourceFMod::getStaticBuffer()
{
	if (_State == source_swap_pending)
		return _NextSample;
	else
		return _Sample;

}

/// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming.
/// Should be called by a thread which checks countStreamingBuffers every 100ms.
void CSourceFMod::submitStreamingBuffer(IBuffer * /* buffer */)
{
	throw ESoundDriverNoBufferStreaming();
}

/// Return the amount of buffers in the queue (playing and waiting). 3 buffers is optimal.
uint CSourceFMod::countStreamingBuffers() const
{
	throw ESoundDriverNoBufferStreaming();
}

// ***************************************************************************
bool CSourceFMod::play()
{
	// stop any current sound
	stop();

	// play the new one
	if(_Sample)
	{
		CBufferFMod	*bufFMod= static_cast<CBufferFMod*>(_Sample);
		_State= source_playing;
		// start paused, for Loop behaviour to work properly
		if(bufFMod->_FModSample)
			_FModChannel= FSOUND_PlaySoundEx(FSOUND_FREE, bufFMod->_FModSample, NULL, true);

		// Update all setup for this channel
		if(_FModChannel!=-1)
		{
			FSOUND_SetLoopMode(_FModChannel, _Loop?FSOUND_LOOP_NORMAL:FSOUND_LOOP_OFF);
			FSOUND_3D_SetMinMaxDistance(_FModChannel, _MinDist, _MaxDist);
			updateFModPos();
			// reset pitch
			setPitch(_Pitch);

			// Set the correct volume now
			if (!CSoundDriverFMod::getInstance()->getOption(ISoundDriver::OptionManualRolloff))
			{
				FSOUND_SetVolume(_FModChannel, uint32(255*_Gain));
			}
			else
			{
				// manual rolloff => recompute according to position
				CListenerFMod *listener = CListenerFMod::getInstance();
				if (listener) updateVolume(listener->getPos());
			}

			// unpause
			FSOUND_SetPaused(_FModChannel, false);
		}
	}

	return true;
}

// ***************************************************************************
void CSourceFMod::stop()
{
	if(_State == source_swap_pending)
	{
		_State= source_playing;
		_Sample= _NextSample;
		_NextSample= NULL;
	}

	_State= source_stopped;

	// Stop the FMod channel
	if(_FModChannel!=-1)
	{
		FSOUND_StopSound(_FModChannel);
		_FModChannel= -1;
	}
}


// ******************************************************************

void CSourceFMod::setLooping( bool l )
{
	if(_Loop!=l)
	{
		_Loop = l;
		if(_FModChannel!=-1)
		{
			// Must pause/unpause (Hardware limitation)
			FSOUND_SetPaused(_FModChannel, true);
			FSOUND_SetLoopMode(_FModChannel, _Loop?FSOUND_LOOP_NORMAL:FSOUND_LOOP_OFF);
			FSOUND_SetPaused(_FModChannel, false);
		}
	}
}


// ******************************************************************

bool CSourceFMod::getLooping() const
{
	return _Loop;
}

// ******************************************************************
void CSourceFMod::pause()
{
	// TODO : recode this !
	nlassert(false);
}

// ******************************************************************

bool CSourceFMod::isPlaying() const
{
	return _State == source_playing || _State == source_swap_pending;
}


// ******************************************************************

bool CSourceFMod::isPaused() const
{
	// TODO
	nlassert(false);
	return false;
}


// ******************************************************************

bool CSourceFMod::isStopped() const
{
	return _State == source_silencing || _State == source_stopped;
}


// ******************************************************************
bool CSourceFMod::needsUpdate()
{
	return _State == source_silencing || _State == source_playing || _State == source_swap_pending;
}



// ******************************************************************

bool CSourceFMod::update()
{
	// don't stop if loop
	if(_FModChannel!=-1 && isPlaying())
	{
		// If FMod ended
		if(!FSOUND_IsPlaying(_FModChannel))
			_State= source_silencing;
	}

	return true;
}


// ******************************************************************

void CSourceFMod::setPos( const NLMISC::CVector& pos, bool /* deferred */ )
{
	_Pos = pos;
	updateFModPos();
}


// ******************************************************************

const NLMISC::CVector &CSourceFMod::getPos() const
{
	return _Pos;
}


// ******************************************************************

void CSourceFMod::setVelocity( const NLMISC::CVector& vel, bool /* deferred */ )
{
	_Vel= vel;
	updateFModPos();
}


// ******************************************************************

void CSourceFMod::getVelocity( NLMISC::CVector& vel ) const
{
	vel= _Vel;
}


// ******************************************************************

void CSourceFMod::setDirection( const NLMISC::CVector& dir )
{
	_Front= dir;
	updateFModPos();
}


// ******************************************************************

void CSourceFMod::getDirection( NLMISC::CVector& dir ) const
{
	dir= _Front;
}


// ******************************************************************

void CSourceFMod::setGain( float gain )
{
	clamp(gain, 0.00001f, 1.0f);
	_Gain = gain;

	if (!CSoundDriverFMod::getInstance()->getOption(ISoundDriver::OptionManualRolloff))
	{
		if(_FModChannel!=-1)
			FSOUND_SetVolume(_FModChannel, uint32(255*gain));
	}
	// set the volume later in updateVolume() in case of OptionManualRolloff
}


// ***************************************************************************
float CSourceFMod::getGain() const
{
	return _Gain;
}


// ******************************************************************
void CSourceFMod::setPitch( float coeff )
{
	_Pitch= coeff;

	if (_Sample != NULL && _FModChannel!=-1)
	{
		TSampleFormat format;
		uint freq;

		_Sample->getFormat(format, freq);

		uint32 newFreq = (uint32) (coeff * (float) freq);

		FSOUND_SetFrequency(_FModChannel, newFreq);
	}
}


// ******************************************************************
float CSourceFMod::getPitch() const
{
	return _Pitch;
}


// ******************************************************************
void CSourceFMod::setSourceRelativeMode( bool mode )
{
	_PosRelative= mode;
	updateFModPos();
}


// ******************************************************************

bool CSourceFMod::getSourceRelativeMode() const
{
	return _PosRelative;
}


// ******************************************************************
void CSourceFMod::setMinMaxDistances( float mindist, float maxdist, bool /* deferred */ )
{
	static float maxSqrt = sqrt(std::numeric_limits<float>::max());
	if (maxdist >= maxSqrt)
	{
		nlwarning("SOUND_DEV (FMod): Ridiculously high max distance set on source");
		maxdist = maxSqrt;
	}

	_MinDist= mindist;
	_MaxDist= maxdist;
	if(_FModChannel!=-1)
	{
		FSOUND_3D_SetMinMaxDistance(_FModChannel, _MinDist, _MaxDist);
	}
}


// ******************************************************************
void CSourceFMod::getMinMaxDistances( float& mindist, float& maxdist ) const
{
	mindist= _MinDist;
	maxdist= _MaxDist;
}

// ******************************************************************
void CSourceFMod::updateVolume( const NLMISC::CVector& listener )
{
	nlassert(CSoundDriverFMod::getInstance()->getOption(ISoundDriver::OptionManualRolloff));

	// only if channel active
	if(_FModChannel==-1)
		return;

	CVector pos = getPos();
	// make relative to listener (if not already!)
	if(!_PosRelative)
		pos -= listener;
	float sqrdist = pos.sqrnorm();

	// compute volume in DB, according to current gain
	//sint32 volumeDB= sint32(floor(2000.0 * log10(_Gain))); // convert to 1/100th decibels
	//const	sint32	dbMin= -10000;
	//const	sint32	dbMax= 0;
	//clamp(volumeDB, dbMin, dbMax);

	//// attenuate the volume according to distance and alpha
	//volumeDB= ISource::computeManualRollOff(volumeDB, dbMin, dbMax, _Alpha, sqrdist);

	//// retransform to linear form
	//double	attGain= pow((double)10.0, double(volumeDB)/2000.0);
	//clamp(attGain, 0.f, 1.f);
	
	float rolloff = ISource::computeManualRolloff(_Alpha, sqrdist, _MinDist, _MaxDist);
	float volume = _Gain * rolloff;

	// set the attenuated volume
	FSOUND_SetVolume(_FModChannel, (sint)(volume * 255));
}

// ******************************************************************

void CSourceFMod::setCone( float /* innerAngle */, float /* outerAngle */, float /* outerGain */ )
{
	// TODO_SOURCE_DIR
}

// ******************************************************************

void CSourceFMod::getCone( float& /* innerAngle */, float& /* outerAngle */, float& /* outerGain */ ) const
{
	// TODO_SOURCE_DIR
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
void CSourceFMod::setAlpha(double a)
{
	_Alpha = a;
}

// ***************************************************************************
void CSourceFMod::updateFModPos()
{
	// TODO_SOURCE_DIR
	if(_FModChannel!=-1)
	{
		CVector		wpos, wvel;
		wpos= _Pos;
		wvel= _Vel;

		// If relative, must transform to absolute
		if(_PosRelative)
		{
			CListenerFMod	*lsr= CListenerFMod::getInstance();
			if(lsr)
			{
				wpos= lsr->getPosMatrix() * wpos;
				wvel= lsr->getPosMatrix() * wvel;
			}
		}

		// set World Pos/Vel to FMod
		float		fmodPos[3];
		float		fmodVel[3];
		CSoundDriverFMod::toFModCoord(wpos, fmodPos);
		CSoundDriverFMod::toFModCoord(wvel, fmodVel);
		FSOUND_3D_SetAttributes(_FModChannel, fmodPos, fmodVel);
	}
}


// ***************************************************************************
void CSourceFMod::updateFModPosIfRelative()
{
	// Not supported by FMod, emulate each frame (before FSOUND_update())
	if(_PosRelative)
		updateFModPos();
}

/// Enable or disable direct output [true/false], default: true
void CSourceFMod::setDirect(bool /* enable */)
{
	
}

/// Return if the direct output is enabled
bool CSourceFMod::getDirect() const
{
	return true;
}

/// Set the gain for the direct path
void CSourceFMod::setDirectGain(float /* gain */)
{
	
}

/// Get the gain for the direct path
float CSourceFMod::getDirectGain() const
{
	return NLSOUND_DEFAULT_DIRECT_GAIN;
}

/// Enable or disable the filter for the direct channel
void CSourceFMod::enableDirectFilter(bool /* enable */)
{
	
}

/// Check if the filter on the direct channel is enabled
bool CSourceFMod::isDirectFilterEnabled() const
{
	return false;
}

/// Set the filter parameters for the direct channel
void CSourceFMod::setDirectFilter(TFilter /*filterType*/, float /*lowFrequency*/, float /*highFrequency*/, float /*passGain*/)
{
	
}

/// Get the filter parameters for the direct channel
void CSourceFMod::getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = FilterLowPass;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF; 
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF; 
	passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN;
}

/// Set the direct filter gain
void CSourceFMod::setDirectFilterPassGain(float /*passGain*/)
{
	
}

/// Get the direct filter gain
float CSourceFMod::getDirectFilterPassGain() const
{
	return 0.0f;
}

/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
void CSourceFMod::setEffect(IReverbEffect * /* reverbEffect */)
{
	
}

/// Get the effect send for this source
IEffect *CSourceFMod::getEffect() const
{
	return NULL;
}

/// Set the gain for the effect path
void CSourceFMod::setEffectGain(float /* gain */)
{
	
}

/// Get the gain for the effect path
float CSourceFMod::getEffectGain() const
{
	return NLSOUND_DEFAULT_EFFECT_GAIN;
}

/// Enable or disable the filter for the effect channel
void CSourceFMod::enableEffectFilter(bool /* enable */)
{
	
}

/// Check if the filter on the effect channel is enabled
bool CSourceFMod::isEffectFilterEnabled() const
{
	return false;
}

/// Set the filter parameters for the effect channel
void CSourceFMod::setEffectFilter(TFilter /*filterType*/, float /*lowFrequency*/, float /*highFrequency*/, float /*passGain*/)
{
	
}

/// Get the filter parameters for the effect channel
void CSourceFMod::getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	filterType = FilterLowPass;
	lowFrequency = NLSOUND_DEFAULT_FILTER_PASS_LF; 
	highFrequency = NLSOUND_DEFAULT_FILTER_PASS_HF; 
	passGain = NLSOUND_DEFAULT_FILTER_PASS_GAIN;
}

/// Set the effect filter gain
void CSourceFMod::setEffectFilterPassGain(float /*passGain*/)
{
	
}

/// Get the effect filter gain
float CSourceFMod::getEffectFilterPassGain() const
{
	return 0.0f;
}

} // NLSOUND
