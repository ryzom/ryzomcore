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

// curve cone eax time

#include "stdxaudio2.h"

// Project includes
#include "sound_driver_xaudio2.h"
#include "buffer_xaudio2.h"
#include "listener_xaudio2.h"
#include "adpcm_xaudio2.h"
#include "effect_xaudio2.h"
#include "source_xaudio2.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CSourceXAudio2::CSourceXAudio2(CSoundDriverXAudio2 *soundDriver) 
: _SoundDriver(soundDriver), _SourceVoice(NULL), _StaticBuffer(NULL), _OperationSet(soundDriver->getUniqueOperationSet()), 
_Format(IBuffer::FormatUnknown), _Frequency(0), _PlayStart(0), 
_Doppler(1.0f), _Pos(0.0f, 0.0f, 0.0f), _Relative(false), _Alpha(1.0), 
_DirectDryVoice(NULL), _DirectFilterVoice(NULL), _EffectDryVoice(NULL), _EffectFilterVoice(NULL), 
_DirectDryEnabled(false), _DirectFilterEnabled(false), _EffectDryEnabled(false), _EffectFilterEnabled(false), 
_DirectGain(NLSOUND_DEFAULT_DIRECT_GAIN), _EffectGain(NLSOUND_DEFAULT_EFFECT_GAIN), 
_DirectFilterPassGain(NLSOUND_DEFAULT_FILTER_PASS_GAIN), _EffectFilterPassGain(NLSOUND_DEFAULT_FILTER_PASS_GAIN), 
_DirectFilterLowFrequency(NLSOUND_DEFAULT_FILTER_PASS_LF), _DirectFilterHighFrequency(NLSOUND_DEFAULT_FILTER_PASS_HF), 
_EffectFilterLowFrequency(NLSOUND_DEFAULT_FILTER_PASS_LF), _EffectFilterHighFrequency(NLSOUND_DEFAULT_FILTER_PASS_HF), 
_IsPlaying(false), _IsPaused(false), _IsLooping(false), _Pitch(1.0f), 
_Gain(1.0f), _MinDistance(1.0f), _MaxDistance(sqrt(numeric_limits<float>::max())),
_AdpcmUtility(NULL), _Channels(0), _BitsPerSample(0), _BufferStreaming(false)
{
	// nlwarning(NLSOUND_XAUDIO2_PREFIX "Inititializing CSourceXAudio2");
	nlassert(_SoundDriver->getListener());

	memset(&_Emitter, 0, sizeof(_Emitter));
	memset(&_Cone, 0, sizeof(_Cone));
	memset(&_DirectFilter, 0, sizeof(_DirectFilter));
	memset(&_EffectFilter, 0, sizeof(_EffectFilter));

	_Cone.InnerAngle = X3DAUDIO_2PI;
	_Cone.OuterAngle = X3DAUDIO_2PI;
	_Cone.InnerVolume = 1.0f;
	_Cone.OuterVolume = 0.0f;
	_Cone.InnerLPF = 1.0f;
	_Cone.OuterLPF = 0.0f;
	_Cone.InnerReverb = 1.0f;
	_Cone.OuterReverb = 0.0f;

	_Emitter.OrientFront.x = 0.0f;
	_Emitter.OrientFront.y = 0.0f;
	_Emitter.OrientFront.z = 1.0f;
	_Emitter.OrientTop.x = 0.0f;
	_Emitter.OrientTop.y = 1.0f;
	_Emitter.OrientTop.z = 0.0f;
	_Emitter.Position.x = 0.0f;
	_Emitter.Position.y = 0.0f;
	_Emitter.Position.z = 0.0f;
	_Emitter.Velocity.x = 0.0f;
	_Emitter.Velocity.y = 0.0f;
	_Emitter.Velocity.z = 0.0f;
	_Emitter.ChannelCount = 1;
	_Emitter.InnerRadius = 0.0f;
	_Emitter.InnerRadiusAngle = 0.0f;
	_Emitter.ChannelRadius = 0.0f;
	_Emitter.CurveDistanceScaler = 1.0f;
	_Emitter.DopplerScaler = 1.0f;

	_DirectFilter.Frequency = 1.0f;
	_DirectFilter.OneOverQ = 1.0f;
	_DirectFilter.Type = LowPassFilter;

	_EffectFilter.Frequency = 1.0f;
	_EffectFilter.OneOverQ = 1.0f;
	_EffectFilter.Type = LowPassFilter;

	_DirectDryVoice = _SoundDriver->getListener()->getDryVoice();
	_DirectDryEnabled = true;
	_DirectFilterVoice = _SoundDriver->getListener()->getFilterVoice();
	_DirectFilterEnabled = false;
}

CSourceXAudio2::~CSourceXAudio2()
{
	// nlwarning(NLSOUND_XAUDIO2_PREFIX "Destroying CSourceXAudio2");
	CSoundDriverXAudio2 *soundDriver = _SoundDriver;
	release();
	if (soundDriver) soundDriver->removeSource(this);
}

void CSourceXAudio2::release() // called by driver or destructor, whichever is first
{
	if (_AdpcmUtility) { _AdpcmUtility->flushSourceBuffers(); }
	if (_SoundDriver) 
	{
		_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
		if (_SourceVoice) { _SoundDriver->destroySourceVoice(_SourceVoice); _SourceVoice = NULL; }
	}
	if (_AdpcmUtility) { delete _AdpcmUtility; _AdpcmUtility = NULL; }
	stop();
	_SoundDriver = NULL;
}

