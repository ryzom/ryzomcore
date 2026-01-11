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
#include "ext_al.h"
#include "effect_al.h"
#include "sound_driver_al.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

// ******************************************************************

CEffectAL::CEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot) : _SoundDriver(soundDriver), _AlEffect(alEffect), _AlAuxEffectSlot(alAuxEffectSlot)
{

}

CEffectAL::~CEffectAL()
{
	CSoundDriverAL *soundDriver = _SoundDriver;
	release();
	if (soundDriver) soundDriver->removeEffect(this);
}

void CEffectAL::release()
{
	if (_AlAuxEffectSlot != AL_EFFECTSLOT_NULL) { alDeleteAuxiliaryEffectSlots(1, &_AlAuxEffectSlot); _AlAuxEffectSlot = AL_EFFECTSLOT_NULL; }
	if (_AlEffect != AL_EFFECT_NULL) { alDeleteEffects(1, &_AlEffect); _AlEffect = AL_EFFECT_NULL; }
	_SoundDriver = NULL;
}

// ******************************************************************

CStandardReverbEffectAL::CStandardReverbEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot) : CEffectAL(soundDriver, alEffect, alAuxEffectSlot)
{
	// unused params, set default values
	alEffectf(_AlEffect, AL_REVERB_AIR_ABSORPTION_GAINHF, 0.994f);
	alEffectf(_AlEffect, AL_REVERB_ROOM_ROLLOFF_FACTOR, 0.0f);
	alEffectf(_AlEffect, AL_REVERB_DECAY_HFLIMIT, AL_TRUE);

	// set default environment
	setEnvironment();
}

CStandardReverbEffectAL::~CStandardReverbEffectAL()
{

}

void CStandardReverbEffectAL::setEnvironment(const CEnvironment &environment, float roomSize)
{
	nldebug("AL: CStandardReverbEffectAL::setEnvironment, size: %f", roomSize);

	// *** TODO *** environment.RoomSize
	alEffectf(_AlEffect, AL_REVERB_DENSITY, environment.Density / 100.0f); alTestWarning("AL_REVERB_DENSITY");
	alEffectf(_AlEffect, AL_REVERB_DIFFUSION, environment.Diffusion / 100.0f); alTestWarning("AL_REVERB_DIFFUSION");
	alEffectf(_AlEffect, AL_REVERB_GAIN, decibelsToAmplitudeRatio(environment.RoomFilter)); alTestWarning("AL_REVERB_GAIN");
	alEffectf(_AlEffect, AL_REVERB_GAINHF, decibelsToAmplitudeRatio(environment.RoomFilterHF)); alTestWarning("AL_REVERB_GAINHF");
	alEffectf(_AlEffect, AL_REVERB_DECAY_TIME, environment.DecayTime); alTestWarning("AL_REVERB_DECAY_TIME");
	alEffectf(_AlEffect, AL_REVERB_DECAY_HFRATIO, environment.DecayHFRatio); alTestWarning("AL_REVERB_DECAY_HFRATIO");
	alEffectf(_AlEffect, AL_REVERB_REFLECTIONS_GAIN, decibelsToAmplitudeRatio(environment.Reflections)); alTestWarning("AL_REVERB_REFLECTIONS_GAIN");
	alEffectf(_AlEffect, AL_REVERB_REFLECTIONS_DELAY, environment.ReflectionsDelay); alTestWarning("AL_REVERB_REFLECTIONS_DELAY");
	alEffectf(_AlEffect, AL_REVERB_LATE_REVERB_GAIN, decibelsToAmplitudeRatio(environment.LateReverb)); alTestWarning("AL_REVERB_LATE_REVERB_GAIN");
	alEffectf(_AlEffect, AL_REVERB_LATE_REVERB_DELAY, environment.LateReverbDelay); alTestWarning("AL_REVERB_LATE_REVERB_DELAY");
}

// ******************************************************************

#if EFX_CREATIVE_AVAILABLE

CCreativeReverbEffectAL::CCreativeReverbEffectAL(CSoundDriverAL *soundDriver, ALuint alEffect, ALuint alAuxEffectSlot) : CEffectAL(soundDriver, alEffect, alAuxEffectSlot)
{
	// set default environment
	setEnvironment();
}

CCreativeReverbEffectAL::~CCreativeReverbEffectAL()
{

}

