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

#include "stddsound.h"

// The one and only INITGUID
#define INITGUID

#ifdef DIRECTSOUND_VERSION
	#undef DIRECTSOUND_VERSION
#endif
#define DIRECTSOUND_VERSION 0x0800

#include "nel/sound/driver/sound_driver.h"

#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/dynloadlib.h"
#include "sound_driver_dsound.h"
#include "listener_dsound.h"


using namespace std;
using namespace NLMISC;


namespace NLSOUND {

CSoundDriverDSound* CSoundDriverDSound::_Instance = NULL;
uint32 CSoundDriverDSound::_TimerPeriod = 100;
HWND CSoundDriverWnd = 0;

/// import io proc def from buffer_dsound.
LRESULT NelIOProc(LPSTR lpmmioinfo, UINT uMsg, LONG lParam1, LONG lParam2);

#ifndef NL_STATIC

HINSTANCE CSoundDriverDllHandle = 0;

// ******************************************************************
// The main entry of the DLL. It's used to get a hold of the hModule handle.
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
  CSoundDriverDllHandle = (HINSTANCE) hModule;
  return TRUE;
}

class CSoundDriverDSoundNelLibrary : public NLMISC::INelLibrary {
	void onLibraryLoaded(bool /* firstTime */) { }
	void onLibraryUnloaded(bool /* lastTime */) { }
};
NLMISC_DECL_PURE_LIB(CSoundDriverDSoundNelLibrary)

#endif /* #ifndef NL_STATIC */


// ******************************************************************
// The event handling procedure of the invisible window created below.

LRESULT CALLBACK CSoundDriverCreateWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstanceDSound
#else
__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance
#endif
	(ISoundDriver::IStringMapperProvider *stringMapper)
{
#ifdef NL_STATIC
	HINSTANCE CSoundDriverDllHandle = (HINSTANCE)GetModuleHandle(NULL);
#endif

	static bool Registered = false;

	if (!Registered)
	{
		// Don't ask me why we have to create a window to do sound!
		// echo <your comment> | mail support@microsoft.com -s "F#%@cking window"
		WNDCLASS myClass;
		myClass.hCursor = LoadCursor( NULL, IDC_ARROW );
		myClass.hIcon = NULL;
		myClass.lpszMenuName = (LPSTR) NULL;
		myClass.lpszClassName = (LPSTR) "CSoundDriver";
		myClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		myClass.hInstance = CSoundDriverDllHandle;
		myClass.style = CS_GLOBALCLASS;
		myClass.lpfnWndProc = CSoundDriverCreateWindowProc;
		myClass.cbClsExtra = 0;
		myClass.cbWndExtra = 0;

		if (!RegisterClass(&myClass))
		{
			nlwarning("Failed to initialize the sound driver (RegisterClass)");
			return 0;
		}

		Registered = true;
	}

	CSoundDriverWnd = CreateWindow((LPSTR) "CSoundDriver", (LPSTR) "CSoundDriver", WS_OVERLAPPEDWINDOW,
									CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, (HWND) NULL, (HMENU) NULL,
									CSoundDriverDllHandle, (LPSTR) NULL);

	if (CSoundDriverWnd == NULL)
	{
		nlwarning("Failed to initialize the sound driver (CreateWindow)");
		return 0;
	}

/*	// install the NeL Io routine
	LPMMIOPROC ret = mmioInstallIOProc(mmioStringToFOURCC("NEL_", 0), (LPMMIOPROC)NelIOProc, MMIO_INSTALLPROC);
	nlassert(ret != 0);
*/

	return new CSoundDriverDSound(stringMapper);
}

// ******************************************************************

#ifdef NL_STATIC
uint32 interfaceVersionDSound()
#else
__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
#endif
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

