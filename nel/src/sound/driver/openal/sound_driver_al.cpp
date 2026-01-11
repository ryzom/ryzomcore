// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012-2014  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "sound_driver_al.h"
#include "buffer_al.h"
#include "listener_al.h"
#include "source_al.h"
#include "ext_al.h"
#include "effect_al.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

// Currently, the OpenAL headers are different between Windows and Linux versions !
// AL_INVALID_XXXX are part of the spec, though.
/*#ifdef NL_OS_UNIX
#define AL_INVALID_ENUM AL_ILLEGAL_ENUM
#define AL_INVALID_OPERATION AL_ILLEGAL_COMMAND
#endif*/


#ifdef NL_DEBUG
// Test error in debug mode
void alTestError()
{
	ALuint errcode = alGetError();
	switch (errcode)
	{
	case AL_NO_ERROR : break;
	case AL_INVALID_NAME: nlerror("OpenAL: Invalid name");
	case AL_INVALID_ENUM: nlerror("OpenAL: Invalid enum");
	case AL_INVALID_VALUE: nlerror("OpenAL: Invalid value");
	case AL_INVALID_OPERATION: nlerror("OpenAL: Invalid operation");
	case AL_OUT_OF_MEMORY: nlerror("OpenAL: Out of memory");
	default: nlerror("OpenAL: Unknown error %x", errcode);
	}
}
#endif

#if !FINAL_VERSION
void alTestWarning(const char *src)
{
	ALuint errcode = alGetError();
	switch (errcode)
	{
	case AL_NO_ERROR: break;
	case AL_INVALID_NAME: nlwarning("AL: Invalid Name parameter passed to AL call (%s)", src); break;
	case AL_INVALID_ENUM: nlwarning("AL: Invalid parameter passed to AL call (%s)", src); break;
	case AL_INVALID_VALUE: nlwarning("AL: Invalid enum parameter value (%s)", src); break;
	case AL_INVALID_OPERATION: nlwarning("AL: Illegal call (%s)", src); break;
	case AL_OUT_OF_MEMORY: nlerror("AL: Out of memory (%s)", src); break;
	}
}
#endif

#define INITIAL_BUFFERS 8
#define INITIAL_SOURCES 8
#define BUFFER_ALLOC_RATE 8
#define SOURCE_ALLOC_RATE 8

#define ROLLOFF_FACTOR_DEFAULT 1.0f

#ifndef NL_STATIC

class CSoundDriverALNelLibrary : public NLMISC::INelLibrary
{
	void onLibraryLoaded(bool /* firstTime */) { }
	void onLibraryUnloaded(bool /* lastTime */) { }
};
NLMISC_DECL_PURE_LIB(CSoundDriverALNelLibrary)

#endif /* #ifndef NL_STATIC */

/*
 * Sound driver instance creation
 */
