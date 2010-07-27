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

// Project includes
#include "sound_driver_al.h"
#include "source_al.h"
#include "buffer_al.h"
#include "music_channel_al.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND
{

CMusicChannelAL::CMusicChannelAL(CSoundDriverAL *soundDriver) 
: _MusicBuffer(NULL), _SoundDriver(soundDriver), _Gain(1.0), _Source(NULL), _Thread(NULL), _Async(false), _Playing(false), _Buffer(NULL)
{
	// create a default source for music streaming
	_Source = static_cast<CSourceAL*>(_SoundDriver->createSource());
	_Source->setPos(CVector(0, 0, 0));
	_Source->setVelocity(CVector(0, 0, 0));
	_Source->setDirection(CVector(0, 0, 0));
	_Source->setSourceRelativeMode(true);
	_Source->setStreamingBuffersMax(4);
	_Source->setStreamingBufferSize(32768);
//	_Source->setStreaming(true);
}

CMusicChannelAL::~CMusicChannelAL()
{
	release();
	if (_SoundDriver) { _SoundDriver->removeMusicChannel(this); _SoundDriver = NULL; }
}

void CMusicChannelAL::release()
{
	// stop thread before deleting it
	stop();

	// delete thread
	if (_Thread)
	{
		delete _Thread;
		_Thread = NULL;
	}

	// delete source
	if (_Source)
	{
		delete _Source;
		_Source = NULL;
	}
}

/// Fill IBuffer with data from IMusicBuffer
bool CMusicChannelAL::fillBuffer(IBuffer *buffer, uint length)
{
	if (!buffer || !length)
	{
		nlwarning("AL: No data to stream");
		return false;
	}

	// fill buffer with music data
	uint8 *tmp = buffer->lock(length);
	if (tmp == NULL)
	{
		nlwarning("AL: Can't allocate %u bytes for buffer", length);
		return false;
	}

	uint32 size = _MusicBuffer->getNextBytes(tmp, length, length);
	buffer->unlock(size);

	// add buffer to streaming buffers queue
	_Source->submitStreamingBuffer(buffer);

	return true;
}

/// Use buffer format from IMusicBuffer
void CMusicChannelAL::setBufferFormat(IBuffer *buffer)
{
	if (!buffer)
	{
		nlwarning("AL: No buffer specified");
		return;
	}

	// use the same format as music for buffers
	buffer->setFormat(IBuffer::FormatPcm, _MusicBuffer->getChannels(),
		_MusicBuffer->getBitsPerSample(), _MusicBuffer->getSamplesPerSec());
}

void CMusicChannelAL::run()
{

	if (_Async)
	{
		bool first = true;

		// use queued buffers
		do
		{
			// buffers to update
			std::vector<CBufferAL*> buffers;

			if (first)
			{
				// get all buffers to queue
				_Source->getStreamingBuffers(buffers);

				// set format for each buffer
				for(uint i = 0; i < buffers.size(); ++i)
					setBufferFormat(buffers[i]);
			}
			else
			{
				// get unqueued buffers
				_Source->getProcessedStreamingBuffers(buffers);
			}

			// fill buffers
			for(uint i = 0; i < buffers.size(); ++i)
				fillBuffer(buffers[i], _Source->getStreamingBufferSize());

			// play the source
			if (first)
			{
				_Source->play();
				first = false;
			}

			// wait 100ms before rechecking buffers
			nlSleep(100);
		}
		while(!_MusicBuffer->isMusicEnded() && _Playing);
	}
	else
	{
		// use an unique buffer managed by CMusicChannelAL
		_Buffer = _SoundDriver->createBuffer();

		// set format
		setBufferFormat(_Buffer);

		// fill data
		fillBuffer(_Buffer, _MusicBuffer->getUncompressedSize());

		// we don't need _MusicBuffer anymore because all is loaded into memory
		if (_MusicBuffer)
		{
			delete _MusicBuffer;
			_MusicBuffer = NULL;
		}

		// use this buffer as source
		_Source->setStaticBuffer(_Buffer);

		// play the source
		_Source->play();
	}

	// music finished without interruption
	if (_Playing)
	{
		// wait until source is not playing
		while(_Source->isPlaying() && _Playing) nlSleep(1000);

		_Source->stop();

		_Playing = false;
	}
}

/** Play some music (.ogg etc...)
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param filepath file path, CPath::lookup is done here
 *  \param async stream music from hard disk, preload in memory if false
 *	\param loop must be true to play the music in loop. 
 */
bool CMusicChannelAL::play(const std::string &filepath, bool async, bool loop)
{
	// stop a previous music
	stop();

	// when not using async, we must load the whole file once
	_MusicBuffer = IMusicBuffer::createMusicBuffer(filepath, async, async ? loop:false);

	if (_MusicBuffer)
	{
		// create the thread if it's not yet created
		if (!_Thread) _Thread = IThread::create(this);

		if (!_Thread)
		{
			nlwarning("AL: Can't create a new thread");
			return false;
		}

		_Async = async;
		_Playing = true;

		// we need to loop the source only if not async
		_Source->setLooping(async ? false:loop);

		// start the thread
		_Thread->start();
	}
	else
	{
		nlwarning("AL: Can't stream file %s", filepath.c_str());
		return false;
	}
	
	return true;
}

/// Stop the music previously loaded and played (the Memory is also freed)
void CMusicChannelAL::stop()
{
	_Playing = false;

	_Source->stop();

	// if not using async streaming, we manage static buffer ourself
	if (!_Async && _Buffer)
	{
		_Source->setStaticBuffer(NULL);
		delete _Buffer;
		_Buffer = NULL;
	}

	// wait until thread is finished
	if (_Thread)
		_Thread->wait();

	if (_MusicBuffer)
	{
		delete _MusicBuffer;
		_MusicBuffer = NULL;
	}
}

/// Pause the music previously loaded and played (the Memory is not freed)
void CMusicChannelAL::pause()
{
	_Source->pause();
}

/// Resume the music previously paused
void CMusicChannelAL::resume()
{
	_Source->play();
}

/// Return true if a song is finished.
bool CMusicChannelAL::isEnded()
{
	return !_Playing;
}

/// Return true if the song is still loading asynchronously and hasn't started playing yet (false if not async), used to delay fading
bool CMusicChannelAL::isLoadingAsync()
{
	return _Async && _Playing;
}

/// Return the total length (in second) of the music currently played
float CMusicChannelAL::getLength()
{
	if (_MusicBuffer) return _MusicBuffer->getLength(); 
	else return .0f;
}

/** Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
 *	NB: in OpenAL driver, the volume of music IS affected by IListener::setGain()
 */
void CMusicChannelAL::setVolume(float gain)
{
	_Gain = gain;
	_Source->setGain(gain);
}

} /* namespace NLSOUND */

/* end of file */
