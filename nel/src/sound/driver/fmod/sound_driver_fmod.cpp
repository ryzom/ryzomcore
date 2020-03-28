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

#include "stdfmod.h"

#include "sound_driver_fmod.h"
#include "listener_fmod.h"
#include "music_channel_fmod.h"
#include "source_fmod.h"
#include "buffer_fmod.h"

#ifdef NL_OS_WINDOWS
#include <Windows.h>
#endif

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;


namespace NLSOUND
{

#ifndef NL_STATIC

class CSoundDriverFModNelLibrary : public NLMISC::INelLibrary {
	void onLibraryLoaded(bool /* firstTime */) { }
	void onLibraryUnloaded(bool /* lastTime */) { }
};
NLMISC_DECL_PURE_LIB(CSoundDriverFModNelLibrary)

#endif /* #ifndef NL_STATIC */

#ifdef NL_OS_WINDOWS
#ifndef NL_STATIC

HINSTANCE CSoundDriverDllHandle = 0;

// ******************************************************************
// The main entry of the DLL. It's used to get a hold of the hModule handle.
BOOL WINAPI DllMain(HANDLE hModule, DWORD /* ul_reason_for_call */, LPVOID /* lpReserved */)
{
  CSoundDriverDllHandle = (HINSTANCE) hModule;
  return TRUE;
}

#endif /* #ifndef NL_STATIC */

// ***************************************************************************

#ifdef NL_STATIC
ISoundDriver* createISoundDriverInstanceFMod
#else
__declspec(dllexport) ISoundDriver *NLSOUND_createISoundDriverInstance
#endif
	(ISoundDriver::IStringMapperProvider *stringMapper)
{

	return new CSoundDriverFMod(stringMapper);
}

// ******************************************************************

#ifdef NL_STATIC
uint32 interfaceVersionFMod()
#else
__declspec(dllexport) uint32 NLSOUND_interfaceVersion()
#endif
{
	return ISoundDriver::InterfaceVersion;
}

// ******************************************************************

#ifdef NL_STATIC
void outputProfileFMod
#else
__declspec(dllexport) void NLSOUND_outputProfile
#endif
	(string &out)
{
	CSoundDriverFMod::getInstance()->writeProfile(out);
}

// ******************************************************************

#ifdef NL_STATIC
ISoundDriver::TDriver getDriverTypeFMod()
#else
__declspec(dllexport) ISoundDriver::TDriver NLSOUND_getDriverType()
#endif
{
	return ISoundDriver::DriverFMod;
}

#elif defined (NL_OS_UNIX)
extern "C"
{
ISoundDriver *NLSOUND_createISoundDriverInstance(ISoundDriver::IStringMapperProvider *stringMapper)
{
	return new CSoundDriverFMod(stringMapper);
}
uint32 NLSOUND_interfaceVersion ()
{
	return ISoundDriver::InterfaceVersion;
}
} // EXTERN "C"
#endif // NL_OS_UNIX

// ******************************************************************

CSoundDriverFMod::CSoundDriverFMod(ISoundDriver::IStringMapperProvider *stringMapper)
: _StringMapper(stringMapper), _FModOk(false), _MasterGain(1.f), _ForceSoftwareBuffer(false)
{
	
}

// ******************************************************************

CSoundDriverFMod::~CSoundDriverFMod()
{
	//nldebug("Destroying FMOD");

	// Stop any played music
	{
		set<CMusicChannelFMod *>::iterator it(_MusicChannels.begin()), end(_MusicChannels.end());
		for (; it != end; ++it)
		{
			nlwarning("CMusicChannelFMod was not deleted by user, deleting now!");
			delete *it;
		}
		_MusicChannels.clear();
	}


	// Assure that the remaining sources have released all their channels before closing
	set<CSourceFMod*>::iterator iter;
	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		(*iter)->release();
	}


	// Assure that the listener has released all resources before closing down FMod
	if (CListenerFMod::getInstance() != 0)
	{
		CListenerFMod::getInstance()->release();
	}

	// Close FMod
	if(_FModOk)
	{
		FSOUND_Close();
		_FModOk= false;

#ifdef NL_OS_WINDOWS
		// workaround for fmod bug
		CoUninitialize();
#endif
	}
}

/// Return a list of available devices for the user. The value at index 0 is empty, and is used for automatic device selection.
void CSoundDriverFMod::getDevices(std::vector<std::string> &devices)
{
	devices.push_back(""); // empty
}