#ifdef NL_OS_WINDOWS
#ifdef NL_COMP_MINGW
#ifndef NL_STATIC
extern "C"
{
#endif
#endif
// ******************************************************************

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstanceOpenAl
#else
__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance
#endif
	(ISoundDriver::IStringMapperProvider *stringMapper)
{
	return new CSoundDriverAL(stringMapper);
}

// ******************************************************************

#ifdef NL_STATIC
uint32 interfaceVersionOpenAl()
#else
__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
#endif
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

#ifdef NL_STATIC
void outputProfileOpenAl
#else
__declspec(dllexport) void NLSOUND_outputProfile
#endif
	(string &out)
{
	CSoundDriverAL::getInstance()->writeProfile(out);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver::TDriver getDriverTypeOpenAl()
#else
__declspec(dllexport) ISoundDriver::TDriver NLSOUND_getDriverType()
#endif
{
	return ISoundDriver::DriverOpenAl;
}

// ******************************************************************
#ifdef NL_COMP_MINGW
#ifndef NL_STATIC
}
#endif
#endif
#elif defined (NL_OS_UNIX)

#ifndef NL_STATIC
extern "C"
{
#endif

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstanceOpenAl(ISoundDriver::IStringMapperProvider *stringMapper)
#else
ISoundDriver* NLSOUND_createISoundDriverInstance(ISoundDriver::IStringMapperProvider *stringMapper)
#endif
{
	return new CSoundDriverAL(stringMapper);
}

uint32 NLSOUND_interfaceVersion ()
{
	return ISoundDriver::InterfaceVersion;
}

#ifndef NL_STATIC
}
#endif

#endif // NL_OS_UNIX

/*
 * Constructor
 */
CSoundDriverAL::CSoundDriverAL(ISoundDriver::IStringMapperProvider *stringMapper) 
: _StringMapper(stringMapper), _AlDevice(NULL), _AlContext(NULL), 
_NbExpBuffers(0), _NbExpSources(0), _RolloffFactor(1.f)
{
	alExtInit();
}

/*
 * Destructor
 */
CSoundDriverAL::~CSoundDriverAL()
{
	// WARNING: Only internal resources are released here, 
	// the created instances must still be released by the user!
	
	// Remove the allocated (but not exported) source and buffer names-
	// Release internal resources of all remaining ISource instances
	if (!_Sources.empty())
	{
		nlwarning("AL: _Sources.size(): '%u'", (uint32)_Sources.size());
		set<CSourceAL *>::iterator it(_Sources.begin()), end(_Sources.end());
		for (; it != end; ++it) (*it)->release(); // CSourceAL will be deleted by user
		_Sources.clear();
	}
	if (!_Buffers.empty()) alDeleteBuffers(compactAliveNames(_Buffers, alIsBuffer), &*_Buffers.begin());	
	// Release internal resources of all remaining IEffect instances
	if (!_Effects.empty())
	{
		nlwarning("AL: _Effects.size(): '%u'", (uint32)_Effects.size());
		set<CEffectAL *>::iterator it(_Effects.begin()), end(_Effects.end());
		for (; it != end; ++it) (*it)->release(); // CEffectAL will be deleted by user
		_Effects.clear();
	}

	// OpenAL exit
	if (_AlContext) { alcDestroyContext(_AlContext); _AlContext = NULL; }
	if (_AlDevice) { alcCloseDevice(_AlDevice); _AlDevice = NULL; }
}

/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
void CSoundDriverAL::getDevices(std::vector<std::string> &devices)
{
	devices.push_back(""); // empty

	if (AlEnumerateAllExt)
	{	
		const ALchar* deviceNames = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		// const ALchar* defaultDevice = NULL;
		if(!strlen(deviceNames))
		{
			nldebug("AL: No audio devices");
		}
		else
		{
			nldebug("AL: Listing devices: ");
			while(deviceNames && *deviceNames)
			{
				nldebug("AL:   - %s", deviceNames);
				devices.push_back(deviceNames);
				deviceNames += strlen(deviceNames) + 1;
			}
		}
	}
	else
	{
		nldebug("AL: ALC_ENUMERATE_ALL_EXT not present");
	}
}

static const ALchar *getDeviceInternal(const std::string &device)
{
	if (device.empty()) return NULL;
	if (AlEnumerateAllExt)
	{	
		const ALchar* deviceNames = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		if(!strlen(deviceNames))
		{
			nldebug("AL: No audio devices");
		}
		else
		{
			while (deviceNames && *deviceNames)
			{
				if (!strcmp(deviceNames, device.c_str()))
					return deviceNames;
				deviceNames += strlen(deviceNames) + 1;
			}
		}
	}
	else
	{
		nldebug("AL: ALC_ENUMERATE_ALL_EXT not present");
	}
	nldebug("AL: Device '%s' not found", device.c_str());
	return NULL;
}

/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
void CSoundDriverAL::initDevice(const std::string &device, ISoundDriver::TSoundOptions options)
{
	// list of supported options in this driver
	// no adpcm, no manual rolloff (for now)
	const sint supportedOptions = 
		OptionEnvironmentEffects
		| OptionSoftwareBuffer
		| OptionManualRolloff
		| OptionLocalBufferCopy
		| OptionHasBufferStreaming;

	// list of forced options in this driver
	const sint forcedOptions = 0;

	// set the options
	_Options = (TSoundOptions)(((sint)options & supportedOptions) | forcedOptions);
	
	/* TODO: multichannel */

	// OpenAL initialization
	const ALchar *dev = getDeviceInternal(device);
	if (!dev) dev = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	nldebug("AL: Opening device: '%s'", dev == NULL ? "NULL" : dev);
	_AlDevice = alcOpenDevice(dev);
	if (!_AlDevice) throw ESoundDriver("AL: Failed to open device");
	nldebug("AL: ALC_DEVICE_SPECIFIER: '%s'", alcGetString(_AlDevice, ALC_DEVICE_SPECIFIER));
	//int attrlist[] = { ALC_FREQUENCY, 48000,
	//                   ALC_MONO_SOURCES, 12, 
	//                   ALC_STEREO_SOURCES, 4, 
	//                   ALC_INVALID };
	_AlContext = alcCreateContext(_AlDevice, NULL); // attrlist);
	if (!_AlContext) { alcCloseDevice(_AlDevice); throw ESoundDriver("AL: Failed to create context"); }
	alcMakeContextCurrent(_AlContext);
	alTestError();

	// Display version information
	const ALchar *alversion, *alrenderer, *alvendor, *alext;
	alversion = alGetString(AL_VERSION);
	alrenderer = alGetString(AL_RENDERER);
	alvendor = alGetString(AL_VENDOR);
	alext = alGetString(AL_EXTENSIONS);
	alTestError();
	nldebug("AL: AL_VERSION: '%s', AL_RENDERER: '%s', AL_VENDOR: '%s'", alversion, alrenderer, alvendor);
	nldebug("AL: AL_EXTENSIONS: %s", alext);

	// Load and display extensions
	alExtInitDevice(_AlDevice);
#if EAX_AVAILABLE
	nlinfo("AL: EAX: %s, EAX-RAM: %s, ALC_EXT_EFX: %s", 
		AlExtEax ? "Present" : "Not available", 
		AlExtXRam ? "Present" : "Not available", 
		AlExtEfx ? "Present" : "Not available");
#else
	nldebug("AL: EAX-RAM: %s, ALC_EXT_EFX: %s", 
		AlExtXRam ? "Present" : "Not available", 
		AlExtEfx ? "Present" : "Not available");
#endif
	alTestError();

	nldebug("AL: Max. sources: %u, Max. effects: %u", (uint32)countMaxSources(), (uint32)countMaxEffects());

	if (getOption(OptionEnvironmentEffects)) 
	{
		if (!AlExtEfx)
		{
			nlwarning("AL: ALC_EXT_EFX is required, environment effects disabled");
			_Options = (TSoundOptions)((uint)_Options & ~OptionEnvironmentEffects);
		}
		else if (!countMaxEffects())
		{		
			nlwarning("AL: No effects available, environment effects disabled");
			_Options = (TSoundOptions)((uint)_Options & ~OptionEnvironmentEffects);
		}
	}

	// Choose the I3DL2 model (same as DirectSound3D if not manual)
	if (getOption(OptionManualRolloff)) alDistanceModel(AL_NONE);
	else alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	alTestError();

	// Initial buffers and sources allocation
	allocateNewItems(alGenBuffers, alIsBuffer, _Buffers, _NbExpBuffers, INITIAL_BUFFERS);
	alTestError();
}

/// Return options that are enabled (including those that cannot be disabled on this driver).
ISoundDriver::TSoundOptions CSoundDriverAL::getOptions()
{
	return _Options;
}

/// Return if an option is enabled (including those that cannot be disabled on this driver).
bool CSoundDriverAL::getOption(ISoundDriver::TSoundOptions option)
{
	return ((uint)_Options & (uint)option) == (uint)option;
}

/*
 * Allocate nb new items
 */
void CSoundDriverAL::allocateNewItems(TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
									  vector<ALuint>& names, uint index, uint nb )
{
	nlassert( index == names.size() );
	names.resize( index + nb );
	// FIXME assumption about inner workings of std::vector;
	// &(names[...]) only works with "names.size() - nbalive == 1"
	generateItems( algenfunc, altestfunc, nb, &(names[index]) );
}


/*
 * throwGenException
 */
void ThrowGenException( TGenFunctionAL algenfunc )
{
	if ( algenfunc == alGenBuffers )
		throw ESoundDriverGenBuf();
	else if ( algenfunc == alGenSources )
		throw ESoundDriverGenSrc();
	else
		nlstop;
}

/*
 * Generate nb buffers/sources
 */
void CSoundDriverAL::generateItems( TGenFunctionAL algenfunc, TTestFunctionAL altestfunc, uint nb, ALuint *array )
{
	// array is actually a std::vector element address!
	algenfunc( nb, array );

	// Error handling
	if ( alGetError() != AL_NO_ERROR )
	{
		ThrowGenException( algenfunc );
	}

	// Check buffers
	uint i;
	for ( i=0; i!=nb; i++ )
	{
		if ( ! altestfunc( array[i] ) )
		{
			ThrowGenException( algenfunc );
		}
	}
}

/*
 * Create a sound buffer
 */
IBuffer *CSoundDriverAL::createBuffer()
{
	CBufferAL *buffer = new CBufferAL(createItem(alGenBuffers, alIsBuffer, _Buffers, _NbExpBuffers, BUFFER_ALLOC_RATE));
	return buffer;
}


/*
 * Create a source
 */
ISource *CSoundDriverAL::createSource()
{
	CSourceAL *sourceAl = new CSourceAL(this);
	_Sources.insert(sourceAl);
	return sourceAl;
}

/// Create a reverb effect
IReverbEffect *CSoundDriverAL::createReverbEffect()
{
	IReverbEffect *ieffect = NULL;
	CEffectAL *effectal = NULL;
	
	ALuint slot = AL_NONE;
	alGenAuxiliaryEffectSlots(1, &slot);
	if (alGetError() != AL_NO_ERROR)
	{
		nlwarning("AL: alGenAuxiliaryEffectSlots failed");
		return NULL;
	}
	
	ALuint effect = AL_NONE;
	alGenEffects(1, &effect);
	if (alGetError() != AL_NO_ERROR)
	{
		nlwarning("AL: alGenEffects failed");
		alDeleteAuxiliaryEffectSlots(1, &slot);
		return NULL; /* createEffect */
	}

#if EFX_CREATIVE_AVAILABLE
	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
	if (alGetError() != AL_NO_ERROR)
	{
		nlinfo("AL: Creative Reverb Effect not supported, falling back to standard Reverb Effect");
	}
	else
	{
		alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect); alTestError();
		alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE); alTestError(); // auto only for reverb!
		CCreativeReverbEffectAL *eff = new CCreativeReverbEffectAL(this, effect, slot);
		ieffect = static_cast<IReverbEffect *>(eff);
		effectal = static_cast<CEffectAL *>(eff);
		nlassert(ieffect); nlassert(effectal);
		_Effects.insert(effectal);
		return ieffect;
	}