/// Commit all the changes made to 3D settings of listener and sources
void CSourceXAudio2::commit3DChanges()
{
	nlassert(_SourceVoice);
	
	// Only mono buffers get 3d sound, multi-channel buffers go directly to the speakers without any distance rolloff.
	if (_Channels > 1)
	{
		// _SoundDriver->getDSPSettings()->DstChannelCount = 1;
		// _Emitter.pVolumeCurve = NULL; // todo: everything
		// calculate without doppler
		// 1 result in matrix, use with setvolume
		// todo: some more stuff...
		// this isn't really used anyways
		_SourceVoice->SetFrequencyRatio(_Pitch, _OperationSet);
		// nlerror(NLSOUND_XAUDIO2_PREFIX "Stereo16 and Stereo8 not fully implemented, have fun! :)");
	}
	else
	{
		XAUDIO2_VOICE_DETAILS voiceDetails;
		_SoundDriver->getMasteringVoice()->GetVoiceDetails(&voiceDetails);

		FLOAT32 matrixCoefficients[32 * 32]; // technical limit is 32 speakers
		X3DAUDIO_DSP_SETTINGS dspSettings = { 0 };
		dspSettings.pMatrixCoefficients = matrixCoefficients;
		dspSettings.SrcChannelCount = 1;
		dspSettings.DstChannelCount = voiceDetails.InputChannels;
		// // nldebug(NLSOUND_XAUDIO2_PREFIX "_SampleVoice->getBuffer() %u", (uint32)_SampleVoice->getBuffer());
		
		_Emitter.DopplerScaler = _SoundDriver->getListener()->getDopplerScaler();

		X3DAUDIO_DISTANCE_CURVE_POINT curve_points[2];
		X3DAUDIO_DISTANCE_CURVE curve = { (X3DAUDIO_DISTANCE_CURVE_POINT *)&curve_points[0], 2 };
		
		if (_SoundDriver->getOption(ISoundDriver::OptionManualRolloff))
		{
			float sqrdist = _Relative 
				? getPos().sqrnorm()
				: (getPos() - _SoundDriver->getListener()->getPos()).sqrnorm();
			float rolloff = ISource::computeManualRolloff(_Alpha, sqrdist, _MinDistance, _MaxDistance);
			curve_points[0].Distance = 0.f; 
			curve_points[0].DSPSetting = rolloff;
			curve_points[1].Distance = 1.f; 
			curve_points[1].DSPSetting = rolloff;
			_Emitter.pVolumeCurve = &curve;
			_Emitter.pLFECurve = &curve;
		}
		else
		{
			// divide min distance (distance from where to start attenuation) with rolloff scaler (factor to get faster attenuation)
			_Emitter.CurveDistanceScaler = _MinDistance / _SoundDriver->getListener()->getRolloffScaler();
			// _MaxDistance not implemented (basically should cut off sound beyond maxdistance)
		}
		
		X3DAudioCalculate(_SoundDriver->getX3DAudio(), 
			_Relative 
				? _SoundDriver->getEmptyListener() // position is relative to listener (we use 0pos listener)
				: _SoundDriver->getListener()->getListener(), // position is absolute
			&_Emitter, 
			X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER, 
			&dspSettings);
		
		FLOAT32 outputMatrix[32 * 32];

		if (_DirectDryEnabled)
		{
			float directDryGain = _DirectFilterEnabled
				? _DirectFilterPassGain * _DirectGain
				: _DirectGain;
			for (uint32 i = 0; i < dspSettings.DstChannelCount; ++i)
				outputMatrix[i] = matrixCoefficients[i] * directDryGain;
			_SourceVoice->SetOutputMatrix(_DirectDryVoice, 1, dspSettings.DstChannelCount, outputMatrix, _OperationSet);
			if (_DirectFilterEnabled)
			{
				float directFilterGain = _DirectGain - directDryGain;
				for (uint32 i = 0; i < dspSettings.DstChannelCount; ++i)
					outputMatrix[i] = matrixCoefficients[i] * directFilterGain;
				_SourceVoice->SetOutputMatrix(_DirectFilterVoice, 1, dspSettings.DstChannelCount, outputMatrix, _OperationSet);
			}
		}

		if (_EffectDryEnabled)
		{
			float monoRolloff = 0.0f;
			for (uint32 i = 0; i < dspSettings.DstChannelCount; ++i)
				monoRolloff += matrixCoefficients[i];
			monoRolloff /= (float)dspSettings.DstChannelCount;

			float effectDryGain = _EffectFilterEnabled
				? _EffectFilterPassGain  * _EffectGain
				: _EffectGain;
			float outputSingle = monoRolloff * effectDryGain;
			_SourceVoice->SetOutputMatrix(_EffectDryVoice, 1, 1, &outputSingle, _OperationSet);
			if (_EffectFilterEnabled)
			{
				float effectFilterGain = _EffectGain - effectDryGain;
				outputSingle = monoRolloff * effectFilterGain;
				_SourceVoice->SetOutputMatrix(_EffectFilterVoice, 1, 1, &outputSingle, _OperationSet);
			}
		}

		// nldebug(NLSOUND_XAUDIO2_PREFIX "left: %f, right %f", _SoundDriver->getDSPSettings()->pMatrixCoefficients[0], _SoundDriver->getDSPSettings()->pMatrixCoefficients[1]);
		_Doppler = dspSettings.DopplerFactor;
		_SourceVoice->SetFrequencyRatio(_Pitch * _Doppler);
	}
	_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
	// todo: delay?
}