#ifdef NL_STATIC
void outputProfileDSound
#else
__declspec(dllexport) void NLSOUND_outputProfile
#endif
	(string &out)
{
	CSoundDriverDSound::instance()->writeProfile(out);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver::TDriver getDriverTypeDSound()
#else
__declspec(dllexport) ISoundDriver::TDriver NLSOUND_getDriverType()
#endif
{
	return ISoundDriver::DriverDSound;
}

// ******************************************************************




// ******************************************************************

CSoundDriverDSound::CSoundDriverDSound(ISoundDriver::IStringMapperProvider *stringMapper)
: _StringMapper(stringMapper)
{
	if ( _Instance == NULL )
	{
		_Instance = this;

        _DirectSound = NULL;
        _PrimaryBuffer = NULL;
        _SourceCount = 0;
        _TimerID = NULL;

#if NLSOUND_PROFILE
        _TimerIntervalCount = 0;
        _TotalTime = 0.0;
        _TotalUpdateTime = 0.0;
		_UpdateCount = 0;
		_UpdateSources = 0;
		_UpdateExec = 0;
#endif

    }
	else
	{
		nlerror("Sound driver singleton instanciated twice");
	}
}


#if EAX_AVAILABLE == 1

LPKSPROPERTYSET	CSoundDriverDSound::createPropertySet(CSourceDSound *source)
{
	if (_Sources.empty())
		return NULL;

	LPDIRECTSOUND3DBUFFER8 d3dBuffer;
	if (source == NULL)
		d3dBuffer = (*_Sources.begin())->_3DBuffer;
	else
	{
		d3dBuffer = source->_3DBuffer;
	}
	LPKSPROPERTYSET	propertySet;
	d3dBuffer->QueryInterface(IID_IKsPropertySet, (void**) &propertySet);

	// some checking code
	{
		if (propertySet != 0)
		{
			char *listenerProperties[] =
			{
				"DSPROPERTY_EAXLISTENER_NONE",
				"DSPROPERTY_EAXLISTENER_ALLPARAMETERS",
				"DSPROPERTY_EAXLISTENER_ROOM",
				"DSPROPERTY_EAXLISTENER_ROOMHF",
				"DSPROPERTY_EAXLISTENER_ROOMROLLOFFFACTOR",
				"DSPROPERTY_EAXLISTENER_DECAYTIME",
				"DSPROPERTY_EAXLISTENER_DECAYHFRATIO",
				"DSPROPERTY_EAXLISTENER_REFLECTIONS",
				"DSPROPERTY_EAXLISTENER_REFLECTIONSDELAY",
				"DSPROPERTY_EAXLISTENER_REVERB",
				"DSPROPERTY_EAXLISTENER_REVERBDELAY",
				"DSPROPERTY_EAXLISTENER_ENVIRONMENT",
				"DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE",
				"DSPROPERTY_EAXLISTENER_ENVIRONMENTDIFFUSION",
				"DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF",
				"DSPROPERTY_EAXLISTENER_FLAGS"
			};
			uint i;
			for (i=DSPROPERTY_EAXLISTENER_NONE; i<= DSPROPERTY_EAXLISTENER_FLAGS; ++i)
			{
				ULONG ulSupport = 0;
				propertySet->QuerySupport(DSPROPSETID_EAX_ListenerProperties, i, &ulSupport);
				if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
				{
//					nlwarning("CSoundDriverDSound::createPropertySet : listener property %s not supported", listenerProperties[i]);
				}
			}

			char *bufferProperties[] =
			{
				"DSPROPERTY_EAXBUFFER_NONE",
				"DSPROPERTY_EAXBUFFER_ALLPARAMETERS",
				"DSPROPERTY_EAXBUFFER_DIRECT",
				"DSPROPERTY_EAXBUFFER_DIRECTHF",
				"DSPROPERTY_EAXBUFFER_ROOM",
				"DSPROPERTY_EAXBUFFER_ROOMHF",
				"DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR",
				"DSPROPERTY_EAXBUFFER_OBSTRUCTION",
				"DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO",
				"DSPROPERTY_EAXBUFFER_OCCLUSION",
				"DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO",
				"DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO",
				"DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF",
				"DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR",
				"DSPROPERTY_EAXBUFFER_FLAGS"
			};

			for (i=DSPROPERTY_EAXBUFFER_NONE; i<=DSPROPERTY_EAXBUFFER_FLAGS; ++i)
			{
				ULONG ulSupport = 0;
				propertySet->QuerySupport(DSPROPSETID_EAX_BufferProperties, i, &ulSupport);
				if ( (ulSupport&(KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET|KSPROPERTY_SUPPORT_SET) )
				{
//					nlwarning("CSoundDriverDSound::createPropertySet : buffer property %s not supported", bufferProperties[i]);
				}
			}
		}
		else
		{
			nlwarning("CSoundDriverDSound::createPropertySet : propertie set not available !");
		}
	}

	return propertySet;
}

#endif // EAX_AVAILABLE


// ******************************************************************

class CDeviceDescription
{
public:

    static CDeviceDescription* _List;

    CDeviceDescription(LPGUID guid, const char* descr)
    {
        _Guid = guid;
        _Description = strdup(descr);
        _Next = _List;
        _List = this;
    }

    virtual ~CDeviceDescription()
    {
        if (_Description)
        {
            free(_Description);
        }
        if (_Next)
        {
            delete _Next;
        }
    }

    char* _Description;
    CDeviceDescription* _Next;
    LPGUID _Guid;
};

CDeviceDescription* CDeviceDescription::_List = 0;


BOOL CALLBACK CSoundDriverDSoundEnumCallback(LPGUID guid, LPCSTR description, PCSTR /* module */, LPVOID /* context */)
{
    new CDeviceDescription(guid, description);
    return TRUE;
}

// ******************************************************************

CSoundDriverDSound::~CSoundDriverDSound()
{
	nldebug("Destroying DirectSound driver");

    if (_TimerID != NULL)
    {
        timeKillEvent(_TimerID);
        timeEndPeriod(_TimerResolution);
    }


	// Assure that the remaining sources have released all their DSBuffers
	// before closing down DirectSound
	set<CSourceDSound*>::iterator iter;

	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		(*iter)->release();
	}


	// Assure that the listener has released all resources before closing
	// down DirectSound
	if (CListenerDSound::instance() != 0)
	{
		CListenerDSound::instance()->release();
	}


    if (_PrimaryBuffer != NULL)
    {
        _PrimaryBuffer->Release();
        _PrimaryBuffer = NULL;
    }

    if (_DirectSound != NULL)
    {
        _DirectSound->Release();
        _DirectSound = NULL;
    }

	_Instance = 0;

	// free the enumerated list
	if (CDeviceDescription::_List)
	{
		delete CDeviceDescription::_List;
		CDeviceDescription::_List = NULL;
	}
}

/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
void CSoundDriverDSound::getDevices(std::vector<std::string> &devices)
{
	devices.push_back(""); // empty
}

/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
void CSoundDriverDSound::initDevice(const std::string &device, ISoundDriver::TSoundOptions options)
{
	// list of supported options in this driver
	// disable effects if no eax, no buffer streaming
	const sint supportedOptions = 
		OptionAllowADPCM
#if EAX_AVAILABLE
		| OptionEnvironmentEffects
#endif
		| OptionSoftwareBuffer
		| OptionManualRolloff
		| OptionLocalBufferCopy;

	// list of forced options in this driver
	// always have local copy
	const sint forcedOptions = 
		OptionLocalBufferCopy;

	// set the options
	_Options = (TSoundOptions)(((sint)options & supportedOptions) | forcedOptions);

    if (FAILED(DirectSoundEnumerate(CSoundDriverDSoundEnumCallback, this)))
    {
        throw ESoundDriver("Failed to enumerate the DirectSound devices");
    }

    // Create a DirectSound object and set the cooperative level.
#if EAX_AVAILABLE
	if (getOption(OptionEnvironmentEffects))
	{
		if (EAXDirectSoundCreate8(NULL, &_DirectSound, NULL) != DS_OK)
		{
			throw ESoundDriver("Failed to create the DirectSound object from EAX proxy funtion");
		}
	}
	else
#endif
	{
		if (DirectSoundCreate(NULL, &_DirectSound, NULL) != DS_OK)
		{
			throw ESoundDriver("Failed to create the DirectSound object");
		}
	}


    if (_DirectSound->SetCooperativeLevel(CSoundDriverWnd, DSSCL_PRIORITY) != DS_OK)
    {
        throw ESoundDriver("Failed to set the cooperative level");
    }


    // Analyse the capabilities of the sound driver/device

    _Caps.dwSize = sizeof(_Caps);

    if (_DirectSound->GetCaps(&_Caps) != DS_OK)
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }


    // Create primary buffer

    DSBUFFERDESC desc;

    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);


    // First, try to allocate a 3D hardware buffer.
    // If we can't get a 3D hardware buffer, use a 2D hardware buffer.
    // As last option, use a 2D software buffer.

	// check if wa can honor eax request
	if (countHw3DBuffers() > 10)
	{
		_UseEAX = getOption(OptionEnvironmentEffects);
	}
	else
	{
		// not enougth hardware buffer, can't use eax
		_UseEAX = false;
	}

    if (countHw3DBuffers() > 0)
    {
		nldebug("Primary buffer: Allocating 3D buffer in hardware");
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
    }
    else
    {
 		nldebug("Primary buffer: Allocating 3D buffer in software");
        desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME;
//        desc.guid3DAlgorithm = DS3DALG_NO_VIRTUALIZATION;
    }



	HRESULT res = _DirectSound->CreateSoundBuffer(&desc, &_PrimaryBuffer, NULL);

    if (res != DS_OK && res != DS_NO_VIRTUALIZATION)
    {

 		nlwarning("Primary buffer: Failed to create a buffer with 3D capabilities.");

		ZeroMemory(&desc, sizeof(DSBUFFERDESC));
		desc.dwSize = sizeof(DSBUFFERDESC);

		if (countHw2DBuffers() > 0)
		{
			nldebug("Primary buffer: Allocating 2D buffer in hardware");
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCHARDWARE | DSBCAPS_CTRLVOLUME;
		}
		else
		{
 			nldebug("Primary buffer: Allocating 2D buffer in software");
			desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLVOLUME;
		}

		if (_DirectSound->CreateSoundBuffer(&desc, &_PrimaryBuffer, NULL) != DS_OK)
		{
			throw ESoundDriver("Failed to create the primary buffer");
		}
    }


    // Set the format of the primary buffer

    WAVEFORMATEX format;

    format.cbSize = sizeof(WAVEFORMATEX);

    // Make sure the sound card accepts the default settings.
    // For now, only the default settings are accepted. Fallback
    // strategy will be handled later.

    if ((_Caps.dwMinSecondarySampleRate > 22050) && (22050 > _Caps.dwMaxSecondarySampleRate)) {
        throw ESoundDriver("Unsupported sample rate range");
    }

    if ((_Caps.dwFlags & DSCAPS_PRIMARY16BIT) == 0) {
        throw ESoundDriver("Unsupported sample size [16bits]");
    }

    format.wBitsPerSample = 16;
    format.nChannels = 1;
    format.nSamplesPerSec = 22050;
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    format.wFormatTag = WAVE_FORMAT_PCM;


    if (_PrimaryBuffer->SetFormat(&format) != DS_OK)
    {
        throw ESoundDriver("Failed to create set the format of the primary buffer");
    }

	// Fill the buffer with silence
