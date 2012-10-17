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
#include "listener_xaudio2.h"
#include "source_xaudio2.h"
#include "effect_xaudio2.h"
#include "sound_driver_xaudio2.h"

using namespace std;
using namespace NLMISC;

/// Sample rate for master voice (device).
/// Default (0) under windows is 44100 (CD) or 48000 (DVD). 
/// Note 1: OpenAL driver uses 22050 at the moment.
/// Note 2: 44100 seems to be the optimal value here.
//#define NLSOUND_XAUDIO2_MASTER_SAMPLE_RATE 44100
#define NLSOUND_XAUDIO2_MASTER_SAMPLE_RATE XAUDIO2_DEFAULT_SAMPLERATE

namespace NLSOUND {

// ******************************************************************

#ifndef NL_STATIC
class CSoundDriverXAudio2NelLibrary : public NLMISC::INelLibrary { 
	void onLibraryLoaded(bool /* firstTime */) { } 
	void onLibraryUnloaded(bool /* lastTime */) { }  
};
NLMISC_DECL_PURE_LIB(CSoundDriverXAudio2NelLibrary)

HINSTANCE CSoundDriverXAudio2DllHandle = NULL;
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
	CSoundDriverXAudio2DllHandle = (HINSTANCE)hModule;
	return TRUE;
}
#endif /* #ifndef NL_STATIC */

// ***************************************************************************

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstanceXAudio2
#else
__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance
#endif
	(ISoundDriver::IStringMapperProvider *stringMapper)
{
	return new CSoundDriverXAudio2(stringMapper);
}

// ******************************************************************

#ifdef NL_STATIC
uint32 interfaceVersionXAudio2()
#else
__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
#endif
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

#ifdef NL_STATIC
void outputProfileXAudio2
#else
__declspec(dllexport) void NLSOUND_outputProfile
#endif
	(string &out)
{
	CSoundDriverXAudio2::getInstance()->writeProfile(out);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver::TDriver getDriverTypeXAudio2()
#else
__declspec(dllexport) ISoundDriver::TDriver NLSOUND_getDriverType()
#endif
{
	return ISoundDriver::DriverXAudio2;
}

// ******************************************************************

#ifdef NL_DEBUG

static XAUDIO2_DEBUG_CONFIGURATION NLSOUND_XAUDIO2_DEBUG_CONFIGURATION_DISABLED = {
  0, 0, true, true, true, true
};

NLMISC_CATEGORISED_COMMAND(nel, xa2DebugDisable, "", "")
{
	human; quiet; log; args; rawCommandString;
	CSoundDriverXAudio2::getInstance()->getXAudio2()->SetDebugConfiguration(&NLSOUND_XAUDIO2_DEBUG_CONFIGURATION_DISABLED);
	return true;
}

static XAUDIO2_DEBUG_CONFIGURATION NLSOUND_XAUDIO2_DEBUG_CONFIGURATION_HEAVY = {
	(UINT32)(~XAUDIO2_LOG_FUNC_CALLS & ~XAUDIO2_LOG_LOCKS & ~XAUDIO2_LOG_MEMORY), 0, true, true, true, true
};

NLMISC_CATEGORISED_COMMAND(nel, xa2DebugHeavy, "", "")
{
	human; quiet; log; args; rawCommandString;
	CSoundDriverXAudio2::getInstance()->getXAudio2()->SetDebugConfiguration(&NLSOUND_XAUDIO2_DEBUG_CONFIGURATION_HEAVY);
	return true;
}

#endif /* NL_DEBUG */

// ******************************************************************

CSoundDriverXAudio2::CSoundDriverXAudio2(ISoundDriver::IStringMapperProvider * /* stringMapper */) 
	: _XAudio2(NULL), _MasteringVoice(NULL), _Listener(NULL), 
	_SoundDriverOk(false), _CoInitOk(false), _OperationSetCounter(65536), 
	_PerformanceCommit3DCounter(0), _PerformanceADPCMBufferSize(0), 
	_PerformancePCMBufferSize(0), _PerformanceSourcePlayCounter(0)
{
	nlwarning(NLSOUND_XAUDIO2_PREFIX "Creating CSoundDriverXAudio2");

	memset(&_X3DAudioHandle, 0, sizeof(_X3DAudioHandle));
	memset(&_EmptyListener, 0, sizeof(_EmptyListener));

	_EmptyListener.OrientFront.x = 0.0f;
	_EmptyListener.OrientFront.y = 0.0f;
	_EmptyListener.OrientFront.z = 1.0f;
	_EmptyListener.OrientTop.x = 0.0f;
	_EmptyListener.OrientTop.y = 1.0f;
	_EmptyListener.OrientTop.z = 0.0f;
	_EmptyListener.Position.x = 0.0f;
	_EmptyListener.Position.y = 0.0f;
	_EmptyListener.Position.z = 0.0f;
	_EmptyListener.Velocity.x = 0.0f;
	_EmptyListener.Velocity.y = 0.0f;
	_EmptyListener.Velocity.z = 0.0f;	
	
	HRESULT hr;

	// Windows
#ifdef NL_OS_WINDOWS // CoInitializeEx not on xbox, lol
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (hr == RPC_E_CHANGED_MODE) { nlwarning(NLSOUND_XAUDIO2_PREFIX "CoInitializeEx COINIT_APARTMENTTHREADED"); hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); }
	_CoInitOk = (hr == S_OK) || (hr == S_FALSE);
	if (!_CoInitOk) { release(); throw ESoundDriver(NLSOUND_XAUDIO2_PREFIX "FAILED CoInitializeEx"); return; }
#endif

	uint32 flags = 0;
#ifdef NL_DEBUG
	flags |= XAUDIO2_DEBUG_ENGINE; // comment when done using this :)
#endif

	// XAudio2
	if (FAILED(hr = XAudio2Create(&_XAudio2, flags, XAUDIO2_DEFAULT_PROCESSOR)))
	{ release(); throw ESoundDriver(NLSOUND_XAUDIO2_PREFIX "XAudio2 failed to initialize. Please install the latest version of the DirectX End-User Runtimes."); return; }
}