void CSourceXAudio2::update3DChanges()
{
	if (_IsPlaying)
	{
		commit3DChanges();
	}
}

void CSourceXAudio2::updateState()
{
	if (_IsPlaying)
	{
		if (_AdpcmUtility)
		{
			if (!_AdpcmUtility->getSourceData())
			{
				_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
				if (!_BufferStreaming)
				{
					// nldebug(NLSOUND_XAUDIO2_PREFIX "Stop");
					if (FAILED(_SourceVoice->Stop(0))) 
						nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Stop");
					_IsPlaying = false;
				}
			}
		}
		else
		{
			XAUDIO2_VOICE_STATE voice_state;
			_SourceVoice->GetState(&voice_state);
			if (!voice_state.BuffersQueued)
			{
				_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
				if (!_BufferStreaming)
				{
					// nldebug(NLSOUND_XAUDIO2_PREFIX "Stop");
					if (FAILED(_SourceVoice->Stop(0))) 
						nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Stop");
					_IsPlaying = false;
				}
			}
		}
	}
}

/// (Internal) Submit a buffer to the XAudio2 source voice.
void CSourceXAudio2::submitBuffer(CBufferXAudio2 *ibuffer)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "submitBuffer %u", (uint32)(void *)this);

	nlassert(_SourceVoice);
	nlassert(ibuffer->getFormat() == _Format
		&& ibuffer->getChannels() == _Channels
		&& ibuffer->getBitsPerSample() == _BitsPerSample);
	if (_AdpcmUtility)
	{
		nlassert(!_BufferStreaming);
		_AdpcmUtility->submitSourceBuffer(ibuffer);
	}
	else
	{
		XAUDIO2_BUFFER buffer;
		buffer.AudioBytes = ibuffer->getSize();
		buffer.Flags = (_IsLooping || _BufferStreaming) ? 0 : XAUDIO2_END_OF_STREAM;
		buffer.LoopBegin = 0;
		buffer.LoopCount = (_IsLooping && !_BufferStreaming) ? XAUDIO2_LOOP_INFINITE : 0;
		buffer.LoopLength = 0;
		buffer.pAudioData = const_cast<BYTE *>(ibuffer->getData());
		buffer.pContext = ibuffer;
		buffer.PlayBegin = 0;
		buffer.PlayLength = 0;
		
		_SourceVoice->SubmitSourceBuffer(&buffer);
	}
}

/// (Internal) Update the send descriptor
void CSourceXAudio2::setupVoiceSends()
{	
	XAUDIO2_SEND_DESCRIPTOR sendDescriptors[4];
	XAUDIO2_VOICE_SENDS voiceSends;
	voiceSends.pSends = sendDescriptors;
	voiceSends.SendCount = 0;
	
	if (_DirectDryEnabled)
	{
		sendDescriptors[0].Flags = 0;
		sendDescriptors[0].pOutputVoice = _DirectDryVoice;
		if (_DirectFilterEnabled)
		{
			sendDescriptors[1].Flags = XAUDIO2_SEND_USEFILTER;
			sendDescriptors[1].pOutputVoice = _DirectFilterVoice;
			voiceSends.SendCount = 2;
		}
		else voiceSends.SendCount = 1;
	}
	
	if (_EffectDryEnabled)
	{
		sendDescriptors[voiceSends.SendCount].Flags = 0;
		sendDescriptors[voiceSends.SendCount].pOutputVoice = _EffectDryVoice;
		++voiceSends.SendCount;
		if (_EffectFilterEnabled)
		{
			sendDescriptors[voiceSends.SendCount].Flags = XAUDIO2_SEND_USEFILTER;
			sendDescriptors[voiceSends.SendCount].pOutputVoice = _EffectFilterVoice;
			++voiceSends.SendCount;
		}
	}
	
	_SoundDriver->getXAudio2()->CommitChanges(_OperationSet); // SetOutputVoices does not support OperationSet
	_SourceVoice->SetOutputVoices(&voiceSends); // SetOutputVoices does not support OperationSet
	setupDirectFilter();
	setupEffectFilter();
	_SoundDriver->getXAudio2()->CommitChanges(_OperationSet); // SetOutputVoices does not support OperationSet
}

/// Enable or disable streaming mode. Source must be stopped to call this.
void CSourceXAudio2::setStreaming(bool streaming)
{
	nlassert(!_IsPlaying);

	// nldebug(NLSOUND_XAUDIO2_PREFIX "setStreaming %i", (uint32)streaming);

	if (_SourceVoice)
	{
		XAUDIO2_VOICE_STATE voice_state;
		_SourceVoice->GetState(&voice_state);
		if (!voice_state.BuffersQueued)
		{
			nlwarning(NLSOUND_XAUDIO2_PREFIX "Switched streaming mode while buffer still queued!?! Flush");
			_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
			if (FAILED(_SourceVoice->FlushSourceBuffers())) 
				nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED FlushSourceBuffers");
		}
	}

	_BufferStreaming = streaming;

	// nldebug(NLSOUND_XAUDIO2_PREFIX "setStreaming done %i", (uint32)streaming);
}