/*	LPVOID ptr;
	DWORD bytes;

	HRESULT hr = _PrimaryBuffer->Lock(0, 0, &ptr, &bytes, NULL, NULL, DSBLOCK_ENTIREBUFFER);
	if (FAILED(hr))
	{
		switch (hr)
		{
		case DSERR_BUFFERLOST:
			throw ESoundDriver("Failed to lock the DirectSound primary buffer : DSERR_BUFFERLOST");
		case DSERR_INVALIDCALL:
			throw ESoundDriver("Failed to lock the DirectSound primary buffer : DSERR_INVALIDCALL");
		case DSERR_INVALIDPARAM:
			throw ESoundDriver("Failed to lock the DirectSound primary buffer : DSERR_INVALIDPARAM");
		case DSERR_PRIOLEVELNEEDED:
			throw ESoundDriver("Failed to lock the DirectSound primary buffer : DSERR_PRIOLEVELNEEDED");
		default:
			throw ESoundDriver("Failed to lock the DirectSound primary buffer : unkown error");

		}
	}

	memset(ptr, 0, bytes);

	_PrimaryBuffer->Unlock(ptr, bytes, 0, 0);
*/

    uint32 numBuffers = countHw3DBuffers();
	if (numBuffers == 0)
	{
		numBuffers = 31;
	}

	/*
    _Sources = new CSourceDSound*[numBuffers];


	for (uint i = 0; i < numBuffers; i++)
	{
		_Sources[i] = 0;
	}

	try
	{
	    for (i = 0; i < numBuffers; i++)
		{
			_Sources[i] = new CSourceDSound(i);
			_Sources[i]->init(_DirectSound);
			_SourceCount++;
		}
	}
	catch (const ESoundDriver& e)
	{
		// Okay, here's the situation: I'm listening to WinAmp while debugging.
		// The caps told me there were 31 buffers available. In reality, there were
		// only 30 available because WinAmp was using one. Somehow DirectSound didn't
		// notice. So when creating buffer 31, an exception was thrown.
		// If something like this happens, don't bother and go on with the buffers we've
		// got. If no buffers are created at all, throw the exception again.

		if (_Sources == 0)
		{
			throw e;
		}
	}

*/
	
    TIMECAPS tcaps;

    timeGetDevCaps(&tcaps, sizeof(TIMECAPS));
    _TimerResolution = (tcaps.wPeriodMin > 10)? tcaps.wPeriodMin : 10;
    timeBeginPeriod(_TimerResolution);
	
