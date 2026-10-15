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

#ifndef NLSOUND_EFFECT_H
#define NLSOUND_EFFECT_H

#include "nel/misc/types_nl.h"

// STL includes
#include <math.h>

namespace NLSOUND {

NL_FORCE_INLINE float decibelsToAmplitudeRatio(float d) // dbToLinearGain
{
	return pow(10, d / 20.0f);
}

NL_FORCE_INLINE float amplitudeRatioToDecibels(float a)
{
	return 20.0f * log10f(a);
}

#define NLSOUND_ENVIRONMENT_DECAY_TIME_SCALE (0x1) /* The decay time is scaled by the environment size */
#define NLSOUND_ENVIRONMENT_REFLECTIONS_SCALE (0x2) /* The reflections attenuation is scaled by the environment size */
#define NLSOUND_ENVIRONMENT_REFLECTIONS_DELAY_SCALE (0x4) /* The reflections delay is scaled by the environment size */
#define NLSOUND_ENVIRONMENT_LATE_REVERB_SCALE (0x8) /* The reverb attenuation is scaled by the environment size */
#define NLSOUND_ENVIRONMENT_LATE_REVERB_DELAY_SCALE (0x10) /* The reverb delay is scaled by the environment size */
#define NLSOUND_ENVIRONMENT_DECAY_HF_LIMIT (0x20) /* The high-frequency decay time is limited by the air absorption at high frequencies (OpenAL only) */
// #define NLSOUND_ENVIRONMENT_ECHO_TIME_SCALE (0x40) /* The echo time is scaled by the environment size (not yet implemented) */
// #define NLSOUND_ENVIRONMENT_MODULATION_TIME_SCALE (0x80) /* The modulation time is scaled by the environment size (not yet implemented) */

#define NLSOUND_MAX_ENVIRONMENT_SIZE (100.0f) /* The maximum environment size */

#define NLSOUND_MIN_OCCLUSION (-100.00f)
#define NLSOUND_MAX_OCCLUSION (0.f)
#define NLSOUND_DEFAULT_OCCLUSION (0.f)

#define NLSOUND_MIN_OBSTRUCTION (-100.00f)
#define NLSOUND_MAX_OBSTRUCTION (0.f)
#define NLSOUND_DEFAULT_OBSTRUCTION (0.f)

/**
 * \brief IReverbEffect
 * \date 2008-09-15 22:27GMT
 * \author Jan Boon (Kaetemi)
 * IReverbEffect
 */
class IReverbEffect
{
public:
	/*
	    Reverb environment
	    Parameters handled by the API:
	    - RoomFilter [-100.00, 0] in dB, default: -100.00 dB
	        Master volume of the environment reverb and reflections.
	    - RoomFilterHF [-100.00, 0] in dB, default: 0 dB
	        Filters high frequency reverb and reflections of the environment.
	    - DecayTime [0.1, 20.0] in seconds, default: 1.0 s
	        The base decay time.
	    - DecayHFRatio [0.0, 0.2], default: 1.0
	        Scales the decay time for high fequencies. Higher ratio means slower and longer decay for high frequencies.
	    - Reflections [-100.00, 10.00] in dB, default: -100.00 dB
	        Volume of the early reflections. Smaller RoomSize results in louder Reflections.
	    - ReflectionsDelay [0.0, 0.3] in seconds, default: 0.02 s
	        Time it takes before the listener hears the early reflection from the source. Smaller RoomSize results in shorter ReflectionsDelay.
	    - LateReverb [-100.00, 20.00] in dB, default: -100.00 dB
	        Volume of the late reverberation. Smaller RoomSize results in louder LateReverb.
	    - LateReverbDelay [0.0, 0.1] in seconds, default: 0.04 s
	        Time for late reverberation after the first reflection from the ReflectionsDelay. Smaller RoomSize results in shorter LateReverbDelay.
	    - Diffusion [0.0, 100.0] (percentage), default: 100.0 %
	        Lower diffusion seperates echoes by decreasing randomization.
	    - Density [0.0, 100.0] (percentage), default: 100.0 %
	        Lower density increases distance between late reverberations.
	    Flags handled by resize:
	    - DecayTimeScale [true/false], default: true
	        If true, DecayTime depends on RoomSize.
	    - ReflectionsScale [true/false], default: true
	        If true, Reflections gain depends on RoomSize.
	    - ReflectionsDelayScale [true/false], default: true
	        If true, ReflectionsDelay depends on RoomSize.
	    - LateReverbScale [true/false], default: true
	        If true, LateReverb gain depends on RoomSize. Smaller RoomSize results in louder LateReverb.
	    - LateReverbDelayScale [true/false], default: true
	        If true, LateReverbDelay depends on RoomSize. Smaller RoomSize results in shorter LateReverbDelay.
	    Flags not implemented:
	    - EchoTimeScale [true/false], default: unknown
	        If true, EchoTime depends on RoomSize.
	    - ModulationTimeScale [true/false], default: unknown
	        If true, ModulationTime depends on RoomSize.
	    Implemented by the NeL drivers:
	    - RoomSize [1.0, 100.0] meters, default 100.0 m
	        This parameter can be set seperately from the Reverb Environment structure, it adjusts several settings at once.
	        It adjust Reflections, ReflectionsDelay, LateReverb, LateReverbDelay, and DecayTime behind the scenes.
	    Not yet implemented:
	    - RoomSizeFactor [0.0, 100.0] (percentage), default 100.0
	        Scales the influence of the RoomSize on the Reverb Environment settings.
	    - AirAbsorptionHF [-100.0, 0.0] in dB, default: 0 dB (AL_REVERB_AIR_ABSORPTION_GAINHF)
	        High sound frequencies absorbed by the air in the environment (fog, dust, smoke, underwater, etc).
	    - RoomRolloffFactor (AL_REVERB_ROOM_ROLLOFF_FACTOR)
	        Source rolloff factor for the reverb path related  to the main listener rolloff factor. With manual rolloff, alpha per source is used.
	    Limited availability, not yet implemented:
	    - ReflectionsPan
	    - LateReverbPan
	    OpenAL flag only:
	    - DecayHFLimit [true/false], default: unknown
	        High frequency delay time stays below some value based on AirAbsorptionHF, and does not take DecayHFRatio into account.
	    Not available in the APIs:
	    - DecayLFRatio [0.1, 2.0], default: 1.0
	        DecayTime ratio for low frequencies.
	    - EchoDepth
	    - EchoTime
	    - HFReference, 5000.0 Hz
	    - LFReference
	    - RoomFilterLF
	    - ModulationDepth
	    - ModulationTime
	*/
	/// Reverb environment
	struct CEnvironment
	{
		/// Constructor with all parameters, can be used with presets, roomsize.
		CEnvironment(sint id, float roomSize,
		    float roomFilter, float roomFilterHF,
		    float decayTime, float decayHFRatio, float reflections,
		    float reflectionsDelay, float lateReverb, float lateReverbDelay,
		    float diffusion, float density,
		    uint8 flags)
		    : Id(id)
		    , RoomSize(roomSize)
		    , RoomFilter(roomFilter)
		    , RoomFilterHF(roomFilterHF)
		    , DecayTime(decayTime)
		    , DecayHFRatio(decayHFRatio)
		    , Reflections(reflections)
		    , ReflectionsDelay(reflectionsDelay)
		    , LateReverb(lateReverb)
		    , LateReverbDelay(lateReverbDelay)
		    , Diffusion(diffusion)
		    , Density(density)
		    , Flags(flags)
		{
		}
		/// Default constructor.
		CEnvironment()
		    : Id(26)
		    , RoomSize(7.5f)
		    , RoomFilter(-100.00f)
		    , RoomFilterHF(0.00f)
		    , DecayTime(1.0f)
		    , DecayHFRatio(1.0f)
		    , Reflections(-100.00f)
		    , ReflectionsDelay(0.02f)
		    , LateReverb(-100.00f)
		    , LateReverbDelay(0.04f)
		    , Diffusion(100.0f)
		    , Density(100.0f)
		    , Flags(0x20)
		{
		}
		/// Constructor to fade between two environments.
		CEnvironment(const CEnvironment &env0, const CEnvironment &env1, float balance)
		    : Id(env0.Id == env1.Id ? env0.Id : 26)
		    , RoomSize((env0.RoomSize * (1.0f - balance)) + (env1.RoomSize * balance))
		    , RoomFilter((env0.RoomFilter * (1.0f - balance)) + (env1.RoomFilter * balance))
		    , RoomFilterHF((env0.RoomFilterHF * (1.0f - balance)) + (env1.RoomFilterHF * balance))
		    , DecayTime((env0.DecayTime * (1.0f - balance)) + (env1.DecayTime * balance))
		    , DecayHFRatio((env0.DecayHFRatio * (1.0f - balance)) + (env1.DecayHFRatio * balance))
		    , Reflections((env0.Reflections * (1.0f - balance)) + (env1.Reflections * balance))
		    , ReflectionsDelay((env0.ReflectionsDelay * (1.0f - balance)) + (env1.ReflectionsDelay * balance))
		    , LateReverb((env0.LateReverb * (1.0f - balance)) + (env1.LateReverb * balance))
		    , LateReverbDelay((env0.LateReverbDelay * (1.0f - balance)) + (env1.LateReverbDelay * balance))
		    , Diffusion((env0.Diffusion * (1.0f - balance)) + (env1.Diffusion * balance))
		    , Density((env0.Density * (1.0f - balance)) + (env1.Density * balance))
		    , Flags(env0.Flags & env1.Flags)
		{
		}