/// Set the buffer that will be played (no streaming)
void CSourceXAudio2::setStaticBuffer(IBuffer *buffer)
{
	nlassert(!_BufferStreaming);

	// nldebug(NLSOUND_XAUDIO2_PREFIX "setStaticBuffer");

	// if (buffer) // nldebug(NLSOUND_XAUDIO2_PREFIX "setStaticBuffer %s", _SoundDriver->getStringMapper()->unmap(buffer->getName()).c_str());
	// else // nldebug(NLSOUND_XAUDIO2_PREFIX "setStaticBuffer NULL");

	// if (_IsPlaying) nlwarning(NLSOUND_XAUDIO2_PREFIX "Called setStaticBuffer(IBuffer *buffer) while _IsPlaying == true!");
	
	_StaticBuffer = static_cast<CBufferXAudio2 *>(buffer);
}

/// Return the buffer, or NULL if streaming is used.
IBuffer *CSourceXAudio2::getStaticBuffer()
{
	nlassert(!_BufferStreaming); // can be implemented trough voice_state.pCurrentBufferContext

	return _StaticBuffer;
}

/// Add a buffer to the streaming queue.  A buffer of 100ms length is optimal for streaming.
/// Should be called by a thread which checks countStreamingBuffers every 100ms.
void CSourceXAudio2::submitStreamingBuffer(IBuffer *buffer)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "submitStreamingBuffer");

	nlassert(_BufferStreaming);

	// allow to change the format if not playing
	if (!_IsPlaying)
	{
		IBuffer::TBufferFormat bufferFormat;
		uint8 channels;
		uint8 bitsPerSample;
		uint32 frequency;
		buffer->getFormat(bufferFormat, channels, bitsPerSample, frequency);
		preparePlay(bufferFormat, channels, bitsPerSample, frequency);
	}
	
	submitBuffer(static_cast<CBufferXAudio2 *>(buffer));
}

/// Return the amount of buffers in the queue (playing and waiting). 3 buffers is optimal.
uint CSourceXAudio2::countStreamingBuffers() const
{
	nlassert(_BufferStreaming);
	
	if (_SourceVoice)
	{
		XAUDIO2_VOICE_STATE voice_state;
		_SourceVoice->GetState(&voice_state);
		return voice_state.BuffersQueued;
	}
	else
	{
		return 0;
	}
}

/// Set looping on/off for future playbacks (default: off)
void CSourceXAudio2::setLooping(bool l)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setLooping %u", (uint32)l);

	nlassert(!_BufferStreaming);

	if (_IsLooping != l)
	{
		_IsLooping = l;
		if (_AdpcmUtility)
		{
			_AdpcmUtility->setLooping(l);
		}
		else
		{
			if (_SourceVoice)
			{
				if (_IsPlaying)
				{
					if (l)
					{
						// loop requested while already playing, flush any trash
						if (FAILED(_SourceVoice->FlushSourceBuffers())) 
							nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED FlushSourceBuffers");
						// resubmit with updated looping settings
						submitBuffer(_StaticBuffer);
					}
					else
					{
						// flush any queued buffers, keep playing current buffer
						if (FAILED(_SourceVoice->FlushSourceBuffers())) 
							nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED FlushSourceBuffers");
						// exit loop of current buffer
						if (FAILED(_SourceVoice->ExitLoop()))
							nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED ExitLoop");
					}
				}
				else
				{
					// not playing but buffer already queued
					XAUDIO2_VOICE_STATE voice_state;
					_SourceVoice->GetState(&voice_state);
					if (voice_state.BuffersQueued)
					{
						nlwarning(NLSOUND_XAUDIO2_PREFIX "Not playing but buffer already queued while switching loop mode!?! Flush and requeue");
						if (FAILED(_SourceVoice->FlushSourceBuffers())) 
							nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED FlushSourceBuffers");
						// queue buffer with correct looping parameters
						submitBuffer(_StaticBuffer);
					}
				}
			}
		}
	}
}

/// Return the looping state
bool CSourceXAudio2::getLooping() const
{
	nlassert(!_BufferStreaming);

	return _IsLooping;
}

/// (Internal) Initialize voice with this format, if no voice has been created yet.
bool CSourceXAudio2::initFormat(IBuffer::TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample)
{
	// nlwarning(NLSOUND_XAUDIO2_PREFIX "New voice with format %u!", (uint32)_StaticBuffer->getFormat());

	nlassert(!_SourceVoice); nlassert(!_AdpcmUtility);
	// create adpcm utility callback if needed
	if (bufferFormat == IBuffer::FormatDviAdpcm) _AdpcmUtility = new CAdpcmXAudio2(_IsLooping);
	// create voice with adpcm utility callback or NULL callback
	_SourceVoice = _SoundDriver->createSourceVoice(bufferFormat, channels, bitsPerSample, _AdpcmUtility);
	if (_AdpcmUtility) _AdpcmUtility->setSourceVoice(_SourceVoice);
	if (!_SourceVoice) return false; // fail
	_Format = bufferFormat;
	_Channels = channels;
	_BitsPerSample = bitsPerSample;
	_SourceVoice->SetVolume(_Gain, _OperationSet);
	setupVoiceSends();
	_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
	
	// Also commit any 3D settings that were already done
	commit3DChanges();
	
	// test
	//XAUDIO2_VOICE_DETAILS voice_details;
	//_SourceVoice->GetVoiceDetails(&voice_details);
	//_SendDescriptors[0].Flags = XAUDIO2_SEND_USEFILTER;
	//_SourceVoice->SetOutputVoices(&_VoiceSends);
	//XAUDIO2_FILTER_PARAMETERS _FilterParameters[2];
	//_FilterParameters[0].Type = LowPassFilter;
	//nlinfo("voice_details.InputSampleRate: '%u'", voice_details.InputSampleRate);
	//_FilterParameters[0].Frequency = XAudio2CutoffFrequencyToRadians(5000, voice_details.InputSampleRate);
	//nlinfo("_FilterParameters[0].Frequency: '%f'", _FilterParameters[0].Frequency);
	//_FilterParameters[0].OneOverQ = 1.0f;
	//nlinfo("_FilterParameters[0].OneOverQ: '%f'", _FilterParameters[0].OneOverQ);
	//_SourceVoice->SetOutputFilterParameters(_SendDescriptors[0].pOutputVoice, &_FilterParameters[0]);
	// test


	return true;
}