#if NLSOUND_PROFILE
    for (uint i = 0; i < 1024; i++)
    {
        _TimerInterval[i] = 0;
    }

    _TimerDate = CTime::getPerformanceTime();
#endif
	
    _TimerID = timeSetEvent(_TimerPeriod, 0, &CSoundDriverDSound::TimerCallback, (DWORD_PTR)this, TIME_CALLBACK_FUNCTION | TIME_PERIODIC);

    if (_TimerID == NULL)
    {
        throw ESoundDriver("Failed to create the timer");
    }
}

/// Return options that are enabled (including those that cannot be disabled on this driver).
ISoundDriver::TSoundOptions CSoundDriverDSound::getOptions()
{
	return _Options;
}

/// Return if an option is enabled (including those that cannot be disabled on this driver).
bool CSoundDriverDSound::getOption(ISoundDriver::TSoundOptions option)
{
	return ((uint)_Options & (uint)option) == (uint)option;
}

// ******************************************************************

uint CSoundDriverDSound::countMaxSources()
{
	// Try the hardware 3d buffers first
	uint n = countHw3DBuffers();
	if (n > 0)
	{
		return n;
	}

	// If not, try the hardware 2d buffers first
	n = countHw2DBuffers();
	if (n > 0)
	{
		return n;
	}

	// Okay, we'll use 32 software buffers
	return 32;
}

