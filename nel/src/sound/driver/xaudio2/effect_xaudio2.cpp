// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2008  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdxaudio2.h"

// Project includes
#include "sound_driver_xaudio2.h"
#include "listener_xaudio2.h"
#include "effect_xaudio2.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CEffectXAudio2::CEffectXAudio2(CSoundDriverXAudio2 *soundDriver, uint channels) : _SoundDriver(soundDriver), _DryVoice(NULL), _FilterVoice(NULL), _Effect(NULL)
{
	HRESULT hr;

	XAUDIO2_VOICE_DETAILS voice_details;
	soundDriver->getListener()->getDryVoice()->GetVoiceDetails(&voice_details);

	if (channels == 0)
		channels = voice_details.InputChannels;

	// Mix voice
	{
		if (FAILED(hr = soundDriver->getXAudio2()->CreateSubmixVoice(&_DryVoice, channels, min(max(20000, (int)voice_details.InputSampleRate), 48000), 0, 4510, NULL, NULL)))
			{ release(); _SoundDriver = NULL; nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED CreateSubmixVoice _DryVoice!"); return; }
		
		XAUDIO2_VOICE_SENDS voiceSends;
		XAUDIO2_SEND_DESCRIPTOR sendDescriptor;
		voiceSends.pSends = &sendDescriptor;
		voiceSends.SendCount = 1;
		sendDescriptor.Flags = 0;
		sendDescriptor.pOutputVoice = soundDriver->getListener()->getDryVoice();
		_DryVoice->SetOutputVoices(&voiceSends);

		if (channels == 1)
		{
			FLOAT32 outputMatrix[32];
			for (uint32 i = 0; i < voice_details.InputChannels; ++i)
				outputMatrix[i] = 1.0f;
			_DryVoice->SetOutputMatrix(sendDescriptor.pOutputVoice, 1, voice_details.InputChannels, outputMatrix);
		}
	}

	// Filter dummy voice
	{
		if (FAILED(hr = soundDriver->getXAudio2()->CreateSubmixVoice(&_FilterVoice, channels, voice_details.InputSampleRate, 0, 4500, NULL, NULL)))
			{ release(); _SoundDriver = NULL; nlwarning(NLSOUND_XAUDIO2_PREFIX "FAILED CreateSubmixVoice _FilterVoice!"); return; }

		XAUDIO2_VOICE_SENDS voiceSends;
		XAUDIO2_SEND_DESCRIPTOR sendDescriptor;
		voiceSends.pSends = &sendDescriptor;
		voiceSends.SendCount = 1;
		sendDescriptor.Flags = 0;
		sendDescriptor.pOutputVoice = _DryVoice;
		_FilterVoice->SetOutputVoices(&voiceSends);
	}
}

CEffectXAudio2::~CEffectXAudio2()
{
	release();
	if (_SoundDriver) { _SoundDriver->removeEffect(this); _SoundDriver = NULL; }
}

void CEffectXAudio2::release()
{
	if (_FilterVoice) { _FilterVoice->DestroyVoice(); _FilterVoice = NULL; }
	if (_DryVoice) { _DryVoice->DestroyVoice(); _DryVoice = NULL; }
}

CReverbEffectXAudio2::CReverbEffectXAudio2(CSoundDriverXAudio2 *soundDriver) : CEffectXAudio2(soundDriver, 1)
{
	// FIXME: Reverb must be within [20000, 48000] Hz and has limited input and output channel configurations...

	if (_DryVoice)
	{
		HRESULT hr;

		uint flags = 0;
	#ifdef NL_DEBUG
		flags |= XAUDIO2FX_DEBUG;
	#endif		
		if (FAILED(hr = XAudio2CreateReverb(&_Effect, flags)))
			{ release(); nlwarning(NLSOUND_XAUDIO2_PREFIX "XAudio2CreateReverb FAILED"); return; }

		XAUDIO2_VOICE_DETAILS voice_details;
		_DryVoice->GetVoiceDetails(&voice_details);
		XAUDIO2_EFFECT_DESCRIPTOR effect_descriptor;
		effect_descriptor.InitialState = TRUE;
		effect_descriptor.OutputChannels = voice_details.InputChannels; // FIXME
		effect_descriptor.pEffect = _Effect;
		XAUDIO2_EFFECT_CHAIN effect_chain;
		effect_chain.EffectCount = 1;
		effect_chain.pEffectDescriptors = &effect_descriptor;
		if (FAILED(hr = _DryVoice->SetEffectChain(&effect_chain)))
			{ release(); nlwarning(NLSOUND_XAUDIO2_PREFIX "SetEffectChain FAILED"); return; }
		
		setEnvironment();
	}
}

CReverbEffectXAudio2::~CReverbEffectXAudio2()
{
	
}

void CReverbEffectXAudio2::release()
{
	CEffectXAudio2::release();
	if (_Effect) { _Effect->Release(); _Effect = NULL; }
}

// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide.
// This software is distributed without any warranty.
/// Set the environment (you have full control now, have fun)
void CReverbEffectXAudio2::setEnvironment(const CEnvironment &environment, float roomSize)
{
	CEnvironment env = environment;
	env.resize(roomSize);

	// Scale parameters to voice sampling rate, see
	// https://learn.microsoft.com/en-us/windows/win32/api/xaudio2fx/ns-xaudio2fx-xaudio2fx_reverb_parameters
	float scaleRate = 1.0f;
	{
		XAUDIO2_VOICE_DETAILS voice_details;
		_DryVoice->GetVoiceDetails(&voice_details);
		scaleRate = (float)voice_details.InputSampleRate / 48000.0f;
	}
	float invScaleRate = 1.0f / scaleRate;

	// Unused params
	_ReverbParams.LowEQCutoff = 4;
	_ReverbParams.HighEQCutoff = 6;
	_ReverbParams.RearDelay = (BYTE)((float)XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY * invScaleRate);
	clamp(_ReverbParams.RearDelay, (BYTE)0, (BYTE)5);
	_ReverbParams.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	_ReverbParams.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	_ReverbParams.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	_ReverbParams.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	_ReverbParams.RoomFilterFreq = 5000.0f * scaleRate;
	clamp(_ReverbParams.RoomFilterFreq, XAUDIO2FX_REVERB_MIN_ROOM_FILTER_FREQ, XAUDIO2FX_REVERB_MAX_ROOM_FILTER_FREQ);
	_ReverbParams.WetDryMix = 100.0f;

	// Directly mapped
	_ReverbParams.Density = env.Density;
	_ReverbParams.RoomFilterMain = env.RoomFilter;
	_ReverbParams.RoomFilterHF = env.RoomFilterHF;
	_ReverbParams.ReverbGain = env.LateReverb;
	_ReverbParams.ReflectionsGain = env.Reflections;

	// Room size from meters to feet
	_ReverbParams.RoomSize = env.RoomSize * 3.2808399f * invScaleRate;
	clamp(_ReverbParams.RoomSize, XAUDIO2FX_REVERB_MIN_ROOM_SIZE, XAUDIO2FX_REVERB_MAX_ROOM_SIZE);

	// Conversions, see ReverbConvertI3DL2ToNative in case of errors
	if (env.DecayHFRatio >= 1.0f)
	{
		_ReverbParams.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
		sint32 index = (sint32)(log10f(env.DecayHFRatio) * -4.0f) + 8;
		clamp(index, XAUDIO2FX_REVERB_MIN_LOW_EQ_GAIN, XAUDIO2FX_REVERB_MAX_LOW_EQ_GAIN);
		_ReverbParams.LowEQGain = (BYTE)index;
		_ReverbParams.DecayTime = std::max(0.1f, env.DecayTime * env.DecayHFRatio * invScaleRate);
	}
	else
	{
		_ReverbParams.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
		sint32 index = (sint32)(log10f(env.DecayHFRatio) * 4.0f) + 8;
		clamp(index, XAUDIO2FX_REVERB_MIN_HIGH_EQ_GAIN, XAUDIO2FX_REVERB_MAX_HIGH_EQ_GAIN);
		_ReverbParams.HighEQGain = (BYTE)index;
		_ReverbParams.DecayTime = std::max(0.1f, env.DecayTime * invScaleRate);
	}

	sint32 reflections_delay = (sint32)(env.ReflectionsDelay * invScaleRate * 1000.0f);
	clamp(reflections_delay, XAUDIO2FX_REVERB_MIN_REFLECTIONS_DELAY + 1, XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY - 1);
	_ReverbParams.ReflectionsDelay = (UINT32)reflections_delay;

	sint32 reverb_delay = (sint32)(env.LateReverbDelay * invScaleRate * 1000.0f);
	clamp(reverb_delay, XAUDIO2FX_REVERB_MIN_REVERB_DELAY + 1, XAUDIO2FX_REVERB_MAX_REVERB_DELAY - 1);
	_ReverbParams.ReverbDelay = (BYTE)reverb_delay;

	_ReverbParams.EarlyDiffusion = (BYTE)(env.Diffusion * 0.15f);
	_ReverbParams.LateDiffusion = _ReverbParams.EarlyDiffusion;

	_DryVoice->SetEffectParameters(0, &_ReverbParams, sizeof(_ReverbParams), 0);
}

} /* namespace NLSOUND */

/* end of file */