/// (Internal) Prepare to play. Stop the currently playing buffers, and set the correct voice settings.
bool CSourceXAudio2::preparePlay(IBuffer::TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, uint32 frequency)
{
	if (_IsPlaying)
	{
		// nlwarning(NLSOUND_XAUDIO2_PREFIX "Called play() while _IsPlaying == true!");
		// stop the currently playing voice if it's of the same type as we need
		if (bufferFormat == _Format && channels == _Channels && bitsPerSample == _BitsPerSample) 
			// cannot call stop directly before destroy voice, ms bug in xaudio2, see msdn docs
			stop(); // sets _IsPlaying = false;
	}
	if (_SourceVoice && (bufferFormat != _Format || channels != _Channels || bitsPerSample != _BitsPerSample))
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "Switching format %u to %u!", (uint32)_Format, (uint32)bufferFormat);
		// destroy existing voice
		_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
		_SoundDriver->destroySourceVoice(_SourceVoice); _SourceVoice = NULL;
		// destroy adpcm utility (if it exists)
		delete _AdpcmUtility; _AdpcmUtility = NULL;
		// reset current stuff
		_Format = IBuffer::FormatNotSet;
		_Channels = 0;
		_BitsPerSample = 0;
	}
	if (frequency != _Frequency)
	{
		if (_SourceVoice) 
		{
			setupDirectFilter();
			setupEffectFilter();
			_SourceVoice->SetSourceSampleRate(frequency);
		}
		_Frequency = frequency;
	}
	if (!_SourceVoice)
	{
		// initialize a source voice with this format
		if (!initFormat(bufferFormat, channels, bitsPerSample))
		{
			nlwarning(NLSOUND_XAUDIO2_PREFIX "Fail to init voice!");
			return false;
		}
		_SourceVoice->SetSourceSampleRate(frequency);
		if (!_BufferStreaming)
		{
			// notify other sources about this format, so they can make a voice in advance
			// we know that there is a very high chance that all sources use same format
			// so this gives better results at runtime (avoids sound clicks etc)
			_SoundDriver->initSourcesFormat(_Format, _Channels, _BitsPerSample);
		}
	}
	commit3DChanges(); // sets pitch etc

	// test
	//XAUDIO2_VOICE_DETAILS voice_details;
	//_SourceVoice->GetVoiceDetails(&voice_details);
	//nlinfo("voice_details.InputSampleRate: '%u'", voice_details.InputSampleRate);
	// test

	return true;
}

/// Play the static buffer (or stream in and play).
bool CSourceXAudio2::play()
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "play");

	// Commit 3D changes before starting play
	if (_SourceVoice)
		commit3DChanges();
	// else it is commit by the preparePlay > initFormat function

	if (_IsPaused)
	{
		if (SUCCEEDED(_SourceVoice->Start(0))) _IsPaused = false;
			else nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Play");
		return !_IsPaused;
	}
	else
	{
		_SoundDriver->performanceIncreaseSourcePlayCounter();
		
		if (_BufferStreaming)
		{
			// preparePlay already called, 
			// stop already called before going into buffer streaming
			nlassert(!_IsPlaying);
			nlassert(_SourceVoice);
			_PlayStart = CTime::getLocalTime();
			if (SUCCEEDED(_SourceVoice->Start(0))) _IsPlaying = true;
			else nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Play (_BufferStreaming)");
		}
		else
		{
			if (_StaticBuffer)
			{
				preparePlay(_StaticBuffer->getFormat(), 
					_StaticBuffer->getChannels(), 
					_StaticBuffer->getBitsPerSample(),
					_StaticBuffer->getFrequency());
				submitBuffer(_StaticBuffer);
				_PlayStart = CTime::getLocalTime();
				if (SUCCEEDED(_SourceVoice->Start(0))) _IsPlaying = true;
				else nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Play");
			}
			else
			{
				stop();
			}
		}
		return _IsPlaying;
	}
}

/// Stop playing
void CSourceXAudio2::stop()
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "stop");

	_IsPlaying = false;
	_IsPaused = false;
	if (_SourceVoice)
	{
		// stop adpcm stream
		if (_AdpcmUtility) _AdpcmUtility->flushSourceBuffers();

		// stop source voice and remove pending buffers
		_SoundDriver->getXAudio2()->CommitChanges(_OperationSet);
		if (FAILED(_SourceVoice->Stop(0))) 
			nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Stop");
		if (FAILED(_SourceVoice->FlushSourceBuffers())) 
			nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED FlushSourceBuffers");
	}
}