		/// Resize the environment to a new room size [1.0, 100.0] in meters, default 7.5 meters
		void resize(float roomSize = 7.5f);

		/// Legacy environment preset identifier, default: 26
		sint Id;
		/// Reference environment or room size, used for resizing an environment, default: 7.5 meters
		float RoomSize;

		/// [-100.00, 0] in dB, default: -100.00 dB
		float RoomFilter;
		/// [-100.00, 0] in dB, default: 0 dB
		float RoomFilterHF;
		/// [0.1, 20.0] in seconds, default: 1.0 s
		float DecayTime;
		/// [0.1, 2.0], default: 0.1
		float DecayHFRatio;
		/// [-100.00, 10.00] in dB, default: -100.00 dB
		float Reflections;
		/// [0.0, 0.3] in seconds, default: 0.02 s
		float ReflectionsDelay;
		/// [-100.00, 20.00] in dB, default: -100.00 dB
		float LateReverb;
		/// [0.0, 0.1] in seconds, default: 0.04 s
		float LateReverbDelay;
		/// [0.0, 100.0] (percentage), default: 100.0 %
		float Diffusion;
		/// [0.0, 100.0] (percentage), default: 100.0 %
		float Density;

		/// Flags used for resizing an environment, default: 0x20
		uint8 Flags;
	};

