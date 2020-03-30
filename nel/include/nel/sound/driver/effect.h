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

namespace NLSOUND
{

////                                             in dB    ---
//#define NLSOUND_MATERIAL_PRESET_SINGLEWINDOW  -28.00f, 0.71f
//#define NLSOUND_MATERIAL_PRESET_DOUBLEWINDOW  -50.00f, 0.40f
//#define NLSOUND_MATERIAL_PRESET_THINDOOR      -18.00f, 0.66f
//#define NLSOUND_MATERIAL_PRESET_THICKDOOR     -44.00f, 0.64f
//#define NLSOUND_MATERIAL_PRESET_WOODWALL      -40.00f, 0.50f
//#define NLSOUND_MATERIAL_PRESET_BRICKWALL     -50.00f, 0.60f
//#define NLSOUND_MATERIAL_PRESET_STONEWALL     -60.00f, 0.68f
//#define NLSOUND_MATERIAL_PRESET_CURTAIN       -12.00f, 0.15f

inline float decibelsToAmplitudeRatio(float d)
{
	return powf(10.0f, d / 20.0f);
}

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
		Implemented by the NeL drivers:
		Not yet implemented:
		- RoomSize [1.0, 100.0] meters, default 100.0 m
			This parameter can be set seperately from the Reverb Environment structure, it adjusts several settings at once.
			It adjust Reflections, ReflectionsDelay, LateReverb, LateReverbDelay, and DecayTime behind the scenes.
		- RoomSizeFactor [0.0, 100.0] (percentage), default 100.0
			Scales the influence of the RoomSize on the Reverb Environment settings.
		- AirAbsorptionHF [-100.0, 0.0] in dB, default: 0 dB
			High sound frequencies absorbed by the air in the environment (fog, dust, smoke, underwater, etc).
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
		- RoomRolloffFactor
			Source rolloff factor for the reverb path related  to the main listener rolloff factor. With manual rolloff, alpha per source is used.
		Limited availability, not yet implemented:
		- ReflectionsPan
		- LateReverbPan
		Not available in the APIs:
		- DecayHFLimit [true/false], default: unknown
			High frequency delay time stays below some value based on AirAbsorptionHF, and does not take DecayHFRatio into account.
		- DecayLFRatio [0.1, 2.0], default: 1.0
			DecayTime ratio for low frequencies.
		- EchoTimeScale [true/false], default: unknown
			If true, EchoTime depends on RoomSize.
		- ModulationTimeScale [true/false], default: unknown
			If true, ModulationTime depends on RoomSize.
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
		CEnvironment(float roomFilter, float roomFilterHF, 
			float decayTime, float decayHFRatio, float reflections, 
			float reflectionsDelay, float lateReverb, float lateReverbDelay, 
			float diffusion, float density) : 
			RoomFilter(roomFilter), RoomFilterHF(roomFilterHF), 
			DecayTime(decayTime), DecayHFRatio(decayHFRatio), Reflections(reflections), 
			ReflectionsDelay(reflectionsDelay), LateReverb(lateReverb), 
			LateReverbDelay(lateReverbDelay), Diffusion(diffusion), Density(density) { }
		/// Default constructor.
		CEnvironment() : RoomFilter(-100.00f), RoomFilterHF(0.00f), 
			DecayTime(1.0f), DecayHFRatio(1.0f), Reflections(-100.00f), 
			ReflectionsDelay(0.02f), LateReverb(-100.00f), LateReverbDelay(0.04f), 
			Diffusion(100.0f), Density(100.0f) { }
		/// Constructor to fade between two environments.
		CEnvironment(const CEnvironment &env0, const CEnvironment &env1, float balance) :
			RoomFilter((env0.RoomFilter * (1.0f - balance)) + (env1.RoomFilter * balance)), 
			RoomFilterHF((env0.RoomFilterHF * (1.0f - balance)) + (env1.RoomFilterHF * balance)), 
			DecayTime((env0.DecayTime * (1.0f - balance)) + (env1.DecayTime * balance)), 
			DecayHFRatio((env0.DecayHFRatio * (1.0f - balance)) + (env1.DecayHFRatio * balance)), 
			Reflections((env0.Reflections * (1.0f - balance)) + (env1.Reflections * balance)), 
			ReflectionsDelay((env0.ReflectionsDelay * (1.0f - balance)) + (env1.ReflectionsDelay * balance)), 
			LateReverb((env0.LateReverb * (1.0f - balance)) + (env1.LateReverb * balance)), 
			LateReverbDelay((env0.LateReverbDelay * (1.0f - balance)) + (env1.LateReverbDelay * balance)), 
			Diffusion((env0.Diffusion * (1.0f - balance)) + (env1.Diffusion * balance)), 
			Density((env0.Density * (1.0f - balance)) + (env1.Density * balance)) { }
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
		/* This struct can *float* on water! */
	};
	
	IReverbEffect();
	virtual ~IReverbEffect();
	
	/// Set the environment (you have full control now, have fun); size: [1.0, 100.0] in meters, default: 7.5 m; influences environment parameters, 7.5 is no change
	virtual void setEnvironment(const CEnvironment &environment = CEnvironment(), float roomSize = 7.5f) = 0;
	
}; /* class IReverbEffect */

} /* namespace NLSOUND */

#endif /* #ifndef NLSOUND_EFFECT_H */

/* end of file */