/// Pause. Call play() to resume.
void CSourceXAudio2::pause()
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "pause");

	if (_IsPaused) nlwarning(NLSOUND_XAUDIO2_PREFIX "Called pause() while _IsPaused == true!");

	if (_IsPlaying)
	{
		_IsPaused = true;
		if (FAILED(_SourceVoice->Stop(0))) 
			nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED Stop");
	}
	else nlwarning(NLSOUND_XAUDIO2_PREFIX "Called pause() while _IsPlaying == false!");
}

/// Return true if play() or pause(), false if stop().
bool CSourceXAudio2::isPlaying() const
{
	// nlinfo(NLSOUND_XAUDIO2_PREFIX "isPlaying?");

	return _IsPlaying;
}

/// Return true if playing is finished or stop() has been called.
bool CSourceXAudio2::isStopped() const
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "isStopped?");
	
	return !_IsPlaying;
}

/// Return the paused state
bool CSourceXAudio2::isPaused() const
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "isStopped?");

	return _IsPaused;
}

/// Returns the number of milliseconds the source has been playing
uint32 CSourceXAudio2::getTime()
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "getTime");

	// ----------------------------- todo

	return _IsPlaying ? (uint32)(CTime::getLocalTime() - _PlayStart) : 0;
}

/// Set the position vector (default: (0,0,0)).
void CSourceXAudio2::setPos(const NLMISC::CVector& pos, bool /* deffered */) // note: deferred with a different spelling
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setPos %f %f %f", pos.x, pos.y, pos.z);

	_Pos = pos;
	NLSOUND_XAUDIO2_X3DAUDIO_VECTOR_FROM_VECTOR(_Emitter.Position, pos);

	// !! todo if threaded: if (!deffered) { /* nlwarning(NLSOUND_XAUDIO2_PREFIX "!deffered"); */ commit3DChanges(); }
}

/// Get the position vector.
const NLMISC::CVector &CSourceXAudio2::getPos() const
{
	return _Pos;
}

/// Set the velocity vector (3D mode only, ignored in stereo mode) (default: (0,0,0))
void CSourceXAudio2::setVelocity(const NLMISC::CVector& vel, bool /* deferred */)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setVelocity %f %f %f", vel.x, vel.y, vel.z);

	NLSOUND_XAUDIO2_X3DAUDIO_VECTOR_FROM_VECTOR(_Emitter.Velocity, vel);

	// !! todo if threaded: if (!deferred) { /* nlwarning(NLSOUND_XAUDIO2_PREFIX "!deferred"); */ commit3DChanges(); }
}

/// Get the velocity vector
void CSourceXAudio2::getVelocity(NLMISC::CVector& vel) const
{
	NLSOUND_XAUDIO2_VECTOR_FROM_X3DAUDIO_VECTOR(vel, _Emitter.Velocity);
}

/// Set the direction vector (3D mode only, ignored in stereo mode) (default: (0,0,0) as non-directional)
void CSourceXAudio2::setDirection(const NLMISC::CVector& dir)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setDirection %f %f %f", dir.x, dir.y, dir.z);

	NLSOUND_XAUDIO2_X3DAUDIO_VECTOR_FROM_VECTOR(_Emitter.OrientFront, dir);
}

/// Get the direction vector
void CSourceXAudio2::getDirection(NLMISC::CVector& dir) const
{
	NLSOUND_XAUDIO2_VECTOR_FROM_X3DAUDIO_VECTOR(dir, _Emitter.OrientFront);
}

/// Set the gain (volume value inside [0 , 1]). (default: 1)
void CSourceXAudio2::setGain(float gain)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setGain %f", gain);

	_Gain = std::min(std::max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
	if (_SourceVoice) _SourceVoice->SetVolume(_Gain, _OperationSet);
}

/// Get the gain
float CSourceXAudio2::getGain() const
{
	return _Gain;
}

/// Shift the frequency.
void CSourceXAudio2::setPitch(float pitch)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setPitch %f", pitch);

	_Pitch = std::min(std::max(pitch, NLSOUND_MIN_PITCH), NLSOUND_MAX_PITCH);
	if (_SourceVoice)
	{
		if (_Channels > 1) _SourceVoice->SetFrequencyRatio(_Pitch, _OperationSet);
		else _SourceVoice->SetFrequencyRatio(_Pitch * _Doppler, _OperationSet);
	}
}

/// Get the pitch
float CSourceXAudio2::getPitch() const
{
	return _Pitch;
}

/// Set the source relative mode. If true, positions are interpreted relative to the listener position
void CSourceXAudio2::setSourceRelativeMode(bool mode)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setSourceRelativeMode %u", (uint32)mode);

	_Relative = mode;
}

/// Get the source relative mode
bool CSourceXAudio2::getSourceRelativeMode() const
{
	return _Relative;
}

/// Set the min and max distances (default: 1, MAX_FLOAT) (3D mode only)
void CSourceXAudio2::setMinMaxDistances(float mindist, float maxdist, bool /* deferred */)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setMinMaxDistances %f, %f", mindist, maxdist);
	
	static float maxSqrt = sqrt(std::numeric_limits<float>::max());
	if (maxdist >= maxSqrt)
	{
		nlwarning("SOUND_DEV (XAudio2): Ridiculously high max distance set on source");
		maxdist = maxSqrt;
	}
	
	_Emitter.InnerRadius = mindist;
	_MinDistance = mindist;
	_MaxDistance = maxdist;
}

/// Get the min and max distances
void CSourceXAudio2::getMinMaxDistances(float& mindist, float& maxdist) const
{
	mindist = _MinDistance;
	maxdist = _MaxDistance;
}