/// Initialize the driver with a user selected device. If device.empty(), the default or most appropriate device is used.
void CSoundDriverFMod::initDevice(const std::string &device, TSoundOptions options)
{
	// list of supported options in this driver
	// no adpcm, no effects, no buffer streaming
	const sint supportedOptions = 
		OptionSoftwareBuffer
		| OptionManualRolloff
		| OptionLocalBufferCopy;

	// list of forced options in this driver
	const sint forcedOptions = 0;

	// set the options
	_Options = (TSoundOptions)(((sint)options & supportedOptions) | forcedOptions);

	uint initFlags = 0;
#ifdef NL_OS_WINDOWS
	initFlags = FSOUND_INIT_DSOUND_DEFERRED;
#endif

	// Init with 32 channels, and deferred sound
	if (!FSOUND_Init(22050, 32, initFlags))
	{
		throw ESoundDriver("Failed to create the FMod driver object");
	}

	// succeed
	_FModOk = true;

	// Allocate buffer in software?
	_ForceSoftwareBuffer = getOption(OptionSoftwareBuffer);

	// Display Hardware Support
	int num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);
	nlinfo("FMod Hardware Support: %d 2D channels, %d 3D channels, %d Total Channels", num2D, num3D, numTotal);
}

/// Return options that are enabled (including those that cannot be disabled on this driver).
ISoundDriver::TSoundOptions CSoundDriverFMod::getOptions()
{
	return _Options;
}

/// Return if an option is enabled (including those that cannot be disabled on this driver).
bool CSoundDriverFMod::getOption(ISoundDriver::TSoundOptions option)
{
	return ((uint)_Options & (uint)option) == (uint)option;
}

// ******************************************************************

uint CSoundDriverFMod::countMaxSources()
{
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);

	// Try the hardware 3d buffers first
	if (num3D > 0)
	{
		return num3D;
	}

	// If not, try the hardware 2d buffers first
	if (num2D > 0)
	{
		return num2D;
	}

	// Okay, we'll use 32 software buffers
	return 32;
}

// ******************************************************************

void CSoundDriverFMod::writeProfile(string& out)
{
	out+= "\tFMod Driver\n";

    // Write the number of hardware buffers
	int		num2D, num3D, numTotal;
	FSOUND_GetNumHWChannels(&num2D, &num3D, &numTotal);

    out += "\t3d hw buffers: " + toString ((uint32)num3D) + "\n";
	out += "\t2d hw buffers: " + toString ((uint32)num2D) + "\n";
}


// ******************************************************************

void CSoundDriverFMod::update()
{
	H_AUTO(NLSOUND_FModUpdate)

	set<CSourceFMod*>::iterator first(_Sources.begin()), last(_Sources.end());
	for (;first != last; ++first)
	{
		if ((*first)->needsUpdate())
		{
			(*first)->update();
		}
	}
}

// ******************************************************************

IListener *CSoundDriverFMod::createListener()
{

    if (CListenerFMod::isInitialized())
    {
        return CListenerFMod::getInstance();
    }

    if ( !_FModOk )
        throw ESoundDriver("Corrupt driver");

    return new CListenerFMod();
}

// ******************************************************************

IBuffer *CSoundDriverFMod::createBuffer()
{

    if ( !_FModOk )
        throw ESoundDriver("Corrupt driver");

    return new CBufferFMod();
}

// ******************************************************************

void CSoundDriverFMod::removeBuffer(CBufferFMod * /* buffer */)
{
}

// ******************************************************************

ISource *CSoundDriverFMod::createSource()
{

    if ( !_FModOk )
        throw ESoundDriver("Corrupt driver");

	CSourceFMod* src = new CSourceFMod(0);
	src->init();
	_Sources.insert(src);

	return src;
}


// ******************************************************************

void CSoundDriverFMod::removeSource(CSourceFMod *source)
{
	_Sources.erase((CSourceFMod*) source);
}

// ******************************************************************

void CSoundDriverFMod::removeMusicChannel(CMusicChannelFMod *musicChannel)
{
	_MusicChannels.erase(static_cast<CMusicChannelFMod *>(musicChannel));
}

// ******************************************************************

void CSoundDriverFMod::commit3DChanges()
{
    if ( !_FModOk )
		return;

	if (getOption(OptionManualRolloff))
	{
		// We handle the volume of the source according to the distance
		// ourselves. Call updateVolume() to, well..., update the volume
		// according to, euh ..., the new distance!
		CListenerFMod* listener = CListenerFMod::getInstance();
		if(listener)
		{
			const CVector &origin = listener->getPos();
			set<CSourceFMod*>::iterator iter;
			for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
			{
				if ((*iter)->isPlaying())
				{
					(*iter)->updateVolume(origin);
				}
			}
		}
	}

	// We handle the "SourceRelative state" ourselves. Updates sources according to current listener position/velocity
	set<CSourceFMod*>::iterator iter;
	for (iter = _Sources.begin(); iter != _Sources.end(); iter++)
	{
		(*iter)->updateFModPosIfRelative();
	}

	// update sources state each frame though
	update();

	// update the music (XFade etc...)
	updateMusic();

	// update 3D change in FMod
	FSOUND_Update();
}


// ******************************************************************

