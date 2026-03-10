// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2021  Winch Gate Property Limited
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

extern "C"
{

void alExtInit()
{
	nldebug("AL: Initializing extensions");

	if ((AlEnumerationExt = (alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT") == AL_TRUE)) == true)
	{
		// ...
	}

	if ((AlEnumerateAllExt = (alcIsExtensionPresent(nullptr, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE)) == true)
	{
		// ...
	}
}

void alExtInitDevice(ALCdevice *device)
{
	nldebug("AL: Initializing device extensions");

#if EAX_AVAILABLE
	// EAX
	if (AlExtEax = (alIsExtensionPresent("EAX") == AL_TRUE))
	{
		eaxSet = (EAXSet)alGetProcAddress("EAXSet");
		eaxGet = (EAXGet)alGetProcAddress("EAXGet");
		if (!eaxSet || !eaxGet)
		{
			nlwarning("AL: EAX alGetProcAddress failed");
			AlExtEax = false;
		}
	}
#endif
	
	// EAX-RAM
	if ((AlExtXRam = ((alIsExtensionPresent("EAX-RAM") == AL_TRUE)
		|| (alIsExtensionPresent("EAX_RAM") == AL_TRUE))) == true)
	{
		eaxSetBufferMode = (EAXSetBufferMode)alGetProcAddress("EAXSetBufferMode");
		eaxGetBufferMode = (EAXGetBufferMode)alGetProcAddress("EAXGetBufferMode");
		if (!eaxSetBufferMode || !eaxGetBufferMode)
		{
			nlwarning("AL: EAX-RAM alGetProcAddress failed");
			AlExtXRam = false;
		}
	}

	// EFX
	if ((AlExtEfx = (alcIsExtensionPresent(device, "ALC_EXT_EFX") == ALC_TRUE)) == true)
	{
#if !defined(AL_ALEXT_PROTOTYPES)
		// effect objects
		alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
		alIsEffect = (LPALISEFFECT)alGetProcAddress("alIsEffect");
		alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
		alEffectiv = (LPALEFFECTIV)alGetProcAddress("alEffectiv");
		alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		alEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
		alGetEffecti = (LPALGETEFFECTI)alGetProcAddress("alGetEffecti");
		alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress("alGetEffectiv");
		alGetEffectf = (LPALGETEFFECTF)alGetProcAddress("alGetEffectf");
		alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress("alGetEffectfv");
		// effect objects
		alGenFilters = (LPALGENFILTERS)alGetProcAddress("alGenFilters");
		alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress("alDeleteFilters");
		alIsFilter = (LPALISFILTER)alGetProcAddress("alIsFilter");
		alFilteri = (LPALFILTERI)alGetProcAddress("alFilteri");
		alFilteriv = (LPALFILTERIV)alGetProcAddress("alFilteriv");
		alFilterf = (LPALFILTERF)alGetProcAddress("alFilterf");
		alFilterfv = (LPALFILTERFV)alGetProcAddress("alFilterfv");
		alGetFilteri = (LPALGETFILTERI)alGetProcAddress("alGetFilteri");
		alGetFilteriv = (LPALGETFILTERIV)alGetProcAddress("alGetFilteriv");
		alGetFilterf = (LPALGETFILTERF)alGetProcAddress("alGetFilterf");
		alGetFilterfv = (LPALGETFILTERFV)alGetProcAddress("alGetFilterfv");
		// submix objects
		alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress("alIsAuxiliaryEffectSlot");
		alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
		alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
		alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress("alAuxiliaryEffectSlotf");
		alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
		alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
		alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
		alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
		alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress("alGetFilterfv");
#endif
		if (!alGenEffects || !alGenFilters || !alGenAuxiliaryEffectSlots)
		{
			nlwarning("AL: ALC_EXT_EFX alcGetProcAddress failed");
			AlExtEfx = false;
		}
	}
}

#if EAX_AVAILABLE
// EAX
bool AlExtEax = false;
EAXSet eaxSet = NULL;
EAXGet eaxGet = NULL;
#endif

// ALC_ENUMERATION_EXT
bool AlEnumerationExt = false;

// ALC_ENUMERATE_ALL_EXT
bool AlEnumerateAllExt = false;

// EAX-RAM
bool AlExtXRam = false;
EAXSetBufferMode eaxSetBufferMode = nullptr;
EAXGetBufferMode eaxGetBufferMode = nullptr;

// ALC_EXT_EFX
bool AlExtEfx = false;
// effect objects
#if !defined(AL_ALEXT_PROTOTYPES)
LPALGENEFFECTS alGenEffects = nullptr;
LPALDELETEEFFECTS alDeleteEffects = nullptr;
LPALISEFFECT alIsEffect = nullptr;
LPALEFFECTI alEffecti = nullptr;
LPALEFFECTIV alEffectiv = nullptr;
LPALEFFECTF alEffectf = nullptr;
LPALEFFECTFV alEffectfv = nullptr;
LPALGETEFFECTI alGetEffecti = nullptr;
LPALGETEFFECTIV alGetEffectiv = nullptr;
LPALGETEFFECTF alGetEffectf = nullptr;
LPALGETEFFECTFV alGetEffectfv = nullptr;
// filter objects
LPALGENFILTERS alGenFilters = nullptr;
LPALDELETEFILTERS alDeleteFilters = nullptr;
LPALISFILTER alIsFilter = nullptr;
LPALFILTERI alFilteri = nullptr;
LPALFILTERIV alFilteriv = nullptr;
LPALFILTERF alFilterf = nullptr;
LPALFILTERFV alFilterfv = nullptr;
LPALGETFILTERI alGetFilteri = nullptr;
LPALGETFILTERIV alGetFilteriv = nullptr;
LPALGETFILTERF alGetFilterf = nullptr;
LPALGETFILTERFV alGetFilterfv = nullptr;
// submix objects
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = nullptr;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = nullptr;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = nullptr;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = nullptr;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = nullptr;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = nullptr;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = nullptr;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = nullptr;
#endif
}

/* end of file */