#endif		

	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	if (alGetError() != AL_NO_ERROR)
	{
		nlwarning("AL: Reverb Effect not supported");
		alDeleteAuxiliaryEffectSlots(1, &slot);
		alDeleteEffects(1, &effect);
		return NULL; /* createEffect */
	}
	else
	{
		alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, effect); alTestError();
		alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE); alTestError(); // auto only for reverb!
		CStandardReverbEffectAL *eff = new CStandardReverbEffectAL(this, effect, slot);
		ieffect = static_cast<IReverbEffect *>(eff);
		effectal = static_cast<CEffectAL *>(eff);
		nlassert(ieffect); nlassert(effectal);
		_Effects.insert(effectal);
		return ieffect;
	}
}


static uint getMaxNumSourcesInternal()
{
	ALuint sources[256];
	memset(sources, 0, sizeofarray(sources));
	uint sourceCount = 0;
	
	alGetError();
	
	for (; sourceCount < 256; ++sourceCount)
	{
		alGenSources(1, &sources[sourceCount]);
		if (alGetError() != AL_NO_ERROR)
			break;
	}
	
	alDeleteSources(sourceCount, sources);
	if (alGetError() != AL_NO_ERROR)
	{
		for (uint i = 0; i < 256; i++)
		{
			alDeleteSources(1, &sources[i]);
		}
	}

	alGetError();

	return sourceCount;
}

