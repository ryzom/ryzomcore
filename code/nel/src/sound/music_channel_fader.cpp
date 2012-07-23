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

#include "stdsound.h"
#include "nel/sound/music_channel_fader.h"

// Project includes
#include "nel/sound/driver/sound_driver.h"
#include "nel/sound/driver/music_channel.h"
#include "nel/sound/source_music_channel.h"

using namespace std;
using namespace NLMISC;

namespace NLSOUND {

CMusicChannelFader::CMusicChannelFader() : _SoundDriver(NULL), _ActiveMusicFader(0), _Gain(1.0f), _LastTime(0)
{

}

CMusicChannelFader::~CMusicChannelFader()
{
	
}

void CMusicChannelFader::init(ISoundDriver *soundDriver)
{
	nlassert(!_SoundDriver);
	_SoundDriver = soundDriver;
	_ActiveMusicFader = 0;
	_Gain = 1.0f;
	_LastTime = CTime::getLocalTime();
	for (uint i = 0; i < _MaxMusicFader; ++i)
	{
		nlassert(!_MusicFader[i].MusicChannel);
		_MusicFader[i].MusicChannel = _SoundDriver->createMusicChannel();
		if (!_MusicFader[i].MusicChannel)
		{
			if (_SoundDriver->getOption(ISoundDriver::OptionHasBufferStreaming))
			{
				_MusicFader[i].MusicChannel = new CSourceMusicChannel();
			}
			else
			{
				release();
				nlwarning("No music channel available!");
				return;
			}
		}
	}
}

void CMusicChannelFader::release()
{
	if (_SoundDriver)
	{
		for (uint i = 0; i < _MaxMusicFader; ++i) if (_MusicFader[i].MusicChannel) 
		{
			delete _MusicFader[i].MusicChannel;
			_MusicFader[i].MusicChannel = NULL;
		}
		_SoundDriver = NULL;
	}
}

void CMusicChannelFader::reset()
{
	for (uint i = 0; i < _MaxMusicFader; ++i) if (_MusicFader[i].MusicChannel) 
	{
		if (_MusicFader[i].MusicChannel)
			_MusicFader[i].MusicChannel->reset();
	}
}

void CMusicChannelFader::update()
{
	TTime current_time = CTime::getLocalTime();
	float delta_time = (float)(current_time - _LastTime) / 1000.0f;
	_LastTime = current_time;
	for (uint i = 0; i < _MaxMusicFader; ++i)
	{
		_CMusicFader &fader = _MusicFader[i];
		if (fader.Playing)
		{
			if (fader.MusicChannel->isEnded())
			{
				fader.MusicChannel->stop();
				fader.Playing = false;
				fader.Fade = false;
			}
			else if (fader.Fade)
			{
				// wait with fading in until the song has started playing (at 0 volume)
				if (!fader.MusicChannel->isLoadingAsync())
				{
					fader.XFadeVolume += fader.XFadeDVolume * delta_time;
					if (fader.XFadeVolume <= 0.f)
					{
						// fadeout complete
						fader.MusicChannel->stop();
						fader.Fade = false;
						fader.Playing = false;
					}
					else if (fader.XFadeVolume >= 1.f)
					{
						// fadein complete
						fader.Fade = false;
						fader.XFadeVolume = 1.f;
					}
				}
			}
		}
	}
	updateVolume();
}

void CMusicChannelFader::updateVolume()
{
	for (uint i = 0; i < _MaxMusicFader; ++i)
		if (_MusicFader[i].Playing)
			_MusicFader[i].MusicChannel->setVolume(_MusicFader[i].XFadeVolume * _Gain);
}

/** Play some music (.ogg etc...)
 *	NB: if an old music was played, it is first stop with stopMusic()
 *	\param filepath file path, CPath::lookup is done here
 *  \param async stream music from hard disk, preload in memory if false
 *	\param loop must be true to play the music in loop. 
 */
bool CMusicChannelFader::play(const std::string &filepath, uint xFadeTime, bool async, bool loop)
{
	stop(xFadeTime);

	// Find the next best free music channel
	uint nextFader = _MaxMusicFader;
	for (uint i = 0; i < _MaxMusicFader; ++i) if (!_MusicFader[i].Playing)
		{ nextFader = i; break; }
	if (nextFader == _MaxMusicFader)
	{
		nextFader = (_ActiveMusicFader + 1) % _MaxMusicFader;
		_MusicFader[nextFader].MusicChannel->stop();
		_MusicFader[nextFader].Fade = false;
		_MusicFader[nextFader].Playing = false;
	}
	_ActiveMusicFader = nextFader;

	// Play a song in it :)
	_CMusicFader &fader = _MusicFader[_ActiveMusicFader];
	if (xFadeTime) fader.fadeIn(xFadeTime);
	else fader.XFadeVolume = 1.0f;
	fader.Playing = true;
	updateVolume(); // make sure at ok volume to start :)
	fader.Playing = fader.MusicChannel->play(filepath, async, loop);
	return fader.Playing;
}

/// Stop the music previously loaded and played (the Memory is also freed)
void CMusicChannelFader::stop(uint xFadeTime)
{
	if (xFadeTime)
	{
		for (uint i = 0; i < _MaxMusicFader; ++i) if (_MusicFader[i].Playing)
			_MusicFader[i].fadeOut(xFadeTime);
	}
	else
	{
		for (uint i = 0; i < _MaxMusicFader; ++i) if (_MusicFader[i].Playing)
		{
			_MusicFader[i].MusicChannel->stop();
			_MusicFader[i].Fade = false;
			_MusicFader[i].Playing = false;
		}
	}
}

/// Pause the music previously loaded and played (the Memory is not freed)
void CMusicChannelFader::pause()
{
	for (uint i = 0; i < _MaxMusicFader; ++i) 
		if (_MusicFader[i].Playing)
			_MusicFader[i].MusicChannel->pause();
}

/// Resume the music previously paused
void CMusicChannelFader::resume()
{
	for (uint i = 0; i < _MaxMusicFader; ++i) 
		if (_MusicFader[i].Playing)
			_MusicFader[i].MusicChannel->resume();
}

/// Return true if all songs are finished.
bool CMusicChannelFader::isEnded()
{
	for (uint i = 0; i < _MaxMusicFader; ++i)
	{
		if (_MusicFader[i].Playing) if (!_MusicFader[i].MusicChannel->isEnded())
			return false; // at least one song still playing
	}
	return true; // no song found that is still playing
}

/// Return the total length (in second) of the music currently played
float CMusicChannelFader::getLength()
{
	return _MusicFader[_ActiveMusicFader].MusicChannel->getLength();
}

/// Set the music volume (if any music played). (volume value inside [0 , 1]) (default: 1)
/// NB: the volume of music is NOT affected by IListener::setGain()
void CMusicChannelFader::setVolume(float gain)
{
	_Gain = gain;
	updateVolume();
}

} /* namespace NLSOUND */

/* end of file */