CSoundDriverXAudio2::~CSoundDriverXAudio2()
{
	release();
	
	// Windows
#ifdef NL_OS_WINDOWS
	if (_CoInitOk) CoUninitialize();
	_CoInitOk = false;
#else
	nlassert(!_CoInitOk);
#endif
	
	nlinfo(NLSOUND_XAUDIO2_PREFIX "Destroying CSoundDriverXAudio2");
}

#define NLSOUND_XAUDIO2_RELEASE(pointer) if (_SoundDriverOk) nlassert(pointer); \
	/*if (pointer) {*/ delete pointer; pointer = NULL; /*}*/
#define NLSOUND_XAUDIO2_RELEASE_EX(pointer, command) if (_SoundDriverOk) nlassert(pointer); \
	if (pointer) { command; pointer = NULL; }

void CSoundDriverXAudio2::release()
{
	nlinfo(NLSOUND_XAUDIO2_PREFIX "Releasing CSoundDriverXAudio2");

	// WARNING: Only internal resources are released here, 
	// the created instances must still be released by the user!

	// Release internal resources of all remaining ISource instances
	if (_Sources.size())
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "_Sources.size(): '%u'", (uint32)_Sources.size());
		set<CSourceXAudio2 *>::iterator it(_Sources.begin()), end(_Sources.end());
		for (; it != end; ++it) (*it)->release();
		_Sources.clear();
	}
	// Release internal resources of all remaining IBuffer instances
	if (_Buffers.size())
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "_Buffers.size(): '%u'", (uint32)_Buffers.size());
		set<CBufferXAudio2 *>::iterator it(_Buffers.begin()), end(_Buffers.end());
		for (; it != end; ++it) (*it)->release();
		_Buffers.clear();
	}
	// Release internal resources of all remaining IEffect instances
	if (_Effects.size())
	{
		nlwarning(NLSOUND_XAUDIO2_PREFIX "_Effects.size(): '%u'", (uint32)_Effects.size());
		set<CEffectXAudio2 *>::iterator it(_Effects.begin()), end(_Effects.end());
		for (; it != end; ++it) (*it)->release();
		_Effects.clear();
	}
	// Release internal resources of the IListener instance
	if (_Listener) { nlwarning(NLSOUND_XAUDIO2_PREFIX "_Listener: !NULL"); _Listener->release(); _Listener = NULL; }
	
	// XAudio2
	NLSOUND_XAUDIO2_RELEASE_EX(_MasteringVoice, _MasteringVoice->DestroyVoice());	
	NLSOUND_XAUDIO2_RELEASE_EX(_XAudio2, _XAudio2->Release());
	_SoundDriverOk = false;
}