/// Set the cone angles (in radian) and gain (in [0 , 1]) (default: 2PI, 2PI, 0)
void CSourceXAudio2::setCone(float innerAngle, float outerAngle, float outerGain)
{
	// nldebug(NLSOUND_XAUDIO2_PREFIX "setCone %f, %f ,%f", innerAngle, outerAngle, outerGain);
	
	if (innerAngle >= 6.283185f && outerAngle >= 6.283185f)
		_Emitter.pCone = NULL;
	else _Emitter.pCone = &_Cone;
	if (innerAngle > outerAngle)
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "innerAngle > outerAngle");
		innerAngle = outerAngle;
	}
	_Cone.InnerAngle = innerAngle;
	_Cone.OuterAngle = outerAngle;
	_Cone.OuterVolume = outerGain;
}

/// Get the cone angles (in radian)
void CSourceXAudio2::getCone(float& innerAngle, float& outerAngle, float& outerGain) const
{
	innerAngle = _Cone.InnerAngle;
	outerAngle = _Cone.OuterAngle;
	outerGain = _Cone.OuterVolume;
}

/// Set the alpha value for the volume-distance curve
void CSourceXAudio2::setAlpha(double a) 
{  
	nlassert(_SoundDriver->getOption(ISoundDriver::OptionManualRolloff));
		
	// if (a != 1.0) // nldebug(NLSOUND_XAUDIO2_PREFIX "setAlpha %f", (float)a);
	_Alpha = a;
}

/// Enable or disable direct output [true/false], default: true
void CSourceXAudio2::setDirect(bool enable)
{
	if (_DirectDryEnabled != enable)
	{
		_DirectDryEnabled = enable;
		if (_SourceVoice) setupVoiceSends();
	}
}

/// Return if the direct output is enabled
bool CSourceXAudio2::getDirect() const
{
	return _DirectDryEnabled;
}

/// Set the gain for the direct path
void CSourceXAudio2::setDirectGain(float gain)
{
	_DirectGain = min(max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
}

/// Get the gain for the direct path
float CSourceXAudio2::getDirectGain() const
{
	return _DirectGain;
}

/// Enable or disable the filter for the direct channel
void CSourceXAudio2::enableDirectFilter(bool enable)
{
	if (_DirectFilterEnabled != enable)
	{
		_DirectFilterEnabled = enable;
		if (_SourceVoice) setupVoiceSends();
	}
}

/// Check if the filter on the direct channel is enabled
bool CSourceXAudio2::isDirectFilterEnabled() const
{
	return _DirectFilterEnabled;
}

/// (Internal) Setup the direct send filter.
void CSourceXAudio2::setupDirectFilter()
{
	// todo: this sort of works and sounds sort of similar to the openal way, but still needs work
	if (_DirectDryEnabled && _DirectFilterEnabled)
	{
		switch (_DirectFilter.Type)
		{
		case LowPassFilter:
			_DirectFilter.Frequency = XAudio2CutoffFrequencyToRadians(_DirectFilterHighFrequency / 10.0f, _Frequency);
			_DirectFilter.OneOverQ = 1.0f;
			break;
		case BandPassFilter:
			_DirectFilter.Frequency = XAudio2CutoffFrequencyToRadians(((_DirectFilterLowFrequency * 10.0f) + (_DirectFilterHighFrequency / 10.0f)) / 2.0f, _Frequency);
			_DirectFilter.OneOverQ = 1.5f; // todo: calculate OneOverQ from range between low and high frequency
			break;
		case HighPassFilter:
			_DirectFilter.Frequency = XAudio2CutoffFrequencyToRadians(_DirectFilterLowFrequency * 10.0f, _Frequency);
			_DirectFilter.OneOverQ = 1.0f;
			break;
		default:
			_DirectFilter.Type = (XAUDIO2_FILTER_TYPE)~0;
			break;
		}
		_SourceVoice->SetOutputFilterParameters(_DirectFilterVoice, &_DirectFilter, _OperationSet);
	}
}

/// Set the filter parameters for the direct channel
void CSourceXAudio2::setDirectFilter(TFilter filterType, float lowFrequency, float highFrequency, float passGain)
{
	switch (filterType)
	{
	case ISource::FilterLowPass:
		_DirectFilter.Type = LowPassFilter;
		break;
	case ISource::FilterBandPass:
		_DirectFilter.Type = BandPassFilter;
		break;
	case ISource::FilterHighPass:
		_DirectFilter.Type = HighPassFilter;
		break;
	default:
		_DirectFilter.Type = (XAUDIO2_FILTER_TYPE)~0;
		break;
	}
	_DirectFilterLowFrequency = lowFrequency;
	_DirectFilterHighFrequency = highFrequency;
	_DirectFilterPassGain = passGain;
	if (_SourceVoice) setupDirectFilter();
}

/// Get the filter parameters for the direct channel
void CSourceXAudio2::getDirectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	switch (_DirectFilter.Type)
	{
	case LowPassFilter:
		filterType = ISource::FilterLowPass;
		break;
	case BandPassFilter:
		filterType = ISource::FilterBandPass;
		break;
	case HighPassFilter:
		filterType = ISource::FilterHighPass;
		break;
	default:
		filterType = (TFilter)~0;
		break;
	}
	lowFrequency = _DirectFilterLowFrequency;
	highFrequency = _DirectFilterHighFrequency;
	passGain = _DirectFilterPassGain;
}

