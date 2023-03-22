// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010  Matt RAYKOWSKI (sfb) <matt.raykowski@gmail.com>
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

#include "stdsound_lowlevel.h"

#include "nel/misc/types_nl.h"
#include "nel/sound/driver/effect.h"

using namespace std;
// using namespace NLMISC;

namespace NLSOUND {

IReverbEffect::IReverbEffect()
{
}

IReverbEffect::~IReverbEffect()
{
}

// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide.
// This software is distributed without any warranty.
void IReverbEffect::CEnvironment::resize(float roomSize)
{
	// Clamp the room size to the valid range
	roomSize = std::min(std::max(roomSize, 1.0f), 100.0f);

	// Return early if the environment size is unchanged
	if (roomSize == RoomSize)
		return;

	// Calculate the scale factor for adjusting the properties
	const float scaleFactor = roomSize / RoomSize;
	const uint8 flags = Flags;
	RoomSize = roomSize;
	bool calcScaleFactorLog10 = false;
	float scaleFactorLog10;

	// Decay Time adjustment
	if (flags & NLSOUND_ENVIRONMENT_DECAY_TIME_SCALE)
	{
		float decayTime = scaleFactor * DecayTime;
		DecayTime = std::min(std::max(0.1f, decayTime), 20.0f);
	}

	// Reflections adjustment
	if (flags & NLSOUND_ENVIRONMENT_REFLECTIONS_SCALE)
	{
		if (flags & NLSOUND_ENVIRONMENT_REFLECTIONS_DELAY_SCALE)
		{
			if (!calcScaleFactorLog10)
			{
				scaleFactorLog10 = log10f(scaleFactor);
				calcScaleFactorLog10 = true;
			}
			float reflectionsAdjust = 20.0f * scaleFactorLog10;
			float reflections = Reflections - reflectionsAdjust;
			Reflections = std::min(std::max(-100.0f, reflections), 10.0f);
		}
	}

	// Reflections Delay adjustment
	if (flags & NLSOUND_ENVIRONMENT_REFLECTIONS_DELAY_SCALE)
	{
		float reflectionsDelay = ReflectionsDelay * scaleFactor;
		ReflectionsDelay = std::min(std::max(0.0f, reflectionsDelay), 0.3f);
	}

	// Reverb adjustment
	if (flags & NLSOUND_ENVIRONMENT_LATE_REVERB_SCALE)
	{
		if (!calcScaleFactorLog10)
		{
			scaleFactorLog10 = log10f(scaleFactor);
			calcScaleFactorLog10 = true;
		}
		// The value of 20.0f used in the scaling comes from the relationship between the decibel (dB) scale and the linear scale used to represent the intensity of sound.
		float lateReverbAdjust = ((flags & NLSOUND_ENVIRONMENT_DECAY_TIME_SCALE) ? 20.0f : 30.0f) * scaleFactorLog10;
		float lateReverb = LateReverb - lateReverbAdjust;
		LateReverb = std::min(std::max(-100.0f, lateReverb), 20.0f);
	}

	// Reverb Delay adjustment
	if (flags & NLSOUND_ENVIRONMENT_LATE_REVERB_DELAY_SCALE)
	{
		float reverbDelay = scaleFactor * LateReverbDelay;
		LateReverbDelay = std::min(std::max(0.0f, reverbDelay), 0.1f);
	}

	/*
	// Echo Time adjustment
	if (flags & NLSOUND_ENVIRONMENT_ECHO_TIME_SCALE)
	{
	    float echoTime = EchoTime * scaleFactor;
	    EchoTime = std::min(std::max(0.075f, echoTime), 0.25f);
	}

	// Modulation Time adjustment
	if (flags & NLSOUND_ENVIRONMENT_MODULATION_TIME_Scale)
	{
	    float modulationTime = scaleFactor * ModulationTime;
	    ModulationTime = std::min(std::max(0.04f, modulationTime), 4.0f);
	}
	*/
}

// To the extent possible under law, the author(s) have dedicated all
// copyright and related and neighboring rights to this software to the
// public domain worldwide.
// This software is distributed without any warranty.
CFilterParameters::CFilterParameters(float occlusion, float occlusionLfRatio, float occlusionRoomRatio, float obstruction)
{
	const float occlusionDirectRatio = 1.0f;

	// Calculate gain for direct low-pass filter
	float directDb = std::min(occlusionDirectRatio * occlusionLfRatio,
		occlusionDirectRatio + occlusionLfRatio - 1.0f);
	DirectGain = decibelsToAmplitudeRatio(directDb * obstruction);

	// Set direct high-frequency gain
	DirectGainPass = decibelsToAmplitudeRatio(occlusion) * occlusionDirectRatio;

	// Calculate gain for effect low-pass filter
	float effectDb = std::min(occlusionRoomRatio * occlusionLfRatio,
		occlusionRoomRatio + occlusionLfRatio - 1.0f);
	EffectGain = decibelsToAmplitudeRatio(effectDb * obstruction);

	// Set effect high-frequency gain
	EffectGainPass = decibelsToAmplitudeRatio(occlusion) * occlusionRoomRatio;

	// Calculate obstruction low-pass filter parameters.
	float obstructionGain = decibelsToAmplitudeRatio(obstruction * occlusionLfRatio);
	float obstructionGainPass = decibelsToAmplitudeRatio(obstruction);
	DirectGain *= obstructionGain;
	DirectGainPass *= obstructionGainPass;
}

} /* namespace NLSOUND */

/* end of file */