void CCreativeReverbEffectAL::setEnvironment(const CEnvironment &environment, float roomSize)
{
	nldebug("AL: CCreativeReverbEffectAL::setEnvironment, size: %f", roomSize);

	EAXREVERBPROPERTIES eaxreverb;
	eaxreverb.ulEnvironment = 26;
	eaxreverb.flEnvironmentSize = roomSize;
	eaxreverb.flEnvironmentDiffusion = environment.Diffusion / 100.0f;
	eaxreverb.lRoom = (long)(environment.RoomFilter * 100.0f);
	eaxreverb.lRoomHF = (long)(environment.RoomFilterHF * 100.0f);
	eaxreverb.lRoomLF = 0;
	eaxreverb.flDecayTime = environment.DecayTime;
	eaxreverb.flDecayHFRatio = environment.DecayHFRatio;
	eaxreverb.flDecayLFRatio = 1.0f;
	eaxreverb.lReflections = (long)(environment.Reflections * 100.0f);
	eaxreverb.flReflectionsDelay = environment.ReflectionsDelay;
	eaxreverb.vReflectionsPan.x = 0.0f;
	eaxreverb.vReflectionsPan.y = 0.0f;
	eaxreverb.vReflectionsPan.z = 0.0f;
	eaxreverb.lReverb = (long)(environment.LateReverb * 100.0f);
	eaxreverb.flReverbDelay = environment.LateReverbDelay;
	eaxreverb.vReverbPan.x = 0.0f;
	eaxreverb.vReverbPan.y = 0.0f;
	eaxreverb.vReverbPan.z = 0.0f;
	eaxreverb.flEchoTime = 0.250f;
	eaxreverb.flEchoDepth = 0.000f;
	eaxreverb.flModulationTime = 0.250f;
	eaxreverb.flModulationDepth = 0.000f;
	eaxreverb.flAirAbsorptionHF = -5.0f;
	eaxreverb.flHFReference = 5000.0f;
	eaxreverb.flLFReference = 250.0f;
	eaxreverb.flRoomRolloffFactor = 0.0f;
	eaxreverb.ulFlags = 0x3f;
	EFXEAXREVERBPROPERTIES efxcreativereverb;
	ConvertReverbParameters(&eaxreverb, &efxcreativereverb);
	efxcreativereverb.flDensity = environment.Density / 100.0f;
	alEffectf(_AlEffect, AL_EAXREVERB_DENSITY, efxcreativereverb.flDensity); alTestWarning("AL_EAXREVERB_DENSITY");
	alEffectf(_AlEffect, AL_EAXREVERB_DIFFUSION, efxcreativereverb.flDiffusion); alTestWarning("AL_EAXREVERB_DIFFUSION");
	alEffectf(_AlEffect, AL_EAXREVERB_GAIN, efxcreativereverb.flGain); alTestWarning("AL_EAXREVERB_GAIN");
	alEffectf(_AlEffect, AL_EAXREVERB_GAINHF, efxcreativereverb.flGainHF); alTestWarning("AL_EAXREVERB_GAINHF");
	alEffectf(_AlEffect, AL_EAXREVERB_GAINLF, efxcreativereverb.flGainLF); alTestWarning("AL_EAXREVERB_GAINLF");
	alEffectf(_AlEffect, AL_EAXREVERB_DECAY_TIME, efxcreativereverb.flDecayTime); alTestWarning("AL_EAXREVERB_DECAY_TIME");
	alEffectf(_AlEffect, AL_EAXREVERB_DECAY_HFRATIO, efxcreativereverb.flDecayHFRatio); alTestWarning("AL_EAXREVERB_DECAY_HFRATIO");
	alEffectf(_AlEffect, AL_EAXREVERB_DECAY_LFRATIO, efxcreativereverb.flDecayLFRatio); alTestWarning("AL_EAXREVERB_DECAY_LFRATIO");
	alEffectf(_AlEffect, AL_EAXREVERB_REFLECTIONS_GAIN, efxcreativereverb.flReflectionsGain); alTestWarning("AL_EAXREVERB_REFLECTIONS_GAIN");
	alEffectf(_AlEffect, AL_EAXREVERB_REFLECTIONS_DELAY, efxcreativereverb.flReflectionsDelay); alTestWarning("AL_EAXREVERB_REFLECTIONS_DELAY");
	alEffectfv(_AlEffect, AL_EAXREVERB_REFLECTIONS_PAN, efxcreativereverb.flReflectionsPan); alTestWarning("AL_EAXREVERB_REFLECTIONS_PAN");
	alEffectf(_AlEffect, AL_EAXREVERB_LATE_REVERB_GAIN, efxcreativereverb.flLateReverbGain); alTestWarning("AL_EAXREVERB_LATE_REVERB_GAIN");
	alEffectf(_AlEffect, AL_EAXREVERB_LATE_REVERB_DELAY, efxcreativereverb.flLateReverbDelay); alTestWarning("AL_EAXREVERB_LATE_REVERB_DELAY");
	alEffectfv(_AlEffect, AL_EAXREVERB_LATE_REVERB_PAN, efxcreativereverb.flLateReverbPan); alTestWarning("AL_EAXREVERB_LATE_REVERB_PAN");
	alEffectf(_AlEffect, AL_EAXREVERB_ECHO_TIME, efxcreativereverb.flEchoTime); alTestWarning("AL_EAXREVERB_ECHO_TIME");
	alEffectf(_AlEffect, AL_EAXREVERB_ECHO_DEPTH, efxcreativereverb.flEchoDepth); alTestWarning("AL_EAXREVERB_ECHO_DEPTH");
	alEffectf(_AlEffect, AL_EAXREVERB_MODULATION_TIME, efxcreativereverb.flModulationTime); alTestWarning("AL_EAXREVERB_MODULATION_TIME");
	alEffectf(_AlEffect, AL_EAXREVERB_MODULATION_DEPTH, efxcreativereverb.flModulationDepth); alTestWarning("AL_EAXREVERB_MODULATION_DEPTH");
	alEffectf(_AlEffect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, efxcreativereverb.flAirAbsorptionGainHF); alTestWarning("AL_EAXREVERB_AIR_ABSORPTION_GAINHF");
	alEffectf(_AlEffect, AL_EAXREVERB_HFREFERENCE, efxcreativereverb.flHFReference); alTestWarning("AL_EAXREVERB_HFREFERENCE");
	alEffectf(_AlEffect, AL_EAXREVERB_LFREFERENCE, efxcreativereverb.flLFReference); alTestWarning("AL_EAXREVERB_LFREFERENCE");
	alEffectf(_AlEffect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, efxcreativereverb.flRoomRolloffFactor); alTestWarning("AL_EAXREVERB_ROOM_ROLLOFF_FACTOR");
	alEffecti(_AlEffect, AL_EAXREVERB_DECAY_HFLIMIT, efxcreativereverb.iDecayHFLimit); alTestWarning("AL_EAXREVERB_DECAY_HFLIMIT"); // note: spec says AL_EAXREVERB_DECAYHF_LIMIT
}

#endif /* #if EFX_CREATIVE_AVAILABLE */

// ******************************************************************

} /* namespace NLSOUND */

/* end of file */