/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
void CSoundDriverXAudio2::getDevices(std::vector<std::string> &devices)
{
	devices.push_back(""); // empty

	UINT32 deviceCount;
	_XAudio2->GetDeviceCount(&deviceCount);

	if (deviceCount == 0)
	{
		nldebug("XA2: No audio devices");
	}
	else
	{
		XAUDIO2_DEVICE_DETAILS deviceDetails;
		nldebug("XA2: Listing devices: ");
		for (uint i = 0; i < deviceCount; ++i)
		{
			_XAudio2->GetDeviceDetails(i, &deviceDetails);
			std::basic_string<WCHAR> deviceNameW = deviceDetails.DisplayName;
			std::string deviceName = std::string(deviceNameW.begin(), deviceNameW.end());
			nldebug("XA2:   - %s", deviceName.c_str());
			devices.push_back(deviceName);
		}
	}
}

/// (Internal) Get device index and details from string.
uint CSoundDriverXAudio2::getDeviceIndex(const std::string &device, XAUDIO2_DEVICE_DETAILS *deviceDetails)
{
	if (device.empty())
	{
		_XAudio2->GetDeviceDetails(0, deviceDetails);
		return 0;
	}
	
	UINT32 deviceCount;
	_XAudio2->GetDeviceCount(&deviceCount);

	for (uint i = 0; i < deviceCount; ++i)
	{
		_XAudio2->GetDeviceDetails(i, deviceDetails);
		std::basic_string<WCHAR> deviceNameW = deviceDetails->DisplayName;
		std::string deviceName = std::string(deviceNameW.begin(), deviceNameW.end());
		if (deviceName == device)
			return i;
	}

	nldebug("XA2: Device '%s' not found", device.c_str());
	return 0;
}

/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
void CSoundDriverXAudio2::initDevice(const std::string &device, TSoundOptions options)
{
	nlinfo(NLSOUND_XAUDIO2_PREFIX "Initializing CSoundDriverXAudio2");

	// list of supported options in this driver
	const sint supportedOptions = 
		OptionEnvironmentEffects
		| OptionAllowADPCM
		| OptionSoftwareBuffer
		| OptionManualRolloff
		| OptionLocalBufferCopy
		| OptionHasBufferStreaming;

	// list of forced options in this driver
	// always use software buffer, always have local copy
	const sint forcedOptions = 
		OptionSoftwareBuffer
		| OptionManualRolloff
		| OptionLocalBufferCopy;

	// set the options
	_Options = (TSoundOptions)(((sint)options & supportedOptions) | forcedOptions);

#ifdef NL_OS_WINDOWS // CoInitializeEx not on xbox, lol
	if (!_CoInitOk) { throw ESoundDriver(NLSOUND_XAUDIO2_PREFIX "FAILED CoInitializeEx"); return; }
#endif

	HRESULT hr;

	// XAudio2
	XAUDIO2_DEVICE_DETAILS deviceDetails;
	uint deviceIndex = getDeviceIndex(device, &deviceDetails);
	if (FAILED(hr = _XAudio2->CreateMasteringVoice(&_MasteringVoice, 0, NLSOUND_XAUDIO2_MASTER_SAMPLE_RATE, 0, deviceIndex, NULL)))
		{ release(); throw ESoundDriver(NLSOUND_XAUDIO2_PREFIX "FAILED CreateMasteringVoice _MasteringVoice!"); return; }
	
	// X3DAudio
	// speed of sound in meters per second for dry air at approximately 20C, used with X3DAudioInitialize
	// #define X3DAUDIO_SPEED_OF_SOUND 343.5f
	X3DAudioInitialize(deviceDetails.OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, _X3DAudioHandle);
	
	_SoundDriverOk = true;
}

