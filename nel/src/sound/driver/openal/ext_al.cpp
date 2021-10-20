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

extern "C"
{

void alExtInit()
{
	nldebug("AL: Initializing extensions");

	if ((AlEnumerationExt = (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT") == AL_TRUE)) == true)
	{
		// ...
	}

	if ((AlEnumerateAllExt = (alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE)) == true)
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

// Windows and Mac OS always link to shared OpenAL library
#if defined(NL_OS_WINDOWS) || defined(NL_OS_MAC) || !defined(NL_STATIC)
	// EFX
	if ((AlExtEfx = (alcIsExtensionPresent(device, "ALC_EXT_EFX") == ALC_TRUE)) == true)
	{
		// effect objects
		alGenEffects = (LPALGENEFXOBJECTS)alGetProcAddress("alGenEffects");
		alDeleteEffects = (LPALDELETEEFXOBJECTS)alGetProcAddress("alDeleteEffects");
		alIsEffect = (LPALISEFXOBJECT)alGetProcAddress("alIsEffect");
		alEffecti = (LPALEFXOBJECTI)alGetProcAddress("alEffecti");
		alEffectiv = (LPALEFXOBJECTIV)alGetProcAddress("alEffectiv");
		alEffectf = (LPALEFXOBJECTF)alGetProcAddress("alEffectf");
		alEffectfv = (LPALEFXOBJECTFV)alGetProcAddress("alEffectfv");
		alGetEffecti = (LPALGETEFXOBJECTI)alGetProcAddress("alGetEffecti");
		alGetEffectiv = (LPALGETEFXOBJECTIV)alGetProcAddress("alGetEffectiv");
		alGetEffectf = (LPALGETEFXOBJECTF)alGetProcAddress("alGetEffectf");
		alGetEffectfv = (LPALGETEFXOBJECTFV)alGetProcAddress("alGetEffectfv");
		// effect objects
		alGenFilters = (LPALGENEFXOBJECTS)alGetProcAddress("alGenFilters");
		alDeleteFilters = (LPALDELETEEFXOBJECTS)alGetProcAddress("alDeleteFilters");
		alIsFilter = (LPALISEFXOBJECT)alGetProcAddress("alIsFilter");
		alFilteri = (LPALEFXOBJECTI)alGetProcAddress("alFilteri");
		alFilteriv = (LPALEFXOBJECTIV)alGetProcAddress("alFilteriv");
		alFilterf = (LPALEFXOBJECTF)alGetProcAddress("alFilterf");
		alFilterfv = (LPALEFXOBJECTFV)alGetProcAddress("alFilterfv");
		alGetFilteri = (LPALGETEFXOBJECTI)alGetProcAddress("alGetFilteri");
		alGetFilteriv = (LPALGETEFXOBJECTIV)alGetProcAddress("alGetFilteriv");
		alGetFilterf = (LPALGETEFXOBJECTF)alGetProcAddress("alGetFilterf");
		alGetFilterfv = (LPALGETEFXOBJECTFV)alGetProcAddress("alGetFilterfv");
		// submix objects
		alGenAuxiliaryEffectSlots = (LPALGENEFXOBJECTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		alDeleteAuxiliaryEffectSlots = (LPALDELETEEFXOBJECTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		alIsAuxiliaryEffectSlot = (LPALISEFXOBJECT)alGetProcAddress("alIsAuxiliaryEffectSlot");
		alAuxiliaryEffectSloti = (LPALEFXOBJECTI)alGetProcAddress("alAuxiliaryEffectSloti");
		alAuxiliaryEffectSlotiv = (LPALEFXOBJECTIV)alGetProcAddress("alAuxiliaryEffectSlotiv");
		alAuxiliaryEffectSlotf = (LPALEFXOBJECTF)alGetProcAddress("alAuxiliaryEffectSlotf");
		alAuxiliaryEffectSlotfv = (LPALEFXOBJECTFV)alGetProcAddress("alAuxiliaryEffectSlotfv");
		alGetAuxiliaryEffectSloti = (LPALGETEFXOBJECTI)alGetProcAddress("alGetAuxiliaryEffectSloti");
		alGetAuxiliaryEffectSlotiv = (LPALGETEFXOBJECTIV)alGetProcAddress("alGetAuxiliaryEffectSlotiv");
		alGetAuxiliaryEffectSlotf = (LPALGETEFXOBJECTF)alGetProcAddress("alGetAuxiliaryEffectSlotf");
		alGetAuxiliaryEffectSlotfv = (LPALGETEFXOBJECTFV)alGetProcAddress("alGetFilterfv");
		if (!alGenEffects || !alGenFilters || !alGenAuxiliaryEffectSlots)
		{
			nlwarning("AL: ALC_EXT_EFX alcGetProcAddress failed");
			AlExtEfx = false;
		}
	}
#endif
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
EAXSetBufferMode eaxSetBufferMode = NULL;
EAXGetBufferMode eaxGetBufferMode = NULL;

// ALC_EXT_EFX
bool AlExtEfx = false;
// effect objects
#if defined(NL_OS_WINDOWS) || defined(NL_OS_MAC) || !defined(NL_STATIC)
LPALGENEFXOBJECTS alGenEffects = NULL;
LPALDELETEEFXOBJECTS alDeleteEffects = NULL;
LPALISEFXOBJECT alIsEffect = NULL;
LPALEFXOBJECTI alEffecti = NULL;
LPALEFXOBJECTIV alEffectiv = NULL;
LPALEFXOBJECTF alEffectf = NULL;
LPALEFXOBJECTFV alEffectfv = NULL;
LPALGETEFXOBJECTI alGetEffecti = NULL;
LPALGETEFXOBJECTIV alGetEffectiv = NULL;
LPALGETEFXOBJECTF alGetEffectf = NULL;
LPALGETEFXOBJECTFV alGetEffectfv = NULL;
// filter objects
LPALGENEFXOBJECTS alGenFilters = NULL;
LPALDELETEEFXOBJECTS alDeleteFilters = NULL;
LPALISEFXOBJECT alIsFilter = NULL;
LPALEFXOBJECTI alFilteri = NULL;
LPALEFXOBJECTIV alFilteriv = NULL;
LPALEFXOBJECTF alFilterf = NULL;
LPALEFXOBJECTFV alFilterfv = NULL;
LPALGETEFXOBJECTI alGetFilteri = NULL;
LPALGETEFXOBJECTIV alGetFilteriv = NULL;
LPALGETEFXOBJECTF alGetFilterf = NULL;
LPALGETEFXOBJECTFV alGetFilterfv = NULL;
// submix objects
LPALGENEFXOBJECTS alGenAuxiliaryEffectSlots = NULL;
LPALDELETEEFXOBJECTS alDeleteAuxiliaryEffectSlots = NULL;
LPALISEFXOBJECT alIsAuxiliaryEffectSlot = NULL;
LPALEFXOBJECTI alAuxiliaryEffectSloti = NULL;
LPALEFXOBJECTIV alAuxiliaryEffectSlotiv = NULL;
LPALEFXOBJECTF alAuxiliaryEffectSlotf = NULL;
LPALEFXOBJECTFV alAuxiliaryEffectSlotfv = NULL;
LPALGETEFXOBJECTI alGetAuxiliaryEffectSloti = NULL;
LPALGETEFXOBJECTIV alGetAuxiliaryEffectSlotiv = NULL;
LPALGETEFXOBJECTF alGetAuxiliaryEffectSlotf = NULL;
LPALGETEFXOBJECTFV alGetAuxiliaryEffectSlotfv = NULL;
#endif
}

/* end of file */