/// Return the maximum number of sources that can created
uint CSoundDriverAL::countMaxSources()
{
	// ALC_MONO_SOURCES
	// software allows 256 sources (software audio ftw!)
	// cheap openal cards 32, expensive openal cards 128
	// trying to go too high is safely handled anyways
	return getMaxNumSourcesInternal() + (uint)_Sources.size();
}

/// Return the maximum number of effects that can be created, which is only 1 in openal software mode :(
uint CSoundDriverAL::countMaxEffects()
{
	if (!getOption(OptionEnvironmentEffects)) return 0;
	if (!AlExtEfx) return 0;
	ALCint max_auxiliary_sends;
	alcGetIntegerv(_AlDevice, ALC_MAX_AUXILIARY_SENDS, 1, &max_auxiliary_sends);
	return (uint)max_auxiliary_sends;
}

/*
 * Create a sound buffer or a sound source
 */
ALuint CSoundDriverAL::createItem(TGenFunctionAL algenfunc, TTestFunctionAL altestfunc,
								  vector<ALuint>& names, uint& index, uint allocrate)
{
	nlassert( index <= names.size() );
	if ( index == names.size() )
	{
		// Generate new items
		uint nbalive = compactAliveNames( names, altestfunc );
		if ( nbalive == names.size() )
		{
			// Extend vector of names
			// FIXME? assumption about inner workings of std::vector
			allocateNewItems( algenfunc, altestfunc, names, index, allocrate );
		}
		else
		{
			// Take the room of the deleted names
			nlassert(nbalive < names.size());
			index = nbalive;
			// FIXME assumption about inner workings of std::vector;
			// &(names[...]) only works with "names.size() - nbalive == 1"
			generateItems(algenfunc, altestfunc, (uint)names.size() - nbalive, &(names[nbalive]));
		}
	}

	// Return the name of the item
	nlassert( index < names.size() );
	ALuint itemname = names[index];
	index++;
	return itemname;
}