	IReverbEffect();
	virtual ~IReverbEffect();

	/// Set the environment (you have full control now, have fun); size: [1.0, 100.0] in meters, default: 7.5 m; influences environment parameters, 7.5 is no change
	virtual void setEnvironment(const CEnvironment &environment = CEnvironment(), float roomSize = 7.5f) = 0;

}; /* class IReverbEffect */

/*

In EAX, occlusion and obstruction are two distinct audio effects used
to simulate how sound is affected by objects in the environment.

1. Occlusion:
Occlusion refers to the reduction in sound intensity when the direct
path between the sound source and the listener is blocked by an object
or a barrier. It affects both the direct sound and the reverberation.
The occlusion effect takes into consideration the material properties
of the object blocking the path, such as its thickness and density. In
EAX, the occlusion effect is characterized by parameters such as
Occlusion, OcclusionLFFactor, and OcclusionRoomRatio.
- Occlusion: Represents the overall reduction in sound level due to the
blocking object.
- OcclusionLFFactor: Represents the degree to which low-frequency
sounds are occluded, as low-frequency sounds can pass through objects
more easily than high-frequency sounds.
- OcclusionRoomRatio: Represents the proportion of direct to
reverberant sound that reaches the listener after being occluded by an
object.

2. Obstruction:
Obstruction refers to the reduction in sound intensity when the direct
path between the sound source and the listener is partially blocked by
an object or a barrier. Unlike occlusion, which affects both direct and
reverberant sound, obstruction only affects the direct sound. In EAX,
the obstruction effect is characterized by a single parameter,
Obstruction.
- Obstruction: Represents the overall reduction in the direct sound
level due to the partially blocking object.

Both occlusion and obstruction effects contribute to creating a more
immersive and realistic audio experience by simulating how sound
behaves in real-world environments when interacting with objects and
barriers.

*/

class CFilterMaterial
{
public:
	/// Represents the overall reduction in sound level due to the blocking object, in dB.
	float Occlusion;

	/// Same as `Occlusion`, but represented as linear gain.
	float OcclusionGain;

	/// Represents the degree to which low-frequency sounds are occluded,
	/// as low-frequency sounds can pass through objects more easily than high-frequency sounds.
	float OcclusionLFFactor;

	/// Represents the proportion of direct to reverberant sound
	/// that reaches the listener after being occluded by an object. (Not implemented)
	float OcclusionRoomRatio;

	float DirectCutoffFrequency;

	float EffectCutoffFrequency;

	CFilterMaterial()
	    : Occlusion(0.0f)
	    , OcclusionGain(1.0f)
	    , OcclusionLFFactor(0.0f)
	    , OcclusionRoomRatio(0.5f)
		, DirectCutoffFrequency(1000.0f)
		, EffectCutoffFrequency(1000.0f)
	{
	}

	CFilterMaterial(float occlusion, float occlusionLFFactor, float occlusionRoomRatio, float directCutoffFrequency, float effectCutoffFrequency)
	    : Occlusion(occlusion)
	    , OcclusionGain(pow(10, (double)occlusion / 20.0f))
	    , OcclusionLFFactor(occlusionLFFactor)
	    , OcclusionRoomRatio(occlusionRoomRatio)
	    , DirectCutoffFrequency(directCutoffFrequency)
	    , EffectCutoffFrequency(effectCutoffFrequency)
	{
	}
};

/// Converts from filter material to a set of low pass filters for the direct and effect sends
class CFilterParameters
{
public:
	float DirectGain; // OpenAL EFX: AL_LOWPASS_GAIN
	float DirectGainPass; // OpenAL EFX: AL_LOWPASS_GAINHF
	// float DirectCutoffFrequency; // XAudio2: XAUDIO2_FILTER_PARAMETERS::Frequency, not supported in OpenAL
	float EffectGain;
	float EffectGainPass;
	// float EffectCutoffFrequency;

	CFilterParameters(float occlusion, float occlusionLfRatio, float occlusionRoomRatio, float obstruction);

};

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_EFFECT_H */

/* end of file */
