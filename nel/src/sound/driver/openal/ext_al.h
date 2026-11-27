// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_EXT_AL_H
#define NL_EXT_AL_H

#ifndef EFX_CREATIVE_AVAILABLE
#define EFX_CREATIVE_AVAILABLE 0
#endif

#ifndef EAX_AVAILABLE
#define EAX_AVAILABLE 0
#endif

#if EAX_AVAILABLE
#	define OPENAL
#	ifdef NL_OS_WINDOWS
#		include <objbase.h>
#	endif
#	include <eax.h>
#endif
#if EFX_CREATIVE_AVAILABLE
#	include <AL/efx-creative.h>
#	include <AL/EFX-Util.h>
#endif
#ifdef AL_LIBTYPE_STATIC
// the static library exports the EFX functions
#	ifndef AL_ALEXT_PROTOTYPES
#		define AL_ALEXT_PROTOTYPES
#	endif
#	ifdef NL_OS_MAC
#		include <efx.h>
#	else
#		include <AL/efx.h>
#	endif
#endif

extern "C"
{

void alExtInit();
void alExtInitDevice(ALCdevice *device);

#if EAX_AVAILABLE
// EAX
extern bool AlExtEax;
extern EAXSet eaxSet;
extern EAXGet eaxGet;
#endif

// ALC_ENUMERATION_EXT
extern bool AlEnumerationExt;

// ALC_ENUMERATE_ALL_EXT
extern bool AlEnumerateAllExt;
#ifndef ALC_DEFAULT_ALL_DEVICES_SPECIFIER
#	define ALC_DEFAULT_ALL_DEVICES_SPECIFIER 0x1012
#endif
#ifndef ALC_ALL_DEVICES_SPECIFIER
#	define ALC_ALL_DEVICES_SPECIFIER 0x1013
#endif

// ALC_EXT_disconnect
extern bool AlExtDisconnect;
#ifndef ALC_CONNECTED
#	define ALC_CONNECTED 0x313
#endif

// ALC_SOFT_reopen_device and ALC_SOFT_system_events, looked up at runtime so older libraries still link
extern bool AlSoftReopenDevice;
typedef ALCboolean (ALC_APIENTRY *TAlcReopenDevice)(ALCdevice *device, const ALCchar *deviceName, const ALCint *attribs);
extern TAlcReopenDevice alcReopenDevice;
extern bool AlSoftSystemEvents;
#ifndef ALC_PLAYBACK_DEVICE_SOFT
#	define ALC_PLAYBACK_DEVICE_SOFT 0x19D4
#endif
#ifndef ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT
#	define ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT 0x19D6
#endif
typedef void (ALC_APIENTRY *TAlcEventProc)(ALCenum eventType, ALCenum deviceType, ALCdevice *device, ALCsizei length, const ALCchar *message, void *userParam);
typedef ALCboolean (ALC_APIENTRY *TAlcEventControl)(ALCsizei count, const ALCenum *events, ALCboolean enable);
extern TAlcEventControl alcEventControl;
typedef void (ALC_APIENTRY *TAlcEventCallback)(TAlcEventProc callback, void *userParam);
extern TAlcEventCallback alcEventCallback;

// AL_SOFTX_hold_on_disconnect
extern bool AlSoftHoldOnDisconnect;
#ifndef AL_STOP_SOURCES_ON_DISCONNECT_SOFT
#	define AL_STOP_SOURCES_ON_DISCONNECT_SOFT 0x19AB
#endif

// ALC_SOFT_HRTF
#ifndef ALC_HRTF_SOFT
#	define ALC_HRTF_SOFT 0x1992
#endif

// EAX-RAM (see OpenAL Programmer's Guide.pdf and http://icculus.org/alextreg/)
extern bool AlExtXRam;
typedef ALboolean (AL_APIENTRY *EAXSetBufferMode)(ALsizei n, ALuint *buffers, ALint value);
extern EAXSetBufferMode eaxSetBufferMode;
typedef ALenum (AL_APIENTRY *EAXGetBufferMode)(ALuint buffer, ALint *value);
extern EAXGetBufferMode eaxGetBufferMode;

// ALC_EXT_EFX (see Effects Extension Guide.pdf and http://icculus.org/alextreg/)
extern bool AlExtEfx;
#define ALC_MAX_AUXILIARY_SENDS 0x20003
typedef void (AL_APIENTRY *LPALGENEFXOBJECTS)(ALsizei n, ALuint* objects);
typedef void (AL_APIENTRY *LPALDELETEEFXOBJECTS)(ALsizei n, ALuint* objects);
typedef ALboolean (AL_APIENTRY *LPALISEFXOBJECT)(ALuint id);
typedef void (AL_APIENTRY *LPALEFXOBJECTI)(ALuint id, ALenum param, ALint value);
typedef void (AL_APIENTRY *LPALEFXOBJECTIV)(ALuint id, ALenum param, ALint* values);
typedef void (AL_APIENTRY *LPALEFXOBJECTF)(ALuint id, ALenum param, ALfloat value);
typedef void (AL_APIENTRY *LPALEFXOBJECTFV)(ALuint id, ALenum param, ALfloat* values);
typedef void (AL_APIENTRY *LPALGETEFXOBJECTI)(ALuint id, ALenum pname, ALint* value);
typedef void (AL_APIENTRY *LPALGETEFXOBJECTIV)(ALuint id, ALenum pname, ALint* values);
typedef void (AL_APIENTRY *LPALGETEFXOBJECTF)(ALuint id, ALenum pname, ALfloat* value);
typedef void (AL_APIENTRY *LPALGETEFXOBJECTFV)(ALuint id, ALenum pname, ALfloat* values);
// source objects
#define AL_DIRECT_FILTER 0x20005
#define AL_AUXILIARY_SEND_FILTER 0x20006
#define AL_AIR_ABSORPTION_FACTOR 0x20007
#define AL_ROOM_ROLLOFF_FACTOR 0x20008
#define AL_CONE_OUTER_GAINHF 0x20009
#define AL_DIRECT_FILTER_GAINHF_AUTO 0x2000A
#define AL_AUXILIARY_SEND_FILTER_GAIN_AUTO 0x2000B
#define AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO 0x2000C
// effect objects
#ifndef AL_LIBTYPE_STATIC
extern LPALGENEFXOBJECTS alGenEffects;
extern LPALDELETEEFXOBJECTS alDeleteEffects;
extern LPALISEFXOBJECT alIsEffect;
extern LPALEFXOBJECTI alEffecti;
extern LPALEFXOBJECTIV alEffectiv;
extern LPALEFXOBJECTF alEffectf;
extern LPALEFXOBJECTFV alEffectfv;
extern LPALGETEFXOBJECTI alGetEffecti;
extern LPALGETEFXOBJECTIV alGetEffectiv;
extern LPALGETEFXOBJECTF alGetEffectf;
extern LPALGETEFXOBJECTFV alGetEffectfv;
#endif
#define AL_EFFECT_FIRST_PARAMETER 0x0000
#define AL_EFFECT_LAST_PARAMETER 0x8000
#define AL_EFFECT_TYPE 0x8001
#define AL_EFFECT_NULL 0x0000
#define AL_EFFECT_REVERB 0x0001
#define AL_REVERB_DENSITY 0x0001
#define AL_REVERB_DIFFUSION 0x0002
#define AL_REVERB_GAIN 0x0003
#define AL_REVERB_GAINHF 0x0004
#define AL_REVERB_DECAY_TIME 0x0005
#define AL_REVERB_DECAY_HFRATIO 0x0006
#define AL_REVERB_REFLECTIONS_GAIN 0x0007
#define AL_REVERB_REFLECTIONS_DELAY 0x0008
#define AL_REVERB_LATE_REVERB_GAIN 0x0009
#define AL_REVERB_LATE_REVERB_DELAY 0x000A
#define AL_REVERB_AIR_ABSORPTION_GAINHF 0x000B 
#define AL_REVERB_ROOM_ROLLOFF_FACTOR 0x000C
#define AL_REVERB_DECAY_HFLIMIT 0x000D
// filter objects
#ifndef AL_LIBTYPE_STATIC
extern LPALGENEFXOBJECTS alGenFilters;
extern LPALDELETEEFXOBJECTS alDeleteFilters;
extern LPALISEFXOBJECT alIsFilter;
extern LPALEFXOBJECTI alFilteri;
extern LPALEFXOBJECTIV alFilteriv;
extern LPALEFXOBJECTF alFilterf;
extern LPALEFXOBJECTFV alFilterfv;
extern LPALGETEFXOBJECTI alGetFilteri;
extern LPALGETEFXOBJECTIV alGetFilteriv;
extern LPALGETEFXOBJECTF alGetFilterf;
extern LPALGETEFXOBJECTFV alGetFilterfv;
#endif
#define AL_LOWPASS_GAIN 0x0001
#define AL_LOWPASS_GAINHF 0x0002
#define AL_HIGHPASS_GAIN 0x0001
#define AL_HIGHPASS_GAINLF 0x0002
#define AL_BANDPASS_GAIN 0x0001
#define AL_BANDPASS_GAINLF 0x0002
#define AL_BANDPASS_GAINHF 0x0003
#define AL_FILTER_FIRST_PARAMETER 0x0000
#define AL_FILTER_LAST_PARAMETER 0x8000
#define AL_FILTER_TYPE 0x8001
#define AL_FILTER_NULL 0x0000
#define	AL_FILTER_LOWPASS 0x0001
#define AL_FILTER_HIGHPASS 0x0002
#define AL_FILTER_BANDPASS 0x0003
// submix objects
#ifndef AL_LIBTYPE_STATIC
extern LPALGENEFXOBJECTS alGenAuxiliaryEffectSlots;
extern LPALDELETEEFXOBJECTS alDeleteAuxiliaryEffectSlots;
extern LPALISEFXOBJECT alIsAuxiliaryEffectSlot;
extern LPALEFXOBJECTI alAuxiliaryEffectSloti;
extern LPALEFXOBJECTIV alAuxiliaryEffectSlotiv;
extern LPALEFXOBJECTF alAuxiliaryEffectSlotf;
extern LPALEFXOBJECTFV alAuxiliaryEffectSlotfv;
extern LPALGETEFXOBJECTI alGetAuxiliaryEffectSloti;
extern LPALGETEFXOBJECTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETEFXOBJECTF alGetAuxiliaryEffectSlotf;
extern LPALGETEFXOBJECTFV alGetAuxiliaryEffectSlotfv;
#endif
#define AL_EFFECTSLOT_EFFECT 0x0001
#define AL_EFFECTSLOT_GAIN  0x0002
#define AL_EFFECTSLOT_AUXILIARY_SEND_AUTO 0x0003
#define AL_EFFECTSLOT_NULL 0x0000

}

#endif /* #ifndef NL_EXT_AL_H */

/* end of file */