/*
 * Remove names of deleted buffers and return the number of valid buffers
 */
uint CSoundDriverAL::compactAliveNames( vector<ALuint>& names, TTestFunctionAL altestfunc )
{
	vector<ALuint>::iterator iball, ibcompacted;
	for ( iball=names.begin(), ibcompacted=names.begin(); iball!=names.end(); ++iball )
	{
		// iball is incremented every iteration
		// ibcompacted is not incremented if a buffer is not valid anymore
		if ( altestfunc( *iball ) )
		{
			*ibcompacted = *iball;
			++ibcompacted;
		}
	}
	nlassert( ibcompacted <= names.end() );
	return (uint)(ibcompacted - names.begin());
}


void CSoundDriverAL::commit3DChanges()
{
	// Sync up sources & listener 3d position.
	if (getOption(OptionManualRolloff))
	{
		for (std::set<CSourceAL *>::iterator it(_Sources.begin()), end(_Sources.end()); it != end; ++it)
			(*it)->updateManualRolloff();
	}
}

/// Write information about the driver to the output stream.
void CSoundDriverAL::writeProfile(std::string& out)
{
	out = toString("OpenAL\n");
	out += toString("Source size: %u\n", (uint32)_Sources.size());
	out += toString("Effects size: %u\n", (uint32)_Effects.size());

	// TODO: write other useful information like OpenAL version and supported extensions
}

// Does not create a sound loader .. what does it do then?
void CSoundDriverAL::startBench()
{
	NLMISC::CHTimer::startBench();
}

void CSoundDriverAL::endBench()
{
	NLMISC::CHTimer::endBench();
}

void CSoundDriverAL::displayBench(NLMISC::CLog *log)
{
	NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	NLMISC::CHTimer::displayHierarchical(log, true, 48, 2);
	NLMISC::CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	NLMISC::CHTimer::display(log, CHTimer::TotalTime);
}

/// Remove a buffer
void CSoundDriverAL::removeBuffer(CBufferAL *buffer)
{
	nlassert(buffer != NULL);
	if (!deleteItem( buffer->bufferName(), alDeleteBuffers, _Buffers))
		nlwarning("AL: Deleting buffer: name not found");
}

/// Remove a source
void CSoundDriverAL::removeSource(CSourceAL *source)
{
	if (_Sources.find(source) != _Sources.end()) _Sources.erase(source);
	else nlwarning("AL: removeSource already called");
}

/// Remove an effect
void CSoundDriverAL::removeEffect(CEffectAL *effect)
{
	if (_Effects.find(effect) != _Effects.end()) _Effects.erase(effect);
	else nlwarning("AL: removeEffect already called");
}

/// Delete a buffer or a source
bool CSoundDriverAL::deleteItem( ALuint name, TDeleteFunctionAL aldeletefunc, vector<ALuint>& names )
{
	vector<ALuint>::iterator ibn = find( names.begin(), names.end(), name );
	if ( ibn == names.end() )
	{
		return false;
	}
	aldeletefunc( 1, &*ibn );
	*ibn = AL_NONE;
	alTestError();
	return true;
}

/// Create the listener instance
IListener *CSoundDriverAL::createListener()
{
	nlassert(!CListenerAL::isInitialized());
	return new CListenerAL();
}

/// Apply changes of rolloff factor to all sources
void CSoundDriverAL::applyRolloffFactor( float f )
{
	_RolloffFactor = f;
	if (!getOption(OptionManualRolloff))
	{
		set<CSourceAL *>::iterator it(_Sources.begin()), end(_Sources.end());
		for (; it != end; ++it) alSourcef((*it)->getSource(), AL_ROLLOFF_FACTOR, _RolloffFactor);
		alTestError();
	}
}

/// Helper for loadWavFile()
TSampleFormat ALtoNLSoundFormat( ALenum alformat )
{
	switch ( alformat )
	{
	case AL_FORMAT_MONO8 : return Mono8;
	case AL_FORMAT_MONO16 : return Mono16;
	case AL_FORMAT_STEREO8 : return Stereo8;
	case AL_FORMAT_STEREO16 : return Stereo16;
	default : nlstop; return Mono8;
	}
}

} // NLSOUND
