// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2012  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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
#include "music_channel_fmod.h"
#include "sound_driver_fmod.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

using namespace std;
using namespace NLMISC;

namespace  NLSOUND
{

signed char F_CALLBACKAPI streamEndCallBack(
	FSOUND_STREAM *stream,
	void * /* buff */,
	int /* len */,
	void *userdata
	)
{
	// Avoid any problem, check that the sound driver is still allocated
	if (!CSoundDriverFMod::getInstance()) return false;

	// mark this fader as music ended
	CSoundDriverFMod::getInstance()->markMusicChannelEnded(stream, static_cast<CMusicChannelFMod *>(userdata));
	
	return true;
}

CMusicChannelFMod::CMusicChannelFMod(CSoundDriverFMod *soundDriver) 
: _Gain(1.0f), _MusicStream(NULL), _MusicBuffer(NULL), 
_MusicChannel(-1), _CallBackEnded(false), _SoundDriver(soundDriver)
{
	
}

CMusicChannelFMod::~CMusicChannelFMod()
{
	stop();
	_SoundDriver->removeMusicChannel(this);
}

/// Play async, if bnp give path of bnp and position and size of file inside, else just path to file with fileSize 0.
bool CMusicChannelFMod::playAsync(const std::string &filepath, bool loop, uint fileOffset , uint fileSize)
{
	nlassert(!_MusicBuffer);

	// open fmod stream async
	_MusicStream = FSOUND_Stream_Open((const char *)filepath.c_str(),
		FSOUND_2D | ( loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF) | FSOUND_NONBLOCKING, fileOffset, fileSize);
	nlassert(_MusicStream);
	
	// with FSOUND_NONBLOCKING, the file is surely not ready, but still try now (will retry to replay at each updateMusic())
	playStream();
	
	nlassert(!_MusicBuffer);
	return true;
}

/// Play from memory.
bool CMusicChannelFMod::playSync(const std::string &filepath, bool loop)
{
	CIFile ifile;
	ifile.allowBNPCacheFileOnOpen(false);
	ifile.setCacheFileOnOpen(false);
	ifile.open(filepath);

	// try to load the music in memory
	uint32 fs = ifile.getFileSize();
	if (!fs) { nlwarning("NLSOUND FMod Driver: Empty music file"); return false; }
	
	// read Buffer
	nlassert(!_MusicBuffer);
	_MusicBuffer = new uint8[fs];
	try { ifile.serialBuffer(_MusicBuffer, fs); }
	catch (...) 
	{ 
		nlwarning("NLSOUND FMod Driver: Error while reading music file"); 
		delete[] _MusicBuffer; _MusicBuffer = NULL; 
		return false; 
	}

	// open FMOD stream
	_MusicStream = FSOUND_Stream_Open((const char*)_MusicBuffer,
		FSOUND_2D | FSOUND_LOADMEMORY | (loop ? FSOUND_LOOP_NORMAL : FSOUND_LOOP_OFF), 0, fs);
	if (!_MusicStream)
	{ 
		nlwarning("NLSOUND FMod Driver: Error while creating the FMOD stream for music file"); 
		delete[] _MusicBuffer; _MusicBuffer = NULL; 
		return false; 
	}

	if (!playStream())
	{
		nlwarning("NLSOUND FMod Driver: Error While trying to play sync music file"); 
		FSOUND_Stream_Close(_MusicStream); _MusicStream = NULL; 
		delete[] _MusicBuffer; _MusicBuffer = NULL; 
		return false;
	}

	return true;
}

bool CMusicChannelFMod::playStream()
{
	if (FSOUND_Stream_GetOpenState(_MusicStream) == -3)
	{
		nlwarning("NLSOUND FMod Driver: stream failed to open. (file not found, out of memory or other error)");
		FSOUND_Stream_Close(_MusicStream); _MusicStream = NULL; 
		return false;
	}

	// Start playing
	if ((_MusicChannel = FSOUND_Stream_PlayEx(FSOUND_FREE, _MusicStream, NULL, true)) == -1)
		return false;

	// stereo pan (as reccomended)
	FSOUND_SetPan(_MusicChannel, FSOUND_STEREOPAN);
	// update volume
	int vol255 = (int)(_Gain * 255.0f);
	FSOUND_SetVolumeAbsolute(_MusicChannel, vol255);
	// Set a callback to know if stream has ended
	_CallBackEnded = false;
	FSOUND_Stream_SetEndCallback(_MusicStream, streamEndCallBack, static_cast<void *>(this));
	// unpause
	FSOUND_SetPaused(_MusicChannel, false);

	return true;
}

void CMusicChannelFMod::update()
{
	// if this channel is playing an async music, may retry to start the music each frame
	if (_MusicStream && _MusicBuffer == NULL && _MusicChannel == -1)
	{
		// keeeep trying
		playStream();
	}

	// close anything that still needs to be closed
	updateWaitingForClose();
}

void CMusicChannelFMod::updateWaitingForClose()
{
	std::list<FSOUND_STREAM*>::iterator	it= _WaitingForClose.begin();
	while (it != _WaitingForClose.end())
	{
		// try to stop, will fail if still loading
		bool ok = FSOUND_Stream_Stop(*it) != 0;
		if (ok) ok = FSOUND_Stream_Close(*it) !=0;
		// erase from list, or next
		if (ok) it = _WaitingForClose.erase(it);
		else ++it;
	}
}

void CMusicChannelFMod::markMusicChannelEnded(void *stream)
{
	if (stream == _MusicStream)
		_CallBackEnded = true;
}

/** Play some music (.ogg etc...)
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param filepath file path, CPath::lookup is done here
 *  \param async stream music from hard disk, preload in memory if false
 *	\param loop must be true to play the music in loop. 
 */
bool CMusicChannelFMod::play(const std::string &filepath, bool async, bool loop)
{
	// stop
	stop();
	// stuff that was in nlsound for async file in bnp to work with fmod
	string pathName = CPath::lookup(filepath, false);
	if (pathName.empty())
	{
		nlwarning("NLSOUND FMod Driver: Music file %s not found!", filepath.c_str());
		return false;
	}
	if (async)
	{
		// if the file is in a bnp
		if (pathName.find('@') != string::npos)
		{
			// get info for location in this bnp
			uint32 fileOffset, fileSize;
			if (CBigFile::getInstance().getFileInfo(pathName, fileSize, fileOffset))
			{
				// then play async this bnp file (with offset/size)
				string bnpName = pathName.substr(0, pathName.find('@'));
				return playAsync(CPath::lookup(bnpName, false), loop, fileOffset, fileSize);
			}
			else
			{
				nlwarning("NLSOUND FMod Driver: BNP BROKEN");
				return false;
			}
		}
		// else standard file
		else
		{
			// play it async
			return playAsync(pathName, loop);
		}
	}
	else
	{
		return playSync(pathName, loop);
	}
}

/// Stop the music previously loaded and played (the Memory is also freed)
void CMusicChannelFMod::stop()
{
	if (_MusicStream)
	{
		/* just append this channel for closing. We have to maintain such a list because in case of async playing,
		   FMod FSOUND_Stream_Stop() and FSOUND_Stream_Close() calls fail if the file is not ready (hapens if music stopped
		   in the 50 ms after the play for instance)
		*/
		_WaitingForClose.push_back(_MusicStream);

		// force stop now (succeed in 99% of case, and if not succeed, it means the play has not yet begun, so no problem)
		updateWaitingForClose();

		// reset
		_MusicChannel = -1;
		_MusicStream = NULL;
	}
	if (_MusicBuffer)
	{
		delete[] _MusicBuffer;
		_MusicBuffer = NULL;
	}
	_CallBackEnded = false;
}

void CMusicChannelFMod::reset()
{
	// don't care
	stop();
}

/** Pause the music previously loaded and played (the Memory is not freed)
 */
void CMusicChannelFMod::pause()
{
	if (_MusicChannel != -1)
		FSOUND_SetPaused(_MusicChannel, true);
}

/// Resume the music previously paused
void CMusicChannelFMod::resume()
{
	if (_MusicChannel != -1)
		FSOUND_SetPaused(_MusicChannel, false);
}

/// Return true if a song is finished.
bool CMusicChannelFMod::isEnded()
{
	if (_MusicStream && _MusicChannel != -1)
	{
		// test with file position
		if ((int)FSOUND_Stream_GetPosition(_MusicStream) == FSOUND_Stream_GetLength(_MusicStream))
			return true;

		// NB: the preceding code don't work with .ogg vorbis encoded mp3. Thus test also the end with a callback
		if(_CallBackEnded)
			return true;
	}
	// if playing, but not starting because of async, not ended (because not even really started)
	else if (_MusicStream) return false;

	return false;
}

/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
bool CMusicChannelFMod::isLoadingAsync()
{
	return _MusicStream && _MusicBuffer == NULL && _MusicChannel == -1;
}

/// Return the total length (in second) of the music currently played
float CMusicChannelFMod::getLength()
{
	if (_MusicStream && _MusicChannel != -1)
		return FSOUND_Stream_GetLengthMs(_MusicStream) / 1000.f;
	return 0.f;
}

/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
 *	NB: the volume of music is NOT affected by IListener::setGain()
 */
void CMusicChannelFMod::setVolume(float gain)
{
	_Gain = gain;
	if (_MusicStream && _MusicChannel != -1)
	{
		int vol255 = (int)(gain * 255.0f);
		FSOUND_SetVolumeAbsolute(_MusicChannel, vol255);
	}
}

}