// ******************************************************************

void CSoundDriverDSound::writeProfile(string& out)
{
    // Write the available sound devices
    CDeviceDescription* list = CDeviceDescription::_List;
    while (list) {
		out += "\t" + string(list->_Description) + "\n";
        list = list->_Next;
    }

    // Write the buffers sizes
    out += "\tBuffer size: " + toString ((int)CSourceDSound::_SecondaryBufferSize) + "\n";
    out += "\tCopy size: " + toString ((int)CSourceDSound::_UpdateCopySize) + "\n";
    out += "\tSwap size: " + toString ((int)CSourceDSound::_SwapCopySize) + "\n";

    // Write the number of hardware buffers
    DSCAPS caps;
    caps.dwSize = sizeof(caps);
    _DirectSound->GetCaps(&caps);

    out += "\t3d hw buffers: " + toString ((uint32)caps.dwMaxHw3DAllBuffers) + "\n";
	out += "\t2d hw buffers: " + toString ((uint32)caps.dwMaxHwMixingAllBuffers) + "\n";

    // Write the number of hardware buffers
#if NLSOUND_PROFILE
    out += "\tUpdate time total --- " + toString (getAverageUpdateTime()) + "\n";
	out += "\tUpdate time source --- " + toString (CSourceDSound::getAverageUpdateTime()) + "\n";
	out += "\tUpdate --- t: " + toString (CSourceDSound::getAverageCumulTime());
	out += "\t - p: " + toString (CSourceDSound::getAveragePosTime());
	out += "\t - l: " + toString (CSourceDSound::getAverageLockTime());
	out += "\t - c: " + toString (CSourceDSound::getAverageCopyTime());
	out += "\t - u: " + toString (CSourceDSound::getAverageUnlockTime()) + "\n";
	out += "\tUpdate percentage: --- " + toString (getUpdatePercentage()) + "\n";
	out += "\tUpdate num sources --- " + toString ((int)getAverageUpdateSources()) + "\n";
	out += "\tUpdate byte size --- " + toString (CSourceDSound::getAverageUpdateSize()) + "\n";
	out += "\tSwap time --- " + toString (CSourceDSound::getTestAverage()) + "\n";
	out += "\tSrc --- " + toString (countPlayingSources()) + "\n";
#endif
}


void CALLBACK CSoundDriverDSound::TimerCallback(UINT /* uID */, UINT /* uMsg */, DWORD_PTR dwUser, DWORD_PTR /* dw1 */, DWORD_PTR /* dw2 */)
{
	// a little speed check
	static NLMISC::TTime lastUpdate = NLMISC::CTime::getLocalTime();
	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	if (now - lastUpdate > _TimerPeriod * 2)
	{
//		nlwarning("CSoundDriverDSound::TimerCallback : no update since %u millisec (nominal update = %u", uint32(now-lastUpdate), uint32(_TimerPeriod));
	}
	else
	{
//		nldebug("Callback delay = %u ms", uint32(now-lastUpdate));
	}

	lastUpdate = now;


    CSoundDriverDSound* driver = (CSoundDriverDSound*) dwUser;
    driver->update();
}

