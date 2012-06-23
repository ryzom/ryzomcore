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

#include "stdxaudio2.h"

// Project includes
#include "sound_driver_xaudio2.h"
#include "listener_xaudio2.h"
#include "effect_xaudio2.h"

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
		if (FAILED(hr = soundDriver->getXAudio2()->CreateSubmixVoice(&_DryVoice, channels, voice_details.InputSampleRate, 0, 4510, NULL, NULL)))
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
		effect_descriptor.OutputChannels = voice_details.InputChannels;
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

/// Set the environment (you have full control now, have fun)
void CReverbEffectXAudio2::setEnvironment(const CEnvironment &environment, float roomSize)
{
	// unused params
	_ReverbParams.LowEQCutoff = 4;
	_ReverbParams.HighEQCutoff = 6;
	_ReverbParams.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
	_ReverbParams.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	_ReverbParams.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
	_ReverbParams.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	_ReverbParams.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
	_ReverbParams.RoomFilterFreq = 5000.0f;
	_ReverbParams.WetDryMix = 100.0f;

	// directly mapped
	_ReverbParams.Density = environment.Density;
	_ReverbParams.RoomFilterMain = environment.RoomFilter;
	_ReverbParams.RoomFilterHF = environment.RoomFilterHF;
	_ReverbParams.ReverbGain = environment.LateReverb;
	_ReverbParams.ReflectionsGain = environment.Reflections;
	_ReverbParams.RoomSize = roomSize;

	// conversions, see ReverbConvertI3DL2ToNative in case of errors
	if (environment.DecayHFRatio >= 1.0f)
	{
		_ReverbParams.HighEQGain = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN;
		sint32 index = (sint32)(log10(environment.DecayHFRatio) * -4.0f) + 8;
		clamp(index, XAUDIO2FX_REVERB_MIN_LOW_EQ_GAIN, XAUDIO2FX_REVERB_MAX_LOW_EQ_GAIN);
		_ReverbParams.LowEQGain = (BYTE)index;
		_ReverbParams.DecayTime = environment.DecayTime * environment.DecayHFRatio;
	}
	else
	{
		_ReverbParams.LowEQGain = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN;
		sint32 index = (sint32)(log10(environment.DecayHFRatio) * 4.0f) + 8;
		clamp(index, XAUDIO2FX_REVERB_MIN_HIGH_EQ_GAIN, XAUDIO2FX_REVERB_MAX_HIGH_EQ_GAIN);
		_ReverbParams.HighEQGain = (BYTE)index;
		_ReverbParams.DecayTime = environment.DecayTime;
	}

	sint32 reflections_delay = (sint32)(environment.ReflectionsDelay * 1000.0f);
	clamp(reflections_delay, XAUDIO2FX_REVERB_MIN_REFLECTIONS_DELAY, XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY);
	_ReverbParams.ReflectionsDelay = (UINT32)reflections_delay;
	
	sint32 reverb_delay = (sint32)(environment.LateReverbDelay * 1000.0f);
	clamp(reverb_delay, XAUDIO2FX_REVERB_MIN_REVERB_DELAY, XAUDIO2FX_REVERB_MAX_REVERB_DELAY);
	_ReverbParams.ReverbDelay = (BYTE)reverb_delay;

	_ReverbParams.EarlyDiffusion = (BYTE)(environment.Diffusion * 0.15f);
	_ReverbParams.LateDiffusion = _ReverbParams.EarlyDiffusion;

	_DryVoice->SetEffectParameters(0, &_ReverbParams, sizeof(_ReverbParams), 0);
}

} /* namespace NLSOUND */

/* end of file */