/// Return options that are enabled (including those that cannot be disabled on this driver).
ISoundDriver::TSoundOptions CSoundDriverXAudio2::getOptions()
{
	return _Options;
}

/// Return if an option is enabled (including those that cannot be disabled on this driver).
bool CSoundDriverXAudio2::getOption(ISoundDriver::TSoundOptions option)
{
	return ((uint)_Options & (uint)option) == (uint)option;
}

/// Tell sources without voice about a format
void CSoundDriverXAudio2::initSourcesFormat(IBuffer::TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample)
{
	std::set<CSourceXAudio2 *>::iterator it(_Sources.begin()), end(_Sources.end());
	for (; it != end; ++it) { if (!(*it)->getSourceVoice()) { (*it)->initFormat(bufferFormat, channels, bitsPerSample); } }
}

/// (Internal) Create an XAudio2 source voice of the specified format.
IXAudio2SourceVoice *CSoundDriverXAudio2::createSourceVoice(IBuffer::TBufferFormat bufferFormat, uint8 channels, uint8 bitsPerSample, IXAudio2VoiceCallback *callback)
{
	nlassert(_Listener);

	HRESULT hr;

	WAVEFORMATEX wfe;
	wfe.cbSize = 0;

	if (bufferFormat == IBuffer::FormatDviAdpcm)
		nlassert(channels == 1 && bitsPerSample == 16);

	wfe.wFormatTag = WAVE_FORMAT_PCM; // DVI_ADPCM is converted in the driver

	wfe.nChannels = channels;
	wfe.wBitsPerSample = bitsPerSample;

	XAUDIO2_VOICE_DETAILS voice_details;
	_Listener->getDryVoice()->GetVoiceDetails(&voice_details);
	wfe.nSamplesPerSec = voice_details.InputSampleRate;

	wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
	wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;
	
	// TODO: Set callback (in CSourceXAudio2 maybe) for when error happens on voice, so we can restart it!
	IXAudio2SourceVoice *source_voice = NULL;

	if (FAILED(hr = _XAudio2->CreateSourceVoice(&source_voice, &wfe, 0, NLSOUND_MAX_PITCH, callback, NULL, NULL)))
	{ if (source_voice) source_voice->DestroyVoice(); nlerror(NLSOUND_XAUDIO2_PREFIX "FAILED CreateSourceVoice"); return NULL; }

	return source_voice;
}

/// (Internal) Destroy an XAudio2 source voice.
void CSoundDriverXAudio2::destroySourceVoice(IXAudio2SourceVoice *sourceVoice)
{
	if (sourceVoice) sourceVoice->DestroyVoice();
}

/// Create the listener instance
IListener *CSoundDriverXAudio2::createListener()
{
	if (!_Listener) _Listener = new CListenerXAudio2(this);
	return static_cast<IListener *>(_Listener);
}

/// Create a source, destroy with delete
ISource *CSoundDriverXAudio2::createSource()
{
	CSourceXAudio2 *source = new CSourceXAudio2(this);
	_Sources.insert(source);
	return static_cast<ISource *>(source);
}

/// Create a sound buffer, destroy with delete
IBuffer *CSoundDriverXAudio2::createBuffer()
{
	CBufferXAudio2 *buffer = new CBufferXAudio2(this);
	_Buffers.insert(buffer);
	return static_cast<IBuffer *>(buffer);
}

/// Create a reverb effect
IReverbEffect *CSoundDriverXAudio2::createReverbEffect()
{
	CReverbEffectXAudio2 *reverb = new CReverbEffectXAudio2(this);
	if (reverb->getEffect())
	{
		_Effects.insert(reverb);
		return static_cast<IReverbEffect *>(reverb);
	}
	else 
	{ 
		delete reverb;
		return NULL; 
	}
}

/// Return the maximum number of sources that can created
uint CSoundDriverXAudio2::countMaxSources()
{
	// the only limit is the user's cpu
	// keep similar to openal limit for now ...
	return 128;
}

/// Return the maximum number of effects that can be created
uint CSoundDriverXAudio2::countMaxEffects()
{
	// the only limit is the user's cpu
	// openal only allows 1 in software ...
	return 32;
}