// ******************************************************************

void CSoundDriverDSound::update()
{
	H_AUTO(NLSOUND_DSoundUpdate)
#if NLSOUND_PROFILE
    TTicks tnow = CTime::getPerformanceTime();
#endif

	NLMISC::TTime now = NLMISC::CTime::getLocalTime();

	set<CSourceDSound*>::iterator first(_Sources.begin()), last(_Sources.end());
	for (;first != last; ++first)
	{
		if ((*first)->needsUpdate())
		{
			if ((*first)->update())
			{
#if NLSOUND_PROFILE
				_UpdateSources++;
#endif
			}
		}
	}

/*
	set<CSourceDSound*>::iterator iter;

	iter = _Sources.begin();


	if ((iter != _Sources.end()) && (*iter)->needsUpdate())
	{
		while (iter != _Sources.end())
		{
//			if ((*iter)->update2()) {
			if ((*iter)->update())
			{
#if NLSOUND_PROFILE
				_UpdateSources++;
#endif
			}
			iter++;
		}
	}

*/
	{
		NLMISC::TTime	last = CTime::getLocalTime() - now;
		if (last > _TimerPeriod / 2)
		{
			nlwarning("CSoundDriverDSound::TimerCallback : update took %u millisec", (uint32)last);
		}
	}

#if NLSOUND_PROFILE
    _TotalUpdateTime += 1000.0 * CTime::ticksToSecond(CTime::getPerformanceTime() - tnow);
	_UpdateCount++;
#endif
}

// ******************************************************************

uint CSoundDriverDSound::countHw3DBuffers()
{
    DSCAPS caps;
    caps.dwSize = sizeof(caps);

    if (_DirectSound->GetCaps(&caps) != DS_OK)
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }

    return caps.dwFreeHw3DStreamingBuffers;
}

// ******************************************************************

uint CSoundDriverDSound::countHw2DBuffers()
{
    DSCAPS caps;
    caps.dwSize = sizeof(caps);

    if (_DirectSound->GetCaps(&caps) != DS_OK)
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }

    return caps.dwFreeHwMixingStreamingBuffers;
}

// ******************************************************************

IListener *CSoundDriverDSound::createListener()
{
    LPDIRECTSOUND3DLISTENER dsoundListener;

    if (CListenerDSound::instance() != NULL)
    {
        return CListenerDSound::instance();
    }

    if (_PrimaryBuffer == 0)
    {
        throw ESoundDriver("Corrupt driver");
    }

    if (FAILED(_PrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *) &dsoundListener)))
    {
		nlwarning("The 3D listener interface is not available.");
        return new CListenerDSound(NULL);
    }

    return new CListenerDSound(dsoundListener);
}

// ******************************************************************

IBuffer *CSoundDriverDSound::createBuffer()
{
    if (_PrimaryBuffer == 0)
    {
        throw ESoundDriver("Corrupt driver");
    }


    // FIXME: set buffer ID
    return new CBufferDSound();
}

// ******************************************************************

void CSoundDriverDSound::removeBuffer(IBuffer * /* buffer */)
{
}

//// ******************************************************************
//bool CSoundDriverDSound::readWavBuffer( IBuffer *destbuffer, const std::string &name, uint8 *wavData, uint dataSize)
//{
//	return ((CBufferDSound*) destbuffer)->readWavBuffer(name, wavData, dataSize);
//}
//
//bool CSoundDriverDSound::readRawBuffer( IBuffer *destbuffer, const std::string &name, uint8 *rawData, uint dataSize, TSampleFormat format, uint32 frequency)
//{
//	return ((CBufferDSound*) destbuffer)->readRawBuffer(name, rawData, dataSize, format, frequency);
//}


// ******************************************************************

ISource *CSoundDriverDSound::createSource()
{
    if (_PrimaryBuffer == 0)
    {
        throw ESoundDriver("Corrupt driver");
    }


	CSourceDSound* src = new CSourceDSound(0);
	src->init(_DirectSound, _UseEAX);
	_Sources.insert(src);

	return src;
}


// ******************************************************************