uint CSoundDriverFMod::countPlayingSources()
{
    uint n = 0;
	set<CSourceFMod*>::iterator iter;

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

void CSoundDriverFMod::setGain( float gain )
{
	clamp(gain, 0.f, 1.f);
	_MasterGain= gain;

	// set FMod volume
    if ( _FModOk )
	{
		uint	volume255= (uint)floor(_MasterGain*255);
		FSOUND_SetSFXMasterVolume(volume255);
	}
}

// ******************************************************************

float CSoundDriverFMod::getGain()
{
	return _MasterGain;
}


// ***************************************************************************
void	CSoundDriverFMod::startBench()
{
	NLMISC::CHTimer::startBench();
}
void	CSoundDriverFMod::endBench()
{
	NLMISC::CHTimer::endBench();
}
void	CSoundDriverFMod::displayBench(CLog *log)
{
	NLMISC::CHTimer::displayHierarchicalByExecutionPathSorted(log, CHTimer::TotalTime, true, 48, 2);
	NLMISC::CHTimer::displayHierarchical(log, true, 48, 2);
	NLMISC::CHTimer::displayByExecutionPath(log, CHTimer::TotalTime);
	NLMISC::CHTimer::display(log, CHTimer::TotalTime);
}


// ***************************************************************************
void	CSoundDriverFMod::toFModCoord(const CVector &in, float out[3])
{
	out[0]= in.x;
	out[1]= in.z;
	out[2]= in.y;
}

/// Create a music channel
IMusicChannel *CSoundDriverFMod::createMusicChannel()
{
	CMusicChannelFMod *music_channel = new CMusicChannelFMod(this);
	_MusicChannels.insert(music_channel);
	return static_cast<IMusicChannel *>(music_channel);
}

bool getTag (std::string &result, const char *tag, FSOUND_STREAM *stream)
{
	void *name;
	int size;
	char tmp[512];
	int types[]=
	{
		FSOUND_TAGFIELD_ID3V1,
		FSOUND_TAGFIELD_ID3V2,
		FSOUND_TAGFIELD_VORBISCOMMENT,
	};
	uint i;
	for (i=0; i<sizeof(types)/sizeof(int); i++)
	{
		if (FSOUND_Stream_FindTagField(stream, types[i], tag, &name, &size))
		{
			strncpy (tmp, (const char*)name, min((int)sizeof(tmp),size));
			result = trim(string(tmp));
			return true;
		}
	}
	return false;
}

/** Get music info. Returns false if the song is not found or the function is not implemented. 
 *  \param filepath path to file, CPath::lookup done by driver
 *  \param artist returns the song artist (empty if not available)
 *  \param title returns the title (empty if not available)
 */
bool CSoundDriverFMod::getMusicInfo(const std::string &filepath, std::string &artist, std::string &title, float &length)
{
	/* Open a stream, get the tag if it exists, close the stream */
	string pathName = CPath::lookup(filepath, false);
	uint32 fileOffset = 0, fileSize = 0;
	if (pathName.empty())
	{
		nlwarning("NLSOUND FMod Driver: Music file %s not found!", filepath.c_str());
		return false;
	}
	// if the file is in a bnp
	if (pathName.find('@') != string::npos)
	{
		if (CBigFile::getInstance().getFileInfo(pathName, fileSize, fileOffset))
		{
			// set pathname to bnp
			pathName = pathName.substr(0, pathName.find('@'));			
		}
		else
		{
			nlwarning("NLSOUND FMod Driver: BNP BROKEN");
			return false;
		}
	}

	FSOUND_STREAM *stream = FSOUND_Stream_Open((const char *)CPath::lookup(filepath, false).c_str(), FSOUND_2D, (sint)fileOffset, (sint)fileSize);
	if (stream)
	{
		getTag(artist, "ARTIST", stream);
		getTag(title, "TITLE", stream);
		// get length of the music in seconds
		length = (float)FSOUND_Stream_GetLengthMs(stream) / 1000.f;
		FSOUND_Stream_Close(stream);
		return true;
	}
	artist.clear();
	title.clear();
	return false;
}

void CSoundDriverFMod::updateMusic()
{
	set<CMusicChannelFMod *>::iterator it(_MusicChannels.begin()), end(_MusicChannels.end());
	for (; it != end; ++it) (*it)->update();
}

void CSoundDriverFMod::markMusicChannelEnded(void *stream, CMusicChannelFMod *musicChannel)
{
	// verify if it exists
	set<CMusicChannelFMod *>::iterator it(_MusicChannels.find(musicChannel));
	if (it != _MusicChannels.end()) musicChannel->markMusicChannelEnded(stream);
}

/// Get audio/container extensions that are supported natively by the driver implementation.
void CSoundDriverFMod::getMusicExtensions(std::vector<std::string> &extensions) const
{
	extensions.push_back("ogg");
	extensions.push_back("mp3");
	extensions.push_back("mp2");
	extensions.push_back("mp1");
	extensions.push_back("wav");
	extensions.push_back("raw");
}
/// Return if a music extension is supported by the driver's music channel.
bool CSoundDriverFMod::isMusicExtensionSupported(const std::string &extension) const
{
	return (extension == "ogg")
		|| (extension == "mp3")
		|| (extension == "mp2")
		|| (extension == "mp1")
		|| (extension == "wav")
		|| (extension == "raw");
}

} // NLSOUND