/// Commit all the changes made to 3D settings of listener and sources
void CSoundDriverXAudio2::commit3DChanges()
{
	performanceIncreaseCommit3DCounter();

	// Sync up sources & listener 3d position.
	{
		std::set<CSourceXAudio2 *>::iterator it(_Sources.begin()), end(_Sources.end());
		for (; it != end; ++it) { (*it)->updateState(); (*it)->update3DChanges(); }
	}
}

/// Write information about the driver to the output stream.
void CSoundDriverXAudio2::writeProfile(std::string& out)
{
	XAUDIO2_PERFORMANCE_DATA performance;
	_XAudio2->GetPerformanceData(&performance);

	out = toString(NLSOUND_XAUDIO2_NAME)
		+ "\n\tPCMBufferSize: " + toString(_PerformancePCMBufferSize)
		+ "\n\tADPCMBufferSize: " + toString(_PerformanceADPCMBufferSize)
		+ "\n\tSourcePlayCounter: " + toString(_PerformanceSourcePlayCounter)
		+ "\n\tCommit3DCounter: " + toString(_PerformanceCommit3DCounter)
		+ "\nXAUDIO2_PERFORMANCE_DATA"
		+ "\n\tAudioCyclesSinceLastQuery: " + toString(performance.AudioCyclesSinceLastQuery)
		+ "\n\tTotalCyclesSinceLastQuery: " + toString(performance.TotalCyclesSinceLastQuery)
		+ "\n\tMinimumCyclesPerQuantum: " + toString(performance.MinimumCyclesPerQuantum)
		+ "\n\tMaximumCyclesPerQuantum: " + toString(performance.MaximumCyclesPerQuantum)
		+ "\n\tMemoryUsageInBytes: " + toString(performance.MemoryUsageInBytes)
		+ "\n\tCurrentLatencyInSamples: " + toString(performance.CurrentLatencyInSamples)
		+ "\n\tGlitchesSinceEngineStarted: " + toString(performance.GlitchesSinceEngineStarted)
		+ "\n\tActiveSourceVoiceCount: " + toString(performance.ActiveSourceVoiceCount)
		+ "\n\tTotalSourceVoiceCount: " + toString(performance.TotalSourceVoiceCount)
		+ "\n\tActiveSubmixVoiceCount: " + toString(performance.ActiveSubmixVoiceCount)
		+ "\n\tActiveXmaSourceVoices: " + toString(performance.ActiveXmaSourceVoices)
		+ "\n\tActiveXmaStreams: " + toString(performance.ActiveXmaStreams)
		+ "\n";
	return;
}

// Does not create a sound loader .. what does it do then?
void CSoundDriverXAudio2::startBench()
{
	NLMISC::CHTimer::startBench();
}

void CSoundDriverXAudio2::endBench()
{
	NLMISC::CHTimer::endBench();
}

void CSoundDriverXAudio2::displayBench(NLMISC::CLog *log)
{
	NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	NLMISC::CHTimer::displayHierarchical(log, true, 48, 2);
	NLMISC::CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	NLMISC::CHTimer::display(log, CHTimer::TotalTime);
}

/// Remove a buffer (should be called by the friend destructor of the buffer class)
void CSoundDriverXAudio2::removeBuffer(CBufferXAudio2 *buffer)
{
	if (_Buffers.find(buffer) != _Buffers.end()) _Buffers.erase(buffer);
	else nlwarning("removeBuffer already called");
}

/// Remove a source (should be called by the friend destructor of the source class)
void CSoundDriverXAudio2::removeSource(CSourceXAudio2 *source)
{
	if (_Sources.find(source) != _Sources.end()) _Sources.erase(source);
	else nlwarning("removeSource already called");
}

/// (Internal) Remove an effect (should be called by the destructor of the effect class)
void CSoundDriverXAudio2::removeEffect(CEffectXAudio2 *effect)
{
	if (_Effects.find(effect) != _Effects.end()) _Effects.erase(effect);
	else nlwarning("removeEffect already called");
}

} /* namespace NLSOUND */

/* end of file */