void CSoundDriverDSound::removeSource(ISource *source)
{
	_Sources.erase((CSourceDSound*) source);
}

// ******************************************************************

void CSoundDriverDSound::commit3DChanges()
{
	CListenerDSound* listener = CListenerDSound::instance();
	listener->commit3DChanges();


	const CVector &origin = listener->getPos();

	set<CSourceDSound*>::iterator iter;

	// We handle the volume of the source according to the distance
	// ourselves. Call updateVolume() to, well..., update the volume
	// according to, euh ..., the new distance!
	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		if ((*iter)->isPlaying())
		{
			(*iter)->updateVolume(origin);
		}
	}
}


// ******************************************************************

uint CSoundDriverDSound::countPlayingSources()
{
    uint n = 0;
	set<CSourceDSound*>::iterator iter;

	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		if ((*iter)->isPlaying())
		{
			n++;
		}
	}

    return n;
}


// ******************************************************************

void CSoundDriverDSound::setGain( float gain )
{
	if (_PrimaryBuffer != 0)
	{
		if (gain < 0.00001f)
		{
			gain = 0.00001f;
		}

		/* convert from linear amplitude to hundredths of decibels */
		LONG volume = (LONG)(100.0 * 20.0 * log10(gain));

		if (volume < DSBVOLUME_MIN)
		{
			volume = DSBVOLUME_MIN;
		}
		else if (volume > DSBVOLUME_MAX)
		{
			volume = DSBVOLUME_MAX;
		}

		HRESULT hr = _PrimaryBuffer->SetVolume(volume);

		if (hr != DS_OK)
		{
			nldebug("Failed to set the volume");
		}
	}
}

// ******************************************************************

float CSoundDriverDSound::getGain()
{
	if (_PrimaryBuffer != 0)
	{
		/* convert from hundredths of decibels to linear amplitude */
		LONG volume;
		HRESULT hr = _PrimaryBuffer->GetVolume(&volume);

		if (hr != DS_OK)
		{
			nldebug("Failed to get the volume");
			return 1.0;
		}

		return (float) pow((double)10, (double) volume / 20.0 / 100.0);
	}

	return 1.0;
}



#if NLSOUND_PROFILE

// ******************************************************************

uint CSoundDriverDSound::countTimerIntervals()
{
    return 1024;
}

// ******************************************************************

uint CSoundDriverDSound::getTimerIntervals(uint index)
{
    return _TimerInterval[index];
}

// ******************************************************************

void CSoundDriverDSound::addTimerInterval(uint32 dt)
{
    if (_TimerIntervalCount >= 1024)
    {
        _TimerIntervalCount = 0;
    }

    _TimerInterval[_TimerIntervalCount++] = dt;
}

// ******************************************************************

double CSoundDriverDSound::getCPULoad()
{
    return (_TotalTime > 0.0)? 100.0 * _TotalUpdateTime / _TotalTime : 0.0;
}

// ******************************************************************

void CSoundDriverDSound::printDriverInfo(FILE* fp)
{
    CDeviceDescription* list = CDeviceDescription::_List;

    while (list) {
        fprintf(fp, "%s\n", list->_Description);
        list = list->_Next;
    }

    fprintf(fp, "\n");

    fprintf(fp, "buffer size: %d\n"
				"copy size: %d\n"
				"swap size: %d\n",
			CSourceDSound::_SecondaryBufferSize,
			CSourceDSound::_UpdateCopySize,
			CSourceDSound::_SwapCopySize);

    fprintf(fp, "\n");

    DSCAPS caps;
    caps.dwSize = sizeof(caps);

    if (_DirectSound->GetCaps(&caps) != DS_OK)
    {
        throw ESoundDriver("Failed to query the sound device caps");
    }


    fprintf(fp, "3d hw buffers: %d\n" "2d hw buffers: %d\n\n", caps.dwMaxHw3DAllBuffers, caps.dwMaxHwMixingAllBuffers);
}

#endif


void	CSoundDriverDSound::startBench()
{
	NLMISC::CHTimer::startBench();
}
void	CSoundDriverDSound::endBench()
{
	NLMISC::CHTimer::endBench();
}
void	CSoundDriverDSound::displayBench(CLog *log)
{
		NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
		NLMISC::CHTimer::displayHierarchical(log, true, 48, 2);
		NLMISC::CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
		NLMISC::CHTimer::display(log, CHTimer::TotalTime);
}

} // NLSOUND