/// Set the direct filter gain
void CSourceXAudio2::setDirectFilterPassGain(float passGain)
{
	_DirectFilterPassGain = min(max(passGain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
}

/// Get the direct filter gain
float CSourceXAudio2::getDirectFilterPassGain() const
{
	return _DirectFilterPassGain;
}

/// Set the effect send for this source, NULL to disable. [IEffect], default: NULL
void CSourceXAudio2::setEffect(CEffectXAudio2 *effect)
{
	// if (_Effect != effect)
	{
		if (effect)
		{
			_EffectDryEnabled = true;
			_EffectDryVoice = effect->getDryVoice();
			_EffectFilterVoice = effect->getFilterVoice();
		}
		else
		{
			_EffectDryEnabled = false;
			_EffectDryVoice = NULL;
			_EffectFilterVoice = NULL;
		}
		if (_SourceVoice) setupVoiceSends();
	}
}

/// Set the effect send for this source, NULL to disable.
void CSourceXAudio2::setEffect(IReverbEffect *reverbEffect)
{
	setEffect(reverbEffect ? static_cast<CEffectXAudio2 *>(static_cast<CReverbEffectXAudio2 *>(reverbEffect)) : NULL);
}

/// Get the effect send for this source
IEffect *CSourceXAudio2::getEffect() const
{
	return NULL;
}

/// Set the gain for the direct path
void CSourceXAudio2::setEffectGain(float gain)
{
	_EffectGain = min(max(gain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
}

/// Get the gain for the direct path
float CSourceXAudio2::getEffectGain() const
{
	return _EffectGain;
}

/// Enable or disable the filter for the effect channel
void CSourceXAudio2::enableEffectFilter(bool enable)
{
	if (_EffectFilterEnabled != enable)
	{
		_EffectFilterEnabled = enable;
		if (_SourceVoice) setupVoiceSends();
	}
}

/// Check if the filter on the effect channel is enabled
bool CSourceXAudio2::isEffectFilterEnabled() const
{
	return _EffectFilterEnabled;
}

/// (Internal) Setup the direct send filter.
void CSourceXAudio2::setupEffectFilter()
{
	// todo: this sort of works and sounds sort of similar to the openal way, but still needs work
	if (_EffectDryEnabled && _EffectFilterEnabled)
	{
		switch (_EffectFilter.Type)
		{
		case LowPassFilter:
			_EffectFilter.Frequency = XAudio2CutoffFrequencyToRadians(_EffectFilterHighFrequency / 10.0f, _Frequency);
			_EffectFilter.OneOverQ = 1.0f;
			break;
		case BandPassFilter:
			_EffectFilter.Frequency = XAudio2CutoffFrequencyToRadians(((_EffectFilterLowFrequency * 10.0f) + (_EffectFilterHighFrequency / 10.0f)) / 2.0f, _Frequency);
			_EffectFilter.OneOverQ = 1.5f; // todo: calculate OneOverQ from range between low and high frequency
			break;
		case HighPassFilter:
			_EffectFilter.Frequency = XAudio2CutoffFrequencyToRadians(_EffectFilterLowFrequency * 10.0f, _Frequency);
			_EffectFilter.OneOverQ = 1.0f;
		default:
			_EffectFilter.Type = (XAUDIO2_FILTER_TYPE)~0;
			break;
		}
		_SourceVoice->SetOutputFilterParameters(_EffectFilterVoice, &_EffectFilter, _OperationSet);
	}
}

/// Set the filter parameters for the direct channel
void CSourceXAudio2::setEffectFilter(TFilter filterType, float lowFrequency, float highFrequency, float passGain)
{
	switch (filterType)
	{
	case ISource::FilterLowPass:
		_EffectFilter.Type = LowPassFilter;
		break;
	case ISource::FilterBandPass:
		_EffectFilter.Type = BandPassFilter;
		break;
	case ISource::FilterHighPass:
		_EffectFilter.Type = HighPassFilter;
		break;
	default:
		_EffectFilter.Type = (XAUDIO2_FILTER_TYPE)~0;
		break;
	}
	_EffectFilterLowFrequency = lowFrequency;
	_EffectFilterHighFrequency = highFrequency;
	_EffectFilterPassGain = passGain;
	if (_SourceVoice) setupEffectFilter();
}

/// Get the filter parameters for the direct channel
void CSourceXAudio2::getEffectFilter(TFilter &filterType, float &lowFrequency, float &highFrequency, float &passGain) const
{
	switch (_EffectFilter.Type)
	{
	case LowPassFilter:
		filterType = ISource::FilterLowPass;
		break;
	case BandPassFilter:
		filterType = ISource::FilterBandPass;
		break;
	case HighPassFilter:
		filterType = ISource::FilterHighPass;
		break;
	default:
		filterType = (TFilter)~0;
		break;
	}
	lowFrequency = _EffectFilterLowFrequency;
	highFrequency = _EffectFilterHighFrequency;
	passGain = _EffectFilterPassGain;
}

/// Set the effect filter gain
void CSourceXAudio2::setEffectFilterPassGain(float passGain)
{
	_EffectFilterPassGain = min(max(passGain, NLSOUND_MIN_GAIN), NLSOUND_MAX_GAIN);
}

/// Get the effect filter gain
float CSourceXAudio2::getEffectFilterPassGain() const
{
	return _EffectFilterPassGain;
}

} /* namespace NLSOUND */

/* end of file */
